
#if 0
#include <boost/type_traits/type_with_alignment.hpp>
#endif

namespace N60
{
	namespace mpl
	{
		template<typename T>
		struct result_
		{
			typedef T type;
		};

		template<bool Cond, typename True, typename False>
		struct if_c
		{
			typedef result_<True> type;
		};
	}

	namespace detail
	{
		union max_align
		{
			int m;
		};
		template<int i>
		struct is_aligned
		{
			enum { value = i };
		};

		template<int target>
		union lower_alignment
		{
			is_aligned<target> m;
		};

		template<int Align>
		class type_with_alignment_imp
		{
			typedef::N60::detail::lower_alignment<Align>t1;
			typedef typename mpl::if_c< ::N60::detail::is_aligned<4>::value, t1, ::N60::detail::max_align>::type align_t;
		};
	}
};

#if 1
namespace boost
{
	namespace mpl
	{
		template<typename T>
		struct result_
		{
			typedef T type;
		};

		template<bool Cond, typename True, typename False>
		struct if_c
		{
			typedef result_<True> type;
		};
	}

	template<typename T>
	struct alignment_of
	{
		enum { value = sizeof(T) };
	};
}

namespace boost
{
	namespace detail
	{
		class alignment_dummy;
		typedef void(*function_ptr)();
		typedef int(alignment_dummy::*member_ptr);
		typedef int(alignment_dummy::*member_function_ptr)();

