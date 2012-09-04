
#if 1
namespace N023
{
	template<typename T>
	struct Wrapper
	{
	};

	template<typename F, class A=Wrapper<F> >
	struct Tmpl;

	template<typename E, class A> // the default-argument for A should be merged from the forward-declaration
	struct Tmpl
	{
		typedef A Type;
	};

	Tmpl<char>::Type x;
}
#endif

namespace N031
{
	template<typename T>
	struct Tmpl
	{
		Tmpl f(); // 'Tmpl' should resolve to 'Tmpl<T>'
	};

	template<>
	struct Tmpl<int>
	{
		Tmpl f(); // 'Tmpl' should resolve to 'Tmpl<int>'
	};

	void f()
	{
		Tmpl<float> x;
		x.f();
		Tmpl<int> y;
		y.f();
	}
}

#if 0 // TODO: friend function in template
namespace N030
{
	template<typename T>
	struct S
	{
		friend void f(T);
	};

	template<typename T>
	struct X : S<X<T> >
	{
	};

	void f()
	{
		X<int> i;
		f(i);
	}
}
#endif

#if 0
namespace N028
{
	template<typename T>
	struct Base
	{
		typedef T Type;
		typedef Base<T> Nested;
		T member;
	};

	struct Derived : Base<int> // test type-uniquing for names that are implicitly qualified by 'Base<int>'
	{
		Type g();
		Nested::Type h();
		void f()
		{
			member = 0;
			int t = Type(0); // 'Type' should be resolved to 'int'
			int u = (Type)0; // 'Type' should be resolved to 'int'
		}
	};
}
#endif

namespace N027
{

	template<typename T>
	class match
	{
		match();
		int val;
	};

	template<>
	class match<int>
	{
	};

	template<typename T>
	inline match<T>::match(): val() // 'val' should be looked up in match<T>, not match<int>
	{
	}
}

namespace N029
{
	struct nil_t {};

	template<typename T=nil_t>
	class match
	{
		match();
		int len;
		int val;
	};
	template<>
	class match<nil_t>
	{
		match();
		int len;
	};

	template<typename T>
	inline match<T>::match(): len(-1), val() // 'val' should be looked up in match<T>, not match<nil_t>
	{
	}

}

namespace N025
{
	template<class _Elem, class _OutIt>
	struct time_put
	{
		static int id;
	};

	template<class _Elem, class _OutIt>
	int time_put<_Elem, _OutIt>::id;

	template<class _OutIt>
	struct time_put<wchar_t, _OutIt>
	{
		static int id;
	};

	template<class _OutIt>
	int time_put<wchar_t, _OutIt>::id;
}

#if 0 // TODO!
namespace N026
{
	template<typename T>
	struct Outer
	{
		template<class _Elem, class _OutIt>
		struct time_put
		{
			static int id;
		};

		template<class _OutIt>
		struct time_put<T, _OutIt>
		{
			static int id;
		};
	};

	template<typename T>
	template<class _Elem, class _OutIt>
	int Outer<T>::time_put<_Elem, _OutIt>::id;

	template<typename T>
	template<class _OutIt>
	int Outer<T>::time_put<T, _OutIt>::id;
}
#endif

namespace N024
{
	template<typename T>
	struct Wrapper
	{
	};
	
	template<class _Ty, class _Ax=Wrapper<_Ty> >
	struct vector;

	template<class _Alloc>
	struct vector<bool, _Alloc>
	{
		typedef bool Type;
	};

	template<class _Ty, class _Ax>
	struct vector
	{
		typedef _Ty Type;
	};

	vector<char>::Type x;
}

namespace N020
{
	struct Wrapper
	{
		typedef int Int;
	};

	template<typename T> 
	struct Tmpl;

	template<typename T = Wrapper> 
	struct Tmpl;

	template<> 
	struct Tmpl<int>
	{
	};

	template<typename T>
	struct Tmpl
	{
		typedef T Type;
	};

	Tmpl<>::Type::Int i; // T should default to 'Wrapper'
}


namespace N022
{

