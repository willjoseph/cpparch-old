

#include "semantic.h"

#include "printer.h"
#include "profiler.h"
#include "util.h"
#include "list.h"
#include "symbols.h"

#include "parser/symbols.h"

#include <fstream>


#include <iostream>
#include <set>

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
		std::cout << getValue((*i).second.name);
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
		std::cout << getValue((*i).declaration->name) << ": ";
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
	Declaration* declaration;
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
		printPosition(e.declaration->name.position);
		std::cout << std::endl;
	}
}



struct WalkerContext : public TreeAllocator<int>
{
	Scope global;
	Declaration globalDecl;
	Type globalType;

	WalkerContext(const TreeAllocator<int>& allocator) :
		TreeAllocator<int>(allocator),
		global(allocator, makeIdentifier("$global"), SCOPETYPE_NAMESPACE),
		globalDecl(allocator, 0, makeIdentifier("$global"), TYPE_NULL, &global),
		globalType(&globalDecl, allocator)
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
	Scope* enclosing;
	const Type* qualifying_p;
	Scope* templateParams;
	Scope* templateEnclosing;
	bool* ambiguity;
	DeferredSymbols* deferred;

	WalkerState(WalkerContext& context)
		: context(context), enclosing(0), qualifying_p(0), templateParams(0), templateEnclosing(0), ambiguity(0), deferred(0)
	{
	}
	const WalkerState& getState() const
	{
		return *this;
	}

	Declaration* findDeclaration(const Identifier& id, LookupFilter filter = isAny)
	{
		ProfileScope profile(gProfileLookup);
#ifdef LOOKUP_DEBUG
		std::cout << "lookup: " << getValue(id) << " (" << getIdentifierType(filter) << ")" << std::endl;
#endif
		if(getQualifyingScope() != 0)
		{
			Declaration* result = ::findDeclaration(*getQualifyingScope(), id, filter);
			if(result != 0)
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
				Declaration* result = ::findDeclaration(*templateParams, id, filter);
				if(result != 0)
				{
#ifdef LOOKUP_DEBUG
					std::cout << "HIT: templateParams" << std::endl;
#endif
					return result;
				}
			}
#endif
			Declaration* result = ::findDeclaration(*enclosing, id, filter);
			if(result != 0)
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
		return &gUndeclared;
	}

	Declaration* pointOfDeclaration(
		const TreeAllocator<int>& allocator,
		Scope* parent,
		const Identifier& name,
		const Type& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateArguments& arguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = DEPENDENT_NULL)
	{
		if(ambiguity != 0)
		{
			*ambiguity = true;
		}

		if(specifiers.isFriend)
		{
			// TODO
			return &gFriend;
		}
		
		Declaration other(allocator, parent, name, type, enclosed, specifiers, isTemplate, arguments, templateParameter, valueDependent);
		if(name.value != 0) // unnamed class/struct/union/enum
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
					std::cout << "'" << name.value << "': " << e.description << std::endl;
					printPosition(declaration->name.position);
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

		return &(*parent->declarations.insert(Scope::Declarations::value_type(name.value, other))).second;
	}

