

namespace N220
{
	struct true_;
	struct false_;

	template<typename T, template<typename P1> class F>
	struct S
	{
		typedef int Primary;
	};

	template<template<typename P1> class F>
	struct S<true_, F>
	{
		typedef int Special;
	};

	template<typename T>
	struct A
	{
	};

	typedef S<true_, A>::Special Special; // deduces 'A' as template-template-argument for S<true_, F>
	typedef S<false_, A>::Primary Primary;
}

namespace N219
{
	template<typename T>
	struct A
	{
	};

	template<template<class> class TT>
	struct C
	{
	};

	template<template<class> class TT>
	struct B : C<TT> // template-template-parameter used as a template-template-argument
	{
		typedef int Special;
	};

	typedef B<A>::Special Special;
}

namespace N218
{
	template<typename T>
	struct A
	{
	};

	template<template<class> class TT>
	struct C
	{
	};

	template<typename T>
	struct B
	{
		typedef int Primary;
	};

	template<template<class> class TT>
	struct B<TT<int> > : C<TT> // template-template-parameter used as a template-template-argument
	{
		typedef int Special;
	};

	typedef B<A<int> >::Special Special;
}

namespace N217
{
	template<typename T, typename U = int>
	struct A
	{
	};

	template<typename T>
	struct B
	{
		typedef int Primary;
	};

	template<template<typename P1>class F>
	struct B<F<int> >
	{
		typedef int Special;
	};

	typedef B<A<int> >::Primary Primary; // matches B<T>, not B<F<int> >
}


namespace N214
{
	template<typename T>
	struct A
	{
	};

	template<typename F>
	struct B
	{
		typedef int Primary;
	};

	template<template<typename P1>class F, typename T1>
	struct B<F<T1> >
	{
		typedef int Special;
	};

	typedef B<A<int> >::Special Special;
}


namespace N213
{
	struct Null;

	template<typename T0=Null, typename T1=Null>
	struct B;

	template<typename T0>
	struct B<T0, Null>
	{
		typedef int One;
	};

	template<typename T0, typename T1>
	struct B
	{
		typedef int Two;
	};

	typedef B<int>::One One;
	//typedef B<int, int>::Two Two;
}

#if 0 // static_assert
namespace N212
{
	template<typename T>
	struct Fail
	{
		typedef Fail<T::m> Type;
	};
	template<int N>
	struct Error
	{
		typedef typename Fail<N>::Type Type;
	};
	template<>
	struct Error<2>
	{
		typedef int Type;
	};

	template<int N>
	struct Test
	{
		typedef typename Error<N>::Type Type;
	};

	Test<2>::Type x;
};
#endif

namespace N211
{
	template<typename T>
	struct S
	{
		typedef int Primary;
	};

	template<int N>
	struct A
	{
	};

	template<typename T>
	struct B
	{
	};

	template<int N>
	struct S<A<N> >
	{
		typedef int Special;
	};

	typedef S<B<A<0> > >::Primary Primary; // instantiates S<T>
	typedef S<A<0> >::Special Special; // instantiates S<A<T> >
}

#if 0 // TODO
namespace N210
{
	template<typename T=int, int not_le_=0> // non-type default parameter
	struct S
	{
	};

	template<typename T>
	struct A
	{
		struct S<T> s; // should be ok to add unnecessary 'struct' qualifier
	};

	A<int> a;
}
#endif

namespace N209
{
	template<typename T=int, int not_le_=0> // non-type default parameter
	struct S
	{
	};

	template<typename T>
	struct A
	{
		S<T> s; // instantiates S<T, 0>
	};

	A<int> a;
}

namespace N208
{
	namespace aux
	{
		template<long N>
		struct vector_tag
		{
		};
		struct set_tag
		{
		};
	}

	template<typename Tag>
	struct clear_impl; // 4
	template<long N>
	struct clear_impl<aux::vector_tag<N> > // 3
	{
		template<typename Vector>
		struct apply
		{
		};
	};
	template<>
	struct clear_impl<aux::set_tag> // 2
	{
		template<typename Set>
		struct apply
		{
		};
	};
	template<typename Tag>
	struct clear_impl // 4
	{
		template<typename Sequence>
		struct apply;
	};
	template<>
	struct clear_impl<int> // 1
	{
	};

	// when lookup finds clear_impl, the list of specializations searched should be in the order shown above
	struct clear : clear_impl<aux::vector_tag<2> >::apply<int>
	{
	};
}


#if 0 // SFINAE: requires checking type of dependent non-type template argument expression against type of non-type template parameter
namespace N197
{
	template<typename T, void (T::*member)()>
	struct sfinae
	{
		typedef void Type;
	};

	template<class T, class U = void>
	struct S
	{
		typedef int Primary;
	};

	template<class T>
	struct S<T, typename sfinae<T, &T::member>::Type >
	{
		typedef int Special;
	};

	struct B
	{
		static void member();
	};

	struct C
	{
		void member();
	};

	S<B>::Primary primary;
	S<C>::Special special;
}
#endif

#if 1
namespace N176
{
	template<typename T>
	struct sfinae
	{
		typedef void type;
	};
	template<typename T, typename U=void>
	struct Test
	{
		typedef int False;
	};
	template<typename T>
	struct Test<T, typename sfinae<typename T::Type>::type>
	{
		typedef int True;
	};

	struct A
	{
		typedef int Type;
	};

	struct B
	{
	};

	Test<A>::True a;
	Test<B>::False b;
}
#endif

namespace N207
{
	template<int N>
	struct S
	{
		static const int value = N;
	};

	template<int N>
	struct A : S<N>
	{
	};

	int i = A<0>::value;
}

namespace N206
{
	template<int N>
	struct S
	{
		static const int value = N;
		typedef S<value + 1> next;
	};

	S<0> s;
}

namespace N205
{
	struct O
	{
		struct S
		{
			S(S s): m(s.m)
			{
			}
			int m;
		};
	};
}
namespace N204
{
	class C
	{
		C();
		int m;
	};

	C::C() : m(m) // 'm' must be usable in an unqualified expression within the initializer list
	{
	}
}

namespace N203
{

	struct Q
	{
		struct X;
		void N(X*);
	};

	struct C
	{
		typedef Q::X Y;
		static const int c = 0;
		friend void Q::N(Y* = c); // 'Y' and 'c' should be looked up in the scope of C rather than Q
	};

}

namespace N202
{
	// testing name-lookup within declarator-suffix
	class C6
	{
		enum { SIZE = 1 };
		static int m[SIZE];
	};

	int C6::m[SIZE];
}

namespace N201
{
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
	T C2<X>::C3<T>::f(I)
	{
		I i;
	}

