
namespace N009
{

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
		derived.::N009::N::Base2::m = 0; // N looked up in global scope
		Derived::TmplBase<int>::ms = 0;
		derived.TmplBase<int>::m = 0;
		Derived::TmplBase<float>::ms = 0;
		derived.TmplBase<float>::m = 0;
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

namespace N007
{
	template<class T>
	struct Tmpl
	{
		friend T operator*(T lhs, T rhs) // friend function is declared in global namespace
			// when deferring evaluation of 'T', must find enclosing template correctly
		{
			return lhs*=rhs;
		}
	};
}

namespace N008
{
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
}

namespace N006
{
	namespace N
	{
		template<class T>
		class Tmpl
		{
			typedef N::Tmpl<void(*)(T t)> F; // declaration of param 't' is removed by backtracking
		};
	}
}




namespace N002
{
	template<class T>
	struct Tmpl
	{
		typedef T Type;
		typedef Type Other; // typedef of typedef
	};

	void f()
	{
		Tmpl<int>::Type a; // lookup of 'Type' via Tmpl::T
		a = 0;
		Tmpl<int>::Other b; // lookup of 'Other' via Tmpl::T
		b = 0;
	}
}







//----------
// dependent member (via base class)
namespace N507
{
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
}

namespace N508
{
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
}


namespace N505
{
	// anonymous template type param
	template<class>
	class TmplFwd;

	template<template<class>class>
	class TmplFwd2;
}

namespace N506
{
	// non-type template param
	template<int _SizeFIXED>
	class TmplFwd;

	template<template<int _SizeFIXED>class T>
	class TmplFwd2;
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


namespace N71
{
	// dereference template-param
	template<typename T>
	void f(T t)
	{
		*t;
	}
}

namespace N49
{
	template<typename T>
	struct Inner
	{
	};

	template<typename T>
	struct Outer
	{
	};

	Outer<typename Inner<int> > a; // typename-specifier as template-argument
}

namespace N48
{
	void f(int a);

	void (*pf(int a)) (int hidden)
	{
		int i = a;
		return f;
	}
}

namespace N47
{
	template<bool b>
	struct BoolTmpl
	{
	};

	const bool RESULT = true;

	enum { X = sizeof(BoolTmpl<RESULT>) };
	enum { Z = sizeof(BoolTmpl<bool(RESULT)>) };
}

namespace N46
{
	namespace _bi
	{
		template<class F, class L> class bind_t
		{
		};

		class list0
		{
		};
	}

	template<class R>
	_bi::bind_t<R (*) (), _bi::list0>
		bind()
	{
	}
}

namespace N45
{
	struct S
	{
		S operator()() const
		{
			return S();
		}
	};

	void f()
	{
		S()(); // construct and call
		S()()();
		S s;
		s()(); // nested call
		s()()();
	}

}

namespace N117
{
	namespace N
	{
		template<typename T>
		struct Tmpl
		{
		};
	}

	using N::Tmpl;

	struct S : Tmpl<int>
	{
	};
}


namespace N116
{
	template<typename T>
	struct Tmpl
	{
		int x[sizeof(T)];
	};

	struct S : Tmpl<int>
	{
		using Tmpl<int>::x;
	};
}


namespace N115
{
	template<typename T>
	struct Tmpl
	{
		Tmpl(int)
		{
		}
	};

	int f()
	{
		return 0;
	}

	struct S
	{
		S(const S& arguments = S(Tmpl<int>(f())))
		{
		}
		S(const Tmpl<int>& arguments)
		{
		}
	};
}

namespace N113
{
	template<typename T>
	class Base
	{
	};