	TreeAllocator<int> getAllocator()
	{
#ifdef TREEALLOCATOR_LINEAR
		return parser->lexer.allocator;
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

	void pushTemplateParams(Scope* scope)
	{
		SEMANTIC_ASSERT(findScope(templateParams, scope) == 0);
		scope->parent = templateParams;
		templateParams = scope;
	}

	void addBase(Declaration* declaration, const Type& base)
	{
		if(getInstantiatedType(base).declaration == declaration)
		{
			return; // TODO: implement template-instantiation, and disallow inheriting from current-instantiation
		}
		declaration->enclosed->bases.push_back(base);
	}

	Scope* getQualifyingScope()
	{
		if(qualifying_p == 0)
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
		if(!IsConcrete<T>::RESULT) // if the grammar-symbol is abstract
		{
			// the symbol was allocated by the parser and must be deallocated
			deleteSymbol(symbol, parser->lexer.allocator);
		}
		result = 0;
		gIdentifierMismatch = IdentifierMismatch(id, declaration, expected);
	}

	Scope* getEtsScope()
	{
		Scope* scope = enclosing;
		for(; !enclosesEts(scope->type); scope = scope->parent)
		{
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
		return ::isDependent(type, DependentContext(*enclosing, templateParams != 0 ? *templateParams : Scope(getAllocator(), IDENTIFIER_NULL)));
	}

	bool isDependent(const Types& bases)
	{
		return ::isDependent(bases, DependentContext(*enclosing, templateParams != 0 ? *templateParams : Scope(getAllocator(), IDENTIFIER_NULL)));
	}

	bool isDependent(const Type* qualifying)
	{
		return ::isDependent(qualifying, DependentContext(*enclosing, templateParams != 0 ? *templateParams : Scope(getAllocator(), IDENTIFIER_NULL)));
	}


	void addDependent(Dependent& dependent, const DependencyCallback& callback)
	{
		dependent.push_back(DependencyNode(callback, context));
	}
	void addDependentName(Dependent& dependent, Declaration* declaration)
	{
		addDependent(dependent, makeDependencyCallback(declaration, isDependentName));
	}
	void addDependentType(Dependent& dependent, Declaration* declaration)
	{
		addDependent(dependent, makeDependencyCallback(&declaration->type, isDependentType));
	}
	void addDependent(Dependent& dependent, Type& type)
	{
		TypeRef tmp(TYPE_NULL, context);
		tmp.back().swap(type);
		addDependent(dependent, makeDependencyCallback(tmp.get(), isDependentType));
		dependent.back().type.swap(tmp);
	}
	void addDependent(Dependent& dependent, Scope* scope)
	{
		addDependent(dependent, makeDependencyCallback(scope, isDependentClass));
	}
};


struct WalkerBase : public WalkerState
{
	typedef WalkerBase Base;

#if 1
	typedef std::list<Scope, TreeAllocator<int> > Scopes;
#else
	typedef List<Scope, TreeAllocator<int> > Scopes;
#endif

	// Contains all elaborated-type-specifiers declared during the current parse.
	// If these declarations have been added to a scope that has already been committed,
	// when the parse fails, these declarations must be removed from their containing scope.
	struct Declarations
	{
		typedef std::list<Declaration*, TreeAllocator<int> > List;

		List declarations;

		typedef List::iterator iterator;
		iterator begin()
		{
			return declarations.begin();
		}
		iterator end()
		{
			return declarations.end();
		}

		Declarations(const TreeAllocator<int>& allocator)
			: declarations(allocator)
		{
		}
	private:
		Declarations(const Declarations&);
		Declarations operator=(const Declarations&);
	public:
		~Declarations()
		{
			for(Declarations::iterator i = declarations.begin(); i != declarations.end(); ++i)
			{
				SEMANTIC_ASSERT(&(*(*i)->scope->declarations.find((*i)->name.value)).second == (*i));

				(*i)->scope->declarations.erase((*i)->name.value);
			}
		}

		void push_back(Declaration* declaration)
		{
			declarations.push_back(declaration);
		}
		void splice(iterator pos, Declarations& other)
		{
			declarations.splice(pos, other.declarations);
		}
		void swap(Declarations& other)
		{
			declarations.swap(other.declarations);
		}
		void erase(iterator first, iterator last)
		{
			declarations.erase(first, last);
		}
	};

#ifdef ALLOCATOR_DEBUG
	Scopes scopes; // under-construction scopes
#endif
	Declarations declarations; // declarations must be destroyed before scopes!

	WalkerBase(WalkerContext& context)
		: WalkerState(context),
#ifdef ALLOCATOR_DEBUG
		scopes(context),
#endif
		declarations(context)
	{
	}
	WalkerBase(const WalkerState& state)
		: WalkerState(state),
#ifdef ALLOCATOR_DEBUG
		scopes(context),
#endif
		declarations(context)
	{
	}
	Scope* newScope(const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
	{
#ifdef ALLOCATOR_DEBUG
		scopes.push_back(Scope(context, name, type));
		return &scopes.back();
#else
		return allocatorNew(context, Scope(context, name, type));
#endif
	}
	void hit(WalkerBase& other)
	{
#ifdef ALLOCATOR_DEBUG
		scopes.splice(scopes.end(), other.scopes);
#endif
		declarations.splice(declarations.end(), other.declarations);
	}

	Declaration* declareClass(Identifier* id, const TemplateArguments& arguments)
	{
		Scope* enclosed = templateParams != 0 ? templateParams : newScope(makeIdentifier("$class"));
		enclosed->type = SCOPETYPE_CLASS; // convert template-param-scope to class-scope if present
		Declaration* declaration = pointOfDeclaration(context, enclosing, id == 0 ? makeIdentifier(enclosing->getUniqueName()) : *id, TYPE_CLASS, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, arguments);
#ifdef ALLOCATOR_DEBUG
		declarations.push_back(declaration);
#endif
		if(id != 0)
		{
			id->dec.p = declaration;
		}
		enclosed->name = declaration->name;
		return declaration;
	}

	Declaration* declareObject(Scope* parent, Identifier* id, const Type& type, Scope* enclosed, DeclSpecifiers specifiers, size_t templateParameter, const Dependent& valueDependent)
	{
		Declaration* declaration = pointOfDeclaration(context, parent, *id, type, enclosed, specifiers, enclosing == templateEnclosing, TEMPLATEARGUMENTS_NULL, templateParameter, valueDependent); // 3.3.1.1
#ifdef ALLOCATOR_DEBUG
		declarations.push_back(declaration);
#endif
		if(id != &gAnonymousId)
		{
			id->dec.p = declaration;
		}
		return declaration;
	}

	void declareEts(Type& type, Identifier* forward)
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
			declarations.push_back(declaration);
			forward->dec.p = declaration;
			type = declaration;
		}
	}
};

// saves and restores the state in walker-base
#if 0 // TODO: this optimisation is broken: fix it!
struct ScopeGuard
{
	WalkerBase& base;
#ifdef ALLOCATOR_DEBUG
	WalkerBase::Scopes::iterator scopes;
#endif
	WalkerBase::Declarations::iterator declarations;
	ScopeGuard(WalkerBase& base)
		: base(base),
#ifdef ALLOCATOR_DEBUG
		scopes(base.scopes.end()),
#endif
		declarations(base.declarations.end())
	{
	}
	~ScopeGuard()
	{
#ifdef ALLOCATOR_DEBUG
		base.scopes.erase(scopes, base.scopes.end());
#endif
		base.declarations.erase(declarations, base.declarations.end());
	}
	// if parse succeeds, append new state to previous state
	void hit()
	{
#ifdef ALLOCATOR_DEBUG
		scopes = base.scopes.end();
#endif
		declarations = base.declarations.end();
	}
};
#else
struct ScopeGuard
{
	WalkerBase& base;
#ifdef ALLOCATOR_DEBUG
	WalkerBase::Scopes scopes;
#endif
	WalkerBase::Declarations declarations;
	ScopeGuard(WalkerBase& base)
		: base(base),
#ifdef ALLOCATOR_DEBUG
		scopes(base.context),
#endif
		declarations(base.context)
	{
#ifdef ALLOCATOR_DEBUG
		scopes.swap(base.scopes);
#endif
		declarations.swap(base.declarations);
	}
	~ScopeGuard()
	{
#ifdef ALLOCATOR_DEBUG
		scopes.swap(base.scopes);
#endif
		declarations.swap(base.declarations);
	}
	// if parse succeeds, append new state to previous state
	void hit()
	{
#ifdef ALLOCATOR_DEBUG
		scopes.splice(scopes.end(), base.scopes);
#endif
		declarations.splice(declarations.end(), base.declarations);
	}
};
#endif

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
		qualifying_p = &context.globalType;
	}

	void swapQualifying(Type& type)
	{
#if 0 // allow incomplete types as qualifying, for nested-name-specifier in ptr-operator (defining member-function-ptr)
		if(type.declaration->enclosed == 0)
		{
			// TODO
			//printPosition(symbol->id->value.position);
			std::cout << "'" << getValue(type.declaration->name) << "' is incomplete, declared here:" << std::endl;
			printPosition(type.declaration->name.position);
			throw SemanticError();
		}
#endif
		Qualifying tmp(TYPE_NULL, context);
		tmp.back().swap(type);
		swapQualifying(tmp);
	}
	void swapQualifying(Qualifying& other)
	{
		qualifying.swap(other);
		qualifying_p = qualifying.get();
	}
};



bool isTypeName(const Declaration& declaration)
{
	return isType(declaration);
}

bool isNamespaceName(const Declaration& declaration)
{
	return isNamespace(declaration);
}

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


