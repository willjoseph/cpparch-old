



// anonymous template type param
template<class>
class TmplFwd;

// non-type template param
template<int _SizeFIXED>
void f()
{
}



// deferred name lookup
namespace N504
{
	template<class T>
	struct Iterator
	{
		typedef typename T::Ref Ref; // cannot be looked up without instantiating

		Ref operator*()const
		{
		}
	};

	struct Value
	{
		int v;
		typedef Value& Ref;
	};

	void f()
	{
		Iterator<Value> i;
		(*i).v=0;
	}
}



//----------
// default template params
namespace N502
{
	template<typename T, typename Ref = T&>
	struct Tmpl
	{
		Ref operator*() const
		{
		}
		void f(T& t)
		{
			Ref tmp = **this;
		}
	};
}

//----------
// member type instantiation
namespace N503
{

struct Value
{
	int v;
};

template<typename T>
struct Iterator
{
	T operator*()
	{
	}
};

void f()
{
	Iterator<Value> i;
	(*i).v = 0; // v not dependent in context of i
}

struct Value2
{
	int v2;
};

template<typename T>
struct Tmpl
{
	T m;
	typedef T Type;

#if 0
	template<typename T2>
	struct Tmpl2
	{
		typedef T2 Type;
	};

	typename Tmpl2<Value2>::Type m2;
	typename Tmpl<T>::Type m3;
	typename Tmpl2<Value>::Type m4;
#endif
};

void f()
{
	Tmpl<Value> i;
	i.m.v = 0; // v not dependent in context of i
#if 0
	// i.m -> Tmpl<Value>:T -> Value
	i.m2.v2 = 0; // v2 not dependent in context of i
	// i.m2 -> Tmpl<Value>:Tmpl<Value2>::Type -> Tmpl<Value>::Tmpl<Value2>::T -> Value2
	i.m3.v = 0; // v not dependent in context of i
	// i.m3 -> Tmpl<Value>:Tmpl<T>::Type -> Tmpl<Value>:Tmpl<T>::T -> Tmpl<Value>:T -> Value
#endif
}

void f()
{
	Tmpl< Tmpl<Value> > i;
	i.m.m.v = 0; // v not dependent in context of i
	// i.m -> T -> Tmpl<Value>
	// i.m.m -> T -> Value
}

void f()
{
	Tmpl< Tmpl<Value>::Type > i;
	i.m.v = 0; // v not dependent in context of i
	// i.m -> Tmpl<Value>::Type -> T -> Value
}

} // N503

//----------
// dependent member (via arguments)
namespace N501
{
	template<class T>
	class Tmpl
	{
		int f(T t)
		{
			return (f(t).dependent);
		}
	};
}


#if 0 //TODO: crash (pair not defined)
template<class _Traits>
class _Tree
{
public:
	typedef typename _Traits::value_type value_type;

	class iterator
	{
	};
	typedef pair<iterator, bool>_Pairib;

	_Pairib insert(const value_type&_Val)
	{
	}
	iterator insert(iterator _Where, const value_type&_Val)
	{
		return (insert(_Val).first);
	}
	template<class _Iter>
	void insert(_Iter _First, _Iter _Last)
	{
	}
};

#endif




//----------
// dependent member (via base class)

template<typename T>
struct C2 : T
{
	void f()
	{
		T::dependent.dependent();
	}
};

struct C
{
	int m;
	void f()
	{
		int m;
		m = 0;
		this->m = 0;
	}
};

void f(int a)
{
	//(a.x == a);
	C c;
	c.m = a;
}

//----------
// dependent base class

struct Arg
{
	typedef Arg Type;
};

template<typename T1>
struct Derived1 : T1::Type
{
	typedef T1 Type;
};

template<typename T2>
struct Derived2 : Derived1<T2>
{
	typedef T2 Type;
};

template<typename T3>
struct Derived3 : T3::Type::Type
{
	typedef T3 Type;
};

Derived2<Arg> d2;
Derived3<Arg> d3;

struct Arg2
{
	template<typename T1>
	struct Derived : T1::Type
	{
	};
	typedef Arg Type;
	Derived<Arg> d;
};

//----------
// illegal: violates ODR

template<typename T>
struct OneDefinition
{
};

OneDefinition<int> o1;

template<>
struct OneDefinition<int>
{
};

OneDefinition<int> o2;

// -------

// dependent base class
//#include "boost\wave\util\time_conversion_helper.hpp"

// -------
template<class T>
struct Traits
{
};

template<>
struct Traits<char>
{
};

template<>
struct Traits<wchar_t>
{
};

template<>
struct Traits<signed> // accept 'signed' as signed-int
{
};

template<>
struct Traits<short> // accept 'short' as short-int
{
};

// ------

void overloaded(void*);

void bleh()
{
	overloaded(0);
}

typedef int* P;
P* pp;

struct S
{
} s;

S* f(S);
double* f(double);
float* f(float);
int* f(int);
long* f(long);
unsigned* f(unsigned);
unsigned long* f(unsigned long);
char* f(char);
wchar_t* f(wchar_t);
char** f(const char*);
wchar_t** f(const wchar_t*);
long double* f(const long double);

void f()
{
	&s;
	S* p;
	*p;
	p + 3;

	int i;
	-i;
	+i;
	~i;
	!i;

	s, 0;
	f(s);
	f(9 * (8.2));
	f(9 * 8.2);
	f(3 * (9 * 8.2));
	f(1 * 2U * 3.0);

	f('\0');
	f(L'\0');
	f("");
	f(L"");
	f(0.1);
	f(0.1f);
	f(0.1l);
	f(1);
	f(1L);
	f(1U);
	f(1UL);
	f(1l);
	f(1u);
	f(1ul);

}

typedef int* A[1]; // array of pointer to

void f(A(*x)[1]) // pointer to array of
{
}
void f(int*(*x)[1][1])// pointer to array of array of pointer to
{
}

void f(int*(*x)[1]);

typedef void (*Pf) (float hidden);

void f(Pf pf(int a));


void f(void (*pf(int a)) (float hidden))
{
}

void f(int*) // pointer to
{
}
void f(int**) // pointer to pointer to
{
}
void f(int*()) // function returning pointer to
{
}
void f(int**()) // function returning pointer to pointer to
{
}
void f(int*[]) // array of pointer to
{
}
void f(int(*)()) // pointer to function returning
{
}
void f(int*(*)()) // pointer to function returning pointer to
{
}
void f(int(*)[]) // pointer to array of
{
}

void f(int(*())[]) // function returning pointer to array of
{
}
void f(int(*[])()) // array of pointer to function returning
{
}

void f(int(S::*())[]) // function returning member-pointer to array of
{
}
void f(int(S::*[])()) // array of member-pointer to function returning
{
}

// not allowed:
// *[](): array of function returning pointer to
// (*)[](): pointer to array of function returning
// *()[]: function returning array of pointer to
// (*)()[]: pointer to function returning array of


template<typename T>
struct C99
{
	template<class _Other>
	operator C99<_Other>()
	{
		return (C99<_Other>(*this));
	}
};

int f(void (*)(void));



