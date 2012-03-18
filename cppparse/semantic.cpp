

#include "semantic.h"
#include "util.h"

#include "profiler.h"
#include "symbols.h"

#include "parser_symbols.h"

#include <iostream>

struct SemanticError
{
	SemanticError()
	{
	}
};

#define SEMANTIC_ASSERT(condition) if(!(condition)) { throw SemanticError(); }

inline void semanticBreak()
{
}



void printDeclarations(const Scope::Declarations& declarations)
{
	std::cout << "{ ";
	for(Scope::Declarations::const_iterator i = declarations.begin(); i != declarations.end();)
	{
		std::cout << getValue((*i).second.getName());
		if(++i != declarations.end())
		{
			std::cout << ", ";
		}
	}
	std::cout << " }";
}

void printBases(const Types& bases)
{
	std::cout << "{ ";
	for(Types::const_iterator i = bases.begin(); i != bases.end();)
	{
		std::cout << getValue((*i).declaration->getName()) << ": ";
		Scope* scope = (*i).declaration->enclosed;
		if(scope != 0)
		{
			printDeclarations((*i).declaration->enclosed->declarations);
		}
		if(++i != bases.end())
		{
			std::cout << ", ";
		}
	}
	std::cout << " }";
}

void printScope(const Scope& scope)
{
	std::cout << getValue(scope.name) << ": ";
	std::cout << std::endl;
	std::cout << "  declarations: ";
	printDeclarations(scope.declarations);
	std::cout << std::endl;
	std::cout << "  bases: ";
	printBases(scope.bases);
	std::cout << std::endl;
	if(scope.parent != 0)
	{
		printScope(*scope.parent);
	}
}

void printName(const Scope& scope)
{
	if(scope.parent != 0)
	{
		printName(*scope.parent);
		std::cout << "::";
		std::cout << getValue(scope.name);
	}
}


struct IdentifierMismatch
{
	Identifier id;
	const char* expected;
	DeclarationPtr declaration;
	IdentifierMismatch()
	{
	}
	IdentifierMismatch(const Identifier& id, Declaration* declaration, const char* expected) :
		id(id), declaration(declaration), expected(expected)
	{
	}
};

const IdentifierMismatch IDENTIFIERMISMATCH_NULL = IdentifierMismatch(IDENTIFIER_NULL, 0, 0);
IdentifierMismatch gIdentifierMismatch = IDENTIFIERMISMATCH_NULL;

void printIdentifierMismatch(const IdentifierMismatch& e)
{
	printPosition(e.id.position);
	std::cout << "'" << getValue(e.id) << "' expected " << e.expected << ", " << (e.declaration == &gUndeclared ? "was undeclared" : "was declared here:") << std::endl;
	if(e.declaration != &gUndeclared)
	{
		printPosition(e.declaration->getName().position);
		std::cout << std::endl;
	}
}

Identifier gGlobalId = makeIdentifier("$global");


struct WalkerContext : public TreeAllocator<int>
{
	Scope global;
	Declaration globalDecl;
	TypeRef globalType;

	WalkerContext(const TreeAllocator<int>& allocator) :
		TreeAllocator<int>(allocator),
		global(allocator, gGlobalId, SCOPETYPE_NAMESPACE),
		globalDecl(allocator, 0, gGlobalId, TYPE_NULL, &global),
		globalType(Type(&globalDecl, allocator), allocator)
	{
	}
};

typedef std::list< DeferredParse<struct WalkerBase, struct WalkerState> > DeferredSymbols;

typedef bool (*IdentifierFunc)(const Declaration& declaration);
const char* getIdentifierType(IdentifierFunc func);

struct WalkerState
	: public ContextBase
{
	typedef WalkerState State;

	WalkerContext& context;
	ScopePtr enclosing;
	TypePtr qualifying_p;
	ScopePtr templateParams;
	ScopePtr templateEnclosing;
	DeferredSymbols* deferred;

	WalkerState(WalkerContext& context)
		: context(context), enclosing(0), qualifying_p(0), templateParams(0), templateEnclosing(0), deferred(0)
	{
	}
	const WalkerState& getState() const
	{ 
		return *this;
	}

	LookupResult findDeclaration(const Identifier& id, LookupFilter filter = IsAny())
	{
		ProfileScope profile(gProfileLookup);
#ifdef LOOKUP_DEBUG
		std::cout << "lookup: " << getValue(id) << " (" << getIdentifierType(filter) << ")" << std::endl;
#endif
		LookupResult result;
		if(getQualifyingScope() != 0)
		{
			if(result.append(::findDeclaration(*getQualifyingScope(), id, filter)))
			{
#ifdef LOOKUP_DEBUG
				std::cout << "HIT: qualified" << std::endl;
#endif
				return result;
			}
		}
		else
		{
#if 1
			if(templateParams != 0)
			{
				if(result.append(::findDeclaration(*templateParams, id, filter)))
				{
#ifdef LOOKUP_DEBUG
					std::cout << "HIT: templateParams" << std::endl;
#endif
					return result;
				}
			}
#endif
			if(result.append(::findDeclaration(*enclosing, id, filter)))
			{
#ifdef LOOKUP_DEBUG
				std::cout << "HIT: unqualified" << std::endl;
#endif
				return result;
			}
		}
#ifdef LOOKUP_DEBUG
		std::cout << "FAIL" << std::endl;
#endif
		result.filtered = &gUndeclared;
		return result;
	}

	Declaration* pointOfDeclaration(
		const TreeAllocator<int>& allocator,
		Scope* parent,
		Identifier& name,
		const Type& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		TemplateArguments& arguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = DEPENDENT_NULL)
	{
		SEMANTIC_ASSERT(parent != 0);

		parser->context.allocator.deferredBacktrack(); // flush cached parse-tree

		Declaration other(allocator, parent, name, type, enclosed, specifiers, isTemplate, arguments, templateParameter, valueDependent);
		if(!name.value.empty()) // unnamed class/struct/union/enum
		{
			/* 3.4.4-1
			An elaborated-type-specifier (7.1.6.3) may be used to refer to a previously declared class-name or enum-name
			even though the name has been hidden by a non-type declaration (3.3.10).
			*/
			Declaration* declaration = ::findDeclaration(parent->declarations, name);
			if(declaration != 0)
			{
				try
				{
					const Declaration& primary = getPrimaryDeclaration(*declaration, other);
					if(&primary == declaration)
					{
						return declaration;
					}
				}
				catch(DeclarationError& e)
				{
					printPosition(name.position);
					std::cout << "'" << name.value.c_str() << "': " << e.description << std::endl;
					printPosition(declaration->getName().position);
					throw SemanticError();
				}

				if(isClass(other)
					&& isClass(*declaration)
					&& isIncomplete(*declaration)) // if this class-declaration was previously forward-declared
				{
					// quick hack - complete all previous forward-declarations, in case they are referenced by typedefs
					for(Declaration* p = declaration; p != 0; p = p->overloaded)
					{
						p->enclosed = other.enclosed;
					}
				}

				if(!isNamespace(other)
					&& !isType(other)
					&& isFunction(other))
				{
					// quick hack - if any template overload of a function has been declared, all subsequent declarations are template functions
					if(declaration->isTemplate)
					{
						other.isTemplate = true;
					}
				}
				other.overloaded = declaration;
			}
		}

		Declaration* result = parent->declarations.insert(other);
		return result;
	}

	TreeAllocator<int> getAllocator()
	{
#ifdef TREEALLOCATOR_LINEAR
		return parser->context.allocator;
#else
		return DebugAllocator<int>();
#endif
	}

	Scope* findScope(Scope* scope, Scope* other)
	{
		if(scope == 0)
		{
			return 0;
		}
		if(scope == other)
		{
			return scope;
		}
		return findScope(scope->parent, other);
	}

	void pushScope(Scope* scope)
	{
		SEMANTIC_ASSERT(findScope(enclosing, scope) == 0);
		scope->parent = enclosing;
		enclosing = scope;
	}

	void addBase(Declaration* declaration, const Type& base)
	{
		if(getInstantiatedType(base).declaration == declaration)
		{
			return; // TODO: implement template-instantiation, and disallow inheriting from current-instantiation
		}
		declaration->enclosed->bases.push_front(base);
	}

	Scope* getQualifyingScope()
	{
		if(!qualifying_p)
		{
			return 0;
		}
		return getInstantiatedType(*qualifying_p).declaration->enclosed;
	}

	void clearQualifying()
	{
		qualifying_p = 0;
	}

	template<typename T>
	void reportIdentifierMismatch(T* symbol, const Identifier& id, Declaration* declaration, const char* expected)
	{
		result = 0;
#if 0
		gIdentifierMismatch = IdentifierMismatch(id, declaration, expected);
#endif
	}

	Scope* getEtsScope()
	{
		Scope* scope = enclosing;
		for(; !enclosesEts(scope->type); scope = scope->parent)
		{
		}
		return scope;
	}

	Scope* getFriendScope()
	{
		SEMANTIC_ASSERT(enclosing->type == SCOPETYPE_CLASS);
		Scope* scope = enclosing;
		for(; scope->type != SCOPETYPE_NAMESPACE; scope = scope->parent)
		{
			if(scope->type == SCOPETYPE_LOCAL)
			{
				return enclosing; // friend declaration in a local class lives in class scope
			}
		}
		return scope;
	}

	Scope* getClassScope()
	{
		Scope* scope = enclosing;
		for(; scope->type != SCOPETYPE_CLASS; scope = scope->parent)
		{
		}
		return scope;
	}

	void printScope()
	{
#if 1
		if(templateParams != 0)
		{
			std::cout << "template-params:" << std::endl;
			::printScope(*templateParams);
		}
#endif
		if(getQualifyingScope() != 0)
		{
			std::cout << "qualifying:" << std::endl;
			::printScope(*getQualifyingScope());
		}
		else
		{
			std::cout << "enclosing:" << std::endl;
			::printScope(*enclosing);
		}
	}

	bool isDependent(const Type& type)
	{
		//std::cout << "isDependent(Type)" << std::endl;
		return ::isDependent(type, DependentContext(*enclosing, templateParams != 0 ? *templateParams : SCOPE_NULL));
	}

	bool isDependent(const Types& bases)
	{
		//std::cout << "isDependent(Types)" << std::endl;
		return ::isDependent(bases, DependentContext(*enclosing, templateParams != 0 ? *templateParams : SCOPE_NULL));
	}

	bool isDependent(const TypePtr& qualifying)
	{
		if(qualifying != 0)
		{
			//std::cout << "isDependent(Type*)" << std::endl;
		}
		return ::isDependent(qualifying.get(), DependentContext(*enclosing, templateParams != 0 ? *templateParams : SCOPE_NULL));
	}


	void addDependent(Dependent& dependent, const DependencyCallback& callback)
	{
		dependent.push_front(callback);
	}
	void addDependentName(Dependent& dependent, Declaration* declaration)
	{
		static DependencyCallbacks<Declaration> callbacks = makeDependencyCallbacks(isDependentName);
		addDependent(dependent, makeDependencyCallback(declaration, &callbacks));
	}
	void addDependentType(Dependent& dependent, Declaration* declaration)
	{
		static DependencyCallbacks<const Type> callbacks = makeDependencyCallbacks(isDependentType);
		addDependent(dependent, makeDependencyCallback(const_cast<const Type*>(&declaration->type), &callbacks));
	}
	void addDependent(Dependent& dependent, const Type& type)
	{
		TypeRef tmp(type, context);
		static DependencyCallbacks<TypePtr::Value> callbacks = makeDependencyCallbacks(isDependentTypeRef, ReferenceCallbacks<const Type>::increment, ReferenceCallbacks<const Type>::decrement);
		addDependent(dependent, makeDependencyCallback(tmp.get_ref().p, &callbacks));
	}
	void addDependent(Dependent& dependent, Scope* scope)
	{
		static DependencyCallbacks<Scope> callbacks = makeDependencyCallbacks(isDependentClass);
		addDependent(dependent, makeDependencyCallback(scope, &callbacks));
	}
#if 0
	void addDependent(Dependent& dependent, Dependent& other)
	{
		dependent.splice(other);
	}
#else
	void addDependent(Dependent& dependent, Dependent& other)
	{
		if(other.empty())
		{
			return;
		}
		if(dependent.empty())
		{
			dependent = other;
			return;
		}
		CopiedReference<Dependent, TreeAllocator<int> > tmp(other, context);
		static DependencyCallbacks<Reference<Dependent>::Value> callbacks = makeDependencyCallbacks(isDependentListRef, ReferenceCallbacks<Dependent>::increment, ReferenceCallbacks<Dependent>::decrement);
		addDependent(dependent, makeDependencyCallback(tmp.get_ref().p, &callbacks));
	}
#endif
};