template<typename Walker, typename T>
inline T* walkAmbiguity(Walker& walker, cpp::ambiguity<T>* symbol)
{
	semanticBreak();
	bool ambiguousDeclaration = false;
	Walker tmp(walker);
	IdentifierMismatch first;
	IdentifierMismatch second;
	try
	{
		walker.ambiguity = &ambiguousDeclaration;
		symbol->first->accept(walker);
		walker.ambiguity = 0;
		return symbol->first;
	}
	catch(IdentifierMismatch& e)
	{
		SEMANTIC_ASSERT(!ambiguousDeclaration);
		walker.~Walker();
		new(&walker) Walker(tmp);
		first = e;
	}

	try
	{
		walker.ambiguity = &ambiguousDeclaration;
		symbol->second->accept(walker);
		walker.ambiguity = 0;
		return symbol->second;
	}
	catch(IdentifierMismatch& e)
	{
		SEMANTIC_ASSERT(!ambiguousDeclaration);
		walker.~Walker();
		new(&walker) Walker(tmp);
		second = e;
	}
	if(walker.ambiguity == 0)
	{
		std::cout << "first:" << std::endl;
		printIdentifierMismatch(first);
		walker.printScope();
		std::cout << "second:" << std::endl;
		printIdentifierMismatch(second);
		walker.printScope();
		throw SemanticError();
	}
	throw first;
}

#define TREEWALKER_WALK(walker, symbol) SYMBOL_WALK(walker, symbol); hit(walker)
#define TREEWALKER_WALK_NOHIT(walker, symbol) SYMBOL_WALK(walker, symbol)
#define TREEWALKER_LEAF(symbol) SYMBOL_WALK(*this, symbol)
#define TREEWALKER_DEFAULT PARSERCONTEXT_DEFAULT


bool isUnqualified(cpp::elaborated_type_specifier_default* symbol)
{
	return symbol != 0
		&& symbol->isGlobal.value == 0
		&& symbol->context.p == 0;
}

bool isForwardDeclaration(cpp::decl_specifier_seq* symbol)
{
	return symbol != 0
		&& symbol->prefix == 0
		&& symbol->suffix == 0
		&& isUnqualified(dynamic_cast<cpp::elaborated_type_specifier_default*>(symbol->type.p));
}


struct Walker
{


struct NamespaceNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	LookupFilter filter;
	NamespaceNameWalker(const WalkerState& state, LookupFilter filter = isAny)
		: WalkerBase(state), declaration(0), filter(filter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		declaration = findDeclaration(symbol->value, filter);
		if(declaration == &gUndeclared
			|| !isNamespaceName(*declaration))
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
		}
		symbol->value.dec.p = declaration;
	}
};

struct UncheckedTemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	TemplateArguments arguments;
	UncheckedTemplateIdWalker(const WalkerState& state)
		: WalkerBase(state), id(0), arguments(context)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::template_argument_clause* symbol)
	{
		clearQualifying();
		// TODO: store args
		TemplateArgumentListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		arguments.swap(walker.arguments);
	}
};

struct UnqualifiedIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	bool isIdentifier;
	bool isTemplate;
	UnqualifiedIdWalker(const WalkerState& state, bool isTemplate = false)
		: WalkerBase(state), declaration(0), id(0), isIdentifier(false), isTemplate(isTemplate)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
		isIdentifier = true;
		if(!isDependent(qualifying_p))
		{
			declaration = findDeclaration(*id);
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		UncheckedTemplateIdWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
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
			else
			{
				declaration = findTemplateSpecialization(declaration, walker.arguments);
			}
		}
		hit(walker);
	}
	void visit(cpp::template_id_operator_function* symbol)
	{
		UncheckedTemplateIdWalker walker(getState());
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
		TREEWALKER_WALK(*this, symbol);
		id = &symbol->name->value;
	}
};

struct QualifiedIdWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
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
		TREEWALKER_WALK(walker, symbol);
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
	Declaration* declaration;
	Identifier* id;
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
		typeDependent.splice(walker.valueDependent);
	}
	void visit(cpp::new_type* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.type);
		typeDependent.splice(walker.valueDependent);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.valueDependent);
	}
	void visit(cpp::cast_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.valueDependent);
	}
};

struct DependentPrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
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
		TREEWALKER_WALK_NOHIT(walker, symbol);
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
				else
				{
					addDependentType(typeDependent, declaration);
				}
				walker.id->dec.p = declaration;
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
		hit(walker);
	}
	void visit(cpp::primary_expression_parenthesis* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
	}
};

struct DependentPostfixExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
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
		typeDependent.splice(walker.typeDependent);
	}
	void visit(cpp::postfix_expression_call* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		typeDependent.splice(walker.typeDependent);
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
					else
					{
						addDependentType(typeDependent, declaration);
					}
					id->dec.p = declaration;
				}
			}
			else
			{
				id->dec.p = &gDependentObject;
			}
		}
		hit(walker);
	}
};

struct ExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	/* 14.6.2.2-1
	...an expression is type-dependent if any subexpression is type-dependent.
	*/
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWalker(const WalkerState& state)
		: WalkerBase(state), typeDependent(context), valueDependent(context)
	{
	}
	void visit(cpp::postfix_expression_member* symbol)
	{
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: name-lookup for member id-expression
		// TODO: inherit type-dependent property
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
		TREEWALKER_WALK_NOHIT(walker, symbol);
		Declaration* declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name");
			}
			else
			{
				addDependentType(typeDependent, declaration);
				addDependentType(valueDependent, declaration);
				addDependentName(valueDependent, declaration);
			}
			walker.id->dec.p = declaration;
		}
		else
		{
			if(isDependent(walker.qualifying.get()))
			{
				walker.id->dec.p = &gDependentObject;
			}
			if(!walker.qualifying.empty())
			{
				addDependent(typeDependent, walker.qualifying.back());
			}
		}
		hit(walker);
	}
	/* 14.6.2.2-1
	... an expression is type-dependent if any subexpression is type-dependent.
	*/
	void visit(cpp::postfix_expression_disambiguate* symbol)
	{
		// TODO
		/* 14.6.2-1
		In an expression of the form:
		postfix-expression ( expression-list. )
		where the postfix-expression is an unqualified-id but not a template-id, the unqualified-id denotes a dependent
		name if and only if any of the expressions in the expression-list is a type-dependent expression (
		*/
		DependentPostfixExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
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
		typeDependent.splice(walker.typeDependent);
	}
	void visit(cpp::new_expression_placement* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
	}
	void visit(cpp::new_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		typeDependent.splice(walker.typeDependent);
	}
	void visit(cpp::postfix_expression_cast* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(symbol->op->id != cpp::cast_operator::DYNAMIC)
		{
			Dependent tmp(walker.typeDependent);
			valueDependent.splice(tmp);
		}
		typeDependent.splice(walker.typeDependent);
		valueDependent.splice(walker.valueDependent);
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		Dependent tmp(walker.typeDependent);
		valueDependent.splice(tmp);
		typeDependent.splice(walker.typeDependent);
		valueDependent.splice(walker.valueDependent);
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
		valueDependent.splice(walker.typeDependent);
	}
	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.type);
		valueDependent.splice(walker.valueDependent);
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