		template<bool found, int target, class TestType>
		struct lower_alignment_helper
		{
			typedef char type;
			enum
			{
				value=true
			};
		};
		template<int target, class TestType>
		struct lower_alignment_helper<false, target, TestType>
		{
			enum
			{
				value=(alignment_of<TestType>::value==target)
			};
			typedef typename mpl::if_c<value, TestType, char>::type type;
		};
		template<typename T>
		struct has_one_T
		{
			T data;
		};
		template<int target>
		union lower_alignment
		{
			enum
			{
				found0=false
			};
			typename lower_alignment_helper<found0, target, char>::type t0;
			enum
			{
				found1=lower_alignment_helper<found0, target, char>::value
			};
			typename lower_alignment_helper<found1, target, short>::type t1;
			enum
			{
				found2=lower_alignment_helper<found1, target, short>::value
			};
			typename lower_alignment_helper<found2, target, int>::type t2;
			enum
			{
				found3=lower_alignment_helper<found2, target, int>::value
			};
			typename lower_alignment_helper<found3, target, long>::type t3;
			enum
			{
				found4=lower_alignment_helper<found3, target, long>::value
			};
			typename lower_alignment_helper<found4, target, float>::type t4;
			enum
			{
				found5=lower_alignment_helper<found4, target, float>::value
			};
			typename lower_alignment_helper<found5, target, double>::type t5;
			enum
			{
				found6=lower_alignment_helper<found5, target, double>::value
			};
			typename lower_alignment_helper<found6, target, long double>::type t6;
			enum
			{
				found7=lower_alignment_helper<found6, target, long double>::value
			};
			typename lower_alignment_helper<found7, target, void*>::type t7;
			enum
			{
				found8=lower_alignment_helper<found7, target, void*>::value
			};
			typename lower_alignment_helper<found8, target, function_ptr>::type t8;
			enum
			{
				found9=lower_alignment_helper<found8, target, function_ptr>::value
			};
			typename lower_alignment_helper<found9, target, member_ptr>::type t9;
			enum
			{
				found10=lower_alignment_helper<found9, target, member_ptr>::value
			};
			typename lower_alignment_helper<found10, target, member_function_ptr>::type t10;
			enum
			{
				found11=lower_alignment_helper<found10, target, member_function_ptr>::value
			};
			typename lower_alignment_helper<found11, target, boost::detail::has_one_T<char> >::type t11;
			enum
			{
				found12=lower_alignment_helper<found11, target, boost::detail::has_one_T<char> >::value
			};
			typename lower_alignment_helper<found12, target, boost::detail::has_one_T<short> >::type t12;
			enum
			{
				found13=lower_alignment_helper<found12, target, boost::detail::has_one_T<short> >::value
			};
			typename lower_alignment_helper<found13, target, boost::detail::has_one_T<int> >::type t13;
			enum
			{
				found14=lower_alignment_helper<found13, target, boost::detail::has_one_T<int> >::value
			};
			typename lower_alignment_helper<found14, target, boost::detail::has_one_T<long> >::type t14;
			enum
			{
				found15=lower_alignment_helper<found14, target, boost::detail::has_one_T<long> >::value
			};
			typename lower_alignment_helper<found15, target, boost::detail::has_one_T<float> >::type t15;
			enum
			{
				found16=lower_alignment_helper<found15, target, boost::detail::has_one_T<float> >::value
			};
			typename lower_alignment_helper<found16, target, boost::detail::has_one_T<double> >::type t16;
			enum
			{
				found17=lower_alignment_helper<found16, target, boost::detail::has_one_T<double> >::value
			};
			typename lower_alignment_helper<found17, target, boost::detail::has_one_T<long double> >::type t17;
			enum
			{
				found18=lower_alignment_helper<found17, target, boost::detail::has_one_T<long double> >::value
			};
			typename lower_alignment_helper<found18, target, boost::detail::has_one_T<void*> >::type t18;
			enum
			{
				found19=lower_alignment_helper<found18, target, boost::detail::has_one_T<void*> >::value
			};
			typename lower_alignment_helper<found19, target, boost::detail::has_one_T<function_ptr> >::type t19;
			enum
			{
				found20=lower_alignment_helper<found19, target, boost::detail::has_one_T<function_ptr> >::value
			};
			typename lower_alignment_helper<found20, target, boost::detail::has_one_T<member_ptr> >::type t20;
			enum
			{
				found21=lower_alignment_helper<found20, target, boost::detail::has_one_T<member_ptr> >::value
			};
			typename lower_alignment_helper<found21, target, boost::detail::has_one_T<member_function_ptr> >::type t21;
			enum
			{
				found22=lower_alignment_helper<found21, target, boost::detail::has_one_T<member_function_ptr> >::value
			};
		};
		union max_align
		{
			char t0;
			short t1;
			int t2;
			long t3;
			float t4;
			double t5;
			long double t6;
			void*t7;
			function_ptr t8;
			member_ptr t9;
			member_function_ptr t10;
			boost::detail::has_one_T<char>t11;
			boost::detail::has_one_T<short>t12;
			boost::detail::has_one_T<int>t13;
			boost::detail::has_one_T<long>t14;
			boost::detail::has_one_T<float>t15;
			boost::detail::has_one_T<double>t16;
			boost::detail::has_one_T<long double>t17;
			boost::detail::has_one_T<void*>t18;
			boost::detail::has_one_T<function_ptr>t19;
			boost::detail::has_one_T<member_ptr>t20;
			boost::detail::has_one_T<member_function_ptr>t21;
		};
		template<int TAlign, int Align>
		struct is_aligned
		{
			static const bool value=(TAlign>=Align)&(TAlign%Align==0);
		};
	}
	namespace detail
	{
		template<int Align>
		class type_with_alignment_imp
		{
			typedef::boost::detail::lower_alignment<Align>t1;
			typedef typename mpl::if_c< ::boost::detail::is_aligned<4>::value, t1, ::boost::detail::max_align>::type align_t;
			static const int found=alignment_of<align_t>::value;
		public:
			typedef align_t type;
		};
	}
	template<int Align>
	class type_with_alignment: public::boost::detail::type_with_alignment_imp<Align>
	{
	};
	namespace align
	{
		struct a8
		{
			char m[8];
			typedef a8 type;
		};
		struct a16
		{
			char m[16];
			typedef a16 type;
		};
		struct a32
		{
			char m[32];
			typedef a32 type;
		};
		struct a64
		{
			char m[64];
			typedef a64 type;
		};
		struct a128
		{
			char m[128];
			typedef a128 type;
		};
	}