struct WalkerBase : public WalkerState
{
	typedef WalkerBase Base;

	WalkerBase(WalkerContext& context)
		: WalkerState(context)
	{
	}
	WalkerBase(const WalkerState& state)
		: WalkerState(state)
	{
	}
	Scope* newScope(const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
	{
		return allocatorNew(context, Scope(context, name, type));
	}

	void trackDeclaration(Declaration* declaration)
	{
		parser->addBacktrackCallback(makeUndeclareCallback(declaration));
	}

	Declaration* declareClass(Identifier* id, TemplateArguments& arguments)
	{
		Scope* enclosed = templateParams != 0 ? static_cast<Scope*>(templateParams) : newScope(makeIdentifier("$class"));
		enclosed->type = SCOPETYPE_CLASS; // convert template-param-scope to class-scope if present
		Declaration* declaration = pointOfDeclaration(context, enclosing, id == 0 ? enclosing->getUniqueName() : *id, TYPE_CLASS, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, arguments);
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		if(id != 0)
		{
			id->dec.p = declaration;
		}
		enclosed->name = declaration->getName();
		return declaration;
	}

	Declaration* declareObject(Scope* parent, Identifier* id, const Type& type, Scope* enclosed, DeclSpecifiers specifiers, size_t templateParameter, const Dependent& valueDependent)
	{
		// 7.3.1.2 Namespace member definitions
		// Paragraph 3
		// Every name first declared in a namespace is a member of that namespace. If a friend declaration in a non-local class
		// first declares a class or function (this implies that the name of the class or function is unqualified) the friend
		// class or function is a member of the innermost enclosing namespace.
		if(specifiers.isFriend // is friend
			&& parent == enclosing) // is unqualified
		{
			parent = getFriendScope();
		}
		Declaration* declaration = pointOfDeclaration(context, parent, *id, type, enclosed, specifiers, enclosing == templateEnclosing, TEMPLATEARGUMENTS_NULL, templateParameter, valueDependent); // 3.3.1.1
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		if(id != &gAnonymousId)
		{
			id->dec.p = declaration;
		}
		return declaration;
	}

	bool declareEts(Type& type, Identifier* forward)
	{
		if(isClassKey(*type.declaration))
		{
			SEMANTIC_ASSERT(forward != 0);
			/* 3.3.1-6
			if the elaborated-type-specifier is used in the decl-specifier-seq or parameter-declaration-clause of a
			function defined in namespace scope, the identifier is declared as a class-name in the namespace that
			contains the declaration; otherwise, except as a friend declaration, the identifier is declared in the
			smallest non-class, non-function-prototype scope that contains the declaration.
			*/
			Declaration* declaration = pointOfDeclaration(context, getEtsScope(), *forward, TYPE_CLASS, 0, DeclSpecifiers(), enclosing == templateEnclosing);
			trackDeclaration(declaration);
			forward->dec.p = declaration;
			type = declaration;
			return true;
		}
		return false;
	}

	Declaration* lookupType(const Identifier& id, LookupFilter filter)
	{
		if(!isDependent(qualifying_p))
		{
			return findDeclaration(id, filter);
		}
		return &gDependentTemplate;
	}

	// 4.5 Integral Promotions
	// TODO: handle bitfield types?
	const Type& promoteToIntegralType(const Type& type)
	{
		if(&type == &gChar
			|| &type == &gSignedChar
			|| &type == &gUnsignedChar
			|| &type == &gSignedShortInt
			|| &type == &gUnsignedShortInt)
		{
			return gSignedInt;
		}
		if(&type == &gWCharT
			|| type.declaration == &gEnum)
		{
			return gSignedInt;
		}
		if(&type == &gBool)
		{
			return gSignedInt;
		}
		return type;
	}
	// 5 Expressions
	// paragraph 9: usual arithmetic conversions
	const Type& binaryOperatorIntegralType(const Type& left, const Type& right)
	{
		if(&left == &gUnsignedLongInt || &right == &gUnsignedLongInt)
		{
			return gLongDouble;
		}
		if((&left == &gSignedLongInt && &right == &gUnsignedInt)
			|| (&left == &gUnsignedInt && &right == &gSignedLongInt))
		{
			return gUnsignedLongInt;
		}
		if(&left == &gSignedLongInt || &right == &gSignedLongInt)
		{
			return gSignedLongInt;
		}
		if(&left == &gUnsignedInt || &right == &gUnsignedInt)
		{
			return gUnsignedInt;
		}
		return gSignedInt;
	}
	const Type& binaryOperatorType(const Type& left, const Type& right)
	{
		if(&left == &gLongDouble || &right == &gLongDouble)
		{
			return gLongDouble;
		}
		if(&left == &gDouble || &right == &gDouble)
		{
			return gDouble;
		}
		if(&left == &gFloat || &right == &gFloat)
		{
			return gFloat;
		}
		return binaryOperatorIntegralType(promoteToIntegralType(left), promoteToIntegralType(right));
	}
};

struct WalkerQualified : public WalkerBase
{
	Qualifying qualifying;
	WalkerQualified(const WalkerState& state)
		: WalkerBase(state), qualifying(context)
	{
	}

