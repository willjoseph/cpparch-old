

bool b1 = __is_class(int);
bool b2 = __is_base_of(int, int);

#if 1
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>

#if 0
namespace boost
{
	namespace multi_index
	{
		namespace detail
		{
			struct index_applier
			{
				template<typename IndexSpecifierMeta, typename SuperMeta>
				struct apply
				{
					typedef typename IndexSpecifierMeta::type index_specifier;
					typedef typename index_specifier::template index_class<SuperMeta>::type type;
				};
			};
			template<int N, typename Value, typename IndexSpecifierList, typename Allocator>
			struct nth_layer
			{
				static const int length=mpl::size<IndexSpecifierList>::value;
				typedef typename mpl::eval_if_c<N==length, mpl::identity<index_base<Value, IndexSpecifierList, Allocator> >, mpl::apply2<index_applier, mpl::at_c<IndexSpecifierList, N>, nth_layer<N+1, Value, IndexSpecifierList, Allocator> > >::type type;
			};
			template<typename Value, typename IndexSpecifierList, typename Allocator>
			struct multi_index_base_type: nth_layer<0, Value, IndexSpecifierList, Allocator>
			{
				typedef ::boost::static_assert_test<sizeof(::boost::STATIC_ASSERTION_FAILURE<(bool)(detail::is_index_list<IndexSpecifierList>::value)>)>boost_static_assert_typedef___COUNTER__;
			};
		}
	}
}
#endif

namespace boost
{
	namespace wave
	{
		namespace util
		{
			struct from
			{
			};
			struct to
			{
			};
			struct pair
			{
				int first;
				int second;
			};

			typedef pair value_type;
			typedef int FromType;
			typedef int ToType;

			typedef value_type Value;
			typedef boost::multi_index::indexed_by<boost::multi_index::ordered_unique<boost::multi_index::tag<from>, boost::multi_index::member<value_type, FromType, &value_type::first> >, boost::multi_index::ordered_non_unique<boost::multi_index::tag<to>, boost::multi_index::member<value_type, ToType, &value_type::second> > > IndexSpecifierList;
			typedef std::allocator<Value> Allocator;

			static const int N = 0;

			static const int length=mpl::size<IndexSpecifierList>::value;
			typedef mpl::eval_if_c<N==length,
				mpl::identity<boost::multi_index::detail::index_base<Value, IndexSpecifierList, Allocator> >,
				mpl::apply2<boost::multi_index::detail::index_applier, mpl::at_c<IndexSpecifierList, N>, boost::multi_index::detail::nth_layer<N+1, Value, IndexSpecifierList, Allocator> >
			>::type tmp;

			typedef boost::multi_index::multi_index_container<value_type, IndexSpecifierList>::value_type type;
		}
	}
}
#endif

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
