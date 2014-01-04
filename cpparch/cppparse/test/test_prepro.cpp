
namespace Temptest
{
	int* p;
	int* q = p++;
}

namespace N83 // test parse of explicit call of implicitly declared assignment operator
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

namespace N343 // test parse of explicit call of implicitly declared assignment operator
{
	class C
	{
		void f()
		{
			// [special] Programs may explicitly refer to implicitly declared special member functions.
			this->operator=(*this); // explicit call of operator not supported
		}
	};
}


#if 0 // TODO: offsetof as constant expression: &(((A*)0)->m)
namespace N375
{
	struct A
	{
		int m;
	};

	typedef char __C_ASSERT__[((((long)(long*)&(((A*)0)->m))&(64-1))==0)?1: -1];
}
#endif

namespace N374 // test name hiding for class member declared with same name as type of a base class
{
	struct A
	{
	};

	struct B : A
	{
		int A; // hides base 'A'
		B() : A(0) // name lookup finds name of member 'A', which hides base 'A'
		{
		}
	};
}

namespace N260 // test pre-C++11 behaviour allowing/requiring 'template' in base-specifier and mem-initializer
{
	template <typename T>
	struct A
	{
	};

	template <typename T>
	struct B
		: A<T>::template Dependent<T> // 'template' is unnecessary according to C++11, but we allow it for compatibility
	{
		B() : A<T>::template Dependent<T>() // 'template' is unnecessary according to C++11, but we allow it for compatibility
		{
		}
	};
}

namespace N373 // test parse of explicit destructor call using qualified-id syntax
{
	struct A
	{
		void f()
		{
			A::~A(); // qualified-id syntax for calling destructor
		}
	};
}

namespace N283 // test parse of explicit destructor call using psuedo-destructor-name syntax
{
	struct S
	{
		void f()
		{
			this->~S(); // destructor call using class member access syntax
		}
	};
}



namespace N370
{
	struct Page;

	inline bool isAllocated(const char* first, const char* last);

	template<bool checked>
	struct LinearAllocator
	{

		int position;
		static void*debugAddress;
		static char debugValue[4];
		static int debugAllocationId;
		void*allocate(int size)
		{
			if(position==debugAllocationId)
			{

			}
			int available=0;//sizeof(Page)-(position&Page::MASK);
			if(size>available)
			{
				position+=available;
			}
			Page*page=0;//getPage(position>>Page::SHIFT);
			void*p=0;//page->buffer+(position&Page::MASK);
			if(!(!checked||!isAllocated(reinterpret_cast<char*>(p), reinterpret_cast<char*>(p)+size)))
			{

			};

			position+=size;
			return p;
		}
	};
}

namespace N369 // test evaluation of constant expression used as template parameter
{
	template<int i>
	struct A
	{
		typedef A<i + 1> First;
	};

	template<>
	struct A<1>
	{
		typedef int Last;
	};

	typedef A<0>::First First; // type is 'A<1>'
	typedef First::Last Last; // explicit specialization of 'A<1>' has member 'Last'
}


#if 1//ndef _CPPP_TEST // TODO
namespace N365
{
	struct B
	{
		B& operator=(int*);
	};

	struct A : B
	{
		A& operator=(int* p)
		{
			B::operator=(p);
		}
		A& operator=(const B& b)
		{
			B::operator=(b);
		}
	};
}
#endif

namespace N363 // test parse of nested class template partial specialization defined within class
{
	struct Null {};

	struct S
	{
		template<typename, typename = void> struct map { typedef Null type; };
		template<typename T> struct S::map<int, T> { typedef char type; };
		template<typename T> struct S::map<char, T> { typedef int type; };
	};
}

namespace N144 // test that bitfield member declarations are correctly determined to be dependent 
{
	template<typename T>
	struct S
	{
		// all three declarations have a dependent type
		const T i : 1, // first in list, named
j : 1, // second in list, named
	: 1; // third in list, anonymous
		void f()
		{
			dependent(i);
			dependent(j);
		}
	};
}


