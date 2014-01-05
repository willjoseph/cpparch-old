

namespace N372
{
	// using decltype-specifier in simple-type-specifier
	decltype(0) a;

	struct A
	{
		void f()
		{
#ifdef _CPPP_TEST // msvc 2010 fails this test
			// using decltype-specifier in nested-name-specifier and unqualified-id
			decltype(A())::~decltype(A())();
#endif
		}
	};

	void f()
	{
#ifdef _CPPP_TEST // msvc 2010 fails this test
		// using decltype-specifier in psuedo-destructor-name
		(0).~decltype(0)();
#endif
		// using decltype-specifier in simple-type-specifier
		decltype(A()) a;
#ifdef _CPPP_TEST // msvc 2010 fails this test
		// using decltype-specifier in psuedo-destructor-name
		a.~decltype(A())();
#endif
	};

#ifdef _CPPP_TEST // msvc 2010 fails this test
	struct B : decltype(A()) // using decltype-specifier in base-specifier
	{
		B() : decltype(A())(A()) // using decltype-specifier in mem-initializer
		{
		}
	};
#endif
}

#ifdef _CPPP_TEST // causes static_assert to fire
namespace N371
{
	static_assert(true, "");
	static_assert(false, "?false");

	class A
	{
		static_assert(true, "");
		static_assert(false, "?false");
	};

	void f()
	{
		static_assert(true, "");
		static_assert(false, "?false");
	}
}
#endif

namespace N385 // test deferred evaluation of static_assert
{
	template<bool b>
	class A
	{
		static_assert(!b, "");
		static_assert(b, "?false");

		typedef int Type;
	};

	typedef A<false>::Type Type;
}

namespace cppp
{
	template<class T, class U>
	struct is_same
	{
		static const bool value = false;
	};

	template<class T>
	struct is_same<T, T> // TODO: unit test for partial specialization
	{
		static const bool value = true;
	};

}

#define STATIC_ASSERT(e) static_assert(e, "")
#define IS_SAME(T, U) cppp::is_same<T, U>::value
#define STATIC_ASSERT_IS_SAME(T, U) STATIC_ASSERT(IS_SAME(T, U))
#define STATIC_ASSERT_IS_DIFFERENT(T, U) STATIC_ASSERT(!IS_SAME(T, U))
#define ASSERT_EXPRESSION_TYPE(e, T) STATIC_ASSERT_IS_SAME(decltype(e), T)

namespace N382
{
	STATIC_ASSERT_IS_SAME(int, int);
	STATIC_ASSERT_IS_DIFFERENT(int, bool);
	STATIC_ASSERT_IS_SAME(int*, int*);
	STATIC_ASSERT_IS_DIFFERENT(int, int*);
	STATIC_ASSERT_IS_SAME(int&, int&);
	STATIC_ASSERT_IS_DIFFERENT(int, int&);
	STATIC_ASSERT_IS_SAME(const int&, const int&);
	STATIC_ASSERT_IS_DIFFERENT(int, const int&);
	STATIC_ASSERT_IS_SAME(const int, const int);
	STATIC_ASSERT_IS_DIFFERENT(int, const int);
	STATIC_ASSERT_IS_SAME(unsigned int, unsigned int);
	STATIC_ASSERT_IS_DIFFERENT(int, unsigned int);
}

namespace N383
{
	ASSERT_EXPRESSION_TYPE(true, bool);
	ASSERT_EXPRESSION_TYPE(false, bool);
	ASSERT_EXPRESSION_TYPE(0, int);
	ASSERT_EXPRESSION_TYPE(0u, unsigned);
	ASSERT_EXPRESSION_TYPE(0l, long);
	ASSERT_EXPRESSION_TYPE(0ul, unsigned long);
	ASSERT_EXPRESSION_TYPE(0U, unsigned);
	ASSERT_EXPRESSION_TYPE(0L, long);
	ASSERT_EXPRESSION_TYPE(0UL, unsigned long);
	ASSERT_EXPRESSION_TYPE(0.f, float);
	ASSERT_EXPRESSION_TYPE(0.0, double);
	ASSERT_EXPRESSION_TYPE(0.l, long double);
	// ASSERT_EXPRESSION_TYPE(0ll, long long); // TODO: long long
	ASSERT_EXPRESSION_TYPE('\0', char);
	ASSERT_EXPRESSION_TYPE(L'\0', wchar_t);
	//ASSERT_EXPRESSION_TYPE("", const char(&)[1]); // TODO: array size for string literal
	//ASSERT_EXPRESSION_TYPE(L"", const wchar_t*(&)[1]);
	ASSERT_EXPRESSION_TYPE(9 * (8.2), double);
	ASSERT_EXPRESSION_TYPE(9 * 8.2, double);
	ASSERT_EXPRESSION_TYPE(3 * (9 * 8.2), double);
	ASSERT_EXPRESSION_TYPE(1 * 2U * 3.0, double);

