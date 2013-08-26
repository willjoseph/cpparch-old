
#ifndef INCLUDED_CPPPARSE_SEMA_SEMADECLARATOR_H
#define INCLUDED_CPPPARSE_SEMA_SEMADECLARATOR_H

#include "SemaCommon.h"
#include "SemaDeclaratorFunction.h"
#include "SemaOperator.h"

struct SemaUnqualifiedDeclaratorId : public SemaBase
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	SemaUnqualifiedDeclaratorId(const SemaState& state)
		: SemaBase(state), id(&gAnonymousId), conversionType(0, context)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::template_id, SemaPolicyPushCached<struct SemaTemplateId>)
	void action(cpp::template_id* symbol, const SemaTemplateIdResult& walker) 
	{
		id = walker.id;
	}
	SEMA_POLICY(cpp::operator_function_id, SemaPolicyPushId<struct SemaOperatorFunctionId>)
	void action(cpp::operator_function_id* symbol, const SemaOperatorFunctionId& walker) 
	{
		symbol->value.value = walker.name;
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::conversion_function_id, SemaPolicyPushId<struct SemaTypeId>)
	void action(cpp::conversion_function_id* symbol, const SemaTypeIdResult& walker) 
	{
		// note: no commit here, because type-id will be uniqued later on..
		symbol->value.value = gConversionFunctionId;
		id = &symbol->value;
		conversionType = walker.type;
	}
	SEMA_POLICY(cpp::destructor_id, SemaPolicyIdentity)
	void action(cpp::destructor_id* symbol) 
	{
		id = &symbol->name->value;
	}
};

struct SemaQualifiedDeclaratorId : public SemaQualified
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	SemaQualifiedDeclaratorId(const SemaState& state)
		: SemaQualified(state), id(&gAnonymousId)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	// in a template member definition, the qualifying nested-name-specifier may be dependent on a template-parameter
	// no need to cache: the nested-name-specifier is not a shared-prefix
	SEMA_POLICY_ARGS(cpp::nested_name_specifier, SemaPolicyPushBool<struct SemaNestedNameSpecifier>, true)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying, true);
	}
	SEMA_POLICY(cpp::unqualified_id, SemaPolicyPush<struct SemaUnqualifiedDeclaratorId>)
	void action(cpp::unqualified_id* symbol, const SemaUnqualifiedDeclaratorId& walker)
	{
		id = walker.id;
	}
};

struct SemaDeclaratorId : public SemaQualified
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	SemaDeclaratorId(const SemaState& state)
		: SemaQualified(state), id(&gAnonymousId), conversionType(0, context)
	{
	}
	SEMA_POLICY(cpp::qualified_id_default, SemaPolicyPush<struct SemaQualifiedDeclaratorId>)
	void action(cpp::qualified_id_default* symbol, const SemaQualifiedDeclaratorId& walker)
	{
		id = walker.id;
		swapQualifying(walker.qualifying, true);
	}
	SEMA_POLICY(cpp::qualified_id_global, SemaPolicyPush<struct SemaQualifiedDeclaratorId>)
	void action(cpp::qualified_id_global* symbol, const SemaQualifiedDeclaratorId& walker)
	{
		id = walker.id;
		swapQualifying(walker.qualifying, true);
	}
	SEMA_POLICY(cpp::unqualified_id, SemaPolicyPush<struct SemaUnqualifiedDeclaratorId>)
	void action(cpp::unqualified_id* symbol, const SemaUnqualifiedDeclaratorId& walker)
	{
		id = walker.id;
		conversionType = walker.conversionType;
	}
};



struct SemaDeclaratorArray : public SemaBase
{
	SEMA_BOILERPLATE;

	Dependent valueDependent;
	ArrayRank rank;
	ExpressionWrapper expression;
	SemaDeclaratorArray(const SemaState& state)
		: SemaBase(state)
	{
	}

	void action(cpp::terminal<boost::wave::T_LEFTBRACKET> symbol)
	{
		// we may parse multiple pairs of brackets: omitted constant-expression indicates an array of unknown size
		expression = ExpressionWrapper();
	}
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker)
	{
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		// [temp.dep.constexpr] An identifier is value-dependent if it is:- a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
		addDependent(valueDependent, walker.valueDependent);
		expression = walker.expression;
	}
	void action(cpp::terminal<boost::wave::T_RIGHTBRACKET> symbol)
	{
		rank.push_back(expression);
	}
};



