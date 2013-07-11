
#if 0
// deferred name lookup
namespace N504
{
	template<class T>
	struct Iterator
	{
		typedef typename T::Ref Ref; // cannot be looked up without instantiating

		Ref operator*()const
		{
		}
	};

	struct Value
	{
		int v;
		typedef Value& Ref;
	};

	void f()
	{
		Iterator<Value> i;
		(*i).v=0;
	}
}
#endif


namespace N231
{
	template<typename T>
	struct S
	{
	};

	template<>
	struct S<wchar_t>
	{
		static int eof();
		static int not_eof(const int&_Meta)
		{
			return (_Meta!=eof()?_Meta: !eof());
		}
	};
}

namespace N242
{
	struct S
	{
		operator int**()
		{
			return 0;
		}
	};
	void f(S& s)
	{
		s.operator int**();
	}
}

namespace N507
{
	struct S
	{
		struct Type
		{
		};
		operator Type()
		{
			return Type();
		}
	};
	void f(S& s)
	{
		s.operator Type(); // Type should be looked up in context of S
	}
}

namespace N508
{
	struct Type
	{
	};

	struct S
	{
		operator Type()
		{
			return Type();
		}
	};
	void f(S& s)
	{
		s.operator Type(); // Type should be looked up in context of entire postfix-expression
	}
}


namespace N241
{
	class locale;
	template<class _Facet>
	const _Facet& use_facet(const locale&);
	template<class _Elem>
	class ctype
	{
	};

	template<class _Elem>
	inline _Elem(tolower)(_Elem _Ch, const locale&_Loc)
	{
		return (use_facet<_Elem>(_Loc).tolower(_Ch)); // an explicit template argument list in a function call causes the expression to be dependent
	}
}



namespace N001
{
	struct M
	{
		template<int i>
		float dependent(int j)
		{
			return 0;
		}
	};

	template<typename A>
	struct C
	{
		typedef M Type;
	};

	template<typename T1>
	struct O0
	{
		typename T1::Type f()
		{
			return typename T1::Type();
		}
	};

	void f(int)
	{
	}

	void f()
	{
		O0< C<int> > o;
		o.f().dependent<0>(0); // type of expression should resolve to 'float'
	}
}


namespace N049
{
	template<typename T>
	struct Tmpl
	{
	};

	template<>
	struct Tmpl<int>
	{
		template<typename T>
		void f(); // declaration
	};

	// omitting optional 'template<>'
	template<typename T>
	void Tmpl<int>::f() // definition
	{
	}
}


namespace N096
{
	template<typename T>
	struct S
	{
		void f(T); // declaration
	};

	template<typename T>
	inline void S<T>::f(T) // definition of previous declaration
	{
	}

	void f()
	{
		S<int> s;
		s.f(0);
	}
}

namespace N234
{
	template<int i>
	int f()
	{
		return 0;
	}

	template<typename T>
	typename T::Result f()
	{
		return 0;
	}

	template<typename T>
	struct S
	{
		typedef int Result;
	};

	int a = f<S<int> >();
	int b = f<0>(); // SFINAE during overload resolution
}

namespace N123
{
	template<typename T>
	struct S
	{
		typedef T Type;

		Type f();
	};

	template<typename U>
	typename S<U>::Type S<U>::f()
	{
	}

	S<int> s;
	int i = s.f(); // return type is 'int'
}


namespace N240
{
	template<class A>
	struct S
	{
		template<class T>
		struct Inner;
	};


	template<>
	template<class T>
	struct S<int>::Inner // 'Inner' is a template
	{
		void f();
	};

	template<>
	template<class T>
	void S<int>::Inner<T>::f() // 'f' is not a template
	{
	}

	template<>
	template<>
	struct S<int>::Inner<int> // 'Inner' is a template specialization
	{
		void f();
	};

	void S<int>::Inner<int>::f() // 'f' is not a template
	{
	}

}

namespace N236
{
	template<class T>
	struct S
	{
		struct Inner;
	};

	template<class T>
	struct S<T>::Inner // 'Inner' is not a template
	{
	};
}

namespace N239
{
	template<class T>
	struct S
	{
		void f();
	};

	template<class T>
	void S<T>::f() // 'inner' is not a template
	{
	}
}

