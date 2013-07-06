

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
#ifdef WIN32
		__debugbreak();
#endif
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
		std::cout << getValue((*i).second->getName());
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
	printPosition(e.id.source);
	std::cout << "'" << getValue(e.id) << "' expected " << e.expected << ", " << (e.declaration == &gUndeclared ? "was undeclared" : "was declared here:") << std::endl;
	if(e.declaration != &gUndeclared)
	{
		printPosition(e.declaration->getName().source);
		std::cout << std::endl;
	}
}


inline void setDecoration(Identifier* id, const DeclarationInstance& declaration)
{
	SEMANTIC_ASSERT(declaration.name != 0);
	id->dec.p = &declaration;
}

inline bool isReturnTypeEqual(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	SYMBOLS_ASSERT(left.isFunction());
	SYMBOLS_ASSERT(right.isFunction());
	return isEqualInner(left, right);
}

inline bool isEquivalentSpecialization(const Declaration& declaration, const Declaration& other)
{
	return !(isComplete(declaration) && isComplete(other)) // if both are complete, assume that they have different argument lists!
		&& matchTemplateSpecialization(declaration, other.templateArguments);
}

inline bool isEquivalentTypedef(const Declaration& declaration, const Declaration& other)
{
	return getType(declaration) == getType(other);
}

inline bool isEquivalent(const Declaration& declaration, const Declaration& other)
{
	if(isClass(declaration)
		&& isClass(other))
	{
		// TODO: compare template-argument-lists of partial specializations
		return isSpecialization(declaration) == isSpecialization(other)
			&& (!isSpecialization(declaration) // both are not explicit/partial specializations
			|| isEquivalentSpecialization(declaration, other)); // both are specializations and have matching arguments

	}

	if(isEnum(declaration)
		|| isEnum(other))
	{
		return isEquivalentTypedef(declaration, other);
	}

	if(isClass(declaration)
		|| isClass(other))
	{
		return isEquivalentTypedef(declaration, other);
	}

	{
		SEMANTIC_ASSERT(declaration.type.unique != 0);
		SEMANTIC_ASSERT(other.type.unique != 0);
		UniqueTypeWrapper l(declaration.type.unique);
		UniqueTypeWrapper r(other.type.unique);
		if(l.isFunction())
		{
			// 13.2 [over.dcl] Two functions of the same name refer to the same function
			// if they are in the same scope and have equivalent parameter declarations.
			// TODO: also compare template parameter lists: <class, int> is not equivalent to <class, float>
			SYMBOLS_ASSERT(r.isFunction()); // TODO: non-fatal error: 'id' previously declared as non-function, second declaration is a function
			return declaration.isTemplate == other.isTemplate // early out
				// [over.load] Function declarations that differ only in the return type cannot be overloaded.
				//&& isReturnTypeEqual(l, r) // return-types match (this cannot be determined for dependent return types, and is not necessary to distinguish overloads)
				&& isEquivalent(getParameterTypes(l.value), getParameterTypes(r.value)); // and parameter-types match
		}
		return true; // redeclaring an object (cannot be overloaded)
	}
	return false;
}

inline const DeclarationInstance* findRedeclared(const Declaration& declaration, const DeclarationInstance* overloaded)
{
	for(const DeclarationInstance* p = overloaded; p != 0; p = p->overloaded)
	{
		if(isEquivalent(declaration, *(*p)))
		{
			return p;
		}
	}
	return 0;
}


Identifier gGlobalId = makeIdentifier("$global");


struct WalkerContext : public TreeAllocator<int>
{
	Scope global;
	Declaration globalDecl;
	TypeRef globalType;
	std::size_t declarationCount;

	WalkerContext(const TreeAllocator<int>& allocator) :
		TreeAllocator<int>(allocator),
		global(allocator, gGlobalId, SCOPETYPE_NAMESPACE),
		globalDecl(allocator, 0, gGlobalId, TYPE_NULL, &global),
		globalType(Type(&globalDecl, allocator), allocator),
		declarationCount(0)
	{
	}
};

typedef std::list< DeferredParse<struct WalkerBase, struct WalkerState> > DeferredSymbolsList;

struct DeferredSymbols
{
	DeferredSymbolsList first;
	DeferredSymbolsList second;

	void splice(DeferredSymbols& other)
	{
		first.splice(first.end(), other.first);
		second.splice(second.end(), other.second);
	}
	bool empty() const
	{
		return first.empty() && second.empty();
	}
};


typedef bool (*IdentifierFunc)(const Declaration& declaration);
const char* getIdentifierType(IdentifierFunc func);


