

namespace N343
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

#if 1 // TODO: defer lookup of 'f' until arguments are known 
namespace N321
{
	namespace N
	{
		struct A
		{
		};
		int f(A);
	}

	N::A a;
	int x = f(a);
}
#endif


namespace N341
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


namespace N339
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

namespace N255
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

namespace N278
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

namespace N338
{
	enum E
	{
	};

	// requires overload resolution against built-in operators, because operand has enumeration type
	int i = ~E();
	int j = -E();
	int k = +E();
}

namespace N336
{
	struct A
	{
		int m;
	};

	struct B
	{
		int A::* m;
	};

	A a;
	B b;
	int i = a.*b.m;
}

namespace N337
{
	struct A
	{
		int m();
	};

	struct B
	{
		int (A::* m)();
	};

	A a;
	B b;
	int i = (a.*b.m)();
}

namespace N335
{
	struct A
	{
		int m;
	};

	int A::* m = &A::m;

	A a;
	int i = a.*m;
	int j = (&a)->*m;
}

namespace N334
{
	template<class T>
	struct A
	{
		template<typename U>
		friend bool operator==(const T&x, const U&y)
		{
			return false;
		}
	};

	struct B : A<B>
	{
	};

	B b;
	bool x = b == b; // name lookup should find 'A<B>::operator==<B>'
}

namespace N333
{
	struct A
	{
		operator const int&()const;
		operator int&();
	};

	bool f(const int& x, const A& y);
	bool f(int, int);


	int i;
	A a;
	bool x = f(i, a); // calls 'f(const int&, const A&)' - because it is a better candidate than 'f(int, int)'
}

namespace N332
{
	template<class T, class U>
	struct B
	{
	private:
		friend bool operator<(const U&x, const T&y);
	};
	struct A : B<A, int>
	{
		operator const int&()const;
		operator int&();
	};

	int i;
	A a;
	bool x = i < a; // calls 'B<A>::operator<(const int&, const A&)' via koenig lookup - because it is a better candidate than built-in 'operator<(int, int)'
}


namespace N331
{
	template<class T>
	struct A
	{
		friend bool operator==(const T&x, const T&y)
		{
			return false;
		}
	};

	struct B : A<B>
	{
	};

	B b;
	bool x = b == b; // name lookup should find 'A<B>::operator=='
}

namespace N330
{
	struct A
	{
		operator const int&()const;
		operator int&(); // implicit-object parameter is better conversion from 'A&' because no 'added qualification'
	};

	int i;
	A a;
	bool x = i < a; // calls built-in operator<(int, int), via conversion function 'A::operator int&()'
}

namespace N329
{
	template<class T>
	struct A
	{
		friend bool operator==(const T&x, const T&y)
		{
			return false;
		}
	};

	enum E
	{
	};

	E e;
	bool x = e == e; // name lookup should not find 'A<T>::operator=='
}

namespace N328
{
	template<bool B, class T=void>
	struct enable_if_c
	{
		typedef T type;
	};
	template<class T>
	struct enable_if_c<false, T>
	{
	};
	template<class Cond, class T=void>
	struct enable_if: public enable_if_c<Cond::value, T>
	{
	};

	enum errc_t
	{
	};

	template< class T >
	struct is_error_condition_enum { static const bool value = false; };

	template<>
	struct is_error_condition_enum<errc_t>
	{
		static const bool value=true;
	
	};

	class error_condition
	{
	public:
		template<class ErrorConditionEnum>
		error_condition(ErrorConditionEnum e, typename enable_if<is_error_condition_enum<ErrorConditionEnum> >::type* =0);
		inline friend bool operator==(const error_condition&lhs, const error_condition&rhs);
	};
	struct A
	{
		inline friend bool operator==(const A&lhs, const A&rhs);
	};
	inline bool operator!=(const A&lhs, const A&rhs)
	{
		return !(lhs==rhs); // SFINAE: attempts and fails substitution of 'error_condition::error_condition<A>(A)' 
	}
}

namespace N222
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

namespace N326
{
	template<int i>
	struct A
	{
	};

	template<int i>
	bool f(A<i>& a);

	A<0> a;
	bool x = f(a);
}

namespace N325
{
	struct A
	{
		bool operator()();
	};

	A a;

