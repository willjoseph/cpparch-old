
#ifndef INCLUDED_CPPPARSE_SEMA_SEMACLASS_H
#define INCLUDED_CPPPARSE_SEMA_SEMACLASS_H

#include "SemaCommon.h"
#include "SemaTypeName.h"
#include "SemaNestedNameSpecifier.h"

struct SemaBaseSpecifier : public SemaQualified
{
	SEMA_BOILERPLATE;

	Type type;
	SemaBaseSpecifier(const SemaState& state)
		: SemaQualified(state), type(0, context)
	{
	}

	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY_ARGS(cpp::class_name, SemaPolicyPushBool<struct SemaTypeName>, true)
	void action(cpp::class_name* symbol, const SemaTypeName& walker)
	{
		/* [class.derived]
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		type = walker.type;
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		makeUniqueTypeSafe(type);
	}
};

struct SemaClassHead : public SemaBase
{
	SEMA_BOILERPLATE;

	DeclarationPtr declaration;
	IdentifierPtr id;
	ScopePtr parent;
	TemplateArguments arguments;
	bool isUnion;
	bool isSpecialization;
	SemaClassHead(const SemaState& state)
		: SemaBase(state), declaration(0), id(0), parent(enclosing), arguments(context), isUnion(false), isSpecialization(false)
	{
	}

	SEMA_POLICY(cpp::class_key, SemaPolicyIdentityCached)
	void action(cpp::class_key* symbol)
	{
		isUnion = symbol->id == cpp::class_key::UNION;
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol) // class_name
	{
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaNestedNameSpecifier& walker)
	{
		// resolve the (possibly dependent) qualifying scope
		if(walker.getDeclaratorQualifying() != 0)
		{
			parent = walker.getDeclaratorQualifying()->enclosed; // class is declared in scope of qualifying class/namespace
		}

		if(templateParams != 0
			&& !templateParams->empty()
			&& consumeTemplateParams(walker.qualifying))
		{
			templateParams = 0;
		}
	}
	SEMA_POLICY_ARGS(cpp::simple_template_id, SemaPolicyPushCachedBool<struct SemaTemplateId>, true) // TODO: specifying isTemplate as a temporary workaround: name lookup of qualified class-name currently fails.
	void action(cpp::simple_template_id* symbol, const SemaTemplateIdResult& walker) // class_name
	{
		// TODO: don't declare anything - this is a template (partial) specialization
		id = walker.id;
		arguments = walker.arguments;
		isSpecialization = true;
	}
	void action(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until ':' is discovered
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = declareClass(parent, id, isSpecialization, arguments);
	}
	SEMA_POLICY(cpp::base_specifier, SemaPolicyPush<struct SemaBaseSpecifier>)
	void action(cpp::base_specifier* symbol, const SemaBaseSpecifier& walker) 
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		SEMANTIC_ASSERT(declaration->enclosed != 0);
		SEMANTIC_ASSERT(walker.type.unique != 0);
		addBase(declaration, walker.type);
		setExpressionType(symbol, walker.type.isDependent ? gUniqueTypeNull : UniqueTypeWrapper(walker.type.unique));
	}
};



struct SemaMemberDeclaration : public SemaBase, SemaMemberDeclarationResult
{
	SEMA_BOILERPLATE;

	SemaMemberDeclaration(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::member_template_declaration, SemaPolicyPush<struct SemaTemplateDeclaration>)
	void action(cpp::member_template_declaration* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::member_declaration_implicit, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::member_declaration_implicit* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::member_declaration_default, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::member_declaration_default* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::member_declaration_nested, SemaPolicyPush<struct SemaQualifiedId>)
	void action(cpp::member_declaration_nested* symbol, const SemaQualifiedId& walker)
	{
	}
	SEMA_POLICY(cpp::using_declaration, SemaPolicyPush<struct SemaUsingDeclaration>)
	void action(cpp::using_declaration* symbol, const SemaUsingDeclaration& walker)
	{
	}
};



struct SemaClassSpecifier : public SemaBase, SemaClassSpecifierResult
{
	SEMA_BOILERPLATE;

	DeferredSymbols deferred;
	SemaClassSpecifier(const SemaState& state)
		: SemaBase(state), SemaClassSpecifierResult(context)
	{
	}

	SEMA_POLICY(cpp::class_head, SemaPolicyPush<struct SemaClassHead>)
	void action(cpp::class_head* symbol, const SemaClassHead& walker)
	{
		declaration = walker.declaration;
		id = walker.id;
		isUnion = walker.isUnion;
		isSpecialization = walker.isSpecialization;
		arguments = walker.arguments;
		enclosing = walker.parent;
		templateParams = walker.templateParams; // template-params may have been consumed by qualifying template-name
	}
	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until '{' is discovered
		if(declaration == 0)
		{
			// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
			declaration = declareClass(enclosing, id, isSpecialization, arguments);
		}

		/* basic.scope.class-1
		The potential scope of a name declared in a class consists not only of the declarative region following
		the name’s point of declaration, but also of all function bodies, brace-or-equal-initializers of non-static
		data members, and default arguments in that class (including such things in nested classes).
		*/
		SEMANTIC_ASSERT(declaration->enclosed != 0);

		pushScope(declaration->enclosed);
		if(templateParamScope != 0)
		{
			// insert the template-parameter scope to enclose the class scope
			templateParamScope->parent = enclosing->parent;
			enclosing->parent = templateParamScope; // required when looking up template-parameters from within a template class
		}
		if(declaration->isTemplate)
		{
			enclosing->templateDepth = templateDepth; // indicates that this is a template
		}
		declaration->templateParamScope = templateParamScope; // required by findEnclosingType

		Type type(declaration, context);
		type.id = &declaration->getName();
		setDependent(type);
		if(declaration->isTemplate)
		{
			setDependentEnclosingTemplate(type.dependent, declaration);
		}
		type.isDependent = isDependent(type);
		type.isImplicitTemplateId = declaration->isTemplate;
		type.isEnclosingClass = true;
		bool isExplicitSpecialization = isSpecialization && declaration->templateParams.empty();
		bool allowDependent = type.isDependent || (declaration->isTemplate && !isExplicitSpecialization); // prevent uniquing of template-arguments in implicit template-id
		declaration->type.isDependent = type.isDependent;
		declaration->type.unique = makeUniqueType(type, getInstantiationContext(), allowDependent).value;
		enclosingType = &getSimpleType(declaration->type.unique);
		const_cast<SimpleType*>(enclosingType)->declaration = declaration; // if this is a specialization, use the specialization instead of the primary template
		instantiateClass(*enclosingType, InstantiationContext(getLocation(), 0, 0), allowDependent); // instantiate non-dependent base classes

		addDependent(enclosingDependent, type);

		clearTemplateParams();

		if(SemaState::enclosingDeferred == 0)
		{
			SemaState::enclosingDeferred = &deferred;
		}
	}
	SEMA_POLICY(cpp::member_declaration, SemaPolicyPush<struct SemaMemberDeclaration>)
	void action(cpp::member_declaration* symbol, const SemaMemberDeclaration& walker)
	{
	}
#if 0 // TODO!
	inline bool hasCopyAssignmentOperator(const Declaration& declaration)
	{
		Identifier id;
		id.value = gOperatorAssignId;
		const DeclarationInstance* result = ::findDeclaration(declaration.enclosed->declarations, id);
		if(result == 0)
		{
			return false;
		}
		for(const Declaration* p = findOverloaded(*result); p != 0; p = p->overloaded)
		{
			if(p->isTemplate)
			{
				continue; // TODO: check compliance: copy-assignment-operator cannot be a template?
			}

		}
		return false;
	}
#endif
	void action(cpp::terminal<boost::wave::T_RIGHTBRACE> symbol)
	{
		declaration->isComplete = true;
		// If the class definition does not explicitly declare a copy assignment operator, one is declared implicitly.
		// The implicitly-declared copy assignment operator for a class X will have the form
		//   X& X::operator=(const X&)
		// TODO: correct constness of parameter
		// TODO: this must occur at point of instantiation

		parseDeferred(deferred.first, context.parserContext);
		parseDeferred(deferred.second, context.parserContext);
	}
};