struct WalkerState
	: public ContextBase
{
	typedef WalkerState State;

	WalkerContext& context;
	ScopePtr enclosing;
	const TypeInstance* enclosingType;
	Dependent enclosingDependent;
	TypePtr qualifying_p;
	DeclarationPtr qualifyingScope;
	const TypeInstance* qualifyingType;
	ScopePtr memberObject;
	const TypeInstance* memberType;
	SafePtr<TemplateParameters> templateParams;
	ScopePtr templateParamScope;
	ScopePtr templateEnclosing;
	DeferredSymbols* deferred;
	std::size_t templateDepth;
	bool isExplicitInstantiation;

	WalkerState(WalkerContext& context)
		: context(context)
		, enclosing(0)
		, enclosingType(0)
		, qualifying_p(0)
		, qualifyingScope(0)
		, qualifyingType(0)
		, memberObject(0)
		, memberType(0)
		, templateParams(0)
		, templateParamScope(0)
		, templateEnclosing(0)
		, deferred(0)
		, templateDepth(0)
		, isExplicitInstantiation(false)
	{
	}
	const WalkerState& getState() const
	{ 
		return *this;
	}
	Location getLocation() const
	{
		return Location(parser->get_source(), context.declarationCount);
	}

	LookupResult findDeclaration(const Identifier& id, bool isDeclarator, LookupFilter filter = IsAny())
	{
		return isDeclarator
			? findDeclaratorDeclaration(id, filter)
			: findDeclaration(id, filter);
	}
	LookupResult findDeclaratorDeclaration(const Identifier& id, LookupFilter filter = IsAny())
	{
		LookupResult result;
		if(result.append(::findDeclaration(*getQualifyingScope(), id, filter)))
		{
			return result;
		}
		result.filtered = &gUndeclaredInstance;
		return result;
	}
	LookupResult findDeclaration(const Identifier& id, LookupFilter filter = IsAny(), bool isUnqualifiedId = false)
	{
		ProfileScope profile(gProfileLookup);
#ifdef LOOKUP_DEBUG
		std::cout << "lookup: " << getValue(id) << " (" << getIdentifierType(filter) << ")" << std::endl;
#endif
		LookupResult result;
		Scope* qualifying = getQualifyingScope();
		if(qualifying != 0)
		{
			// [basic.lookup.qual]
			if(qualifyingType != 0)
			{
				instantiateClass(*qualifyingType, getLocation(), enclosingType);
				if(result.append(::findDeclaration(*qualifyingType, id, filter)))
				{
					return result;
				}
			}
			else if(result.append(::findNamespaceDeclaration(*qualifying, id, filter)))
			{
#ifdef LOOKUP_DEBUG
				std::cout << "HIT: qualified" << std::endl;
#endif
				return result;
			}
		}
		else
		{
			if(memberObject != 0)
			{
				// [basic.lookup.classref]
				if(memberObject == &nullScope())
				{
					 // TODO: this occurs if evaluation of the left-hand-side of a member access expression fails: report an error!
				}
				else if(memberType != 0)
				{
					SYMBOLS_ASSERT(memberType->declaration->enclosed == memberObject);
					if(result.append(::findDeclaration(*memberType, id, filter)))
					{
#ifdef LOOKUP_DEBUG
						std::cout << "HIT: member" << std::endl;
#endif
						return result;
					}
				}
				// else if we're parsing a nested-name-specifier prefix, look up in the current context
			}

			if(memberObject == 0 || !isUnqualifiedId)
			{
				if(templateParamScope != 0)
				{
					// this occurs when looking up template parameters during parse of (but before the point of declaration of) a template class/function, 
					if(result.append(::findDeclaration(*templateParamScope, id, filter)))
					{
#ifdef LOOKUP_DEBUG
						std::cout << "HIT: templateParamScope" << std::endl;
#endif
						return result;
					}
				}
				if(result.append(::findClassOrNamespaceMemberDeclaration(*enclosing, id, filter)))
				{
#ifdef LOOKUP_DEBUG
					std::cout << "HIT: unqualified" << std::endl;
#endif
					return result;
				}
			}
		}
#ifdef LOOKUP_DEBUG
		std::cout << "FAIL" << std::endl;
#endif
		result.filtered = &gUndeclaredInstance;
		return result;
	}

	const DeclarationInstance& pointOfDeclaration(
		const TreeAllocator<int>& allocator,
		Scope* parent,
		Identifier& name,
		const TypeId& type,
		Scope* enclosed,
		DeclSpecifiers specifiers = DeclSpecifiers(),
		bool isTemplate = false,
		const TemplateParameters& params = TEMPLATEPARAMETERS_NULL,
		bool isSpecialization = false,
		const TemplateArguments& arguments = TEMPLATEARGUMENTS_NULL,
		size_t templateParameter = INDEX_INVALID,
		const Dependent& valueDependent = Dependent())
	{
		SEMANTIC_ASSERT(parent != 0);
		SEMANTIC_ASSERT(templateParameter == INDEX_INVALID || ::isTemplate(*parent));
		SEMANTIC_ASSERT(isTemplate || params.empty());
		SEMANTIC_ASSERT(isClassKey(*type.declaration) || !hasTemplateParamDefaults(params)); // 14.1-9: a default template-arguments may be specified in a class template declaration/definition (not for a function or class-member)

		parser->context.allocator.deferredBacktrack(); // flush cached parse-tree

		static size_t uniqueId = 0;

		SEMANTIC_ASSERT(!name.value.empty());
		Declaration declaration(allocator, parent, name, type, enclosed, specifiers, isTemplate, params, isSpecialization, arguments, templateParameter, valueDependent);
		declaration.uniqueId = ++uniqueId;
		DeclarationInstance instance;
		const DeclarationInstance* existing = 0;
		if(!isAnonymous(declaration)) // unnamed class/struct/union/enum
		{
			existing = ::findDeclaration(parent->declarations, name);
		}
		/* 3.4.4-1
		An elaborated-type-specifier (7.1.6.3) may be used to refer to a previously declared class-name or enum-name
		even though the name has been hidden by a non-type declaration (3.3.10).
		*/
		if(existing != 0)
		{
			instance = *existing;
			try
			{
				const Declaration& primary = getPrimaryDeclaration(*instance, declaration);
				if(&primary == instance)
				{
					return *existing;
				}
			}
			catch(DeclarationError& e)
			{
				printPosition(name.source);
				std::cout << "'" << name.value.c_str() << "': " << e.description << std::endl;
				printPosition(instance->getName().source);
				throw SemanticError();
			}

			if(!isNamespace(declaration)
				&& !isType(declaration)
				&& isFunction(declaration))
			{
				// quick hack - if any template overload of a function has been declared, all subsequent declarations are template functions
				if(instance->isTemplate)
				{
					declaration.isTemplate = true;
				}
			}

			declaration.overloaded = findOverloaded(instance); // the new declaration refers to the existing declaration

			instance.p = 0;
			instance.overloaded = existing;
			instance.redeclared = findRedeclared(declaration, existing);
			if(instance.redeclared != 0)
			{
				instance.p = *instance.redeclared;
				if(isClass(declaration)
					&& declaration.isTemplate)
				{
					TemplateParameters tmp(context);
					tmp.swap(instance->templateParams);
					instance->templateParams = declaration.templateParams;
					if(declaration.isSpecialization) // this is a partial-specialization
					{
						SEMANTIC_ASSERT(!hasTemplateParamDefaults(declaration.templateParams)); // TODO: non-fatal error: partial-specialization may not have default template-arguments
					}
					else
					{
						SEMANTIC_ASSERT(!declaration.templateParams.empty());
						mergeTemplateParamDefaults(*instance, tmp);
					}
				}
				if(isClass(declaration)
					&& isIncomplete(*instance)) // if this class-declaration was previously forward-declared
				{
					instance->enclosed = declaration.enclosed; // complete it
					instance->setName(declaration.getName()); // make this the definition
				}
			}
		}
		if(instance.p == 0)
		{
			instance.p = allocatorNew(context, Declaration());
			instance->swap(declaration);
		}

		instance.name = &name;
		instance.ordering = context.declarationCount++;
		const DeclarationInstance& result = parent->declarations.insert(instance);
		parent->declarationList.push_back(instance);
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

	void pushScope(Scope* scope)
	{
		SEMANTIC_ASSERT(findScope(enclosing, scope) == 0);
		scope->parent = enclosing;
		enclosing = scope;
	}

	void addBase(Declaration* declaration, const Type& base)
	{
#if 0
		if(getUnderlyingType(base).declaration == declaration)
		{
			return; // TODO: implement template-instantiation, and disallow inheriting from current-instantiation
		}
#endif
		declaration->enclosed->bases.push_front(base);
	}

	Declaration* getDeclaratorQualifying()
	{
		if(qualifying_p == TypePtr(0))
		{
			return 0;
		}
		Declaration* declaration = qualifying_p->declaration;
		if(isNamespace(*declaration))
		{
			return declaration;
		}
		SEMANTIC_ASSERT(isClass(*declaration)); // TODO: non-fatal error: declarator names must not be typedef names
		// only declarator names may be dependent
		if(declaration->isTemplate) // TODO: template partial specialization
		{
			Declaration* specialization = findTemplateSpecialization(declaration, qualifying_p->templateArguments);
			if(specialization != 0)
			{
				return specialization;
			}
			return findPrimaryTemplate(declaration);
		}
		return declaration; 
	}

	Scope* getQualifyingScope()
	{
		if(qualifyingScope == 0)
		{
			return 0;
		}
		SEMANTIC_ASSERT(qualifyingScope->enclosed != 0);
		return qualifyingScope->enclosed;
	}

	void clearQualifying()
	{
		qualifying_p = 0;
		qualifyingScope = 0;
		qualifyingType = 0;
		memberObject = 0;
		memberType = 0;
	}

	const TemplateParameters& getTemplateParams(Scope* parent) const
	{
		return templateParams != 0 && parent == templateEnclosing ? *templateParams : TEMPLATEPARAMETERS_NULL;
	}

	void clearTemplateParams()
	{
		templateParamScope = 0;
		templateParams = 0;
	}

	template<typename T>
	void reportIdentifierMismatch(T* symbol, const Identifier& id, Declaration* declaration, const char* expected)
	{
		result = 0;
#if 0
		gIdentifierMismatch = IdentifierMismatch(id, declaration, expected);
#endif
	}

	Scope* getEtsScope() const
	{
		Scope* scope = enclosing;
		for(; !enclosesEts(scope->type); scope = scope->parent)
		{
		}
		return scope;
	}

	Scope* getFriendScope() const
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

	Scope* getClassScope() const
	{
		return ::getEnclosingClass(enclosing);
	}

	void printScope()
	{
#if 1
		if(templateParamScope != 0)
		{
			std::cout << "template-params:" << std::endl;
			::printScope(*templateParamScope);
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


	bool isDependent(Declaration* dependent) const
	{
		return ::isDependent(dependent, enclosing, templateParamScope);
	}
	bool isDependent(const Type& type) const
	{
		return isDependent(type.dependent);
	}
	bool isDependent(const Types& bases) const
	{
		DeclarationPtr dependent(0);
		setDependent(dependent, bases);
		return isDependent(dependent);
	}
	bool isDependent(const TypePtr& qualifying) const
	{
		DeclarationPtr dependent(0);
		setDependent(dependent, qualifying.get());
		return isDependent(dependent);
	}
	bool isDependent(const TemplateArguments& arguments) const
	{
		DeclarationPtr dependent(0);
		setDependent(dependent, arguments);
		return isDependent(dependent);
	}
	bool isDependent(const Dependent& dependent) const
	{
		return isDependent(static_cast<Declaration*>(dependent));
	}
	// the dependent-scope is the outermost template-definition
	void setDependent(DeclarationPtr& dependent, Declaration* candidate) const
	{
		SEMANTIC_ASSERT(dependent == DeclarationPtr(0) || isDependent(dependent));
		if(!isDependent(candidate))
		{
			return;
		}
		SEMANTIC_ASSERT(candidate->scope->type != SCOPETYPE_NAMESPACE);
		if(dependent != 0
			&& findScope(candidate->scope, dependent->scope)) // if the candidate template-parameter's template-definition is within the current dependent-scope
		{
			return; // already dependent on outer template
		}
		dependent = candidate; // the candidate template-parameter is within the current dependent-scope
	}
	void setDependent(DeclarationPtr& dependent, Declaration& declaration) const
	{
		if(declaration.templateParameter != INDEX_INVALID)
		{
			setDependent(dependent, &declaration);
		}
		else if(declaration.specifiers.isTypedef)
		{
			setDependent(dependent, declaration.type.dependent);
		}
		else if(isClass(declaration)
			&& isComplete(declaration))
		{
			setDependent(dependent, declaration.enclosed->bases);
		}

		{
			Declaration* enclosingTemplate = findEnclosingClassTemplate(&declaration);
			if(enclosingTemplate != 0)
			{
				// 'declaration' is a class that is dependent because it is a (possibly specialized) member of an enclosing template class
				SEMANTIC_ASSERT(enclosingTemplate->isSpecialization || !enclosingTemplate->templateParams.empty());
				if(!enclosingTemplate->templateParams.empty()) // if the enclosing template class is not an explicit specialization
				{
					// depend on the template parameter(s) of the enclosing template class
					setDependent(dependent, enclosingTemplate->templateParams.back().declaration);
				}
			}
		}

		setDependent(dependent, declaration.valueDependent);
	}
	void setDependent(DeclarationPtr& dependent, const Type* qualifying) const
	{
		if(qualifying == 0)
		{
			return;
		}
		setDependent(dependent, qualifying->dependent);
		setDependent(dependent, qualifying->qualifying.get());
	}
	void setDependent(DeclarationPtr& dependent, const Qualifying& qualifying) const
	{
		setDependent(dependent, qualifying.get());
	}
	void setDependent(DeclarationPtr& dependent, const Types& bases) const
	{
		for(Types::const_iterator i = bases.begin(); i != bases.end(); ++i)
		{
			setDependent(dependent, (*i).dependent);
		}
	}
	void setDependent(DeclarationPtr& dependent, const TemplateArguments& arguments) const
	{
		for(TemplateArguments::const_iterator i = arguments.begin(); i != arguments.end(); ++i)
		{
			setDependent(dependent, (*i).type.dependent);
			setDependent(dependent, (*i).valueDependent);
		}
	}
	void setDependent(DeclarationPtr& dependent, const Parameters& parameters) const
	{
		for(Parameters::const_iterator i = parameters.begin(); i != parameters.end(); ++i)
		{
			setDependent(dependent, (*i).declaration->type.dependent);
		}
	}
	void setDependent(Type& type, Declaration* declaration) const
	{
		setDependent(type.dependent, declaration);
	}
	void setDependent(Type& type) const
	{
		setDependent(type.dependent, *type.declaration);
	}

	void addDependentName(Dependent& dependent, Declaration* declaration)
	{
		setDependent(dependent, *declaration);
	}
	void addDependentType(Dependent& dependent, Declaration* declaration)
	{
		setDependent(dependent, declaration->type.dependent);
	}
	void addDependent(Dependent& dependent, const Type& type)
	{
		setDependent(dependent, type.dependent);
	}
	void addDependent(Dependent& dependent, Scope* scope)
	{
		setDependent(dependent, scope->bases);
	}
	void addDependent(Dependent& dependent, Dependent& other)
	{
		setDependent(dependent, other);
	}
};


inline const char* getIdentifierType(IdentifierFunc func)
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

	void disableBacktrack()
	{
		parser->addBacktrackCallback(makeBacktrackErrorCallback());
	}

	// Causes /p declaration to be undeclared when backtracking.
	// In practice this only happens for the declaration in an elaborated-type-specifier.
	void trackDeclaration(const DeclarationInstance& declaration)
	{
		parser->addBacktrackCallback(makeUndeclareCallback(&declaration));
	}

	Declaration* declareClass(Identifier* id, bool isSpecialization, TemplateArguments& arguments)
	{
		Scope* enclosed = newScope(makeIdentifier("$class"), SCOPETYPE_CLASS);
		DeclarationInstanceRef declaration = pointOfDeclaration(context, enclosing, id == 0 ? enclosing->getUniqueName() : *id, TYPE_CLASS, enclosed, DeclSpecifiers(), enclosing == templateEnclosing, getTemplateParams(enclosing), isSpecialization, arguments);
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		if(id != 0)
		{
			setDecoration(id, declaration);
		}
		enclosed->name = declaration->getName();
		return declaration;
	}

	Declaration* declareObject(Scope* parent, Identifier* id, const TypeId& type, Scope* enclosed, DeclSpecifiers specifiers, size_t templateParameter, const Dependent& valueDependent)
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
		DeclarationInstanceRef declaration = pointOfDeclaration(context, parent, *id, type, enclosed, specifiers, parent == templateEnclosing, getTemplateParams(parent), false, TEMPLATEARGUMENTS_NULL, templateParameter, valueDependent); // 3.3.1.1
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		if(id != &gAnonymousId)
		{
			setDecoration(id, declaration);
		}

		if(isMember(*declaration)
			&& !declaration->isTemplate // TODO: template function instantiation
			&& declaration->type.isDependent
			&& declaration->type.declaration != &gCtor // ignore constructor
			&& getEnclosingTemplate(declaration->scope)->type == SCOPETYPE_CLASS)
		{
			Scope* scope = getEnclosingClass(declaration->scope);
			Declaration* enclosingClass = getClassDeclaration(scope);
			const TypeInstance& instance = getObjectType(enclosingClass->type.unique);
			if(declaration->instance != INDEX_INVALID)
			{
				SEMANTIC_ASSERT(instance.members[declaration->instance] == UniqueTypeWrapper(declaration->type.unique));
			}
			else
			{
				declaration->instance = instance.members.size();
				const_cast<TypeInstance*>(&instance)->members.push_back(UniqueTypeWrapper(declaration->type.unique));
				const_cast<TypeInstance*>(&instance)->memberDeclarations.push_back(declaration);
			}
		}

		// TODO: accurate sizeof
		if(!declaration->type.isDependent
			&& isMember(*declaration)
			&& !isTypedef(*declaration)
			&& getEnclosingClass(declaration->scope) != 0
			&& (UniqueTypeWrapper(declaration->type.unique).isSimple()
			|| UniqueTypeWrapper(declaration->type.unique).isArray()))
		{
			Scope* scope = getEnclosingClass(declaration->scope);
			Declaration* enclosingClass = getClassDeclaration(scope);
			const TypeInstance& instance = getObjectType(enclosingClass->type.unique);
			const_cast<TypeInstance*>(&instance)->size += requireCompleteObjectType(UniqueTypeWrapper(declaration->type.unique), getLocation(), enclosingType);
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
			DeclarationInstanceRef declaration = pointOfDeclaration(context, getEtsScope(), *forward, TYPE_CLASS, 0, DeclSpecifiers(), enclosing == templateEnclosing);
			
			trackDeclaration(declaration);
			setDecoration(forward, declaration);
			type = declaration;
			return true;
		}
		return false;
	}

	LookupResultRef lookupTemplate(const Identifier& id, LookupFilter filter)
	{
		if(!isDependent(qualifying_p))
		{
			return LookupResultRef(findDeclaration(id, filter));
		}
		return gDependentTemplateInstance;
	}

	void addOverloads(OverloadResolver& resolver, Declaration* declaration, Location source, const TypeInstance* enclosing = 0)
	{
		for(Declaration* p = declaration; p != 0; p = p->overloaded)
		{
			if(p->isTemplate)
			{
				return; // TODO: template argument deduction
			}
		}

		for(Declaration* p = declaration; p != 0; p = p->overloaded)
		{
			if(p->enclosed == 0)
			{
				continue;
			}

			UniqueTypeWrapper type = getUniqueType(p->type, source, enclosing, isDependent(p->type));
			resolver.add(FunctionOverload(p, type));
		}
	}

	// source: where the overload resolution occurs (point of instantiation)
	// enclosingType: the class of which the declaration is a member (along with all its overloads).
	inline FunctionOverload findBestMatch(Declaration* declaration, const UniqueTypeIds& arguments, Location source, const TypeInstance* enclosingType)
	{
		OverloadResolver resolver(arguments, source, enclosingType);
		addOverloads(resolver, declaration, source, enclosingType);

		if(resolver.ambiguous != 0)
		{
#if 0
			std::cout << "overload resolution failed:" << std::endl;
			std::cout << "  ";
			printPosition(resolver.ambiguous->getName().position);
			printName(resolver.ambiguous);
			std::cout << std::endl;
			if(resolver.best.declaration != 0)
			{
				std::cout << "  ";
				printPosition(resolver.best.declaration->getName().position);
				printName(resolver.best.declaration);
				std::cout << std::endl;
			}
#endif
		}

		return resolver.get();
	}

	FunctionOverload findBestOverloadedOperator(const Identifier& id, const UniqueTypeId& type)
	{
		if((isClass(type) || isEnumeration(type)) // if the operand has class or enum type
			&& (type.isSimple() || type.isReference())) // and is a simple object or reference
		{
			UniqueTypeIds::Pointer::Value value = UniqueTypeIds::Pointer::Value(UniqueTypeIds::Node(type));
			UniqueTypeIds arguments = UniqueTypeIds(TREEALLOCATOR_NULL);
			arguments.head.next = &value;
			OverloadResolver resolver(arguments, Location(id.source, context.declarationCount), enclosingType);

			if(isClass(type))
			{
				SEMANTIC_ASSERT(isComplete(type)); // TODO: non-fatal parse error
				const TypeInstance* enclosing = &getObjectType(type.value);
				instantiateClass(*enclosing, Location(id.source, context.declarationCount), enclosingType); // searching for overloads requires a complete type
				LookupResultRef declaration = ::findDeclaration(*enclosing, id, IsAny());
				if(declaration != 0)
				{
					enclosing = findEnclosingType(enclosing, declaration->scope); // find the base class which contains the member-declaration
					SEMANTIC_ASSERT(enclosing != 0);
					addOverloads(resolver, declaration, Location(id.source, context.declarationCount), enclosing);
				}
			}
			// TODO: ignore non-member candidates if no operand has a class type, unless one or more params has enum (ref) type
			LookupResultRef declaration = findDeclaration(id, IsNonMemberName()); // look up non-member candidates in this context (ignoring members)
			if(declaration != &gUndeclared
				&& !declaration->isTemplate // TODO: template argument deduction for overloaded operator
				&& !declaration->specifiers.isFriend) // TODO: 14.5.3: friend function as member of a template-class, which depends on template arguments
			{
				addOverloads(resolver, declaration, Location(id.source, context.declarationCount));
			}
			{
				// TODO: 13.3.1.2: built-in operators for overload resolution
				// These are relevant either when the operand has a user-defined conversion to a non-class type, or is an enum that can be converted to an arithmetic type
				CandidateFunction candidate(FunctionOverload(&gUnknown, gUniqueTypeNull));
				candidate.conversions.reserve(1);
				candidate.conversions.push_back(IMPLICITCONVERSION_USERDEFINED);//getIcsRank(???, type)); // TODO: cv-qualified overloads
				resolver.add(candidate); // TODO: ignore built-in overloads that have same signature as a non-member
			}
			return resolver.get();
		}
		return FunctionOverload(&gUnknown, gUniqueTypeNull);
	}

	// 5 Expressions
	// paragraph 9: usual arithmetic conversions
	static UniqueTypeWrapper binaryOperatorIntegralType(UniqueTypeWrapper left, UniqueTypeWrapper right)
	{
		if(left.value == UNIQUETYPE_NULL
			|| right.value == UNIQUETYPE_NULL)
		{
			// TODO: assert
			return gUniqueTypeNull;
		}

		if(isEqual(left, gUnsignedLongInt)
			|| isEqual(right, gUnsignedLongInt))
		{
			return gUnsignedLongInt;
		}
		if((isEqual(left, gSignedLongInt)
				&& isEqual(right, gUnsignedInt))
			|| (isEqual(left, gUnsignedInt)
				&& isEqual(right, gSignedLongInt)))
		{
			return gUnsignedLongInt;
		}
		if(isEqual(left, gSignedLongInt)
			|| isEqual(right, gSignedLongInt))
		{
			return gSignedLongInt;
		}
		if(isEqual(left, gUnsignedInt)
			|| isEqual(right, gUnsignedInt))
		{
			return gUnsignedInt;
		}
		return gSignedInt;
	}
	static UniqueTypeWrapper binaryOperatorArithmeticType(UniqueTypeWrapper left, UniqueTypeWrapper right)
	{
		if(left.value == UNIQUETYPE_NULL
			|| right.value == UNIQUETYPE_NULL)
		{
			// TODO: assert
			return gUniqueTypeNull;
		}

		//TODO: SEMANTIC_ASSERT(isArithmetic(left) && isArithmetic(right));
		if(isEqual(left, gLongDouble)
			|| isEqual(right, gLongDouble))
		{
			return gLongDouble;
		}
		if(isEqual(left, gDouble)
			|| isEqual(right, gDouble))
		{
			return gDouble;
		}
		if(isEqual(left, gFloat)
			|| isEqual(right, gFloat))
		{
			return gFloat;
		}
		return binaryOperatorIntegralType(promoteToIntegralType(left), promoteToIntegralType(right));
	}
	static UniqueTypeWrapper binaryOperatorAdditiveType(UniqueTypeWrapper left, UniqueTypeWrapper right)
	{
		if(left.value == UNIQUETYPE_NULL
			|| right.value == UNIQUETYPE_NULL)
		{
			// TODO: assert
			return gUniqueTypeNull;
		}

		if(left.isPointer())
		{
			if(isIntegral(right)
				|| isEnumeration(right))
			{
				return left;
			}
			if(right.isPointer())
			{
				return gSignedLongInt; // TODO: ptrdiff_t
			}
		}
		return binaryOperatorArithmeticType(left, right);
	}
	static UniqueTypeWrapper getBuiltInUnaryOperatorReturnType(cpp::unary_operator* symbol, UniqueTypeWrapper type)
	{
		if(symbol->id == cpp::unary_operator::AND) // address-of
		{
			UniqueTypeId result = type;
			result.push_front(PointerType()); // produces a non-const pointer
			return result;
		}
		else if(symbol->id == cpp::unary_operator::STAR) // dereference
		{
			UniqueTypeId result = type;
			if(!result.empty()) // TODO: assert
			{
				result.pop_front();
			}
			return result;
		}
		else if(symbol->id == cpp::unary_operator::PLUS
			|| symbol->id == cpp::unary_operator::MINUS)
		{
			if(!isFloating(type))
			{
				// TODO: check type is integral or enumeration
				return promoteToIntegralType(type);
			}
			return type;
		}
		else if(symbol->id == cpp::unary_operator::NOT)
		{
			return gBool;
		}
		else if(symbol->id == cpp::unary_operator::COMPL)
		{
			// TODO: check type is integral or enumeration
			return promoteToIntegralType(type);
		}
		SEMANTIC_ASSERT(symbol->id == cpp::unary_operator::PLUSPLUS || symbol->id == cpp::unary_operator::MINUSMINUS);
		return type;
	}

	template<typename T>
	void makeUniqueTypeImpl(T& type, Location source)
	{
		type.isDependent = isDependent(type);
		type.unique = makeUniqueType(type, source, enclosingType, type.isDependent).value;
	}
	UniqueTypeWrapper makeUniqueTypeSafe(Type& type, Location source)
	{
		makeUniqueTypeImpl(type, source);
		return type.isDependent ? gUniqueTypeNull : UniqueTypeWrapper(type.unique);
	}
	UniqueTypeWrapper makeUniqueTypeSafe(TypeId& type, Location source)
	{
		makeUniqueTypeImpl(type, source);
		return type.isDependent ? gUniqueTypeNull : UniqueTypeWrapper(type.unique);
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
		qualifyingScope = qualifying_p->declaration;
		qualifyingType = 0;
	}

	void swapQualifying(const Type& type, bool isDeclarator = false)
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
		swapQualifying(tmp, isDeclarator);
	}
	void swapQualifying(Qualifying& other, bool isDeclarator = false)
	{
		qualifying.swap(other);
		qualifying_p = qualifying.get_ref();
		if(isDeclarator)
		{
			qualifyingScope = getDeclaratorQualifying();
		}
		else if(qualifying_p != TypePtr(0))
		{
			Declaration* declaration = qualifying_p->declaration;
			if(isNamespace(*declaration))
			{
				qualifyingScope = declaration;
			}
			else if(isDependent(qualifying_p))
			{
				qualifyingScope = 0;
			}
			else
			{
				qualifyingType = &getObjectType(getUniqueType(*qualifying_p, getLocation(), enclosingType).value);
				qualifyingScope = qualifyingType->declaration;
			}
		}
	}
};




