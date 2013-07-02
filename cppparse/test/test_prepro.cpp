
namespace N220
{
	struct true_;
	struct false_;

	template<typename T, template<typename P1> class F>
	struct S
	{
		typedef int Primary;
	};

	template<template<typename P1> class F>
	struct S<true_, F>
	{
		typedef int Special;
	};

	template<typename T>
	struct A
	{
	};

	typedef S<true_, A>::Special Special;
	typedef S<false_, A>::Primary Primary;
}

namespace N219
{
	template<typename T>
	struct A
	{
	};

	template<template<class> class TT>
	struct C
	{
	};

	template<template<class> class TT>
	struct B : C<TT> // TT as a template-template argument
	{
		typedef int Special;
	};

	typedef B<A>::Special Special;
}

namespace N218
{
	template<typename T>
	struct A
	{
	};

	template<template<class> class TT>
	struct C
	{
	};

	template<typename T>
	struct B
	{
		typedef int Primary;
	};

	template<template<class> class TT>
	struct B<TT<int> > : C<TT> // TT as a template-template argument
	{
		typedef int Special;
	};

	typedef B<A<int> >::Special Special;
}

#if 0 // TODO: handle defaults 
namespace N215
{
	template<typename T>
	struct A
	{
	};

	template<typename F>
	struct B
	{
		typedef int Primary;
	};

	template<template<typename P1 = int>class F>
	struct B<F<> >
	{
		typedef int Special;
	};

	typedef B<A<int> >::Special Special;
}
#endif

#if 0 // bit suspect?
namespace N216
{
	template<typename T>
	struct A
	{
	};

	template<typename F, typename G = int>
	struct B
	{
		typedef int Primary;
	};

	template<class F>
	struct B<F>
	{
		typedef int Special;
	};

	typedef B<A<int> >::Special Special;
}
#endif

#include <boost/mpl/lambda.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_same.hpp>

using namespace boost::mpl;

apply_wrap1<lambda<identity<_1> >::type, int>::type i = 0;
