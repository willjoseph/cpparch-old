
#ifndef INCLUDED_CPPPARSE_ALLOCATOR_H
#define INCLUDED_CPPPARSE_ALLOCATOR_H

#include <memory>

extern size_t gAllocatorFootprint;

template<class T>
class DebugAllocator : public std::allocator<T>
{
public:
	typedef std::allocator<T> Base;
	typedef typename Base::value_type value_type;
	typedef typename Base::pointer pointer;
	typedef typename Base::reference reference;
	typedef typename Base::const_pointer const_pointer;
	typedef typename Base::const_reference const_reference;

	typedef typename Base::size_type size_type;
	typedef typename Base::difference_type difference_type;

	template<class OtherT>
	struct rebind
	{
		typedef DebugAllocator<OtherT> other;
	};
	DebugAllocator()
	{
	}
	DebugAllocator(const DebugAllocator<T>&)
	{
	}
	template<class OtherT>
	DebugAllocator(const DebugAllocator<OtherT>&)
	{
	}
	template<class OtherT>
	DebugAllocator<T>& operator=(const DebugAllocator<OtherT>&)
	{
		return (*this);
	}

	void deallocate(pointer p, size_type count)
	{
		gAllocatorFootprint -= count * sizeof(T);
		return Base::deallocate(p, count);
	}

	pointer allocate(size_type count)
	{
		gAllocatorFootprint += count * sizeof(T);
		return Base::allocate(count);
	}

	pointer allocate(size_type count, const void* hint)
	{
		gAllocatorFootprint += count * sizeof(T);
		return Base::allocate(count, hint);
	}
};

template<class T,
class OtherT> inline
	bool operator==(const DebugAllocator<T>&, const DebugAllocator<OtherT>&)
{
	return (true);
}

template<class T,
class OtherT> inline
	bool operator!=(const DebugAllocator<T>&, const DebugAllocator<OtherT>&)
{
	return (false);
}

#endif


