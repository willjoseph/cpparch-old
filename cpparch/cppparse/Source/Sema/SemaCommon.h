
#ifndef INCLUDED_CPPPARSE_SEMA_SEMACOMMON_H
#define INCLUDED_CPPPARSE_SEMA_SEMACOMMON_H

#include "Ast/Type.h"
#include "Core/NameLookup.h"
#include "Core/Special.h"
#include "Core/TypeUnique.h"

struct SemanticError
{
	SemanticError()
	{
#ifdef ALLOCATOR_DEBUG
		DEBUG_BREAK();
#endif
	}
};

#define SEMANTIC_ASSERT(condition) if(!(condition)) { throw SemanticError(); }

inline void semanticBreak()
{
}

inline void printDeclarations(const Scope::Declarations& declarations)
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

inline void printBases(const Types& bases)
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

inline void printScope(const Scope& scope)
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

inline void printName(const Scope& scope)
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

extern IdentifierMismatch gIdentifierMismatch;

inline void printIdentifierMismatch(const IdentifierMismatch& e)
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
	SEMANTIC_ASSERT(id != &gAnonymousId);
	id->dec.p = &declaration;
}




inline bool isEqual(const TypeId& l, const TypeId& r)
{
	SYMBOLS_ASSERT(l.unique != 0);
	SYMBOLS_ASSERT(r.unique != 0);
	return l.unique == r.unique;
}

inline bool isEqual(const Type& left, const Type& right)
{
	SYMBOLS_ASSERT(left.unique != 0);
	SYMBOLS_ASSERT(right.unique != 0);
	return left.unique == right.unique;
}

inline bool isEqual(const TemplateArgument& l, const TemplateArgument& r)
{
	if((l.type.declaration == &gNonType)
		!= (r.type.declaration == &gNonType))
	{
		return false;
	}
	return l.type.declaration == &gNonType
		? l.expression.p == r.expression.p
		: isEqual(l.type, r.type);
}

inline bool matchTemplateSpecialization(const Declaration& declaration, const TemplateArguments& arguments)
{
	// TODO: check that all arguments are specified!
	TemplateArguments::const_iterator a = arguments.begin();
	for(TemplateArguments::const_iterator i = declaration.templateArguments.begin(); i != declaration.templateArguments.end(); ++i)
	{
		SYMBOLS_ASSERT(a != arguments.end()); // a template-specialization must have no more arguments than the template parameters

		if(!isEqual(*i, *a))
		{
			return false;
		}
		++a;
	}
	SYMBOLS_ASSERT(a == arguments.end());
	return true;
}

inline Declaration* findTemplateSpecialization(Declaration* declaration, const TemplateArguments& arguments)
{
	for(; declaration != 0; declaration = declaration->overloaded)
	{
		if(!isSpecialization(*declaration))
		{
			continue;
		}

		if(matchTemplateSpecialization(*declaration, arguments))
		{
			return declaration;
		}
	}
	return 0;
}


inline bool isAnonymous(const Declaration& declaration)
{
	return *declaration.getName().value.c_str() == '$';
}

struct DeclarationError
{
	const char* description;
	DeclarationError(const char* description) : description(description)
	{
	}
};



// int i; // type -> int
// typedef int I; // type -> int
// I i; // type -> I -> int
// typedef I J; // type -> I -> int
// J j; // type -> J -> I -> int
// struct S; // type -> struct
// typedef struct S S; // type -> S -> struct
// typedef struct S {} S; // type -> S -> struct
// typedef enum E {} E; // type -> E -> enum

// returns the type of a declaration
// int i; -> built-in
// class A a; -> A
// enum E e; -> E
// typedef int T; T t; -> built-in
inline const Declaration* getType(const Declaration& declaration)
{
	if(declaration.specifiers.isTypedef)
	{
		return getType(*declaration.type.declaration);
	}
	return declaration.type.declaration;
}

inline const Declaration& getPrimaryDeclaration(const Declaration& first, const Declaration& second)
{
	if(isNamespace(first))
	{
		if(!isNamespace(second))
		{
			throw DeclarationError("non-namespace already declared as namespace");
		}
		return first; // namespace continuation
	}
	if(isType(first))
	{
		if(!isType(second))
		{
			throw DeclarationError("non-type already declared as type");
		}
		if(getType(first) != getType(second))
		{
			throw DeclarationError("type already declared as different type");
		}
		if(isTypedef(first))
		{
			return second; // redeclaration of typedef, or definition of type previously used in typedef
		}
		if(isTypedef(second))
		{
			return second; // typedef of type previously declared: typedef struct S {} S;
		}
		if(isClass(first))
		{
			if(isSpecialization(second))
			{
				return second; // TODO: class template partial/explicit-specialization
			}
			if(isSpecialization(first))
			{
				return second; // TODO: class template partial/explicit-specialization
			}
			if(isIncomplete(second))
			{
				return second; // redeclaration of previously-declared class
			}
			if(isIncomplete(first))
			{
				return second; // definition of forward-declared class
			}
			throw DeclarationError("class-definition already defined");
		}
		if(isEnum(first))
		{
			throw DeclarationError("enum-definition already defined");
		}
		throw SymbolsError(); // should not be reachable
	}
	if(isType(second))
	{
		throw DeclarationError("type already declared as non-type");
	}
	if(isFunction(first)
		|| isFunction(second))// TODO: function overloading
	{
		return second; // multiple declarations allowed
	}
	if(isStaticMember(first))
	{
		// TODO: disallow inline definition of static member: class C { static int i; int i; };
		if(!isMemberObject(second))
		{
			throw DeclarationError("non-member-object already declared as static member-object");
		}
		return second; // multiple declarations allowed
	}
	if(isExtern(first)
		|| isExtern(second))
	{
		return second; // multiple declarations allowed
	}
	// HACK: ignore multiple declarations for members of template - e.g. const char Tmpl<char>::VALUE; const int Tmpl<int>::VALUE;
	if(!first.templateParams.defaults.empty())
	{
		// if enclosing is a template
		return first;
	}
	throw DeclarationError("symbol already defined");
}