	void setQualifyingGlobal()
	{
		SEMANTIC_ASSERT(qualifying.empty());
		qualifying_p = context.globalType.get_ref();
	}

	void swapQualifying(Type& type)
	{
#if 0 // allow incomplete types as qualifying, for nested-name-specifier in ptr-operator (defining member-function-ptr)
		if(type.declaration->enclosed == 0)
		{
			// TODO
			//printPosition(symbol->id->value.position);
			std::cout << "'" << getValue(type.declaration->name) << "' is incomplete, declared here:" << std::endl;
			printPosition(type.declaration->getName().position);
			throw SemanticError();
		}
#endif
		Qualifying tmp(type, context);
		swapQualifying(tmp);
	}
	void swapQualifying(Qualifying& other)
	{
		qualifying.swap(other);
		qualifying_p = qualifying.get_ref();
	}
};



bool isTypeName(const Declaration& declaration)
{
	return isType(declaration);
}

typedef LookupFilterDefault<isTypeName> IsTypeName;

bool isNamespaceName(const Declaration& declaration)
{
	return isNamespace(declaration);
}

typedef LookupFilterDefault<isNamespaceName> IsNamespaceName;


bool isTemplateName(const Declaration& declaration)
{
	// returns true if \p declaration is a template class, function or template-parameter
	return declaration.isTemplate && (isClass(declaration) || isFunction(declaration) || isTypedef(declaration));
}

bool isNestedName(const Declaration& declaration)
{
	return isTypeName(declaration)
		|| isNamespaceName(declaration);
}

typedef LookupFilterDefault<isNestedName> IsNestedName;


const char* getIdentifierType(IdentifierFunc func)
{
	if(func == isTypeName)
	{
		return "type-name";
	}
	if(func == isNamespaceName)
	{
		return "namespace-name";
	}
	if(func == isTemplateName)
	{
		return "template-name";
	}
	if(func == isNestedName)
	{
		return "nested-name";
	}
	return "<unknown>";
}


#define TREEWALKER_WALK(walker, symbol) SYMBOL_WALK(walker, symbol)
#define TREEWALKER_LEAF(symbol) SYMBOL_WALK(*this, symbol)

#define TREEWALKER_DEFAULT PARSERCONTEXT_DEFAULT


#define TREEWALKER_WALK_CACHED_DATA(walker, symbol, data) \
	if(!parser->cacheLookup((symbol), (data))) \
	{ \
		CachedSymbols::Key key = parser->context.position; \
		TREEWALKER_WALK(walker, symbol); \
		parser->cacheStore(key, (symbol), (data)); \
	} \
	else \
	{ \
		result = symbol; \
	}

#define TREEWALKER_WALK_CACHED(walker, symbol) TREEWALKER_WALK_CACHED_DATA(walker, symbol, walker)
#define TREEWALKER_LEAF_CACHED(symbol) int _dummy; TREEWALKER_WALK_CACHED_DATA(*this, symbol, _dummy)


bool isUnqualified(cpp::elaborated_type_specifier_default* symbol)
{
	return symbol != 0
		&& symbol->isGlobal.value.empty()
		&& symbol->context.p == 0;
}


struct Walker
{


struct IsHiddenNamespaceName
{
	DeclarationPtr hidingType; // valid if the declaration is hidden by a type name

	IsHiddenNamespaceName()
		: hidingType(0)
	{
	}

	bool operator()(const Declaration& declaration)
	{
		if(isNamespaceName(declaration))
		{
			return true;
		}
		if(hidingType == 0
			&& isTypeName(declaration))
		{
			hidingType = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		return false;
	}
};

struct NamespaceNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IsHiddenNamespaceName filter;
	NamespaceNameWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		declaration = findDeclaration(symbol->value, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
		}
		symbol->value.dec.p = declaration;
	}
};

struct TemplateArgumentListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateArgument argument;
	TemplateArguments arguments;

	TemplateArgumentListWalker(const WalkerState& state)
		: WalkerBase(state), argument(context), arguments(context)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		argument.type.swap(walker.type);
		argument.dependent.swap(walker.valueDependent);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		argument.dependent.swap(walker.typeDependent);
		addDependent(argument.dependent, walker.valueDependent);
	}
	void visit(cpp::template_argument_list* symbol)
	{
		TemplateArgumentListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		arguments.swap(walker.arguments);
		arguments.push_front(walker.argument); // allocates last element first!
	}
};

struct TemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	TemplateArguments arguments;
	TemplateIdWalker(const WalkerState& state)
		: WalkerBase(state), id(0), arguments(context)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
	}
	void visit(cpp::template_argument_clause* symbol)
	{
		clearQualifying();
		TemplateArgumentListWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		arguments.swap(walker.arguments);
	}
};


struct UnqualifiedIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	bool isIdentifier;
	bool isTemplate;
	UnqualifiedIdWalker(const WalkerState& state, bool isTemplate = false)
		: WalkerBase(state), declaration(0), id(0), isIdentifier(false), isTemplate(isTemplate)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
		isIdentifier = true;
		if(!isDependent(qualifying_p))
		{
			declaration = findDeclaration(*id);
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		if(!isTemplate
			&& !isDependent(qualifying_p))
		{
			declaration = findDeclaration(*id);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *id, declaration, "template-name");
			}
			declaration = findTemplateSpecialization(declaration, walker.arguments);
		}
	}
	void visit(cpp::template_id_operator_function* symbol)
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		TREEWALKER_LEAF(symbol);
		// TODO
		id = &gOperatorFunctionId;
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO
		id = &gConversionFunctionId;
	}
	void visit(cpp::destructor_id* symbol)
	{
		// TODO: can destructor-id be dependent?
		TREEWALKER_LEAF(symbol);
		id = &symbol->name->value;
	}
};

struct QualifiedIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	bool isTemplate;
	QualifiedIdWalker(const WalkerState& state)
		: WalkerQualified(state), declaration(0), id(0), isTemplate(false)
	{
	}

	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
	}
	void visit(cpp::qualified_id_suffix* symbol)
	{
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
	}
};

struct IdExpressionWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	/* 14.6.2.2-3
	An id-expression is type-dependent if it contains:
	 an identifier that was declared with a dependent type,
	 a template-id that is dependent,
	 a conversion-function-id that specifies a dependent type,
	 a nested-name-specifier or a qualified-id that names a member of an unknown specialization
	*/
	DeclarationPtr declaration;
	IdentifierPtr id;
	bool isIdentifier;
	bool isTemplate;
	IdExpressionWalker(const WalkerState& state)
		: WalkerQualified(state), declaration(0), id(0), isIdentifier(false), isTemplate(false)
	{
	}
	// HACK: for postfix-expression-member 
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::qualified_id_default* symbol)
	{
		// TODO
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		// TODO
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		// TODO
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		isIdentifier = walker.isIdentifier;
	}
};

struct ExplicitTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent typeDependent;
	Dependent valueDependent;
	ExplicitTypeExpressionWalker(const WalkerState& state)
		: WalkerBase(state), typeDependent(context), valueDependent(context)
	{
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
		addDependent(typeDependent, walker.valueDependent);
	}
	void visit(cpp::new_type* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
		addDependent(typeDependent, walker.valueDependent);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::cast_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct DependentPrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	Dependent typeDependent;
	DependentPrimaryExpressionWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), typeDependent(context)
	{
	}
	void visit(cpp::id_expression* symbol)
	{
		/* temp.dep.expr
		An id-expression is type-dependent if it contains:
		 an identifier that was declared with a dependent type,
		 a template-id that is dependent,
		 a conversion-function-id that specifies a dependent type,
		 a nested-name-specifier or a qualified-id that names a member of an unknown specialization.
		*/
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		if(walker.isIdentifier // expression is 'identifier'
			&& (declaration == &gUndeclared // identifier was not previously declared
				|| (isObject(*declaration) && declaration->scope->type == SCOPETYPE_NAMESPACE))) // identifier was previously declared in namespace-scope
		{
			// defer name-lookup: this identifier may be a dependent-name.
			id = walker.id;
		}
		else
		{
			if(declaration != 0)
			{
				if(declaration == &gUndeclared
					|| !isObject(*declaration))
				{
					return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name");
				}
				walker.id->dec.p = declaration;
				addDependentType(typeDependent, declaration);
			}
			else if(walker.id != 0)
			{
				walker.id->dec.p = &gDependentObject;
				if(!walker.qualifying.empty())
				{
					addDependent(typeDependent, walker.qualifying.back());
				}
			}
		}
	}
	void visit(cpp::primary_expression_parenthesis* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
};