namespace N116 // test that default-arguments are parsed before member function bodies
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

namespace N362 // test name lookup for declaration of class first declared as elaborated-type-specifier in conditional statement
{
	void f()
	{
		if(struct S* p = 0) // declares 'S' in the current scope
		{
			S* x; // name lookup finds 'S'
		}
	}
}


namespace N361 // test disambiguation between 'id-expression < expression' and simple-template-id
{
	class A
	{
	public:
		void f(const A&lhs, const A&rhs)
		{
			lhs.m_cat<rhs.m_cat||(lhs.m_cat==rhs.m_cat&&lhs.m_val<rhs.m_val); // 'lhs.m_cat' does not name a template
		}
	private:
		int m_val;
		int* m_cat;
	};
}


namespace N349 // test disambiguation between 'id-expression < expression' and simple-template-id
{
	template<typename T1, typename T2>
	struct pair
	{
		T1 first;
		T2 second;
	};

	template<class _Ty1, class _Ty2>
	inline bool operator<(const pair<_Ty1, _Ty2>&_Left, const pair<_Ty1, _Ty2>&_Right)
	{
		return (_Left.first<_Right.first||!(_Right.first<_Left.first)&&_Left.second<_Right.second); // '_Left.first' does not name a template
	}
}


namespace N357
{
	template<int N>
	struct A
	{
	};

	template<class T>
	struct B
	{
		typedef const T* const_iterator;
	};

	template<int N>
	void f()
	{
		for(const A<N>* i=0; i!=2; ++i)
		{
			typedef A<N> Tmp;
			typedef B<Tmp> Blah;
			for(typename Blah::const_iterator i=0; i!=2; ++i)
			{
			}
		}
	}
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

namespace N360
{
	template<int i>
	struct B
	{
		typedef int Type;
	};

	template<bool b, int x = sizeof(B<sizeof(b)>::Type)> // type of 'b' is not dependent and evaluation of 'sizeof(b)' is not deferred
	struct A
	{
	};
}

#if 0 // TODO: tests that expect syntax error
namespace N359
{
	template<typename T>
	void f(int i)
	{
		T ac;
		ac.operator int*i; // syntax error:
		// parsed as: &(ac.operator int *) i
		// not as: &(ac.operator int)*i
	}
}
#endif

namespace N358 // test instantiation of class template with dependent base class of the form `T::Type`
{
	struct A
	{
		typedef A Type;
	};

	template<typename T>
	struct B : T::Type
	{
	};

	typedef B<A>::Type Type; // B<A>::Type is not dependent
}


namespace N356 // test parse of explicit specialization member function definition which omits 'template<>'
{
	struct A
	{
		template<class T>
		struct B
		{
		};
	};

	template<>
	struct A::B<int>
	{
		B();
	};

	// optional explicit specialization syntax not used for a member of
	// explicitly specialized class template specialization
	A::B<int>::B() { }
}

namespace N355
{
	template<class T1>
	struct A
	{
		template<class T2>
		struct B
		{
			B();
			typename A<T1>::template B<T2> f();
		};
	};

	template<>
	template<>
	A<int>::B<int> // ok, typename/template required
		A<int>::B<int>::f()
	{
		return f();
	}
	template<>
	template<>
	A<int>::B<int>::B()
	{
	}
}

namespace N354
{
	template<class T1>
	struct A
	{
		template<class T2>
		struct B
		{
			static B f;
			static typename A<T2>::template B<T1> g;
		};
	};

	template<class T1>
	template<class T2>
	typename A<T2>::template B<T1> // ok, typename/template required
		A<T1>::B<T2>::g;

#if 0 // illegal
	template<class T1>
	template<class T2>
	A<T1>::B<T2> // clang/gcc accept, msvc rejects missing typename
		A<T1>::B<T2>::f
#endif


	template<class T1>
	struct C
	{
		template<class T2>
		struct D
		{
			static typename A<T1>::template B<T2> f;
			static typename A<T1>::template B<T2> g;
		};
	};