inline bool hasTemplateParamDefaults(const TemplateParameters& params)
{
	for(TemplateArguments::const_iterator i = params.defaults.begin(); i != params.defaults.end(); ++i)
	{
		if((*i).type.declaration != 0)
		{
			return true;
		}
	}
	return false;
}

// substitute references to template-parameters of 'otherParams' for template-parameters of 'params'
inline void fixTemplateParamDefault(TemplateArgument& argument, const TemplateParameters& params, const TemplateParameters& otherParams)
{
	if(argument.type.declaration == 0)
	{
		return;
	}
	std::size_t index = argument.type.declaration->templateParameter;
	if(index != INDEX_INVALID)
	{
		Types::const_iterator i = params.begin();
		std::advance(i, index);
		Types::const_iterator j = otherParams.begin();
		std::advance(j, index);
		if(argument.type.declaration->scope == (*j).declaration->scope)
		{
			argument.type.declaration = (*i).declaration;
		}
	}
	for(TemplateArguments::iterator i = argument.type.templateArguments.begin(); i != argument.type.templateArguments.end(); ++i)
	{
		fixTemplateParamDefault(*i, params, otherParams);
	}
}

inline void copyTemplateParamDefault(TemplateArgument& argument, const TemplateArgument& otherArgument, const TemplateParameters& params, const TemplateParameters& otherParams)
{
	argument = otherArgument;
	fixTemplateParamDefault(argument, params, otherParams);
}

inline void copyTemplateParamDefaults(TemplateParameters& params, const TemplateParameters& otherParams)
{
	SYMBOLS_ASSERT(params.defaults.empty());
	for(TemplateArguments::const_iterator i = otherParams.defaults.begin(); i != otherParams.defaults.end(); ++i)
	{
		params.defaults.push_back(TEMPLATEARGUMENT_NULL);
		copyTemplateParamDefault(params.defaults.back(), *i, params, otherParams);
	}
}

/// 14.1-10: the set of template param defaults is obtained by merging those from the definition and all declarations currently in scope (excluding explicit-specializations)
inline void mergeTemplateParamDefaults(TemplateParameters& params, const TemplateParameters& otherParams)
{
	if(params.defaults.empty())
	{
		copyTemplateParamDefaults(params, otherParams);
		return;
	}
	if(!hasTemplateParamDefaults(otherParams)) // ignore declarations with no default-arguments, e.g. explicit/partial-specializations
	{
		return;
	}
	SYMBOLS_ASSERT(!otherParams.defaults.empty());
	TemplateArguments::iterator d = params.defaults.begin();
	for(TemplateArguments::const_iterator i = otherParams.defaults.begin(); i != otherParams.defaults.end(); ++i)
	{
		SYMBOLS_ASSERT(d != params.defaults.end());
		SYMBOLS_ASSERT((*d).type.declaration == 0 || (*i).type.declaration == 0); // TODO: non-fatal error: default param defined more than once
		if((*d).type.declaration == 0)
		{
			copyTemplateParamDefault(*d, *i, params, otherParams);
		}
		++d;
	}
	SYMBOLS_ASSERT(d == params.defaults.end());
}

inline void mergeTemplateParamDefaults(Declaration& declaration, const TemplateParameters& templateParams)
{
	SYMBOLS_ASSERT(declaration.isTemplate);
	SYMBOLS_ASSERT(isClass(declaration));
	SYMBOLS_ASSERT(!isSpecialization(declaration)); // explicit/partial-specializations cannot have default-arguments
	mergeTemplateParamDefaults(declaration.templateParams, templateParams);
	SYMBOLS_ASSERT(!declaration.templateParams.defaults.empty());
}

//-----------------------------------------------------------------------------

inline bool isFunctionParameterEquivalent(UniqueTypeWrapper left, UniqueTypeWrapper right)
{
	return adjustFunctionParameter(left) == adjustFunctionParameter(right);
}

