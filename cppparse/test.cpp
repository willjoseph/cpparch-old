
#include "test.h"


#if 1

namespace N
{
	template<typename T>
	class C
	{
		template<typename X>
		void f()
		{
			N::undeclared<X> x;
		}
	};
}

struct S
{
};

template<typename First, typename Second>
int f(First x, Second y);

int x = f(S(), int());

template<typename Second, typename First>
int f(Second a, First b)
{
	return b;
}


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

	namespace detail
	{
		typedef int max_align;

		template<int Align>
		class type_with_alignment_imp
		{
		};
	};

	namespace align
	{
		struct a2{ short s; };
		struct a4{ int s; };
		struct a8{ double s; };
		struct a16{ long double s; };
	}

	template<typename T>
	struct alignment_of
	{
		enum { value = sizeof(T) };
	};

	template<int Align>
	class type_with_alignment
	{
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
}
#endif