	template<class T1>
	template<class T2>
	typename A<T1>::template B<T2> // ok, typename/template required
		C<T1>::D<T2>::f;

#if 0 // illegal
	template<class T1>
	template<class T2>
	A<T1>::B<T2> // clang rejects with incorrect error
		C<T1>::D<T2>::g;
#endif
}

#ifdef _CPPP_TEST // illegal?
namespace N353
{
	template<class>
	struct ambiguous
	{
		static const int value = 0;
	};

	namespace N
	{
		template<class>
		int ambiguous();

		int i = ambiguous<int>::value;
	}
}
#endif




namespace N346
{
	struct A
	{
		int m[1];
	};

	void f(int)
	{
		const A& a = A();
		f((sizeof(a.m)));
	}
}


namespace N45 // test parsing of chained calls to 'operator()'
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
			S*const& p = this;
			p->m = 0;
		}
	};
}


namespace N341 // test parsing of call to conversion function for dependent type
{
	template<typename T>
	void f(T t)
	{
		typedef typename T::Dependent Dependent;
		t.operator Dependent();
	}
}

namespace N285
{
	enum E
	{
	};
	template<typename T>
	struct A
	{
		struct S
		{
			operator E() const;
			void f(const S& s)
			{
				s.operator E(); // E is looked up in the context of the entire postfix expression, not only within the qualifying type 'S'
			}
		};
	};
}


#if 0 // TODO: friend found via ADL
namespace N320
{
	template<typename T>
	struct B
	{
		friend T f(B);
	};

	struct D : B<int>
	{
	};

	D d;
	int i = f(d); // koenig lookup finds 'B<int>::f'
}
#endif

#if 0 // TODO: resolve 'identifier <' ambiguity earlier, or defer evaluation of expression type
namespace N327
{
	struct error_category;
	struct error_condition
	{
		template<class ErrorConditionEnum>
		error_condition(ErrorConditionEnum e, typename boost::enable_if<is_error_condition_enum<ErrorConditionEnum> >::type* =0)
		{
			*this=make_error_condition(e);
		}
		inline friend bool operator<(const error_condition&lhs, const error_condition&rhs)
		{
			return lhs.m_cat<rhs.m_cat||(lhs.m_cat==rhs.m_cat&&lhs.m_val<rhs.m_val);
		}
		int m_val;
		const error_category*m_cat;
	};
}
#endif


namespace N340
{
	struct B
	{
		struct M
		{
			void f();
		};
	};

	namespace N
	{
		struct M
		{
			void f();
		};
	}

	template<typename>
	struct A : N::M, B::M
	{
		typedef B N; // error: lookup of 'N' in member access expression is ambiguous
	};

	template<typename T>
	struct S : A<T>
	{

		A<T> m;
		void f()
		{
			m.N::M::f();
		}
	};

	//template class S<int>;
}

namespace N232
{
	template<typename T>
	struct S
	{
		T m;
		void f()
		{
			typedef typename T::Dependent Dependent;
			m.Dependent::dependent();
			m.dependent();
			m->dependent();
		}
	};
}


namespace N339 // test handling of ambiguity between psuedo-destructor-name and valid unary-expression: ~ id-expression
{
	enum
	{
		TokenTypeMask
	};

	enum
	{
		T_LAST_TOKEN_ID, T_LAST_TOKEN=((T_LAST_TOKEN_ID)&~TokenTypeMask)
	};
}

namespace N255 // test determination of type-dependent-ness for enumerator initializer consisting of sizeof expression
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

namespace N278 // test determination of type-dependent-ness for qualified-id referring to class that may or may not be a base-class
{
	struct C
	{
		int f();
		int m;
	};

	template<int i>
	struct S
	{
		typedef int Type;
	};


	template<typename T>
	struct B : C
	{
	};

	template<typename T>
	struct A : B<T>
	{
		void f()
		{
			!C::f().dependent; // dependent
			&C::f().dependent; // dependent
			typedef S<sizeof(C::m)>::Type Type1; // typename not required
			typedef S<sizeof(&C::m)>::Type Type2; // typename not required
		}
	};
}