	template<>
	class type_with_alignment<8>  
	{ 
		typedef mpl::if_c<
			::boost::alignment_of<detail::max_align>::value < 8,
			align::a8,
			detail::type_with_alignment_imp<8> >::type t1; 
	public: 
		typedef t1::type type;
	};
	template<>
	class type_with_alignment<8>  
	{ 
	   typedef mpl::if_c<
		  ::boost::alignment_of<detail::max_align>::value < 8,
		  align::a8,
		  detail::type_with_alignment_imp<8> >::type t1; 
	public: 
	   typedef t1::type type;
	};
	template<>
	class type_with_alignment<16> 
	{ 
	   typedef mpl::if_c<
		  ::boost::alignment_of<detail::max_align>::value < 16,
		  align::a16,
		  detail::type_with_alignment_imp<16> >::type t1; 
	public: 
	   typedef t1::type type;
	};
	template<>
	class type_with_alignment<32> 
	{ 
	   typedef mpl::if_c<
		  ::boost::alignment_of<detail::max_align>::value < 32,
		  align::a32,
		  detail::type_with_alignment_imp<32> >::type t1; 
	public: 
	   typedef t1::type type;
	};
	template<>
	class type_with_alignment<64>
	{
	   typedef mpl::if_c<
		  ::boost::alignment_of<detail::max_align>::value < 64,
		  align::a64,
		  detail::type_with_alignment_imp<64> >::type t1; 
	public: 
	   typedef t1::type type;
	};
	template<>
	class type_with_alignment<128>
	{
	   typedef mpl::if_c<
		  ::boost::alignment_of<detail::max_align>::value < 128,
		  align::a128,
		  detail::type_with_alignment_imp<128> >::type t1; 
	public: 
	   typedef t1::type type;
	};
}
#endif


namespace BLAH
{
	template<bool b>
	struct Tmpl
	{
		typedef int I;
	};

	template<typename T>
	struct S
	{
		struct P
		{
			template<typename U>
			struct Q
			{
				enum { value = sizeof(T) + sizeof(U) };
			};
		};

		struct R
		{
			typedef typename Tmpl<P::Q<int>::value>::I I;
		};
	};

	template<typename T>
	struct X
	{
		typedef Tmpl<S<int>::P::Q<int>::value>::I I; // no need for 'typename' because 'value' is not dependent
		// this is because 'value' is dependent on template-params that are not visible
		// i.e. 'value' is first qualified by 'S' which is not a member of a template
	};
}

namespace N57
{
	template<bool b>
	struct tmpl
	{
		typedef int type;
	};

	template<typename T>
	struct test
	{
		enum { value = sizeof(T) }; // dependent?
	};

	typedef tmpl<
		test<int>::value < 0
		>::type t1; 
}

namespace N56
{
	template<bool b>
	struct tmpl
	{
		typedef int type;
	};

	template<typename T>
	struct test
	{
		enum { value = 0 };
	};

	typedef tmpl<
		test<int>::value < 0
		>::type t1; 
}

namespace N55
{
	template<bool b>
	struct tmpl
	{
		typedef int type;
	};

	namespace test
	{
		enum { value = 0 };
	}

	typedef tmpl<
		test::value < 0
		>::type t1; 
}

namespace N84
{
	template<typename Base>
	struct S : Base
	{
		using Base::dependent;
	};
}


namespace N89
{
	typedef int size_t;

	namespace N
	{
		template<size_t N>
		struct size_t;
	}
}


// name-lookup for explicit argument-specification of overloaded function-template
namespace N91
{
	template<typename T>
	T f()
	{
	}
	int f()
	{
		  return f<int>();
	}
}

// name-lookup within (nested class) template member definition
template<typename X>
class C2
{
	template<typename T>
	class C3
	{
		typedef T I;
		static T m;
		static T f(I);
	};
};

template<typename X>
template<typename T>
T C2<X>::C3<T>::m = I();

template<typename X>
template<typename T>
T C2<X>::C3<T>::f(I)
{
	I i;
}

namespace N88
{
	template<typename T>
	struct flex_string
	{
		typedef int size_type;
		static const size_type npos;
	};

	template<typename X>
	const typename flex_string<X>::size_type
		flex_string<X>::npos = (typename flex_string<X>::size_type)(-1);
}


namespace N092
{
	template<typename T>
	struct Base
	{
	};

	template<typename T>
	class Derived
		: public Base< Derived<T> >
	{
		typedef int I;
	};

	typedef Derived<int>::I I;
}

namespace N39
{
	template<typename T>
	struct lambda;

	template<>
	struct lambda<int>
	{
		typedef int result_;
	};

	template<typename T>
	struct lambda
	{
		typedef T result_;
	};
}

