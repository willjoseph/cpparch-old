

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

namespace TestTypes
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

namespace TestDecltype
{
	STATIC_ASSERT_IS_SAME(decltype(true), bool);
	STATIC_ASSERT_IS_SAME(decltype(false), bool);
	STATIC_ASSERT_IS_SAME(decltype(0), int);
	STATIC_ASSERT_IS_SAME(decltype(0.f), float);
	STATIC_ASSERT_IS_SAME(decltype(0.0), double);
	STATIC_ASSERT_IS_SAME(decltype(0l), long);
	// STATIC_ASSERT_IS_SAME(decltype(0ll), long long); // TODO: long long
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