struct TemplateArgumentListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateArguments arguments;

	TemplateArgumentListWalker(const WalkerState& state)
		: WalkerBase(state), arguments(context)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
#if 0
		TemplateArgument& argument = append(arguments);
		argument.type.swap(walker.type);
		argument.dependent.splice(walker.valueDependent);
#else
		arguments.push_back(TemplateArgument(context));
		arguments.back().type.swap(walker.type);
		arguments.back().dependent.splice(walker.valueDependent);
#endif
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
#if 0
		TemplateArgument& argument = append(arguments);
		argument.dependent.splice(walker.typeDependent);
		argument.dependent.splice(walker.valueDependent);
#else
		arguments.push_back(TemplateArgument(context)); // todo: evaluate constant-expression (unless it's dependent expression)
		arguments.back().dependent = walker.typeDependent;
		arguments.back().dependent.splice(walker.valueDependent);
#endif
	}
};

struct TemplateIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	LookupFilter filter;
	bool isTypename;
	TemplateIdWalker(const WalkerState& state, LookupFilter filter = isAny, bool isTypename = false)
		: WalkerBase(state), type(0, context), filter(filter), isTypename(isTypename)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(!isDependent(qualifying_p))
		{
			Declaration* declaration = findDeclaration(symbol->value, filter);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "template-name");
			}
			type.declaration = declaration;
			symbol->value.dec.p = declaration;
		}
		else if(!isTypename)
		{
			return reportIdentifierMismatch(symbol, symbol->value, &gUndeclared, "typename");
		}
		else
		{
			Declaration* declaration = &gDependentTemplate;
			type.declaration = declaration;
			symbol->value.dec.p = declaration;
		}
	}
	void visit(cpp::template_argument_clause* symbol)
	{
		clearQualifying();

		TemplateArgumentListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);

		if(type.declaration != 0)
		{
			type.declaration = findTemplateSpecialization(type.declaration, walker.arguments);
		}
		type.arguments.swap(walker.arguments);
	}
};

struct TypeNameWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	LookupFilter filter;
	bool isTypename;
	TypeNameWalker(const WalkerState& state, LookupFilter filter = isAny, bool isTypename = false)
		: WalkerBase(state), type(0, context), filter(filter), isTypename(isTypename)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(!isDependent(qualifying_p))
		{
			Declaration* declaration = findDeclaration(symbol->value, filter);
			if(declaration == &gUndeclared
				|| !isTypeName(*declaration))
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "type-name");
			}
			type.declaration = declaration;
			type.isImplicitTemplateId = declaration->isTemplate;
			symbol->value.dec.p = declaration;
 		}
		else if(!isTypename)
		{
			return reportIdentifierMismatch(symbol, symbol->value, &gUndeclared, "typename");
		}
		else
		{
			Declaration* declaration = &gDependentType;
			type.declaration = declaration;
			symbol->value.dec.p = declaration;
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(getState(), filter, isTypename);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
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
		TREEWALKER_LEAF(symbol);
		if(allowDependent
			|| !isDependent(qualifying_p))
		{
			Declaration* declaration = findDeclaration(symbol->value, isNestedName);
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "nested-name");
			}
			else
			{
				type = declaration;
			}
			symbol->value.dec.p = declaration;
		}
		else
		{
			symbol->value.dec.p = &gDependentNested;
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		UncheckedTemplateIdWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		if(!isTemplate
			&& (allowDependent
				|| !isDependent(qualifying_p)))
		{
			Declaration* declaration = findDeclaration(*walker.id, isNestedName);
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "nested-name");
			}
			else
			{
				declaration = findTemplateSpecialization(declaration, walker.arguments);
				type.declaration = declaration;
				type.arguments.swap(walker.arguments);
			}
		}
		hit(walker);
	}
};

struct NestedNameSpecifierPrefixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool allowDependent;
	NestedNameSpecifierPrefixWalker(const WalkerState& state, bool allowDependent = false)
		: WalkerBase(state), type(0, context), allowDependent(allowDependent)
	{
	}
	void visit(cpp::namespace_name* symbol)
	{
		NamespaceNameWalker walker(getState(), isNestedName);
		TREEWALKER_WALK(walker, symbol);
		type = walker.declaration;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState(), isNestedName, true);
		TREEWALKER_WALK(walker, symbol);
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
	void visit(cpp::simple_type_specifier_template* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		fundamental = walker.fundamental;
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
	void visit(cpp::terminal<boost::wave::T_COLON_COLON> symbol)
	{
		setQualifyingGlobal();
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type.swap(walker.type);
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

	Identifier* id;
	UnqualifiedDeclaratorIdWalker(const WalkerState& state)
		: WalkerBase(state), id(&gAnonymousId)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		UncheckedTemplateIdWalker walker(getState());
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

	Identifier* id;
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
		NestedNameSpecifierWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
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

	Identifier* id;
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
		pushScope(newScope(makeIdentifier("$declarator"), SCOPETYPE_PROTOTYPE));
		if(templateParams != 0)
		{
			enclosing->declarations = templateParams->declarations;
			for(Scope::Declarations::iterator i = enclosing->declarations.begin(); i != enclosing->declarations.end(); ++i)
			{
				(*i).second.scope = enclosing;
			}
		}
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
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
};

struct DeclaratorSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Scope* paramScope;
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
		valueDependent.splice(walker.valueDependent);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Identifier* id;
	Scope* paramScope;
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
		TREEWALKER_WALK(walker, symbol);
		valueDependent.splice(walker.valueDependent);
	}
	void visit(cpp::declarator_suffix_function* symbol)
	{
		DeclaratorSuffixWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		paramScope = walker.paramScope;
		valueDependent.splice(walker.valueDependent);
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
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_name* symbol)
	{
		/* 10-2
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		TypeNameWalker walker(getState(), isTypeName, true);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
};

struct ClassHeadWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
	TemplateArguments arguments;
	bool isUnion;
	ClassHeadWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), arguments(context), isUnion(false)
	{
	}

	void visit(cpp::class_key* symbol)
	{
		TREEWALKER_WALK(*this, symbol);
		isUnion = symbol->id == cpp::class_key::UNION;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);

		if(walker.getQualifyingScope() != 0)
		{
			if(enclosing == templateEnclosing)
			{
				templateEnclosing = walker.getQualifyingScope();
			}
			enclosing = walker.getQualifyingScope(); // names in declaration of nested-class are looked up in scope of enclosing class
		}
	}
	void visit(cpp::simple_template_id* symbol) 
	{
		UncheckedTemplateIdWalker walker(getState());
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
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedIdWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		if(!isTypename
			&& !isDependent(qualifying_p))
		{
			Declaration* declaration = walker.declaration;
			if(declaration == &gUndeclared
				|| !(isObject(*declaration) || isTypeName(*declaration)))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name or type-name");
			}
			else
			{
				// TODO: check for conflicts with earlier declarations
				enclosing->declarations.insert(Scope::Declarations::value_type(declaration->name.value, *declaration));
			}
			walker.id->dec.p = declaration;
		}
		else
		{
			// TODO: introduce typename into enclosing namespace
			walker.id->dec.p = &gDependentType;
		}
		hit(walker);
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
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::namespace_name* symbol)
	{
		/* basic.lookup.udir
		When looking up a namespace-name in a using-directive or namespace-alias-definition, only namespace
		names are considered.
		*/
		NamespaceNameWalker walker(getState(), isNamespaceName);
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

	Identifier* id;
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
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(id == 0)
		{
			id = &symbol->value;
		}
		else
		{
			Declaration* declaration = findDeclaration(symbol->value, isNamespace);
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
			}
			else
			{
				// TODO: check for conflicts with earlier declarations
				declaration = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, declaration->enclosed);
#ifdef ALLOCATOR_DEBUG
				declarations.push_back(declaration);
#endif
			}
			id->dec.p = declaration;
		}
	}
};

struct MemberDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
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

	Declaration* declaration;
	Identifier* id;
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

	Declaration* declaration;
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
		declarations.push_back(declaration);
#endif
		symbol->value.dec.p = declaration;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration->valueDependent.splice(walker.valueDependent);
	}
};

struct EnumSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Identifier* id;
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
			declaration = pointOfDeclaration(context, enclosing, makeIdentifier(enclosing->getUniqueName()), TYPE_ENUM, 0);
#ifdef ALLOCATOR_DEBUG
			declarations.push_back(declaration);
#endif
		}
		EnumeratorDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
};

struct ElaboratedTypeSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	Declaration* key;
	Type type;
	Identifier* id;
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
	void visit(cpp::nested_name_specifier* symbol)
	{
		NestedNameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_key* symbol)
	{
		TREEWALKER_LEAF(symbol);
		key = &gClass;
	}
	void visit(cpp::enum_key* symbol)
	{
		TREEWALKER_LEAF(symbol);
		key = &gEnum;
	}
	void visit(cpp::simple_template_id* symbol)
	{
		SEMANTIC_ASSERT(key == &gClass);
		TemplateIdWalker walker(getState(), isType);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		/* 3.4.4-2
		If the elaborated-type-specifier has no nested-name-specifier ...
		... the identifier is looked up according to 3.4.1 but ignoring any non-type names that have been declared. If
		the elaborated-type-specifier is introduced by the enum keyword and this lookup does not find a previously
		declared type-name, the elaborated-type-specifier is ill-formed. If the elaborated-type-specifier is introduced by
		the class-key and this lookup does not find a previously declared type-name ...
		the elaborated-type-specifier is a declaration that introduces the class-name as described in 3.3.1.
		*/
		id = &symbol->value;
		Declaration* declaration = findDeclaration(symbol->value, isType);
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
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier refers to a typedef" << std::endl;
				printPosition(declaration->name.position);
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
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier key does not match declaration" << std::endl;
				printPosition(declaration->name.position);
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
				std::cout << "'" << symbol->value.value << "': elaborated-type-specifier refers to undefined enum" << std::endl;
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
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::type_name* symbol)
	{
		TypeNameWalker walker(getState(), isAny, true);
		TREEWALKER_WALK(walker, symbol);
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
	Identifier* forward;
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
		pushScope(newScope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
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
		pushScope(newScope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
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
		pushScope(newScope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
	}
	void visit(cpp::exception_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
		walker.commit();
		hit(walker);
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
		TREEWALKER_WALK(walker, symbol);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::class_name* symbol)
	{
		TypeNameWalker walker(getState(), isAny, true);
		TREEWALKER_WALK(walker, symbol);
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

	Identifier* id;
	MemberDeclaratorBitfieldWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
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
		valueDependent.splice(walker.valueDependent);
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

	Declaration* declaration;
	Scope* parent;
	Identifier* id;
	Type type;
	Scope* enclosed;
	DeclSpecifiers specifiers;
	Identifier* forward;

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

			if(enclosed != 0)
			{
				enclosed->name = declaration->name;
				enclosing = enclosed; // 3.3.2.1 parameter scope
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
		TREEWALKER_WALK_NOHIT(walker, symbol);
		walker.commit();
		hit(walker);
	}
	// when parsing parameter-declaration, ensure that state of walker does not persist after failing parameter-declaration-abstract
	void visit(cpp::parameter_declaration_abstract* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK_NOHIT(walker, symbol);
		walker.commit();
		hit(walker);
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

	// not required for mingled parse 
	void visit(cpp::init_declarator_disambiguate* symbol)
	{
		result = 0;
	}

	template<typename T>
	void walkDeclarator(T* symbol)
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

		TREEWALKER_WALK(walker, symbol);
		parent = walker.enclosing;
		id = walker.id;
		enclosed = walker.paramScope;
		/* temp.dep.constexpr
		An identifier is value-dependent if it is:
			 a name declared with a dependent type,
			 the name of a non-type template parameter,
			 a constant with effective literal type and is initialized with an expression that is value-dependent.
		*/
		valueDependent.splice(walker.valueDependent);
	}
	void visit(cpp::declarator* symbol)
	{
		walkDeclarator(symbol);
	}
	void visit(cpp::declarator_disambiguate* symbol)
	{
		walkDeclarator(symbol);
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
			declarations.push_back(declaration);
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
			TREEWALKER_WALK(*this, symbol);
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
		declaration->valueDependent.splice(walker.valueDependent);
	}

	void visit(cpp::statement_seq_wrapper* symbol)
	{
		ScopeGuard guard(*this); // ensure that symbol-table modifications within the scope of 'guard' are undone on parse fail
		pushScope(newScope(makeIdentifier(enclosing->getUniqueName()), SCOPETYPE_LOCAL));
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
			TREEWALKER_WALK(*this, symbol);
		}
		guard.hit();
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
			TREEWALKER_WALK(*this, symbol);
		}
	}

	void visit(cpp::simple_declaration_named* symbol)
	{
		ScopeGuard guard(*this);
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
		guard.hit();
	}
	void visit(cpp::member_declaration_named* symbol)
	{
		ScopeGuard guard(*this);
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
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
		ScopeGuard guard(*this);
		declareEts(type, forward);
		TREEWALKER_WALK(*this, symbol);
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
			declarations.push_back(declaration);
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
					const Identifier& name = makeIdentifier(enclosing->getUniqueName());
					member.name = name;
					if(member.enclosed != 0)
					{
						member.enclosed->name = name;
					}
				}
				else
				{
					Declaration* declaration = ::findDeclaration(enclosing->declarations, member.name);
					if(declaration != 0)
					{
						printPosition(member.name.position);
						std::cout << "'" << member.name.value << "': anonymous union member already declared" << std::endl;
						printPosition(declaration->name.position);
						throw SemanticError();
					}
				}
				member.scope = enclosing;
				enclosing->declarations.insert(*i);
			}

		}
	}
};

