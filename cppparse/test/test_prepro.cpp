
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


#endif
