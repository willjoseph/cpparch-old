

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

#define STATIC_ASSERT(e) static_assert(e, "?evaluated")
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
	STATIC_ASSERT_IS_SAME(decltype(true), bool);
	STATIC_ASSERT_IS_SAME(decltype(false), bool);
	STATIC_ASSERT_IS_SAME(decltype(0), int);
	STATIC_ASSERT_IS_SAME(decltype(0.f), float);
	STATIC_ASSERT_IS_SAME(decltype(0.0), double);
	STATIC_ASSERT_IS_SAME(decltype(0l), long);
	// STATIC_ASSERT_IS_SAME(decltype(0ll), long long); // TODO: long long

	STATIC_ASSERT_IS_SAME(decltype(int()), int);

	// The type denoted by decltype(e) is defined as follows:
	//  - if e is an unparenthesized id-expression or an unparenthesized class member access (5.2.5), decltype(e)
	//  	is the type of the entity named by e. If there is no such entity, or if e names a set of overloaded functions,
	//  	the program is ill-formed;
	//  - otherwise, if e is an xvalue, decltype(e) is T&&, where T is the type of e;
	//  - otherwise, if e is an lvalue, decltype(e) is T&, where T is the type of e;
	//  - otherwise, decltype(e) is the type of e.

	struct A
	{
		int m;
	};

	int f();
	int i;
	const int ci = 0;
	A a;

	enum E { VALUE };

	STATIC_ASSERT_IS_SAME(decltype(i), int); // unparenthesized id-expression
	STATIC_ASSERT_IS_SAME(decltype(ci), const int); // unparenthesized id-expression
	STATIC_ASSERT_IS_SAME(decltype(f), int()); // unparenthesized id-expression (not overloaded)
	STATIC_ASSERT_IS_SAME(decltype(a.m), int); // unparenthesized class-member-access
	STATIC_ASSERT_IS_SAME(decltype(VALUE), E); // unparenthesized id-expression

	STATIC_ASSERT_IS_SAME(decltype((i)), int&); // yields a reference because 'i' is an lvalue
	STATIC_ASSERT_IS_SAME(decltype((ci)), const int&); // yields a reference because 'ci' is an lvalue
	STATIC_ASSERT_IS_SAME(decltype((f)), int(&)()); // yields a reference because 'f' is an lvalue
	STATIC_ASSERT_IS_SAME(decltype((a.m)), int&); // yields a reference because 'a.m' is an lvalue
	STATIC_ASSERT_IS_SAME(decltype((VALUE)), E); // does not yield a reference because 'VALUE' is not an lvalue

	STATIC_ASSERT_IS_SAME(decltype(f()), int); // not an lvalue
	STATIC_ASSERT_IS_SAME(decltype(A()), A);
	STATIC_ASSERT_IS_SAME(decltype(E()), E);
}


namespace N344 // non-dependent non-type-template-parameter
{
	template<bool b, bool func()>
	void f()
	{
		ASSERT_EXPRESSION_TYPE(b, bool); // type of 'b' is not dependent and can be resolved during initial parse
		ASSERT_EXPRESSION_TYPE(func(), bool); // type of 'func' is not dependent and can be resolved during initial parse
	}

	template<bool b, bool func()>
	struct A
	{
#ifdef _CPPP_TEST // msvc 2010 fails this test
		ASSERT_EXPRESSION_TYPE(b, bool); // type of 'b' is not dependent and can be resolved during initial parse
		ASSERT_EXPRESSION_TYPE(func(), bool); // type of 'func' is not dependent and can be resolved during initial parse
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

namespace N368 // test evaluation of type of conditional operator with null-pointer and function-pointer
{
	typedef void(*F)();
	void f();
	void g()
	{
		ASSERT_EXPRESSION_TYPE(false ? 0 : f, F); // 'f' is not overloaded, its type can be resolved to 'void()'
	}
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

	int i = g(f); // 'f' is overloaded, the correct overload 'f(int)' should be chosen
}

namespace N298 // test template argument deduction and return-type substitution in call to function-template 
{
	template<typename T>
	T f(const T*);

	int a;
	ASSERT_EXPRESSION_TYPE(f(&a), int); // calls f(const int*)
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

//-----------------------------------------------------------------------------
// name lookup for friend declaration

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