	class Derived : Base<float>
	{
		typedef int T;
		void f(T t)
		{
		}
	};
}

// 7.3.1.2 Namespace member definitions
// Paragraph 3
// Every name first declared in a namespace is a member of that namespace. If a friend declaration in a non-local class
// first declares a class or function (this implies that the name of the class or function is unqualified) the friend
// class or function is a member of the innermost enclosing namespace.

namespace N112
{
	// Assume f and g have not yet been defined.
	void h(int);
	template <class T> void f2(T);
	namespace A {
		class X {
			friend void f(X); // A::f(X) is a friend
			class Y {
				friend void g(); // A::g is a friend
				friend void h(int); // A::h is a friend
				// ::h not considered
				friend void f2<>(int); // ::f2<>(int) is a friend
			};
		};
		// A::f, A::g and A::h are not visible here
		X x;
		void g() { f(x); } // definition of A::g
		void f(X) { /* ... */} // definition of A::f
		void h(int) { /* ... */ } // definition of A::h
		// A::f, A::g and A::h are visible here and known to be friends
	}
}

namespace N115
{
	template<unsigned _SizeFIXED>
	inline char*gets_s(char(&_Buffer)[_SizeFIXED])
	{
		return gets_s(_Buffer, _SizeFIXED);
	}
}


namespace N42
{
	template<typename T>
	struct Blah
	{
		typedef typename T::template X<T::value> Type;
	};
}

namespace N111
{
	struct S* f(void)
	{
		return (struct S*)
			(int*) f();
	}
}

namespace N44
{
	namespace N
	{
		int i;
	}
	void f()
	{
		using N::i;

		int j = i; // using-declaration in function-body
	}
}

namespace N43
{
	struct S;

	typedef S T;

	struct S
	{
		static const int i = 0;
	};
	// TODO: causes crash?
	//int i = N::T::i; // typedef 'T' declared for incomplete type 'S'
}


namespace N41
{
	class C
	{
		// symbols may be deferred during attempt to parse 'void f(int i = j) {}'
		// first tried as 'void f(int i = j);', fails on reaching '{'
		void f(int i = j)
		{
		}

		static const int j = 0;
	};
}

namespace N40
{ 
	template<class T, void (T::*)()>
	struct S
	{
	};
}

namespace N39
{
	namespace N
	{
		template<class>
		struct S;
	}

	class C
	{
		template<class>
		friend struct N::S;
	};
}

namespace N38
{
	template<typename T>
	struct S
	{
		S(const char*, int);
	};

	const int i = 0;
	const char* p = 0;

	void f()
	{
		const S<int> node(p, i); // bug causes 'p' to be recognised when ',' is encountered - recognition should be deferred until ';'
	}
}

namespace N37
{
	template<typename T>
	struct S
	{
		S(int)
		{
		}
	};

	inline void f(int i)
	{
		(S<int (*)(int)>(i));
	}
}

namespace N36
{
	template<typename T = int>
	class DummyTmpl
	{
	};


	template<template<typename X> class T = N36::DummyTmpl>
	class TestTemplateTemplateParam
	{
	};
}

namespace N35
{
	template<typename T>
	struct Fwd;
	template<typename T>
	struct Tmpl
	{
		static const int i = Fwd<Tmpl>::value; // if type is the current instantation, it is dependent on the template parameters
	};
}

namespace N34
{
	template<typename T>
	struct Fwd
	{
		static const int value = 0;
	};
	template<typename T>
	struct Tmpl
	{
		typedef Fwd<Tmpl> Type;
	};
	static const int i = Tmpl<int>::Type::value; // 'Type' is not a dependent-type
}

#if 1
namespace N85
{
	template<typename T>
	struct A
	{
		typedef typename T::X X;
	};

	template<typename T>
	struct B
	{
		typedef int I;
	};

	struct P
	{
		typedef int X;
	};

	B<A<P>::X>::I i; // X is incorrectly determined to be a dependent type
}
#endif

namespace N84
{
	template<typename T>
	struct Base
	{
	};
	template<typename T>
	struct C
	{
		struct Derived : Base<int>
		{
			typedef typename T::X Y; // name-lookup finds T in Base and incorrectly determines that T is not a dependent-name
		};
	};
}

namespace N81
{
	template<typename T>
	struct Bool
	{
		enum { value = false };
	};