	ASSERT_EXPRESSION_TYPE(int(), int);

	// The type denoted by decltype(e) is defined as follows:
	//  - if e is an unparenthesized id-expression or an unparenthesized class member access (5.2.5), decltype(e)
	//  	is the type of the entity named by e. If there is no such entity, or if e names a set of overloaded functions,
	//  	the program is ill-formed;
	//  - otherwise, if e is an xvalue, decltype(e) is T&&, where T is the type of e;
	//  - otherwise, if e is an lvalue, decltype(e) is T&, where T is the type of e;
	//  - otherwise, decltype(e) is the type of e.

	struct A
	{
		int f();
		int m;
	};

	int f();
	int& r();
	int i;
	const int ci = 0;
	A a;
	A* p;
	int A::* pm;
	int (A::* pmf)();

	enum E { VALUE };

	ASSERT_EXPRESSION_TYPE(i, int); // unparenthesized id-expression
	ASSERT_EXPRESSION_TYPE(ci, const int); // unparenthesized id-expression
	ASSERT_EXPRESSION_TYPE(f, int()); // unparenthesized id-expression (not overloaded)
	ASSERT_EXPRESSION_TYPE(a.m, int); // unparenthesized class-member-access
	ASSERT_EXPRESSION_TYPE(p->m, int); // unparenthesized class-member-access
	ASSERT_EXPRESSION_TYPE(VALUE, E); // unparenthesized id-expression


	ASSERT_EXPRESSION_TYPE((i), int&); // yields a reference because 'i' is an lvalue
	ASSERT_EXPRESSION_TYPE((ci), const int&); // yields a reference because 'ci' is an lvalue
	ASSERT_EXPRESSION_TYPE((f), int(&)()); // yields a reference because 'f' is an lvalue
	ASSERT_EXPRESSION_TYPE((a.m), int&); // yields a reference because 'a.m' is an lvalue
	ASSERT_EXPRESSION_TYPE((p->m), int&);
	ASSERT_EXPRESSION_TYPE((VALUE), E); // does not yield a reference because 'VALUE' is not an lvalue

	struct C
	{
		void f()
		{
			ASSERT_EXPRESSION_TYPE(this, C*); // not an lvalue
		}
		void f() const
		{
			// TODO: 'this' should be const
			//ASSERT_EXPRESSION_TYPE(this, const C*); // not an lvalue
		}
		static void s();
	};
	C c;

	// postfix expressions
	ASSERT_EXPRESSION_TYPE(p[0], A&); // lvalue
	ASSERT_EXPRESSION_TYPE(f(), int); // not an lvalue
	ASSERT_EXPRESSION_TYPE(r(), int&); // lvalue
	ASSERT_EXPRESSION_TYPE(c.f(), void); // not an lvalue
	ASSERT_EXPRESSION_TYPE(int(), int);
	ASSERT_EXPRESSION_TYPE(A(), A);
	ASSERT_EXPRESSION_TYPE(E(), E);
	ASSERT_EXPRESSION_TYPE(E(), E);
	ASSERT_EXPRESSION_TYPE(c.s, void());
	//ASSERT_EXPRESSION_TYPE((c.s), void()); // TODO
	ASSERT_EXPRESSION_TYPE(p++, A*);
	ASSERT_EXPRESSION_TYPE(p--, A*);
	ASSERT_EXPRESSION_TYPE(i++, int);
	ASSERT_EXPRESSION_TYPE(i--, int);

	struct B : A
	{
		virtual ~B();
	};
	B* b;
	ASSERT_EXPRESSION_TYPE(dynamic_cast<A*>(b), A*);
	ASSERT_EXPRESSION_TYPE(dynamic_cast<A&>(*b), A&);
	ASSERT_EXPRESSION_TYPE(static_cast<float>(0), float);
	ASSERT_EXPRESSION_TYPE(static_cast<A*>(b), A*);
	ASSERT_EXPRESSION_TYPE(static_cast<A&>(*b), A&);
	ASSERT_EXPRESSION_TYPE(reinterpret_cast<A*>(b), A*);
	ASSERT_EXPRESSION_TYPE(reinterpret_cast<A&>(*b), A&);
	ASSERT_EXPRESSION_TYPE(const_cast<const A*>(p), const A*);
	ASSERT_EXPRESSION_TYPE(const_cast<const A&>(*p), const A&);