#define TREEWALKER_WALK(walker, symbol) SYMBOL_WALK(walker, symbol);
#define TREEWALKER_WALK_SRC(walker, symbol) Source source_ = parser->get_source(); TREEWALKER_WALK(walker, symbol); symbol->source = source_
#define TREEWALKER_LEAF(symbol) TREEWALKER_WALK(*this, symbol)
#define TREEWALKER_LEAF_SRC(symbol) TREEWALKER_WALK_SRC(*this, symbol)

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

	LookupResultRef declaration;
	IsHiddenNamespaceName filter;
	NamespaceNameWalker(const WalkerState& state)
		: WalkerBase(state)
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
		setDecoration(&symbol->value, declaration);
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
		argument.source = getLocation();
		makeUniqueTypeSafe(argument.type, argument.source);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.expression.isTemplateArgumentAmbiguity)
		{
			// [temp.arg] In a template argument, an ambiguity between a typeid and an expression is resolved to a typeid
			result = 0; // fail parse, will retry for a type-id
			return;
		}
		addDependent(argument.valueDependent, walker.valueDependent);
		argument.type = &gNonType;
		argument.expression = walker.expression;
		argument.source = getLocation();
	}
	void visit(cpp::template_argument_list* symbol)
	{
		TemplateArgumentListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		arguments.swap(walker.arguments);
		arguments.push_front(walker.argument); // allocates last element first!
	}
};

struct OverloadableOperatorWalker : public WalkerBase
{
	Name name;
	OverloadableOperatorWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	template<typename T>
	void visit(T* symbol)
	{
		TREEWALKER_LEAF(symbol);
		name = getOverloadableOperatorId(symbol);
	}
	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
	}
};

struct OperatorFunctionIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Name name;
	OperatorFunctionIdWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::overloadable_operator* symbol)
	{
		OverloadableOperatorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		name = walker.name;
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
	void visit(cpp::operator_function_id* symbol) 
	{
		Source source = parser->get_source();
		OperatorFunctionIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		symbol->value.value = walker.name;
		symbol->value.source = source;
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

	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isIdentifier;
	bool isTemplate;
	UnqualifiedIdWalker(const WalkerState& state, bool isTemplate = false)
		: WalkerBase(state), id(0), arguments(context), isIdentifier(false), isTemplate(isTemplate)
	{
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		id = &symbol->value;
		isIdentifier = true;
		if(!isDependent(qualifying_p))
		{
			declaration = findDeclaration(*id, IsAny(), true);
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		if(!isTemplate
			&& !isDependent(qualifying_p))
		{
			LookupResultRef declaration = findDeclaration(*walker.id, IsAny(), true);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			this->declaration = declaration;
		}
		id = walker.id;
		arguments.swap(walker.arguments);
	}
	void visit(cpp::template_id_operator_function* symbol)
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
#if 0 // TODO: member lookup
		if(!isTemplate // TODO: is this possible?
			&& !isDependent(qualifying_p))
		{
			LookupResultRef declaration = findDeclaration(*walker.id, IsAny(), true);
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			this->declaration = findTemplateSpecialization(declaration, walker.arguments);
		}
#endif
		id = walker.id;
		arguments.swap(walker.arguments);
	}
	void visit(cpp::operator_function_id* symbol)
	{
		Source source = parser->get_source();
		OperatorFunctionIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		symbol->value.value = walker.name;
		symbol->value.source = source;
		id = &symbol->value;
		if(!isDependent(qualifying_p))
		{
			declaration = findDeclaration(*id, IsAny(), true);
		}
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

	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isTemplate;
	QualifiedIdWalker(const WalkerState& state)
		: WalkerQualified(state), id(0), arguments(context), isTemplate(false)
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
		arguments.swap(walker.arguments);
	}
	void visit(cpp::qualified_id_suffix* symbol)
	{
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
	}
};

struct IdExpressionWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	/* 14.6.2.2-3
	An id-expression is type-dependent if it contains:
	— an identifier that was declared with a dependent type,
	— a template-id that is dependent,
	— a conversion-function-id that specifies a dependent type,
	— a nested-name-specifier or a qualified-id that names a member of an unknown specialization
	*/
	LookupResultRef declaration;
	IdentifierPtr id;
	TemplateArguments arguments; // only used if the identifier is a template-name
	bool isIdentifier;
	bool isTemplate;
	IdExpressionWalker(const WalkerState& state, bool isTemplate = false)
		: WalkerQualified(state), id(0), arguments(context), isIdentifier(false), isTemplate(isTemplate)
	{
	}
	void visit(cpp::qualified_id_default* symbol)
	{
		// TODO
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		// TODO
		QualifiedIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
		swapQualifying(walker.qualifying);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		// TODO
		UnqualifiedIdWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		arguments.swap(walker.arguments);
		isIdentifier = walker.isIdentifier;
	}
};

struct ExplicitTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TypeId type;
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWrapper expression;
	ExplicitTypeExpressionWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context)
	{
	}
	void visit(cpp::simple_type_specifier* symbol)
	{
		TypeSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		if(type.declaration == 0)
		{
			type = getFundamentalType(walker.fundamental);
		}
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type, getLocation());
	}
	void visit(cpp::typename_specifier* symbol)
	{
		TypenameSpecifierWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type, getLocation());
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type, getLocation());
	}
	void visit(cpp::new_type* symbol)
	{
		NewTypeWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		addDependent(typeDependent, type);
		addDependent(typeDependent, walker.valueDependent);
		makeUniqueTypeSafe(type, getLocation());
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::cast_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct ArgumentListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	UniqueTypeIds arguments;
	Dependent typeDependent;
	Dependent valueDependent;
	ArgumentListWalker(const WalkerState& state)
		: WalkerBase(state), arguments(context)
	{
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		arguments.push_front(walker.type);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct LiteralWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ExpressionWrapper expression;
	LiteralWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::numeric_literal* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::numeric_literal::UNKNOWN) // workaround for boost::wave issue: T_PP_NUMBER exists in final token stream
		{
			symbol->id = isFloatingLiteral(symbol->value.value.c_str()) ? cpp::numeric_literal::FLOATING : cpp::numeric_literal::INTEGER;
		}
		expression = makeExpression(parseNumericLiteral(symbol));
	}
	void visit(cpp::string_literal* symbol)
	{
		TREEWALKER_LEAF(symbol);
		expression = makeExpression(IntegralConstantExpression(getStringLiteralType(symbol), IntegralConstant()));
	}
};