	struct S
	{
		template<bool b = Bool<int>::value> // default template-argument parse is incorrectly deferred - should be parsed immediately
		class M;
	};
}

namespace N82
{
	void f()
	{
		try
		{
		}
		catch(int e) // name-lookup for exception-declaration not supported
		{
			e = 0;
		}
		catch(float e) // scope for names declared in exception-declaration extends only to end of handler
		{
			e = 0;
		}
	}
}

namespace N83
{
	class C
	{
		void f()
		{
			// [special] Programs may explicitly refer to implicitly declared special member functions.
			operator=(*this); // explicit call of operator not supported
		}
	};
}

namespace N168
{
	// [expr.new] an implementation shall provide default definitions for the global allocation functions
	inline void operator delete(void * _P, int, const char *, int)
	{
		::operator delete(_P);
	}
}

namespace N80
{
	// Comeau/VC8 compiles this, but fails if g() is moved from N1::impl to N2::impl
	namespace N1
	{
		namespace impl
		{
			inline void g()
			{
			}
		}
	}
	namespace N2
	{
		namespace impl
		{
		}
	}
	namespace N1
	{
		void f()
		{
			using namespace N2;
			impl::g(); // g should be looked up in N1 before N2
		}
	}
}

namespace N78
{

	namespace N
	{
		int i;
	}

	namespace M
	{
		using namespace N;

		void f()
		{
			i = 0;
			M::i = 0; // i should be found in N
		}
	}

	namespace O
	{
		using namespace M;

		void f()
		{
			i = 0;
			O::i = 0; // i should be found in N via M
		}
	}
}

namespace N79
{
	namespace N1
	{
		void f()
		{
		}
	}

	namespace N2
	{
		void g()
		{
			using namespace N1;
			f(); // f should be found in N1
		}
	}
}


namespace N77
{
	template<typename T>
	struct Tmpl
	{
	};

	struct C
	{
		template<typename T>
		struct S;
	};
	template<typename T>
	struct C::S // out-of-line declaration of nested class
		: public Tmpl< C::S<T> > // template-param T should be visible here
	{
		void f(T) // template-param T should be visible here
		{
		}
	};
}

namespace N76
{
	struct C
	{
		struct R
		{
		};
		struct S;
	};

	 // out-of-line declaration of nested class
	struct C::S : public R // C:: is not required to find R
	{
	};
}

namespace N75
{
	struct S
	{
	};

	class C
	{
		struct S;
		struct T;
	};

	struct C::S // declaration of S does not conflict with declaration in enclosing namespace
	{
	};
	struct C::T : public S // C:: is not required to find S
	{
	};
}


#if 0 // TODO: explicit-specification of template arguments in member function call expression
namespace N74
{
	struct S
	{
		template<typename T>
		void f()
		{
		}
	};

	void f()
	{
		S s;
		s.f<int>();
	}
}
#endif

namespace N73
{
	template <class T, class U>
	void f(const U& u, void* a)
	{
		u.template dependent<T>(a); // interpret 'dependent' as a template if preceded by 'template'
	}
}

namespace 
{
	inline void f() // inline specifier
	{
	}
}

namespace N31
{
	namespace N
	{
		typedef int I;

		I f()
		{
		}
	}
	
	namespace M = N31::N; // namespace-alias as declaration

	M::I i; // N::I should be found via namespace-alias M

	void f()
	{
		namespace O = N31::N; // namespace-alias as statement
		O::I i = O::f();// N::I and N::f() should be found via namespace-alias O
	}
}

namespace N9
{
	namespace N
	{
		class C;
	}

	N::C f(N::C);

	namespace N
	{
		class C
		{
			friend C N9::f(C); // a friend declared with a qualified declarator-id declares 'f' in specified namespace
			// and C is looked up first in the context of the enclosing class
		};
	}
}


unsigned long long int i; // should be interpreted as the fundamental type 'unsigned-long-long-int'


namespace N8
{
	namespace N
	{
		class C
		{
		};
	}

	using N::C; // using-declaration with nested-name-specifier

	C c; // N::C should be found via using declaration
}


namespace N7
{
	enum E
	{
	};