namespace N33
{
	template <class Iterator>
	struct S
	{};
}
namespace N77
{
	template <class Iterator>
	struct S
		: N33::S<Iterator>
	{};
}



namespace N16
{
	template<typename T>
	void f()
	{
	}
	typedef int I;
	template<>
	void f<I>();
}

template<>
void N16::f<N16::I>()
{
}




#if 0
namespace N
{
	namespace N
	{
		template <class T>
		struct Tmpl
		{
		};
	}

	template<class T>
	struct Tmpl : N::N::Tmpl<T>
	{
	};
}
#endif

namespace N
{
	namespace N
	{
		template <class T>
		struct Tmpl
		{
			typedef T X;
		};
	}

	template<class T>
	struct Tmpl
	{
		typedef N::Tmpl<T> Y;
	};

	typedef Tmpl<int> I;
}

template<typename T>
struct Spec4
{
	typedef T I;
};

void f()
{
	typedef int I;
	typedef Spec4<I> Spec;

	Spec::I i = 0;
}

struct S
{
	template<bool>
	void operator()()
	{
	}
};
void f()
{
	enum { CONSTANT = 0 };
	S s;
	s.operator()<true>();
	s.operator()<CONSTANT < 0>(); // older versions of Comeau fail to compile this
}


template<bool>
struct Tmpl
{
};

enum { CONSTANT = 0 };

Tmpl< CONSTANT < 0 > t1;



typedef int I;

class C53
{
	C53(I);
	C53(I, bool b);
};

class C77
{
	C77(I = 0);
};

namespace N92
{
	struct B
	{
	};
	B b;
	struct S
	{
		template<int x>
		struct X : public B
		{
			X(B);
		};

		S(B a = X<0>(b));
	};
}


void f(int a)
{
	(a.x == a);
}

/* 3.4.4-1
An elaborated-type-specifier (7.1.6.3) may be used to refer to a previously declared class-name or enum-name
even though the name has been hidden by a non-type declaration (3.3.10).
*/
struct Hidden
{
	typedef int T;
};
namespace N3
{
	int Hidden;
	struct S
	{
		struct Hidden h;
	};
	Hidden::T i;
}

/* 3.4.4-1
An elaborated-type-specifier (7.1.6.3) may be used to refer to a previously declared class-name or enum-name
even though the name has been hidden by a non-type declaration (3.3.10).
*/
enum HiddenE
{
	VALUE
};
namespace N5
{
	namespace HiddenE
	{
	}
	enum HiddenE e;
}



template<typename A>
class Template7
{
	template<typename B>
	void f(B);
};

template<typename X>
template<typename Y>
void Template7::f(Y)
{
}


// testing name-lookup within declarator-suffix
class C6
{
	enum { SIZE = 1 };
	static int m[SIZE];
};

int C6::m[SIZE];

class C8
{
	typedef int I;
	void f(I);
};

void C8::f(I)
{
}

typedef int I;
struct S19
{
	I (*func)(I);
	I (func)(I);
};

namespace N15
{
	struct S
	{
		S(class D* a) : d((D*)(class C*)a)
		{
			C* c = (C*)0;
		}
		D* d;
	};
	D* d;
}

namespace N66
{
	class C* f(union U* (*)(struct S*));

	U* u;
	S* s;
	C* c;
}


namespace N75
{
	class 
	{
	public:
		int operator*() const
		{
		}
		int operator->() const
		{
		}
	};
}

namespace N24
{
	typedef int I;

	class C
	{
	public:
		C(I *a = 0) throw()
			: m(a), n(a == 0)
		{
		}
		int m, n;
	};
}


/* 3.4.1-8
A name used in the definition of a member function (9.3) of class X following the function’s declarator-id 33) shall be
declared in one of the following ways:
...
— if X is a nested class of class Y (9.7), shall be a member of Y, or shall be a member of a base class of Y (this lookup
applies in turn to Y ’s enclosing classes, starting with the innermost enclosing class),34) or
*/
struct Enclosing
{
	struct Nested
	{
		int x;
		Nested(int a = m) : x(m)
		{
			int l = m;
		}
	};
	static const int m = 0;
};


void testFor()
{
	for(int i = 0; i != 1; ++i)
	{
	}
#if 0
	for (_Iterator_base **_Pnext = (_Iterator_base **)&_Myfirstiter;
			*_Pnext != 0; *_Pnext = (*_Pnext)->_Mynextiter)
			(*_Pnext)->_Mycont = 0;
#endif
}

