
#ifndef INCLUDED_CPPPARSE_ALLOCATOR_H
#define INCLUDED_CPPPARSE_ALLOCATOR_H

#include "profiler.h"
#include <memory>
#include <algorithm>
#include <vector>

#ifdef _DEBUG
#define ALLOCATOR_DEBUG // causes unallocated memory to be marked and checked
#endif

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



struct AllocatorError
{
	AllocatorError()
	{
	}
};

#define ALLOCATOR_ASSERT(condition) if(!(condition)) { throw AllocatorError(); }

#define ALLOCATOR_FREECHAR char(0xba)

struct IsAllocated
{
	bool operator()(char c) const
	{
		return c != ALLOCATOR_FREECHAR;
	}
};

inline bool isAllocated(const char* first, const char* last)
{
	const char* p = std::find_if(first, last, IsAllocated());
	if(p != last)
	{
		std::cout << "allocation at: " << static_cast<const void*>(p) << ": ";
		std::cout.write(p, 4);
		std::cout << std::endl;
	}
	return p != last;
}

struct Page
{
	enum { SHIFT = 17 };
	enum { SIZE = 1 << SHIFT };
	enum { MASK = SIZE - 1 };
	char buffer[SIZE]; // debug padding

	Page()
	{
#ifdef ALLOCATOR_DEBUG
		std::uninitialized_fill(buffer, buffer + SIZE, ALLOCATOR_FREECHAR);
#endif
	}
};

template<bool checked>
struct LinearAllocator
{
	typedef std::vector<Page*> Pages;
	Pages pages;
	size_t position;
	size_t pendingBacktrack;
	static void* debugAddress;
	static char debugValue[4];
	LinearAllocator()
		: position(0), pendingBacktrack(0)
	{
	}
	~LinearAllocator()
	{
		ProfileScope profile(gProfileAllocator);
		for(Pages::iterator i = pages.begin(); i != pages.end(); ++i)
		{
			Page* p = *i;
			delete p; // TODO: fix heap-corruption assert
		}
	}
	Page* getPage(size_t index)
	{
		if(index == pages.size())
		{
			ProfileScope profile(gProfileAllocator);
			pages.push_back(new Page);
		}
		return pages[index];
	}
	void* allocate(size_t size)
	{
		checkAllocation();
		size_t available = sizeof(Page) - (position & Page::MASK);
		if(size > available)
		{
			position += available;
		}
		Page* page = getPage(position >> Page::SHIFT);
		void* p = page->buffer + (position & Page::MASK);
		position += size;
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!checked || !isAllocated(reinterpret_cast<char*>(p), reinterpret_cast<char*>(p) + size));
#endif
		return p;
	}
	void deallocate(void* p, size_t size)
	{
#ifdef ALLOCATOR_DEBUG
		std::uninitialized_fill(reinterpret_cast<char*>(p), reinterpret_cast<char*>(p) + size, ALLOCATOR_FREECHAR);
#endif
	}
	void backtrack(size_t original)
	{
		ALLOCATOR_ASSERT(original <= position);
#ifdef ALLOCATOR_DEBUG
		if(pendingBacktrack == 0)
		{
			pendingBacktrack = position;
		}
#endif	
		position = original;
	}
	void checkAllocation()
	{
#ifdef ALLOCATOR_DEBUG
		if(pendingBacktrack == 0)
		{
			return;
		}

		Pages::iterator first = pages.begin() + position / sizeof(Page);
		Pages::iterator last = pages.begin() + pendingBacktrack / sizeof(Page);
		for(Pages::iterator i = first; i != pages.end(); ++i)
		{
			ALLOCATOR_ASSERT(!checked || 
				!isAllocated(
					(*i)->buffer + (i == first ? position % sizeof(Page) : 0),
					(*i)->buffer + (i == last ? pendingBacktrack % sizeof(Page) : sizeof(Page))
				)
			);
			if(i == last)
			{
				break;
			}
		}

		pendingBacktrack = 0;
#endif
	}	
};


template<bool checked>
void* LinearAllocator<checked>::debugAddress;

template<bool checked>
char LinearAllocator<checked>::debugValue[4];

typedef LinearAllocator<true> CheckedLinearAllocator;

inline CheckedLinearAllocator& NullAllocator()
{
	static CheckedLinearAllocator null;
	return null;
}

template<class T>
class LinearAllocatorWrapper
{
public:
	CheckedLinearAllocator& instance;

	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef const T* const_pointer;
	typedef const T& const_reference;

	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template<class OtherT>
	struct rebind
	{
		typedef LinearAllocatorWrapper<OtherT> other;
	};
	LinearAllocatorWrapper() : instance(NullAllocator())
	{
		throw AllocatorError();
	}
	LinearAllocatorWrapper(CheckedLinearAllocator& instance) : instance(instance)
	{
	}
	LinearAllocatorWrapper(const LinearAllocatorWrapper<T>& other) : instance(other.instance)
	{
	}
	template<class OtherT>
	LinearAllocatorWrapper(const LinearAllocatorWrapper<OtherT>& other) : instance(other.instance)
	{
	}
	template<class OtherT>
	LinearAllocatorWrapper<T>& operator=(const LinearAllocatorWrapper<OtherT>& other)
	{
		if(this != &other)
		{
			this->~LinearAllocatorWrapper();
			new(this) LinearAllocatorWrapper(other);
		}
		// do nothing!
		return (*this);
	}

	void deallocate(pointer p, size_type count)
	{
		//std::cout << "deallocate: " << p << std::endl; 
		instance.deallocate(p, count * sizeof(T));
	}

	pointer allocate(size_type count)
	{
		pointer p = pointer(instance.allocate(count * sizeof(T)));
		//std::cout << "allocate: " << p << std::endl;
		return p;
	}

	pointer allocate(size_type count, const void* hint)
	{
		return allocate(count);
	}

	void construct(pointer p, const T& value)
	{
		new(p) T(value);
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	size_type max_size() const
	{
		size_type _Count = size_type(-1) / sizeof(T);
		return (0 < _Count ? _Count : 1);
	}
};

template<class T,
class OtherT>
inline bool operator==(const LinearAllocatorWrapper<T>&, const LinearAllocatorWrapper<OtherT>&)
{
	return true;
}

template<class T,
class OtherT>
inline bool operator!=(const LinearAllocatorWrapper<T>&, const LinearAllocatorWrapper<OtherT>&)
{
	return false;
}

template<typename T>
inline void checkAllocation(LinearAllocatorWrapper<T>& a, T* p)
{
	if(CheckedLinearAllocator::debugAddress == p)
	{
		std::cout << "debug allocation!" << std::endl;
	}
	if(CheckedLinearAllocator::debugAddress >= p && CheckedLinearAllocator::debugAddress < p + 1)
	{
		if(memcmp(CheckedLinearAllocator::debugAddress, CheckedLinearAllocator::debugValue, 1) == 0)
		{
			std::cout << "debug allocation!" << std::endl;
		}
	}
}

template<typename A, typename T>
inline void checkAllocation(A& a, T* p)
{
	// by default, do nothing
}


template<typename T, typename A>
inline T* allocatorNew(const A& a, const T& value)
{
	typename A::template rebind<T>::other tmp(a);
	T* p = tmp.allocate(1);
	tmp.construct(p, value);
	checkAllocation(tmp, p);
	return p;
}

template<typename T, typename A>
void allocatorDelete(const A& a, T* p)
{
	if(p != 0)
	{
		typename A::template rebind<T>::other tmp(a);
		tmp.destroy(p);
		tmp.deallocate(p, 1);
	}
}

#endif


