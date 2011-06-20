
#ifndef INCLUDED_CPPPARSE_COPIED_H
#define INCLUDED_CPPPARSE_COPIED_H

#include "allocator.h"

template<typename T, typename A>
class Copied : private A
{
protected:
	T* p;

	A& getAllocator()
	{
		return *this;
	}
	const A& getAllocator() const
	{
		return *this;
	}
public:
	Copied(const A& allocator)
		: A(allocator), p(0)
	{
	}
	~Copied()
	{
		allocatorDelete(getAllocator(), p);
	}
	Copied(const T& value, const A& allocator)
		: A(allocator), p(allocatorNew(getAllocator(), value))
	{
	}
	Copied(const Copied& other)
		: A(other), p(other.p == 0 ? 0 : allocatorNew(getAllocator(), *other.p))
	{
	}
	Copied& operator=(const Copied& other)
	{
		Copied tmp(other);
		tmp.swap(*this);
		return *this;
	}
	Copied& operator=(const T& value)
	{
		Copied tmp(value, getAllocator());
		tmp.swap(*this);
		return *this;
	}

	void swap(Copied& other)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		std::swap(p, other.p);
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
	}

	T* get()
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return p;
	}
	const T* get() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return p;
	}

	bool empty() const
	{
		return p == 0;
	}
	T& back()
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return *p;
	}
	const T& back() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return *p;
	}
};

namespace std
{
	template<typename T, typename A>
	void swap(Copied<T, A>& left, Copied<T, A>& right)
	{
		left.swap(right);
	}
}

template<typename T, typename A>
class CopiedReference : private A
{
	Reference<T> p;

	A& getAllocator()
	{
		return *this;
	}
	const A& getAllocator() const
	{
		return *this;
	}
public:
	CopiedReference(const A& allocator)
		: A(allocator), p(0)
	{
	}
	CopiedReference(const T& t, const A& allocator)
		: A(allocator), p(allocatorNew(getAllocator(), makeReferenceCounted(t)))
	{
	}
	CopiedReference& operator=(const T& t)
	{
		CopiedReference tmp(t, getAllocator());
		tmp.swap(*this);
		return *this;
	}
	CopiedReference& operator=(CopiedReference tmp)
	{
		tmp.swap(*this);
		return *this;
	}

	void swap(CopiedReference& other)
	{
		p.swap(other.p);
	}

	const Reference<T>& get() const
	{
		return p;
	}

	bool empty() const
	{
		return p.empty();
	}
	T& back()
	{
		return *p;
	}
	const T& back() const
	{
		return *p;
	}
};


#endif