inline bool isEquivalent(const ParameterTypes& left, const ParameterTypes& right)
{
	ParameterTypes::const_iterator l = left.begin();
	ParameterTypes::const_iterator r = right.begin();
	for(;; ++l, ++r)
	{
		if(l == left.end())
		{
			return r == right.end();
		}
		if(r == right.end())
		{
			return false;
		}
		if(!isFunctionParameterEquivalent(*l, *r))
		{
			return false;
		}
	}
	return true;
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

inline bool isEquivalentTemplateParameter(const Type& left, const Type& right)
{
	extern Declaration gParam;
	if((left.declaration->type.declaration == &gParam)
		!= (right.declaration->type.declaration == &gParam))
	{
		return false;
	}
	return left.declaration->type.declaration == &gParam
		? isEqual(left, right)
		: isEqual(left.declaration->type, right.declaration->type);
}

inline bool isEquivalentTemplateParameters(const TemplateParameters& left, const TemplateParameters& right)
{
	if(std::distance(left.begin(), left.end()) != std::distance(right.begin(), right.end()))
	{
		return false;
	}
	TemplateParameters::const_iterator l = left.begin();
	for(TemplateParameters::const_iterator r = right.begin(); r != right.end(); ++l, ++r)
	{
		SYMBOLS_ASSERT(l != left.end());

		if(!isEquivalentTemplateParameter(*l, *r))
		{
			return false;
		}
	}
	SYMBOLS_ASSERT(l == left.end());
	return true;
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
				&& isEquivalentTemplateParameters(declaration.templateParams, other.templateParams)
				// [over.load] Function declarations that differ only in the return type cannot be overloaded.
				&& (declaration.getName().value == gConversionFunctionId
				? isReturnTypeEqual(l, r) // return-types match
				// (only template overloads may differ in return type, return-type is not used to distinguish overloads, except for conversion-function)
				: isEquivalent(getParameterTypes(l.value), getParameterTypes(r.value))); // and parameter-types match
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





inline bool enclosesEts(ScopeType type)
{
	return type == SCOPETYPE_NAMESPACE
		|| type == SCOPETYPE_LOCAL;
}

inline const SimpleType* getEnclosingType(const SimpleType* enclosing)
{
	for(const SimpleType* i = enclosing; i != 0; i = (*i).enclosing)
	{
		if((*i).declaration->getName().value.c_str()[0] != '$') // ignore anonymous union
		{
			return i;
		}
	}
	return 0;
}

inline bool findScope(Scope* scope, Scope* other)
{
	if(scope == 0)
	{
		return false;
	}
	if(scope == other)
	{
		return true;
	}
	return findScope(scope->parent, other);
}

inline Declaration* getClassDeclaration(Scope* scope)
{
	SYMBOLS_ASSERT(scope);
	SYMBOLS_ASSERT(scope->type == SCOPETYPE_CLASS);
	return getDeclaration(scope->name);
}

inline Declaration* findEnclosingClassTemplate(Declaration* dependent)
{
	if(dependent != 0
		&& (isClass(*dependent)
		|| isEnum(*dependent)) // type of enum within class template is dependent
		&& isMember(*dependent))
	{
		Scope* scope = getEnclosingClass(dependent->scope);
		if(scope == 0)
		{
			// enclosing class was anonymous and at namespace scope.
			return 0;
		}
		Declaration* declaration = getClassDeclaration(scope);
		return declaration->isTemplate
			? declaration
			: findEnclosingClassTemplate(declaration);
	}
	return 0;
}

inline bool isDependentImpl(Declaration* dependent, Scope* enclosing, Scope* templateParamScope)
{
	return dependent != 0
		&& (findScope(enclosing, dependent->scope) != 0
		|| findScope(templateParamScope, dependent->scope) != 0); // if we are within the candidate template-parameter's template-definition
}


struct SemaContext : public AstAllocator<int>
{
	ParserContext& parserContext;
	Scope global;
	Declaration globalDecl;
	TypeRef globalType;
	std::size_t declarationCount;
	UniqueTypeWrapper typeInfoType;

	SemaContext(ParserContext& parserContext, const AstAllocator<int>& allocator) :
		AstAllocator<int>(allocator),
		parserContext(parserContext),
		global(allocator, gGlobalId, SCOPETYPE_NAMESPACE),
		globalDecl(allocator, 0, gGlobalId, TYPE_NULL, &global),
		globalType(Type(&globalDecl, allocator), allocator),
		declarationCount(0)
	{
	}
};

typedef std::list< DeferredParse<struct SemaState> > DeferredSymbolsList;

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


struct SemaState
	: public ContextBase
{
	typedef SemaState State;

	SemaContext& context;
	ScopePtr enclosing;
	const SimpleType* enclosingType;
	const SimpleType* enclosingFunction;
	Dependent enclosingDependent;
	TypePtr qualifying_p;
	DeclarationPtr qualifyingScope;
	const SimpleType* qualifyingClass;
	const SimpleType* memberClass;
	UniqueTypeWrapper memberType;
	ExpressionWrapper objectExpression; // the lefthand side of a class member access expression
	SafePtr<const TemplateParameters> templateParams;
	ScopePtr templateParamScope;
	DeferredSymbols* enclosingDeferred;
	std::size_t templateDepth;
	bool isExplicitInstantiation;

	SemaState(SemaContext& context)
		: context(context)
		, enclosing(0)
		, enclosingType(0)
		, enclosingFunction(0)
		, qualifying_p(0)
		, qualifyingScope(0)
		, qualifyingClass(0)
		, memberClass(0)
		, templateParams(0)
		, templateParamScope(0)
		, enclosingDeferred(0)
		, templateDepth(0)
		, isExplicitInstantiation(false)
	{
	}
	const SemaState& getState() const
	{ 
		return *this;
	}
	Location getLocation() const
	{
		return Location(context.parserContext.get_source(), context.declarationCount);
	}
	InstantiationContext getInstantiationContext() const
	{
		return InstantiationContext(getLocation(), enclosingType, enclosingFunction, enclosing);
	}

	UniqueTypeWrapper getTypeInfoType()
	{
		if(context.typeInfoType == gUniqueTypeNull)
		{
			Identifier stdId = makeIdentifier(context.parserContext.makeIdentifier("std"));
			LookupResultRef declaration = ::findDeclaration(context.global, stdId, IsNestedName());
			SEMANTIC_ASSERT(declaration != 0);
			SEMANTIC_ASSERT(declaration->enclosed != 0);
			SEMANTIC_ASSERT(declaration->enclosed->type == SCOPETYPE_NAMESPACE);
			Identifier typeInfoId = makeIdentifier(context.parserContext.makeIdentifier("type_info"));
			declaration = ::findDeclaration(*declaration->enclosed, typeInfoId);
			SEMANTIC_ASSERT(declaration != 0);
			SEMANTIC_ASSERT(isClass(*declaration));
			Type type(declaration, context);
			context.typeInfoType = makeUniqueType(type, InstantiationContext(Location(), 0, 0, 0), false);
			context.typeInfoType.value.setQualifiers(CvQualifiers(true, false));
		}
		return context.typeInfoType;
	}

	bool objectExpressionIsDependent() const 
	{
		return objectExpression.p != 0
			&& objectExpression.isTypeDependent
			&& memberClass != 0;
	}
	bool allowNameLookup() const
	{
		if(isDependent(qualifying_p))
		{
			return false;
		}
		if(objectExpressionIsDependent())
		{
			return false;
		}
		return true;
	}
	LookupResult lookupQualified(const Identifier& id, bool isDeclarator, LookupFilter filter = IsAny())
	{
		return isDeclarator
			? findDeclaratorDeclaration(id, filter)
			: lookupQualified(id, filter);
	}
	LookupResult findDeclaratorDeclaration(const Identifier& id, LookupFilter filter = IsAny())
	{
		SEMANTIC_ASSERT(getQualifyingScope() != 0);
		LookupResult result;
		if(result.append(::findDeclaration(*getQualifyingScope(), id, filter)))
		{
			return result;
		}
		result.filtered = &gUndeclaredInstance;
		return result;
	}
	LookupResult lookupQualified(const Identifier& id, LookupFilter filter = IsAny())
	{
		SEMANTIC_ASSERT(getQualifyingScope() != 0);
		LookupResult result;
		// [basic.lookup.qual]
		if(qualifyingClass != 0)
		{
			instantiateClass(*qualifyingClass, getInstantiationContext());
			if(result.append(::findDeclaration(*qualifyingClass, id, filter)))
			{
				return result;
			}
		}
		else if(result.append(::findNamespaceDeclaration(*getQualifyingScope(), id, filter)))
		{
#ifdef LOOKUP_DEBUG
			std::cout << "HIT: qualified" << std::endl;
#endif
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
		if(getQualifyingScope() != 0)
		{
			return lookupQualified(id, filter);
		}
		else
		{
			bool isQualified = objectExpression.p != 0
				&& memberClass != 0;
			SYMBOLS_ASSERT(!(isUnqualifiedId && objectExpression.isTypeDependent)); // in case of unqualified-id, should check allowNameLookup before calling
			if(isQualified
				&& !objectExpression.isTypeDependent)
			{
				// [basic.lookup.classref]
				SYMBOLS_ASSERT(memberClass != &gDependentSimpleType);
				if(result.append(::findDeclaration(*memberClass, id, filter)))
				{
#ifdef LOOKUP_DEBUG
					std::cout << "HIT: member" << std::endl;
#endif
					return result;
				}
				// else if we're parsing a nested-name-specifier prefix, drop through, look up in the current context
			}

			if(!isQualified || !isUnqualifiedId)
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
		const AstAllocator<int>& allocator,
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
		SEMANTIC_ASSERT(!isSpecialization || isTemplate); // only a template can be a specialization
		SEMANTIC_ASSERT(!isTemplate || isSpecialization || !params.empty()); // only a specialization may have an empty template parameter clause <>

		context.parserContext.allocator.deferredBacktrack(); // flush cached parse-tree

		static size_t uniqueId = 0;

		SEMANTIC_ASSERT(!name.value.empty());
		Declaration declaration(allocator, parent, name, type, enclosed, specifiers, isTemplate, params, isSpecialization, arguments, templateParameter, valueDependent);
		SEMANTIC_ASSERT(!isTemplate || (isClass(declaration) || isFunction(declaration) || declaration.templateParameter != INDEX_INVALID)); // only a class, function or template-parameter can be a template
		declaration.uniqueId = ++uniqueId;
		DeclarationInstance instance;
		const DeclarationInstance* existing = 0;
		if(!isAnonymous(declaration)) // unnamed class/struct/union/enum
		{
			LookupFilter filter = IsAny();
			if(type.declaration == &gCtor)
			{
				filter = IsConstructor(); // find existing constructor declaration
			}
			existing = ::findDeclaration(parent->declarations, name, filter);
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

			if(isFunction(declaration)
				&& instance->isTemplateName)
			{
				// quick hack - if any template overload of a function has been declared, all subsequent declarations are template names
				declaration.isTemplateName = true;
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
		instance.visibility = context.declarationCount++;
		const DeclarationInstance& result = parent->declarations.insert(instance);
		parent->declarationList.push_back(instance);
		return result;
	}

	AstAllocator<int> getAllocator()
	{
#ifdef AST_ALLOCATOR_LINEAR
		return context.parserContext.allocator;
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
		declaration->enclosed->bases.push_front(base);
	}

	Declaration* getDeclaratorQualifying() const
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
		qualifyingClass = 0;
		memberType = gUniqueTypeNull;
		memberClass = 0;
		objectExpression = ExpressionWrapper();
	}

	const TemplateParameters& getTemplateParams() const
	{
		if(templateParams == 0)
		{
			return TEMPLATEPARAMETERS_NULL;
		}
		return *templateParams;
	}

	void clearTemplateParams()
	{
		templateParamScope = 0;
		templateParams = 0;
	}

	template<typename T>
	bool reportIdentifierMismatch(T* symbol, const Identifier& id, Declaration* declaration, const char* expected)
	{
#if 0
		gIdentifierMismatch = IdentifierMismatch(id, declaration, expected);
#endif
		return false;
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
		return ::isDependentImpl(dependent, enclosing, templateParamScope);
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
	void setDependentEnclosingTemplate(DeclarationPtr& dependent, Declaration* enclosingTemplate) const
	{
		if(enclosingTemplate != 0)
		{
			SEMANTIC_ASSERT(enclosingTemplate->isTemplate);
			// 'declaration' is a class that is dependent because it is a (possibly specialized) member of an enclosing template class
			SEMANTIC_ASSERT(enclosingTemplate->isSpecialization || !enclosingTemplate->templateParams.empty());
			if(!enclosingTemplate->templateParams.empty()) // if the enclosing template class is not an explicit specialization
			{
				// depend on the template parameter(s) of the enclosing template class
				setDependent(dependent, enclosingTemplate->templateParams.back().declaration);
			}
		}
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

		setDependentEnclosingTemplate(dependent, findEnclosingClassTemplate(&declaration));

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
		Declaration* old = dependent.p;
		setDependent(dependent, *declaration);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
	void addDependentType(Dependent& dependent, Declaration* declaration)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, declaration->type.dependent);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
	void addDependent(Dependent& dependent, const Type& type)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, type.dependent);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
	void addDependent(Dependent& dependent, Scope* scope)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, scope->bases);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
	}
	void addDependent(Dependent& dependent, const Dependent& other)
	{
		Declaration* old = dependent.p;
		setDependent(dependent, other);
		SEMANTIC_ASSERT(old == 0 || dependent.p != 0);
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


struct SemaBase : public SemaState
{
	typedef SemaBase Base;

	SemaBase(SemaContext& context)
		: SemaState(context)
	{
	}
	SemaBase(const SemaState& state)
		: SemaState(state)
	{
	}
	Scope* newScope(const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
	{
		return allocatorNew(context, Scope(context, name, type));
	}

	template<typename T>
	ExpressionWrapper makeExpression(const T& value, bool isConstant = false, bool isTypeDependent = false, bool isValueDependent = false)
	{
		ExpressionNode* node = isConstant ? makeUniqueExpression(value) : allocatorNew(context, ExpressionNodeGeneric<T>(value));
		ExpressionWrapper result(node, isConstant, isTypeDependent, isValueDependent);
		if(!isTypeDependent)
		{
#if 1 // TODO:
			result.type = typeOfExpressionSafe(node, getInstantiationContext());
#endif
		}
		return result;
	}

	void addBacktrackCallback(const BacktrackCallback& callback)
	{
		context.parserContext.allocator.addBacktrackCallback(context.parserContext.allocator.position, callback);
	}

	void disableBacktrack()
	{
		addBacktrackCallback(makeBacktrackErrorCallback());
	}

	// Causes /p declaration to be undeclared when backtracking.
	// In practice this only happens for the declaration in an elaborated-type-specifier.
	void trackDeclaration(const DeclarationInstance& declaration)
	{
		addBacktrackCallback(makeUndeclareCallback(&declaration));
	}

	Declaration* declareClass(Scope* parent, Identifier* id, bool isSpecialization, TemplateArguments& arguments)
	{
		Scope* enclosed = newScope(makeIdentifier("$class"), SCOPETYPE_CLASS);
		DeclarationInstanceRef declaration = pointOfDeclaration(context, parent, id == 0 ? parent->getUniqueName() : *id, TYPE_CLASS, enclosed, DeclSpecifiers(), templateParams != 0, getTemplateParams(), isSpecialization, arguments);
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

		bool isTemplate = templateParams != 0;
		bool isExplicitSpecialization = isTemplate && templateParams->empty();
		DeclarationInstanceRef declaration = pointOfDeclaration(context, parent, *id, type, enclosed, specifiers, isTemplate, getTemplateParams(), isExplicitSpecialization, TEMPLATEARGUMENTS_NULL, templateParameter, valueDependent); // 3.3.1.1
#ifdef ALLOCATOR_DEBUG
		trackDeclaration(declaration);
#endif
		if(id != &gAnonymousId)
		{
			setDecoration(id, declaration);
		}

		if(declaration->templateParamScope == 0)
		{
			declaration->templateParamScope = templateParamScope; // required by findEnclosingType
		}

		// the type of an object is required to be complete
		// a member's type must be instantiated before the point of declaration of the member, to prevent the member being found by name lookup during the instantiation
		SEMANTIC_ASSERT(type.unique != 0);
		UniqueTypeWrapper uniqueType = UniqueTypeWrapper(type.unique);
		// NOTE: these checks must occur after the declaration because an out-of-line definition of a static member is otherwise not known to be static
		if(parent->type == SCOPETYPE_CLASS // just members, for now
			&& !uniqueType.isFunction() // member functions are not instantiated when class is implicitly instantiated
			&& !isStatic(*declaration) // static members are not instantiated when class is implicitly instantiated
			&& type.declaration != &gCtor) // ignore constructor
		{
			SimpleType* enclosingClass = const_cast<SimpleType*>(getEnclosingType(enclosingType));
			std::size_t size = 0;
			if(!type.isDependent)
			{
				if(!(parent->type == SCOPETYPE_CLASS && isStatic(*declaration)) // ignore static member
					&& !specifiers.isTypedef // ignore typedef
					&& (uniqueType.isSimple() || uniqueType.isArray()))
				{
					// TODO: accurate sizeof
					size = requireCompleteObjectType(uniqueType, getInstantiationContext());
				}
			}
			else if(enclosingClass != 0
				&& templateParams == 0) // ignore template member functions, for now
			{
				enclosingClass->children.push_back(uniqueType);
				// TODO: check compliance: the point of instantiation of a type used in a member declaration is the point of declaration of the member
				// .. along with the point of instantiation of types required when naming the member type. e.g. A<T>::B m; B<A<T>::value> m;
				enclosingClass->childLocations.push_back(getLocation());
			}
			if(enclosingClass != 0)
			{
				enclosingClass->size += size;
			}
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
			DeclarationInstanceRef declaration = pointOfDeclaration(context, getEtsScope(), *forward, TYPE_CLASS, 0);
			
			trackDeclaration(declaration);
			setDecoration(forward, declaration);
			type = declaration;
			return true;
		}
		return false;
	}

	bool consumeTemplateParams(const Qualifying& qualifying)
	{
		if(qualifying.empty())
		{
			return false;
		}
		const Type& type = qualifying.back();
		if(!type.declaration->isTemplate) // if the qualifying type is not a template
		{
			return consumeTemplateParams(type.qualifying);
		}
		Declaration* primary = findPrimaryTemplate(type.declaration);
		SEMANTIC_ASSERT(primary->templateParamScope->templateDepth <= templateDepth); // TODO: non-fatal error: not enough template-parameter-clauses in class declaration
		return primary->templateParamScope->templateDepth == templateDepth;
	}

	LookupResultRef lookupTemplate(const Identifier& id, LookupFilter filter)
	{
		if(!isDependent(qualifying_p))
		{
			return LookupResultRef(findDeclaration(id, filter));
		}
		return gDependentTemplateInstance;
	}

	void addDependentOverloads(Dependent& dependent, Declaration* declaration)
	{
		for(Declaration* p = declaration; p != 0; p = p->overloaded)
		{
			setDependent(dependent, p->type.dependent);
		}
	}
	static UniqueTypeWrapper binaryOperatorIntegralType(UniqueTypeWrapper left, UniqueTypeWrapper right)
	{
		SEMANTIC_ASSERT(!isFloating(left));
		SEMANTIC_ASSERT(!isFloating(right));
		return usualArithmeticConversions(left, right);
	}

	template<typename T>
	void makeUniqueTypeImpl(T& type)
	{
		SYMBOLS_ASSERT(type.unique == 0); // type must not be uniqued twice
		type.isDependent = isDependent(type)
			|| objectExpressionIsDependent(); // this occurs when uniquing the dependent type name in a nested name-specifier in a class-member-access expression
		type.unique = makeUniqueType(type, getInstantiationContext(), type.isDependent).value;
	}
	void makeUniqueTypeSafe(Type& type)
	{
		makeUniqueTypeImpl(type);
	}
	void makeUniqueTypeSafe(TypeId& type)
	{
		makeUniqueTypeImpl(type);
	}
	UniqueTypeWrapper getUniqueTypeSafe(const TypeId& type)
	{
		SEMANTIC_ASSERT(type.unique != 0); // type must have previously been uniqued by makeUniqueTypeImpl
		return type.isDependent ? gUniqueTypeNull : UniqueTypeWrapper(type.unique);
	}
};




struct Args0
{
};

template<typename A1>
struct Args1
{
	A1 a1;
	Args1(A1 a1) : a1(a1)
	{
	}
};

template<typename A1, typename A2>
struct Args2
{
	A1 a1;
	A2 a2;
	Args2(A1 a1, A2 a2) : a1(a1), a2(a2)
	{
	}
};

struct InvokeNone
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		return true;
	}
};

struct InvokeChecked
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		return walker.action(symbol);
	}
};

struct InvokeUnchecked
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		walker.action(symbol);
		return true;
	}
};