namespace N338 // test parse of unary operator applied to operand with enum type
{
	enum E
	{
	};

	// requires overload resolution against built-in operators, because operand has enumeration type
	int i = ~E();
	int j = -E();
	int k = +E();
}


namespace N222 // test determination of dependentness for 'this' within the definition of a member function of a class-template
{
	template<typename T>
	struct B : T
	{
	};

	struct Outer
	{
		template <typename T>
		struct Inner;
	};

	template<typename T>
	struct Outer::Inner : B<T>
	{
		void f();
	};

	template<typename T>
	void Outer::Inner<T>::f()
	{
		this->dependent(); // 'this' is dependent
	}
}

#if 0 // TODO: this is invalid?
namespace N318
{
	int f(int*);

	template<int x>
	int g(int* p)
	{
		const int i = x;
		return f(true ? i : p); // error: operands to ?: have different types 'int' and 'int*'
	}

	int i = g<0>(0);
}
#endif

namespace N307
{
	template<typename T>
	struct B
	{
		typedef typename T::M Type;
	};

	template<typename T>
	struct A
	{
		typedef T M;
		M f();
	};

	template<typename T>
	typename A<T>::M A<T>::f()
	{
	}

	typedef B<A<int> >::Type Type; // implicit instantiation of 'A<int>' via 'B' should not cause instantiation of member function 'A<int>::f'
}

namespace N306
{
	template<typename T>
	struct B
	{
		typedef typename T::M Type;
	};

	template<typename T>
	struct A
	{
		typedef T M;
		static M const m;
	};

	template<typename T>
	typename A<T>::M const A<T>::m;

	typedef B<A<int> >::Type Type; // implicit instantiation of 'A<int>' via 'B' should not cause instantiation of static member 'A<int>::m'
}

namespace N302
{
	struct S
	{
		template<typename T>
		const T* f()const
		{
		}
		template<typename F>
		void g(const F&)const
		{
			this->template f<F>();
		}
	};
}

namespace N303
{
	struct S
	{
		template<typename T>
		const T* operator*()const
		{
		}
		template<typename F>
		void g(const F&)const
		{
			this->template operator*<F>();
		}
	};
}

#if 0 // TODO!
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
#endif


#if 0 // TODO!
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
		friend void Q::N(Y* = c); // 'Y' and 'c' should be looked up in the scope of C after looking within Q
	};

}
#endif

namespace N291 // test parse of  function template definition with return type that depends on template parameter of enclosing type
{
	template<class T>
	class S
	{
		S& f();
	};

	template<class T>
	S<T>& S<T>::f()
	{
		return *this;
	}
}

namespace N292 // test parse of member function template definition with return type that depends on template parameter of enclosing type
{
	template<class T>
	class S
	{
		template<typename U>
		S& f(U);
	};

	template<class T>
	template<class U>
	S<T>& S<T>::f(U)
	{
		return *this;
	}
}

namespace N290 // test parse of expression containing id-expression which depends on a template parameter
{
	template<typename T>
	struct A
	{
	};

	template<typename T>
	inline void operator%(const A<T>& a, char b)
	{
		*(b >> a.dependent()); // name lookup is deferred because the type of 'a' is dependent
	}
}

namespace N289
{
	namespace N
	{
		template<typename T>
		struct A
		{
		};
	}

	template<typename T>
	struct B
	{
		friend struct N::A<B<T> >; // 'B<T>' is dependent
	};
}

namespace N287 // TODO: the prototype of the friend declaration finds names within both the scope of A and B
{
	struct A
	{
		void f(A);
	};

	struct C
	{
		typedef A B;
		friend void A::f(B);
	};
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


namespace N286
{
	template<typename T>
	struct S
	{
		void f1(S&); // S<T>
		int m1;
	};

	template<>
	struct S<int>
	{
		void f2(S&); // S<int>
		int m2;
	};