struct DependentPrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	LookupResultRef declaration;
	IdentifierPtr id;
	Dependent typeDependent;
	DependentPrimaryExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}
	void visit(cpp::id_expression* symbol)
	{
		/* temp.dep.expr
		An id-expression is type-dependent if it contains:
		— an identifier that was declared with a dependent type,
		— a template-id that is dependent,
		— a conversion-function-id that specifies a dependent type,
		— a nested-name-specifier or a qualified-id that names a member of an unknown specialization.
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
				setDecoration(walker.id, declaration);
				addDependentType(typeDependent, declaration); // an id-expression is type-dependent if it contains an identifier that was declared with a dependent type
			}
			else if(walker.id != 0)
			{
				setDecoration(walker.id, gDependentObjectInstance);
				if(!walker.qualifying.empty())
				{
					setDependent(typeDependent, walker.qualifying.get());
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

	LookupResultRef declaration;
	IdentifierPtr id;
	Dependent typeDependent;
	DependentPostfixExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
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
		ArgumentListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		if(id != 0)
		{
			if(!isDependent(walker.typeDependent))
			{
				if(declaration != 0)
				{
					if(declaration == &gUndeclared
						|| !isObject(*declaration))
					{
						return reportIdentifierMismatch(symbol, *id, declaration, "object-name");
					}
					addDependentType(typeDependent, declaration);
					setDecoration(id, declaration);
				}
			}
			else
			{
				setDecoration(id, gDependentObjectInstance);
			}
		}
	}
};

struct PrimaryExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	UniqueTypeId type;
	ExpressionWrapper expression;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	const TypeInstance* idEnclosing; // may be valid when the above id-expression is a qualified-id
	Dependent typeDependent;
	Dependent valueDependent;
	PrimaryExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0), idEnclosing(0)
	{
	}
	void visit(cpp::literal* symbol)
	{
		LiteralWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		expression = walker.expression;
		type = typeofExpression(expression, getLocation());
		SEMANTIC_ASSERT(!type.empty());
	}
	/* temp.dep.constexpr
	An identifier is value-dependent if it is:
	— a name declared with a dependent type,
	— the name of a non-type template parameter,
	— a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
	*/
	void visit(cpp::id_expression* symbol)
	{
		Source source = parser->get_source();
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		type = gUniqueTypeNull;
		LookupResultRef declaration = walker.declaration;
		// [temp.dep.expr] An id-expression is type-dependent if it contains:- a nested-name-specifier that contains a class-name that names a dependent type
		setDependent(typeDependent, walker.qualifying.get());
		setDependent(valueDependent, walker.qualifying.get()); // it's clearly value-dependent too, because name lookup must be deferred

		UniqueTypeWrapper qualifying = walker.qualifying.empty() || isNamespace(*walker.qualifying.back().declaration)
			? gUniqueTypeNull : UniqueTypeWrapper(walker.qualifying.back().unique);

		if(declaration == &gUndeclared
			&& id->value == gOperatorAssignId)
		{
			// TODO: declare operator= if not already declared
			declaration = LookupResultRef();
			expression = ExpressionWrapper();
		}
		else if(declaration == 0)
		{
			if(!isDependent(walker.qualifying.get_ref()))
			{
				SYMBOLS_ASSERT(id->value == gConversionFunctionId.value); // TODO: user defined conversions
			}
			else
			{
				setDecoration(id, gDependentObjectInstance);

				expression = ExpressionWrapper(
					makeExpression(DependentIdExpression(id->value, qualifying)),
					true, // TODO: expression depending on template parameter may or may not be an integral constant expression
					true,
					true
				);
			}
		}
		else
		{
			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				return reportIdentifierMismatch(symbol, *id, declaration, "object-name");
			}

			// [temp.dep.expr] An id-expression is type-dependent if it contains:- an identifier that was declared with a dependent type
			addDependentType(typeDependent, declaration);
			// [temp.dep.expr] An id-expression is type-dependent if it contains: -a template-id that is dependent
			setDependent(typeDependent, walker.arguments); // the id-expression may have an explicit template argument list

			// [temp.dep.constexpr] An identifier is value-dependent if it is:- a name declared with a dependent type
			addDependentType(valueDependent, declaration);
			// [temp.dep.constexpr] An identifier is value-dependent if it is:- the name of a non-type template parameter,
			// - a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
			addDependentName(valueDependent, declaration); // adds 'declaration' if it names a non-type template-parameter; adds a dependent initializer

			setDecoration(id, declaration);

			SEMANTIC_ASSERT(!isDependent(walker.qualifying.get_ref()));

			const TypeInstance* qualifyingType = qualifying == gUniqueTypeNull ? 0 : &getObjectType(qualifying.value);
			SEMANTIC_ASSERT(qualifyingType == walker.qualifyingType);

			SEMANTIC_ASSERT(declaration->templateParameter == INDEX_INVALID || walker.qualifying.empty()); // template params cannot be qualified
			expression = ExpressionWrapper(
				declaration->templateParameter == INDEX_INVALID
					? makeExpression(IdExpression(declaration, qualifyingType))
					: makeExpression(NonTypeTemplateParameter(declaration)),
				false,
				isDependent(typeDependent),
				isDependent(valueDependent)
			);

			expression.isQualifiedNonStaticMemberName = qualifying != gUniqueTypeNull
				&& isMember(*declaration)
				&& !isStatic(*declaration);

			// further type resolution (including overloads) should be made in the context of the enclosing type (if present)
			idEnclosing = makeUniqueEnclosing(walker.qualifying, Location(id->source, context.declarationCount), enclosingType);
			if(isMember(*declaration)) // if the declaration is a class member
			{
				SEMANTIC_ASSERT(idEnclosing != 0);
				// the identifier may name a type in a base-class of the qualifying type; findEnclosingType resolves this.
				idEnclosing = findEnclosingType(idEnclosing, declaration->scope); // it must be a member of (a base of) the qualifying class: find which one.
				SEMANTIC_ASSERT(idEnclosing != 0);
			}

			if(!isFunction(*declaration) // if the id-expression refers to a function, overload resolution dependends on the parameter types; defer evaluation of type
				&& !expression.isTypeDependent)
			{
				type = getUniqueType(declaration->type, Location(id->source, context.declarationCount), idEnclosing);
			}

			// [expr.const]
			// An integral constant-expression can involve only ... enumerators, const variables or static
			// data members of integral or enumeration types initialized with constant expressions, non-type template
			// parameters of integral or enumeration types
			expression.isConstant = declaration->templateParameter != INDEX_INVALID
				|| declaration->initializer.isConstant; // TODO: determining whether the expression is constant depends on the type of the expression!
		}
	}
	void visit(cpp::primary_expression_parenthesis* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		expression = walker.expression;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::primary_expression_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		// TODO: cv-qualifiers: change enclosingType to a UniqueType<ObjectType>
		// TODO: SEMANTIC_ASSERT(enclosingType != 0); // TODO: this occurs for out-of-line member definitions
		type = (enclosingType != 0) ? UniqueTypeWrapper(pushUniqueType(gUniqueTypes, makeUniqueObjectType(*enclosingType).value, PointerType())) : gUniqueTypeNull;
		/* 14.6.2.2-2
		'this' is type-dependent if the class type of the enclosing member function is dependent
		*/
		addDependent(typeDependent, enclosingDependent);
		setExpressionType(symbol, type);
	}
};

struct PostfixExpressionMemberWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	LookupResultRef declaration;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	bool isTemplate;
	bool isArrow;
	PostfixExpressionMemberWalker(const WalkerState& state)
		: WalkerQualified(state), id(0), isTemplate(false), isArrow(false)
	{
	}
	void visit(cpp::member_operator* symbol)
	{
		TREEWALKER_LEAF(symbol);
		isArrow = symbol->id == cpp::member_operator::ARROW;

		if(memberType != 0
			&& !::isDependent(*memberType)
			&& isClass(*memberType->declaration)) // TODO: assert that this is a class type
		{
			// [expr.ref] [the type of the object-expression shall be complete]
			instantiateClass(*memberType, getLocation(), enclosingType);
			memberObject = memberType->declaration->enclosed;
		}
		else
		{
			memberObject = const_cast<Scope*>(&SCOPE_NULL); // indicates that the left-hand side is an unknown type - possibly dependent
		}
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(getState(), isTemplate);
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		declaration = walker.declaration;
		swapQualifying(walker.qualifying);
	}
};

struct TypeTraitsIntrinsicWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent valueDependent;
	UniqueTypeWrapper first;
	UniqueTypeWrapper second;
	TypeTraitsIntrinsicWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::terminal<boost::wave::T_LEFTPAREN> symbol)
	{
		// debugging
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		addDependent(valueDependent, walker.type);

		makeUniqueTypeImpl(walker.type, Location(symbol->source, context.declarationCount));
		UniqueTypeWrapper uniqueType = UniqueTypeWrapper(walker.type.unique);
		setExpressionType(symbol, uniqueType);

		(first == gUniqueTypeNull ? first : second) = uniqueType;
	}
};

struct PostfixExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	UniqueTypeId type;
	ExpressionWrapper expression;
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	const TypeInstance* idEnclosing; // may be valid when the above id-expression is a qualified-id
	Dependent typeDependent;
	Dependent valueDependent;
	bool isPointer;
	PostfixExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0), idEnclosing(0), isPointer(false)
	{
	}
	void clearMemberType()
	{
		memberType = 0;
		isPointer = false;
	}
	void updateMemberType()
	{
		if(type == gUniqueTypeNull)
		{
			memberType = 0; // TODO: left-hand side should always have a known type!
			return;
		}
		// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		UniqueTypeId object = removeReference(type);
		isPointer = object.isPointer();
		if(isPointer)
		{
			object.pop_front();
		}
		// if the left-hand side is (reference-to)(pointer-to) object
		memberType = object.isSimple() ? &getObjectType(object.value) : 0;
	}
	void visit(cpp::primary_expression* symbol)
	{
		PrimaryExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		id = walker.id;
		idEnclosing = walker.idEnclosing;
		setExpressionType(symbol, type);
		updateMemberType();
	}
	// prefix
	void visit(cpp::postfix_expression_disambiguate* symbol)
	{
		// this is reached only if the lookup of the identifier in the primary-expression failed.
		// TODO: 
		/* 14.6.2-1
		In an expression of the form:
		postfix-expression ( expression-list. )
		where the postfix-expression is an unqualified-id but not a template-id, the unqualified-id denotes a dependent
		name if and only if any of the expressions in the expression-list is a type-dependent expression (
		*/
		DependentPostfixExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: type = &gDependent;
		addDependent(typeDependent, walker.typeDependent);
		updateMemberType();
	}
	void visit(cpp::postfix_expression_construct* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		type = makeUniqueTypeSafe(walker.type, Location(symbol->source, context.declarationCount));
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, Location(symbol->source, context.declarationCount), enclosingType);
		setExpressionType(symbol, type);
		updateMemberType();
		expression.isTemplateArgumentAmbiguity = symbol->args == 0;
	}
	void visit(cpp::postfix_expression_cast* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		expression = walker.expression;
		type = makeUniqueTypeSafe(walker.type, Location(symbol->source, context.declarationCount));
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, Location(symbol->source, context.declarationCount), enclosingType);
		if(symbol->op->id != cpp::cast_operator::DYNAMIC)
		{
			Dependent tmp(walker.typeDependent);
			addDependent(valueDependent, tmp);
		}
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		setExpressionType(symbol, type);
		updateMemberType();
	}
	void visit(cpp::postfix_expression_typeid* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: type = std::type_info
		// not dependent
		clearMemberType();
	}
	void visit(cpp::postfix_expression_typeidtype* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		// TODO: type = std::type_info
		// not dependent
		clearMemberType();
	}
	// suffix
	void visit(cpp::postfix_expression_subscript* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		id = 0; // don't perform overload resolution for a[i](x);
		// [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		type = removeReference(type);
		if(type.isArray()
			|| type.isPointer())
		{
			type.pop_front(); // dereference left-hand side
			// [expr.sub] The result is an lvalue of type T. The type "T" shall be a completely defined object type.
			requireCompleteObjectType(type, Location(symbol->source, context.declarationCount), enclosingType);
		}
		else // TODO: overloaded operator[]
		{
			// TODO: non-fatal error: attempting to dereference non-array/pointer
			type = gUniqueTypeNull;
		}
		setExpressionType(symbol, type);
		updateMemberType();
	}
	void visit(cpp::postfix_expression_call* symbol)
	{
		ArgumentListWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		setExpressionType(symbol, type);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		if(!isDependent(typeDependent)) // the expression is not dependent
			// TODO: check valueDependent too?
		{
			if(id != 0 // the prefix contains an id-expression
				&& isDecorated(*id) // TODO: assert!
				&& getDeclaration(*id) != &gDependentObject // the id-expression was not dependent
				&& isFunction(*getDeclaration(*id)) // the identifier names an overloadable function
				&& !isMemberOfTemplate(*getDeclaration(*id))) // the name of a member function of a template may be dependent: TODO: determine exactly when!
			{
				// TODO: 13.3.1.1.1  Call to named function
				FunctionOverload overload = findBestMatch(*id->dec.p, walker.arguments, Location(id->source, context.declarationCount), idEnclosing);
				if(overload.declaration != 0)
				{
					DeclarationInstanceRef instance = findLastDeclaration(*id->dec.p, overload.declaration);
					setDecoration(id, instance);
					//type = isDependent(overload.declaration->type) ? gUniqueTypeNull : getUniqueType(overload.declaration->type, Location(id->source, context.declarationCount), idEnclosing);
				}
				type = overload.type;
			}
		}
		else
		{
			if(id != 0)
			{
				id->dec.deferred = true;
			}
			type = gUniqueTypeNull;
		}
		// TODO: assert
		if(type.isFunction()) // the type of an expression naming a function is T()
		{
			type.pop_front(); // get the return type: T
		}
		// TODO: 13.3.1.1.2  Call to object of class type
		// TODO: set of pointers-to-function
		id = 0; // don't perform overload resolution for a(x)(x);
		updateMemberType();
	}
	void visit(cpp::postfix_expression_member* symbol)
	{
		PostfixExpressionMemberWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		setExpressionType(symbol, type);
		id = walker.id; // perform overload resolution for a.m(x);
		LookupResultRef declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isObject(*declaration))
			{
				// TODO: report non-fatal error
				//reportIdentifierMismatch(symbol, *id, declaration, "object-name");
				clearMemberType();
				return;
			}
			addDependentType(typeDependent, declaration);
			addDependentType(valueDependent, declaration);
			addDependentName(valueDependent, declaration);
			setDecoration(id, declaration);

			// TODO: overloaded operator->
			if(memberType != 0 // TODO: dependent member name lookup
				&& isPointer == walker.isArrow // TODO: report non-fatal error, dot vs arrow
				&& !isDependent(declaration->type)
				&& !isDependent(walker.qualifying.get_ref())
				&& !declaration->isTemplate // TODO: member template
				&& !declaration->type.isImplicitTemplateId) // TODO: 
			{
				// TODO: [expr.ref] inherit const/volatile from object-expression type if member is non-static
				idEnclosing = makeUniqueEnclosing(walker.qualifying, Location(id->source, context.declarationCount), memberType);
#if 0 // TODO: can this be removed?
				if(!enclosing->declaration->isTemplate) // if the left-hand side is not a template instantiation
				{
					SEMANTIC_ASSERT(enclosing->enclosing != 0);
					enclosing = enclosing->enclosing; // use its enclosing template-instantiation
				}
#endif
				SEMANTIC_ASSERT(declaration->scope->type == SCOPETYPE_CLASS);
				// the identifier may name a type in a base-class of the qualifying type; findEnclosingType resolves this.
				idEnclosing = findEnclosingType(idEnclosing, declaration->scope);
				SEMANTIC_ASSERT(idEnclosing != 0);
				type = getUniqueType(declaration->type, Location(id->source, context.declarationCount), idEnclosing);
			}
		}
		else
		{
			if(isDependent(walker.qualifying.get_ref()))
			{
				setDecoration(id, gDependentObjectInstance);
			}
			if(!walker.qualifying.empty())
			{
				setDependent(typeDependent, walker.qualifying.get());
			}
		}
		updateMemberType();
	}
	void visit(cpp::postfix_expression_destructor* symbol)
	{
		TREEWALKER_LEAF_SRC(symbol);
		setExpressionType(symbol, type);
		type = gVoid; // TODO: should this be null-type?
		id = 0;
		// TODO: name-lookup for destructor name
		clearMemberType();
	}
	void visit(cpp::postfix_operator* symbol)
	{
		TREEWALKER_LEAF_SRC(symbol);
		type = removeReference(type);
		// [expr.post.incr] The type of the operand shall be an arithmetic type or a pointer to a complete object type.
		if(type.isPointer())
		{
			type.pop_front();
			requireCompleteObjectType(type, Location(symbol->source, context.declarationCount), enclosingType);
		}
		setExpressionType(symbol, type);
		id = 0;
		updateMemberType();
	}
	void visit(cpp::postfix_expression_typetraits_unary* symbol)
	{
		TypeTraitsIntrinsicWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		type = gBool;
		UnaryTypeTraitsOp operation = getUnaryTypeTraitsOp(symbol->trait);
		Name name = getTypeTraitName(symbol);
		expression = ExpressionWrapper(makeExpression(TypeTraitsUnaryExpression(name, operation, walker.first)), true, false, isDependent(valueDependent));
	}
	void visit(cpp::postfix_expression_typetraits_binary* symbol)
	{
		TypeTraitsIntrinsicWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		type = gBool;
		BinaryTypeTraitsOp operation = getBinaryTypeTraitsOp(symbol->trait);
		Name name = getTypeTraitName(symbol);
		expression = ExpressionWrapper(makeExpression(TypeTraitsBinaryExpression(name, operation, walker.first, walker.second)), true, false, isDependent(valueDependent));
	}
};

