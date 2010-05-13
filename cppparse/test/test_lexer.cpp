

struct _Char_traits_base
{
};

template<class _Elem>
struct char_traits:
	public _Char_traits_base
{
};

template<class _Ty>
struct _Allocator_base
{
};

template<class _Ty>
struct _Allocator_base<const _Ty>
{
};

template<class _Ty>
struct allocator: public _Allocator_base<_Ty>
{
	template<class _Other>
	struct rebind
	{
		typedef allocator<_Other>other;
	};
};

template<class _Ty,
class _Alloc>
struct _String_val
{
	typedef typename _Alloc::template rebind<_Ty>::other _Alty;
};

template<class _Ty,
class _Diff,
class _Pointer,
class _Reference,
class _Base_class>
struct _Ranit_base
	: public _Base_class

{
};

class _Iterator_base_secure
{
};

template<class _Elem,
class _Traits,
class _Alloc>
class _String_const_iterator
	: public _Ranit_base<_Elem, typename _Alloc::difference_type,
	typename _Alloc::const_pointer, typename _Alloc::const_reference, _Iterator_base_secure>
{
};

template<class _Elem,
class _Traits,
class _Alloc>
class _String_iterator
	: public _String_const_iterator<_Elem, _Traits, _Alloc>
{
};

template<class _Elem,
class _Traits,
class _Ax>
struct basic_string
	: public _String_val<_Elem, _Ax>
{
	typedef _String_val<_Elem, _Ax> _Mybase;
	typedef typename _Mybase::_Alty _Alloc;
	typedef _String_iterator<_Elem, _Traits, _Alloc> iterator;
};

typedef basic_string<char, char_traits<char>, allocator<char> > string;

template <typename E,
class T,
class A,
class Storage>
class flex_string : private Storage
{
};

template <typename E, class A = allocator<E> >
struct AllocatorStringStorage : public A
{
	typedef E value_type;
};

template <
typename Storage, 
typename Align = typename Storage::value_type*
>
class CowString
{
};

template <typename StringT>
struct file_position {
};

typedef file_position<flex_string<char, char_traits<char>, allocator<char>, CowString<AllocatorStringStorage<char> > > >file_position_type;


template <typename PositionT>
class lex_token 
{
};

template <typename TokenT>
struct lex_input_interface 
{
};

template<typename TokenT>
struct lex_iterator_functor_shim
{
	typedef TokenT result_type;
	typedef lex_iterator_functor_shim unique;
	typedef lex_input_interface<TokenT>*shared;
};

struct split_functor_input
{
	template<typename Functor>
	class unique
	{
	protected: 
		typedef typename Functor::first_type functor_type;
		typedef typename functor_type::result_type result_type;
	public:
		typedef result_type value_type;
	};
};

struct ref_counted
{
	struct unique
	{
	};
	struct shared
	{
	};
};
struct no_check
{
	struct unique
	{
	};
	struct shared
	{
	};
};
struct split_std_deque
{
	template<typename Value>
	class unique
	{
	};
	template<typename Value>
	struct shared
	{
	};
};

template<typename T, typename Ownership, typename Checking, typename Input, typename Storage>
struct multi_pass_shared: Ownership, Checking, Input, Storage
{
};

template <typename T, typename Ownership, typename Checking, 
typename Input, typename Storage>
struct multi_pass_unique : Ownership, Checking, Input, Storage
{
};

template<typename Ownership, typename Checking, typename Input, typename Storage>
struct default_policy
{
	typedef Storage storage_policy;
	template<typename T>
	struct unique: multi_pass_unique<
		T,
		typename Ownership::unique,
		typename Checking::unique,
		typename Input::template unique<T>,
		typename Storage::template unique<typename Input::template unique<T>::value_type>
	>
	{
#if 0
		typedef typename Ownership::unique ownership_policy;
		typedef typename Checking::unique checking_policy;
		typedef typename Input::template unique<T>input_policy;
		typedef typename Storage::template unique<typename input_policy::value_type>storage_policy;
		typedef multi_pass_unique<T, ownership_policy, checking_policy, input_policy, storage_policy>unique_base_type;
#endif
	};
	template<typename T>
	struct shared: multi_pass_shared<
		T,
		typename Ownership::shared,
		typename Checking::shared,
		typename Input::template shared<T>,
		typename Storage::template shared<typename Input::template unique<T>::value_type>
	>
	{
#if 0
		typedef typename Ownership::shared ownership_policy;
		typedef typename Checking::shared checking_policy;
		typedef typename Input::template shared<T>input_policy;
		typedef typename Storage::template shared<typename Input::template unique<T>::value_type>storage_policy;
		typedef multi_pass_shared<T, ownership_policy, checking_policy, input_policy, storage_policy>shared_base_type;
#endif
	};
};