	template<typename T>
	void S<T>::f1(S& other) // 'S' is interpreted as template-id 'S<T>', not template-name 'S'
	{
		other.m1 = 0;
	}

	void S<int>::f2(S& other) // 'S' is interpreted as template-id 'S<int>', not template-name 'S'
	{
		other.m2 = 0;
	}
}


namespace N277 // test that 'this' is correctly determined to be dependent within the definition of a class template
{
	template<class T>
	struct S
	{
		void f()
		{
			this->dependent();
		}
	};
}

namespace N274
{
	struct C
	{
		int m1;
		static int m2;
	};

	template<int i>
	struct S
	{
		typedef int Type;
	};


	template<typename T>
	struct B : C
	{
	};

	template<typename T>
	struct A : B<T>
	{
		void f()
		{
			typedef typename S<sizeof(C::m1)>::Type Type1; // typename required: implicitly '(*this).C::m1'
			typedef S<sizeof(C::m2)>::Type Type2; // typename not required
		}
	};
}



namespace N270
{
	enum E
	{
		VALUE = 0
	};
}

namespace N269
{
	struct A
	{
		int* p;
	};

	struct B
	{
		void f(int* i)
		{
			m->p[*i]; // only names after -> or . are looked up in 'm'
		}
		A* m;
	};
}


namespace N268
{
	struct A
	{
		void f(int);
	};
	template<class T>
	void f(A a)
	{
		int i = T::dependent;
		a.f(i); // 'i' is not a constant expression and should not be evaluated
	}
}

namespace N267
{
	struct A
	{
		void f(int);
	};

	struct B
	{
		void f(int i)
		{
			m->f(i); // only names after -> or . are looked up in 'm'
		}
		A* m;
	};
}

namespace N265
{
	template<typename T>
	struct S
	{
		union Inner
		{
			int* p;
		} m;

		void f()
		{
			m.p = 0; // 'p' not resolved until instantiation; type of 'm' is dependent
			m.y = 0; // 'y' not resolved until instantiation; type of 'm' is dependent
		}
	};
}

namespace N263
{
	template<class _Pr, class _Ty1>
	inline bool _Debug_lt_pred(_Pr _Pred, const _Ty1&_Left)
	{
		if(!_Pred(_Left))return (false);
		return (true);
	}
}

namespace N262
{
	void* memcpy(void*, const void*, unsigned int);

	wchar_t * wmemcpy(wchar_t *_S1, const wchar_t *_S2, unsigned int _N)
	{
		return (wchar_t *)memcpy(_S1, _S2, _N*sizeof(wchar_t));
	}
}

namespace N256
{
	template<typename X>
	struct Tmpl
	{
		static const X* VALUE;
	};
	template<typename X>
	const X* Tmpl<X>::VALUE = 0; // 'VALUE' is not a template name
}

// name-lookup for explicit argument-specification of overloaded function-template
namespace N91
{
	template<typename T>
	T f()
	{
		return 0;
	}
	int f()
	{
		return f<int>();
	}
}


namespace N115
{
	template<unsigned _SizeFIXED>
	inline char*gets_s(char(&_Buffer)[_SizeFIXED])
	{
		return gets_s(_Buffer, _SizeFIXED); // '_Buffer' is type-dependent, '_SizeFIXED' is value-dependent
	}
}

namespace N244
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


namespace N200
{
	class C
	{
		C* f();
	};

	C* C::f()
	{
		return f()->f();
	}
}

namespace N242
{
	struct S
	{
		operator int**()
		{
			return 0;
		}
	};
	void f(S& s)
	{
		s.operator int**();
	}
}

// deferred name lookup
namespace N504
{
	template<class T>
	struct Iterator
	{
		typedef typename T::Ref Ref; // cannot be looked up without instantiating

		Ref operator*()const;
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


namespace N231
{
	template<typename T>
	struct S
	{
	};

