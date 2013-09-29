
#ifndef INCLUDED_CPPPARSE_COMMON_INDIRECT_SET_H
#define INCLUDED_CPPPARSE_COMMON_INDIRECT_SET_H

#include <set>
#include "Common.h"

// ----------------------------------------------------------------------------
// returns d < b
// requires that D be derived from B, and that B has a virtual function
template<typename D, typename B>
inline bool abstractLess(const D& d, const B& b)
{
	const TypeInfo& dType = getTypeInfo<D>();
	const TypeInfo& bType = getTypeInfo(b);
	return lessThan(dType, bType) ||
		!lessThan(bType, dType) && d < *static_cast<const D*>(&b);
}

struct IndirectLess
{
	template<typename T, typename U>
	bool operator()(T left, U right) const
	{
		return *left < *right;
	}
};

template<typename Ptr>
struct IndirectSet
{
	typedef std::set<Ptr, IndirectLess> Set;
	Set elements;

	typedef typename Set::iterator iterator;

	iterator begin()
	{
		return elements.begin();
	}
	iterator end()
	{
		return elements.end();
	}
	// Ptr must be constructible from T*
	template<typename T>
	iterator find(const T& value)
	{
		return elements.find(&value);
	}
	// Ptr must be constructible from T*
	template<typename T>
	inline iterator insert(const T& value)
	{
		iterator i = elements.lower_bound(&value); // first element not less than value
		if(i != elements.end()
			&& !elements.key_comp()(&value, *i)) // if value is not less than lower bound
		{
			// lower bound is equal to value
			return i;
		}
		return elements.insert(i, new T(value));
	}
	void clear()
	{
		for(iterator i = elements.begin(); i != elements.end(); ++i)
		{
			delete &(*(*i));
		}
		elements.clear();
	}
};


#endif


