
namespace boost
{
	namespace multi_index
	{
		namespace detail
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
				static const int index=sizeof(void*)==sizeof(uintptr_candidates<0>::type)?0:
					sizeof(void*)==sizeof(uintptr_candidates<1>::type)?1:
					sizeof(void*)==sizeof(uintptr_candidates<2>::type)?2:
					sizeof(void*)==sizeof(uintptr_candidates<3>::type)?3:
					sizeof(void*)==sizeof(uintptr_candidates<4>::type)?4:-1;
			};
		}
	}
}

#if 0
namespace mpl
{
	struct na
	{
		typedef na type;
		enum { value = 0 };
	};

	template<typename Dummy=na>
	struct vector0;
	template<>
	struct vector0<na>
	{
		typedef vector0 type;
	};

	template<
		typename T0 = na, typename T1 = na, typename T2 = na, typename T3 = na
		, typename T4 = na, typename T5 = na, typename T6 = na, typename T7 = na
		, typename T8 = na, typename T9 = na, typename T10 = na, typename T11 = na
		, typename T12 = na, typename T13 = na, typename T14 = na
		, typename T15 = na, typename T16 = na, typename T17 = na
		, typename T18 = na, typename T19 = na
	>
	struct vector;
}
namespace mpl
{
	template<

	>
	struct vector<
		na, na, na, na, na, na, na, na, na, na, na, na, na, na, na, na, na
		, na, na, na
	>
	: vector0<  >
	{
		typedef vector0<  >::type type;
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