	template<enum E L> // allow elaborated-type-specifier in non-type template-param
	inline void f(enum E l)
	{
	}
}


#if 1

namespace N692
{
	template<typename T>
	struct Tmpl
	{
		typedef T I;
	};

	template<>
	struct Tmpl<int>
	{
	};

	Tmpl<char>::I i; // I should be found in the primary-template
}

namespace N69
{
	template<typename T = int>
	struct Tmpl; // not defined!

	template<>
	struct Tmpl<int>
	{
		typedef int I;
	};

	Tmpl<>::I i; // I should be found in the specialisation for 'int'
}

namespace N68
{
	template<typename X>
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
}
#endif

namespace stdTEST
{

	template<class _Ty>
	struct _Allocator_base
	{
	};

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
			(string::size_type)1; // string should be determined not to be a dependent-type
		}
	};
}


#if 1
// default-template-parameter
namespace N67
{
	struct S
	{
		typedef int I;
	};

	template<typename T = S>
	struct Tmpl
	{
		typedef T type;
	};

	typedef Tmpl<>::type T;
	T::I i;

	struct D : public T
	{
		I m;
	};
}
#endif

// test deferral of name-lookup for function-call identifier
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

namespace N62
{
	template<typename T>
	struct Tmpl
	{
		template<typename U>
		struct Nested
		{
			typedef T Type;
		};
	};

	struct S
	{
		typedef int I;
	};

	typedef Tmpl<S>::Nested<int>::Type T;
	T::I i; // parse fails if 'T' is not resolved to 'S'
}


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


#if 1
namespace N668
{
	template<bool C>
	struct Cond
	{
		typedef int type;
	};

	template<typename T, T N>
	struct A
	{
		static const T value=0;
	};

	template<class U, U val>
	struct B : public A<U, val>
	{
	};

	typedef Cond<
		B<int, 8>::value < 8 // fails if 'B<int, 8>' is dependent
		>::type t1; 
}
#endif

// template-template params
template<template<typename X> class T>
struct TmplTmpl
{
	typedef T<int> M;
};



namespace N32
{
	template<long C_>
	struct Base
	{
	};

	// multi-part dependent nested-name-specifier
	template<typename T>
	struct Derived : Base<T::type::value>
	{
	};
}




namespace N84
{
	template<typename Base>
	struct S : Base
	{
		using Base::dependent;
	};
}

namespace N65
{
	template<typename T>
	struct Tmpl
	{
		static const int value = 0;
	};
	template<typename T>
	const int Tmpl<T>::value;

	template<bool b>
	struct Cond
	{
		typedef int type;
	};

	typedef Cond<
		Tmpl<int>::value < 8 // parse fails if 'value' is thought to be a template-name
		>::type t1;
}

namespace N61
{
	template<typename T>
	struct Tmpl
	{
		typedef T Type;
	};

	struct S
	{
		typedef int I;
	};

	typedef Tmpl<S>::Type T;
	T::I i; // parse fails if 'T' is not resolved to 'S'
	typedef T U;
	U::I j; // parse fails if 'U' is not resolved to 'S' via 'T'
}

#if 1
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
#endif

namespace N64
{

	typedef struct { typedef int T; } S1;
	typedef struct { typedef int T; } S2;

	S1::T s1t;
	S2::T s2t;

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
void Template7<X>::f(Y)
{
}

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
	I (func2)(I);
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




#if 1
template <class Tag,class T>
class error_info;

typedef error_info<struct throw_function_,char const *> throw_function;
typedef error_info<struct throw_file_,char const *> throw_file;
typedef error_info<struct throw_line_,int> throw_line;

template <>
class
	error_info<throw_function_,char const *> // throw_function_ was forward-declared within template-argument-clause
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
			const ::N::undeclared<X> x; // compiles in msvc, but should be error
			/* 14.6-10
			If a name does not depend on a template-parameter (as defined in 14.6.2), a declaration (or set of declarations)
			for that name shall be in scope at the point where the name appears in the template definition; the name is
			bound to the declaration (or declarations) found at that point and this binding is not affected by declarations
			that are visible at the point of instantiation.
			*/
		}
	};
}

