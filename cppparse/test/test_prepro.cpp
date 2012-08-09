


template<typename Input>
struct multi_pass_shared : Input
{
};
template<typename Input>
struct default_policy
{
	template<typename T>
	struct shared : multi_pass_shared<T>
	{
		typedef typename Input::template DependentTmpl<T>::Dependent Type; // template parameter 'multi_pass_shared::Input' should not be found via base class
		// TODO: crash when parse fails here!
	};
};

namespace N508
{
	struct Type
	{
	};

	struct S
	{
		operator Type()
		{
			return Type();
		}
	};
	void f(S& s)
	{
		s.operator Type(); // Type should be looked up in context of entire postfix-expression
	}
}

namespace N507
{
	struct S
	{
		struct Type
		{
		};
		operator Type()
		{
			return Type();
		}
	};
	void f(S& s)
	{
		s.operator Type(); // Type should be looked up in context of S
	}
}


namespace N
{
	struct Base2
	{
		int m;
		static int ms;
	};
}

struct Base
{
	int m;
	static int ms;
};

struct NotBase
{
	int m;
	static int ms;
};

template<typename T>
struct TmplBase
{
	int m;
	static int ms;
};

struct Derived : Base, N::Base2, TmplBase<int>, TmplBase<float>
{
};

void f()
{

	Derived derived;
	Derived::Base::ms = 0;
	//Derived::NotBase::ms = 0; // error!
	//derived.NotBase::m = 0; // error!
	derived.Base::m = 0; // Base looked up in Derived (also found in context of entire postfix-expression)
	derived.Base2::m = 0; // Base2 looked up in Derived (not found in context of entire postfix-expression)
	derived.N::Base2::m = 0; // N looked up in context of entire postfix-expression (not found in Derived)
	derived.::N::Base2::m = 0; // N looked up in global scope
	Derived::TmplBase<int>::ms = 0;
	derived.TmplBase<int>::m = 0;
	Derived::TmplBase<float>::ms = 0;
	derived.TmplBase<float>::m = 0;
}

class C
{
};

C& operator*(C& c)
{
	return *c;
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



