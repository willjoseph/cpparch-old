
#if 1
namespace N1
{
	namespace impl
	{
		inline void g()
		{
		}
	}
}
namespace N2
{
	namespace impl
	{
	}
}
namespace N1
{
	void f()
	{
		using namespace N2;
		impl::g();
	}
}
#endif


#if 0
#include <boost/wave/util/interpret_pragma.hpp>
#endif

#if 0
#include <boost/spirit/home/classic/tree/common.hpp>
#endif
