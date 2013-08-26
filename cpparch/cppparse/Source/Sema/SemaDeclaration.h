
#ifndef INCLUDED_CPPPARSE_SEMA_SEMADECLARATION_H
#define INCLUDED_CPPPARSE_SEMA_SEMADECLARATION_H

#include "SemaCommon.h"
#include "SemaSpecifier.h"
#include "SemaDeclarator.h"
#include "SemaStatement.h"

struct IsTemplateName
{
	SemaState& context;
	IsTemplateName(SemaState& context) : context(context)
	{
	}
	bool operator()(Identifier& id) const
	{
		LookupResultRef declaration = context.findDeclaration(id);
		return declaration != &gUndeclared && isTemplateName(*declaration);
	}
};

struct SemaInitializer : public SemaBase
{
	SEMA_BOILERPLATE;

	ExpressionWrapper expression;
	Dependent valueDependent;
	SemaInitializer(const SemaState& state) : SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct SemaDeclarationSuffix : public SemaBase
{
	SEMA_BOILERPLATE;

	SemaDeclarationArgs args;

	// prefix
	const SemaDeclSpecifierSeqResult& seq;

	// declarator
	TypeId type;
	DeclarationPtr declaration; // the result of the declaration
	IdentifierPtr id;
	ScopePtr parent;
	ScopePtr enclosed;
	Dependent typeDependent;
	Dependent valueDependent;
	cpp::default_argument* defaultArgument; // parsing of this symbol will be deferred if this is a member-declaration
	bool isConversionFunction;
	CvQualifiers conversionFunctionQualifiers;


	SemaDeclarationSuffix(const SemaState& state, SemaDeclarationArgs args, const SemaDeclSpecifierSeqResult& seq) : SemaBase(state),
		args(args),
		seq(seq),
		type(0, context),
		declaration(0),
		id(&gAnonymousId),
		parent(state.enclosing),
		defaultArgument(0),
		enclosed(0),
		isConversionFunction(false)
	{
		*static_cast<Type*>(&type) = seq.type;
		type.qualifiers = seq.qualifiers;
		declareEts(type, seq.forward);
	}

	// commit the declaration to the enclosing scope.
	// invoked when no further ambiguities remain.
	void commit()
	{
		if(id != 0)
		{
			if(id == &gAnonymousId // if the declaration is anonymous
				&& !(args.isParameter || args.templateParameter != INDEX_INVALID)) // and this is not the declaration of a function parameter or non-type template parameter
			{
				id = 0; // do not declare anything
				return;
			}
			if(isConversionFunction)
			{
				type.typeSequence.push_back(DeclaratorFunctionType(Parameters(), conversionFunctionQualifiers));
			}
			DeclarationPtr tmpDependent = type.dependent;
			setDependent(type.dependent, typeDependent);
			makeUniqueTypeSafe(type);
			if(enclosed == 0
				&& templateParamScope != 0)
			{
				templateParamScope->parent = parent;
				enclosed = templateParamScope; // for a static-member-variable definition, store template-params with different names than those in the class definition
			}
			declaration = declareObject(parent, id, type, enclosed, seq.specifiers, args.templateParameter, valueDependent);

			enclosing = parent;

			if(enclosed != 0) // if the declaration has a parameter-declaration-clause
			{
				enclosed->name = declaration->getName();
				enclosing = enclosed; // subsequent declarations are contained by the parameter-scope - see 3.3.2-1: parameter scope
			}
			clearTemplateParams();

			id = 0;

			// clear state that was modified while committing 
			type.unique = 0;
			type.isDependent = false;
			type.dependent = tmpDependent;
		}
	}

	SEMA_POLICY(cpp::declarator, SemaPolicyPushCached<struct SemaDeclarator>)
	void action(cpp::declarator* symbol, const SemaDeclarator& walker)
	{
		parent = walker.enclosing; // if the id-expression in the declarator is a qualified-id, this is the qualifying scope
		id = walker.id;
		enclosed = walker.paramScope;
		type.typeSequence = walker.typeSequence;
		addDependent(typeDependent, walker.dependent);
		/* temp.dep.constexpr
		An identifier is value-dependent if it is:
			- a name declared with a dependent type,
			- the name of a non-type template parameter,
			- a constant with effective literal type and is initialized with an expression that is value-dependent.
		*/
		addDependent(valueDependent, walker.valueDependent);

		if(walker.qualifying != gUniqueTypeNull)
		{
			SEMANTIC_ASSERT(walker.qualifying.isSimple());
			enclosingType = &getSimpleType(walker.qualifying.value);
			enclosingDependent = walker.enclosingDependent; // not using addDependent, workaround for issue when 'enclosing' is not (yet) referring to qualifying class in declarator 'S<T>::f()' 
		}
		templateParams = walker.templateParams; // template-params may have been consumed by qualifying template-name

		if(walker.conversionType.declaration != 0)
		{
			// [class.conv.fct] The type of a conversion function is "function taking no parameter returning conversion-type-id."
			// take the cv-qualifiers from the declarator
			conversionFunctionQualifiers = getDeclaratorFunctionType(type.typeSequence.get()).qualifiers;
			type = walker.conversionType;
			isConversionFunction = true;
		}
	}
	SEMA_POLICY(cpp::abstract_declarator, SemaPolicyPush<struct SemaDeclarator>)
	void action(cpp::abstract_declarator* symbol, const SemaDeclarator& walker)
	{
		enclosed = walker.paramScope;
		type.typeSequence = walker.typeSequence;
		addDependent(typeDependent, walker.dependent);
	}
	SEMA_POLICY(cpp::member_declarator_bitfield, SemaPolicyPush<struct SemaMemberDeclaratorBitfield>)
	void action(cpp::member_declarator_bitfield* symbol, const SemaMemberDeclaratorBitfield& walker)
	{
		if(walker.id != 0)
		{
			DeclarationPtr tmpDependent = type.dependent;
			setDependent(type.dependent, typeDependent);
			makeUniqueTypeSafe(type);

			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *walker.id, type, 0, seq.specifiers); // 3.3.1.1
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(instance);
#endif
			setDecoration(walker.id, instance);
			declaration = instance;

			// clear state that was modified while committing 
			type.unique = 0;
			type.isDependent = false;
			type.dependent = tmpDependent;
		}
	}
	void action(cpp::terminal<boost::wave::T_ASSIGN> symbol) // begins initializer_default
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_LEFTPAREN> symbol) // begins initializer_parenthesis
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_TRY> symbol)
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_LEFTBRACE> symbol) // begins function_body
	{
		commit();
		SEMANTIC_ASSERT(declaration != 0);
		declaration->isFunctionDefinition = true;

		// NOTE: we must ensure that symbol-table modifications within the scope of this function are undone on parse fail
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	void action(cpp::terminal<boost::wave::T_COMMA> symbol) // init_declarator_list, member_declarator_list, member_declaration_bitfield
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_SEMICOLON> symbol) // simple_declaration_named, member_declaration_named, type_declaration_suffix, member_declaration_bitfield, member_declaration_nested
	{
		commit();
	}
	void action(cpp::terminal<boost::wave::T_COLON> symbol) // in member_declarator_bitfield, or ctor_initializer
	{
		commit();
	}

	struct DeferDefaultArgument
	{
		static bool isDeferredParse(SemaInitializer& walker)
		{
			return walker.enclosingDeferred != 0;
		}
		static SkipDefaultArgument<IsTemplateName> getSkipFunc(SemaInitializer& walker)
		{
			return makeSkipDefaultArgument(IsTemplateName(walker));
		}
		static DeferredSymbolsList& getDeferredSymbolsList(SemaInitializer& walker)
		{
			return walker.enclosingDeferred->first;
		}
	};
	typedef SemaPolicyPushDeferred<struct SemaInitializer, DeferDefaultArgument> SemaPolicyDefaultArgument;
	// in case of an inline member-function-definition, default-argument parse will be deferred - otherwise parsed as normal
	SEMA_POLICY(cpp::default_argument, SemaPolicyDefaultArgument)
	void action(cpp::default_argument* symbol, const SemaInitializer& walker) // parameter_declaration (looks like an initializer)
	{
		if(!args.isParameter // if this is not a parameter declaration (parameters cannot be constants)
			&& SemaState::enclosingDeferred == 0) // and parse was not defered
		{
			declaration->initializer = walker.expression;
			addDependent(declaration->valueDependent, walker.valueDependent);
		}
	}
	// handle assignment-expression(s) in initializer
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker) // condition_init
	{
		if(!args.isParameter) // parameters cannot be constants
		{
			declaration->initializer = walker.expression;
			addDependent(declaration->valueDependent, walker.valueDependent);
		}
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	SEMA_POLICY(cpp::initializer_clause, SemaPolicyPush<struct SemaInitializer>)
	void action(cpp::initializer_clause* symbol, const SemaInitializer& walker) // initializer_default
	{
		SEMANTIC_ASSERT(declaration != 0);
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	SEMA_POLICY(cpp::expression_list, SemaPolicyPush<struct SemaInitializer>)
	void action(cpp::expression_list* symbol, const SemaInitializer& walker) // initializer_parenthesis
	{
		SEMANTIC_ASSERT(declaration != 0);
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
	SEMA_POLICY(cpp::constant_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::constant_expression* symbol, const SemaExpressionResult& walker) // member_declarator_bitfield
	{
		SEMANTIC_ASSERT(declaration != 0);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}

	typedef void(*SkipFunc)(Parser&);
	template<typename SemaT, SkipFunc skipFunc>
	struct DeferBody
	{
		static bool isDeferredParse(SemaT& walker)
		{
			return walker.enclosingDeferred != 0;
		}
		static SkipFunc getSkipFunc(SemaT& walker)
		{
			return skipFunc;
		}
		static DeferredSymbolsList& getDeferredSymbolsList(SemaT& walker)
		{
			return walker.enclosingDeferred->second;
		}
	};
	typedef DeferBody<SemaMemInitializerClause, skipMemInitializerClause> DeferMemInitializerClause;
	typedef SemaPolicyPushDeferred<struct SemaMemInitializerClause, DeferMemInitializerClause> SemaPolicyDeferMemInitializerClause;
	// in case of an inline constructor-definition, parse will be deferred - otherwise parsed as normal
	SEMA_POLICY(cpp::mem_initializer_clause, SemaPolicyDeferMemInitializerClause)
	void action(cpp::mem_initializer_clause* symbol, const SemaMemInitializerClause& walker) // function_definition_suffix->ctor_initializer
	{
	}
	typedef DeferBody<SemaStatementSeq, skipBraced> DeferFunctionBody;
	typedef SemaPolicyPushDeferred<struct SemaStatementSeq, DeferFunctionBody> SemaPolicyDeferFunctionBody;
	// in case of an inline member-function-definition, parse will be deferred - otherwise parsed as normal
	SEMA_POLICY(cpp::statement_seq_wrapper, SemaPolicyDeferFunctionBody)
	void action(cpp::statement_seq_wrapper* symbol, const SemaStatementSeq& walker) // function_definition_suffix->function_body
	{
	}
	SEMA_POLICY(cpp::handler_seq, SemaPolicyPush<struct SemaHandlerSeq>)
	void action(cpp::handler_seq* symbol, const SemaHandlerSeq& walker) // function_definition_suffix->function_try_block
	{
	}

	SEMA_POLICY(cpp::parameter_declaration_default, SemaPolicyIdentity)
	void action(cpp::parameter_declaration_default* symbol)
	{
		commit();
		defaultArgument = symbol->init;
	}
	SEMA_POLICY(cpp::parameter_declaration_abstract, SemaPolicyIdentity)
	void action(cpp::parameter_declaration_abstract* symbol)
	{
		commit();
		defaultArgument = symbol->init;
	}
};


struct SemaSimpleDeclaration : public SemaBase, SemaSimpleDeclarationResult
{
	SEMA_BOILERPLATE;

	SemaDeclarationArgs args;

	// prefix
	SemaDeclSpecifierSeqResult seq;

	SemaSimpleDeclaration(const SemaState& state, SemaDeclarationArgs args = SemaDeclarationArgs())
		: SemaBase(state), args(args), seq(&gCtor, context)
	{
	}

	SEMA_POLICY(cpp::decl_specifier_seq, SemaPolicyPush<struct SemaDeclSpecifierSeq>)
	void action(cpp::decl_specifier_seq* symbol, const SemaDeclSpecifierSeq& walker)
	{
		seq = walker.seq;
		templateParams = walker.templateParams; // if this is a class template declaration, templateParams will have been cleared
	}
	SEMA_POLICY(cpp::type_specifier_seq, SemaPolicyPush<struct SemaDeclSpecifierSeq>)
	void action(cpp::type_specifier_seq* symbol, const SemaDeclSpecifierSeq& walker)
	{
		seq = walker.seq;
		templateParams = walker.templateParams; // if this is a class template declaration, templateParams will have been cleared
	}
	SEMA_POLICY(cpp::function_specifier, SemaPolicyIdentity)
	void action(cpp::function_specifier* symbol) // in constructor_definition/member_declaration_implicit -> function_specifier_seq
	{
		if(symbol->id == cpp::function_specifier::EXPLICIT)
		{
			seq.specifiers.isExplicit = true;
		}
	}
	typedef Args2<SemaDeclarationArgs, const SemaDeclSpecifierSeqResult&> DeclarationSuffixWalkerArgs;
	typedef SemaPolicyGeneric<SemaPush<SemaDeclarationSuffix, CommitNull, DeclarationSuffixWalkerArgs> > SemaPolicyDeclarationSuffix;
	SEMA_POLICY_ARGS(cpp::simple_declaration_named, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::simple_declaration_named* symbol, const SemaDeclarationSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::member_declaration_named, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::member_declaration_named* symbol, const SemaDeclarationSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::member_declaration_bitfield, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::member_declaration_bitfield* symbol, const SemaDeclarationSuffix& walker)
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::parameter_declaration_suffix, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::parameter_declaration_suffix* symbol, const SemaDeclarationSuffix& walker) // parameter_declaration
	{
		declaration = walker.declaration;
		defaultArgument = walker.defaultArgument;
	}
	SEMA_POLICY_ARGS(cpp::function_definition, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::function_definition* symbol, const SemaDeclarationSuffix& walker) // constructor_definition
	{
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::condition_declarator, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::condition_declarator* symbol, const SemaDeclarationSuffix& walker) // condition
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY_ARGS(cpp::exception_declarator, SemaPolicyDeclarationSuffix, DeclarationSuffixWalkerArgs(args, seq))
	void action(cpp::exception_declarator* symbol, SemaDeclarationSuffix& walker) // exception_declaration
	{
		walker.commit();
		declaration = walker.declaration;
	}

	SEMA_POLICY(cpp::type_declaration_suffix, SemaPolicyIdentity)
	void action(cpp::type_declaration_suffix* symbol)
	{
		declaration = seq.type.declaration; // no declarator was specified, this is the declaration of a class/enum
		if(seq.forward != 0) // declare the name found in elaborated-type-specifier parse
		{
			bool isSpecialization = !isClassKey(*seq.type.declaration);
			if(isSpecialization
				&& (seq.specifiers.isFriend
					|| isExplicitInstantiation))
			{
				// friend class C<int>; // friend
				// template class C<int>; // explicit instantiation
			}
			else if(seq.specifiers.isFriend)
			{
				// friend class C;
			}
			else
			{
				// class C;
				// template<class T> class C;
				// template<> class C<int>;
				// template<class T> class C<T*>;
				DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *seq.forward, TYPE_CLASS, 0, DeclSpecifiers(), templateParams != 0, getTemplateParams(), isSpecialization, seq.type.templateArguments);
#ifdef ALLOCATOR_DEBUG
				trackDeclaration(instance);
#endif
				setDecoration(seq.forward, instance);
				declaration = instance;
				if(declaration->templateParamScope == 0)
				{
					declaration->templateParamScope = templateParamScope; // required by findEnclosingType
				}
			}
			seq.type = TypeId(declaration, context); // TODO: is this necessary?
		}
		else if(declaration != 0
			&& declaration->isTemplate
			&& templateParams != 0) // if not an explicit-specialization
		{
			// template<class T> class C;
			SEMANTIC_ASSERT(!declaration->isSpecialization);
			mergeTemplateParamDefaults(*declaration, *templateParams);
		}

		if(seq.isUnion
			&& isAnonymous(*declaration))
		{
			/* class.union-2
			The names of the members of an anonymous union
			shall be distinct from the names of any other entity in the scope in which the anonymous union is declared.
			For the purpose of name lookup, after the anonymous union definition, the members of the anonymous union
			are considered to have been defined in the scope in which the anonymous union is declared.
			*/
			// TODO: verify that member names are distinct
			for(Scope::Declarations::iterator i = declaration->enclosed->declarations.begin(); i != declaration->enclosed->declarations.end(); ++i)
			{
				Declaration& member = *(*i).second;
				if(isAnonymous(member))
				{
					member.setName(enclosing->getUniqueName());
					if(member.enclosed != 0)
					{
						member.enclosed->name = member.getName();
					}
				}
				else
				{
					const DeclarationInstance* holder = ::findDeclaration(enclosing->declarations, member.getName());
					if(holder != 0)
					{
						Declaration* declaration = *holder;
						printPosition(member.getName().source);
						std::cout << "'" << member.getName().value.c_str() << "': anonymous union member already declared" << std::endl;
						printPosition(declaration->getName().source);
						throw SemanticError();
					}
				}
				member.scope = enclosing;
				Identifier* id = &member.getName();
				enclosing->declarations.insert(DeclarationInstance(&member, context.declarationCount++));
				enclosing->declarationList.push_back(&member);
			}
			declaration->enclosed = 0;
		}
	}
};