struct InvokeCheckedResult
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		return walker.action(symbol, result);
	}
};

struct InvokeUncheckedResult
{
	template<typename SemaT, typename T, typename Result>
	static bool invokeAction(SemaT& walker, T* symbol, Result& result)
	{
		walker.action(symbol, result);
		return true;
	}
};


struct CommitNull
{
};

struct CommitEnable
{
};


template<typename SemaT, typename Inner>
struct SemaInner
{
};


template<typename Inner, typename Commit = CommitNull, typename Args = Args0>
struct SemaPush : Args
{
	typedef Args ArgsType;
	SemaPush(const Args& args)
		: Args(args)
	{
	}
};

template<typename Inner, typename Commit>
struct SemaPush<Inner, Commit, Args0>
{
	typedef SemaPush ArgsType;
};


template<typename SemaT, typename Inner, typename Commit>
struct SemaInner<SemaT, SemaPush<Inner, Commit, Args0> >
{
	typedef Inner Type;
	Inner sema;
	SemaInner(SemaT& walker, const SemaPush<Inner, Commit, Args0>& args)
		: sema(walker.getState())
	{
	}
	Inner& get()
	{
		return sema;
	}
};

template<typename SemaT, typename Inner, typename Commit, typename A1>
struct SemaInner<SemaT, SemaPush<Inner, Commit, Args1<A1> > >
{
	typedef Inner Type;
	Inner sema;
	SemaInner(SemaT& walker, const SemaPush<Inner, Commit, Args1<A1> >& args)
		: sema(walker.getState(), args.a1)
	{
	}
	Inner& get()
	{
		return sema;
	}
};