	// unary expressions
	ASSERT_EXPRESSION_TYPE(++p, A*&);
	ASSERT_EXPRESSION_TYPE(--p, A*&);
	ASSERT_EXPRESSION_TYPE(++i, int&);
	ASSERT_EXPRESSION_TYPE(--i, int&);
	ASSERT_EXPRESSION_TYPE(*p, A&);
	ASSERT_EXPRESSION_TYPE(&a, A*);
	ASSERT_EXPRESSION_TYPE(+i, int);
	ASSERT_EXPRESSION_TYPE(-i, int);
	ASSERT_EXPRESSION_TYPE(!i, bool);
	ASSERT_EXPRESSION_TYPE(~i, int);
	ASSERT_EXPRESSION_TYPE(sizeof(0), size_t);
	ASSERT_EXPRESSION_TYPE(sizeof(int), size_t);
	ASSERT_EXPRESSION_TYPE(new int, int*);
	ASSERT_EXPRESSION_TYPE(new int[1], int*);
	ASSERT_EXPRESSION_TYPE(delete p, void);

	// cast
	ASSERT_EXPRESSION_TYPE((A*)b, A*);
	ASSERT_EXPRESSION_TYPE((A&)*b, A&);

	// pm
	ASSERT_EXPRESSION_TYPE(a.*pm, int&);
	ASSERT_EXPRESSION_TYPE(p->*pm, int&);
	ASSERT_EXPRESSION_TYPE((a.*pmf)(), int);
	ASSERT_EXPRESSION_TYPE((p->*pmf)(), int);

	ASSERT_EXPRESSION_TYPE(i * 3, int);
	ASSERT_EXPRESSION_TYPE(i + 3, int);
	ASSERT_EXPRESSION_TYPE(p + 3, A*);
	//ASSERT_EXPRESSION_TYPE(p - p, ptrdiff_t); // TODO: ptrdiff_t

	ASSERT_EXPRESSION_TYPE(i << 3, int);
	ASSERT_EXPRESSION_TYPE(i < 3, bool);
	ASSERT_EXPRESSION_TYPE(i == 3, bool);
	ASSERT_EXPRESSION_TYPE(i & 3, int);

	ASSERT_EXPRESSION_TYPE(i * VALUE, int);
	ASSERT_EXPRESSION_TYPE(i + VALUE, int);
	ASSERT_EXPRESSION_TYPE(p + VALUE, A*);

	ASSERT_EXPRESSION_TYPE(i << VALUE, int);
	ASSERT_EXPRESSION_TYPE(i < VALUE, bool);
	ASSERT_EXPRESSION_TYPE(i == VALUE, bool);
	ASSERT_EXPRESSION_TYPE(i & VALUE, int);

	ASSERT_EXPRESSION_TYPE(i && 3, bool);

	ASSERT_EXPRESSION_TYPE(false ? 0 : 0, int);

	ASSERT_EXPRESSION_TYPE(i = 0, int&);

	ASSERT_EXPRESSION_TYPE((i, 0), int); // comma operator
	ASSERT_EXPRESSION_TYPE((0, i), int&); // comma operator

}

#if 0 // TODO
namespace N384
{
	int f(int);
	int& fr(int);

	template<typename T>
	struct C
	{
		T m;
		T mf();
		T& mfr();
		T f();
		static T smf();
	};

	template<typename T, T i>
	struct A
	{
		typedef A<int, 0> Self;
		T m;
		const T cm = 0;
		static T sm;
		static T* p;
		static C<T> c;
		static C<T>* pc;
		static C<T>& rc;


		ASSERT_EXPRESSION_TYPE(i, int); // unparenthesized id-expression
		ASSERT_EXPRESSION_TYPE(m, int);
		ASSERT_EXPRESSION_TYPE(cm, const int);
		ASSERT_EXPRESSION_TYPE(sm, int);
		ASSERT_EXPRESSION_TYPE(c.m, int); // TODO: fails
		ASSERT_EXPRESSION_TYPE(pc->m, int);
		ASSERT_EXPRESSION_TYPE(rc.m, int);
		ASSERT_EXPRESSION_TYPE(c.smf, int());

		ASSERT_EXPRESSION_TYPE((i), int); // not lvalue
		ASSERT_EXPRESSION_TYPE((m), int&);
		ASSERT_EXPRESSION_TYPE((cm), const int&);
		ASSERT_EXPRESSION_TYPE((sm), int&);
		ASSERT_EXPRESSION_TYPE((c.m), int&);
		ASSERT_EXPRESSION_TYPE((pc->m), int&);
		ASSERT_EXPRESSION_TYPE((rc.m), int&);
		ASSERT_EXPRESSION_TYPE((c.smf), int());

		int mf()
		{
			ASSERT_EXPRESSION_TYPE(this, Self*);
		}
		int mfc() const
		{
			ASSERT_EXPRESSION_TYPE(this, const Self*);
		}