namespace N
{
	namespace N
	{
		struct S { static const int x = 0; };
		namespace N
		{
			struct S { static const int y = 0; };
		}
	}
	struct T : N::N::S
	{
		static const int a = y;
	};
}


// test conversion-operator
template<typename T>
struct C99
{
	template<class _Other>
	operator C99<_Other>()
	{
		return (C99<_Other>(*this));
	}
};

// test deferal of name-lookup for function-call identifier
template<typename T>
class DependentTmpl
{
};

template<typename T>
typename T::dependent f(typename T::dependent t)
{
	dependent1(T::dependent2());

	DependentTmpl<T>::dependent(t); // function-call
	dependent(dependent(t));

	typedef DependentTmpl<T> DependentType;
	dependent(DependentType(t)); // dependent-type looks like a dependent-name

	typename DependentTmpl<T>::dependent l; // member-typedef
	int i = (typename DependentTmpl<T>::dependent)1;

	dependent(DependentTmpl<T>());
	dependent(t);
}

template<typename T>
class DependentMemInit : public T, public DependentTmpl<T>
{
	typename T::M m;
	DependentMemInit() : T(0), DependentTmpl<T>(0), m(0)
	{
	}
};

// template-param-dependent value
template<bool VALUE>
struct ValTmpl
{
	static const int I = VALUE;
	typedef typename ValTmpl<I>::Type J;

	typedef char P[VALUE];
	typedef typename ValTmpl<sizeof(P)>::Type Q;

	typedef typename ValTmpl<sizeof(char[VALUE])>::Type Y;
	typedef typename ValTmpl<!VALUE>::Type X;
	typedef typename ValTmpl<sizeof(Y)>::Type Z;

	enum { E = VALUE };
	typedef typename ValTmpl<E>::Type F;
};


struct UnionTest
{
	union { int a; } x;
	union { int a; } y;
	union z { int a; };
	union { struct { int a; }; struct { int b; }; };
	union { int b; };
};

// test name-lookup for template-param-dependent nested-name-specifier
template<typename T>
struct Unspec
{
	void dependent()
	{
	}
	template<typename X>
	struct Dependent
	{
		void dependent()
		{
		}
	};
};

template<typename T>
struct TestDependent
{
	// template-dependent ]type-name
	typename T::C m1;
	typename Unspec<T>::template Dependent<T> m2;
	void f()
	{
		// template-dependent non-type-name
		T::dependent();
		T::C::dependent();
		Unspec<T>::dependent();
		Unspec<T>::template Dependent<T>::dependent();
	}
};


#if 0
void f()
{
	int x, y;
	((x) & y);
	((x) + y);
	((x) - y);
	((x) * y);
	((x) ~ y);
};
#endif

class C18
{
    union
    {
        mutable int buf_;
        int align_;
    };

	void f()
	{
		buf_ = 0;
	}
};



template<typename T, typename X>
struct Spec1;

template<typename T>
struct Spec1<T, int>
{
};

template<typename T, typename X>
struct Spec1
{
};



#if 1
template <class Tag,class T>
class error_info;

typedef error_info<struct throw_function_,char const *> throw_function;
typedef error_info<struct throw_file_,char const *> throw_file;
typedef error_info<struct throw_line_,int> throw_line;

template <>
class
	error_info<throw_function_,char const *>
{
public:
	typedef char const * value_type;
	value_type v_;
	explicit
		error_info( value_type v ):
	v_(v)
	{
	}
};
#endif

void f()
{
	new int();
}

template <typename T>
struct Tmpl3
	: Tmpl3< T >::template Dependent<T>
{
};


// pathological case for template-id ambiguity
#if 0
template<class T>
struct Tmpl
: Tmpl<
      Tmpl<T,T>
    , Tmpl<T>
    , Tmpl<
          Tmpl<T,T>
        , Tmpl<T>
        , Tmpl<
              Tmpl<T,T>
            , Tmpl<T>
            , Tmpl<
                  Tmpl<T,T>
                , Tmpl<T>
                , void
              >
          >
      >
  >
{
};
#endif



template<typename T>
struct TemplateParamTest
{
	void f(T t)
	{
	}
};