	template<class _Elem, class _Traits >
	class basic_ostream;

	typedef basic_ostream<char> ostream;

	template<class _Elem, class _Traits = int> // default-argument specified after typedef
	class basic_ostream
	{
		typedef basic_ostream<_Elem, _Traits> _Myt;

		_Traits& f();
	};

	void f()
	{
		ostream o;
		o.f(); // return-type should be correctly resolved to 'int'
	}
}

namespace N021
{

	template<class _Elem, class _Traits=int >
	class basic_ostream;

	typedef basic_ostream<char, int> ostream; // typedef specified before definition

	template<class _Elem, class _Traits>
	class basic_ostream
	{
		typedef basic_ostream<_Elem, _Traits> _Myt;

		_Myt& f();
	};

	void f()
	{
		ostream o;
		o.f(); // return-type should be correctly resolved to 'basic_ostream'
	}
}



template<class>
struct get_info;
template<>
struct get_info<int>;


namespace N66
{
	template<bool C>
	struct Cond
	{
		typedef int type;
	};

	template<typename T>
	struct A
	{
		static const T value=0;
	};

	struct B : public A<int>
	{
	};

	typedef Cond<
		B::value < 8 // look up specialization of 'value' in A<int>
		>::type t1; 

	struct C
	{
		typedef B Type;
	};

	typedef Cond<
		C::Type::value < 8 // look up specialization of 'value' in A<int>
		>::type t1; 
}

namespace N019
{
	struct Wrapper
	{
		typedef int Int;
	};

	template<typename T>
	struct Tmpl
	{
		typedef T Type;
	};

	template<typename T = Wrapper> 
	struct Tmpl;

	Tmpl<>::Type::Int i; // T should default to 'Wrapper'
}

namespace N017
{
	struct Wrapper
	{
		typedef int Int;
	};

	template<typename T = Wrapper>
	struct Tmpl;

	template<typename T>
	struct Tmpl
	{
		typedef T Type;
	};

	Tmpl<>::Type::Int i; // T should default to 'Wrapper'
}

namespace N018
{
	struct Wrapper
	{
		typedef int Int;
	};

	template<typename T>
	struct Tmpl;

	template<typename T = Wrapper>
	struct Tmpl
	{
		typedef T Type;
	};

	Tmpl<>::Type::Int i; // T should default to 'Wrapper'
}

namespace N016
{
	template<typename X /*, typename Y = X*/> // the type of Y should be correctly resolved
	struct Tmpl
	{
	};
	template<>
	struct Tmpl<char>
	{
		static const char VALUE = 0;
	};

	const char Tmpl<char>::VALUE;

	template<>
	struct Tmpl<int>
	{
		static const int VALUE = 0;
	};
	const int Tmpl<int>::VALUE; // Tmpl<int>::VALUE should be distinct from Tmpl<char>::VALUE

	template<typename X>
	struct Tmpl<X*>
	{
		static const X* VALUE = 0;
	};
	template<typename X>
	const X* Tmpl<X*>::VALUE; // Tmpl<X*>::VALUE should be distinct from Tmpl<char>::VALUE
}


namespace N015
{
	template<typename T>
	struct Wrapper
	{
	};

	template<typename E, class A=Wrapper<E> > // the type of E should be correctly resolved
	class Tmpl
	{
		typedef A Type;
	};

	Tmpl<char>::Type x;
}



namespace N014
{
	template<typename T>
	struct Wrapper
	{
		typedef Wrapper<T> Type;
	};

	template<typename T>
	struct Tmpl
	{
		typedef typename T::Type Type;
		typedef typename Type::Type Type2;
		static Type2 f();
	};

	void f()
	{
		int i = Tmpl<Wrapper<int> >::f();
	}
}

namespace N013
{
	template<typename T>
	struct Tmpl
	{
	};

	template<class _InIt>
	Tmpl<_InIt> _Copy_opt(_InIt _First)
	{
		return _Copy_opt(_First); // should correctly determine that the type of '_Copy_opt' is dependent
	}
}