struct SizeofTypeExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent valueDependent;
	ExpressionWrapper expression;
	SizeofTypeExpressionWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::type_id* symbol)
	{
		TypeIdWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		// [temp.dep.expr] Expressions of the following form [sizeof(T)] are never type-dependent (because the type of the expression cannot be dependent)
		// [temp.dep.constexpr] Expressions of the following form [sizeof(T)] are value-dependent if ... the type-id is dependent
		addDependent(valueDependent, walker.type);

		UniqueTypeId uniqueType = makeUniqueTypeSafe(walker.type, Location(symbol->source, context.declarationCount));
		setExpressionType(symbol, uniqueType);

		expression = ExpressionWrapper(makeExpression(SizeofTypeExpression(uniqueType)), true, false, isDependent(valueDependent));
	}
};

struct ConditionalExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWrapper left;
	ExpressionWrapper right;
	ConditionalExpressionWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		left = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		right = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct ExpressionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	UniqueTypeId type;
	ExpressionWrapper expression;
	/* 14.6.2.2-1
	...an expression is type-dependent if any subexpression is type-dependent.
	*/
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWalker(const WalkerState& state)
		: WalkerBase(state), id(0)
	{
	}

	// this path handles the right-hand side of a binary expression
	// it is assumed that 'type' already contains the type of the left-hand side
	template<typename T, typename TypeOp>
	void walkBinaryExpression(T*& symbol, TypeOp typeOp)
	{
		// TODO: SEMANTIC_ASSERT(walker.type.declaration != 0);
		ExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		// TODO: SEMANTIC_ASSERT(type.declaration != 0 && walker.type.declaration != 0);
		BinaryIceOp iceOp = getBinaryIceOp(symbol);
		expression = ExpressionWrapper(
			makeExpression(BinaryExpression(getBinaryOperatorName(symbol), iceOp, typeOp, expression, walker.expression)),
			expression.isConstant && walker.expression.isConstant && iceOp != 0,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		if(!expression.isTypeDependent)
		{
			type = typeOp(type, walker.type); // TODO: call typeofExpression
			// TODO: conditional-expression: SYMBOLS_ASSERT(type != gUniqueTypeNull);
		}
		ExpressionType<T>::set(symbol, type);
	}
	template<typename T>
	void walkBinaryArithmeticExpression(T* symbol)
	{
		walkBinaryExpression(symbol, binaryOperatorArithmeticType);
		// TODO: overloaded arithmetic operators
	}
	template<typename T>
	void walkBinaryAdditiveExpression(T* symbol)
	{
		walkBinaryExpression(symbol, binaryOperatorAdditiveType);
		// TODO: overloaded arithmetic operators
	}
	template<typename T>
	void walkBinaryIntegralExpression(T* symbol)
	{
		walkBinaryExpression(symbol, binaryOperatorIntegralType);
		// TODO: overloaded shift operators
	}
	template<typename T>
	void walkBinaryBooleanExpression(T* symbol)
	{
		walkBinaryExpression(symbol, binaryOperatorBoolean);
		// TODO: overloaded boolean operators
	}
	void visit(cpp::assignment_expression_suffix* symbol)
	{
		// 5.1.7 Assignment operators
		// the type of an assignment expression is that of its left operand
		walkBinaryExpression(symbol, binaryOperatorAssignment);
	}
	void visit(cpp::conditional_expression_suffix* symbol)
	{
		ConditionalExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = ExpressionWrapper(
			makeExpression(TernaryExpression(conditional, ternaryOperatorNull, expression, walker.left, walker.right)),
			expression.isConstant && walker.left.isConstant && walker.right.isConstant,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		type = gUniqueTypeNull; // TODO
	}
	void visit(cpp::logical_or_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::logical_and_expression_default* symbol)
	{
		walkBinaryBooleanExpression(symbol);
	}
	void visit(cpp::inclusive_or_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::exclusive_or_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::and_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::equality_expression_default* symbol)
	{
		walkBinaryBooleanExpression(symbol);
	}
	void visit(cpp::relational_expression_default* symbol)
	{
		walkBinaryBooleanExpression(symbol);
	}
	void visit(cpp::shift_expression_default* symbol)
	{
		walkBinaryIntegralExpression(symbol);
	}
	void visit(cpp::additive_expression_default* symbol)
	{
		walkBinaryAdditiveExpression(symbol);
	}
	void visit(cpp::multiplicative_expression_default* symbol)
	{
		walkBinaryArithmeticExpression(symbol);
	}
	void visit(cpp::pm_expression_default* symbol)
	{
		walkBinaryExpression(symbol, binaryOperatorNull);
		// TODO: determine type of pm expression
	}
#if 0
	void visit(cpp::assignment_expression_default* symbol)
	{
		TREEWALKER_LEAF_SRC(symbol);
		setExpressionType(symbol, type);
	}
#endif
	void visit(cpp::assignment_expression* symbol) // expression_list, assignment_expression_suffix, conditional_expression_suffix
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		// [expr.comma] The type and value of the result are the type and value of the right operand
		expression = walker.expression;
		type = walker.type;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		setExpressionType(symbol, type);
	}
	void visit(cpp::expression_list* symbol) // a comma-separated list of assignment_expression
	{
		TREEWALKER_LEAF(symbol);
		setExpressionType(symbol, type);
	}
	void visit(cpp::postfix_expression* symbol)
	{
		PostfixExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		id = walker.id;
		type.swap(walker.type);
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		//setDependent(dependent, walker.dependent); // TODO:
		setExpressionType(symbol, type);
	}
	void visit(cpp::unary_expression_op* symbol)
	{
		Source source = parser->get_source();
		TREEWALKER_LEAF_SRC(symbol); 
		if(type.value != UNIQUETYPE_NULL) // TODO: assert
		{
			Identifier id;
			id.value = getUnaryOperatorName(symbol->op);
			id.source = source;
			FunctionOverload overload = findBestOverloadedOperator(id, type);
			if(overload.declaration == &gUnknown)
			{
				if(symbol->op->id == cpp::unary_operator::AND
					&& expression.isQualifiedNonStaticMemberName)
				{
					// [expr.unary.op]
					// The result of the unary & operator is a pointer to its operand. The operand shall be an lvalue or a qualified-id.
					// In the first case, if the type of the expression is “T,” the type of the result is “pointer to T.” In particular,
					// the address of an object of type “cv T” is “pointer to cv T,” with the same cv-qualifiers.
					// For a qualified-id, if the member is a static member of type “T”, the type of the result is plain “pointer to T.”
					// If the member is a non-static member of class C of type T, the type of the result is “pointer to member of class C of type
					// T.”
					UniqueTypeWrapper classType = makeUniqueObjectType(*getIdExpression(expression).enclosing);
					type.push_front(MemberPointerType(classType)); // produces a non-const pointer
				}
				else
				{
					type = getBuiltInUnaryOperatorReturnType(symbol->op, type);
				}
			}
			else
			{
				SEMANTIC_ASSERT(overload.declaration != 0);
				type = overload.type;
			}
			// TODO: decorate parse-tree with declaration
		}
		else
		{
			type = gUniqueTypeNull;
		}


		UnaryIceOp iceOp = getUnaryIceOp(symbol);
		expression = ExpressionWrapper(
			makeExpression(UnaryExpression(symbol->op->value.value, iceOp, 0, expression)),
			expression.isConstant && iceOp != 0,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		setExpressionType(symbol, type);
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
	void visit(cpp::new_expression_placement* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		type = makeUniqueTypeSafe(walker.type, Location(symbol->source, context.declarationCount));
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		requireCompleteObjectType(type, Location(symbol->source, context.declarationCount), enclosingType);
		type.push_front(PointerType());
		addDependent(typeDependent, walker.typeDependent);
		setExpressionType(symbol, type);
	}
	void visit(cpp::new_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		type = makeUniqueTypeSafe(walker.type, Location(symbol->source, context.declarationCount));
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		requireCompleteObjectType(type, Location(symbol->source, context.declarationCount), enclosingType);
		type.push_front(PointerType());
		addDependent(typeDependent, walker.typeDependent);
		setExpressionType(symbol, type);
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		ExplicitTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		expression = walker.expression;
		type = makeUniqueTypeSafe(walker.type, Location(symbol->source, context.declarationCount));
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, Location(symbol->source, context.declarationCount), enclosingType);
		Dependent tmp(walker.typeDependent);
		addDependent(valueDependent, tmp);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		setExpressionType(symbol, type);
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
		TREEWALKER_WALK_SRC(walker, symbol);
		// [temp.dep.expr] Expressions of the following form [sizeof(expr)] are never type-dependent (because the type of the expression cannot be dependent)
		// [temp.dep.constexpr] Expressions of the following form [sizeof(expr)] are value-dependent if the unary-expression is type-dependent
		addDependent(valueDependent, walker.typeDependent);
		type = gUnsignedInt;
		setExpressionType(symbol, type);
		expression = ExpressionWrapper(makeExpression(SizeofExpression(walker.expression)), true, false, isDependent(valueDependent));
	}
	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		SizeofTypeExpressionWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		type = gUnsignedInt;
		setExpressionType(symbol, type);
		expression = walker.expression;
	}
	void visit(cpp::delete_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = gVoid;
		setExpressionType(symbol, type);
		expression = ExpressionWrapper();
	}
	void visit(cpp::throw_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type = gUniqueTypeNull; // throw-expression has no type
		expression = ExpressionWrapper();
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
	IsHiddenTypeName filter; // allows type-name to be parsed without knowing whether it is the prefix of a nested-name-specifier (in which case it cannot be hidden by a non-type name)
	bool isTypename; // true if a type is expected in this context; e.g. following 'typename', preceding '::'
	TypeNameWalker(const WalkerState& state, bool isTypename = false)
		: WalkerBase(state), type(0, context), isTypename(isTypename)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		LookupResultRef declaration = gDependentTypeInstance;
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
		type.id = &symbol->value;
		type.declaration = declaration;
		type.isImplicitTemplateId = declaration->isTemplate;
		type.isEnclosingClass = isClass(*declaration)
			&& isComplete(*declaration)
			&& findScope(enclosing, declaration->enclosed); // is this the type of an enclosing class?
		setDecoration(&symbol->value, declaration);
		setDependent(type);
#if 1 // temp hack, imitate previous isDependent behaviour
		if(declaration->isTemplate
			&& declaration->templateParameter == INDEX_INVALID) // ignore template-template-parameter
		{
			if(declaration->isSpecialization)
			{
				setDependent(type.dependent, declaration->templateArguments);
			}
			else
			{
				SEMANTIC_ASSERT(!declaration->templateParams.empty());
				setDependent(type.dependent, *declaration->templateParams.front().declaration); // depend on first template param
			}
		}
#endif
	}

	void visit(cpp::simple_template_id* symbol)
	{
		//ProfileScope profile(gProfileTemplateId);

		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		LookupResultRef declaration = lookupTemplate(*walker.id, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "type-name");
		}
		if(declaration == &gDependentTemplate
			&& !isTypename)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}

		setDecoration(walker.id, declaration);
		type.id = walker.id;
		type.declaration = declaration;
		type.templateArguments.swap(walker.arguments);
		setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of its arguments are dependent
	}
};

struct NestedNameSpecifierSuffixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool isDeclarator;
	bool isTemplate;
	NestedNameSpecifierSuffixWalker(const WalkerState& state, bool isDeclarator = false)
		: WalkerBase(state), type(0, context), isDeclarator(isDeclarator), isTemplate(false)
	{
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isTemplate = true;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF_CACHED(symbol);
		LookupResultRef declaration = gDependentNestedInstance;
		if(isDeclarator
			|| !isDependent(qualifying_p))
		{
			declaration = findDeclaration(symbol->value, isDeclarator, IsNestedName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "nested-name");
			}
		}
		type = declaration;
		type.id = &symbol->value;
		setDecoration(&symbol->value, declaration);
		if(declaration != &gDependentNested)
		{
			setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		}
	}
	void visit(cpp::simple_template_id* symbol)
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		LookupResultRef declaration = gDependentNestedTemplateInstance;
		if(!isTemplate // TODO: should perform name lookup anyway, even if 'qualifying_p' not dependent!
			&& (isDeclarator
				|| !isDependent(qualifying_p)))
		{
			declaration = findDeclaration(*walker.id, isDeclarator, IsNestedName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "nested-name");
			}
		}
		type = declaration;
		type.id = walker.id;
		type.templateArguments.swap(walker.arguments);
		if(declaration != &gDependentNestedTemplate)
		{
			setDependent(type); // a template-id is dependent if the 'identifier' is a template-parameter
		}
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of its arguments are dependent
	}
};

// basic.lookup.qual
// During the lookup for a name preceding the :: scope resolution operator, object, function, and enumerator names are ignored.
struct NestedNameSpecifierPrefixWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	bool isDeclarator;
	NestedNameSpecifierPrefixWalker(const WalkerState& state, bool isDeclarator = false)
		: WalkerBase(state), type(0, context), isDeclarator(isDeclarator)
	{
	}

#if 0 // for debugging parse-tree cache
	void visit(cpp::nested_name* symbol)
	{
		NestedNameSpecifierPrefixWalker walker(getState(), isDeclarator);
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
		if(isDeclarator
			&& !isClass(*walker.type.declaration))
		{
			// the prefix of the nested-name-specifier in a qualified declarator-id must be a class-name (not a typedef)
			return reportIdentifierMismatch(symbol, walker.type.declaration->getName(), walker.type.declaration, "class-name");
		}
		type.swap(walker.type);
		makeUniqueTypeSafe(type, getLocation());
	}
};

struct NestedNameSpecifierWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	bool isDeclarator;
	NestedNameSpecifierWalker(const WalkerState& state, bool isDeclarator = false)
		: WalkerQualified(state), isDeclarator(isDeclarator)
	{
	}
	void visit(cpp::nested_name_specifier_prefix* symbol)
	{
		NestedNameSpecifierPrefixWalker walker(getState(), isDeclarator);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		makeUniqueTypeSafe(walker.type, getLocation());
		swapQualifying(walker.type, isDeclarator);
		//disableBacktrack();
	}
	void visit(cpp::nested_name_specifier_suffix_template* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(getState(), isDeclarator);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		walker.type.qualifying.swap(qualifying);
		setDependent(walker.type.dependent, walker.type.qualifying);
		makeUniqueTypeSafe(walker.type, getLocation());
		swapQualifying(walker.type, isDeclarator);
		//disableBacktrack();
	}
	void visit(cpp::nested_name_specifier_suffix_default* symbol)
	{
		NestedNameSpecifierSuffixWalker walker(getState(), isDeclarator);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		walker.type.qualifying.swap(qualifying);
		setDependent(walker.type.dependent, walker.type.qualifying);
		makeUniqueTypeSafe(walker.type, getLocation());
		swapQualifying(walker.type, isDeclarator);
		//disableBacktrack();
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
			// 3.3.7: a type-name can be hidden by a non-type name in the same scope (this rule applies to a type-specifier)
			return reportIdentifierMismatch(symbol, walker.filter.nonType->getName(), walker.filter.nonType, "type-name");
		}
		SEMANTIC_ASSERT(walker.type.declaration != 0);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
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
		TREEWALKER_WALK_CACHED(walker, symbol);
		IsHiddenTypeName filter;
		LookupResultRef declaration = lookupTemplate(*walker.id, makeLookupFilter(filter));
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, declaration, "type-name");
		}
		if(declaration == &gDependentTemplate)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}
		if(filter.nonType != 0)
		{
			// 3.3.7: a type-name can be hidden by a non-type name in the same scope (this rule applies to a type-specifier)
			return reportIdentifierMismatch(symbol, filter.nonType->getName(), filter.nonType, "type-name");
		}
		setDecoration(walker.id, declaration);
		type.declaration = declaration;
		type.templateArguments.swap(walker.arguments);
		type.qualifying.swap(qualifying);
		setDependent(type); // a template-id is dependent if the template-name is a template-parameter
		setDependent(type.dependent, type.templateArguments); // a template-id is dependent if any of the template arguments are dependent
		setDependent(type.dependent, type.qualifying);
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		fundamental = combineFundamental(0, symbol->id);
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
	void visit(cpp::template_id* symbol) 
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		id = walker.id;
	}
	void visit(cpp::operator_function_id* symbol) 
	{
		Source source = parser->get_source();
		FilePosition position = parser->get_position();
		OperatorFunctionIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		symbol->value.value = walker.name;
		symbol->value.source = source;
		id = &symbol->value;
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
		NestedNameSpecifierWalker walker(getState(), true); // in a template member definition, the qualifying nested-name-specifier may be dependent on a template-parameter
		TREEWALKER_WALK(walker, symbol); // no need to cache: the nested-name-specifier is not a shared-prefix
		swapQualifying(walker.qualifying, true);
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
		swapQualifying(walker.qualifying, true);
	}
	void visit(cpp::qualified_id_global* symbol)
	{
		QualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		swapQualifying(walker.qualifying, true);
	}
	void visit(cpp::unqualified_id* symbol)
	{
		UnqualifiedDeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
	}
};

struct ParameterDeclarationListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Parameters parameters;

	ParameterDeclarationListWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::parameter_declaration* symbol)
	{
		ParameterDeclarationWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		if(!isVoidParameter(walker.declaration->type))
		{
			parameters.push_back(Parameter(walker.declaration, walker.defaultArgument));
		}
	}
};

struct ParameterDeclarationClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Parameters parameters;

	ParameterDeclarationClauseWalker(const WalkerState& state)
		: WalkerBase(state)
	{
		pushScope(newScope(makeIdentifier("$prototype"), SCOPETYPE_PROTOTYPE));
		if(templateParamScope != 0)
		{
			// insert the template-parameter scope to enclose the declarator scope
			templateParamScope->parent = enclosing->parent;
			enclosing->parent = templateParamScope;
			enclosing->templateDepth = templateParamScope->templateDepth;
		}
		clearTemplateParams();
	}

	void visit(cpp::parameter_declaration_list* symbol)
	{
		ParameterDeclarationListWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		parameters = walker.parameters;
	}
	void visit(cpp::terminal<boost::wave::T_ELLIPSIS> symbol)
	{
		parameters.isEllipsis = true;
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

struct CvQualifierSeqWalker : WalkerBase
{
	TREEWALKER_DEFAULT;

	CvQualifiers qualifiers;
	CvQualifierSeqWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}
	void visit(cpp::cv_qualifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::cv_qualifier::CONST)
		{
			qualifiers.isConst = true;
		}
		else if(symbol->id == cpp::cv_qualifier::VOLATILE)
		{
			qualifiers.isVolatile = true;
		}
	}

};

struct PtrOperatorWalker : public WalkerQualified
{
	TREEWALKER_DEFAULT;

	CvQualifiers qualifiers;
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
	void visit(cpp::cv_qualifier_seq* symbol)
	{
		CvQualifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		qualifiers = walker.qualifiers;
	}
};

struct DeclaratorFunctionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ScopePtr paramScope;
	Parameters parameters;
	CvQualifiers qualifiers;
	DeclaratorFunctionWalker(const WalkerState& state)
		: WalkerBase(state), paramScope(0)
	{
	}

	void visit(cpp::parameter_declaration_clause* symbol)
	{
		ParameterDeclarationClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		paramScope = walker.enclosing; // store reference for later resumption
		parameters = walker.parameters;
	}
	void visit(cpp::exception_specification* symbol)
	{
		ExceptionSpecificationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	void visit(cpp::cv_qualifier_seq* symbol)
	{
		CvQualifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		qualifiers = walker.qualifiers;
	}
};

struct DeclaratorArrayWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Dependent valueDependent;
	ArrayRank rank;
	ExpressionWrapper expression;
	DeclaratorArrayWalker(const WalkerState& state)
		: WalkerBase(state)
	{
	}

	void visit(cpp::terminal<boost::wave::T_LEFTBRACKET> symbol)
	{
		// we may parse multiple pairs of brackets: omitted constant-expression indicates an array of unknown size
		expression = ExpressionWrapper();
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		// [temp.dep.constexpr] An identifier is value-dependent if it is:- a constant with integral or enumeration type and is initialized with an expression that is value-dependent.
		addDependent(valueDependent, walker.valueDependent);
		expression = walker.expression;
	}
	void visit(cpp::terminal<boost::wave::T_RIGHTBRACKET> symbol)
	{
		rank.push_back(expression);
	}
};

struct DeclaratorWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	UniqueTypeWrapper qualifying;
	ScopePtr paramScope;
	Dependent valueDependent;
	TypeSequence typeSequence;
	CvQualifiers qualifiers;
	Qualifying memberPointer;
	Dependent dependent; // track which template parameters the declarator's type depends on. e.g. 'T::* memberPointer', 'void f(T)'
	DeclaratorWalker(const WalkerState& state)
		: WalkerBase(state), id(&gAnonymousId), paramScope(0), typeSequence(context), memberPointer(context)
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

	void visit(cpp::ptr_operator* symbol)
	{
		PtrOperatorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		qualifiers = walker.qualifiers;
		memberPointer.swap(walker.qualifying);
	}
	template<typename T>
	void walkDeclaratorPtr(T* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol); // if parse fails, state of typeSeqence is not modified.
		id = walker.id;
		qualifying = walker.qualifying;
		enclosing = walker.enclosing;
		paramScope = walker.paramScope;
		addDependent(dependent, walker.dependent);
		addDependent(valueDependent, walker.valueDependent);
		SYMBOLS_ASSERT(typeSequence.empty());
		typeSequence = walker.typeSequence;

		qualifiers = walker.qualifiers;
		memberPointer.swap(walker.memberPointer);
		pushPointerType(symbol->op);
	}
	void visit(cpp::declarator_ptr* symbol)
	{
		return walkDeclaratorPtr(symbol);
	}
	void visit(cpp::abstract_declarator_ptr* symbol)
	{
		return walkDeclaratorPtr(symbol);
	}
	void visit(cpp::new_declarator_ptr* symbol)
	{
		return walkDeclaratorPtr(symbol);
	}
	void visit(cpp::declarator_id* symbol)
	{
		DeclaratorIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		qualifying = walker.qualifying.empty() || isNamespace(*walker.qualifying.back().declaration)
			? gUniqueTypeNull : UniqueTypeWrapper(walker.qualifying.back().unique);

		if(walker.getQualifyingScope()
			&& enclosing->type != SCOPETYPE_CLASS) // in 'class C { friend void Q::N(X); };' X should be looked up in the scope of C rather than Q
		{
			enclosing = walker.getQualifyingScope(); // names in declarator suffix (array-size, parameter-declaration) are looked up in declarator-id's qualifying scope
			if(qualifying != gUniqueTypeNull)
			{
				enclosingType = &getObjectType(qualifying.value);
			}
		}
	}
	template<typename T>
	void walkDeclaratorArray(T* symbol)
	{
		DeclaratorArrayWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		typeSequence.push_front(DeclaratorArrayType(walker.rank));
	}
	void visit(cpp::declarator_suffix_array* symbol)
	{
		return walkDeclaratorArray(symbol);
	}
	void visit(cpp::declarator_suffix_function* symbol)
	{
		DeclaratorFunctionWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		if(paramScope == 0) // only interested in the innermost parameter-list
		{
			paramScope = walker.paramScope;
		}
		typeSequence.push_front(DeclaratorFunctionType(walker.parameters, walker.qualifiers));
		setDependent(dependent, walker.parameters);
	}
	void visit(cpp::new_declarator_suffix* symbol)
	{
		return walkDeclaratorArray(symbol);
	}
	void visit(cpp::expression* symbol) // in direct_new_declarator
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
	template<typename T>
	void walkDeclarator(T* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		id = walker.id;
		qualifying = walker.qualifying;
		enclosing = walker.enclosing;
		paramScope = walker.paramScope;
		addDependent(dependent, walker.dependent);
		addDependent(valueDependent, walker.valueDependent);
		SYMBOLS_ASSERT(typeSequence.empty());
		typeSequence = walker.typeSequence;
	}
	void visit(cpp::direct_abstract_declarator* symbol)
	{
		return walkDeclarator(symbol); // if parse fails, state of typeSeqence is not modified. e.g. type-id: int((int))
	}
	void visit(cpp::direct_abstract_declarator_parenthesis* symbol)
	{
		return walkDeclarator(symbol); // if parse fails, state of typeSeqence is not modified. e.g. function-style-cast type-id followed by parenthesised expression: int(*this)
	}
	void visit(cpp::direct_new_declarator* symbol)
	{
		return walkDeclarator(symbol);
	}
	void visit(cpp::declarator* symbol)
	{
		return walkDeclarator(symbol);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		return walkDeclarator(symbol);
	}
	void visit(cpp::new_declarator* symbol)
	{
		return walkDeclarator(symbol);
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
		/* [class.derived]
		The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
		called a direct base class for the class being defined. During the lookup for a base class name, non-type
		names are ignored (3.3.10)
		*/
		TypeNameWalker walker(getState(), true);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifying.swap(qualifying);
		setDependent(type.dependent, type.qualifying);
		makeUniqueTypeSafe(type, getLocation());
	}
};