template<typename SemaT, typename Inner, typename Commit, typename A1, typename A2>
struct SemaInner<SemaT, SemaPush<Inner, Commit, Args2<A1, A2> > >
{
	typedef Inner Type;
	Inner sema;
	SemaInner(SemaT& walker, const SemaPush<Inner, Commit, Args2<A1, A2> >& args)
		: sema(walker.getState(), args.a1, args.a2)
	{
	}
	Inner& get()
	{
		return sema;
	}
};


template<typename SemaT, LexTokenId ID, typename U = void>
struct HasAction
{
	static const bool value = false;
};

template<typename SemaT, LexTokenId ID>
struct HasAction<SemaT, ID, typename SfinaeNonType<void(SemaT::*)(cpp::terminal<ID>), &SemaT::action>::Type>
{
	static const bool value = true;
};


template<typename SemaT, LexTokenId ID>
typename EnableIf<!HasAction<SemaT, ID>::value>::Type
	semaAction(SemaT& walker, cpp::terminal<ID>)
{
	// do nothing
}

template<typename SemaT, LexTokenId ID>
typename EnableIf<HasAction<SemaT, ID>::value>::Type
	semaAction(SemaT& walker, cpp::terminal<ID> symbol)
{
	walker.action(symbol);
}


template<typename SemaT, typename Inner, typename Args>
void semaCommit(SemaT& walker, const SemaPush<Inner, CommitNull, Args>& inner)
{
	// do nothing
}

