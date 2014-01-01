
#ifndef INCLUDED_CPPPARSE_SEMA_SEMAPOSTFIXEXPRESSION_H
#define INCLUDED_CPPPARSE_SEMA_SEMAPOSTFIXEXPRESSION_H

#include "SemaCommon.h"
#include "SemaPrimaryExpression.h"
#include "Core/TypeTraits.h"

struct SemaArgumentList : public SemaBase
{
	SEMA_BOILERPLATE;

	Arguments arguments;
	Dependent typeDependent;
	Dependent valueDependent;
	SemaArgumentList(const SemaState& state)
		: SemaBase(state)
	{
		clearQualifying();
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		arguments.push_back(makeArgument(walker.expression, walker.type));
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct SemaSubscript : public SemaBase
{
	SEMA_BOILERPLATE;

	ExpressionWrapper expression;
	UniqueTypeWrapper type;
	Dependent typeDependent;
	Dependent valueDependent;
	SemaSubscript(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_LEFTBRACKET>)
	{
		clearQualifying(); // the expression in [] is looked up in the context of the entire postfix expression
	}
	SEMA_POLICY(cpp::expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression* symbol, const SemaExpressionResult& walker)
	{
		expression = walker.expression;
		type = walker.type;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
};


struct SemaPostfixExpressionMember : public SemaQualified
{
	SEMA_BOILERPLATE;

	ExpressionWrapper expression;
	LookupResultRef declaration;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	TemplateArguments arguments; // only used if the identifier is a template-name
	Dependent typeDependent;
	Dependent valueDependent;
	bool isTemplate;
	SemaPostfixExpressionMember(const SemaState& state)
		: SemaQualified(state), id(0), arguments(context), isTemplate(false)
	{
	}
	SEMA_POLICY(cpp::member_operator, SemaPolicyIdentity)
	void action(cpp::member_operator* symbol)
	{
		bool isArrow = symbol->id == cpp::member_operator::ARROW;

		memberClass = &gDependentSimpleType;
		SEMANTIC_ASSERT(objectExpression.p != 0);
		if(!objectExpression.isTypeDependent) // if the type of the object expression is not dependent
		{
			// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
			UniqueTypeWrapper operand = removeReference(memberType);
			memberClass = &getMemberOperatorType(makeArgument(expression, operand), isArrow, getInstantiationContext());

			objectExpression = makeExpression(ObjectExpression(memberClass));
		}
#if 0
		else
		{
			objectExpression = makeExpression(DependentObjectExpression(objectExpression, isArrow));
		}
#endif
	}
	void action(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	SEMA_POLICY_ARGS(cpp::id_expression, SemaPolicyPushBool<struct SemaIdExpression>, isTemplate)
	void action(cpp::id_expression* symbol, SemaIdExpression& walker)
	{
		bool isObjectName = walker.commit();
		SEMANTIC_ASSERT(isObjectName); // TODO: non-fatal error: expected object name
		id = walker.id;
		arguments = walker.arguments;
		declaration = walker.declaration;
		swapQualifying(walker.qualifying);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = walker.expression;
	}
};

struct SemaTypeTraitsIntrinsic : public SemaBase
{
	SEMA_BOILERPLATE;

	Dependent valueDependent;
	UniqueTypeWrapper first;
	UniqueTypeWrapper second;
	SemaTypeTraitsIntrinsic(const SemaState& state)
		: SemaBase(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		// debugging
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		addDependent(valueDependent, walker.type);

		UniqueTypeWrapper type = UniqueTypeWrapper(walker.type.unique);
		setExpressionType(symbol, type);

		(first == gUniqueTypeNull ? first : second) = type;
	}
};

struct SemaPostfixExpression : public SemaBase
{
	SEMA_BOILERPLATE;

	UniqueTypeId type;
	ExpressionWrapper expression;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	TemplateArguments arguments; // only valid when the expression is a (qualified) template-id
	const SimpleType* idEnclosing; // may be valid when the above id-expression is a qualified-id
	Dependent typeDependent;
	Dependent valueDependent;
	bool isUndeclared;
	SemaPostfixExpression(const SemaState& state)
		: SemaBase(state), id(0), arguments(context), idEnclosing(0), isUndeclared(false)
	{
	}
	void clearMemberType()
	{
		memberType = gUniqueTypeNull;
		objectExpression = ExpressionWrapper();
	}
	void updateMemberType()
	{
		memberClass = 0;
		if(type.isFunction())
		{
			memberType = gUniqueTypeNull;
			objectExpression = ExpressionWrapper();
		}
		else
		{
			memberType = type;
			objectExpression = makeExpression(ExplicitTypeExpression(type),
				false, isDependent(typeDependent), isDependent(valueDependent)
				);
			SEMANTIC_ASSERT(objectExpression.isTypeDependent || !::isDependent(type));
		}
	}
	SEMA_POLICY(cpp::primary_expression, SemaPolicyPush<struct SemaPrimaryExpression>)
	void action(cpp::primary_expression* symbol, const SemaPrimaryExpression& walker)
	{
		type = walker.type;
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		id = walker.id;
		arguments = walker.arguments;
		idEnclosing = walker.idEnclosing;
		isUndeclared = walker.isUndeclared;
		setExpressionType(symbol, type);
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_construct, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::postfix_expression_construct* symbol, const SemaExplicitTypeExpressionResult& walker)
	{
		addDependent(typeDependent, walker.typeDependent);
		type = getUniqueTypeSafe(walker.type);
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, getInstantiationContext());
		expression = makeExpression(CastExpression(type, walker.expression), walker.expression.isConstant, isDependent(typeDependent), false);
		expression.isTemplateArgumentAmbiguity = symbol->args == 0;
		if(!expression.isTypeDependent)
		{
			SYMBOLS_ASSERT(expression.type == type);
		}
		setExpressionType(symbol, type);
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_cast, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::postfix_expression_cast* symbol, const SemaExplicitTypeExpressionResult& walker)
	{
		type = getUniqueTypeSafe(walker.type);
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, getInstantiationContext());
		if(symbol->op->id != cpp::cast_operator::DYNAMIC)
		{
			Dependent tmp(walker.typeDependent);
			addDependent(valueDependent, tmp);
		}
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = makeExpression(CastExpression(type, walker.expression), walker.expression.isConstant, isDependent(typeDependent), isDependent(valueDependent)); // TODO: can this be value-dependent?
		if(!expression.isTypeDependent)
		{
			SYMBOLS_ASSERT(expression.type == type);
		}
		setExpressionType(symbol, type);
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_typeid, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::postfix_expression_typeid* symbol, const SemaExpressionResult& walker)
	{
		// TODO: operand type required to be complete?
		type = getTypeInfoType();
		expression = makeExpression(ExplicitTypeExpression(type));
		if(!expression.isTypeDependent)
		{
			SYMBOLS_ASSERT(expression.type == type);
		}
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_typeidtype, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::postfix_expression_typeidtype* symbol, const SemaTypeIdResult& walker)
	{
		// TODO: operand type required to be complete?
		type = getTypeInfoType();
		expression = makeExpression(ExplicitTypeExpression(type));
		if(!expression.isTypeDependent)
		{
			SYMBOLS_ASSERT(expression.type == type);
		}
		updateMemberType();
	}

	// suffix
	SEMA_POLICY(cpp::postfix_expression_subscript, SemaPolicyPushSrc<struct SemaSubscript>)
	void action(cpp::postfix_expression_subscript* symbol, const SemaSubscript& walker)
	{
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		id = 0; // don't perform overload resolution for a[i](x);
		if(isDependent(typeDependent))
		{
			type = gUniqueTypeNull;
		}
		else
		{
			SEMANTIC_ASSERT(type != gUniqueTypeNull);
			// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
			type = removeReference(type);
			type = typeOfSubscriptExpression(
				makeArgument(expression, type),
				makeArgument(walker.expression, walker.type),
				getInstantiationContext());
		}
		expression = makeExpression(SubscriptExpression(expression, walker.expression), false, isDependent(typeDependent), isDependent(valueDependent));
		expression.isLvalue = true;
		if(!expression.isTypeDependent)
		{
			SYMBOLS_ASSERT(expression.type == type);
		}
		setExpressionType(symbol, type);
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_call, SemaPolicyPushSrc<struct SemaArgumentList>)
	void action(cpp::postfix_expression_call* symbol, const SemaArgumentList& walker)
	{
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);

		if(!isDependent(typeDependent)
			&& expression.isNonStaticMemberName // if the id-expression names a nonstatic member
			&& memberClass == 0) // and this is not a class-member-access expression
		{
			SEMANTIC_ASSERT(enclosingType != 0); // TODO: check that the id-expression is found in the context of a non-static member (i.e. 'this' is valid)
			// [class.mfct.nonstatic] An id-expression (that is not part of a class-member-access expression, and is found in the context of a nonstatic member)
			// that names a nonstatic member is transformed to a class-member-access expression prefixed by (*this)

			addDependent(typeDependent, enclosingDependent);
			// when a nonstatic member name is used in a function call, overload resolution is dependent on the type of the implicit object parameter
		}


		if(isDependent(typeDependent)) // if either the argument list or the id-expression are dependent
			// TODO: check valueDependent too?
		{
			if(id != 0)
			{
				id->dec.deferred = true;
			}
			type = gUniqueTypeNull;
			expression = ExpressionWrapper();
		}
		else
		{
			{ // consistency checking
				SEMANTIC_ASSERT(isUndeclared == isDependentIdExpression(expression));
				SEMANTIC_ASSERT(!isUndeclared || getDependentIdExpression(expression).name == id->value);

				TemplateArgumentsInstance templateArguments;
				makeUniqueTemplateArguments(arguments, templateArguments, getInstantiationContext());

				if(isUndeclared)
				{
					SEMANTIC_ASSERT(templateArguments.empty());
				}
				else
				{
					bool isCallToNamedFunction = expression.p != 0
						&& (isIdExpression(expression)
						|| isClassMemberAccessExpression(expression));
					SEMANTIC_ASSERT((id != 0) == (isCallToNamedFunction || (expression.p != 0 && isNonTypeTemplateParameter(expression))));
					if(!isCallToNamedFunction)
					{
						SEMANTIC_ASSERT(templateArguments.empty());
					}
					else
					{
						bool isCallToNamedMemberFunction = isClassMemberAccessExpression(expression);
						const IdExpression& idExpression = getIdExpression(
							isCallToNamedMemberFunction ? getClassMemberAccessExpression(expression).right : expression);
						SEMANTIC_ASSERT(id == &gAnonymousId // e.g. ~decltype(x)
							|| idExpression.declaration.p == &getDeclaration(*id));
						SEMANTIC_ASSERT(idExpression.templateArguments == templateArguments);

						if(type.isFunction())
						{
							const SimpleType* tmp = isCallToNamedMemberFunction ? getObjectExpression(getClassMemberAccessExpression(expression).left).classType : 0;
							SEMANTIC_ASSERT(memberClass == tmp);

							if(!isSpecialMember(*idExpression.declaration))
							{
								const SimpleType* tmp = getIdExpressionClass(idExpression.enclosing, idExpression.declaration, memberClass != 0 ? memberClass : enclosingType);
								SEMANTIC_ASSERT(idEnclosing == tmp);
							}
						}
					}
				}
			}
			type = typeOfFunctionCallExpression(makeArgument(expression, type), walker.arguments, getInstantiationContext());
			expression = makeExpression(FunctionCallExpression(expression, walker.arguments), false, isDependent(typeDependent), isDependent(valueDependent));
			SYMBOLS_ASSERT(expression.type == type);
		}
		setExpressionType(symbol, type);
		// TODO: set of pointers-to-function
		id = 0; // don't perform overload resolution for a(x)(x);
		idEnclosing = 0;
		updateMemberType();
		isUndeclared = false; // for an expression of the form 'undeclared-id(args)'
	}

	SEMA_POLICY(cpp::postfix_expression_member, SemaPolicyPushSrc<struct SemaPostfixExpressionMember>)
	void action(cpp::postfix_expression_member* symbol, const SemaPostfixExpressionMember& walker)
	{
		id = walker.id; // perform overload resolution for a.m(x);
		arguments = walker.arguments;
		type = gUniqueTypeNull;
		LookupResultRef declaration = walker.declaration;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);

		expression = ExpressionWrapper();

		if(walker.expression.p != 0
			&& !isDependent(typeDependent))
		{
			SEMANTIC_ASSERT(objectExpression.p != 0);
			SEMANTIC_ASSERT(!objectExpression.isTypeDependent); // the object-expression should not be dependent
			SEMANTIC_ASSERT(walker.memberClass != 0);
			SEMANTIC_ASSERT(walker.memberClass != &gDependentSimpleType);

			// TODO: [expr.ref] inherit const/volatile from object-expression type if member is non-static
			UniqueTypeWrapper qualifyingType = makeUniqueQualifying(walker.qualifying, getInstantiationContext());
			const SimpleType* qualifyingClass = qualifyingType == gUniqueTypeNull ? 0 : &getSimpleType(qualifyingType.value);
			type = typeOfIdExpression(qualifyingClass, declaration, setEnclosingTypeSafe(getInstantiationContext(), walker.memberClass));
			idEnclosing = isSpecialMember(*declaration) ? 0 : getIdExpressionClass(qualifyingClass, declaration, walker.memberClass);

			expression = makeExpression(ClassMemberAccessExpression(walker.objectExpression, walker.expression),
				false, isDependent(typeDependent), isDependent(valueDependent)
				);
			if(isOverloadedFunction(declaration))
			{
				type = gOverloaded;
			}
			SYMBOLS_ASSERT(expression.type == type);
			expression.isLvalue = isLvalue(*declaration);
		}

		setExpressionType(symbol, type);
		updateMemberType();

		if(type.isFunction())
		{
			// type determination is deferred until overload resolution is complete
			memberClass = walker.memberClass; // store the type of the implied object argument in a qualified function call.
		}
	}
	SEMA_POLICY(cpp::postfix_expression_destructor, SemaPolicySrc)
	void action(cpp::postfix_expression_destructor* symbol)
	{
		type = gVoid; // TODO: should this be null-type?
		id = 0;
		expression = ExpressionWrapper();
		setExpressionType(symbol, type);
		// TODO: name-lookup for destructor name
		clearMemberType();
	}
	SEMA_POLICY(cpp::postfix_operator, SemaPolicySrc)
	void action(cpp::postfix_operator* symbol)
	{
		if(isDependent(typeDependent))
		{
			type = gUniqueTypeNull;
			expression = ExpressionWrapper();
		}
		else
		{
			type = removeReference(type);
			type = typeOfPostfixOperatorExpression(
				getOverloadedOperatorId(symbol),
				makeArgument(expression, type),
				getInstantiationContext());

			expression = makeExpression(PostfixOperatorExpression(getOverloadedOperatorId(symbol), expression));
			SYMBOLS_ASSERT(expression.type == type);
		}
		setExpressionType(symbol, type);
		id = 0;
		updateMemberType();
	}
	SEMA_POLICY(cpp::postfix_expression_typetraits_unary, SemaPolicyPush<struct SemaTypeTraitsIntrinsic>)
	void action(cpp::postfix_expression_typetraits_unary* symbol, const SemaTypeTraitsIntrinsic& walker)
	{
		addDependent(valueDependent, walker.valueDependent);
		type = gBool;
		UnaryTypeTraitsOp operation = getUnaryTypeTraitsOp(symbol->trait);
		Name name = getTypeTraitName(symbol);
		expression = makeExpression(TypeTraitsUnaryExpression(name, operation, walker.first), true, false, isDependent(valueDependent));
		SYMBOLS_ASSERT(expression.type == type);
	}
	SEMA_POLICY(cpp::postfix_expression_typetraits_binary, SemaPolicyPush<struct SemaTypeTraitsIntrinsic>)
	void action(cpp::postfix_expression_typetraits_binary* symbol, const SemaTypeTraitsIntrinsic& walker)
	{
		addDependent(valueDependent, walker.valueDependent);
		type = gBool;
		BinaryTypeTraitsOp operation = getBinaryTypeTraitsOp(symbol->trait);
		Name name = getTypeTraitName(symbol);
		expression = makeExpression(TypeTraitsBinaryExpression(name, operation, walker.first, walker.second), true, false, isDependent(valueDependent));
		SYMBOLS_ASSERT(expression.type == type);
	}
};

#endif