		ASSERT_EXPRESSION_TYPE(p[0], int&);
		ASSERT_EXPRESSION_TYPE(f(i), int);
		ASSERT_EXPRESSION_TYPE(fr(i), int&);
		ASSERT_EXPRESSION_TYPE(c.f(), int);
		ASSERT_EXPRESSION_TYPE(T(), int);
		ASSERT_EXPRESSION_TYPE(C<T>(), C<int>);
		ASSERT_EXPRESSION_TYPE(p++, int*);
		ASSERT_EXPRESSION_TYPE(p--, int*);
		ASSERT_EXPRESSION_TYPE(sm++, int);
		ASSERT_EXPRESSION_TYPE(sm--, int);

		typedef int Type;
	};

	// instantiate
	typedef A<int, 0>::Type Type;
	A<int, 0> a;
	int i = a.mf();
	int j = a.mfc();
}
#endif

namespace N344 // non-dependent non-type-template-parameter
{
	template<bool b, bool func()>
	void f()
	{
		static_assert(sizeof(b), "?evaluated"); // type of 'b' is not dependent and can be resolved during initial parse
		static_assert(sizeof(func()), "?evaluated"); // type of 'func' is not dependent and can be resolved during initial parse
	}

	template<bool b, bool func()>
	struct A
	{
#ifdef _CPPP_TEST // msvc 2010 fails this test
		static_assert(sizeof(b), "?evaluated"); // type of 'b' is not dependent and can be resolved during initial parse
		static_assert(sizeof(func()), "?evaluated"); // type of 'func' is not dependent and can be resolved during initial parse
#endif
	};
}

namespace N352 // test nondependent non-type template parameter in non-type template parameter default argument
{
	template<bool b, typename T = decltype(b)> // type of 'b' is not dependent and evaluation of 'decltype(b)' is not deferred
	struct A
	{
		static const T value = b;
	};

	ASSERT_EXPRESSION_TYPE(A<false>::value, const bool);
}

namespace N351 // test nondependent non-type template parameter in constant-expression
{
	template<bool b>
	struct A
	{
		static const int value = sizeof(b);
	};

	ASSERT_EXPRESSION_TYPE(A<false>::value, const int); // check that the expression type is successfully evaluated
}

namespace N317 // test evaluation of type of conditional expression with null-pointer and pointer
{
	int* p;
	ASSERT_EXPRESSION_TYPE(true ? 0 : p, int*);
}

namespace N368 // test evaluation of type of conditional expression with null-pointer and function-pointer
{
	void f();
	ASSERT_EXPRESSION_TYPE(false ? 0 : f, void(*)()); // 'f' is not overloaded, its type can be resolved to 'void()'
}


namespace N323 // test evaluation of type of additive expression with pointer operand
{
	enum E
	{
		VALUE
	};

	const char p[1] = "";
	ASSERT_EXPRESSION_TYPE(p + VALUE, const char*);
}

namespace N319 // test evaluation of type of additive expression with enum operand
{
	enum E
	{
	};

	ASSERT_EXPRESSION_TYPE(E() + 1, int);
}


namespace N310 // test evaluation of type of expression involving overloaded operator->
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
	ASSERT_EXPRESSION_TYPE((a->m), int&);
};

namespace N322 // test overload resolution
{
	int f(const char*);
	void f(bool);

	ASSERT_EXPRESSION_TYPE(f(""), int); // calls 'f(const char*)'
}


namespace N333 // test overload resolution with direct-reference-binding and user-defined-conversion
{
	struct A
	{
		operator const int&()const;
		operator int&();
	};

	int f(const int& x, const A& y);
	bool f(int, int);


	int i;
	A a;
	ASSERT_EXPRESSION_TYPE(f(i, a), int); // calls 'f(const int&, const A&)' - because it is a better candidate than 'f(int, int)'
}

namespace N313 // test overload resolution with user-defined-conversion to const reference
{
	struct S
	{
		operator const int&();
	};

	int f(const int&);

	ASSERT_EXPRESSION_TYPE(f(S()), int); // overload resolution chooses conversion 'S::operator const int&()'
}

namespace N308 // test overload resolution with viable and non-viable user-defined-conversion
{
	struct S
	{
		operator int*();
		operator int();
	};

	int f(int);

	ASSERT_EXPRESSION_TYPE(f(S()), int); // overload resolution chooses conversion 'S::operator int()'
}

namespace N312 // test overload resolution with user-defined-conversion to pointer
{
	struct B
	{
		operator int*();
	};


	int f(int*);

	ASSERT_EXPRESSION_TYPE(f(B()), int); // overload resolution chooses conversion 'B::operator int*()'
}

namespace N282 // test overload resolution with multiple converting constructors
{
	struct S
	{
		S(int);
		S(float);
		~S();
	};

	int f(S);

	ASSERT_EXPRESSION_TYPE(f(0), int); // calls 'S(int)'
}

