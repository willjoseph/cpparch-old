
#ifndef INCLUDED_CPPPARSE_ALLOCATOR_H
#define INCLUDED_CPPPARSE_ALLOCATOR_H

#include "profiler.h"
#include <memory>
#include <algorithm>
#include <vector>

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
	return std::find_if(first, last, IsAllocated()) != last;
}

struct Page
{
	enum { SIZE = 128 * 1024 };
	char buffer[SIZE]; // debug padding

	Page()
	{
#ifdef _DEBUG
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
	LinearAllocator()
		: position(0)
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
			pages.push_back(new Page);
		}
		return pages[index];
	}
	void* allocate(size_t size)
	{
		ProfileScope profile(gProfileAllocator);
		size_t available = sizeof(Page) - position % sizeof(Page);
		if(size > available)
		{
			position += available;
		}
		Page* page = getPage(position / sizeof(Page));
		void* p = page->buffer + position % sizeof(Page);
		position += size;
#ifdef _DEBUG
		ALLOCATOR_ASSERT(!checked || !isAllocated(reinterpret_cast<char*>(p), reinterpret_cast<char*>(p) + size));
#endif
		return p;
	}
	void deallocate(void* p, size_t size)
	{
#ifdef _DEBUG
		std::uninitialized_fill(reinterpret_cast<char*>(p), reinterpret_cast<char*>(p) + size, ALLOCATOR_FREECHAR);
#endif
	}
	void backtrack(size_t original)
	{
#ifdef _DEBUG
		Pages::iterator first = pages.begin() + original / sizeof(Page);
		Pages::iterator last = pages.begin() + position / sizeof(Page);
		for(Pages::iterator i = first; i != pages.end(); ++i)
		{
			ALLOCATOR_ASSERT(!checked || 
				!isAllocated(
					(*i)->buffer + (i == first ? original % sizeof(Page) : 0),
					(*i)->buffer + (i == last ? position % sizeof(Page) : sizeof(Page))
				)
			);
			if(i == last)
			{
				break;
			}
		}
#endif
		position = original;
	}	
};

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


#endif