	bool b = true && a();
}

namespace N324
{
	struct A
	{
		operator int*();
	};

	A a;

	bool b = a != 0;
}

namespace N323
{
	enum E
	{
		VALUE
	};

	const char p[1] = "";
	const char* i = p + VALUE;
}

namespace N322
{
	int f(const char*);
	void f(bool);

	int x = f("");
}

namespace N319
{
	enum E
	{
	};

	int i = E() + 1;
}

namespace N318
{
	int f(int*);

	template<int x>
	int g(int* p)
	{
		const int i = x;
		return f(true ? i : p);
	}

	int i = g<0>(0);
}


namespace N317
{
	int f(int*);

	int* p;
	int i = f(true ? 0 : p);
}


namespace N316
{
	template<typename>
	struct C
	{
	};

	struct I
	{
	};

	struct B : I
	{
	};

	struct D : B
	{
	};


	struct A
	{
		template<typename X, typename T>
		static T* f(C<X>&, T*);
		template<typename X>
		B* f(C<X>&, B*);
		template<typename X>
		I* f(C<X>&, I*);
	};

	C<int> c;
	A a;
	D* p = a.f(c, p); // overload resolution chooses 'A::f<int, D>(C<int>&, D*)'
}

namespace N315
{
	struct A
	{
		template<typename X>
		static int* f(X&, int*);
		template<typename X, typename T>
		int* f(X&, T*);
	};

	int x;
	A a;
	int* p = a.f(x, p); // overload resolution chooses 'A::f<int>(int&, int*)'
	// TODO: clang thinks this is ambiguous?
}


namespace N314
{
	template<typename X>
	int* f(X&, int*);
	template<typename X, typename T>
	int* f(X&, T*);

	int x;
	int* p = f(x, p); // overload resolution chooses 'f<int>(int&, int*)'
}

namespace N313
{
	struct S
	{
		operator const int&();
	};

	int f(const int&);

	int i = f(S()); // overload resolution chooses conversion 'S::operator const int&()'
}

namespace N308
{
	struct S
	{
		operator int*();
		operator int();
	};

	int f(int);

	int i = f(S()); // overload resolution chooses conversion 'S::operator int()'
}

namespace N312
{
	struct B
	{
		operator int*();
	};


	int f(int*);

	int i = f(B()); // overload resolution chooses conversion 'B::operator int*()'
}

namespace N311
{
	template<typename T>
	struct B
	{
		operator T*() const;
	};

	struct D : B<int>
	{
	};

	int f(int*);

	const D& d = D();
	int i = f(d); // overload resolution chooses conversion 'B<int>::operator int*()'
}

namespace N310
{
	struct B
	{
		int m;
	};

	struct A
	{
		B* operator->();
	};

	A a;
	int i = a->m;
};

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

namespace N305
{
	template<class _Elem>
	class basic_string
	{
	public:
		basic_string();
		basic_string(const _Elem*_Ptr);
		~basic_string();
	};

	int f(basic_string<char>);

	const char* s = "";
	int i = f(s); // conversion to basic_string<char>(int)
}

namespace N304
{
	template<class _Elem>
	class basic_string
	{
	public:
		typedef basic_string<_Elem> _Myt;
		typedef const _Elem* const_pointer;
		basic_string();
		basic_string(const _Myt&_Right);
		basic_string(const _Elem*_Ptr);
		template<class _It>
		basic_string(_It _First, _It _Last);
		template<class _It>
		basic_string(const_pointer _First, const_pointer _Last);
	};

	int f(basic_string<char>);

	const char* s = "";
	int i = f(s); // conversion to basic_string<char>(int)
}

namespace N282
{
	struct S
	{
		S(int);
		S(float);
		~S();
	};

	int f(S);

	int i = f(0); // conversion to S(int)
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

namespace N300
{
	template<typename T>
	T f(const T*const*);

	int** a;
	int i = f(a); // calls f(const int*const*)
}

namespace N298
{
	template<typename T>
	T f(const T*);

	int a;
	int i = f(&a); // calls f(const int*)
}

namespace N296
{
	template<typename T>
	struct S
	{
	};

	template<typename T>
	T f(const S<T>*);

	struct A : S<int>
	{
	};