// ----------------------------------------------------------------------------

namespace N
{
	class X;
}

namespace N
{
	X x; // lookup of 'X' finds declaration in namespace 'N'
}

// ----------------------------------------------------------------------------

struct S3
{
	typedef S3 Type;
	typedef int T;
};

// ----------------------------------------------------------------------------

template<typename T>
class Template3
{
	void f()
	{
		typename T::dependent x; // 'dependent' is determined to be dependent and name lookup is deferred.
	}
};

// ----------------------------------------------------------------------------


template<typename T>
class Template2;

template<typename T>
class Template2
{
	static int x;
};

template<typename T>
int Template2<T>::x; // lookup of 'x' finds declaration in definition of Template2

// ----------------------------------------------------------------------------

struct Incomplete;

template<typename T>
class Template
{
	void f()
	{
#if 0
		Incomplete::X(); // 'X' should be looked up and not found in 'Incomplete'
#endif
	};
};

// ----------------------------------------------------------------------------

int f(int a)
{
	int t(a); // should not be interpreted as function-declaration, not variable declaration with init: e.g. int t = a;
	typedef int T;
	int (*x)(T)(f);
	// illegal
	// int (*t)(T)[1];
	// int t[1](a, b);
	// int t(a)(b);
	// int t(a)[1];
}

// ----------------------------------------------------------------------------

template<class T>
class Template1
{
	friend class Template1;
	typedef Template1<int> Type;
};

// ----------------------------------------------------------------------------

void f()
{
	struct S; // declares 'S' in global namespace
	S *p; // lookup finds '::S'
}

// ----------------------------------------------------------------------------

class C3
{
	void reset(C3*) // lookup of 'C3' within in member declaration finds declaration of enclosing class
	{
	}
	void f()
	{
		C3 *_Ptr;
		reset(_Ptr);
		return (*this); // parenthesised return expression
	}
};

// ----------------------------------------------------------------------------

template<typename T>
class Tmpl7
{
	template<typename Q>
	void func()
	{
		Q q; // lookup finds 'Q' in enclosing member-template declaration
	}
};

// ----------------------------------------------------------------------------

enum { ENUM1 }; // declares '::ENUM1'
enum { ENUM2 = ENUM1 }; // declares '::ENUM2', lookup finds '::ENUM1'

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

typedef struct _div_t {
} div_t;

typedef struct _ldiv_t {
} ldiv_t;

inline ldiv_t  div()
{
}

div_t  __cdecl div();

// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------

class type_info;

const type_info& type;

// ----------------------------------------------------------------------------

template<class T>
class C
{
	friend class C; // does not declare anything
	C()  // lookup finds enclosing class 'C', declares the constructor 'C::C()'
	{
	}
	~C() // lookup finds enclosing class 'C', declares the destructor 'C::~C()'
	{
	}
};

// ----------------------------------------------------------------------------

class A
{
	void f(); // declares 'A::f()'
};

class B
{
	friend class A; // does not declare anything
};

void A::f() // lookup finds 'A::f', defines 'A::f()'
{
}



// ----------------------------------------------------------------------------

typedef unsigned int size_t;

template<size_t _Size>
inline char*gets_s(char(&_Buffer)[_Size])
{
}

template<size_t _Size>
inline int _snprintf_s(char(&_Dest)[_Size], size_t __Size, const char*_Format, ...)
{
};

// ----------------------------------------------------------------------------

template<typename A>
struct X
{
	template<typename B>
	struct Y
	{
		template<typename C>
		struct Z
		{
			A a; // lookup finds parameter 'A' of enclosing template
			B b; // lookup finds parameter 'B' of enclosing template
			C c; // lookup finds parameter 'C' of enclosing template
		};
	};
};

X<int>::Y<int>::Z<int> p; // lookup finds 'X', 'Y' and 'Z'

// ----------------------------------------------------------------------------
template<typename A>
struct Y
{
};