template<typename SemaT, typename Inner, typename Args>
void semaCommit(SemaT& walker, const SemaPush<Inner, CommitEnable, Args>& inner)
{
	walker.commit();
}

struct Once
{
	bool done;
	Once()
		: done(false)
	{
	}
	void operator()()
	{
		SEMANTIC_ASSERT(!done);
		done = true;
	}
	void test() const
	{
		SEMANTIC_ASSERT(done);
	}
};

struct SemaIdentity
{
	typedef SemaIdentity ArgsType;
};

template<typename SemaT>
struct SemaInner<SemaT, SemaIdentity>
{
	typedef SemaT Type;
	SemaT& sema;
	SemaInner(SemaT& walker, const SemaIdentity&) : sema(walker)
	{
	}
	SemaT& get()
	{
		 return sema;
	}
};


template<typename SemaT>
void semaCommit(SemaT& walker, const SemaIdentity& inner)
{
	// do nothing
}

struct Nothing
{
};

struct AnnotateNull
{
	typedef Nothing Data;
	static Data makeData(const Token& token)
	{
		return Nothing();
	}
	template<typename T>
	static void annotate(T* symbol, const Nothing&)
	{
	}
};

struct AnnotateSrc
{
	typedef Source Data;
	static Data makeData(const Token& token)
	{
		return token.source;
	}
	template<typename T>
	static void annotate(T* symbol, const Source& source)
	{
		symbol->source = source;
	}
};