struct ClassHeadWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	TemplateArguments arguments;
	bool isUnion;
	bool isSpecialization;
	ClassHeadWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), arguments(context), isUnion(false), isSpecialization(false)
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

		// resolve the (possibly dependent) qualifying scope
		if(walker.getDeclaratorQualifying() != 0)
		{
			if(enclosing == templateEnclosing)
			{
				templateEnclosing = walker.getDeclaratorQualifying()->enclosed;
			}
			enclosing = walker.getDeclaratorQualifying()->enclosed; // names in declaration of nested-class are looked up in scope of enclosing class
		}
	}
	void visit(cpp::simple_template_id* symbol) // class_name
	{
		TemplateIdWalker walker(getState());
		TREEWALKER_WALK_CACHED(walker, symbol);
		// TODO: don't declare anything - this is a template (partial) specialisation
		id = walker.id;
		arguments.swap(walker.arguments);
		isSpecialization = true;
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until ':' is discovered
		// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
		declaration = declareClass(id, isSpecialization, arguments);
	}
	void visit(cpp::base_specifier* symbol) 
	{
		BaseSpecifierWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		if(walker.type.declaration != 0) // declaration == 0 if base-class is dependent
		{
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			addBase(declaration, walker.type);
		}
		SEMANTIC_ASSERT(walker.type.unique != 0);
		setExpressionType(symbol, walker.type.isDependent ? gUniqueTypeNull : UniqueTypeWrapper(walker.type.unique));
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
			LookupResultRef declaration = walker.declaration;
			if(declaration == &gUndeclared
				|| !(isObject(*declaration) || isTypeName(*declaration)))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "object-name or type-name");
			}

			setDecoration(walker.id, declaration); // refer to the primary declaration of this name, rather than the one declared by this using-declaration
			
			DeclarationInstance instance(declaration);
			instance.name = walker.id;
			instance.overloaded = declaration.p;
			instance.redeclared = declaration.p;
			DeclarationInstanceRef redeclaration = enclosing->declarations.insert(instance);
			enclosing->declarationList.push_back(instance);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(redeclaration);
#endif

		}
		else
		{
			// TODO: introduce typename into enclosing namespace
			setDecoration(walker.id, gDependentTypeInstance);
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
			LookupResultRef declaration = findDeclaration(symbol->value, IsNamespaceName());
			if(declaration == &gUndeclared)
			{
				return reportIdentifierMismatch(symbol, symbol->value, declaration, "namespace-name");
			}

			// TODO: check for conflicts with earlier declarations
			declaration = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, declaration->enclosed);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(declaration);
#endif
			setDecoration(id, declaration);
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
	bool isSpecialization;
	ClassSpecifierWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), id(0), arguments(context), isUnion(false), isSpecialization(false)
	{
	}

	void visit(cpp::class_head* symbol)
	{
		ClassHeadWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		id = walker.id;
		isUnion = walker.isUnion;
		isSpecialization = walker.isSpecialization;
		arguments.swap(walker.arguments);
		enclosing = walker.enclosing;
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol)
	{
		// defer class declaration until we know this is a class-specifier - it may be an elaborated-type-specifier until '{' is discovered
		if(declaration == 0)
		{
			// 3.3.1.3 The point of declaration for a class first declared by a class-specifier is immediately after the identifier or simple-template-id (if any) in its class-head
			declaration = declareClass(id, isSpecialization, arguments);
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

		Location source = getLocation();
		Type type(declaration, context);
		type.id = &declaration->getName();
		setDependent(type);
		type.isDependent = isDependent(type);
		type.isImplicitTemplateId = declaration->isTemplate;
		type.isEnclosingClass = true;
		bool isExplicitSpecialization = isSpecialization && declaration->templateParams.empty();
		bool allowDependent = type.isDependent || (declaration->isTemplate && !isExplicitSpecialization); // prevent uniquing of template-arguments in implicit template-id
		declaration->type.isDependent = type.isDependent;
		declaration->type.unique = makeUniqueType(type, source, enclosingType, allowDependent).value;
		enclosingType = &getObjectType(declaration->type.unique);
		const_cast<TypeInstance*>(enclosingType)->declaration = declaration; // if this is a specialization, use the specialization instead of the primary template
		instantiateClass(*enclosingType, source, 0, allowDependent); // instantiate non-dependent base classes

		addDependent(enclosingDependent, type);

		clearTemplateParams();
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
		declaration->isComplete = true;
		parseDeferred(deferred.first, *this);
		parseDeferred(deferred.second, *this);
	}
};

struct EnumeratorDefinitionWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	bool isInitialized;
	EnumeratorDefinitionWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), isInitialized(false)
	{
	}

	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		/* 3.1-4
		The point of declaration for an enumerator is immediately after its enumerator-definition.
		*/
		// TODO: give enumerators a type
		DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, symbol->value, TYPE_ENUMERATOR, 0, DeclSpecifiers());
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(instance);
#endif
		setDecoration(&symbol->value, instance);
		declaration = instance;
	}
	void visit(cpp::constant_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		declaration->initializer = walker.expression;
		isInitialized = true;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
};

struct EnumSpecifierWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	IdentifierPtr id;
	ExpressionWrapper value;
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
			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_ENUM, 0);
			setDecoration(id, instance);
			declaration = instance;
		}
		// [dcl.enum] If the first enumerator has no initializer, the value of the corresponding constant is zero.
		value = makeExpression(IntegralConstantExpression(gSignedInt, IntegralConstant(0))); // TODO: [dcl.enum] underlying type of enumerator
	}

	void visit(cpp::enumerator_definition* symbol)
	{
		if(declaration == 0)
		{
			// unnamed enum
			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, enclosing->getUniqueName(), TYPE_ENUM, 0);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(instance);
#endif
			declaration = instance;
		}
		EnumeratorDefinitionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.isInitialized)
		{
			SEMANTIC_ASSERT(isDependent(walker.declaration->valueDependent) || walker.declaration->initializer.isConstant);
			value = walker.declaration->initializer;
		}
		else
		{
			walker.declaration->initializer = value;
		}
		// [dcl.enum] An enumerator-definition without an initializer gives the enumerator the value obtained by increasing the value of the previous enumerator by one.
		ExpressionPtr one = makeExpression(IntegralConstantExpression(gSignedInt, IntegralConstant(1)));
		value = ExpressionWrapper(
			makeExpression(BinaryExpression(Name("+"), operator+, binaryOperatorAssignment, value, one)),
			true, value.isTypeDependent, value.isValueDependent
		);
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
		TREEWALKER_WALK_CACHED(walker, symbol);
		// 3.4.4-2: when looking up 'identifier' in elaborated-type-specifier, ignore any non-type names that have been declared. 
		LookupResultRef declaration = lookupTemplate(*walker.id, IsTypeName());
		if(declaration == &gUndeclared)
		{
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "type-name");
		}
		if(declaration == &gDependentTemplate)
		{
			// dependent type, are you missing a 'typename' keyword?
			return reportIdentifierMismatch(symbol, *walker.id, &gUndeclared, "typename");
		}
		setDecoration(walker.id, declaration);
		id = walker.id;
		type.declaration = declaration;
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
		LookupResultRef declaration = findDeclaration(symbol->value, IsTypeName());
		if(declaration == &gUndeclared // if there is no existing declaration
			|| isTypedef(*declaration) // or the existing declaration is a typedef
			|| declaration->isTemplate // or the existing declaration is a template class
			|| enclosing == templateEnclosing // or we are forward-declaring a template class
			|| (key == &gClass && declaration->scope == getEtsScope())) // or this is a forward-declaration of a class/struct
		{
			if(key != &gClass)
			{
				SEMANTIC_ASSERT(key == &gEnum);
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier refers to undefined enum" << std::endl;
				throw SemanticError();
			}
			type = key;
		}
		else
		{
#if 0 // elaborated type specifier cannot refer to a template in a different scope - this case will be treated as a redeclaration
			// template<typename T> class C
			if(declaration->isSpecialization) // if the lookup found a template explicit/partial-specialization
			{
				SEMANTIC_ASSERT(declaration->isTemplate);
				declaration = findPrimaryTemplateLastDeclaration(declaration); // the name is a plain identifier, not a template-id, therefore the name refers to the primary template
			}
#endif
			setDecoration(&symbol->value, declaration);
			/* [dcl.type.elab]
			3.4.4 describes how name lookup proceeds for the identifier in an elaborated-type-specifier. If the identifier
			resolves to a class-name or enum-name, the elaborated-type-specifier introduces it into the declaration the
			same way a simple-type-specifier introduces its type-name. If the identifier resolves to a typedef-name, the
			elaborated-type-specifier is ill-formed.
			*/
#if 0 // allow hiding a typedef with a forward-declaration
			if(isTypedef(*declaration))
			{
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier refers to a typedef" << std::endl;
				printPosition(declaration->getName().source);
				throw SemanticError();
			}
#endif
			/* 7.1.6.3-3
			The class-key or enum keyword present in the elaborated-type-specifier shall agree in kind with the declaration
			to which the name in the elaborated-type-specifier refers.
			*/
			if(declaration->type.declaration != key)
			{
				printPosition(symbol->value.source);
				std::cout << "'" << symbol->value.value.c_str() << "': elaborated-type-specifier key does not match declaration" << std::endl;
				printPosition(declaration->getName().source);
				throw SemanticError();
			}
			type = declaration;
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

	void visit(cpp::terminal<boost::wave::T_TYPENAME> symbol)
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
		setDependent(type.dependent, type.qualifying);
	}
};

struct DeclSpecifierSeqWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	Type type;
	unsigned fundamental;
	DeclSpecifiers specifiers;
	CvQualifiers qualifiers;
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
			fundamental = walker.fundamental;
			type = getFundamentalType(fundamental);
		}
	}
	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		TREEWALKER_LEAF(symbol);
		fundamental = combineFundamental(fundamental, symbol->id);
		type = getFundamentalType(fundamental);
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
		templateParams = walker.templateParams;
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
	void visit(cpp::cv_qualifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(symbol->id == cpp::cv_qualifier::CONST)
		{
			qualifiers.isConst = true;
		}
		else if(symbol->id == cpp::cv_qualifier::VOLATILE)
		{
			qualifiers.isVolatile = true;
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
		SEMANTIC_ASSERT(walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
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
		LookupResultRef declaration = findDeclaration(symbol->value);
		if(declaration == &gUndeclared
			|| !isObject(*declaration))
		{
			return reportIdentifierMismatch(symbol, symbol->value, declaration, "object-name");
		}
		setDecoration(&symbol->value, declaration);
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
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
	}
};

struct TypeIdWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TypeId type;
	TypeIdWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context)
	{
	}
	void visit(cpp::terminal<boost::wave::T_OPERATOR> symbol) 
	{
		 // for debugging purposes
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifiers = walker.qualifiers;
		declareEts(type, walker.forward);
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.typeSequence = walker.typeSequence;
		// [temp.dep.type] A type is dependent if it is a compound type constructed from any dependent type
		setDependent(type.dependent, walker.dependent);
		// [temp.dep.type] A type is dependent if it is an array type constructed from any dependent type or whose size is specified by a constant expression that is value-dependent
		setDependent(type.dependent, walker.valueDependent);
	}
};

struct NewTypeWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TypeId type;
	Dependent valueDependent;
	NewTypeWalker(const WalkerState& state)
		: WalkerBase(state), type(0, context)
	{
	}
	void visit(cpp::terminal<boost::wave::T_OPERATOR> symbol) 
	{
		// for debugging purposes
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifiers = walker.qualifiers;
		declareEts(type, walker.forward);
	}
	void visit(cpp::new_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		addDependent(valueDependent, walker.valueDependent);
		type.typeSequence = walker.typeSequence;
		setDependent(type.dependent, walker.dependent);
		// new T
		// new T*
		// new T[variable]
		// new T[variable][constant]
		// new T*[variable]
		// new T C::*
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
		LookupResultRef declaration = context.findDeclaration(id);
		return declaration != &gUndeclared && isTemplateName(*declaration);
	}
};

struct InitializerWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	ExpressionWrapper expression;
	Dependent valueDependent;
	InitializerWalker(const WalkerState& state) : WalkerBase(state)
	{
	}
	void visit(cpp::assignment_expression* symbol)
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
};