	A a;
	int i = f(&a); // calls f(const S<int>*)
}

namespace N297
{
	template<typename DerivedT>
	struct parser
	{
	};
	template<typename DerivedT>
	struct char_parser: public parser<DerivedT>
	{
	};
	struct anychar_parser: public char_parser<anychar_parser>
	{
	};
	anychar_parser const anychar_p=anychar_parser();

	template<typename S>
	S operator*(parser<S>const&a);

	void f()
	{
		*anychar_p;
	}
}

namespace N295
{
	struct S
	{
	};

	template<typename T>
	int operator*(const T&);

	S s;
	int i = *s; // calls operator*(const S&);
}

namespace N294
{
	template<typename T>
	struct S
	{
	};

	template<typename T>
	T f(const S<T>&);

	struct A : S<int>
	{
	};

	A a;
	int i = f(a); // calls f(const S<int>&)
}

namespace N293
{
	template<typename T>
	struct S
	{
	};

	template<typename T>
	T f(const S<T>&);

	S<int> s;
	int i = f(s); // calls f(const S<int>&)
}

namespace N292
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

namespace N291
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

namespace N290
{
	template<typename A>
	struct parser
	{
	};

	template<typename A>
	inline void operator%(parser<A>const&a, char b)
	{
		*(b>>a.derived());
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

namespace N284
{
	template<class T>
	void f(const T&)
	{
	}
	void g(const int& i)
	{
		f(i);
	}
}

namespace N283
{
	struct S
	{
		void f()
		{
			this->~S();
		}
	};
}

namespace N281
{
	template<class U>
	inline int f(const U&_Val);

	int i = f(char(0xba));
}

namespace N279
{
	template<class T>
	inline int f(T _First, T _Last);

	char buffer[1];
	int i = f(buffer, buffer+1);
}

namespace N280
{
	enum E
	{
	};

	int f(int);
	int f(unsigned int);
	
	int i = f(E(0));
}

namespace N277
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

namespace N276
{
	typedef void(*event_callback)(int);
	struct _Fnarray
	{
		event_callback _Pfn;
	};
	struct S
	{
		void _Callfns()
		{
			_Fnarray*_Ptr = 0;
			(*_Ptr->_Pfn)(0);
		}
	};
}

namespace N275
{
	template<class _InIt>
	inline void _Debug_pointer(_InIt&)
	{
	}
	template<class _Ty>
	inline void _Debug_pointer(const _Ty*_First)
	{
	}
	template<class _Ty>
	inline void _Debug_pointer(_Ty*_First)
	{
	}

	struct ios_base;

	template<class _Elem, class _Traits>
	class basic_ostream
	{
		void operator<<(ios_base&(*_Pfn)(ios_base&))
		{
			_Debug_pointer(_Pfn);
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

namespace N273
{
	struct A
	{
		int operator[](int);
	};

	A a;
	int i = a[0];
}

namespace N272
{
	int f(...);

	int i = f(1);
}

namespace N271
{
	void f()
	{
		char *_Ptr = 0;
		*_Ptr++;
	}
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

namespace N257
{
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

namespace N264
{
	template<typename T>
	int f(const T*, const wchar_t*, unsigned int)
	{
		return 0;
	}
	template<typename T>
	int f(T*, const wchar_t*, unsigned int)
	{
		return 0;
	}

	const char* p;
	int i = f(p, L"", 0);
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

namespace N260
{
	template <typename T>
	struct Tmpl3
		: Tmpl3< T >::template Dependent<T>
	{
	};
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

namespace N234
{
	template<int i>
	int f()
	{
		return 0;
	}

	template<typename T>
	typename T::Result f()
	{
		return 0;
	}

	template<typename T>
	struct S
	{
		typedef int Result;
	};

	int a = f<S<int> >();
	int b = f<0>(); // SFINAE during overload resolution
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
	typename S<U>::Type S<U>::f();

	S<int> s;
	int i = s.f(); // return type is 'int'
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

namespace N235
{
	template<typename T, template<class> class TT>
	void f(TT<T>)
	{
	}

	template<class T>
	struct S
	{
	};

	void g()
	{
		S<int> s;
		f<int>(s);
	}
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

namespace N233
{
	void f(char*)
	{
		f(0); // null-pointer-constant matches T*
	}
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