	template<>
	struct S<wchar_t>
	{
		static int eof();
		static int not_eof(const int&_Meta)
		{
			return (_Meta!=eof()?_Meta: !eof());
		}
	};
}

#ifdef _CPPP_TEST // fails in msvc 10 and clang 3.4
namespace N507
{
	namespace N
	{
		struct Type
		{
		};
		struct S : Type
		{
			operator Type()
			{
				return Type();
			}
		};
	}
	void f(N::S& s)
	{
		// If the id-expression is a conversion-function-id, its conversion-type-id is first looked up in the class of the
		// object expression and the name, if found, is used. Otherwise it is looked up in the context of the entire
		// postfix-expression.
		s.operator Type(); // Type should be looked up both in context of S and in context of postfix-expression
	}
}
#endif

#ifdef _CPPP_TEST // fails in msvc 10 and clang 3.4
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
		// If the id-expression is a conversion-function-id, its conversion-type-id is first looked up in the class of the
		// object expression and the name, if found, is used. Otherwise it is looked up in the context of the entire
		// postfix-expression.
		s.operator Type(); // Type should be looked up both in context of S and in context of postfix-expression
	}
}
#endif

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


namespace N241
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
		s.f(0);
	}
}


namespace N240
{
	template<class A>
	struct S
	{
		template<class T>
		struct Inner;
	};


	template<>
	template<class T>
	struct S<int>::Inner // 'Inner' is a template
	{
		void f();
	};

	template<>
	template<class T>
	void S<int>::Inner<T>::f() // 'f' is not a template
	{
	}

	template<>
	template<>
	struct S<int>::Inner<int> // 'Inner' is a template specialization
	{
		void f();
	};

	void S<int>::Inner<int>::f() // 'f' is not a template
	{
	}

}

namespace N236
{
	template<class T>
	struct S
	{
		struct Inner;
	};

	template<class T>
	struct S<T>::Inner // 'Inner' is not a template
	{
	};
}

namespace N239
{
	template<class T>
	struct S
	{
		void f();
	};

	template<class T>
	void S<T>::f() // 'inner' is not a template
	{
	}
}

namespace N238
{
	struct S
	{
		template<class T>
		void f();
	};

	template<class T>
	void S::f() // 'inner' is a template
	{
	}
}

namespace N237
{
	struct S
	{
		template<class T>
		struct Inner;
	};

	template<class T>
	struct S::Inner // 'Inner' is a template
	{
	};
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


namespace N230
{
	template<typename T>
	struct S
	{
		S f();
	};

	typedef S<int> Type;

	inline Type f(Type& p)
	{
		return p.f();
	}
}



namespace N223
{
	template<int i>
	struct S;
	template<>
	struct S<1>;
	template<>
	struct S<2>;
	template<>
	struct S<3>;
}

namespace N229
{
#if 0
	template<typename T>
	struct S
	{
		template<typename U>
		int f(U);
	};

	template<typename T>
	template<typename U>
	int S<T>::f(U)
	{
		return 0;
	}

	S<int> s;
	int i = s.f(0); // instantiates S<int> and S<int>::f<int>
#endif
}

namespace N228
{
	template<typename T>
	struct B
	{
		typedef typename T::Before Before; // ok
		//typedef typename T::Instantiate Instantiate; // ok
		//typedef typename T::After After; // ok
	};

	struct A
	{
		typedef int Before;
		typedef B<A>::Before Instantiate;
		//typedef int After;
	};
}

namespace N227
{
	template<typename T>
	struct B
	{
		static const int value = T::value; // ok
		//static const int instantiate = T::instantiate; // error: initializer is not a constant expression
		//static const int after = T::after; // error: no member named 'after' in 'A<int>'
	};

	template<typename T>
	struct A
	{
		static const int value = 0;
		static const int instantiate = B<A>::value;
		static const int after = 0;
	};

	static const int value = A<int>::instantiate; // instantiate A<int>
}

namespace N226
{
	template<void(*op)()>
	struct S
	{
		static int thunk();
	};

	inline void f();

