
#include "test.h"


#if 1

namespace N
{
	template<typename T>
	class C
	{
		template<typename X>
		void f()
		{
			N::undeclared<X> x;
		}
	};
}

struct S
{
};

template<typename First, typename Second>
int f(First x, Second y);

int x = f(S(), int());

template<typename Second, typename First>
int f(Second a, First b)
{
	return b;
}

#endif


struct symbol1
{
	struct symbol2* inner;
};

struct symbol2
{
	struct symbol1* first;
	struct symbol3* second;
};

struct symbol3
{
};

#if 0

template<class T>
bool parse(T)
{
	return true;
}


struct State
{
};

struct WalkerN
{
	struct Walker1 : State
	{
		int a;
		Walker1(State base) : State(base)
		{
		}
		void visit(symbol1*)
		{
			Walker2 walker(*this);
			if(!parse(walker)) return;
			int b = walker.b;
		}
	};
	struct Walker2 : State
	{
		int b;
		Walker2(State base) : State(base)
		{
		}
		void visit(symbol1*)
		{
			Walker1 walker(*this);
			if(!parse(walker)) return;
			int a = walker.a;
		}
	};
};
#else

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

template<typename Inner, bool CHECKED, typename Args = Args0>
struct SemaPolicyPushGeneric : Args
{
	SemaPolicyPushGeneric(Args args = Args())
		: Args(args)
	{
	}
	template<typename WalkerType, typename T, typename Inner>
	static bool invokeAction(WalkerType& walker, T* symbol, const Inner* inner)
	{
		return inner == 0 ? false : walker.action(symbol, inner->result);
	};
};

template<typename Inner, typename Args>
struct SemaPolicyPushGeneric<Inner, false, Args> : Args
{
	SemaPolicyPushGeneric(Args args = Args())
		: Args(args)
	{
	}
	template<typename WalkerType, typename T, typename Inner>
	static bool invokeAction(WalkerType& walker, T* symbol, const Inner* inner)
	{
		if(inner == 0)
		{
			return false;
		}
		walker.action(symbol, inner->result);
		return true;
	};
};

template<typename WalkerType, bool CHECKED, typename Inner>
Inner makeWalker(WalkerType& walker, SemaPolicyPushGeneric<Inner, CHECKED, Args0> args)
{
	return Inner(walker);
}

template<typename WalkerType, bool CHECKED, typename Inner, typename A1>
Inner makeWalker(WalkerType& walker, SemaPolicyPushGeneric<Inner, CHECKED, Args1<A1> > args)
{
	return Inner(walker, args.a1);
}

template<typename WalkerType, bool CHECKED, typename Inner, typename A1, typename A2>
Inner makeWalker(WalkerType& walker, SemaPolicyPushGeneric<Inner, CHECKED, Args2<A1, A2> > args)
{
	return Inner(walker, args.a1, args.a2);
}

template<bool CHECKED>
struct SemaPolicyIdentityGeneric
{
	template<typename WalkerType, typename T, typename Inner>
	static bool invokeAction(WalkerType& walker, T* symbol, const Inner* inner)
	{
		return inner == 0 ? false : walker.action(symbol);
	};
};

template<>
struct SemaPolicyIdentityGeneric<false>
{
	template<typename WalkerType, typename T, typename Inner>
	static bool invokeAction(WalkerType& walker, T* symbol, const Inner* inner)
	{
		if(inner == 0)
		{
			return false;
		}
		walker.action(symbol);
		return true;
	};
};

template<typename WalkerType, bool CHECKED>
WalkerType& makeWalker(WalkerType& walker, SemaPolicyIdentityGeneric<CHECKED>)
{
	return walker;
}


template<typename WalkerType, typename T>
bool visit(WalkerType& walker, T* symbol)
{
	return walker.makePolicy(symbol).invokeAction(walker, symbol, parseSymbol(makeWalker(walker, walker.makePolicy(symbol)), symbol));
}	

#define TREEWALKER_TRY(Symbol) \
	SemaPolicyIdentityGeneric<false> makePolicy(Symbol*) \
	{ \
		return SemaPolicyIdentityGeneric<false>(); \
	}

#define TREEWALKER_TRY_CHECKED(Symbol) \
	SemaPolicyIdentityGeneric<true> makePolicy(Symbol*) \
	{ \
		return SemaPolicyIdentityGeneric<true>(); \
	}

#define TREEWALKER_PUSH(Symbol, Walker) \
	SemaPolicyPushGeneric<Walker, false> makePolicy(Symbol*) \
	{ \
		return SemaPolicyPushGeneric<Walker, false>(); \
	}

#define TREEWALKER_PUSH_ARGS(Symbol, Walker, Args, args) \
	SemaPolicyPushGeneric<Walker, false, Args > makePolicy(Symbol*) \
	{ \
		return SemaPolicyPushGeneric<Walker, false, Args >(Args args); \
	}

#define TREEWALKER_PUSH_CHECKED(Symbol, Walker) \
	SemaPolicyPushGeneric<Walker, true> makePolicy(Symbol*) \
	{ \
		return SemaPolicyPushGeneric<Walker, true>(); \
	}

#define TREEWALKER_PUSH_ARGS_CHECKED(Symbol, Walker, Args, args) \
	SemaPolicyPushGeneric<Walker, true, Args > makePolicy(Symbol*) \
	{ \
		return SemaPolicyPushGeneric<Walker, true, Args >(Args args); \
	}


template<typename WalkerType>
WalkerType* parseSymbol(WalkerType& walker, symbol1* p)
{
	p = new symbol1();
	if(!visit(walker, p->inner))
	{
		return 0;
	}
	return &walker;
}

template<typename WalkerType>
WalkerType* parseSymbol(WalkerType& walker, symbol2* p)
{
	p = new symbol2();
	visit(walker, p->first);
	visit(walker, p->second);
	return 0;
}

template<typename WalkerType>
WalkerType* parseSymbol(WalkerType& walker, symbol3* p)
{
	p = new symbol3();
	return &walker;
}

struct Symbol1Result
{
	int value;
};

struct Symbol2Result
{
	int value;
};

struct State
{
};

typedef Args1<bool> Symbol1WalkerArgs;
typedef Args0 Symbol2WalkerArgs;

struct Symbol1Walker : State
{
	Symbol1Result result;
	Symbol1Walker(State base, bool a1) : State(base)
	{
	}

	TREEWALKER_PUSH_ARGS_CHECKED(symbol2, struct Symbol2Walker, Symbol2WalkerArgs, ())
	bool action(symbol2* symbol, const Symbol2Result& inner)
	{
		result.value = inner.value;
		return true;
	}
};

struct Symbol2Walker : State
{
	Symbol2Result result;
	Symbol2Walker(State base) : State(base)
	{
	}

	TREEWALKER_PUSH_ARGS(symbol1, struct Symbol1Walker, Symbol1WalkerArgs, (false))
	void action(symbol1* symbol, const Symbol1Result& inner)
	{
		result.value = inner.value;
	}

	TREEWALKER_TRY_CHECKED(symbol3)
	bool action(symbol3* symbol)
	{
		return true;
	}
};

void test()
{
	symbol1* symbol = 0;
	Symbol1Walker walker = Symbol1Walker(State(), false);
	parseSymbol(walker, symbol);
}

#if 0
struct TESTS
{
	TESTS()
	{
		test();
	}
} gTESTS;
#endif

#endif