struct AnnotateId
{
	typedef Source Data;
	static Data makeData(const Token& token)
	{
		return token.source;
	}
	template<typename T>
	static void annotate(T* symbol, const Source& source)
	{
		symbol->value.source = source;
	}
};

struct SourceEvents : Source, IncludeEvents
{
	SourceEvents(const Source& source, const IncludeEvents& events)
		: Source(source), IncludeEvents(events)
	{
	}
};

struct AnnotateTop
{
	typedef SourceEvents Data;
	static Data makeData(const Token& token)
	{
		return SourceEvents(token.source, token.events);
	}
	static void annotate(cpp::declaration* symbol, const Data& data)
	{
		symbol->source = data;
		symbol->events = data;
	}
};



template<typename Inner, typename Annotate = AnnotateNull, typename Invoke = InvokeUncheckedResult, typename Cache = DisableCache, typename Defer = DeferDefault>
struct SemaPolicyGeneric : Inner, Annotate, Invoke, Cache, Defer
{
	typedef typename Inner::ArgsType ArgsType;
	SemaPolicyGeneric(const ArgsType& args = ArgsType())
		: Inner(args)
	{
	}
	const Inner& getInnerPolicy() const
	{
		return *this;
	}
	typedef Annotate AnnotateType;
	const Annotate& getAnnotatePolicy() const
	{
		return *this;
	}
	const Cache& getCachePolicy() const
	{
		return *this;
	}
	const Defer& getDeferPolicy() const
	{
		return *this;
	}
	const Invoke& getActionPolicy() const
	{
		return *this;
	}
};

#ifdef _WIN32
#define SEMA_INLINE __forceinline 
#endif

typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeNone> SemaPolicyNone;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeUnchecked> SemaPolicyIdentity;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateSrc, InvokeUnchecked> SemaPolicySrc;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeChecked> SemaPolicyIdentityChecked;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeUnchecked, EnableCache> SemaPolicyIdentityCached;
typedef SemaPolicyGeneric<SemaIdentity, AnnotateNull, InvokeChecked, EnableCache> SemaPolicyIdentityCachedChecked;
template<typename SemaT>
struct SemaPolicyPush : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateNull> {};
template<typename SemaT>
struct SemaPolicyPushCommit : SemaPolicyGeneric<SemaPush<SemaT, CommitEnable>, AnnotateNull> {};
template<typename SemaT>
struct SemaPolicyPushSrc : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateSrc> {};
template<typename SemaT>
struct SemaPolicyPushId : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateId> {};
template<typename SemaT>
struct SemaPolicyPushIdCommit : SemaPolicyGeneric<SemaPush<SemaT, CommitEnable>, AnnotateId> {};
template<typename SemaT>
struct SemaPolicyPushTop : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateTop> {};
template<typename SemaT>
struct SemaPolicyPushSrcChecked : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateSrc, InvokeCheckedResult> {};
template<typename SemaT>
struct SemaPolicyPushIdChecked : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateId, InvokeCheckedResult> {};
template<typename SemaT>
struct SemaPolicyPushChecked : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateNull, InvokeCheckedResult> {};
template<typename SemaT>
struct SemaPolicyPushCached : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateNull, InvokeUncheckedResult, CachedWalk> {};
template<typename SemaT>
struct SemaPolicyPushCachedChecked : SemaPolicyGeneric<SemaPush<SemaT>, AnnotateNull, InvokeCheckedResult, CachedWalk> {};
template<typename SemaT>
struct SemaPolicyPushBool : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<bool> >, AnnotateNull, InvokeUncheckedResult, DisableCache>
{
	SemaPolicyPushBool(bool value) : SemaPolicyGeneric(Args1<bool>(value))
	{
	}
};
template<typename SemaT>
struct SemaPolicyPushCheckedBool : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<bool> >, AnnotateNull, InvokeCheckedResult, DisableCache>
{
	SemaPolicyPushCheckedBool(bool value) : SemaPolicyGeneric(Args1<bool>(value))
	{
	}
};
template<typename SemaT>
struct SemaPolicyPushCachedBool : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<bool> >, AnnotateNull, InvokeUncheckedResult, CachedWalk>
{
	SemaPolicyPushCachedBool(bool value) : SemaPolicyGeneric(Args1<bool>(value))
	{
	}
};
template<typename SemaT>
struct SemaPolicyPushCachedCheckedBool : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<bool> >, AnnotateNull, InvokeCheckedResult, CachedWalk>
{
	SemaPolicyPushCachedCheckedBool(bool value) : SemaPolicyGeneric(Args1<bool>(value))
	{
	}
};
template<typename SemaT>
struct SemaPolicyPushIndexCommit : SemaPolicyGeneric<SemaPush<SemaT, CommitEnable, Args1<std::size_t> >, AnnotateNull, InvokeUncheckedResult, DisableCache>
{
	SemaPolicyPushIndexCommit(std::size_t value) : SemaPolicyGeneric(Args1<std::size_t>(value))
	{
	}
};

template<typename SemaT, typename Defer>
struct SemaPolicyPushDeferred : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args0>, AnnotateNull, InvokeUncheckedResult, DisableCache, Defer>
{
};