	int i = S<f>::thunk();
}

namespace N225
{
	template<typename T>
	struct B
	{
		typedef typename T::Before Before; // ok
		//typedef typename T::After After; // error: no type named 'After' in 'A'
	};

	struct A
	{
		typedef int Before;
		typedef B<A>::Before Instantiate;
		typedef int After;
	};
}

namespace N224
{
	template<typename T>
	struct B
	{
		typedef typename T::Before Before; // ok
		//typedef typename T::After After; // error: no type named 'After' in 'A<int>'
	};

	template<typename T>
	struct A
	{
		typedef int Before;
		typedef typename B<A>::Before Instantiate;
		typedef int After;
	};

	typedef A<int>::Instantiate Type; // instantiate A<int>
}

namespace N221
{
	bool b1 = __is_class(int);
	bool b2 = __is_base_of(int, int);
}


#if 0
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#if 0
namespace boost
{
	namespace multi_index
	{
		namespace detail
		{
			struct index_applier
			{
				template<typename IndexSpecifierMeta, typename SuperMeta>
				struct apply
				{
					typedef typename IndexSpecifierMeta::type index_specifier;
					typedef typename index_specifier::template index_class<SuperMeta>::type type;
				};
			};
			template<int N, typename Value, typename IndexSpecifierList, typename Allocator>
			struct nth_layer
			{
				static const int length=mpl::size<IndexSpecifierList>::value;
				typedef typename mpl::eval_if_c<N==length, mpl::identity<index_base<Value, IndexSpecifierList, Allocator> >, mpl::apply2<index_applier, mpl::at_c<IndexSpecifierList, N>, nth_layer<N+1, Value, IndexSpecifierList, Allocator> > >::type type;
			};
			template<typename Value, typename IndexSpecifierList, typename Allocator>
			struct multi_index_base_type: nth_layer<0, Value, IndexSpecifierList, Allocator>
			{
				typedef ::boost::static_assert_test<sizeof(::boost::STATIC_ASSERTION_FAILURE<(bool)(detail::is_index_list<IndexSpecifierList>::value)>)>boost_static_assert_typedef___COUNTER__;
			};
		}
	}
}
#endif

namespace boost
{
	namespace wave
	{
		namespace util
		{
			struct from
			{
			};
			struct to
			{
			};
			struct pair
			{
				int first;
				int second;
			};

			typedef pair value_type;
			typedef int FromType;
			typedef int ToType;

			typedef value_type Value;
			typedef boost::multi_index::indexed_by<boost::multi_index::ordered_unique<boost::multi_index::tag<from>, boost::multi_index::member<value_type, FromType, &value_type::first> >, boost::multi_index::ordered_non_unique<boost::multi_index::tag<to>, boost::multi_index::member<value_type, ToType, &value_type::second> > > IndexSpecifierList;
			typedef std::allocator<Value> Allocator;

			static const int N = 0;

			static const int length=mpl::size<IndexSpecifierList>::value;
			typedef mpl::eval_if_c<N==length,
				mpl::identity<boost::multi_index::detail::index_base<Value, IndexSpecifierList, Allocator> >,
				mpl::apply2<boost::multi_index::detail::index_applier, mpl::at_c<IndexSpecifierList, N>, boost::multi_index::detail::nth_layer<N+1, Value, IndexSpecifierList, Allocator> >
			>::type tmp;

			typedef boost::multi_index::multi_index_container<value_type, IndexSpecifierList>::value_type type;
		}
	}
}
#endif

#if 0 // TODO: handle defaults 
namespace N215
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

	template<template<typename P1 = int>class F>
	struct B<F<> >
	{
		typedef int Special;
	};

	typedef B<A<int> >::Special Special;
}
#endif

#if 0 // bit suspect?
namespace N216
{
	template<typename T>
	struct A
	{
	};

	template<typename F, typename G = int>
	struct B
	{
		typedef int Primary;
	};

	template<class F>
	struct B<F>
	{
		typedef int Special;
	};

	typedef B<A<int> >::Special Special;
}
#endif