namespace N
{
	struct Blah
	{
	};
};

using N::Blah;

// argument-dependent-lookup
namespace N
{
	struct S {};
	void f(S);
}

void g(N::S s)
{
	//f(s); // TODO!
}


// members should be bound as early as possible
struct S7
{
	void m2(int)
	{
	}
};

template<typename T>
class C13 : public S7
{
	void f(T t)
	{
		m1(t);
		m2(t);
		dependent(t);
	}
	void m1(int)
	{
	}
};



int f(int i)
{
	int x(f(i));
}



template<typename T = int>
class DummyTmpl
{
};


template<template<typename X> class T = ::DummyTmpl>
class TestTemplateTemplateParam
{
};


namespace stdTEST
{

	template<class _Ty>
	struct _Allocator_base<const _Ty>
	{
	};

	template<class _Ty>
	class allocator: public _Allocator_base<_Ty>
	{
	};

	template<class _Ty,
	class _Alloc>
	class _String_val
	{
	};

	template<class _Elem,
	class _Ax>
	class basic_string
		: public _String_val<_Elem, _Ax>
	{
		typedef int size_type;
	};

	typedef basic_string<char, allocator<char> > string;

	template<class _Elem,
		class _InIt >
	class num_get
	{
		void f()
		{
			(string::size_type)1;
		}
	};
}

template<typename T>
class C10 : public T
{
	class DependentNested : public T
	{
	};

	void f()
	{
		dependent(this);
		dependent(DependentNested());
	}
};


// nested-name-specifier should ignore non-class non-namespace names
template<typename T>
struct C9
{
	template<typename X>
	C9()
	{
	}
	void f()
	{
		C9::f();
	}
};



template<typename T>
struct Fwd;

template<typename T>
void f()
{
	Fwd<T>::f();
	typedef Fwd<T> M;
	M::f();
}





// template-dependent non-type-name
template<typename T>
void f()
{
	T::f(); // TODO: T::C::f()
}


#if 1
/* 3.4.1-7
A name used in the definition of a class X outside of a member function body or nested class definition26
shall be declared in one of the following ways:
— before its use in class X or be a member of a base class of X (10.2)
*/
struct Base
{
	struct T
	{
		typedef int I;
	};
};

namespace N5
{
	/* 10-2
	The class-name in a base-specifier shall not be an incompletely defined class (Clause class); this class is
	called a direct base class for the class being defined. During the lookup for a base class name, non-type
	names are ignored (3.3.10)
	*/
	int Base;

	struct Derived : public Base, public Base::T
	{
		T t;
		I i;
	};

	template<typename T>
	struct Derived3 : public T
	{
	};
}
#endif



namespace std
{

	template<class _Ty> inline
		void swap(_Ty& _Left, _Ty& _Right)
	{	// exchange values stored at _Left and _Right
		_Ty _Tmp = _Left;
		_Left = _Right, _Right = _Tmp;
	}

	// TEMPLATE STRUCT pair
	template<class _Ty1,
	class _Ty2> struct pair
	{	// store a pair of values
		typedef pair<_Ty1, _Ty2> _Myt;
		typedef _Ty1 first_type;
		typedef _Ty2 second_type;

		void swap(_Myt& _Right)
		{	// exchange contents with _Right
			std::swap(first, _Right.first);
			std::swap((_Ty2)second, (_Ty2)_Right.second);
		}

		template<typename T>
		void f(_Myt t)
		{
			swap((int)t, (_Ty1)t);
		}

		_Ty1 first;	// the first stored value
		_Ty2 second;	// the second stored value
	};
}


template<typename First, typename Second>
int f(First x, Second y);

struct S4
{
};

int x = f(S4(), int());

template<typename Second, typename First>
int f(Second a, First b)
{
	return b;
}





struct Base3
{
	typedef int I;
};

template<typename T>
struct Spec : public Base3
{
	I m;
};

template<>
struct Spec<int> : public Base3
{
	I m;
	typedef Spec<float> other;
};


typedef struct _MIDL_SYNTAX_INFO MIDL_SYNTAX_INFO;

typedef struct _MIDL_SYNTAX_INFO
{
} MIDL_SYNTAX_INFO;



void f()
{
	{
		typedef int T;
		T i;
	}
	{
		typedef float T;
		T f;
	}
}