namespace N304 // test overload resolution with multiple converting constructor templates
{
	template<class T>
	class A
	{
	public:
		typedef A<T> _Myt;
		typedef const T* const_pointer;
		A();
		A(const _Myt&);
		A(const T*);
		template<class _It>
		A(_It, _It);
		template<class _It>
		A(const_pointer, const_pointer);
	};

	int f(A<char>);

	const char* s = "";
	ASSERT_EXPRESSION_TYPE(f(s), int); // calls 'A<char>(const char*)'
}

namespace N305 // test overload resolution with converting constructor template
{
	template<class T>
	struct A
	{
		A();
		A(const T*);
	};

	int f(A<char>);

	const char* s = "";
	ASSERT_EXPRESSION_TYPE(f(s), int); // calls 'A<char>(const char*)'
}

namespace N311 // test overload resolution with user-defined-conversion template
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
	ASSERT_EXPRESSION_TYPE(f(d), int); // overload resolution chooses conversion 'B<int>::operator int*()'
}

namespace N314 // test overload resolution with overloaded function templates
{
	template<typename X>
	int* f(X&, int*);
	template<typename X, typename T>
	void* f(X&, T*);

	int x;
	int* p;
	ASSERT_EXPRESSION_TYPE(f(x, p), int*); // overload resolution chooses 'f<int>(int&, int*)'
}

namespace N315 // test overload resolution with overloaded member function templates
{
	struct A
	{
		template<typename X>
		static int* f(X&, int*);
		template<typename X, typename T>
		void* f(X&, T*);
	};

	int x;
	A a;
	int* p;
	ASSERT_EXPRESSION_TYPE(a.f(x, p), int*); // overload resolution chooses 'A::f<int>(int&, int*)'
}

namespace N316 // test overload resolution with overloaded function templates and class hierarchy
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
	D* p;
	ASSERT_EXPRESSION_TYPE(a.f(c, p), D*); // overload resolution chooses 'A::f<int, D>(C<int>&, D*)'
}

namespace N367 // test overload resolution of overloaded operator+ with pointer-to-overloaded-function argument
{
	struct A
	{
	};

	int operator+(A, void(*)(int));
	float operator+(A, void(*)(float));

	void f();
	void f(int);

	void g()
	{
		A a;
		ASSERT_EXPRESSION_TYPE(a + f, int); // 'f' is overloaded, the correct overload 'f(int)' should be chosen
#ifdef _CPPP_TEST // msvc 2010 crashes with this test
		ASSERT_EXPRESSION_TYPE(a + &f, int); // 'f' is overloaded, the correct overload 'f(int)' should be chosen
#endif
	}
}

namespace N348 // test call to overloaded operator++
{
	struct A
	{
		A& operator++(int);
	};

	A a;
	ASSERT_EXPRESSION_TYPE(a++, A&); // calls 'A::operator++(int)'
}

namespace N271 // test call to built-in operator++(char*) followed by built-in operator*(char*)
{
	void f()
	{
		char* _Ptr = 0;
		ASSERT_EXPRESSION_TYPE(*_Ptr++, char&);
	}
}

namespace N366 // test function call with pointer-to-overloaded-function argument
{
	void f();
	void f(int);

	int g(void(*)(int));

	ASSERT_EXPRESSION_TYPE(g(f), int); // 'f' is overloaded, the correct overload 'f(int)' should be chosen
}

namespace N281 // test template argument deduction and return-type substitution in call to function-template
{
	template<class U>
	inline U f(const U&_Val);

	ASSERT_EXPRESSION_TYPE(f(char(0xba)), char);
}

namespace N284 // test template argument deduction and return-type substitution in call to function-template
{
	template<class T>
	int f(const T&);

	const int& i = 0;
	ASSERT_EXPRESSION_TYPE(f(i), int);
}

namespace N298 // test template argument deduction and return-type substitution in call to function-template 
{
	template<typename T>
	T f(const T*);

	int a;
	ASSERT_EXPRESSION_TYPE(f(&a), int); // calls f(const int*)
}

namespace N296 // test template argument deduction for pointer to class-template parameter
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
	ASSERT_EXPRESSION_TYPE(f(&a), int); // calls f(const S<int>*)
}

namespace N293 // test template argument deduction for reference to class-template parameter
{
	template<typename T>
	struct S
	{
	};

	template<typename T>
	T f(const S<T>&);

	S<int> s;
	ASSERT_EXPRESSION_TYPE(f(s), int); // calls f(const S<int>&)
}

namespace N294 // test template argument deduction for reference to class-template parameter
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
	ASSERT_EXPRESSION_TYPE(f(a), int); // calls f(const S<int>&)
}

namespace N295 // test template argument deduction for overloaded operator*
{
	struct S
	{
	};

	template<typename T>
	int operator*(const T&);