namespace N012
{
	template<class _InIt>
	_InIt _Copy_opt(_InIt _First)
	{
		return _Copy_opt(_First); // should correctly determine that the type of '_Copy_opt' is dependent
	}
}

namespace N013
{
	template<class _InIt>
	_InIt _Copy_opt(_InIt _First)
	{
		return dependent(_First); // should correctly determine that the type of '_First' is dependent
	}
}

namespace N011
{
	template<class _Iter>
	struct iterator_traits
	{
	};

	template<class _Ty>
	struct iterator_traits<_Ty *>
	{
	};

	template<class _Ty>
	struct iterator_traits<const _Ty *>
	{
	};

	template<> struct iterator_traits<bool>
	{
	};
}

namespace N001
{
	struct M
	{
		template<int i>
		void dependent(int j)
		{
		}
	};

	template<typename A>
	struct C
	{
		typedef M Type;
	};

	template<typename T1>
	struct O0
	{
		typename T1::Type f()
		{
			return typename T1::Type();
		}
	};

	void f(int)
	{
	}

	void f()
	{
		O0< C<int> > o;
		o.f().dependent<0>(0);
	}
}



namespace N010
{
	struct Wrapper
	{
		typedef int Type;
	};

	template<typename T>
	struct Tmpl
	{
		typedef typename T::Type Dependent;
	};

	Tmpl<Wrapper>::Dependent i;
}

template<class String, class Traits>
struct basic_path
{
	basic_path root_path() const;
	bool has_root_path() const;
	bool empty() const;

	struct inner;
};

template<class String, class Traits>
basic_path<String, Traits>basic_path<String, Traits>::root_path()const
{
	return basic_path<String, Traits>();
}

template<class String, class Traits>
inline bool basic_path<String, Traits>::has_root_path()const
{
	return !root_path().empty(); // should correctly determine the type of the left-hand side of the member-access expression
}

template<class String, class Traits>
struct basic_path<String, Traits>::inner
{
};

#if 0 // TODO: explicit specialization
template<class String, class Traits
struct basic_path<int, int>
{
	basic_path root_path() const;
};

template<class String, class Traits>
basic_path<String, Traits>basic_path<int, int>::root_path()const
{
	return basic_path<String, Traits>();
}
#endif







namespace std
{
	class locale;
	template<class _Facet>
	const _Facet& use_facet(const locale&);
	template<class _Elem>
	class ctype
	{
	};

	template<class _Elem>
	inline _Elem(tolower)(_Elem _Ch, const locale&_Loc)
	{
		return (use_facet<_Elem>(_Loc).tolower(_Ch)); // an explicit template argument list in a function call causes the expression to be dependent
	}
}










#if 0

template<typename A, typename B = A>
struct C
{
	typedef int I;
};

template<typename T1>
struct O0
{
	struct S1
	{
		static void f()
		{
			O0<T1>::S1 a;
		}
	};

	template<typename T2>
	struct L1
	{
		typedef C<T1> D_O0; // depends on O0
		typedef C<T2> D_L1; // depends on L1
		typedef C<T1, T2> D_O0_L1; // depends on L1 and O0
		static void f()
		{
			// C<int>::dependent();  // not dependent
			C<T1>::dependent(); // depends on O0, instantiated with L1
			C<T2>::dependent(); // depends on L1, instantiated with L1
			C<T1, T2>::dependent(); // depends on L1 and O0, instantiated with L1

			D_O0::dependent(); // depends on O0
			D_L1::dependent(); // depends on L1
			D_O0_L1::dependent(); // depends on L1 and O0
		}
	};

	static void f()
	{
		//C<int>::dependent(); // not dependent
		typename O0<T1>::S1::T a; // dependent
		O0<T1>::S1::dependent(); // S1 is member of O0, causes it to depend on O0
		L1<int>::dependent(); // L1 is member of O0, causes it to depend on O0

		L1<int>::D_O0::dependent(); // depends on O0
		L1<int>::D_L1::dependent(); // depends on L1
		L1<int>::D_O0_L1::dependent(); // depends on L1 and O0
	}
};

