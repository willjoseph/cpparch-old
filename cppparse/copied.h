
#ifndef INCLUDED_CPPPARSE_COPIED_H
#define INCLUDED_CPPPARSE_COPIED_H


template<typename T, typename A>
class Copied : private A
{
	T* p;

	A& getAllocator()
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
		std::swap(p, other.p);
	}

	T* get()
	{
		return p;
	}
	const T* get() const
	{
		return p;
	}

	bool empty() const
	{
		return p == 0;
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

namespace std
{
	template<typename T, typename A>
	void swap(Copied<T, A>& left, Copied<T, A>& right)
	{
		left.swap(right);
	}
}

#endif