	template<typename X>
	template<typename T>
	T C2<X>::C3<T>::m = I();
}

namespace N200
{
	class C
	{
		C* f();
	};

	C* C::f()
	{
		f()->f();
	}
}

namespace N199
{
	template<typename T>
	T f();

	void g()
	{
		f<int>(); // TODO: id-expression containing template-arguments
	}
}

namespace N198
{
	template<class T, class U = int >
	struct S
	{
	};

	template<class T>
	void f(S<T>)
	{
	}

	template<class T>
	void f(S<T, int>) // TODO: should fail: redefinition of void f(S<T, int>);
	{
	}
}

namespace N197
{
	template<typename T>
	struct A
	{
	};

	template<class T, class U = A<T> >
	struct S
	{
	};

	template<typename T>
	struct B
	{
		typedef typename S<A<T> > Type;
	};

	B<int>::Type s;
}

namespace N196
{
	struct C
	{
		void member();
	};

	template<void (C::*member)()>
	struct A
	{
	};

	template<class T>
	struct S
	{
		typedef A<&T::member> Type;
	};

	S<C>::Type t;
}

namespace N195
{
	template<class> // anonymous template type param $T0
	class S
	{
		typedef S Type; // resolves to S<$T0>
	};
}

namespace N194
{
	template<int> // anonymous template nontype param $i0
	class S
	{
		typedef S Type; // resolves to S<$i0>
	};
}

namespace N193
{
	template<template<class> class> // anonymous template template param $TT0
	class S
	{
		typedef S Type; // resolves to S<$TT0>
	};
}

namespace N054
{
	template<bool C_>
	struct bool_;

	template<bool C_>
	struct bool_
	{
		typedef bool_ type;
	};

	bool_<false>::type x;
}


namespace N190
{
	template<bool b>
	struct B
	{
		typedef int False;
	};
	template<>
	struct B<true>
	{
		typedef int True;
	};

	template<typename T>
	struct D : B<T::value> // dependent id-expression as non-type template-argument
	{
	};

	template<bool a>
	struct A
	{
		static const bool value = a;
	};

	typedef D<A<true> >::True True;
	typedef D<A<false> >::False False;
}

namespace N131
{
	template<typename T>
	struct S : T::Nested::X
	{
	};
}

namespace N129
{
	template<typename T>
	struct A
	{
		struct B // B is dependent: could be explicitly specialized
		{
			static T g();
		};

		struct C
		{
			typedef int Type;
#if 0
			A<T>::C::Type m; // vcpp complains that 'C' is dependent, GCC, ICC and Clang do not
			// 'C' names 'the template itself' because it is within the definition of 'C'
#endif
		};

		struct D : B // B should not be evaluated
		{
			void f()
			{
				C::f(); // lookup of 'f' should be deferred
				this->g(); // lookup of 'g' should be deferred
			}
		};
	};

	int i = A<int>::D::g();
}


namespace N192
{
	template<class T>
	struct S
	{
		typedef T Primary;
	};

	template<class T>
	struct B
	{
		typedef S<T> Type;
	};

	template<>
	struct S<char> // explicit-specialization declared after reference in B<T>
	{
		typedef char Char;
	};

	typedef B<char>::Type::Char Char;
	typedef B<int>::Type::Primary Primary;
}

namespace N191
{
	template<class T>
	struct Alloc
	{
		template<class U, class D = T>
		struct rebind
		{
			typedef Alloc<U> other;
		};
	};

	template<class T, class A>
	struct S
	{
		typedef typename A::template rebind<T>::other Type; // dependent template-name
	};

	typedef S<float, Alloc<int> >::Type Type;
	S<float, Alloc<int> > object;
}

namespace N059
{
	template<typename T, typename U>
	struct Tmpl
	{
		static void f(T); // declaration
		static void f(U); // redeclaration (illegal)
	};

	Tmpl<int, const int>::f(0);
}

namespace N186
{
	template<typename T>
	struct sfinae
	{
		typedef T Type;
	};
	template<typename T, typename U=void>
	struct S
	{
		typedef int False;
	};
	template<typename T>
	struct S<T, typename sfinae<typename T::Type>::Type>
	{
		typedef int True;
		// S<T, typename sfinae<typename T::Type>::Type>::True m; // TODO: names the current instantiation, not dependent
	};

	template<typename T>
	struct A
	{
		typedef T Type;
	};

	struct B
	{
	};

	typedef S<A<void> >::True True;
	typedef S<B>::False False;
}


namespace N190
{
	template<bool C, typename T1, typename T2>
	struct Cond
	{
		typedef T1 Left;
	};
	template<typename T1, typename T2>
	struct Cond<false, T1, T2>
	{
		typedef T2 Right;
	};

	Cond<false, int, int>::Right right;
	Cond<true, int, int>::Left left;
}

#if 0 // TODO: declaring member-class qualified by name of derived class
namespace N189
{
	struct Base
	{
		// dummy template parameter...
		template<class U, class _ = void> struct Inner {};

		// ... to allow in-class partial specialization
		template<class _> struct Inner<char, _> {};
	};

	struct Derived
		:
		Base
	{
		// cannot partially specialize Inner inside Derived...
		//template<class _>
		//struct Inner<int, _>: std::false_type {};
	};

	// ... but specializing Derived::Inner at namespace scope, also specializes it for Base::Inner
	template<class _> struct Derived::Inner<int, _> {};
}
#endif

namespace N188
{
	template<typename T, T t>
	struct B
	{
		typedef T Type;
		static const T value = t;
	};

	template<bool b>
	struct B<bool, b>
	{
		typedef bool Type;
		static const bool boolValue = b;
	};

	template<typename T, T t>
	struct S : B<T, t>
	{
	};

	const int s = S<int, 37>::value;
	//const bool b = S<bool, true>::boolValue;
}

namespace N187
{
	template<typename T, T t>
	struct B
	{
		typedef T Type;
		static const T value = t;
	};

	template<typename T, T t>
	struct S : N187::B<T, t>
	{
	};

	const int s = S<int, 37>::value;
}

namespace N185
{
	template<typename T>
	struct A
	{
	};

	template<typename T>
	struct B
	{
	};

	template<class T>
	struct S
	{
	};
	template<class T>
	struct S< A<T> >
	{
		typedef int First;
	};
	template<class T>
	struct S< B<T> >
	{
		typedef int Second;
	};

	typedef  S< A<int> >::First First;
	typedef  S< B<int> >::Second Second;
}

namespace N184
{
	template<class T, class U>
	struct S
	{
	};

	template<class T>
	struct S<T**, T**>
	{
	};

