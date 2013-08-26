
#ifndef INCLUDED_CPPPARSE_CORE_NAMELOOKUP_H
#define INCLUDED_CPPPARSE_CORE_NAMELOOKUP_H

#include "Ast/Type.h"
#include "Ast/Print.h"

struct LookupFilter
{
	typedef bool (*Function)(void* context, const DeclarationInstance& declaration);
	Function function;
	void* context;

	bool operator()(const DeclarationInstance& declaration)
	{
		return function(context, declaration);
	}
};

inline bool isAny(const Declaration& declaration)
{
	// always ignore constructors during name-lookup
	return declaration.type.declaration != &gCtor;
}


template<typename T>
struct LookupFilterThunk
{
	static bool apply(void* context, const DeclarationInstance& declaration)
	{
		return (*static_cast<T*>(context))(*declaration);
	}
};

template<typename T>
LookupFilter makeLookupFilter(T& filter)
{
	LookupFilter result = { LookupFilterThunk<T>::apply, &filter };
	return result;
}

template<bool filter(const Declaration& declaration)>
struct LookupFilterDefault : LookupFilter
{
	explicit LookupFilterDefault(std::size_t visibility = VISIBILITY_ALL)
	{
		LookupFilter::context = reinterpret_cast<void*>(visibility);
		LookupFilter::function = apply;
	}
	static bool apply(void* context, const DeclarationInstance& declaration)
	{
		std::size_t visibility = reinterpret_cast<std::size_t>(context);
		return declaration.visibility < visibility
			&& filter(*declaration);
	}
};

typedef LookupFilterDefault<isAny> IsAny;

inline bool isConstructor(const Declaration& declaration)
{
	return declaration.type.declaration == &gCtor;
}

typedef LookupFilterDefault<isConstructor> IsConstructor;

inline bool isTypeName(const Declaration& declaration)
{
	return isType(declaration);
}

typedef LookupFilterDefault<isTypeName> IsTypeName;

inline bool isNamespaceName(const Declaration& declaration)
{
	return isNamespace(declaration);
}

typedef LookupFilterDefault<isNamespaceName> IsNamespaceName;


// returns true if \p declaration is a template class, function or template-parameter
inline bool isTemplateName(const Declaration& declaration)
{
	return declaration.isTemplateName;
}

inline bool isNestedName(const Declaration& declaration)
{
	return isTypeName(declaration)
		|| isNamespaceName(declaration);
}

typedef LookupFilterDefault<isNestedName> IsNestedName;


inline bool isNonMemberName(const Declaration& declaration)
{
	return isNonMember(declaration);
}

typedef LookupFilterDefault<isNonMemberName> IsNonMemberName;

inline bool isFunctionName(const Declaration& declaration)
{
	return isFunction(declaration);
}

typedef LookupFilterDefault<isFunctionName> IsFunctionName;





// ----------------------------------------------------------------------------

struct LookupResult
{
	const DeclarationInstance* filtered; // the declaration found by the name-lookup, using the filter
	const SimpleType* enclosing;

	LookupResult()
		: filtered(0), enclosing(0)
	{
	}
	operator const DeclarationInstance*() const
	{
		return filtered;
	}

	// Combines the result of a subsequent lookup, returns true if lookup succeeded
	bool append(const LookupResult& other)
	{
		*this = other;
		return filtered != 0;
	}
};

struct LookupResultRef : DeclarationInstanceRef
{
	LookupResultRef()
	{
	}
	LookupResultRef(const DeclarationInstance& p)
		: DeclarationInstanceRef(p)
	{
	}
	LookupResultRef(const LookupResult& result)
		: DeclarationInstanceRef(*result.filtered)
	{
	}
};


inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter = IsAny());

inline const DeclarationInstance* findDeclaration(Scope::Declarations& declarations, const Identifier& id, LookupFilter filter)
{
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(filter((*i).second)) // if the filter passes this declaration
		{
			return &(*i).second;
		}
	}

	return 0;
}

struct RecursionGuard
{
	const SimpleType& instance;
	RecursionGuard(const SimpleType& instance)
		: instance(instance)
	{
		SYMBOLS_ASSERT(!instance.visited);
		instance.visited = true;
	}
	~RecursionGuard()
	{
		instance.visited = false;
	}
};