// walks an argument-dependent-lookup function-call expression: postfix-expression ( expression-list. )
struct DependentPostfixExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	Dependent typeDependent;
	DependentPostfixExpressionWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), typeDependent(context)
	{
	}
	void visit(cpp::primary_expression* symbol)
	{
		DependentPrimaryExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::postfix_expression_call* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		if(id != 0)
		{
			if(typeDependent.empty())
			{
				if(declaration != 0)
				{
					if(declaration == &gUndeclared
						|| !isObject(*declaration))
					{
						return reportIdentifierMismatch(symbol, *id, declaration, "object-name");
					}
					addDependentType(typeDependent, declaration);
					id->dec.p = declaration;
				}
			}
			else
			{
				id->dec.p = &gDependentObject;
			}
		}
	}
};

struct LiteralWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	LiteralWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context)
	{
	}
	void visit(cpp::numeric_literal* symbol)
	{
		TREEWALKER_LEAF(symbol);
		type = gSignedInt; // TODO: determine actual type
	}
	void visit(cpp::string_literal* symbol)
	{
		TREEWALKER_LEAF(symbol);
		type = gChar; // TODO: determine actual type
	}
};

struct BinaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;
	Type type;
	Dependent typeDependent;
	Dependent valueDependent;
	bool postOperator;
	BinaryExpressionWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context), typeDependent(context), valueDependent(context), postOperator(false)
	{
	}
	template<typename Walker>
	void parsedExpression(Walker& walker)
	{
		if(postOperator)
		{
			if(type.declaration != 0) // TODO: assert
			{
				type = binaryOperatorType(type, walker.type);
			}
		}
		else
		{
			type.swap(walker.type);
		}
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::multiplicative_operator* symbol)
	{
		TREEWALKER_LEAF(symbol);
		postOperator = true;
	}
	void visit(cpp::pm_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		parsedExpression(walker);
	}
};

struct ExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	/* 14.6.2.2-1
	...an expression is type-dependent if any subexpression is type-dependent.
	*/
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context), typeDependent(context), valueDependent(context)
	{
	}
	void visit(cpp::postfix_expression_member* symbol)
	{
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: name-lookup for member id-expression
		// TODO: inherit type-dependent property
	}
	void visit(cpp::literal* symbol)
	{
		LiteralWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
	}
	void visit(cpp::multiplicative_expression* symbol)
	{
		BinaryExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::primary_expression_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		// TODO
		/* 14.6.2.2-2
		'this' is type-dependent if the class type of the enclosing member function is dependent
		*/
		addDependent(typeDependent, getClassScope());
	}
	void visit(cpp::type_id* symbol)
	{
		// TODO
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	/* temp.dep.constexpr
	An identifier is value-dependent if it is:
	 a name declared with a dependent type,
	 the name of a non-type template parameter,
	 a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
	*/
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		Declaration* declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name");
			}
			addDependentType(typeDependent, declaration);
			addDependentType(valueDependent, declaration);
			addDependentName(valueDependent, declaration);
			walker.id->dec.p = declaration;
		}
		else
		{
			if(isDependent(walker.qualifying.get_ref()))
			{
				walker.id->dec.p = &gDependentObject;
			}
			if(!walker.qualifying.empty())
			{
				addDependent(typeDependent, walker.qualifying.back());
			}
		}
	}
	/* 14.6.2.2-1
	... an expression is type-dependent if any subexpression is type-dependent.
	*/
	void visit(cpp::postfix_expression_disambiguate* symbol)
	{
		// TODO: should this also apply to postfix_expression_default?
		// TODO
		/* 14.6.2-1
		In an expression of the form:
		postfix-expression ( expression-list. )
		where the postfix-expression is an unqualified-id but not a template-id, the unqualified-id denotes a dependent
		name if and only if any of the expressions in the expression-list is a type-dependent expression (
		*/
		DependentPostfixExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
	/* 14.6.2.2-3
	Expressions of the following forms are type-dependent only if the type specified by the type-id, simple-type-specifier
	or new-type-id is dependent, even if any subexpression is type-dependent:
	- postfix-expression-construct
	- new-expression
	- postfix-expression-cast
	- cast-expression
	*/
	/* temp.dep.constexpr
	Expressions of the following form are value-dependent if either the type-id or simple-type-specifier is dependent or the
	expression or cast-expression is value-dependent:
	simple-type-specifier ( expression-listopt )
	static_cast < type-id > ( expression )
	const_cast < type-id > ( expression )
	reinterpret_cast < type-id > ( expression )
	( type-id ) cast-expression
	*/
	void visit(cpp::postfix_expression_construct* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::new_expression_placement* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::new_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
	}
	void visit(cpp::postfix_expression_cast* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(symbol->op->id != cpp::cast_operator::DYNAMIC)
		{
			Dependent tmp(walker.typeDependent);
			addDependent(valueDependent, tmp);
		}
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		Dependent tmp(walker.typeDependent);
		addDependent(valueDependent, tmp);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
	/* 14.6.2.2-4
	Expressions of the following forms are never type-dependent (because the type of the expression cannot be
	dependent):
	literal
	postfix-expression . pseudo-destructor-name
	postfix-expression -> pseudo-destructor-name
	sizeof unary-expression
	sizeof ( type-id )
	sizeof ... ( identifier )
	alignof ( type-id )
	typeid ( expression )
	typeid ( type-id )
	::opt delete cast-expression
	::opt delete [ ] cast-expression
	throw assignment-expressionopt
	*/
	// TODO: destructor-call is not dependent
	/* temp.dep.constexpr
	Expressions of the following form are value-dependent if the unary-expression is type-dependent or the type-id is dependent
	(even if sizeof unary-expression and sizeof ( type-id ) are not type-dependent):
	sizeof unary-expression
	sizeof ( type-id )
	*/
	void visit(cpp::unary_expression_sizeof* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.typeDependent);
	}
	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.type);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::postfix_expression_typeid* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::postfix_expression_typeidtype* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::delete_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::throw_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct IsHiddenTypeName
{
	DeclarationPtr nonType; // valid if the declaration is hidden by a non-type name
	DeclarationPtr hidingNamespace; // valid if the declaration is hidden by a namespace name

	IsHiddenTypeName()
		: nonType(0), hidingNamespace(0)
	{
	}

	bool operator()(const Declaration& declaration)
	{
		if(isTypeName(declaration))
		{
			return true;
		}
		if(nonType == 0
			&& isAny(declaration))
		{
			nonType = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		if(hidingNamespace == 0
			&& isNamespaceName(declaration))
		{
			hidingNamespace = const_cast<Declaration*>(&declaration); // TODO: fix const
		}
		return false;
	}
};


struct TypeNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	IsHiddenTypeName filter;
	bool isTypename; // true if a type is expected in this context; e.g. following 'typename', preceding '::'
	TypeNameWalker(const WalkerState& state, bool isTypename = false)
		: WalkerBase(state), type(0, context), isTypename(isTypename)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		Declaration* declaration = &gDependentType;
		if(!isDependent(qualifying_p))
		{
			declaration = findDeclaration(symbol->value, makeLookupFilter(filter));
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "type-name");
			}
 		}
		else if(!isTypename)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, symbol->value, &gUndeclared, "typename");
		}
		type.declaration = declaration;
		type.isImplicitTemplateId = declaration->isTemplate;
		symbol->value.dec.p = declaration;
	}

	void visit(cpp::simple_template_id* symbol)
	{
		//ProfileScope profile(gProfileTemplateId);

		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		Declaration* declaration = lookupType(*walker.id, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "type-name");
		}
		if(declaration == &gDependentType
			&& !isTypename)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}

		walker.id->dec.p = declaration;
		type.declaration = findTemplateSpecialization(declaration, walker.arguments);
		type.templateArguments.swap(walker.arguments);
	}
};

