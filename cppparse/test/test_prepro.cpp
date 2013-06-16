
namespace N186
{
	template<typename T>
	struct sfinae
	{
		typedef void Type;
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

		static const bool value=true;
		typedef sfinae<value> type;
	};

	struct A
	{
		typedef int Type;
	};

	struct B
	{
	};

	typedef S<A>::True True;
	typedef S<B>::False False;
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

namespace N177
{
	template<template<class> class TT>
	void f(TT<int>)
	{
	}

	template<class T>
	struct S
	{
	};

	void g()
	{
		S<int> s;
		f(s);
	}
}

#if 0
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

namespace N152
{
	template<int x>
	struct I;

	template<typename T>
	struct S
	{
		static int f(int);
		static T f(const char*);

		// [temp.dep.expr] 'f' is dependent because it is an identifier that was declared with a dependent type
		static const int value = I<sizeof(f(3))>::dependent;
		static const int x = sizeof(f(3).x);
	};
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
	struct A;

	template<>
	struct A<int>
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



namespace N131
{
	template<typename T>
	struct S : T::Nested::X
	{
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

namespace N123
{
	template<typename T>
	struct S
	{
		typedef T Type;

		Type f();
	};

	template<typename U>
	typename S<U>::Type S<U>::f()
	{
	}

	S<int> s;
	int i = s.f(); // return type is 'int'
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
			(*this)->m = 0;
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
		s.f(A()); // should link to N111.S.f(???)
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
namespace N096
{
	template<typename T>
	struct S
	{
		void f(T); // declaration
	};

	template<typename T>
	inline void S<T>::f(T) // definition of previous declaration
	{
	}

	void f()
	{
		S<int> s;
		s.f();
	}
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

namespace N059
{
	template<typename T, typename U>
	struct Tmpl
	{
		void f(T); // declaration
		void f(U); // redeclaration (illegal)
	};

	Tmpl<int, const int> t;
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





namespace N049
{
	template<typename T>
	struct Tmpl
	{
	};

	template<>
	struct Tmpl<int>
	{
		template<typename T>
		void f(); // declaration
	};

	// omitting optional 'template<>'
	template<typename T>
	void Tmpl<int>::f() // definition
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

namespace N001
{
	struct M
	{
		template<int i>
		float dependent(int j)
		{
			return 0;
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
		o.f().dependent<0>(0); // type of expression should resolve to 'float'
	}
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