	S s;
	ASSERT_EXPRESSION_TYPE(*s, int); // calls operator*(const S&);
}


namespace N300 // test template argument deduction for parameter with additional cv-qualification
{
	template<typename T>
	T f(const T*const*);

	int** a;
	ASSERT_EXPRESSION_TYPE(f(a), int); // calls f(const int*const*)
}

namespace N326 // test non-type template argument deduction and return-type substitution in call to function template
{
	template<int i>
	struct A
	{
	};

	template<int i>
	bool f(A<i>& a);

	A<0> a;
	ASSERT_EXPRESSION_TYPE(f(a), bool); // calls f(A<0>&)
}

namespace N350 // test template argument deduction and return-type substitution in call to function-template, from parameter type with a defaulted template argument
{
	template<typename T, typename U = T>
	struct A
	{
	};

	template<typename T>
	bool g(A<T>);

	A<int> a;
	ASSERT_EXPRESSION_TYPE(g(a), bool); // calls 'g(A<int, int>)'
}

namespace N331 // test argument dependent lookup of friend function definition
{
	template<class T>
	struct A
	{
		friend T f(const T&);
	};

	struct B : A<B>
	{
	};

	B b;
	ASSERT_EXPRESSION_TYPE(f(b), B); // name lookup should find 'A<B>::f'
}

namespace N334 // test template argument deduction during argument dependent lookup of friend function template definition
{
	template<class T>
	struct A
	{
		template<typename U>
		friend U f(const U&);
	};

	struct B : A<B>
	{
	};

	B b;
	ASSERT_EXPRESSION_TYPE(f(b), B); // name lookup should find 'A<B>::f<B>'
}

namespace N332 // test overload resolution of friend function overloaded operator< against built-in operator
{
	template<class T, class U>
	struct B
	{
	private:
		friend int operator<(const U&, const T&);
	};
	struct A : B<A, int>
	{
		operator const int&()const;
		operator int&();
	};

	int i;
	A a;
	ASSERT_EXPRESSION_TYPE(i < a, int); // calls 'B<A>::operator<(const int&, const A&)' via argument dependent lookup - because it is a better candidate than built-in 'operator<(int, int)'
}

namespace N330 // test viability of built-in operator< called with an object convertible to built in type
{
	struct A
	{
		operator const int&()const;
		operator int&(); // implicit-object parameter is better conversion from 'A&' because no 'added qualification'
	};

	int i;
	A a;
	ASSERT_EXPRESSION_TYPE(i < a, bool); // calls built-in operator<(int, int), via conversion function 'A::operator int&()'
}

namespace N325 // test viability of built-in operator&& called with an object convertible to built in type
{
	struct A
	{
		bool operator()();
	};

	A a;

	ASSERT_EXPRESSION_TYPE(true && a(), bool);
}

namespace N324 // test viability of built-in operator!= called with an object convertible to built in type
{
	struct A
	{
		operator int*();
	};

	A a;

	ASSERT_EXPRESSION_TYPE(a != 0, bool);
}


//-----------------------------------------------------------------------------
// explicit specialization

namespace N157 // test choice of correct explicit specialization by function type
{
	template<class R>
	struct S
	{
		typedef int Primary;
	};
	template<>
	struct S<void()>
	{
		typedef int Spec1;
	};
	template<>
	struct S<void(...)>
	{
		typedef int Spec2;
	};

	S<int>::Primary p;
	S<void()>::Spec1 s1;
	S<void(...)>::Spec2 s2;
}


//-----------------------------------------------------------------------------
// name lookup for friend declaration

namespace N376 // test name lookup for namespace-scope redeclaration of function first declared as friend
{
	struct A
	{
		friend int f();
	};

	int f(); // redeclares 'f'

	int c = f(); // unqualified name lookup finds 'f'
}

namespace N377 // test name lookup for namespace-scope declaration of function redeclared as friend
{
	int f();

	struct A
	{
		friend int f(); // redeclares 'f'
	};

	int c = f(); // unqualified name lookup finds 'f'
}

namespace N379 // test name lookup for namespace-scope declaration of class redeclared as friend
{
	struct B;

	struct A
	{
		friend struct B; // redeclares 'B'
	};

	B* b; // unqualified name lookup finds 'B'
}

namespace N378 // test name lookup for namespace-scope redeclaration of class first declared as friend
{
	struct A
	{
		friend struct B;
	};

	struct B; // redeclares 'B'

	B* b; // unqualified name lookup finds 'B'
}

namespace N329 // test name lookup for overloaded operator declared as friend
{
	template<class T>
	struct A
	{
		friend int operator==(const T&x, const T&y);
	};

	enum E
	{
	};

	E e;
	ASSERT_EXPRESSION_TYPE(e == e, bool); // name lookup should not find 'A<T>::operator=='
}