template < typename MemberType, int UniqueID = 0 >
class base_from_member
{
};

template<typename T, typename Policies>
class multi_pass : private base_from_member<typename Policies::template shared<T>*>, public Policies::template unique<T>
{
};

template<class _T1, class _T2>
struct pair
{
	typedef _T1 first_type;
	typedef _T2 second_type;
};

template<typename FunctorData>
struct make_multi_pass
{
	typedef pair<typename FunctorData::unique, typename FunctorData::shared> functor_data_type;
	typedef split_functor_input input_policy;
	typedef ref_counted ownership_policy;
	typedef no_check check_policy;
	typedef split_std_deque storage_policy;
	typedef default_policy<ownership_policy, check_policy, input_policy, storage_policy> policy_type;
	typedef multi_pass<functor_data_type, policy_type> type;
};


template <typename TokenT>
struct lex_iterator 
	:   public make_multi_pass<lex_iterator_functor_shim<TokenT> >::type
{
	typedef TokenT token_type;
};

struct load_file_to_string
{
};

struct default_preprocessing_hooks 
{
};

template <typename TokenT>
class eat_whitespace 
	:   public default_preprocessing_hooks
{
};

typedef lex_token<file_position_type> token_type;
typedef lex_iterator<token_type> lex_iterator_type;
typedef load_file_to_string input_policy_type;
class Hooks : eat_whitespace<token_type>
{
};

struct input_iterator_tag
{	// identifying tag for input iterators
};

struct output_iterator_tag
{	// identifying tag for output iterators
};

struct forward_iterator_tag
	: public input_iterator_tag
{	// identifying tag for forward iterators
};

struct _Iterator_base
{
};
template<class _Category,
class _Ty,
class _Diff = int,
class _Pointer = _Ty *,
class _Reference = _Ty&>
struct iterator
	: public _Iterator_base

{	// base type for all iterator classes
	typedef _Category iterator_category;
	typedef _Ty value_type;
	typedef _Diff difference_type;
	typedef _Diff distance_type;	// retained
	typedef _Pointer pointer;
	typedef _Reference reference;
};

template<class Category, class T, class Distance, class Pointer, class Reference>
struct iterator_base: iterator<Category, T, Distance, Pointer, Reference>
{
};

template<class Category, class T, class Distance=int, class Pointer=T*, class Reference=T&>
struct boost_iterator: iterator_base<Category, T, Distance, Pointer, Reference>
{
};

template <typename InputPolicyT, typename InputT>
struct iterator_base_creator
{
	typedef typename InputPolicyT::template inner<InputT>input_t;
	typedef boost_iterator<
		forward_iterator_tag,
		typename input_t::value_type,
		typename input_t::difference_type,
		typename input_t::pointer,
		typename input_t::reference
	>type;
};

class input_iterator
{
};

class buf_id_check
{
};

struct std_deque
{
	template <typename ValueT>
	class inner
	{
	};
};

template<typename InputT,
typename InputPolicy=input_iterator,
typename OwnershipPolicy=ref_counted,
typename CheckingPolicy=buf_id_check,
typename StoragePolicy=std_deque>
class classic_multi_pass
	: public OwnershipPolicy
	, public CheckingPolicy
	, public StoragePolicy::template inner< typename InputPolicy::template inner<InputT>::value_type>
	, public InputPolicy::template inner<InputT>
	, public iterator_base_creator<InputPolicy, InputT>::type
{
};

template <typename ContextT> 
struct pp_iterator_functor 
{
	typedef typename ContextT::token_type result_type;
};

struct functor_input
{
	template<typename FunctorT>
	class inner
	{
		typedef typename FunctorT::result_type result_type;
	public:
		typedef result_type value_type;
		typedef int difference_type;
		typedef result_type*pointer;
		typedef result_type&reference;
	};
};

template <typename ContextT>
class pp_iterator 
	:   public classic_multi_pass<
	pp_iterator_functor<ContextT>,
	functor_input
	>
{
};

struct this_type {};

class noncopyable
{
};

template <
typename IteratorT,
typename LexIteratorT, 
typename InputPolicyT,
typename HooksT,
typename DerivedT = this_type
>
struct context : private noncopyable
{
	typedef pp_iterator<context> iterator_type;
	typedef typename LexIteratorT::token_type token_type;
};

typedef context<string::iterator, lex_iterator_type, input_policy_type, Hooks> context_type;

struct LexIterator: public context_type::iterator_type
{
	LexIterator(const context_type::iterator_type& iterator)
		: context_type::iterator_type(iterator)
	{
	}
};
