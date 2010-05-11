
template<typename T>
struct Bool
{
	enum { value = false };
};

struct S
{
	template<bool b = Bool<int>::value>
	class M;
};

void f()
{
	try
	{
	}
	catch(int e)
	{
		e = 0;
	}
	catch(float e)
	{
		e = 0;
	}
}

class C
{
	void f()
	{
		operator=(*this);
	}
};


#if 0
#include <boost/integer_fwd.hpp>
#endif

#if 0
#include <boost/wave/util/interpret_pragma.hpp>
#endif

#if 0
#include <boost/spirit/home/classic/tree/common.hpp>
#endif