/* 3.4.3-1
During the lookup for a name preceding the :: scope resolution
operator, object, function, and enumerator names are ignored.
*/
namespace N2
{
	typedef int T;
}

namespace N4
{
	int N2;
	N2::T i;
}

namespace N
{
#if 1
	template<typename T>
	class undeclared;
#endif

	template<typename T>
	class C
	{
		template<typename X>
		void f()
		{
			const N::undeclared<X> x; // compiles in msvc, but should be error
			/* 14.6-10
			If a name does not depend on a template-parameter (as defined in 14.6.2), a declaration (or set of declarations)
			for that name shall be in scope at the point where the name appears in the template definition; the name is
			bound to the declaration (or declarations) found at that point and this binding is not affected by declarations
			that are visible at the point of instantiation.
			*/
		}
	};
}

namespace N
{
	class X;
}

namespace N
{
	X x;
}

struct S3
{
	typedef S3 Type;
	typedef int T;
};


template<typename T>
class Template3
{
	void f()
	{
		typename T::dependent x;
	}
};


template<typename T>
class Template2;

template<typename T>
class Template2
{
	static int x;
};

template<typename T>
int Template2<T>::x;


struct Incomplete;

template<typename T>
class Template
{
	void f()
	{
#if 0
		Incomplete::X(); // should give an error!
#endif
	};
};

int f(int a)
{
	int t(a); // function-declaration or variable declaration with init
	typedef int T;
	int (*x)(T)(f);
	// illegal
	// int (*t)(T)[1];
	// int t[1](a, b);
	// int t(a)(b);
	// int t(a)[1];
}

template<class T>
class Template1
{
	friend class Template1;
	typedef Template1<int> Type;
};

typedef struct { typedef int T; } S1;
typedef struct { typedef int T; } S2;

S1::T s1t;
S2::T s2t;


void f()
{
	struct S;
	S *p;
}


class C3
{
	void reset(C3*)
	{
	}
	void f()
	{
		C3 *_Ptr;
		reset(_Ptr);
		return (*this);
	}
};


template<typename T>
class Tmpl7
{
	template<typename Q>
	void func()
	{
		Q q;
	}
};

enum { ENUM1 };
enum { ENUM2 = ENUM1 };

template<typename T>
class C1
{
	static T t;
};

template<typename T>
T C1<T>::t;

template<>
class C1<int>
{
};


namespace std
{
	class _Lockit
	{
	public:
		explicit _Lockit();
		explicit _Lockit(int);
		~_Lockit();
		static void _Lockit_ctor(int);
		static void _Lockit_dtor(int);
	private:
		static void _Lockit_ctor(_Lockit*);
		static void _Lockit_ctor(_Lockit*,int);
		static void _Lockit_dtor(_Lockit*);
		_Lockit(const _Lockit&);
		_Lockit&operator=(const _Lockit&);
		int _Locktype;
	};

	class _Mutex
	{	// lock under program control

	};
}

typedef struct _div_t {
} div_t;

typedef struct _ldiv_t {
} ldiv_t;

inline ldiv_t  div()
{
}

div_t  __cdecl div();


struct threadlocaleinfostruct;
typedef struct threadlocaleinfostruct * pthreadlocinfo;

typedef enum IMAGE_AUX_SYMBOL_TYPE {
	IMAGE_AUX_SYMBOL_TYPE_TOKEN_DEF = 1,
} IMAGE_AUX_SYMBOL_TYPE;

typedef struct fd_set
{
} fd_set;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;


class type_info;

const type_info& type;


template<class T>
class C
{
	friend class C;
	C()
	{
	}
	~C()
	{
	}
};


class A
{
	void f();
};

class B
{
	friend class A;
};

void A::f()
{
}




typedef unsigned int size_t;

template<size_t _Size>
inline char*gets_s(char(&_Buffer)[_Size])
{
}

template<size_t _Size>
inline int _snprintf_s(char(&_Dest)[_Size], size_t __Size, const char*_Format, ...)
{
};


template<typename A>
struct X
{
	template<typename B>
	struct Y
	{
		template<typename C>
		struct Z
		{
			A a;
			B b;
			C c;
		};
	};
};

X<int>::Y<int>::Z<int> p;


template<typename A>
struct Y
{
};