static int f()
{
	C<int>::I a; // not dependent
	O0<int>::L1<int>::D_O0 x;
	O0<int>::L1<int>::D_L1 y;
	O0<int>::L1<int>::D_O0_L1 z;
}


template<typename U>
struct Outer
{
	typedef U Type;

	template<typename T>
	struct First
	{
		typedef T Type; // 'Type' depends on First<>;
		typedef typename First<Type>::Type Type2; // First<Type> is a dependent template-id'; First<Type>::Type is a dependent qualified-id
		typedef U Type3; // 'Type3' depends on Outer<>;
		typedef typename First<Type3>::Type Type4; // First<Type> is a dependent template-id'; First<Type>::Type is a dependent qualified-id


		struct Inner : T
		{
		};
	};

	// can't instantiate First<int> because it is implicitly Outer<U>::First<int>
	typedef First<int>::Type FirstType; // instantiate First<int>; 'Type' depends on First<>, so look it up in the instantiation
	typedef First<int>::Type2 FirstType2; // instantiate First<int>; 'Type2' depends on First<>
	typedef First<int>::Type3 FirstType3; // instantiate First<int>; 'Type3' depends on Outer<>, not instantiated at this point
	typedef First<int>::Type4 FirstType4; // instantiate First<int>; 'Type4' depends on Outer<>, not instantiated at this point
	typedef First<int>::Inner InnerT; // instantiate First<int>;
	typedef typename First<U>::Type FirstType2; // instantiate First<U> - fails. Lookup of 'Type' deferred.

	template<typename T>
	struct Second
	{
		First<T>::Type m; // 'Type' depends on Second<> (via its qualified-id), but not First<> (because outside its definition)
		// can't instantiate First<int> because it is implicitly Outer<U>::First<int>
		First<int>::Type m; // 'Type' not dependent on First<> (because outside its definition)
		First<int>::Type3 m; // 'Type3' depends on Outer<>
		typedef First<T>::Type FirstType; // add 'FirstType' to type-instantiation set
		FirstType m2; // add 'm2' to type-instantiation set
	};
};


template<typename T>
struct Tmpl
{
	typedef T Type;
	typedef Tmpl<T> Self; // uses Tmpl::T

	template<typename U> // .. T also is an inherited implicit template param
	struct Inner
	{
		typedef Inner<U> Self; // uses Inner::U
		typedef Inner<T> Self2; // uses Tmpl::T
		typedef U Type; // uses Inner::U
		typedef T OuterType; // uses Tmpl::T

		void f()
		{
			Tmpl<int> m1; // not dependent
			Tmpl<T> m2; // not dependent (is current instantiation)
			Tmpl<U>::dependent(); // dependent (uses Inner::U)
		}
	};

	void f()
	{
		Self::Type m; // not dependent (is current instantiation)
		Type::dependent(); // dependent (uses Tmpl::T)
		Inner<int>::dependent(); // uses Tmpl::T
		Inner<int>::Self::dependent(); // dependent (uses Tmpl::T)
		Inner<int>::Self2::dependent(); // dependent (uses Tmpl::T)
	}
};

template<typename T>
struct C
{
	Tmpl<T> dependent; // uses C::T
	Tmpl<int>::Inner<T> dep2; // uses C::T (and Tmpl::T)
	Tmpl<int>::Inner<T> dep3; // uses C::T (and Tmpl::Inner::U)
};

#endif


void hidden(int)
{
}
namespace N
{
	void hidden(void*)
	{
		hidden(1);
	}
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




namespace N004
{
	template<typename T>
	struct Tmpl
	{
		static const T m = 0;
	};

	struct S : Tmpl<int>
	{
		union
		{
			int x[m]; // using 'Tmpl<int>::m' in an anonymous union.
			// when searching for the enclosing class of this declaration, the anonymous union should be ignored.
		};
	};
}


namespace N005
{
	template<bool C>
	struct if_c
	{
	};

	template<bool C>
	struct eval_if_c : if_c<C> // using a non-type template-parameter in a base-specifier
	{
	};
}