struct SimpleDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	ScopePtr parent;
	IdentifierPtr id;
	TypeId type;
	Dependent typeDependent;
	ScopePtr enclosed;
	DeclSpecifiers specifiers;
	IdentifierPtr forward;

	DeferredSymbols deferred;

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
			DeclarationPtr tmpDependent = type.dependent;
			setDependent(type.dependent, typeDependent);
			makeUniqueTypeSafe(type, getLocation());
			if(enclosed == 0
				&& templateParamScope != 0)
			{
				templateParamScope->parent = parent;
				enclosed = templateParamScope; // for a static-member-variable definition, store template-params with different names than those in the class definition
			}
			declaration = declareObject(parent, id, type, enclosed, specifiers, templateParameter, valueDependent);

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

	void visit(cpp::decl_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState(), templateParameter != INDEX_INVALID);
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifiers = walker.qualifiers;
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		specifiers = walker.specifiers;
		forward = walker.forward;
		templateParams = walker.templateParams;
		isUnion = walker.isUnion;
	}
	void visit(cpp::type_specifier_seq* symbol)
	{
		DeclSpecifierSeqWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		type.swap(walker.type);
		type.qualifiers = walker.qualifiers;
		declaration = type.declaration; // if no declarator is specified later, this is probably a class-declaration
		forward = walker.forward;
		templateParams = walker.templateParams;
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
			// In this situation, 'deferred' contains the reference to the deferred expression.
			walker.deferred = &deferred;
		}

		TREEWALKER_WALK_CACHED(walker, symbol);
		parent = walker.enclosing; // if the id-expression in the declarator is a qualified-id, this is the qualifying scope
		id = walker.id;
		enclosed = walker.paramScope;
		type.typeSequence = walker.typeSequence;
		addDependent(typeDependent, walker.dependent);
		/* temp.dep.constexpr
		An identifier is value-dependent if it is:
			— a name declared with a dependent type,
			— the name of a non-type template parameter,
			— a constant with effective literal type and is initialized with an expression that is value-dependent.
		*/
		addDependent(valueDependent, walker.valueDependent);

		if(walker.qualifying != gUniqueTypeNull)
		{
			SEMANTIC_ASSERT(walker.qualifying.isSimple());
			enclosingType = &getObjectType(walker.qualifying.value);
		}
	}
	void visit(cpp::abstract_declarator* symbol)
	{
		DeclaratorWalker walker(getState());
		if(isParameter)
		{
			walker.declareEts(type, forward);
		}
		TREEWALKER_WALK(walker, symbol);
		enclosed = walker.paramScope;
		type.typeSequence = walker.typeSequence;
		addDependent(typeDependent, walker.dependent);
	}
	void visit(cpp::member_declarator_bitfield* symbol)
	{
		MemberDeclaratorBitfieldWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(walker.id != 0)
		{
			DeclarationPtr tmpDependent = type.dependent;
			setDependent(type.dependent, typeDependent);
			makeUniqueTypeSafe(type, getLocation());

			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *walker.id, type, 0, specifiers); // 3.3.1.1
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
	void visit(cpp::terminal<boost::wave::T_ASSIGN> symbol) // begins initializer_default
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_LEFTPAREN> symbol) // begins initializer_parenthesis
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_TRY> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_LEFTBRACE> symbol) // begins function_body
	{
		commit();
		SEMANTIC_ASSERT(declaration != 0);
		declaration->isFunctionDefinition = true;

		// symbols may be deferred during attempt to parse shared-prefix declarator: f(int i = j)
		// first parsed as member_declaration_named, backtracks on reaching '{'
		if(WalkerState::deferred != 0
			&& !deferred.empty())
		{
			WalkerState::deferred->splice(deferred);
		}
	}
	void visit(cpp::terminal<boost::wave::T_COMMA> symbol)
	{
		commit();
	}
	void visit(cpp::terminal<boost::wave::T_SEMICOLON> symbol)
	{
		commit();

		// symbols may be deferred during attempt to parse shared-prefix declarator: f(int i = j)
		// first parsed as member_declaration_named, backtracks on reaching '{'
		if(WalkerState::deferred != 0
			&& !deferred.empty())
		{
			WalkerState::deferred->splice(deferred);
		}
	}
	void visit(cpp::terminal<boost::wave::T_COLON> symbol) // in member_declarator_bitfield, or ctor_initializer
	{
		commit();
	}

	void visit(cpp::default_argument* symbol)
	{
		// We cannot correctly skip a template-id in a default-argument if it refers to a template declared later in the class.
		// This is considered to be correct: http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#325
		if(WalkerState::deferred != 0
			&& templateParameter == INDEX_INVALID) // don't defer parse of default-argument for non-type template-parameter
		{
			result = defer(WalkerState::deferred->first, *this, makeSkipDefaultArgument(IsTemplateName(*this)), symbol);
		}
		else
		{
			TREEWALKER_LEAF(symbol);
		}
	}
	// handle assignment-expression(s) in initializer
	void visit(cpp::assignment_expression* symbol) // condition_init
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		if(!isParameter // parameters cannot be constants
			&& declaration != 0) // declaration is 0 during deferred parse of default-argument
		{
			declaration->initializer = walker.expression;
			addDependent(declaration->valueDependent, walker.valueDependent);
		}
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	void visit(cpp::initializer_clause* symbol) // initializer_default
	{
		InitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
	// handle initializer in separate context to avoid ',' confusing recognition of declaration
	void visit(cpp::expression_list* symbol) // initializer_parenthesis
	{
		InitializerWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}
	void visit(cpp::constant_expression* symbol) // member_declarator_bitfield
	{
		ExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(declaration != 0);
		SEMANTIC_ASSERT(isDependent(walker.valueDependent) || walker.expression.isConstant); // TODO: non-fatal error: expected constant expression
		declaration->initializer = walker.expression;
		addDependent(declaration->valueDependent, walker.valueDependent);
	}

	void visit(cpp::statement_seq_wrapper* symbol)
	{
		// NOTE: we must ensure that symbol-table modifications within the scope of this function are undone on parse fail
		pushScope(newScope(enclosing->getUniqueName(), SCOPETYPE_LOCAL));
		if(WalkerState::deferred != 0)
		{
			result = defer(WalkerState::deferred->second, *this, skipBraced, symbol);
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
			result = defer(WalkerState::deferred->second, *this, skipMemInitializerClause, symbol);
		}
		else // in case of an out-of-line constructor-definition
		{
			TREEWALKER_LEAF(symbol);
		}
	}

	struct DeclareEtsGuard
	{
		Type* p;
		TypeSequence* typeSequence;
		DeclareEtsGuard(SimpleDeclarationWalker& walker)
		{
			p = walker.declareEts(walker.type, walker.forward) ? &walker.type : 0;
			typeSequence = &walker.type.typeSequence;
		}
		~DeclareEtsGuard()
		{
			if(p != 0)
			{
				*p = &gClass;
			}
			if(typeSequence != 0)
			{
				typeSequence->clear(); // if declaration parse failed, don't keep a reference to the (deleted) type-sequence
			}
		}
		void hit()
		{
			p = 0;
			typeSequence = 0;
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
	}
	void visit(cpp::function_definition* symbol)
	{
		DeclareEtsGuard guard(*this);
		TREEWALKER_LEAF(symbol);
		guard.hit();
	}
	void visit(cpp::type_declaration_suffix* symbol)
	{
		TREEWALKER_LEAF(symbol);
		if(forward != 0) // declare the name found in elaborated-type-specifier parse
		{
			bool isSpecialization = !isClassKey(*type.declaration);
			if(isSpecialization
				&& (specifiers.isFriend
					|| isExplicitInstantiation))
			{
				// friend class C<int>; // friend
				// template class C<int>; // explicit instantiation
			}
			else if(specifiers.isFriend)
			{
				// friend class C;
			}
			else
			{
				if(isSpecialization)
				{
					SEMANTIC_ASSERT(enclosing == templateEnclosing);
				}
				// class C;
				// template<class T> class C;
				// template<> class C<int>;
				// template<class T> class C<T*>;
				DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *forward, TYPE_CLASS, 0, DeclSpecifiers(), isSpecialization || enclosing == templateEnclosing, getTemplateParams(enclosing), isSpecialization, type.templateArguments);
#ifdef ALLOCATOR_DEBUG
				trackDeclaration(instance);
#endif
				setDecoration(forward, instance);
				declaration = instance;
				if(declaration->templateParamScope == 0)
				{
					declaration->templateParamScope = templateParamScope; // required by findEnclosingType
				}
			}
			type = TypeId(declaration, context); // TODO: is this necessary?
		}
		else if(declaration != 0
			&& declaration->isTemplate
			&& templateParams != 0) // if not an explicit-specialization
		{
			// template<class T> class C;
			SEMANTIC_ASSERT(!declaration->isSpecialization);
			mergeTemplateParamDefaults(*declaration, *templateParams);
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

struct ParameterDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	size_t templateParameter;
	cpp::default_argument* defaultArgument;
	bool isParameter;

	ParameterDeclarationWalker(const WalkerState& state, bool isParameter = false, size_t templateParameter = INDEX_INVALID)
		: WalkerBase(state), templateParameter(templateParameter), defaultArgument(0), isParameter(isParameter)
	{
	}
	void visit(cpp::parameter_declaration_default* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		declaration = walker.declaration;
		defaultArgument = symbol->init;
	}
	void visit(cpp::parameter_declaration_abstract* symbol)
	{
		SimpleDeclarationWalker walker(getState(), isParameter, templateParameter);
		TREEWALKER_WALK(walker, symbol);
		walker.parent = enclosing;
		walker.id = &gAnonymousId;
		walker.commit();
		declaration = walker.declaration;
		defaultArgument = symbol->init;
	}
};

struct TypeParameterWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	IdentifierPtr id;
	DeclarationPtr declaration;
	TemplateArgument argument; // the default argument for this param
	TemplateParameters params; // the template parameters for this param (if template-template-param)
	size_t templateParameter;
	TypeParameterWalker(const WalkerState& state, size_t templateParameter)
		: WalkerBase(state), id(&gAnonymousId), declaration(0), argument(context), params(context), templateParameter(templateParameter)
	{
	}
	void commit()
	{
		SEMANTIC_ASSERT(declaration == 0); // may only be called once, after parse of type-parameter succeeds
		DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_PARAM, 0, DECLSPEC_TYPEDEF, !params.empty(), params, false, TEMPLATEARGUMENTS_NULL, templateParameter);
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(instance);
#endif
		if(id != &gAnonymousId)
		{
			setDecoration(id, instance);
		}
		declaration = instance;
	}
	void visit(cpp::identifier* symbol)
	{
		TREEWALKER_LEAF(symbol);
		id = &symbol->value;
	}
	void visit(cpp::type_id* symbol)
	{
		SEMANTIC_ASSERT(params.empty());
		TypeIdWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		argument.type.swap(walker.type);
		makeUniqueTypeSafe(argument.type, getLocation());
	}
	void visit(cpp::template_parameter_clause* symbol)
	{
		TemplateParameterClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
	}
	void visit(cpp::id_expression* symbol)
	{
		IdExpressionWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		LookupResultRef declaration = walker.declaration;
		if(declaration != 0)
		{
			if(declaration == &gUndeclared
				|| !isTemplateName(*declaration))
			{
				return reportIdentifierMismatch(symbol, *walker.id, declaration, "template-name");
			}
			setDecoration(walker.id, declaration);
		}
	}
};

struct TemplateParameterListWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateParameter param;
	TemplateParameters params;
	size_t count;
	TemplateParameterListWalker(const WalkerState& state, size_t count)
		: WalkerBase(state), param(context), params(context), count(count)
	{
	}
	void visit(cpp::type_parameter_default* symbol)
	{
		TypeParameterWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		param = walker.declaration;
		setDependent(param);
		makeUniqueTypeSafe(param, getLocation());
		param.argument.swap(walker.argument);
		++count;
	}
	void visit(cpp::type_parameter_template* symbol)
	{
		TypeParameterWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		walker.commit();
		SEMANTIC_ASSERT( walker.declaration != 0);
		param = walker.declaration;
		setDependent(param);
		makeUniqueTypeSafe(param, getLocation());
		param.argument.swap(walker.argument);
		++count;
	}
	void visit(cpp::parameter_declaration* symbol)
	{
		ParameterDeclarationWalker walker(getState(), false, count);
		TREEWALKER_WALK(walker, symbol);
		SEMANTIC_ASSERT(walker.declaration != 0);
		param = walker.declaration;
		if(walker.defaultArgument != 0)
		{
			addDependent(param.argument.valueDependent, walker.declaration->valueDependent);
			param.argument.type = &gNonType;
			param.argument.expression = walker.declaration->initializer;
		}
		++count;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState(), count);
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
		params.push_front(walker.param);
	}
};

struct TemplateParameterClauseWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	TemplateParameters params;
	TemplateParameterClauseWalker(const WalkerState& state)
		: WalkerBase(state), params(context)
	{
		// collect template-params into a new scope
		if(templateParamScope != 0)
		{
			pushScope(templateParamScope); // the existing template parameter scope encloses the new scope
		}
		pushScope(newScope(makeIdentifier("$template"), SCOPETYPE_TEMPLATE));
		clearTemplateParams();
		enclosing->templateDepth = templateDepth;
	}
	void visit(cpp::template_parameter_list* symbol)
	{
		TemplateParameterListWalker walker(getState(), 0);
		TREEWALKER_WALK(walker, symbol);
		params.swap(walker.params);
		params.push_front(walker.param);
	}
};

struct TemplateDeclarationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	TemplateParameters params;
	TemplateDeclarationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0), params(context)
	{
		templateEnclosing = enclosing;
		++templateDepth;
	}
	void visit(cpp::template_parameter_clause* symbol)
	{
		TemplateParameterClauseWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		templateParamScope = walker.enclosing;
		enclosing = walker.enclosing->parent;
		params.swap(walker.params);
		templateParams = &params;
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		TREEWALKER_WALK_SRC(walker, symbol);
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
	void visit(cpp::member_declaration* symbol)
	{
		MemberDeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
		SEMANTIC_ASSERT(declaration != 0);
	}
};

struct ExplicitInstantiationWalker : public WalkerBase
{
	TREEWALKER_DEFAULT;

	DeclarationPtr declaration;
	ExplicitInstantiationWalker(const WalkerState& state)
		: WalkerBase(state), declaration(0)
	{
	}
	void visit(cpp::terminal<boost::wave::T_TEMPLATE> symbol)
	{
		isExplicitInstantiation = true;
	}
	void visit(cpp::declaration* symbol)
	{
		DeclarationWalker walker(getState());
		TREEWALKER_WALK(walker, symbol);
		declaration = walker.declaration;
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
	void visit(cpp::explicit_instantiation* symbol)
	{
		ExplicitInstantiationWalker walker(getState());
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
			DeclarationInstanceRef instance = pointOfDeclaration(context, enclosing, *id, TYPE_NAMESPACE, 0);
#ifdef ALLOCATOR_DEBUG
			trackDeclaration(instance);
#endif
			setDecoration(id, instance);
			declaration = instance;
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
		TREEWALKER_WALK_SRC(walker, symbol);
		symbol->events = events;
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
	gUniqueTypes.clear();
	gUniqueExpressions.clear();

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
	catch(TypeError& e)
	{
		e.report();
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
	gUniqueNames.clear();
	gUniqueTypes.clear();
	gUniqueExpressions.clear();

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
	catch(TypeError& e)
	{
		e.report();
	}
	if(!context.finished())
	{
		printError(parser);
	}
	return result;
}