//-----------------------------------------------------------------------------
// argument dependent name lookup

namespace N380 // test argument dependent name lookup for function declared only as friend
{
	struct A
	{
		friend int f(A); // redeclares 'f'
	};

	int c = f(A()); // argument dependent name lookup finds 'f'
}

namespace N321 // test argument dependent name lookup for function declared in nested namespace
{
	namespace N
	{
		struct A
		{
		};
		int f(A);
	}

	N::A a;
	int x = f(a); // lookup of unqualified id 'f' should be deferred until arguments are known, then looked up via ADL
}

//-----------------------------------------------------------------------------
// explicit template argument specification

namespace N257 // test parse of explicit template argument specification in call to 'operator()'
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

namespace N347 // test interaction of explicit template argument specification and template argument deduction
{
	template<typename First, typename Second>
	struct C
	{
	};

	template<typename First, typename Second>
	C<First, Second> f(const Second& src);

	struct A
	{
	};

	struct B
	{
	};

	A a;
	B b;

	typedef C<B, A> Expected;

	// first template argument is explicitly specified as 'B'
	ASSERT_EXPRESSION_TYPE(f<B>(a), Expected); // second template argument is deduced as 'A'
}

//-----------------------------------------------------------------------------
// class template partial specialization

namespace N381 // test selection of correct specialization when both template arguments are the same type
{
	template<typename Target, typename Src>
	struct C
	{
		typedef Target type;
	};
	template<typename Src>
	struct C<Src, Src>
	{
		typedef const Src& type;
	};

	typedef C<int, float> Different;
	typedef C<int, int> Same;

	STATIC_ASSERT_IS_SAME(Different::type, int);
	STATIC_ASSERT_IS_SAME(Same::type, const int&);
}


//-----------------------------------------------------------------------------
// type of class-member-access

namespace N345 // test call of pointer to function named by class-member-access
{
	struct A
	{
		A* f()
		{
			ASSERT_EXPRESSION_TYPE(f()->m(), void);
		}
		void(*m)();
	};
}

//-----------------------------------------------------------------------------
// type of expression involving pointer-to-member

namespace N337 // check type of function-call expression with class-member-access expression with pointer-to-member as object expression
{
	struct A
	{
	};

	struct B
	{
		int (A::* m)();
	};

	A a;
	B b;
	ASSERT_EXPRESSION_TYPE((a.*b.m)(), int); // not lvalue
}

namespace N336 // check type of class-member-access expression with pointer-to-member as object expression
{
	struct A
	{
	};

	struct B
	{
		int A::* m;
	};

	A a;
	B b;
	ASSERT_EXPRESSION_TYPE(a.*b.m, int&); // lvalue
	ASSERT_EXPRESSION_TYPE(A().*b.m, int); // not lvalue
}

namespace N335 // check type of pointer-to-member id-expression
{
	struct A
	{
		int m;
	};

	int A::* m = &A::m;

	A a;
	ASSERT_EXPRESSION_TYPE(a.*m, int&); // lvalue
	ASSERT_EXPRESSION_TYPE((&a)->*m, int&); // lvalue
	ASSERT_EXPRESSION_TYPE(A().*m, int); // not lvalue
}

namespace N328 // test SFINAE to remove overload candidate with enable_if in function call
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

	template< class T >
	struct is_int { static const bool value = false; };

	template<>
	struct is_int<int>
	{
		static const bool value=true;

	};

	template<class T>
	int f(T, typename enable_if<is_int<T> >::type* = 0); // SFINAE

	bool f(float);

	ASSERT_EXPRESSION_TYPE(f(false), bool); // calls 'f(float)' because argument is not int
	ASSERT_EXPRESSION_TYPE(f(0), int); // calls 'f<int>(int)'
}


namespace N297 // test template argument deduction with template base class parameter
{
	template<typename T>
	struct C
	{
	};
	template<typename T>
	struct B : public C<T>
	{
	};
	struct A : public B<A>
	{
	};

	template<typename T>
	T operator*(const C<T>&);

	A a;
	ASSERT_EXPRESSION_TYPE(*a, A); // calls operator*(const C<A>)
}


namespace N279 // test template argument deduction with argument types that both decay to pointer
{
	template<class T>
	T f(T, T);

	char buffer[1];
	ASSERT_EXPRESSION_TYPE(f(buffer, buffer+1), char*); // calls f<char*>(char*, char*)
}


namespace N276 // test dereference of pointer to function before calling it
{
	typedef int(*F)();

	F p;
	ASSERT_EXPRESSION_TYPE((*p)(), int);
}

namespace N280 // test overload resolution with enum promotion to int
{
	enum E
	{
	};

	int f(int);
	bool f(unsigned int);

	ASSERT_EXPRESSION_TYPE(f(E(0)), int); // f(int) is a better match
}