struct SemaPtrOperator : public SemaQualified
{
	SEMA_BOILERPLATE;

	CvQualifiers qualifiers;
	SemaPtrOperator(const SemaState& state)
		: SemaQualified(state)
	{
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::cv_qualifier_seq, SemaPolicyPush<struct SemaCvQualifierSeq>)
	void action(cpp::cv_qualifier_seq* symbol, const SemaCvQualifierSeq& walker)
	{
		qualifiers = walker.qualifiers;
	}
};


struct SemaDeclarator : public SemaBase
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	UniqueTypeWrapper qualifying;
	ScopePtr paramScope;
	Dependent valueDependent;
	TypeSequence typeSequence;
	CvQualifiers qualifiers;
	Qualifying memberPointer;
	Dependent dependent; // track which template parameters the declarator's type depends on. e.g. 'T::* memberPointer', 'void f(T)'
	TypeId conversionType; // the return-type, if this is a conversion-function declarator
	SemaDeclarator(const SemaState& state)
		: SemaBase(state), id(&gAnonymousId), paramScope(0), typeSequence(context), memberPointer(context), conversionType(0, context)
	{
	}
	void pushPointerType(cpp::ptr_operator* op)
	{
		if(op->key->id == cpp::ptr_operator_key::REF)
		{
			typeSequence.push_front(DeclaratorReferenceType());
		}
		else
		{
			if(memberPointer.empty())
			{
				typeSequence.push_front(DeclaratorPointerType(qualifiers));
			}
			else
			{
				typeSequence.push_front(DeclaratorMemberPointerType(memberPointer.back(), qualifiers));
				setDependent(dependent, memberPointer);
			}
		}
	}

	SEMA_POLICY(cpp::ptr_operator, SemaPolicyPush<struct SemaPtrOperator>)
	void action(cpp::ptr_operator* symbol, const SemaPtrOperator& walker)
	{
		qualifiers = walker.qualifiers;
		memberPointer = walker.qualifying;
	}
	template<typename T>
	void walkDeclaratorPtr(T* symbol, SemaDeclarator& walker)
	{
		// if parse fails, state of typeSeqence is not modified.
		walker.pushPointerType(symbol->op);
		id = walker.id;
		qualifying = walker.qualifying;
		enclosing = walker.enclosing;
		paramScope = walker.paramScope;
		templateParams = walker.templateParams;
		addDependent(dependent, walker.dependent);
		addDependent(valueDependent, walker.valueDependent);
		addDependent(enclosingDependent, walker.enclosingDependent);
		SYMBOLS_ASSERT(typeSequence.empty());
		typeSequence = walker.typeSequence;

		qualifiers = walker.qualifiers;
		memberPointer = walker.memberPointer;
		conversionType = walker.conversionType;
	}
	SEMA_POLICY(cpp::declarator_ptr, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::declarator_ptr* symbol, SemaDeclarator& walker)
	{
		return walkDeclaratorPtr(symbol, walker);
	}
	SEMA_POLICY(cpp::abstract_declarator_ptr, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::abstract_declarator_ptr* symbol, SemaDeclarator& walker)
	{
		return walkDeclaratorPtr(symbol, walker);
	}
	SEMA_POLICY(cpp::new_declarator_ptr, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::new_declarator_ptr* symbol, SemaDeclarator& walker)
	{
		return walkDeclaratorPtr(symbol, walker);
	}
	SEMA_POLICY(cpp::conversion_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::conversion_declarator* symbol, SemaDeclarator& walker)
	{
		return walkDeclaratorPtr(symbol, walker);
	}
	SEMA_POLICY(cpp::declarator_id, SemaPolicyPush<struct SemaDeclaratorId>)
	void action(cpp::declarator_id* symbol, SemaDeclaratorId& walker)
	{
		id = walker.id;
		qualifying = walker.qualifying.empty() || isNamespace(*walker.qualifying.back().declaration)
			? gUniqueTypeNull : UniqueTypeWrapper(walker.qualifying.back().unique);

		if(walker.getQualifyingScope()
			&& enclosing->type != SCOPETYPE_CLASS) // //TODO: in 'class C { friend void Q::N(X); };' X should be looked up in the scope of Q rather than C (if Q is a class)
		{
			enclosing = walker.getQualifyingScope(); // names in declarator suffix (array-size, parameter-declaration) are looked up in declarator-id's qualifying scope
		}

		if(qualifying != gUniqueTypeNull)// if the declarator is qualified by a class-name
		{
			// represents the type of 'this'
			enclosingType = &getSimpleType(qualifying.value);
			if(enclosingType->declaration->isTemplate // if the declarator is qualified with a template-id
				&& !enclosingType->declaration->templateParams.empty()) // and the template is not an explicit-specialization
			{
				// 'this' is dependent within a template-definition (except for an explicit-specialization)
				// NOTE: depends on state of 'enclosing', modified above!
				setDependent(enclosingDependent, enclosingType->declaration->templateParams.back().declaration);
			}
		}


		if(templateParams != 0
			&& !templateParams->empty()
			&& consumeTemplateParams(walker.qualifying))
		{
			templateParams = 0;
		}

		if(walker.conversionType.declaration != 0)
		{
			conversionType = walker.conversionType;
		}
		addDependent(dependent, conversionType); // TODO: check compliance: conversion-function declarator-id is dependent if it contains a dependent type?
	}
	template<typename T>
	void walkDeclaratorArray(T* symbol, const SemaDeclaratorArray& walker)
	{
		// [temp.dep.type] A type is dependent if it is - an array type [...] whose size is specified by a constant expression that is value-dependent
		addDependent(dependent, walker.valueDependent);
		addDependent(valueDependent, walker.valueDependent);
		typeSequence.push_front(DeclaratorArrayType(walker.rank));
	}
	SEMA_POLICY(cpp::declarator_suffix_array, SemaPolicyPushCached<struct SemaDeclaratorArray>)
	void action(cpp::declarator_suffix_array* symbol, const SemaDeclaratorArray& walker)
	{
		return walkDeclaratorArray(symbol, walker);
	}
	SEMA_POLICY(cpp::declarator_suffix_function, SemaPolicyPushCached<struct SemaDeclaratorFunction>)
	void action(cpp::declarator_suffix_function* symbol, const SemaDeclaratorFunction& walker)
	{
		if(paramScope == 0) // only interested in the innermost parameter-list
		{
			paramScope = walker.paramScope;
		}
		typeSequence.push_front(DeclaratorFunctionType(walker.parameters, walker.qualifiers));
		setDependent(dependent, walker.parameters);
	}
	SEMA_POLICY(cpp::new_declarator_suffix, SemaPolicyPushCached<struct SemaDeclaratorArray>)
	void action(cpp::new_declarator_suffix* symbol, const SemaDeclaratorArray& walker)
	{
		return walkDeclaratorArray(symbol, walker);
	}
	SEMA_POLICY(cpp::expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression* symbol, const SemaExpressionResult& walker) // in direct_new_declarator
	{
	}
	template<typename T>
	void walkDeclarator(T* symbol, const SemaDeclarator& walker)
	{
		id = walker.id;
		qualifying = walker.qualifying;
		enclosing = walker.enclosing;
		paramScope = walker.paramScope;
		templateParams = walker.templateParams;
		addDependent(dependent, walker.dependent);
		addDependent(valueDependent, walker.valueDependent);
		addDependent(enclosingDependent, walker.enclosingDependent);
		SYMBOLS_ASSERT(typeSequence.empty());
		typeSequence = walker.typeSequence;
		conversionType = walker.conversionType;
	}
	SEMA_POLICY(cpp::direct_abstract_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::direct_abstract_declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker); // if parse fails, state of typeSeqence is not modified. e.g. type-id: int((int))
	}
	SEMA_POLICY(cpp::direct_abstract_declarator_parenthesis, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::direct_abstract_declarator_parenthesis* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker); // if parse fails, state of typeSeqence is not modified. e.g. function-style-cast type-id followed by parenthesised expression: int(*this)
	}
	SEMA_POLICY(cpp::direct_new_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::direct_new_declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker);
	}
	SEMA_POLICY(cpp::declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker);
	}
	SEMA_POLICY(cpp::abstract_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::abstract_declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker);
	}
	SEMA_POLICY(cpp::new_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::new_declarator* symbol, const SemaDeclarator& walker)
	{
		return walkDeclarator(symbol, walker);
	}
};

#endif
