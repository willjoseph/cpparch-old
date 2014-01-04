
#ifndef INCLUDED_CPPPARSE_AST_SCOPE_H
#define INCLUDED_CPPPARSE_AST_SCOPE_H

#include "Declaration.h"

// ----------------------------------------------------------------------------
// scope

struct UniqueName : public Identifier
{
	char buffer[10];
	UniqueName(size_t index)
	{
		sprintf(buffer, "$%x", unsigned(index));
		Identifier::value = TokenValue(buffer);
	}
};
typedef std::vector<UniqueName*> UniqueNames;
extern UniqueNames gUniqueNames;

enum ScopeType
{
	SCOPETYPE_UNKNOWN,
	SCOPETYPE_NAMESPACE, // contains class, enum, object or function declarations
	SCOPETYPE_FUNCTION, // contains declarations of function parameters
	SCOPETYPE_LOCAL, // contains statements
	SCOPETYPE_CLASS, // contains member declarations
	SCOPETYPE_TEMPLATE, // contains template parameter declarations
};

extern size_t gScopeCount;

struct ScopeCounter
{
	ScopeCounter()
	{
		++gScopeCount;
	}
	ScopeCounter(const ScopeCounter&)
	{
		++gScopeCount;
	}
	~ScopeCounter()
	{
		--gScopeCount;
	}
};

const std::size_t VISIBILITY_ALL = UINT_MAX;

// An instance of a declaration - multiple declarations may refer to the same entity.
// e.g. definition, forward declaration, redeclaration
struct DeclarationInstance : DeclarationPtr
{
	Identifier* name; // the identifier used in this declaration.
	const DeclarationInstance* overloaded; // the previously declared overload of this name (which may or may not refer to the same entity.)
	const DeclarationInstance* redeclared; // the previous declaration that refers to the same entity.
	std::size_t visibility; // every declaration declared before this has a lesser value
	DeclarationInstance()
		: DeclarationPtr(0), name(0), overloaded(0), redeclared(0), visibility(VISIBILITY_ALL)
	{
	}
	// used when cloning an existing declaration, in the process of copying declarations from one scope to another.
	explicit DeclarationInstance(Declaration* declaration, std::size_t visibility = 0)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0), visibility(visibility)
	{
		SYMBOLS_ASSERT(name != 0);
	}
#if 0
	explicit DeclarationInstance(DeclarationPtr declaration)
		: DeclarationPtr(declaration), name(declaration != 0 ? &declaration->getName() : 0), overloaded(0), redeclared(0)
	{
		SYMBOLS_ASSERT(name != 0);
	}
#endif
};

inline bool operator<(const DeclarationInstance& l, const DeclarationInstance& r)
{
	return l.name < r.name;
}

inline cpp::terminal_identifier& getDeclarationId(const DeclarationInstance* declaration)
{
	return (*declaration)->getName();
}

inline bool isDecorated(const Identifier& id)
{
	return id.dec.p != 0;
}

inline const DeclarationInstance& getDeclaration(const Identifier& id)
{
	SYMBOLS_ASSERT(isDecorated(id));
	return *id.dec.p;
}


struct DeclarationInstanceRef
{
	const DeclarationInstance* p;
	DeclarationInstanceRef()
		: p(0)
	{
	}
	DeclarationInstanceRef(const DeclarationInstance& p)
		: p(&p)
	{
		checkAllocated(this->p);
	}
	Declaration& operator*() const
	{
		checkAllocated(p);
		return p->operator*();
	}
	Declaration* operator->() const
	{
		checkAllocated(p);
		return p->operator->();
	}
	operator const DeclarationInstance&() const
	{
		checkAllocated(p);
		return *p;
	}
	operator Declaration*() const
	{
		checkAllocated(p);
		return p == 0 ? 0 : static_cast<Declaration*>(*p);
	}
};

struct Scope : public ScopeCounter
{
	ScopePtr parent;
	Identifier name;
	size_t enclosedScopeCount; // number of scopes directly enclosed by this scope
	typedef std::less<TokenValue> IdentifierLess;

	typedef std::multimap<TokenValue, DeclarationInstance, IdentifierLess, AstAllocator<int> > Declarations2;