	template<class T>
	struct S<T**, const T**>
	{
	};
}

namespace N183
{
	template<class T, class U>
	struct S
	{
	};

	template<class T>
	struct S<T**, T**>
	{
	};

	typedef S<int**, const int**> Type;
}

namespace N182
{
	template<class T, class U>
	struct S
	{
	};

	template<class T>
	struct S<T, const T>
	{
	};

	typedef S<int, const int> Type;
}

namespace N181
{
	template<class T, class U>
	struct S
	{
	};

	template<class T>
	struct S<T, const T>
	{
	};

	typedef S<int, const int> Type;
}

namespace N180
{
	template<typename T>
	struct S
	{
		typedef int Primary;
	};

	template<typename T>
	struct S<const T> // f(const T)
	{
		typedef int Const;
	};

	typedef S<int>::Primary Primary; // matches 'S<T>'
	typedef S<const int>::Const Const; // matches 'S<const T>'
}

namespace N179
{
	template<typename T>
	struct S
	{
		typedef int Primary;
	};

	template<typename T>
	struct S<T*>
	{
		typedef int Ptr;
	};

	template<typename T>
	struct S<T**>
	{
		typedef int PtrPtr;
	};

	typedef S<int>::Primary Primary; // matches 'S<T>'
	typedef S<int*>::Ptr Ptr; // matches 'S<T*>'
	typedef S<int**>::PtrPtr PtrPtr; // matches 'S<T**>', not S<T*>
}

namespace N155
{
	template<typename T>
	struct Tmpl
	{
		Tmpl f(); // 'Tmpl' should resolve to 'Tmpl<T>'
	};

	template<typename T>
	struct Tmpl<const T*>
	{
		Tmpl f(); // 'Tmpl' should resolve to 'Tmpl<const T*>'
	};

	template<typename T>
	struct Tmpl<T*>
	{
		Tmpl f(); // 'Tmpl' should resolve to 'Tmpl<T*>'
	};
}


namespace N163
{
	struct large_size
	{
		char c[256];
	};
	large_size dispatch(struct exception*);
	struct small_size
	{
	};
	small_size dispatch(void*);
	template<class, int>
	struct S;
	template<class T>
	struct S<T, sizeof(large_size)>
	{
		typedef int type;
	};
	template<class T>
	struct S<T, sizeof(small_size)>
	{
		typedef int type;
	};
	template<class T>
	struct A
	{
		typedef typename S<T, sizeof(dispatch((T*)0))>::type type; // overload resolution is performed within sizeof
	};

	A<struct exception> a;
	A<struct Blah> b;
}


namespace N178
{
	template<class T, class U = void>
	class A
	{
		typedef T Type; // dependent type is 'T'
	};
	template<>
	class A<int>
	{
		typedef int Type;
	};
	template<class T, class U = void>
	class B
	{
		typedef typename A<T>::Type Type; // dependent type is 'A<T, void>::Type'
		typedef A<T> Type2; // dependent type is 'A<T, void>'
		typedef B<T> Type3; // dependent type is 'B<T, void>'
		typedef B Type4; // dependent type is 'B<T, void>'
	};
}

namespace N175
{
	template<class T>
	struct S
	{
		struct iterator
		{
			typedef typename T::Type Type2;
			Type2 f();
		};
	};

	struct A
	{
		typedef int Type;
	};

	void f()
	{
		S<A>::iterator s;
		s.f(); // instantiates S<A>::iterator
	}
}

#if 0
boost.mpl.aux.iter_fold_if_impl<
	boost.mpl.void_,
	void,
	boost.mpl.arg<1>,
	boost.mpl.protect<
	boost.mpl.aux.iter_fold_if_pred<
	boost.mpl.protect<
	boost.mpl.aux.find_if_pred<
	boost.multi_index.detail.has_tag<
	boost.wave.util.to
	>
	>,
	0
	>,
	boost.mpl.void_
	>,
	0
	>,
	boost.mpl.na,
	boost.mpl.always<
	boost.mpl.bool_<0>
	>
>

typedef typename if_<
typename forward_step4::not_last,
iter_fold_if_impl<
typename forward_step4::iterator, typename forward_step4::state, ForwardOp, ForwardPredicate, BackwardOp, BackwardPredicate
>,
iter_fold_if_null_step<typename forward_step4::iterator, typename forward_step4::state>
>::type backward_step4;

#endif

namespace N174
{
	template<typename T, int x = 0>
	struct S
	{
	};
	template<>
	struct S<int> // specializes S<int, 0>
	{
	};
}

namespace N173
{
	template<typename T, int T::*member>
	struct I
	{
	};

	template<class T>
	struct S
	{
		T m;
	};

	typedef S<int> Type;

	I<Type, &Type::m> i;
}

namespace N172
{
	template<int N>
	struct I
	{
	};

	template<class T, T N>
	struct S
	{
		static const T value = N; // 
	};

	I<S<int, 37>::value> i; // instantiates 'I<37>'
}

namespace N171
{
	template<int N>
	struct I
	{
	};

	I<true ? 3 : 5> i; // instantiates 'I<3>'
}

namespace N166
{
	template<int N>
	struct I
	{
	};

	template<int N>
	struct S
	{
		static const int value = N;
	};

	I<S<37>::value> i; // instantiates 'I<37>'
}


template<int>
struct S
{
};

void f(S<1 * 5 / 3 + 7 & 15> s)
{
	f(s);
}

namespace N170
{
	template<int>
	struct S
	{
	};

	namespace Inner
	{
		struct S; // declares a type distinct from the outer 'S'!

		void f(struct S*);
	}
}

namespace N169
{
	template<int>
	struct S
	{
	};

	namespace Inner
	{
		template<int>
		struct S; // declares a type distinct from the outer 'S'!
		template<int>
		struct S; // redeclares a type distinct from the outer 'S'!
	}
}

namespace N165
{
	template<int N>
	struct S
	{
	};

	// [temp.dep.constexpr] an identifier is value-dependent if it is ... a constant with integral or enumeration type and is initialized with an expression that is value-dependent
	template<typename T>
	void f()
	{
		const bool b = T::value;
		S<b> s;
	}
}


namespace N164
{
	template<typename T>
	struct is_array
	{
	};
	template<typename T, int N>
	struct is_array<T[N]> // T[N] is distinct from..
	{
	};
	template<typename T>
	struct is_array<T[]> // ..T[]
	{
	};
}


namespace N167
{
	int i(i); // point of declaration for name 'i' is immediately following the declarator 'i'
}