struct SemaQualifiedTypeName : public SemaQualified
{
	SEMA_BOILERPLATE;

	SemaQualifiedTypeName(const SemaState& state)
		: SemaQualified(state)
	{
	}
	void action(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	SEMA_POLICY(cpp::nested_name_specifier, SemaPolicyPushCached<struct SemaNestedNameSpecifier>)
	void action(cpp::nested_name_specifier* symbol, const SemaQualifyingResult& walker)
	{
		swapQualifying(walker.qualifying);
	}
	SEMA_POLICY(cpp::class_name, SemaPolicyPushChecked<struct SemaTypeName>)
	bool action(cpp::class_name* symbol, const SemaTypeName& walker)
	{
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		return true;
	}
};

struct SemaMemInitializer : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaMemInitializer(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::mem_initializer_id_base, SemaPolicyPush<struct SemaQualifiedTypeName>)
	void action(cpp::mem_initializer_id_base* symbol, const SemaQualifiedTypeName& walker)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityChecked)
	bool action(cpp::identifier* symbol)
	{
		SEMANTIC_ASSERT(getQualifyingScope() == 0);
		LookupResultRef declaration = findDeclaration(symbol->value);
		if(declaration == &gUndeclared
			|| !isObject(*declaration))
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "object-name");
		}
		setDecoration(&symbol->value, declaration);
		return true;
	}
	SEMA_POLICY(cpp::expression_list, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression_list* symbol, const SemaExpressionResult& walker)
	{
	}
};

struct SemaMemberDeclaratorBitfield : public SemaBase
{
	SEMA_BOILERPLATE;

	IdentifierPtr id;
	SemaMemberDeclaratorBitfield(const SemaState& state)
		: SemaBase(state), id(0)
	{
	}
	SEMA_POLICY(cpp::identifier, SemaPolicyIdentityCached)
	void action(cpp::identifier* symbol)
	{
		id = &symbol->value;
	}
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker)
	{
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
	}
};

struct SemaMemInitializerClause : public SemaBase
{
	SEMA_BOILERPLATE;
	SemaMemInitializerClause(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::mem_initializer, SemaPolicyPush<struct SemaMemInitializer>)
	void action(cpp::mem_initializer* symbol, const SemaMemInitializer& walker)
	{
	}
};

#endif