struct TypeParameterWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	Type argument; // the default argument for this param
	DeclarationList params;
	Types arguments; // the default arguments for this param's template-params (if template-template-param)
	size_t templateParameter;
	TypeParameterWalker(const WalkerState& state, size_t templateParameter)
		: WalkerBase(state), declaration(0), argument(0, context), params(context), arguments(context), templateParameter(templateParameter)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		declaration = pointOfDeclaration(context, enclosing, symbol->value, TYPE_PARAM, 0, DECLSPEC_TYPEDEF, !arguments.empty(), TEMPLATEARGUMENTS_NULL, templateParameter);
#ifdef ALLOCATOR_DEBUG
		declarations.push_back(declaration);
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
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		arguments.swap(walker.arguments);
	}
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK_NOHIT(walker, symbol);
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
		hit(walker);
	}
};

struct TemplateParameterListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationList params;
	Types arguments;
	TemplateParameterListWalker(const WalkerState& state)
		: WalkerBase(state), params(context), arguments(context)
	{
		// collect template-params into a new scope
		Scope* scope = templateParams != 0 ? templateParams : newScope(makeIdentifier("$template"), SCOPETYPE_TEMPLATE);
		templateParams = 0;
		pushScope(scope);
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		TypeParameterWalker walker(getState(), params.size());
		TREEWALKER_WALK(walker, symbol);
		params.push_back(walker.declaration);
		arguments.push_back(Type(0, context));
		arguments.back().swap(walker.argument);
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		TypeParameterWalker walker(getState(), params.size());
		TREEWALKER_WALK(walker, symbol);
		params.push_back(walker.declaration);
		arguments.push_back(Type(0, context));
		arguments.back().swap(walker.argument);
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		SimpleDeclarationWalker walker(getState(), false, params.size());
		TREEWALKER_WALK(walker, symbol);
		params.push_back(walker.declaration);
		arguments.push_back(Type(0, context)); // TODO: default value for non-type template-param
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Declaration* declaration;
	DeclarationList params;
	Types arguments;
	TemplateDeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), params(context), arguments(context)
	{
		templateEnclosing = enclosing;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		templateParams = walker.enclosing;
		enclosing = walker.enclosing->parent;
		params.swap(walker.params);
		arguments.swap(walker.arguments);
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		const char* source = parser->get_source();
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

	Declaration* declaration;
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

	Declaration* declaration;
	Identifier* id;
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
			declarations.push_back(declaration);
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
		const char* source = parser->get_source();
		TREEWALKER_WALK(walker, symbol);
		symbol->events = events;
		symbol->source = source;
	}
};

};



template<typename OutputStreamType>
struct TreePrinter // TODO: better name
{
	OutputStreamType& out;
	bool visited;
	TreePrinter(OutputStreamType& out)
		: out(out), visited(false)
	{
	}

	void visit(cpp::terminal_identifier symbol)
	{
	}

	void visit(cpp::terminal_string symbol)
	{
	}

	void visit(cpp::terminal_choice2 symbol)
	{
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
	}

	template<typename T>
	void visit(T* symbol)
	{
		if(typeid(T) != typeid(*symbol)) // if abstract
		{
			TREEWALKER_WALK(*this, symbol);
		}
		else
		{
			if(visited)
			{
				out << ", ";
			}
			visited = true;
			out << SYMBOL_NAME(*symbol);
#if 0 // don't print children
			out << '(';
			TreePrinter<OutputStreamType> tmp(out);
			symbol->accept(tmp);
			out << ')';
#endif
		}
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}

	void visit(cpp::declaration* symbol)
	{
	}

	void visit(cpp::member_declaration* symbol)
	{
	}

	void visit(cpp::statement* symbol)
	{
	}
};

typedef TokenPrinter<std::ofstream> FileTokenPrinter;

struct PrintingWalker
{
	FileTokenPrinter& printer;
	PrintingWalker(FileTokenPrinter& printer)
		: printer(printer)
	{
	}
	template<typename T>
	void printSymbol(T* symbol)
	{
		SymbolPrinter walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}

	void printName(Scope* scope)
	{
		if(scope != 0
			&& scope->parent != 0)
		{
			printName(scope->parent);
			printer.out << getValue(scope->name) << ".";
		}
	}

	void printName(Declaration* name)
	{
		if(name == 0)
		{
			printer.out << "<unknown>";
		}
		else
		{
			printName(name->scope);
			printer.out << getValue(name->name);
		}
	}