namespace N160
{
	template<int N>
	struct uintptr_candidates;
	template<>
	struct uintptr_candidates<-1>
	{
		typedef unsigned int type;
	};
	template<>
	struct uintptr_candidates<0>
	{
		typedef unsigned int type;
	};
	template<>
	struct uintptr_candidates<1>
	{
		typedef unsigned short type;
	};
	template<>
	struct uintptr_candidates<2>
	{
		typedef unsigned long type;
	};
	template<>
	struct uintptr_candidates<3>
	{
		typedef unsigned int type;
	};
	template<>
	struct uintptr_candidates<4>
	{
		typedef unsigned __int64 type;
	};
	struct uintptr_aux
	{
		static const int index=sizeof(void*)==sizeof(uintptr_candidates<0>::type)?0: sizeof(void*)==sizeof(uintptr_candidates<1>::type)?1: sizeof(void*)==sizeof(uintptr_candidates<2>::type)?2: sizeof(void*)==sizeof(uintptr_candidates<3>::type)?3: sizeof(void*)==sizeof(uintptr_candidates<4>::type)?4: -1;
		static const bool has_uintptr_type=(index>=0);
		typedef uintptr_candidates<index>::type type;
	};
}

namespace N162
{
	template<typename T>
	struct S
	{
		enum { RESULT = S<T>::RESULT }; // enumerator 'RESULT' is dependent
	};
}

namespace N161
{
	bool f(int x)
	{
		return (x < x > x); // not a template-id
	}
}

namespace N157
{
	template<class R>
	struct S
	{
	};
	template<>
	struct S<void(...)>
	{
	};
	template<>
	struct S<void()>
	{
	};

	S<void()> s;
}


namespace N076
{
	template<bool b>
	struct Tmpl
	{
		typedef int I;
	};

	template<typename T>
	struct S
	{
		template<typename U>
		struct Q
		{
			enum { value = sizeof(T) + sizeof(U) };
		};

		typedef typename Tmpl<Q<int>::value>::I I;
	};
}


namespace N159
{
	template<int N>
	struct S;
	template<>
	struct S<0>
	{
		typedef int Type;
	};
	template<>
	struct S<1>
	{
		typedef int Type;
	};
	typedef S<0>::Type Type;
}

namespace N158
{
	template<bool _Cond, class _Ty1, class _Ty2>
	class _If
	{
	public:
		typedef _Ty2 _Result;
	};
	template<class _Ty1, class _Ty2>
	class _If<true, _Ty1, _Ty2>
	{
	public:
		typedef _Ty1 _Result;
	};
}

namespace N156
{
	template<typename T>
	struct S
	{
	};

	template<>
	struct S<__int64>
	{
	};

	template<>
	struct S<int>
	{
	};

	S<__int64> a;
}

namespace N154
{
	template<typename T>
	T f()
	{
		return T();
	}

	struct S
	{
		int m;
	};

	int i = f<S>().m; // 'f<S>()' should have type 'S'
};

namespace N142
{
	template<typename T>
	struct A
	{
		enum Type { N = 0 }; // 'Type' is dependent: aka A<T>::Type
		static const Type m = (Type)0;
	};

	template<typename T>
	const typename A<T>::Type A<T>::m;

	struct B : A<int>
	{
		void f(int a = m)
		{
		}
	};
}


namespace N153
{
	template<typename T>
	struct S
	{
		union
		{
			int i;
			T m; // should be instantiated when S is instantiated
		};

	};

	S<int> s;
}

namespace N151
{
	template<class T, class A>
	struct vector
	{
	};

	template<class A>
	struct vector<bool, A>
	{
		typedef typename A::size_type size_type; // 'A' maps to the second template argument, not the first
	};
}

namespace N150
{
	template<class T, class U>
	struct B
	{
	};

	template<class T, class U>
	struct D : B<T, U>
	{
		typedef int Type;
	};

	template<class T>
	void f()
	{
		typedef D<char, T> Type; // instantiates 'B<char, T>' not 'B<T, U>'
		const Type t;
	}
}

namespace N149
{
	template<class T, class U>
	struct A
	{
	};

	template<>
	struct A<wchar_t, char>
	{
		void g();
		void f()
		{
			g(); // 'A<wchar_t, char>::g', not '<A<T, U>::g'
		}
	};
}

namespace N148
{
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
}

namespace N031
{
	template<typename T>
	struct Tmpl
	{
		Tmpl f(); // 'Tmpl' should resolve to 'Tmpl<T>'
	};

	template<typename T>
	struct Tmpl<T*>
	{
		Tmpl f(); // 'Tmpl' should resolve to 'Tmpl<T*>'
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


namespace N147
{
	template<class T>
	class A
	{
	};
	template<class T>
	class A<T*>
	{
		void g();
		void f()
		{
			g(); // 'A<T*>::g', not '<A<T>::g'
		}
	};
}

namespace N146
{
	template<class T>
	class A
	{
	};
	template<>
	class A<void>
	{
		void g();
		void f()
		{
			g(); // 'A<void>::g', not '<A<T>::g'
		}
	};
}

namespace N145
{
	template<class T>
	class A
	{
	};
	template<>
	class A<void>
	{
	};
	template<class T>
	void f(T a, A<void>*) // 'A<void>' is not dependent, 'void(T, A<void>)' is dependent
	{
	}
}


namespace N144
{
	template<typename T>
	struct S
	{
		const T i : 1,
j : 1,
	: 1; // all three declarations have a dependent type
		void f()
		{
			dependent(i);
			dependent(j);
		}
	};
}

namespace N143
{
	template<typename T>
	void f()
	{
		T a, b; // both declarations have a dependent type
		dependent(a);
		dependent(b);
	}
}

namespace N141
{
	template<typename T>
	struct A
	{
	};

	template<>
	struct A<int>
	{
	};

	template<typename T>
	struct B
	{
	};

	typedef B<A<int> > Type;

	struct C
	{
		Type f(); // 'Type' should be 'B<A<int>>', referring to the explicit specialization 'A<int>'
	};

	void f()
	{
		C c;
		c.f(); // return value should be 'B<A<int>>', referring to the explicit specialization 'A<int>'
	}

}


namespace N140
{
	template<typename T>
	struct A
	{
		struct B // B is dependent: could be explicitly specialized
		{
		};

		struct D : B // B should not be evaluated
		{
			void f()
			{
				this->g(); // lookup of 'g' should be deferred
			}
		};
	};
}

namespace N137
{
	typedef int I;
	typedef I J;
	typedef J I;
}

namespace N138
{
	typedef struct S { } S;

	typedef struct S S;
}

namespace N139
{
	typedef enum E { } E;

