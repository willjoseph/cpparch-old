
#ifndef INCLUDED_CPPPARSE_ALLOCATOR_H
#define INCLUDED_CPPPARSE_ALLOCATOR_H

#include "profiler.h"
#include <memory>
#include <algorithm>
#include <vector>

#ifdef _DEBUG
#define ALLOCATOR_DEBUG // causes unallocated memory to be marked and checked
#endif

#ifdef ALLOCATOR_DEBUG
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

#else
#define DebugAllocator std::allocator
#endif


struct AllocatorError
{
	AllocatorError()
	{
	}
};

#define ALLOCATOR_ASSERT(condition) if(!(condition)) { throw AllocatorError(); }

#define ALLOCATOR_FREECHAR char(0xba)
#define ALLOCATOR_INITCHAR char(0xcd)

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

struct IsDeallocated
{
	bool operator()(char c) const
	{
		return c == ALLOCATOR_FREECHAR;
	}
};

inline bool isDeallocated(const char* first, const char* last)
{
	const char* p = std::find_if(first, last, IsDeallocated());
	if(p != last)
	{
		std::cout << "deallocation at: " << static_cast<const void*>(p) << ": ";
		std::cout.write(p, 4);
		std::cout << std::endl;
	}
	return p != last;
}

template<typename T>
inline bool isDeallocated(const T* p)
{
	return p != 0 &&
#if 1
		*reinterpret_cast<const size_t*>(p) == 0xbabababa;
#else
		isDeallocated(reinterpret_cast<const char*>(p), reinterpret_cast<const char*>(p + 1));
#endif
}

template<typename T>
struct SafePtr
{
	typedef T Type;
	T* p;
	SafePtr()
	{
	}
	SafePtr(T* p)
		: p(p)
	{
	}
	SafePtr(const SafePtr<T>& other)
		: p(other.p)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
	}
	SafePtr<T>& operator=(const SafePtr<T>& other)
	{
		p = other;
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return *this;
	}
	SafePtr<T>& operator=(T* other)
	{
		p = other;
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return *this;
	}
	T& operator*() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return *p;
	}
	T* operator->() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return p;
	}
	operator T*() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return p;
	}
};

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


	static void* debugAddress;
	static char debugValue[4];
	static size_t debugAllocationId;

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
			ProfileScope profile(gProfileAllocator);
			pages.push_back(new Page);
		}
		return pages[index];
	}
	void* allocate(size_t size)
	{
#ifdef ALLOCATOR_DEBUG
		if(position == debugAllocationId)
		{
			std::cout << "debug allocation!" << std::endl;
		}
#endif
		size_t available = sizeof(Page) - (position & Page::MASK);
		if(size > available)
		{
			position += available;
		}
		Page* page = getPage(position >> Page::SHIFT);
		void* p = page->buffer + (position & Page::MASK);
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!checked || !isAllocated(reinterpret_cast<char*>(p), reinterpret_cast<char*>(p) + size));
		std::uninitialized_fill(reinterpret_cast<char*>(p), reinterpret_cast<char*>(p) + size, ALLOCATOR_INITCHAR);
#endif
		position += size;
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


template<bool checked>
void* LinearAllocator<checked>::debugAddress;

template<bool checked>
char LinearAllocator<checked>::debugValue[4];

template<bool checked>
size_t LinearAllocator<checked>::debugAllocationId = 0xffffffff;

typedef LinearAllocator<true> CheckedLinearAllocator;

inline CheckedLinearAllocator& NullAllocator()
{
	static CheckedLinearAllocator null;
	return null;
}

template<typename T, typename Instance = CheckedLinearAllocator>
class LinearAllocatorWrapper
{
public:
	Instance& instance;

	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef const T* const_pointer;
	typedef const T& const_reference;

	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template<typename OtherT>
	struct rebind
	{
		typedef LinearAllocatorWrapper<OtherT, Instance> other;
	};
	LinearAllocatorWrapper() : instance(NullAllocator())
	{
		throw AllocatorError();
	}
	LinearAllocatorWrapper(Instance& instance) : instance(instance)
	{
	}
	LinearAllocatorWrapper(const LinearAllocatorWrapper<T, Instance>& other) : instance(other.instance)
	{
	}
	template<typename OtherT>
	LinearAllocatorWrapper(const LinearAllocatorWrapper<OtherT, Instance>& other) : instance(other.instance)
	{
	}
	template<typename OtherT>
	LinearAllocatorWrapper<T>& operator=(const LinearAllocatorWrapper<OtherT, Instance>& other)
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
		instance.deallocate(p, count * sizeof(T)
#ifdef ALLOCATOR_DEBUG
			+ sizeof(size_t)
#endif
			);
	}

	pointer allocate(size_type count)
	{
		pointer p = pointer(instance.allocate(count * sizeof(T)
#ifdef ALLOCATOR_DEBUG
			+ sizeof(size_t)
#endif
			));
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

template<typename T, typename Instance, typename OtherT>
inline bool operator==(const LinearAllocatorWrapper<T, Instance>&, const LinearAllocatorWrapper<OtherT, Instance>&)
{
	return true;
}

template<typename T, typename Instance, typename OtherT>
inline bool operator!=(const LinearAllocatorWrapper<T, Instance>&, const LinearAllocatorWrapper<OtherT, Instance>&)
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

template<typename T>
struct TypeTraits
{
	typedef T Value;
};

template<typename T>
struct TypeTraits<const T>
{
	typedef T Value;
};



template<typename T>
struct ReferenceCounted : T
{
	size_t count;
	ReferenceCounted(const T& value)
		: T(value), count(0)
	{
	}
	~ReferenceCounted()
	{
		ALLOCATOR_ASSERT(count == 0);
	}
};



template<typename T>
struct Reference
{
#ifdef ALLOCATOR_DEBUG
	typedef ReferenceCounted<T> Value;
	static Value makeValue(const T& t)
	{
		return Value(t);
	}
	void decrement()
	{
		if(p != 0)
		{
			--p->count;
		}
	}
	void increment()
	{
		if(p != 0)
		{
			++p->count;
		}
	}
#else
	typedef T Value;
	static Value makeValue(const T& t)
	{
		return t;
	}
	void decrement()
	{
	}
	void increment()
	{
	}
#endif
	Value* p;
	Reference()
		: p(0)
	{
	}
	Reference(Value* p)
		: p(p)
	{
		increment();
	}
	~Reference()
	{
		decrement();
	}
	Reference(const Reference& other)
		: p(other.p)
	{
		increment();
	}
	Reference& operator=(Reference tmp)
	{
		tmp.swap(*this);
		return *this;
	}
	bool empty() const
	{
		return p == 0;
	}
	void swap(Reference& other)
	{
		std::swap(p, other.p);
	}
	T& operator*() const
	{
		return *p;
	}
	T* operator->() const
	{
		return p;
	}
	T* get() const
	{
		return p;
	}
	operator bool() const
	{
		return p != 0;
	}
};

#endif