	void printName(const char* caption, Declaration* type, Declaration* name)
	{
		printer.out << "/* ";
		printer.out << caption;
		printName(type);
		printer.out << ": ";
		printName(name);
		printer.out << " */";
	}

};

const char* escapeTerminal(LexTokenId id, const char* value)
{
	switch(id)
	{
	case boost::wave::T_LESS: return "&lt;";
	case boost::wave::T_LESSEQUAL: return "&lt;=";
	case boost::wave::T_SHIFTLEFT: return "&lt;&lt;";
	case boost::wave::T_SHIFTLEFTASSIGN: return "&lt;&lt;=";
	case boost::wave::T_AND_ALT:
	case boost::wave::T_AND: return "&amp;";
	case boost::wave::T_ANDAND_ALT:
	case boost::wave::T_ANDAND: return "&amp;&amp;";
	case boost::wave::T_ANDASSIGN_ALT:
	case boost::wave::T_ANDASSIGN: return "&amp;=";
	default: break;
	}

	return value;
}

template<LexTokenId id> 
const char* escapeTerminal(cpp::terminal<id> symbol)
{
	return escapeTerminal(id, symbol.value);
}


const char* escapeTerminal(cpp::terminal_choice2 symbol)
{
	return escapeTerminal(symbol.id, symbol.value);
}


bool isPrimary(const Identifier& id)
{
	// TODO: optimise
	return id.dec.p != 0 && id.position == id.dec.p->name.position;
}

typedef std::pair<const char*, Declaration*> ModuleDeclaration; // first=source, second=declaration

typedef std::set<ModuleDeclaration> DeclarationSet;

typedef std::map<const char*, DeclarationSet> ModuleDependencyMap; // key=source

// recursively merges all source files in the \p graph into \p includes
void mergeIncludes(IncludeDependencyNodes& includes, const IncludeDependencyNode& graph)
{
	if(graph.empty())
	{
		return;
	}
	IncludeDependencyNodes tmp;

	std::set_union(
		includes.begin(), includes.end(),
		graph.begin(), graph.end(),
		std::inserter(tmp, tmp.begin())
	);

	for(IncludeDependencyNode::const_iterator i = graph.begin(); i != graph.end(); ++i)
	{
		mergeIncludes(tmp, *(*i));
	}

	includes.swap(tmp);
}

struct SymbolPrinter : PrintingWalker
{
	std::ofstream out;
	FileTokenPrinter printer;
	const char* root;
	const IncludeDependencyGraph& includeGraph;
	ModuleDependencyMap moduleDependencies;

	SymbolPrinter(const PrintSymbolArgs& args)
		: PrintingWalker(printer),
		printer(out),
		root(args.path),
		includeGraph(args.includeGraph)
	{
		includes.push("$outer");
		open(includes.top());
	}
	~SymbolPrinter()
	{
		while(!includes.empty())
		{
			pop();
		}
		SEMANTIC_ASSERT(!out.is_open());
	}

	void open(const char* path)
	{
		SEMANTIC_ASSERT(!out.is_open());
		out.open(OutPath(root, path).c_str());

		out << "<html>\n"
			"<head>\n"
			"<link rel='stylesheet' type='text/css' href='identifier.css'/>\n"
			"</style>\n"
			"</head>\n"
			"<body>\n"
			"<pre style='color:#000000;background:#ffffff;'>\n";
	}
	void close()
	{
		SEMANTIC_ASSERT(out.is_open());

		out << "</pre>\n"
			"</body>\n"
			"</html>\n";

		out.close();
	}
	void suspend()
	{
		SEMANTIC_ASSERT(out.is_open());
		out.close();
	}
	void resume(const char* path)
	{
		SEMANTIC_ASSERT(!out.is_open());
		out.open(OutPath(root, path).c_str(), std::ios::app);
	}

	void visit(cpp::terminal_identifier symbol)
	{
		printer.printToken(boost::wave::T_IDENTIFIER, symbol.value);
	}

	void visit(cpp::terminal_string symbol)
	{
#if 1
		printer.formatToken(boost::wave::T_STRINGLIT);
		for(const char* p = symbol.value; *p != '\0'; ++p)
		{
			char c = *p;
			switch(c)
			{
			case '"': printer.out << "&quot;"; break;
			case '&': printer.out << "&amp;"; break;
			case '<': printer.out << "&lt;"; break;
			case '>': printer.out << "&gt;"; break;
			default: printer.out << c; break;
			}
		}
#else
		printer.printToken(boost::wave::T_STRINGLIT, symbol.value);
#endif
	}

	void visit(cpp::terminal_choice2 symbol)
	{
		printer.printToken(symbol.id, escapeTerminal(symbol));
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
		if(symbol.value != 0)
		{
			printer.printToken(id, escapeTerminal(symbol));
		}
	}

	template<typename T>
	void visit(T* symbol)
	{
		symbol->accept(*this);
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}

	template<typename T>
	void visit(cpp::ambiguity<T>* symbol)
	{
		SEMANTIC_ASSERT(symbol->second == 0);
		visit(symbol->first);
	}

	typedef StringStack Includes;

	Includes includes;

	void push()
	{
		if(includes.top() != 0)
		{
			suspend();
		}
		includes.push(0);
	}

	bool isIncluded(const IncludeDependencyNodes& included, const char* source)
	{
		IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(source);

		return i != includeGraph.includes.end() // should be impossible?
			&& included.find(&(*i)) != included.end();
	}

	bool isIncluded(const IncludeDependencyNodes& included, Declaration* declaration)
	{
		for(; declaration != 0; declaration = declaration->overloaded)
		{
			if(*declaration->name.source == '$' // '$outer'
				|| isIncluded(included, declaration->name.source))
			{
				return true;
			}
		}
		return false;
	}