	struct Declarations : public Declarations2
	{
		Declarations(const AstAllocator<int>& allocator)
			: Declarations2(IdentifierLess(), allocator)
		{
		}
		Declarations(const Declarations& other)
			: Declarations2(other)
		{
			SYMBOLS_ASSERT(other.empty());
		}
		~Declarations()
		{
			SYMBOLS_ASSERT(Declarations2::empty());
			// hack: stop declarations being cleared
			new(static_cast<Declarations2*>(this)) Declarations2(IdentifierLess(), AST_ALLOCATOR_NULL);
		}

		const DeclarationInstance& insert(const DeclarationInstance& declaration)
		{
			SYMBOLS_ASSERT(declaration.name != 0);
			Declarations2::iterator result = Declarations2::insert(Declarations2::value_type(declaration.name->value, declaration));
			return (*result).second;
		}
	};


	Declarations declarations;
	ScopeType type;
	Types bases;
	typedef List<ScopePtr, AstAllocator<int> > Scopes;
	Scopes usingDirectives;
	typedef List<DeclarationPtr, AstAllocator<int> > DeclarationList;
	DeclarationList declarationList;
	size_t templateDepth;

	Scope(const AstAllocator<int>& allocator, const Identifier& name, ScopeType type = SCOPETYPE_UNKNOWN)
		: parent(0), name(name), enclosedScopeCount(0), declarations(allocator), type(type), bases(allocator), usingDirectives(allocator), declarationList(allocator), templateDepth(0)

	{
	}
	~Scope()
	{
	}

	Identifier& getUniqueName()
	{
		if(enclosedScopeCount == gUniqueNames.size())
		{
			gUniqueNames.push_back(new UniqueName(enclosedScopeCount));
		}
		return *gUniqueNames[enclosedScopeCount++];
	}

private:
	//Scope(const Scope&);
	//Scope& operator=(const Scope&);
};


inline Scope::Declarations::iterator findDeclaration(Scope::Declarations& declarations, const DeclarationInstance* declaration)
{
	const Identifier& id = *declaration->name;
	Scope::Declarations::iterator i = declarations.upper_bound(id.value);

	for(; i != declarations.begin()
		&& (*--i).first == id.value;)
	{
		if(&(*i).second == declaration)
		{
			return i;
		}
	}

	return declarations.end();
}

inline void undeclare(const DeclarationInstance* p, LexerAllocator& allocator)
{
	SYMBOLS_ASSERT(p->name->dec.p == 0 || p->name->dec.p == p);
	p->name->dec.p = 0;

	Declaration* declaration = *p;
	SYMBOLS_ASSERT(!declaration->scope->declarations.empty());
	SYMBOLS_ASSERT(!declaration->scope->declarationList.empty());

	SYMBOLS_ASSERT(declaration == declaration->scope->declarationList.back());
	declaration->scope->declarationList.pop_back(); // TODO: optimise

	Scope::Declarations::iterator i = findDeclaration(declaration->scope->declarations, p);
	SYMBOLS_ASSERT(i != declaration->scope->declarations.end());
	declaration->scope->declarations.erase(i);

}

inline BacktrackCallback makeUndeclareCallback(const DeclarationInstance* p)
{
	BacktrackCallback result = { BacktrackCallbackThunk<const DeclarationInstance, undeclare>::thunk, const_cast<DeclarationInstance*>(p) };
	return result;
}

static void backtrackError(void* data, LexerAllocator& allocator)
{
	throw SymbolsError(); // cannot backtrack before this point!
}

inline BacktrackCallback makeBacktrackErrorCallback()
{
	BacktrackCallback result = { backtrackError, 0 };
	return result;
}

inline const Scope& nullScope()
{
	static Scope null(AST_ALLOCATOR_NULL, IDENTIFIER_NULL);
	return null;
}
#define SCOPE_NULL nullScope()


inline bool isTemplate(const Scope& scope)
{
	return scope.templateDepth != 0;
}

inline Scope* getEnclosingClass(Scope* scope)
{
	for(; scope != 0; scope = scope->parent)
	{
		if(scope->type == SCOPETYPE_CLASS
			&& *scope->name.value.c_str() != '$') // ignore anonymous union
		{
			return scope;
		}
	}
	return 0;
}

inline Scope* getEnclosingFunction(Scope* scope)
{
	for(; scope != 0; scope = scope->parent)
	{
		if(scope->type == SCOPETYPE_FUNCTION)
		{
			return scope;
		}
	}
	return 0;
}



#endif
