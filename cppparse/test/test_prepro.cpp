

template<typename T>
struct Tmpl
{
};

struct C
{
	template<typename T>
	struct S;
};
template<typename T>
struct C::S
	: public Tmpl< C::S<T> >
{
};

#if 1
#include <boost/spirit/home/classic/tree/common.hpp>
#endif