namespace N275 // test template argument deduction with parameter T* and function-pointer argument
{
	template<class T>
	inline T* f(T*);

	typedef void(*F)();

	F p;
	ASSERT_EXPRESSION_TYPE(f(p), F); // calls f<void(*)()>(void(*)())
}

namespace N273 // test call of overloaded operator[]
{
	struct A
	{
		int operator[](int);
	};

	A a;
	ASSERT_EXPRESSION_TYPE(a[0], int);
}

namespace N272 // test call of function with ellipsis parameter list
{
	int f(...);

	ASSERT_EXPRESSION_TYPE(f(1), int);
}


namespace N264 // test partial ordering of function templates
{
	template<typename T>
	int f(const T*);

	template<typename T>
	bool f(T*);

	const char* p;
	ASSERT_EXPRESSION_TYPE(f(p), int); // calls f<char>(const char*)
}


namespace N234 // test SFINAE with explicitly specified template argument for function template overloads with both type and non-type parameter.
{
	template<int i>
	int f();

	template<typename T>
	T f();

	ASSERT_EXPRESSION_TYPE(f<bool>(), bool);
	ASSERT_EXPRESSION_TYPE(f<0>(), int);
}

namespace N123 // test type substitution for return type that depends on a template parameter
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
	ASSERT_EXPRESSION_TYPE(s.f(), int);
}


namespace N235 // test template argument deduction with TT<T> parameter, with explicitly specified template argument for T
{
	template<typename T, template<class> class TT>
	TT<T> f(TT<T>);

	template<class T>
	struct S
	{
	};

	S<int> s;
	ASSERT_EXPRESSION_TYPE(f<int>(s), S<int>);
}

namespace N177 // test template argument deduction with TT<int> parameter
{
	template<template<class> class TT>
	TT<int> f(TT<int>);

	template<class T>
	struct S
	{
	};

	S<int> s;
	ASSERT_EXPRESSION_TYPE(f(s), S<int>);
}

namespace N233 // test implicit conversion of null-pointer-constant to pointer in function call expression
{
	int f(char*);
	ASSERT_EXPRESSION_TYPE(f(0), int); // null-pointer-constant matches T*
}

namespace N154 // test template argument deduction with T parameter
{
	template<typename T>
	T f();

	ASSERT_EXPRESSION_TYPE(f<int>(), int); // calls 'f<int>()'
};


namespace N124 // test type substitution in function call expression for function with dependent type
{
	struct S
	{
		typedef int Type;
	};

	template<typename T>
	typename T::Type f(T);

	S s;
	ASSERT_EXPRESSION_TYPE(f(s), int); // return type is 'int'
}

namespace N127 // test type substitution in function call expression for member function which depends on a template parameter of the enclosing class template
{
	template<typename T>
	struct S
	{
		static T f(T t)
		{
			return t;
		}
	};

	ASSERT_EXPRESSION_TYPE(S<int>::f(0), int); // type of 'S<int>::f' should be 'int(int)'
}


namespace N112 // test type substitution in function call expression for member function which indirectly depends on a template parameter of the enclosing class template
{
	template<typename T>
	struct S
	{
		typedef typename T::Type Type;
		int f(Type);
	};

	struct A
	{
		typedef int Type;
	};

	S<A> s;
	ASSERT_EXPRESSION_TYPE(s.f(0), int); // calls S::f(int)
}

namespace N104
{
	int const f(); // const is ignored
	ASSERT_EXPRESSION_TYPE(f, int const()); // const is also ignored here!
}

namespace N103
{
	int const f(); // const is ignored
	ASSERT_EXPRESSION_TYPE(f(), int);
}

namespace N102
{
	const int& f();
	ASSERT_EXPRESSION_TYPE(f(), const int&);
}


namespace N100 // test using declaration
{
	namespace A
	{
		int f(int);
	}

	namespace B
	{
		using A::f;
		float f(float);
		ASSERT_EXPRESSION_TYPE(f(0), int); // overload resolution should choose A::f(int)
	}
}


namespace N089
{
	void f(bool, bool = false);
	int f(int = 0);

	ASSERT_EXPRESSION_TYPE(f(0), int); // should call f(int)
}

namespace N094
{
	void f(float, ...);
	int f(float, float);
	void f(int, float);
	ASSERT_EXPRESSION_TYPE(f(0.f, 0.f), int); // overload resolution should choose f(float, float)
}

namespace N093
{
	void f(float, float);
	int f(float, int = 37);
	void f(int);
	ASSERT_EXPRESSION_TYPE(f(0.f), int); // overload resolution should choose f(float, int)
}

namespace N088
{
	int f(const wchar_t*);
	void f();
	ASSERT_EXPRESSION_TYPE(f(L""), int); /// overload resolution should pick 'f(const wchar_t*)'
}