#define SEMA_POLICY(Symbol, Policy) \
	SEMA_INLINE Policy makePolicy(Symbol*) \
	{ \
		return Policy(); \
	}

#define SEMA_POLICY_ARGS(Symbol, Policy, args) \
	SEMA_INLINE Policy makePolicy(Symbol*) \
	{ \
		return Policy(args); \
	}

#define SEMA_BOILERPLATE \
	template<typename T> \
	SemaPolicyNone makePolicy(T* symbol) \
	{ \
		return SemaPolicyNone(); \
	}


struct SemaDeclSpecifierSeqResult
{
	Type type;
	IdentifierPtr forward; // if this is an elaborated-type-specifier, the 'identifier'
	CvQualifiers qualifiers;
	DeclSpecifiers specifiers;
	bool isUnion;
	SemaDeclSpecifierSeqResult(Declaration* declaration, const AstAllocator<int>& allocator)
		: type(declaration, allocator), forward(0), isUnion(false)
	{
	}
};

struct SemaDeclarationArgs
{
	bool isParameter;
	size_t templateParameter;
	SemaDeclarationArgs(bool isParameter = false, size_t templateParameter = INDEX_INVALID)
		: isParameter(isParameter), templateParameter(templateParameter)
	{
	}
};

template<typename SemaT>
struct SemaPolicyParameterDeclaration : SemaPolicyGeneric<SemaPush<SemaT, CommitNull, Args1<SemaDeclarationArgs> > >
{
	SemaPolicyParameterDeclaration(SemaDeclarationArgs value) : SemaPolicyGeneric(value)
	{
	}
};

struct SemaExpressionResult
{
	IdentifierPtr id; // only valid when the expression is a (parenthesised) id-expression
	UniqueTypeId type;
	ExpressionWrapper expression;
	/* 14.6.2.2-1
	...an expression is type-dependent if any subexpression is type-dependent.
	*/
	Dependent typeDependent;
	Dependent valueDependent;

	SemaExpressionResult()
		: id(0)
	{
	}
};

struct SemaTypeIdResult
{
	TypeId type;
	Once committed;
	SemaTypeIdResult(const AstAllocator<int>& allocator)
		: type(0, allocator)
	{
	}
};

struct SemaTemplateParameterClauseResult
{
	TemplateParameters params;
	SemaTemplateParameterClauseResult(const AstAllocator<int>& allocator)
		: params(allocator)
	{
	}
};

struct SemaTypeSpecifierResult
{
	Type type;
	unsigned fundamental;
	SemaTypeSpecifierResult(const AstAllocator<int>& allocator)
		: type(0, allocator), fundamental(0)
	{
	}
};

struct SemaTypenameSpecifierResult
{
	Type type;
	SemaTypenameSpecifierResult(const AstAllocator<int>& allocator)
		: type(0, allocator)
	{
	}
};

struct SemaNewTypeResult
{
	TypeId type;
	Dependent valueDependent;
	SemaNewTypeResult(const AstAllocator<int>& allocator)
		: type(0, allocator)
	{
	}
};

struct SemaSimpleDeclarationResult
{
	DeclarationPtr declaration; // the result of the declaration
	cpp::default_argument* defaultArgument; // parsing of this symbol will be deferred if this is a member-declaration

	SemaSimpleDeclarationResult()
		: declaration(0), defaultArgument(0)
	{
	}
};

struct SemaDeclarationResult
{
	DeclarationPtr declaration;

	SemaDeclarationResult()
		: declaration(0)
	{
	}
};

typedef SemaDeclarationResult SemaNamespaceResult;
typedef SemaDeclarationResult SemaEnumSpecifierResult;
typedef SemaDeclarationResult SemaExplicitInstantiationResult;
typedef SemaDeclarationResult SemaMemberDeclarationResult;

struct SemaQualifyingResult
{
	Qualifying qualifying;
	SemaQualifyingResult(const AstAllocator<int>& allocator)
		: qualifying(allocator)
	{
	}
};


struct SemaTemplateIdResult
{
	IdentifierPtr id;
	TemplateArguments arguments;
	SemaTemplateIdResult(AstAllocator<int>& allocator)
		: id(0), arguments(allocator)
	{
	}
};


struct SemaExplicitTypeExpressionResult
{
	SEMA_BOILERPLATE;

	TypeId type;
	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWrapper expression;
	SemaExplicitTypeExpressionResult(AstAllocator<int>& allocator)
		: type(0, allocator)
	{
	}
};

struct SemaElaboratedTypeSpecifierResult
{
	SEMA_BOILERPLATE;

	Type type;
	IdentifierPtr id;
	SemaElaboratedTypeSpecifierResult(AstAllocator<int>& allocator)
		: type(0, allocator), id(0)
	{
	}
};


struct SemaClassSpecifierResult
{
	DeclarationPtr declaration;
	IdentifierPtr id;
	TemplateArguments arguments;
	bool isUnion;
	bool isSpecialization;
	SemaClassSpecifierResult(AstAllocator<int>& allocator)
		: declaration(0), id(0), arguments(allocator), isUnion(false), isSpecialization(false)
	{
	}
};


struct SemaQualified : public SemaBase, SemaQualifyingResult
{
	SemaQualified(const SemaState& state)
		: SemaBase(state), SemaQualifyingResult(context)
	{
	}

	void setQualifyingGlobal()
	{
		SEMANTIC_ASSERT(qualifying.empty());
		qualifying_p = context.globalType.get_ref();
		qualifyingScope = qualifying_p->declaration;
		qualifyingClass = 0;
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
	void swapQualifying(const Qualifying& other, bool isDeclarator = false)
	{
		qualifying = other;
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
				qualifyingClass = &getSimpleType(getUniqueType(*qualifying_p, getInstantiationContext(), isDeclarator).value);
				qualifyingScope = qualifyingClass->declaration;
			}
		}
	}
};

#endif