struct NestedNameSpecifierSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool allowDependent;
	bool isTemplate;
	NestedNameSpecifierSuffixWalker(const WalkerState& state, bool allowDependent = false)
		: WalkerBase(state), type(0, context), allowDependent(allowDependent), isTemplate(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		Declaration* declaration = &gDependentNested;
		if(allowDependent
			|| !isDependent(qualifying_p))
		{
			declaration = findDeclaration(symbol->value, IsNestedName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "nested-name");
			}
			type = declaration;
		}
		symbol->value.dec.p = declaration;
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(!isTemplate
			&& (allowDependent
				|| !isDependent(qualifying_p)))
		{
			Declaration* declaration = findDeclaration(*walker.id, IsNestedName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "nested-name");
			}
			declaration = findTemplateSpecialization(declaration, walker.arguments);
			type.declaration = declaration;
			type.templateArguments.swap(walker.arguments);
		}
	}
};

// basic.lookup.qual
// During the lookup for a name preceding the :: scope resolution operator, object, function, and enumerator names are ignored.
struct NestedNameSpecifierPrefixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool allowDependent;
	NestedNameSpecifierPrefixWalker(const WalkerState& state, bool allowDependent = false)
		: WalkerBase(state), type(0, context), allowDependent(allowDependent)
	{
	}

#if 0 // for debugging parse-tree cache
	void visit(cpp::nested_name* symbol)
	{
		NestedNameSpecifierPrefixWalker walker(getState(), allowDependent);
		TREEWALKER_WALK_CACHED(walker, symbol);
		type.swap(walker.type);
	}
#endif
	void visit(cpp::namespace_name* symbol)
	{
		NamespaceNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.hidingType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.hidingType->getName(), walker.filter.hidingType, "namespace-name");
		}
		type.declaration = walker.declaration;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.hidingNamespace != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.hidingNamespace->getName(), walker.filter.hidingNamespace, "type-name");
		}
		if(allowDependent
			|| !isDependent(qualifying_p))
		{
			type.swap(walker.type);
		}
	}
};

struct NestedNameSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	bool allowDependent;
	NestedNameSpecifierWalker(const WalkerState& state, bool allowDependent = false)
		: WalkerQualified(state), allowDependent(allowDependent)
	{
	}
	void visit(cpp::nested_name_specifier_prefix* symbol)
	{
		NestedNameSpecifierPrefixWalker walker(getState(), allowDependent);
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0)
		{
			swapQualifying(walker.type);
		}
	}
	void visit(cpp::nested_name_specifier_suffix_template* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(getState(), allowDependent);
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0)
		{
			walker.type.qualifying.swap(qualifying);
			swapQualifying(walker.type);
		}
	}
	void visit(cpp::nested_name_specifier_suffix_default* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(getState(), allowDependent);
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0)
		{
			walker.type.qualifying.swap(qualifying);
			swapQualifying(walker.type);
		}
	}
};

struct TypeSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	unsigned fundamental;
	TypeSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context), fundamental(0)
	{
	}
	void visit(cpp::simple_type_specifier_name* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		fundamental = walker.fundamental;
	}
	void visit(cpp::simple_type_specifier_template* symbol) // X::template Y<Z>
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		fundamental = walker.fundamental;
	}
	void visit(cpp::type_name* symbol) // simple_type_specifier_name
	{
		TypeNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol) // simple_type_specifier_name | simple_type_specifier_template
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::simple_template_id* symbol) // simple_type_specifier_template
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		IsHiddenTypeName filter;
		Declaration* declaration = lookupType(*walker.id, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, declaration, "type-name");
		}
		if(declaration == &gDependentType)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}
		if(filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, filter.nonType->getName(), filter.nonType, "type-name");
		}
		walker.id->dec.p = declaration;
		type.declaration = findTemplateSpecialization(declaration, walker.arguments);
		type.templateArguments.swap(walker.arguments);
		type.qualifying.swap(qualifying);
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		fundamental = symbol->id;
	}
};

struct UnqualifiedDeclaratorIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	UnqualifiedDeclaratorIdWalker(const WalkerState& state)
		: WalkerBase(state), id(&gAnonymousId)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		TREEWALKER_LEAF(symbol);
		// TODO
		id = &gOperatorFunctionId;
	}
	void visit(cpp::conversion_function_id* symbol) 
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO
		id = &gConversionFunctionId;
	}
	void visit(cpp::destructor_id* symbol) 
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->name->value;
	}
	void visit(cpp::template_id_operator_function* symbol) 
	{
		TREEWALKER_LEAF(symbol);
		// TODO
		id = &gOperatorFunctionTemplateId;
	}
};

struct QualifiedDeclaratorIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	QualifiedDeclaratorIdWalker(const WalkerState& state)
		: WalkerQualified(state), id(&gAnonymousId)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState(), true); // the qualifying nested-name-specifier may be dependent on a template-parameter
		TREEWALKER_WALK(walker, symbol); // no need to cache: the nested-name-specifier is not a shared-prefix
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
	}
};

struct DeclaratorIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	DeclaratorIdWalker(const WalkerState& state)
		: WalkerQualified(state), id(&gAnonymousId)
	{
	}
	void visit(cpp::qualified_id_default* symbol)
	{
		QualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		QualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
	}
};

struct ParameterDeclarationClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ParameterDeclarationClauseWalker(const WalkerState& state)
		: WalkerBase(state)
	{
#if 0 // TODO: this causes error if parse fails after templateParams is modified
		pushScope(templateParams != 0 ? templateParams : newScope(makeIdentifier("$declarator")));
		enclosing->type = SCOPETYPE_PROTOTYPE;
#else
		pushScope(newScope(makeIdentifier("$declarator"), SCOPETYPE_PROTOTYPE));
		if(templateParams != 0)
		{
			enclosing->declarations = templateParams->declarations;
			for(Scope::Declarations::iterator i = enclosing->declarations.begin(); i != enclosing->declarations.end(); ++i)
			{
				Declaration* declaration = &(*i).second;
				declaration->getName().dec.p = declaration;
				declaration->scope = enclosing;
				trackDeclaration(declaration);
			}
		}
#endif	
		templateParams = 0;
	}

	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ExceptionSpecificationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ExceptionSpecificationWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct PtrOperatorWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	PtrOperatorWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
};

struct DeclaratorSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ScopePtr paramScope;
	Dependent valueDependent;
	DeclaratorSuffixWalker(const WalkerState& state)
		: WalkerBase(state), paramScope(0), valueDependent(context)
	{
	}

	void visit(cpp::parameter_declaration_clause* symbol)
	{
		ParameterDeclarationClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		paramScope = walker.enclosing; // store reference for later resumption
	}
	void visit(cpp::exception_specification* symbol)
	{
		ExceptionSpecificationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	ScopePtr paramScope;
	Dependent valueDependent;
	DeclaratorWalker(const WalkerState& state)
		: WalkerBase(state), id(&gAnonymousId), paramScope(0), valueDependent(context)
	{
	}

	void visit(cpp::ptr_operator* symbol)
	{
		PtrOperatorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::declarator_id* symbol)
	{
		DeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;

		if(walker.getQualifyingScope() != 0
			&& enclosing->type != SCOPETYPE_CLASS) // in 'class C { friend Q::N(X); };' X should be looked up in the scope of C rather than Q
		{
			enclosing = walker.getQualifyingScope(); // names in declarator suffix (array-size, parameter-declaration) are looked up in declarator-id's qualifying scope
		}
	}
	void visit(cpp::declarator_suffix_array* symbol)
	{
		DeclaratorSuffixWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::declarator_suffix_function* symbol)
	{
		DeclaratorSuffixWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		paramScope = walker.paramScope;
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct BaseSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	BaseSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context)
	{
	}

	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_name* symbol)
	{
		/* 10-2
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		TypeNameWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
};

struct ClassHeadWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	TemplateArguments arguments;
	bool isUnion;
	ClassHeadWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), arguments(context), isUnion(false)
	{
	}

	void visit(cpp::class_key* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		isUnion = symbol->id == cpp::class_key::UNION;
	}
	void visit(cpp::identifier* symbol) // class_name
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);

		if(walker.getQualifyingScope() != 0)
		{
			if(enclosing == templateEnclosing)
			{
				templateEnclosing = walker.getQualifyingScope();
			}
			enclosing = walker.getQualifyingScope(); // names in declaration of nested-class are looked up in scope of enclosing class
		}
	}
	void visit(cpp::simple_template_id* symbol) // class_name
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: don't declare anything - this is a template (partial) specialisation
		id = walker.id;
		arguments.swap(walker.arguments);
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until ':' is discovered
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = declareClass(id, arguments);
	}
	void visit(cpp::base_specifier* symbol) 
	{
		BaseSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.type.declaration != 0) // declaration == 0 if base-class is dependent
		{
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			addBase(declaration, walker.type);
		}
	}
};

struct UsingDeclarationWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	bool isTypename;
	UsingDeclarationWalker(const WalkerState& state)
		: WalkerQualified(state), isTypename(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(!isTypename
			&& !isDependent(qualifying_p))
		{
			Declaration* declaration = walker.declaration;
			if(declaration == &gUndeclared
				|| !(isObject(*declaration) || isTypeName(*declaration)))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name or type-name");
			}

			walker.id->dec.p = declaration; // refer to the primary declaration of this name, rather than the one declared by this using-declaration
			 
			declaration = pointOfDeclaration(context, enclosing, *walker.id, declaration->type, isFunction(*declaration) ? declaration->enclosed : 0, declaration->specifiers,
				declaration->isTemplate,
				TEMPLATEARGUMENTS_NULL, // the name in a using-declaration may not have template-arguments ...
				INDEX_INVALID, // ... or be a template-argument
				declaration->valueDependent);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif

		}
		else
		{
			// TODO: introduce typename into enclosing namespace
			walker.id->dec.p = &gDependentType;
		}
	}
	void visit(cpp::terminal<boost::wave::T_TYPENAME> symbol)
	{
		isTypename = true;
	}
};

struct UsingDirectiveWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	UsingDirectiveWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::namespace_name* symbol)
	{
		/* basic.lookup.udir
		When looking up a namespace-name in a using-directive or namespace-alias-definition, only namespace
		names are considered.
		*/
		NamespaceNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(!findScope(enclosing, walker.declaration->enclosed))
		{
			enclosing->usingDirectives.push_back(walker.declaration->enclosed);
		}
	}
};

struct NamespaceAliasDefinitionWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	NamespaceAliasDefinitionWalker(const WalkerState& state)
		: WalkerQualified(state), id(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(id == 0) // first identifier
		{
			id = &symbol->value;
		}
		else // second identifier
		{
			Declaration* declaration = findDeclaration(symbol->value, IsNamespaceName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
			}

			// TODO: check for conflicts with earlier declarations
			declaration = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, declaration->enclosed);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif
			id->dec.p = declaration;
		}
	}
};

struct MemberDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	MemberDeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::member_template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_implicit* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::member_declaration_nested* symbol)
	{
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};


struct ClassSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	TemplateArguments arguments;
	DeferredSymbols deferred;
	bool isUnion;
	ClassSpecifierWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), arguments(context), isUnion(false)
	{
	}

	void visit(cpp::class_head* symbol)
	{
		ClassHeadWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		isUnion = walker.isUnion;
		arguments.swap(walker.arguments);
		enclosing = walker.enclosing;
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until '{' is discovered
		if(declaration == 0)
		{
			// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
			declaration = declareClass(id, arguments);
		}

		/* basic.scope.class-1
		The potential scope of a name declared in a class consists not only of the declarative region following
		the names point of declaration, but also of all function bodies, brace-or-equal-initializers of non-static
		data members, and default arguments in that class (including such things in nested classes).
		*/
		if(declaration->enclosed != 0)
		{
			pushScope(declaration->enclosed);
		}
		templateParams = 0;
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(getState());
		if(WalkerState::deferred == 0)
		{
			walker.deferred = &deferred;
		}
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::terminal<boost::wave::T_RIGHTBRACE> symbol)
	{
		parseDeferred(deferred, *this);
	}
};

struct EnumeratorDefinitionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	EnumeratorDefinitionWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		/* 3.1-4
		The point of declaration for an enumerator is immediately after its enumerator-definition.
		*/
		// TODO: give enumerators a type
		declaration = pointOfDeclaration(context, enclosing, symbol->value, TYPE_ENUMERATOR, 0, DeclSpecifiers());
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		symbol->value.dec.p = declaration;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
};

struct EnumSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	EnumSpecifierWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}

	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer declaration until '{' resolves ambiguity between enum-specifier and elaborated-type-specifier
		if(id != 0)
		{
			declaration = pointOfDeclaration(context, enclosing, *id, TYPE_ENUM, 0);
			id->dec.p = declaration;
		}
	}

	void visit(cpp::enumerator_definition* symbol)
	{
		if(declaration == 0)
		{
			// unnamed enum
			declaration = pointOfDeclaration(context, enclosing, enclosing->getUniqueName(), TYPE_ENUM, 0);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif
		}
		EnumeratorDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ElaboratedTypeSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	DeclarationPtr key;
	Type type;
	IdentifierPtr id;
	ElaboratedTypeSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), key(0), type(0, context), id(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::elaborated_type_specifier_default* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		id = walker.id;

		if(!isUnqualified(symbol)
			|| !isClassKey(*type.declaration))
		{
			id = 0;
		}
	}
	void visit(cpp::elaborated_type_specifier_template* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		id = walker.id;
	}
	void visit(cpp::nested_name_specifier* symbol) // elaborated_type_specifier_default | elaborated_type_specifier_template
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_key* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		key = &gClass;
	}
	void visit(cpp::enum_key* symbol)
	{
		TREEWALKER_LEAF(symbol);
		key = &gEnum;
	}
	void visit(cpp::simple_template_id* symbol) // elaborated_type_specifier_default | elaborated_type_specifier_template
	{
		SEMANTIC_ASSERT(key == &gClass);
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		Declaration* declaration = lookupType(*walker.id, IsTypeName());
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "type-name");
		}
		if(declaration == &gDependentType)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}
		 // 3.4.4-2: when looking up 'identifier' in elaborated-type-specifier, ignore any non-type names that have been declared. 
		walker.id->dec.p = declaration;
		type.declaration = findTemplateSpecialization(declaration, walker.arguments);
		type.templateArguments.swap(walker.arguments);
		type.qualifying.swap(qualifying);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		/* 3.4.4-2
		If the elaborated-type-specifier has no nested-name-specifier ...
		... the identifier is looked up according to 3.4.1 but ignoring any non-type names that have been declared. If
		the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
		declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
		the class-key and this lookup does not find a previously declared type-name ...
		the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
		*/
		id = &symbol->value;
		Declaration* declaration = findDeclaration(symbol->value, IsTypeName());
		if(declaration != &gUndeclared
			&& !isTypedef(*declaration))
		{
			symbol->value.dec.p = declaration;
			/* 7.1.6.3-2
			3.4.4 describes how name lookup proceeds for the identifier in an elaborated-type-specifier. If the identifier
			resolves to a class-name or enum-name, the elaborated-type-specifier introduces it into the declaration the
			same way a simple-type-specifier introduces its type-name. If the identifier resolves to a typedef-name, the
			elaborated-type-specifier is ill-formed.
			*/
#if 0 // allow hiding a typedef with a forward-declaration
			if(isTypedef(*declaration))
			{
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier refers to a typedef" << std::endl;
				printPosition(declaration->getName().position);
				throw SemanticError();
			}
#endif
			/* 7.1.6.3-3
			The class-key or enum keyword present in the elaborated-type-specifier shall agree in kind with the declaration
			to which the name in the elaborated-type-specifier refers.
			*/
			if(declaration->type.declaration != key)
			{
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier key does not match declaration" << std::endl;
				printPosition(declaration->getName().position);
				throw SemanticError();
			}
			type = declaration;
		}
		else
		{
			/* 3.4.4-2
			... If the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
			declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
			the class-key and this lookup does not find a previously declared type-name ...
			the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
			*/
			if(key != &gClass)
			{
				SEMANTIC_ASSERT(key == &gEnum);
				printPosition(symbol->value.position);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier refers to undefined enum" << std::endl;
				throw SemanticError();
			}
			type = key;
		}
	}
};

struct TypenameSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Type type;
	TypenameSpecifierWalker(const WalkerState& state)
		: WalkerQualified(state), type(0, context)
	{
	}

	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		// TODO
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
};

struct DeclSpecifierSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	unsigned fundamental;
	DeclSpecifiers specifiers;
	IdentifierPtr forward;
	bool isUnion;
	bool isTemplateParameter;
	DeclSpecifierSeqWalker(const WalkerState& state, bool isTemplateParameter = false)
		: WalkerBase(state), type(0, context), fundamental(0), forward(0), isUnion(false), isTemplateParameter(isTemplateParameter)
	{
	}

	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		if(type.declaration == 0)
		{
			fundamental = combineFundamental(0, walker.fundamental);
			type = getFundamentalType(fundamental);
		}
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		fundamental = combineFundamental(fundamental, symbol->id);
		type = getFundamentalType(fundamental);
	}
	void visit(cpp::elaborated_type_specifier_template* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		forward = walker.id;
		type.swap(walker.type);
	}
	void visit(cpp::elaborated_type_specifier* symbol)
	{
		ElaboratedTypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		forward = walker.id;
		type.swap(walker.type);
	}
	void visit(cpp::typename_specifier* symbol)
	{
		TypenameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
	}
	void visit(cpp::class_specifier* symbol)
	{
		ClassSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
		isUnion = walker.isUnion;
	}
	void visit(cpp::enum_specifier* symbol)
	{
		EnumSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
	}
	void visit(cpp::decl_specifier_default* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::decl_specifier_default::TYPEDEF)
		{
			specifiers.isTypedef = true;
		}
		else if(symbol->id == cpp::decl_specifier_default::FRIEND)
		{
			specifiers.isFriend = true;
		}
	}
	void visit(cpp::storage_class_specifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::storage_class_specifier::STATIC)
		{
			specifiers.isStatic = true;
		}
		else if(symbol->id == cpp::storage_class_specifier::EXTERN)
		{
			specifiers.isExtern = true;
		}
	}
};