struct SemaDeclaration : public SemaBase, SemaDeclarationResult
{
	SEMA_BOILERPLATE;

	SemaDeclaration(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::linkage_specification, SemaPolicyPush<struct SemaNamespace>)
	void action(cpp::linkage_specification* symbol, const SemaNamespaceResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::namespace_definition, SemaPolicyPush<struct SemaNamespace>)
	void action(cpp::namespace_definition* symbol, const SemaNamespaceResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::namespace_alias_definition, SemaPolicyPush<struct SemaNamespaceAliasDefinition>)
	void action(cpp::namespace_alias_definition* symbol, const SemaNamespaceAliasDefinition& walker)
	{
	}
	SEMA_POLICY(cpp::general_declaration, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::general_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	// occurs in for-init-statement
	SEMA_POLICY(cpp::simple_declaration, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::simple_declaration* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::constructor_definition, SemaPolicyPush<struct SemaSimpleDeclaration>)
	void action(cpp::constructor_definition* symbol, const SemaSimpleDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::template_declaration, SemaPolicyPush<struct SemaTemplateDeclaration>)
	void action(cpp::template_declaration* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::explicit_instantiation, SemaPolicyPush<struct SemaExplicitInstantiation>)
	void action(cpp::explicit_instantiation* symbol, const SemaExplicitInstantiationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::explicit_specialization, SemaPolicyPush<struct SemaTemplateDeclaration>)
	void action(cpp::explicit_specialization* symbol, const SemaDeclarationResult& walker)
	{
		declaration = walker.declaration;
	}
	SEMA_POLICY(cpp::using_declaration, SemaPolicyPush<struct SemaUsingDeclaration>)
	void action(cpp::using_declaration* symbol, const SemaUsingDeclaration& walker)
	{
	}
	SEMA_POLICY(cpp::using_directive, SemaPolicyPush<struct SemaUsingDirective>)
	void action(cpp::using_directive* symbol, const SemaUsingDirective& walker)
	{
	}
};

#endif