namespace N238
{
	struct S
	{
		template<class T>
		void f();
	};

	template<class T>
	void S::f() // 'inner' is a template
	{
	}
}

namespace N237
{
	struct S
	{
		template<class T>
		struct Inner;
	};

	template<class T>
	struct S::Inner // 'Inner' is a template
	{
	};
}


namespace N152
{
	template<int x>
	struct I;

	template<typename T>
	struct S
	{
		static int f(int);
		static T f(const char*);

		// [temp.dep.expr] 'f' is dependent because it is an identifier that was declared with a dependent type
		static const int value = I<sizeof(f(3))>::dependent;
		static const int x = sizeof(f(3).x);
	};
}

namespace N235
{
	template<typename T, template<class> class TT>
	void f(TT<T>)
	{
	}

	template<class T>
	struct S
	{
	};

	void g()
	{
		S<int> s;
		f<int>(s);
	}
}

namespace N177
{
	template<template<class> class TT>
	void f(TT<int>)
	{
	}

	template<class T>
	struct S
	{
	};

	void g()
	{
		S<int> s;
		f(s);
	}
}

namespace N233
{
	void f(char*)
	{
		f(0); // null-pointer-constant matches T*
	}
}

namespace N232
{
	template<typename T>
	struct S
	{
		T m;
		void f()
		{
			m.dependent();
			m->dependent();
			m.Dependent::dependent();
		}
	};
}

namespace N222
{
	template<typename T>
	struct B : T
	{
	};

	struct Outer
	{
		template <typename T>
		struct Inner;
	};

	template<typename T>
	struct Outer::Inner : B<T>
	{
		void f();
	};

	template<typename T>
	void Outer::Inner<T>::f()
	{
		this->dependent(); // 'this' is dependent
	}
}


namespace N230
{
	template<typename T>
	struct S
	{
		S f();
	};

	typedef S<int> Type;

	inline Type f(Type& p)
	{
		return p.f();
	}
}



namespace N223
{
	template<int i>
	struct S;
	template<>
	struct S<1>;
	template<>
	struct S<2>;
	template<>
	struct S<3>;
}

namespace N229
{
#if 0
	template<typename T>
	struct S
	{
		template<typename U>
		int f(U);
	};

	template<typename T>
	template<typename U>
	int S<T>::f(U)
	{
		return 0;
	}

	S<int> s;
	int i = s.f(0); // instantiates S<int> and S<int>::f<int>
#endif
}

namespace N228
{
	template<typename T>
	struct B
	{
		typedef typename T::Before Before; // ok
		typedef typename T::Instantiate Instantiate; // ok
		typedef typename T::After After; // ok
	};

	struct A
	{
		typedef int Before;
		static B<A> Instantiate;
		typedef int After;
	};
}

namespace N227
{
	template<typename T>
	struct B
	{
		static const int value = T::value; // ok
		//static const int instantiate = T::instantiate; // error: initializer is not a constant expression
		//static const int after = T::after; // error: no member named 'after' in 'A<int>'
	};

	template<typename T>
	struct A
	{
		static const int value = 0;
		static const int instantiate = B<A>::value;
		static const int after = 0;
	};

	static const int value = A<int>::instantiate; // instantiate A<int>
}

namespace N226
{
	template<void(*op)()>
	struct S
	{
		static int thunk();
	};

	inline void f();

	int i = S<f>::thunk();
}

namespace N225
{
	template<typename T>
	struct B
	{
		typedef typename T::Before Before; // ok
		//typedef typename T::After After; // error: no type named 'After' in 'A'
	};

	struct A
	{
		typedef int Before;
		typedef B<A>::Before Instantiate;
		typedef int After;
	};
}

namespace N224
{
	template<typename T>
	struct B
	{
		typedef typename T::Before Before; // ok
		//typedef typename T::After After; // error: no type named 'After' in 'A<int>'
	};

	template<typename T>
	struct A
	{
		typedef int Before;
		typedef typename B<A>::Before Instantiate;
		typedef int After;
	};

	typedef A<int>::Instantiate Type; // instantiate A<int>
}

namespace N221
{
	bool b1 = __is_class(int);
	bool b2 = __is_base_of(int, int);
}


#if 0
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