	void pop()
	{
		if(includes.top() != 0)
		{
			close();

			out.open(Concatenate(makeRange(root), makeRange(findFilenameSafe(includes.top())), makeRange(".d")).c_str());

			bool warnings = false;

			bool isHeader = !string_equal_nocase(findExtension(includes.top()), ".inl");

			IncludeDependencyNodes included;
			{
				IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(includes.top());
				if(i != includeGraph.includes.end())
				{
					mergeIncludes(included, *i);
#if 0
					for(IncludeDependencyNodes::const_iterator i = included.begin(); i != included.end(); ++i)
					{
						printer.out << (*i)->name << std::endl;
					}
#endif
				}
			}
			{
				ModuleDependencyMap::const_iterator i = moduleDependencies.find(includes.top());
				if(i != moduleDependencies.end())
				{
					const DeclarationSet& d = (*i).second;
					for(DeclarationSet::const_iterator i = d.begin(); i != d.end(); ++i)
					{
						const ModuleDeclaration& declaration = *i;

						printer.out << (declaration.first != 0 ? declaration.first : "<unknown>") << ": ";
						printName(declaration.second);
						printer.out << std::endl;

						if(declaration.first != 0
							&& declaration.second != 0
							&& isHeader
							&& !isIncluded(included, declaration.second))
						{
							printer.out << "WARNING: depending on file that was not (in)directly included: " << declaration.first << std::endl;
							warnings = true;
						}
					}
				}
			}
			{
				MacroDependencyMap::const_iterator i = includeGraph.macros.find(includes.top());
				if(i != includeGraph.macros.end())
				{
					const MacroDeclarationSet& d = (*i).second;
					for(MacroDeclarationSet::const_iterator i = d.begin(); i != d.end(); ++i)
					{
						const MacroDeclaration& declaration = *i;
						printer.out << declaration.first << ": " << declaration.second << std::endl;

						if(*declaration.first != '<' // <command line>
							&& isHeader
							&& !string_equal(declaration.second, "NULL")// TEMP HACK
							&& !isIncluded(included, declaration.first)) 
						{
							printer.out << "WARNING: depending on file that was not (in)directly included: " << declaration.first << std::endl;
							warnings = true;
						}
					}
				}
			}
			out.close();

			if(warnings)
			{
				std::cout << "warnings found: " << includes.top() << std::endl;
			}
		}
		includes.pop();
		if(includes.top() != 0)
		{
			resume(includes.top());
		}
	}

	struct OutPath : public Concatenate
	{
		OutPath(const char* root, const char* path)
			: Concatenate(makeRange(root), makeRange(findFilenameSafe(path)), makeRange(".html"))
		{
		}
	};

	void visit(cpp::declaration* symbol)
	{
		for(unsigned short i = 0; i != symbol->events.pop; ++i)
		{
			pop();
		}
		for(unsigned short i = 0; i != symbol->events.push; ++i)
		{
			push();
		}
		if(includes.top() == 0)
		{
			includes.top() = symbol->source;
			open(includes.top());

			{
				IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(symbol->source);
				if(i != includeGraph.includes.end())
				{
					const IncludeDependencyNode& d = *i;
					for(IncludeDependencyNode::const_iterator i = d.begin(); i != d.end(); ++i)
					{
						printer.out << "<a href='" << OutPath("", (*i)->name).c_str() << "'>" << (*i)->name << "</a>" << std::endl;
					}
				}
			}

		}

		symbol->accept(*this);
	}
#if 0
	void visit(cpp::declaration* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}

	void visit(cpp::member_declaration* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}

	void visit(cpp::statement* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}
#endif

	void visit(cpp::identifier* symbol)
	{
		printer.formatToken(boost::wave::T_IDENTIFIER);

		const char* type = symbol->value.dec.p != 0 ? getDeclarationType(*symbol->value.dec.p) : "unknown";
		if(isPrimary(symbol->value))
		{
			printer.out << "<a name='";
			printName(symbol->value.dec.p);
			printer.out << "'></a>";
		}
		else
		{
			printer.out << "<a href='";
			if(symbol->value.dec.p != 0)
			{
				printer.out << OutPath(root, symbol->value.dec.p->name.source).c_str() + 4; // HACK! Remove 'out/' from path
			}
			printer.out << "#";
			printName(symbol->value.dec.p);
			printer.out << "'>";

			if(symbol->value.dec.p != 0
				&& !isNamespace(*symbol->value.dec.p)
				&& symbol->value.dec.p->name.source != 0 // refers to a symbol declared in a module
				&& symbol->value.source != symbol->value.dec.p->name.source) // refers to a symbol not declared in the current module
			{
				moduleDependencies[symbol->value.source].insert(ModuleDeclaration(symbol->value.dec.p->name.source, symbol->value.dec.p));
			}
		}
		printer.out << "<" << type << ">";
		printer.out << getValue(symbol->value);
		printer.out << "</" << type << ">";
		if(!isPrimary(symbol->value))
		{
			printer.out << "</a>";
		}
	}

	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		printer.formatToken(symbol->value.id);

		printer.out << "<type>";
		printer.out << symbol->value.value;
		printer.out << "</type>";
	}
	void visit(cpp::primary_expression_builtin* symbol)
	{
		printer.formatToken(symbol->key.ID);

		printer.out << "<object>";
		printer.out << symbol->key.value;
		printer.out << "</object>";
	}
};




void printSymbol(cpp::declaration_seq* p, const PrintSymbolArgs& args)
{
	try
	{
		SymbolPrinter printer(args);
		printer.visit(makeSymbol(p));
	}
	catch(SemanticError&)
	{
	}
}

void printSymbol(cpp::statement_seq* p, const PrintSymbolArgs& args)
{
	try
	{
		SymbolPrinter printer(args);
		printer.visit(makeSymbol(p));
	}
	catch(SemanticError&)
	{
	}

}


TreeAllocator<int> getAllocator(ParserContext& lexer)
{
#ifdef TREEALLOCATOR_LINEAR
	return lexer.allocator;
#else
	return DebugAllocator<int>();
#endif
}


cpp::declaration_seq* parseFile(ParserContext& lexer)
{
	WalkerContext& context = *new WalkerContext(getAllocator(lexer));
	Walker::NamespaceWalker& walker = *new Walker::NamespaceWalker(context);
	ParserGeneric<Walker::NamespaceWalker> parser(lexer, walker);

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
	if(!lexer.finished())
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

cpp::statement_seq* parseFunction(ParserContext& lexer)
{
	WalkerContext& context = *new WalkerContext(getAllocator(lexer));
	Walker::CompoundStatementWalker& walker = *new Walker::CompoundStatementWalker(context);
	ParserGeneric<Walker::CompoundStatementWalker> parser(lexer, walker);

	cpp::symbol_sequence<cpp::statement_seq> result(NULL);
	try
	{
		ProfileScope profile(gProfileParser);
		PARSE_SEQUENCE(parser, result);
	}
	catch(ParseError&)
	{
	}
	if(!lexer.finished())
	{
		printError(parser);
	}
	return result;
}