struct TryBlockWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TryBlockWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct LabeledStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	LabeledStatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		// TODO: goto label
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct StatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	StatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
	}
	void visit(cpp::try_block* symbol)
	{
		TryBlockWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::namespace_alias_definition* symbol)
	{
		NamespaceAliasDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::selection_statement* symbol)
	{
		ControlStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::iteration_statement* symbol)
	{
		ControlStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::expression_statement* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::jump_statement_return* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::jump_statement_goto* symbol)
	{
		TREEWALKER_LEAF(symbol);
		// TODO
	}
	void visit(cpp::labeled_statement* symbol)
	{
		LabeledStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_directive* symbol)
	{
		UsingDirectiveWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ControlStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ControlStatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	void visit(cpp::condition_init* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
	}
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct CompoundStatementWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	CompoundStatementWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct HandlerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	HandlerWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_CATCH> symbol)
	{
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
	}
	void visit(cpp::exception_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
	void visit(cpp::compound_statement* symbol)
	{
		CompoundStatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct HandlerSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	HandlerSeqWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::handler* symbol)
	{
		HandlerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct QualifiedTypeNameWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	QualifiedTypeNameWalker(const WalkerState& state)
		: WalkerQualified(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_name* symbol)
	{
		TypeNameWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		if(walker.filter.nonType != 0)
		{
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
	}
};

struct MemInitializerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	MemInitializerWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::mem_initializer_id_base* symbol)
	{
		QualifiedTypeNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		Declaration* declaration = findDeclaration(symbol->value);
		if(declaration == &gUndeclared
			|| !isObject(*declaration))
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "object-name");
		}
		symbol->value.dec.p = declaration;
	}
	void visit(cpp::expression_list* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct MemberDeclaratorBitfieldWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	MemberDeclaratorBitfieldWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct TypeIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	Dependent valueDependent;
	TypeIdWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context), valueDependent(context)
	{
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		declareEts(type, walker.forward);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct IsTemplateName
{
	WalkerState& context;
	IsTemplateName(WalkerState& context) : context(context)
	{
	}
	bool operator()(Identifier& id) const
	{
		Declaration* declaration = context.findDeclaration(id);
		return declaration != &gUndeclared && isTemplateName(*declaration);
	}
};

struct InitializerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	InitializerWalker(const WalkerState& state) : WalkerBase(state)
	{
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct SimpleDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	ScopePtr parent;
	IdentifierPtr id;
	Type type;
	ScopePtr enclosed;
	DeclSpecifiers specifiers;
	IdentifierPtr forward;

	DeferredSymbols deferred;
	DeferredSymbols deferred2;

	Dependent valueDependent;
	size_t templateParameter;
	bool isParameter;
	bool isUnion;

	SimpleDeclarationWalker(const WalkerState& state, bool isParameter = false, size_t templateParameter = INDEX_INVALID) : WalkerBase(state),
		declaration(0),
		parent(0),
		id(0),
		type(&gCtor, context),
		enclosed(0),
		forward(0),
		valueDependent(context),
		templateParameter(templateParameter),
		isParameter(isParameter),
		isUnion(false)
	{
	}

	// commit the declaration to the enclosing scope.
	// invoked when no further ambiguities remain.
	void commit()
	{
		if(id != 0)
		{
			if(enclosed == 0
				&& templateParams != 0)
			{
				templateParams->parent = parent;
				enclosed = templateParams; // for a static-member-variable definition, store template-params with different names than those in the class definition
			}
			declaration = declareObject(parent, id, type, enclosed, specifiers, templateParameter, valueDependent);

			enclosing = parent;

			if(enclosed != 0) // if the declaration has a parameter-declaration-clause
			{
				enclosed->name = declaration->getName();
				enclosing = enclosed; // subsequent declarations are contained by the parameter-scope - see 3.3.2-1: parameter scope
			}
			templateParams = 0;

			id = 0;
		}
#ifdef MINGLE
		if(WalkerState::deferred != 0
			&& !deferred.empty())
		{
			WalkerState::deferred->splice(WalkerState::deferred->end(), deferred);
		}
#endif
	}

	// when parsing parameter-declaration, ensure that state of walker does not persist after failing parameter-declaration-default
	void visit(cpp::parameter_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
	// when parsing parameter-declaration, ensure that state of walker does not persist after failing parameter-declaration-abstract
	void visit(cpp::parameter_declaration_abstract* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
	}
	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState(), templateParameter != INDEX_INVALID);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		specifiers = walker.specifiers;
		forward = walker.forward;
		isUnion = walker.isUnion;
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		forward = walker.forward;
		isUnion = walker.isUnion;
	}

	void visit(cpp::declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		if(isParameter)
		{
			walker.declareEts(type, forward);
		}

		if(WalkerState::deferred != 0)
		{
			// while parsing simple-declaration-named declarator which contains deferred default-argument expression,
			// on finding '{', we rewind and try parsing function-definition.
			// In this situation, 'deferred2' contains the reference to the deferred expression.
			walker.deferred = &deferred2;
		}

		TREEWALKER_WALK_CACHED(walker, symbol);
		parent = walker.enclosing; // if the id-expression in the declarator is a qualified-id, this is the qualifying scope
		id = walker.id;
		enclosed = walker.paramScope;
		/* temp.dep.constexpr
		An identifier is value-dependent if it is:
			 a name declared with a dependent type,
			 the name of a non-type template parameter,
			 a constant with effective literal type and is initialized with an expression that is value-dependent.
		*/
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		if(isParameter)
		{
			walker.declareEts(type, forward);
		}
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		MemberDeclaratorBitfieldWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.id != 0)
		{
			declaration = pointOfDeclaration(context, enclosing, *walker.id, type, 0, specifiers); // 3.3.1.1
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif
			walker.id->dec.p = declaration;
		}
	}
	void visit(cpp::terminal<boost::wave::T_ASSIGN> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_TRY> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_COMMA> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_SEMICOLON> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		commit();
	}

	// handle assignment-expression(s) in initializer
	void visit(cpp::default_parameter* symbol)
	{
		// todo: we cannot skip a default-argument if it contains a template-name that is declared later in the class.
		// Comeau fails in this case too..
		if(WalkerState::deferred != 0
			&& templateParameter == INDEX_INVALID) // don't defer parse of default for non-type template-argument
		{
			result = defer(*WalkerState::deferred, *this, makeSkipDefaultArgument(IsTemplateName(*this)), symbol);
		}
		else
		{
			TREEWALKER_LEAF(symbol);
		}
	}
	// handle assignment-expression(s) in initializer
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	void visit(cpp::initializer_clause* symbol)
	{
		InitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	void visit(cpp::initializer_parenthesis* symbol)
	{
		InitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		addDependent(declaration->valueDependent, walker.valueDependent);
	}

	void visit(cpp::statement_seq_wrapper* symbol)
	{
		// NOTE: we must ensure that symbol-table modifications within the scope of this function are undone on parse fail
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
		if(WalkerState::deferred != 0)
		{
			result = defer(*WalkerState::deferred, *this, skipBraced, symbol);
			if(result == 0)
			{
				return;
			}
		}
		else
		{
			TREEWALKER_LEAF(symbol);
		}
	}
	void visit(cpp::statement* symbol)
	{
		StatementWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::mem_initializer* symbol)
	{
		MemInitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::handler_seq* symbol)
	{
		HandlerSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::mem_initializer_clause* symbol)
	{
		if(WalkerState::deferred != 0)
		{
			result = defer(*WalkerState::deferred, *this, skipMemInitializerClause, symbol);
		}
		else // in case of an out-of-line constructor-definition
		{
			TREEWALKER_LEAF(symbol);
		}
	}

	struct DeclareEtsGuard
	{
		Type* p;
		DeclareEtsGuard(SimpleDeclarationWalker& walker)
		{
			p = walker.declareEts(walker.type, walker.forward) ? &walker.type : 0;
		}
		~DeclareEtsGuard()
		{
			if(p != 0)
			{
				*p = &gClass;
			}
		}
		void hit()
		{
			p = 0;
		}
	};

	void visit(cpp::simple_declaration_named* symbol)
	{
		DeclareEtsGuard guard(*this); // the point of declaration for the ETS in the decl-specifier-seq is just before the declarator
		TREEWALKER_LEAF(symbol);
		guard.hit();
	}
	void visit(cpp::member_declaration_named* symbol)
	{
		DeclareEtsGuard guard(*this);
		TREEWALKER_LEAF(symbol);
		guard.hit();

		// symbols may be deferred during attempt to parse void f(int i = j) {}
		// first parsed as member_declaration_named, fails on reaching '{'
		if(WalkerState::deferred != 0
			&& !deferred2.empty())
		{
			deferred.splice(deferred.end(), deferred2);
		}
	}
	void visit(cpp::function_definition* symbol)
	{
		DeclareEtsGuard guard(*this);
		TREEWALKER_LEAF(symbol);
		guard.hit();

		if(WalkerState::deferred != 0
			&& !deferred2.empty())
		{
			deferred.splice(deferred.end(), deferred2);
		}
	}
	void visit(cpp::type_declaration_suffix* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(forward != 0)
		{
			declaration = pointOfDeclaration(context, enclosing, *forward, TYPE_CLASS, 0, DeclSpecifiers(), enclosing == templateEnclosing);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif
			forward->dec.p = declaration;
			type = declaration;
		}

		if(isUnion
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
				Declaration& member = (*i).second;
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
					Declaration* declaration = ::findDeclaration(enclosing->declarations, member.getName());
					if(declaration != 0)
					{
						printPosition(member.getName().position);
						std::cout << "'" << member.getName().value.c_str() << "': anonymous union member already declared" << std::endl;
						printPosition(declaration->getName().position);
						throw SemanticError();
					}
				}
				member.scope = enclosing;
				Identifier* id = &member.getName();
				id->dec.p = enclosing->declarations.insert(member);
			}
			declaration->enclosed = 0;
		}
	}
};