	typedef enum E E;
}


namespace N132
{
	template<class T>
	struct Tmpl
	{
		typedef int Type;
		// Tmpl::Type i; // TODO: 'Tmpl' is not dependent: it is an implicit template-id that refers to the template itself
	};
};


namespace N136
{
	template<class T>
	struct S;

	template<class T>
	struct S<T*>
	{
	};

	template<template<typename T> class F>
	struct A;

	typedef A<S> Type; // 'S' names a template-template-parameter 'S<T>'
}

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

namespace N134
{
	template<template<typename T> class F>
	struct A;

	template<typename T>
	struct B;

	typedef B< A<B> > Type; // 'B' should be interpreted as a template-name, not an implicit template-id
}

namespace N135
{
	template<template<typename T> class F>
	struct A;

	template<typename T>
	struct B
	{
		typedef B< A<N135::B> > Type; // 'B' should be interpreted as a template-name, not an implicit template-id
	};
}

namespace N133
{
	template<class T>
	struct Tmpl;

	template<class T>
	struct Tmpl
	{
	};
}

namespace N136
{
	struct B
	{
		int m;
	};

	template<typename T>
	class D : public B
	{
		void f()
		{
			m = 0;
		}
	};
}

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



namespace N130
{
	template<typename T>
	struct C
	{
		static const unsigned int value=8;
	};

	namespace N
	{
		struct S
		{
		};
	}

	template<typename T, typename U>
	struct S
	{
	};


	typedef S<C<int>::value < 8, N::S> Type; // attempted parse of 'value<8, N::S>' as a template-id should not produce an error
}

namespace N128
{
	struct S
	{
		struct Base
		{
			Base* g();
			g(int);
		};

		struct Inner : Base
		{
			Inner()
			{
				g()->g(); // should instantiate 'S::Base'
			}
		};
	};
}

namespace N126
{
	struct S
	{
		struct Base
		{
		};

		struct Inner : Base
		{
			Inner()
			{
				f()->f(); // should instantiate 'S::Inner'
			}
			Inner* f();
			f(int);
		};
	};
}

namespace N127
{
	template<typename T>
	struct S
	{
		static T f(T t)
		{
			return t;
		}
	};

	int i = S<int>::f(0); // type of 'S<int>::f' should be 'int(int)'
}

namespace N032
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
		struct Inner
		{
			Nested::Type h();
		};
	};
}



namespace N125
{
	template<typename T, typename U = typename T::Type>
	struct S;

	struct M
	{
		typedef int Type;
	};

	S<M>* s; // does not require instantiation of S, but requires instantiation of default argument
}

namespace N124
{
	struct S
	{
		typedef int Type;
	};

	template<typename T>
	typename T::Type f(T)
	{
		return 0;
	}

	S s;
	int i = f(s); // return type is 'int'
}

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
		enum { value = sizeof(T) };
	};

	template<typename T>
	struct X
	{
		typedef Tmpl<S<int>::value>::I I; // no need for 'typename' because 'value' is not dependent
		// this is because 'value' is dependent on template-params that are not visible
		// i.e. 'value' is first qualified by 'S' which is not a member of a template
	};
}

namespace N122
{
	template<typename T>
	void g(T t) // 'g' should be determined to be dependent, because it's type 'void(T)' is dependent
	{
		typedef T Type;
		Type tmp;
	}
}

namespace N117
{
	void f(int);
	void f(float);

	template<typename T>
	void g(T t)
	{
		f(*t); // f should be determined to be dependent, 'f' is looked up in this context but overloads are not resolved until instantiation of 'g'
	}
}

namespace N121
{
	struct S
	{
		int m;
		void f()
		{
			int m; // hides declaration of 'S::m'
			// 'm' should be looked up in the scope of 'S'
			this->m = 0;
			(*this).m = 0;
			this[0].m = 0;
			S& s = *this;
			s.m = 0;
			S*& p = this;
			p->m = 0;
		}
	};
}


namespace N120
{
	struct S;
	void f(struct S*);
}

namespace N119
{
	enum E {};
	enum E e; // elaborated type specifier referring to 'E', does not declare anything
}

namespace N118
{
	typedef struct S {} S; // declares a typedef which hides 'S'
	typedef S S; // redeclaration of typedef
}

namespace N116
{
	struct S
	{
		void f(float)
		{
			return f(); // calls f(int) with default-argument '0'. Parse of this statement should be deferred until after deferred-parse of default-arguments
		}
		void f(int i = 0)
		{
		}
	};
}

namespace N063
{
	template<typename T>
	struct Tmpl
	{
		typedef void(*Function)(T);
		void f(Function)
		{
			Function function;
			dependent(function); // 'function' should be determined to be dependent
		}
	};
}

namespace N115
{
	const int x;

	struct S
	{
	};

	S g()
	{
	}

	S s(S(g()), x); // Variation on most-vexing-parse. First parsed as a function definition, parse fails when encountering 'x'. Finally parsed successfully as a variable declaration.
}

#if 0
namespace N114
{
	// http://llvm.org/bugs/show_bug.cgi?id=11856#c2
	template<typename T> T end(T);

	template <typename T>
	void Foo()
	{
		T it1;
		if (it1->end < it1->end) // 'end' is dependent. 
		{
		}
	}
}
#endif

namespace N113
{
	void f();

	template<typename T>
	struct B
	{
	};

	template<typename T>
	struct D : B<T>
	{
		void g()
		{
			this->f(); // should not be looked up until instantiation of D::g
		}
	};

	//template<typename T>
	//void D::g();
}

namespace N112
{
	template<typename T>
	struct S
	{
		typedef typename T::Type Type;
		void f(Type)
		{
		}
	};

	struct A
	{
		typedef int Type;
	};

	void test()
	{
		S<A> s;
		s.f(0); // should link to N111.S.f(int)
	}
}

namespace N111
{
	template<typename T>
	void f(T)
	{
	}

	struct A
	{
	};

	template<typename T>
	struct S
	{
	};

	void test()
	{
		f(S<A>()); // should link to N111.f($T0)
	}
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

namespace N110
{
	template<bool C>
	struct if_c
	{
		typedef int type;
	};

	template<typename T>
	struct integral_c
	{
		static const T value=8;
	};
	template<typename T>
	T const integral_c<T>::value; // presence of this line causes parse fail?

	typedef if_c< integral_c<int>::value<8 >::type t1;
}

namespace N108
{
	template<typename T>
	struct B
	{
	};

	template<typename T>
	struct Tmpl : B<T>
	{
		static int m;
	};

	template<typename T>
	int Tmpl<T>::m = -1; // lookup of 'm' should not attempt to access B<T> because it is dependent
}

namespace N109
{
	template<typename T>
	struct B
	{
	};

