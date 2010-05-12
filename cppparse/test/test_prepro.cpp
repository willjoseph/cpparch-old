
namespace boost
{
	namespace spirit
	{
		namespace iterator_policies
		{
			template<typename Input, typename Storage>
			struct multi_pass_shared: Storage
			{
			};
			template<typename Input>
			struct default_policy
			{
				template<typename T>
				struct shared : multi_pass_shared<
					typename Input::template shared<T>,
					typename Input::template shared<typename Input::template unique<T>::value_type>
				>
				{
					typedef typename Input::template shared<typename Input::template unique<T>::value_type> 
						storage_policy;
				};
			};
		}
	}
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
