
namespace stdX
{
	template<class _Fn>
	class pointer_to_unary_function
	{
	};

	inline void ptr_fun(int _Left)
	{
		(stdX::pointer_to_unary_function<int (*)(int)>(_Left));
	}
}


namespace N66
{
	class C* f(union U* (*)(struct S*));

	U* u;
	S* s;
	C* c;
}

namespace N15
{
	struct S
	{
		S(class D* a) : d((D*)(class C*)a)
		{
			C* c = (C*)0;
		}
		D* d;
	};
	D* d;
}

void f()
{
}

template<typename T>
void f()
{
	dependent1(T::dependent2());
}

extern void __cdecl operator delete(void *);

template<typename T>
struct S
{
	S(const char*, int);
};

const int i = 0;
const char* p = 0;

void f()
{
	const S<int> node(p, i); // bug causes 'p' to be recognised when ',' is encountered - recognition should be deferred until ';'
}

#if 0
class C
{
	// symbols may be deferred during attempt to parse 'void f(int i = j) {}'
	// first tried as 'void f(int i = j);', fails on reaching '{'
	void f(int i = j)
	{
	}

	static const int j = 0;
};
#endif

#if 0
template<typename T>
struct Blah
{
	typedef typename T::template X<T::value> Type fe;
};
#endif

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