	template<typename T>
	struct Tmpl
	{
		struct Inner : B<T>
		{
			void f()
			{
				this->dependent(); // 'this' should be determined to be dependent
			}
		};
	};
}

namespace N110
{
	template<typename T>
	struct B
	{
	};

	template<typename T>
	struct Tmpl : B<T>
	{
		void f()
		{
			this->dependent(); // 'this' should be determined to be dependent
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


namespace N107
{
	template<typename T>
	class DependentTmpl
	{
	};

	template<typename T>
	class DependentMemInit : public T, public DependentTmpl<T>
	{
		typename T::M m;
		DependentMemInit() : T(0), DependentTmpl<T>(0), m(0)
		{
		}
	};
}

namespace N106
{
	template<typename T>
	class B
	{
		typedef T Type;
	};

	template<typename T>
	struct D : public B<T>
	{
	};

	D<int>::Type i;
}



namespace N105
{
	void f();
	void f(int);
	void (*p)() = f; // TODO: [over.over]
}

namespace N104 // quick test of expression type reporting
{
	const int*const f();
	int i = f; // type of expression should be  'const int*const()'
}
namespace N103 // quick test of expression type reporting
{
	const int*const f();
	int i = f(); // type of expression should be  'const int*const'
}
namespace N102 // quick test of expression type reporting
{
	const int& f();
	int i = f(); // type of expression should be  'const int&'
}


namespace N101
{
	template<typename T>
	struct Base
	{
		typedef int I;
		static void f();
	};

	template<typename T>
	struct Tmpl : Base<T>
	{
#if 0
		I m; // should fail
		void g()
		{
			f(); // should fail
		}
#endif
	};

	Tmpl<int>::I i; // OK
	void g()
	{
		Tmpl<int>::f(); // OK
		Tmpl<int>().f(); //OK
		Tmpl<int>().Base<int>::f(); //OK
	}
}

namespace N100
{
	namespace A
	{
		void f(int);
	}

	namespace B
	{
		using A::f;
		void f(float);
		void g()
		{
			f(0); // overload resolution should choose A::f(int)
		}
	}
}
namespace N099
{
	template<typename T>
	struct C
	{
	};
	// friend struct C<int>; // friend
	template struct C<int>; // explicit instantiation
}

namespace N098
{
	template<typename T>
	struct Base
	{
	};

	template<typename T>
	struct Tmpl : Base<T> // type of base class is not resolved at this point
	{	
	};

	template<>
	struct Base<int>
	{
	};

	Tmpl<int> woot; // type of base-class should be resolved to specialization Base<int>
};


namespace N097
{
	void f(float); // 1a
	void f(int); // 2a: overloads 1a
	void f(float); // 1b: overloads 2a, redeclares 1a
	void f(int); // 2b: overloads 1b: redeclares 2a

	void g()
	{
		f(0); // sees 1b, 2b
	}
};


namespace N053
{
	namespace exception_detail
	{
		template<class>
		struct get_info;
		template<>
		struct get_info<int>;
	}
	class exception
	{
		template<class>
		friend struct exception_detail::get_info;
	};
}

namespace N095
{
	struct S
	{
		void f(); // declaration
	};

	inline void S::f() // definition of previous declaration
	{
	}

	void f()
	{
		S s;
		s.f();
	}
}

namespace N094
{
	void f(float, ...);
	void f(float, float);
	void f(int, float)
	{
		f(0.f, 0.f); // overload resolution should choose f(float, float)
	}
}

namespace N093
{
	void f(float, float);
	void f(float, int = 37);
	void f(int)
	{
		f(0.f); // overload resolution should choose f(float, int)
	}
}

namespace N092
{
	struct S
	{
		// parsing type of default-argument should be deferred 
		void f1(int i = 0);
		void f2(int i = 0) {} // test backtracking after shared-prefix declarator
		void f3(int i = 0), f4(int j = 0);
	};
}

namespace N091
{

	namespace N
	{
		template<class T>
		struct Tmpl
		{
		};
	}
	inline void f()
	{
		// parsing shared-prefix template-id: tries nested-name-specifier-suffix first, backtracks and tries type-name (should re-use cached parse-tree of template-id?)
		// ensure that parsing a declarator can be undone safely during backtrack.
		N::Tmpl<void(int a, int b)> t;
	}
}

namespace N089
{
	void f(bool, bool = false);
	void f(int = 0);

	void f()
	{
		f(0); // should call f(int)
	}
}

namespace N090
{
	struct S
	{
		template<int i, int j>
		struct M
		{
			M(int)
			{
			}
			operator int()
			{
				return 0;
			}
		};
		void f(bool b = M<0, 0>(0)) // Clang fails to parse this
		{
		}
	};
}

namespace N088
{
	void f(const wchar_t*);
	void f()
	{
		f(L""); /// overload resolution should pick 'f(const wchar_t*)'
	}
}

namespace N087
{
	template<typename T>
	struct Tmpl
	{
		template<class U>
		void f(U)
		{
		}
	};

	Tmpl<int> t;
	void f()
	{
		t.f(0); // the type of 'f(0)' should be determined to be 'void(int)'
	}
}

namespace N084
{
	template<typename T>
	struct Tmpl
	{
		typedef void(T::*Function)();
		void f(Function)
		{
			Function function;
			dependent(function); // 'function' should be determined to be dependent
		}
	};
}

namespace N085
{
	template<typename T>
	struct Tmpl
	{
		typedef void(T::C::*Function)(); // lookup of 'C' should be deferred
	};
}

namespace N086
{
	template<typename T>
	struct Tmpl
	{
		typedef void(T::template C<T>::*Function)(); // lookup of 'C' should be deferred
	};
}

#if 0 // TODO

namespace N070
{
	template<typename T>
	struct Tmpl
	{
		Tmpl<T>* a; // TODO: 14.6 [temp.res] 'name of template itself' Tmpl<T> is NOT dependent!
	};
}

namespace N066
{
	template<typename T>
	struct Tmpl
	{
		struct S
		{
		};
		void f()
		{
			Tmpl<T>::S::dependent(); // 'S' may be explicitly specialized later
		}
	};

	template<>
	struct Tmpl<int>::S // explicit specialization of Tmpl<T>::S for T=int
	{
		static void dependent()
		{
		}
	};

	Tmpl<int>::S s;
}


namespace N069
{
	template<typename A>
	struct C
	{
	};

	template<typename T>
	struct Tmpl
	{
		template<typename U>
		struct Inner
		{
			typedef C<U> Type;
		};

		static void f()
		{
			Inner<int>::Type::dependent(); // Inner<int> is dependent because Tmpl<T>::Inner<int> may be specialized later
		}
	};

	template<>
	template<typename U>
	struct Tmpl<int>::Inner // explicit specialization of Tmpl<T>::Inner for T=int
	{
		static void dependent()
		{
		}
	};

	Tmpl<int>::Inner<int> s;
}

#endif

namespace N083
{
	template<class T>
	struct Tmpl
	{
		void f(T& self)
		{
			self.dependent();
		}
	};
}

namespace N082
{
	template<typename T>
	struct C
	{
	};

	struct S
	{
		template<typename T>
		struct Inner
		{
		};
	};

	template<typename T>
	struct Tmpl
	{
		typedef C<T> Type;

		void f()
		{
			this->S::template Inner<Type>::dependent(); // 'template Inner<Type>' should be determined to be dependent
		}
	};
}

namespace N081
{
	template<class T>
	struct Tmpl;

	template<class T>
	struct Base
	{
		typedef Tmpl<T> Derived;
		static Derived* derived()
		{
			return 0;
		}
	};

	template<class T>
	struct Tmpl : Base<T>
	{
		typedef Tmpl<T> Self;

		void f(Self& self)
		{
			self.derived()->dependent(); // name-lookup of 'derived' should be deferred!
		}
	};
}


namespace N080
{
	template<class T>
	struct C
	{
		typedef int Type;
	};
	template<class T>
	struct Tmpl
	{
		typedef typename ::N080::C<T>::Type X;
	};

}

namespace N078
{
	template<typename T>
	class Tmpl
	{
		class DependentNested : public T
		{
		};

		typedef DependentNested D;
	};
}

namespace N077
{
	template<bool VALUE>
	struct ValTmpl
	{
		typedef char P[VALUE];
		typedef typename ValTmpl<sizeof(P)>::Type Q;
	};
}

namespace N075
{
	template<bool b>
	struct Tmpl
	{
		typedef int I;
	};

	template<typename T>
	struct S
	{
		enum { value = sizeof(T) };

		typedef typename Tmpl<value>::I I; // 'I' is dependent
	};
}

namespace N074
{
	template<typename T>
	class DependentTmpl
	{
	};

	template<typename T>
	void f()
	{
		int i = (typename DependentTmpl<T>::dependent)1; // 'dependent' is, unsurprisingly, dependent
	}
}

namespace N073
{
	template<class T>
	inline T f(T t)
	{
		return (t.f(t)); // the return-type of f(t) should be determined to be dependent
	}
}

namespace N072
{
	template<typename T1>
	struct O0
	{
		typename T1::Type f() // should determine that T1::Type is dependent
		{
		}
	};
}

namespace N071
{
	template<typename T>
	struct Tmpl
	{
		typedef Tmpl Type; // implicit template-id
	};
	void f()
	{
		Tmpl<int>::Type t;
	}
}

namespace N068
{
	template<typename T>
	struct Tmpl
	{
		typedef Tmpl<T> Type;
		void f()
		{
			Type t; // not dependent
		}
	};
	void f()
	{
		Tmpl<int>::Type t;
	}
}

namespace N067 // simple test for determining if name is dependent
{
	template<typename T>
	struct Tmpl
	{
		typedef T Type;
		void f()
		{
			Type::dependent();
		}
	};
	void f()
	{
		Tmpl<int>::Type t;
	}
}


#if 0

namespace N064
{
	template<typename A, typename B = A>
	struct C
	{
		typedef int I;
	};

	template<typename T1>
	struct O0
	{
		template<typename T2>
		struct L1
		{
			typedef C<T1> D_O0; // depends on O0
			typedef C<T2> D_L1; // depends on L1
			typedef C<T1, T2> D_O0_L1; // depends on L1 and O0
			static void f()
			{
				C<int>::I i1;  // not dependent
				C<T1>::dependent(); // depends on O0
				C<T2>::dependent(); // depends on L1
				C<T1, T2>::dependent(); // depends on L1 and O0

				D_O0::dependent(); // depends on O0
				D_L1::dependent(); // depends on L1
				D_O0_L1::dependent(); // depends on L1 and O0
			}
		};

		static void f()
		{
			C<int>::I i1; // not dependent

			L1<int>::D_O0::dependent(); // depends on O0
			L1<int>::D_L1::dependent(); // depends on L1 (which may be explicitly specialised?)
			L1<int>::D_O0_L1::dependent(); // depends on O0
		}
	};

	static int f()
	{
		C<int>::I a; // not dependent
		O0<int>::L1<int>::D_O0 x;
		O0<int>::L1<int>::D_L1 y;
		O0<int>::L1<int>::D_O0_L1 z;
	}
}
#endif



#if 0 // this is all incorrect!
namespace N065
{

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
		typedef typename First<int>::Type FirstType; // instantiate First<int>; 'Type' depends on First<>, so look it up in the instantiation
		typedef typename First<int>::Type2 FirstType2; // instantiate First<int>; 'Type2' depends on First<>
		typedef typename First<int>::Type3 FirstType3; // instantiate First<int>; 'Type3' depends on Outer<>, not instantiated at this point
		typedef typename First<int>::Type4 FirstType4; // instantiate First<int>; 'Type4' depends on Outer<>, not instantiated at this point
		typedef typename First<int>::Inner InnerT; // instantiate First<int>;
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
}

#endif

namespace N062
{
	void f(int()); // declaration
	void f(int(*)()); // redeclaration
}

namespace N061
{
	void f(int[]); // declaration
	void f(int*); // redeclaration
}

namespace N056
{
	void f(const int); // declaration
	void f(int); // redeclaration
}

namespace N058
{
	void f(const int*); // declaration
	void f(int*); // declaration
}

namespace N060
{
	template<typename T>
	void f(T); // declaration
	template<typename U>
	void f(U); // redeclaration
}


namespace N057
{
	typedef const int ConstInt;
	ConstInt i;
	void f()
	{
		i + 1;
	}

	typedef ConstInt* PtrToConstInt;
	typedef int* PtrToInt;
	typedef const PtrToInt ConstPtrToInt;
	typedef int const* ConstPtrToInt2;

	typedef int FuncRetInt();
	typedef const FuncRetInt ConstFuncRetInt; // unspecified behaviour!

	struct S
	{
		typedef int MemFuncRetInt();
		typedef const MemFuncRetInt ConstMemFuncRetInt; // unspecified behaviour!
	};
}

namespace N055
{
	void f()
	{
		try
		{
		}
		catch(int) // abstract declarator
		{
		}
	}
}

namespace N048
{
	template<typename T>
	struct Tmpl
	{
		void f(Tmpl); // declaration
		void f(int); // declaration
	};
}


namespace N052
{
	template<typename T>
	struct Tmpl
	{
	};

	template class Tmpl<char>; // explicit instantiation
}

namespace N051
{
	struct S
	{
		template<bool _Secure_validation>
		class _Const_iterator; // declaration
		friend class _Const_iterator<true>; // should not declare anything!

		template<bool _Secure_validation>
		class _Const_iterator // definition
		{
		};
		typedef _Const_iterator<true> const_iterator; // should refer to definition, not friend-declaration
	};
}

namespace N042
{
	template<typename T>
	struct Tmpl;

	template<typename T>
	struct Tmpl<T*>; // forward-declaration of partial-specialization

	template<typename T>
	struct Tmpl<T*> // definition of partial-specialization
	{
	};
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

namespace N050
{
	template<typename T>
	struct Tmpl
	{
		void f(Tmpl const&r)
		{
		}
		template<class Y>
		void f(Tmpl<Y>const&r)
		{
		}
	};
}

namespace N047
{
	template<typename T = int>
	struct Tmpl
	{
		void f(Tmpl<>); // declaration
	};

	template<typename T>
	void Tmpl<T>::f(Tmpl<>) // definition
	{
	}
}





namespace N046
{
	template<typename T>
	struct Tmpl
	{
		typedef T* func_t();
		func_t f; // member function declaration using typedef
	};

	template<typename T>
	T* Tmpl<T>::f() // definition
	{
	}
}

namespace N045
{
	struct S
	{
		typedef void* func_t();
		func_t f; // member function declaration using typedef
	};

	void* S::f() // definition
	{
	}
}

namespace N043
{
	struct S
	{
		template<bool b>
		struct Tmpl;

		friend struct Tmpl<true>; // friend declaration of explicit-specialization

		template<bool b>
		struct Tmpl // definition of explicit-specialization
		{
		};
	};
}


namespace N044
{
	template<typename T = int>
	struct Tmpl;

	template<>
	struct Tmpl<>; // forward-declaration of explicit-specialization

	template<>
	struct Tmpl<> // definition of explicit-specialization
	{
	};
}

namespace N041
{
	template<typename T = int>
	struct Template
	{
	};
	template<>
	struct Template<> // <> is equivalent to <int>
	{
	};
}

namespace N040
{
	void f(int);
	void f(int); // redeclaration of function

	template<typename T>
	void f(T);
	template<typename U>
	void f(U); // redeclaration of function-template
}


#if _MSC_VER > 1400
namespace N039
{
	// LLVM/Comeau allows this, but I can't see where the standard allows it
	// Locally declared names (temp.local) are not dependent
	template<typename T>
	struct Template
	{
		typedef int I;
		typedef Template::I Type; // 'Template' is NOT dependent
		typedef Template<T>::I Type2; // 'Template<T>' is NOT dependent
		Template<T>* m;
		void f()
		{
			m->f(); // 'm' is NOT dependent
		}
	};
}
#endif

namespace N038 // name first found in friend declaration, exists in innermost enclosing namespace
{
	namespace
	{
		int T;
	}
	struct S
	{
		friend class T;
		friend void* f();
	};
	int t = T; // should find the definition of T in the anonymous namespace!
}

namespace N034
{
	struct S
	{
		void m(int) const
		{
		}
	};

	void (S::*const p)(int) const = &S::m; // const-ptr to const-member
}

// ------
namespace N035
{
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
	char** f(char*);
	const char** f(const char*);
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
}

namespace N036 // test parsing of type-id
{
	struct S;

	typedef int* A[1]; // array of pointer to

	void f(A(*x)[1]) // pointer to array of
	{
	}
	void g(int*(*x)[1][1])// pointer to array of array of pointer to
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
	void g(int*[]) // array of pointer to
	{
	}
	void f(int(*)()) // pointer to function returning
	{
	}
	void g(int*(*)()) // pointer to function returning pointer to
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

	typedef int Arr2[2];
	typedef Arr2 Arr1[];
	extern Arr1 arr0;  // array(?) of array(2) of int
	extern int arr1[1][2]; // array(1) of array(2) of int
	extern int arr2[][2]; // array(?) of array(2) of int
	extern int* arr3[2]; // array(2) of pointer to int
	extern int (*arr4)[2]; // pointer to array(2) of int

}



namespace N023
{
	template<typename T>
	struct Wrapper
	{
	};

	template<typename E, class A=Wrapper<E> >
	struct Tmpl;

	template<typename E, class A> // the default-argument for A should be merged from the forward-declaration
	struct Tmpl
	{
		typedef A Type;
	};

	Tmpl<char>::Type x;
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

#if 0 // TODO: declaration with multiple sets of template parameters
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

namespace N022
{

	template<class _Elem, class _Traits >
	struct basic_ostream;

	typedef basic_ostream<char, int> ostream;

	template<class _Elem, class _Traits = int> // default-argument specified after typedef
	struct basic_ostream
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
	struct basic_ostream;

	typedef basic_ostream<char, int> ostream; // typedef specified before definition

	template<class _Elem, class _Traits>
	struct basic_ostream
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

#if(_MSC_VER > 1400) // 8.0
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

	template<typename T = Wrapper> // bug in MSVC8, default-template-argument is not found correctly
	struct Tmpl;

	Tmpl<>::Type::Int i; // T should default to 'Wrapper'
}
#endif

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
		static const char VALUE;
	};

	const char Tmpl<char>::VALUE = 0;

	template<>
	struct Tmpl<int>
	{
		static const int VALUE;
	};
	const int Tmpl<int>::VALUE = 0; // Tmpl<int>::VALUE should be distinct from Tmpl<char>::VALUE

	template<typename X>
	struct Tmpl<X*>
	{
		static const X* VALUE;
	};
	template<typename X>
	const X* Tmpl<X*>::VALUE = 0; // Tmpl<X*>::VALUE should be distinct from Tmpl<char>::VALUE
}


namespace N015
{
	template<typename T>
	struct Wrapper
	{
	};

	template<typename E, class A=Wrapper<E> > // the type of E should be correctly resolved
	struct Tmpl
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
		Wrapper<int> i = Tmpl<Wrapper<int> >::f();
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











#if 0
void hidden(int)
{
}
namespace N
{
	void hidden(void*)
	{
		hidden(1); // this should fail! Name lookup should NOT find '::hidden'
	}
}
#endif

namespace N037
{
	class C
	{
	};

	C& operator*(C& c)
	{
		return *c;
	}
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

#if 0 // This should NOT compile: an explicit-specialization of OneDefinition after its point of instantiation
template<>
struct OneDefinition<int>
{
};
#endif

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
		static const T m = 1;
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



