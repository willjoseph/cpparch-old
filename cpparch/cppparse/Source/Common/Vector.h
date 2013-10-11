
#ifndef INCLUDED_CPPPARSE_COMMON_VECTOR_H
#define INCLUDED_CPPPARSE_COMMON_VECTOR_H

#include "Allocator.h"



template<typename T, typename A>
inline void constructObject(const A& a, T* p, const T& value)
{
	typename A::template rebind<T>::other tmp(a);
	tmp.construct(p, value);
}

template<typename T, typename A>
inline void destroyObject(const A& a, T* p)
{
	typename A::template rebind<T>::other tmp(a);
	tmp.destroy(p);
}

template<typename T, typename A>
inline void constructArray(const A& a, T* first, T* last)
{
	typename A::template rebind<T>::other tmp(a);
	for(; first != last; ++first)
	{
		tmp.construct(first, T());
	}
}

template<typename T, typename A>
inline void destroyArray(const A& a, T* first, T* last)
{
	typename A::template rebind<T>::other tmp(a);
	for(; first != last; ++first)
	{
		tmp.destroy(first);
	}
}


template<typename T, typename A>
inline T* allocateArray(const A& a, std::size_t count)
{
	typename A::template rebind<T>::other tmp(a);
	return tmp.allocate(count);
}

template<typename T, typename A>
void deallocateArray(const A& a, T* p, std::size_t count)
{
	typename A::template rebind<T>::other tmp(a);
	tmp.deallocate(p, count);
}

template<typename T>
struct SharedArrayTraits
{
	// a block with the same alignment requirement (and size) of T
#if 1 // temporary hack for C++03: does not support alignment requirement greater than std::size_t
	typedef std::size_t Block;
#else
	typedef typename std::aligned_storage<sizeof(std::size_t), std::alignment_of<T>::value>::type Block;
#endif

};

template<typename T>
std::size_t getSharedArraySize(std::size_t count)
{
	typedef typename SharedArrayTraits<T>::Block Block;
	std::size_t n = ((sizeof(T) + (sizeof(Block) - 1)) / sizeof(Block)) * count
		+ 1; // add storage for an extra block, in which the payload will be stored
	const std::size_t payloadSize = sizeof(std::size_t);
	ALLOCATOR_ASSERT(sizeof(Block) * n >= sizeof(T) * count + payloadSize);
	return n;
}

template<typename T, typename A>
T* allocateSharedArray(A& a, std::size_t count)
{
	typedef typename SharedArrayTraits<T>::Block Block;
	std::size_t n = getSharedArraySize<T>(count);
	return static_cast<T*>(static_cast<void*>(allocateArray<Block>(a, n)));
}

template<typename T, typename A>
void deallocateSharedArray(A& a, T* p, std::size_t count)
{
	typedef typename SharedArrayTraits<T>::Block Block;
	std::size_t n = getSharedArraySize<T>(count);
	deallocateArray<Block>(a, static_cast<Block*>(static_cast<void*>(p)), n);
}


template<typename T, typename A = std::allocator<UniqueTypeWrapper> >
struct SharedVector : private A
{
	T* first;
	T* last;
	std::size_t* payload;
	static std::size_t emptyCount;

	A& getAllocator()
	{
		return *this;
	}
	const A& getAllocator() const
	{
		return *this;
	}

	void destroy()
	{
		if(*payload == 1)
		{
			ALLOCATOR_ASSERT(payload != &emptyCount);
			destroyArray(getAllocator(), first, last);
			deallocateSharedArray(getAllocator(), first, capacity());
		}
		else
		{
			--*payload;
		}
	}
	void construct()
	{
		first = last = static_cast<T*>(static_cast<void*>(&emptyCount));
		payload = &emptyCount;
		++emptyCount;
	}
	void construct(std::size_t count)
	{
		last = first = allocateSharedArray<T>(getAllocator(), count);
		payload = static_cast<std::size_t*>(static_cast<void*>(first + count));
		*payload = 1;
	}
	bool unique() const
	{
		return *payload == 1;
	}

	SharedVector(const A& allocator = A())
		: A(allocator)
	{
		construct();
	}
	SharedVector(std::size_t count, const T& value, const A& allocator = A())
		: A(allocator)
	{
		construct();
		reserve(count);
		for(; count != 0; --count)
		{
			push_back(value);
		}
	}
	~SharedVector()
	{
		destroy();
	}
	SharedVector(const SharedVector& other)
		: first(other.first), last(other.last), payload(other.payload)
	{
		++*payload;
	}
	SharedVector& operator=(SharedVector tmp)
	{
		tmp.swap(*this);
		return *this;
	}
	std::size_t size() const
	{
		return last - first;
	}
	std::size_t capacity() const
	{
		return static_cast<T*>(static_cast<void*>(payload)) - first;
	}
	bool empty() const
	{
		return first == last;
	}
	void swap(SharedVector& other)
	{
		std::swap(first, other.first);
		std::swap(last, other.last);
		std::swap(payload, other.payload);
	}

	void reserve(std::size_t count)
	{
		if(count <= capacity())
		{
			return;
		}
		ALLOCATOR_ASSERT(capacity() == 0);
		destroy();
		construct(count);
	}
	void clear()
	{
		if(empty())
		{
			return;
		}
		ALLOCATOR_ASSERT(unique()); // can't modify if shared
		destroyArray(getAllocator(), first, last);
		last = first;
	}

	void push_back(const T& value)
	{
		ALLOCATOR_ASSERT(capacity() != 0);
		ALLOCATOR_ASSERT(last != first + capacity());
		constructObject(getAllocator(), last, value);
		++last;
	}
	void pop_back()
	{
		ALLOCATOR_ASSERT(capacity() != 0);
		ALLOCATOR_ASSERT(last != first);
		--last;
		destroyObject(getAllocator(), last);
	}
	void resize(std::size_t count, const T& value = T())
	{
		ALLOCATOR_ASSERT(unique()); // can't modify if shared
		ALLOCATOR_ASSERT(capacity() != 0);
		ALLOCATOR_ASSERT(count <= capacity());
		while(size() > count)
		{
			pop_back();
		}
		while(size() < count)
		{
			push_back(value);
		}
	}

	typedef const T* const_iterator;
	const_iterator begin() const
	{
		return first;
	}
	const_iterator end() const
	{
		return last;
	}

	const T& front() const
	{
		return *first;
	}
	const T& back() const
	{
		return *last;
	}

	T& operator[](std::size_t i)
	{
		ALLOCATOR_ASSERT(unique()); // can't modify if shared
		ALLOCATOR_ASSERT(i < size());
		return first[i];
	}
	const T& operator[](std::size_t i) const
	{
		ALLOCATOR_ASSERT(i < size());
		return first[i];
	}
};

template<typename T, typename A>
std::size_t SharedVector<T, A>::emptyCount = 1;


template<typename T, typename A>
inline bool operator==(const SharedVector<T, A>& left, const SharedVector<T, A>& right)
{
	return left.size() == right.size()
		&& std::equal(left.begin(), left.end(), right.begin());
}

template<typename T, typename A>
inline bool operator!=(const SharedVector<T, A>& left, const SharedVector<T, A>& right)
{
	return !operator==(left, right);
}

template<typename T, typename A>
inline bool operator<(const SharedVector<T, A>& left, const SharedVector<T, A>& right)
{
	return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
}

#endif
