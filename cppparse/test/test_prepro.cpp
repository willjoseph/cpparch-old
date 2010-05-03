
#if 0

namespace std
{
	typedef unsigned size_t;
}
namespace boost
{
	namespace mpl
	{
		template<bool C, typename T1, typename T2>
		struct if_c
		{
			typedef T1 type;
		};
		template<typename T1, typename T2>
		struct if_c<false, T1, T2>
		{
			typedef T2 type;
		};
	}
}
namespace boost
{
	namespace mpl
	{
		template<typename T, T N>
		struct integral_c
		{
			static const T value=N;
		};
		template<typename T, T N>
		T const integral_c<T, N>::value;

		template<bool C>
		struct integral_c<bool, C>
		{
			static const bool value=C;
		};
	}
}
namespace boost
{
	template<class T, T val>
	struct integral_constant: public mpl::integral_c<T, val>
	{
		typedef integral_constant<T, val>type;
	};
	template<>
	struct integral_constant<bool, true>
	{
		typedef integral_constant<bool, true>type;
	};
}

namespace boost
{
	template<typename T>
	struct alignment_of: integral_constant<std::size_t, 8>
	{
	};
}

namespace boost
{
	template<std::size_t Align>
	class type_with_alignment
	{
		typedef double type;
	};
	template<>
	class type_with_alignment<8>
	{
	   typedef mpl::if_c<
		  alignment_of<double>::value < 16,
		  double,
		  double >::type t1; 
	};
}

#endif

#if 0
#include <boost/type_traits/type_with_alignment.hpp>

#elif 0
namespace std
{
	typedef unsigned size_t;
}
namespace boost
{
	namespace mpl
	{
		template<bool C, typename T1, typename T2>
		struct if_c
		{
			typedef T1 type;
		};
		template<typename T1, typename T2>
		struct if_c<false, T1, T2>
		{
			typedef T2 type;
		};
	}
}
namespace boost
{
	namespace detail
	{
		template<std::size_t Align>
		struct type_with_alignment_imp
		{
			typedef int type;
		};
	}

	typedef mpl::if_c<
		false,
		detail::type_with_alignment_imp<8>,
		detail::type_with_alignment_imp<8> >::type t1; 

	typedef t1::type type;
}

#elif 0

namespace boost
{

	template<bool C, typename T1, typename T2>
	struct if_c
	{
		typedef T1 type;
	};
	template<typename T1, typename T2>
	struct if_c<false, T1, T2>
	{
		typedef T2 type;
	};

	template<class T, T val>
	struct integral_constant
	{
		enum { value = val };
	};
	template<>
	struct integral_constant<bool, true>
	{
	};
	template<>
	struct integral_constant<bool, false>
	{
	};


	template<typename T>
	struct alignment_of : integral_constant<int, 8>
	{
	};

	typedef if_c<
		alignment_of<double>::value < 8,
		double,
		double >::type t1; 
}

#endif
