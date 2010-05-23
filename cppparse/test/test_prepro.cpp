
namespace boost
{
	template <class Tag,class T>
	class error_info;

	typedef error_info<struct throw_function_,char const *> throw_function;

    template <>
    class
    error_info<throw_function_,char const *>
    {
        public:
        typedef char const * value_type;
        value_type v_;
        explicit error_info( value_type v ) : v_(v)
		{
        }
	};

	template<class E, class Tag, class T>
	E const&operator<<(E const&, error_info<Tag, T>const&);

	class exception
	{
		virtual~exception()throw()=0;
		template<class E>
		friend E const&operator<<(E const&x, throw_function const&y)
		{
			x.throw_function_=y.v_;
			return x;
		}
	};
}

namespace detail
{ 
	template <class Model, void (Model::*)()>
	struct wrap_constraints {};
}

template<class T>
class Template1
{
	friend class Template1;
	typedef Template1<int> Type;
};

#if 0
namespace exception_detail
{
	template<class>
	struct get_info;
}

class exception
{
    template<class>
    friend struct exception_detail::get_info;
};
#endif

int wmemcmp(const wchar_t *_S1, const wchar_t *_S2, int _N)
{
	 return (*_S1 < *_S2 ? -1 : +1);
}

template<typename T = int>
class DummyTmpl
{
};


template<template<typename X> class T = ::DummyTmpl>
class TestTemplateTemplateParam
{
};

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
