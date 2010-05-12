
namespace N84
{
	template<typename T>
	struct Base
	{
	};
	template<typename T>
	struct C
	{
		struct Derived : Base<int>
		{
			typedef typename T::X Y;
		};
	};
}

#if 0
#include "boost\spirit\home\support\iterators\detail\combine_policies.hpp"
#endif

#if 0
#include <boost/integer_fwd.hpp>
#endif

#if 0
#include <boost/wave/util/interpret_pragma.hpp>
#endif

#if 0
#include <boost/spirit/home/classic/tree/common.hpp>
#endif