inline LookupResult findDeclaration(const SimpleType& instance, const Identifier& id, LookupFilter filter)
{
	SYMBOLS_ASSERT(instance.declaration->enclosed != 0);
	SYMBOLS_ASSERT(instance.instantiated); // the qualifying type should have been instantiated by this point
	SYMBOLS_ASSERT(instance.allowLookup);

	LookupResult result;

	if(instance.visited) // TODO: this occurs when the same type is found twice when searching bases: can be caused by two templates that differ only in non-type template arguments
	{
		std::cout << "duplicate base: ";
		printType(instance);
		std::cout << std::endl;
		return result;
	}
	RecursionGuard guard(instance);

	result.filtered = findDeclaration(instance.declaration->enclosed->declarations, id, filter);
	if(result.filtered)
	{
		result.enclosing = &instance;
		return result;
	}
	for(UniqueBases::const_iterator i = instance.bases.begin(); i != instance.bases.end(); ++i)
	{
		const SimpleType& base = *(*i);
		SYMBOLS_ASSERT(base.declaration->enclosed != 0); // TODO: non-fatal error: incomplete type
		SYMBOLS_ASSERT(base.declaration->enclosed->usingDirectives.empty()); // namespace.udir: A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.

		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->getName().value == id.value
			&& filter(DeclarationInstance(base.declaration)))
		{
			result.filtered = &getDeclaration(base.declaration->getName());
			result.enclosing = base.enclosing;
			return result;
		}

		if(result.append(findDeclaration(base, id, filter)))
		{
			return result;
		}
	}
	return result;
}



// find a declaration within a scope.
inline LookupResult findDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	result.filtered = findDeclaration(scope.declarations, id, filter);
	return result;
}

inline LookupResult findNamespaceDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny());

// find a declaration within the set of using-directives present in a namespace
inline LookupResult findDeclaration(Scope::Scopes& scopes, const Identifier& id, LookupFilter filter = IsAny())
{
	LookupResult result;
	for(Scope::Scopes::iterator i = scopes.begin(); i != scopes.end(); ++i)
	{
		Scope& scope = *(*i);

#ifdef LOOKUP_DEBUG
		std::cout << "searching '";
		printName(scope);
		std::cout << "'" << std::endl;
#endif

		if(result.append(findNamespaceDeclaration(scope, id, filter)))
		{
			return result;
		}
	}
	return result;
}

// find a declaration within a namespace scope. Does not search enclosing scopes.
inline LookupResult findNamespaceDeclaration(Scope& scope, const Identifier& id, LookupFilter filter)
{
	SYMBOLS_ASSERT(scope.type == SCOPETYPE_NAMESPACE);
	LookupResult result;
	if(result.append(::findDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(result.append(::findDeclaration(scope.usingDirectives, id, filter)))
	{
		return result;
	}
	return result;
}

inline LookupResult findMemberDeclaration(Scope& scope, const Identifier& id, LookupFilter filter)
{
	LookupResult result;
	if(result.append(findDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(scope.type != SCOPETYPE_CLASS)
	{
		return result;
	}
	for(Types::iterator i = scope.bases.begin(); i != scope.bases.end(); ++i)
	{
		SYMBOLS_ASSERT((*i).unique != 0);
		if((*i).isDependent) // if base class is dependent
		{
			continue;
		}
		const SimpleType& base = getSimpleType((*i).unique);

		// an identifier looked up in the context of a class may name a base class
		if(base.declaration->getName().value == id.value
			&& filter(DeclarationInstance(base.declaration)))
		{
			result.filtered = base.declaration->getName().dec.p;
			result.enclosing = base.enclosing;
			return result;
		}
		Scope* scope = base.declaration->enclosed;
		if(scope != 0)
		{
			// [namespace.udir] A using-directive shall not appear in class scope, but may appear in namespace scope or in block scope.
			SYMBOLS_ASSERT(scope->usingDirectives.empty());
			if(result.append(findMemberDeclaration(*scope, id, filter)))
			{
				return result;
			}
		}
	}
	return result;
}

// find a declaration within a class or namespace
inline LookupResult findClassOrNamespaceMemberDeclaration(Scope& scope, const Identifier& id, LookupFilter filter = IsAny())
{
#ifdef LOOKUP_DEBUG
	std::cout << "searching '";
	printName(scope);
	std::cout << "'" << std::endl;
#endif

	LookupResult result;
	if(result.append(findMemberDeclaration(scope, id, filter)))
	{
		return result;
	}
	if(scope.parent != 0)
	{
		if(result.append(findClassOrNamespaceMemberDeclaration(*scope.parent, id, filter)))
		{
			return result;
		}
	}
	/* basic.lookup.unqual
	The declarations from the namespace nominated by a using-directive become visible in a namespace enclosing
	the using-directive; see 7.3.4. For the purpose of the unqualified name lookup rules described in 3.4.1, the
	declarations from the namespace nominated by the using-directive are considered members of that enclosing
	namespace.
	*/
	result.append(findDeclaration(scope.usingDirectives, id, filter));
	return result;
}


#endif