struct TypeParameterWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	Type argument; // the default argument for this param
	Types arguments; // the default arguments for this param's template-params (if template-template-param)
	size_t templateParameter;
	TypeParameterWalker(const WalkerState& state, size_t templateParameter)
		: WalkerBase(state), declaration(0), argument(0, context), arguments(context), templateParameter(templateParameter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		declaration = pointOfDeclaration(context, enclosing, symbol->value, TYPE_PARAM, 0, DECLSPEC_TYPEDEF, !arguments.empty(), TEMPLATEARGUMENTS_NULL, templateParameter);
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		symbol->value.dec.p = declaration;
		declaration->templateParamDefaults.swap(arguments);
	}
	void visit(cpp::type_id* symbol)
	{
		SEMANTIC_ASSERT(arguments.empty());
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		argument.swap(walker.type);
	}
	void visit(cpp::template_parameter_clause* symbol)
	{
		TemplateParameterClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		arguments.swap(walker.arguments);
	}
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		Declaration* declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			walker.id->dec.p = declaration;
		}
	}
};

struct TemplateParameterListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type param;
	Types params;
	Type argument;
	Types arguments;
	size_t count;
	TemplateParameterListWalker(const WalkerState& state, size_t count)
		: WalkerBase(state), param(0, context), params(context), argument(0, context), arguments(context), count(count)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		TypeParameterWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		param = walker.declaration;
		argument.swap(walker.argument);
		++count;
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		TypeParameterWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		param = walker.declaration;
		argument.swap(walker.argument);
		++count;
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState(), false, count);
		TREEWALKER_WALK(walker, symbol);
		param = walker.declaration;
		// TODO: default value for non-type template-param
		++count;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
		arguments.swap(walker.arguments);
		params.push_front(walker.param);
		arguments.push_front(walker.argument);
	}
};

struct TemplateParameterClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Types params;
	Types arguments;
	TemplateParameterClauseWalker(const WalkerState& state)
		: WalkerBase(state), params(context), arguments(context)
	{
		// collect template-params into a new scope
		Scope* scope = templateParams != 0 ? static_cast<Scope*>(templateParams) : newScope(makeIdentifier("$template"), SCOPETYPE_TEMPLATE);
		templateParams = 0;
		pushScope(scope);
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState(), 0);
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
		arguments.swap(walker.arguments);
		params.push_front(walker.param);
		arguments.push_front(walker.argument);
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	Types params;
	Types arguments;
	TemplateDeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), params(context), arguments(context)
	{
		templateEnclosing = enclosing;
	}
	void visit(cpp::template_parameter_clause* symbol)
	{
		TemplateParameterClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		templateParams = walker.enclosing;
		enclosing = walker.enclosing->parent;
		params.swap(walker.params);
		arguments.swap(walker.arguments);
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		Source source = parser->get_source();
		TREEWALKER_WALK(walker, symbol);
		symbol->source = source;
		declaration = walker.declaration;
		declaration->templateParams.swap(params);
		declaration->templateParamDefaults.swap(arguments);
		SEMANTIC_ASSERT(declaration != 0);
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		declaration->templateParams.swap(params);
		declaration->templateParamDefaults.swap(arguments);
		SEMANTIC_ASSERT(declaration != 0);
	}
};

struct DeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	DeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::linkage_specification* symbol)
	{
		NamespaceWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::namespace_definition* symbol)
	{
		NamespaceWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::namespace_alias_definition* symbol)
	{
		NamespaceAliasDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::general_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	// occurs in for-init-statement
	void visit(cpp::simple_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::constructor_definition* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		declaration = walker.declaration;
	}
	void visit(cpp::template_declaration* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::explicit_specialization* symbol)
	{
		TemplateDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
	}
	void visit(cpp::using_declaration* symbol)
	{
		UsingDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::using_directive* symbol)
	{
		UsingDirectiveWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct NamespaceWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	NamespaceWalker(WalkerContext& context)
		: WalkerBase(context), declaration(0), id(0)
	{
		pushScope(&context.global);
	}

	NamespaceWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		if(id != 0)
		{
			declaration = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, 0);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif
			id->dec.p = declaration;
			if(declaration->enclosed == 0)
			{
				declaration->enclosed = newScope(*id, SCOPETYPE_NAMESPACE);
			}
			pushScope(declaration->enclosed);
		}
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		IncludeEvents events = parser->get_events();
		Source source = parser->get_source();
		TREEWALKER_WALK(walker, symbol);
		symbol->events = events;
		symbol->source = source;
	}
};

};

#if 0
inline cpp::simple_template_id* parseSymbol(ParserGeneric<Walker::TemplateIdWalker>& parser, cpp::simple_template_id* result)
{
	return parseSymbol(parser, result, False());
}
#endif


TreeAllocator<int> getAllocator(ParserContext& context)
{
#ifdef TREEALLOCATOR_LINEAR
	return context.allocator;
#else
	return DebugAllocator<int>();
#endif
}


cpp::declaration_seq* parseFile(ParserContext& context)
{
	gUniqueNames.clear();

	WalkerContext& globals = *new WalkerContext(getAllocator(context));
	Walker::NamespaceWalker& walker = *new Walker::NamespaceWalker(globals);
	ParserGeneric<Walker::NamespaceWalker> parser(context, walker);

	cpp::symbol_sequence<cpp::declaration_seq> result(NULL);
	try
	{
		ProfileScope profile(gProfileParser);
		PARSE_SEQUENCE(parser, result);
	}
	catch(ParseError&)
	{
	}
	catch(SemanticError&)
	{
	}
	if(!context.finished())
	{
		printError(parser);
	}
	dumpProfile(gProfileIo);
	dumpProfile(gProfileWave);
	dumpProfile(gProfileParser);
	dumpProfile(gProfileLookup);
	dumpProfile(gProfileDiagnose);
	dumpProfile(gProfileAllocator);
	dumpProfile(gProfileIdentifier);
	dumpProfile(gProfileTemplateId);

	return result;
}

cpp::statement_seq* parseFunction(ParserContext& context)
{
	WalkerContext& globals = *new WalkerContext(getAllocator(context));
	Walker::CompoundStatementWalker& walker = *new Walker::CompoundStatementWalker(globals);
	ParserGeneric<Walker::CompoundStatementWalker> parser(context, walker);

	cpp::symbol_sequence<cpp::statement_seq> result(NULL);
	try
	{
		ProfileScope profile(gProfileParser);
		PARSE_SEQUENCE(parser, result);
	}
	catch(ParseError&)
	{
	}
	if(!context.finished())
	{
		printError(parser);
	}
	return result;
}


