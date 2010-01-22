		// TEMPLATE CLASS reverse_iterator
template<class _RanIt>
	class reverse_iterator
		: public _Iterator_base_secure
	{	// wrap iterator to run it backwards
public:
	typedef reverse_iterator<_RanIt> _Myt;
	typedef typename iterator_traits<_RanIt>::iterator_category iterator_category;
	typedef typename iterator_traits<_RanIt>::value_type value_type;
	typedef typename iterator_traits<_RanIt>::difference_type difference_type;
	typedef typename iterator_traits<_RanIt>::difference_type distance_type;	// retained
	typedef typename iterator_traits<_RanIt>::pointer pointer;
	typedef typename iterator_traits<_RanIt>::reference reference;
	typedef _RanIt iterator_type;

	 reverse_iterator()
		{	// construct with default wrapped iterator
		}

	explicit  reverse_iterator(_RanIt _Right)
		: current(_Right)
		{	// construct wrapped iterator from _Right
		}

	template<class _Other>
		 reverse_iterator(const reverse_iterator<_Other>& _Right)
		: current(_Right.base())
		{	// initialize with compatible base%%
		}

	_RanIt  base() const
		{	// return wrapped iterator
		return (current);
		}

	reference  operator*() const
		{	// return designated value
		_RanIt _Tmp = current;
		return (*--_Tmp);
		}

	pointer  operator->() const
		{	// return pointer to class object
		return (&**this);
		}

	_Myt&  operator++()
		{	// preincrement
		--current;
		return (*this);
		}

	_Myt  operator++(int)
		{	// postincrement
		_Myt _Tmp = *this;
		--current;
		return (_Tmp);
		}

	_Myt&  operator--()
		{	// predecrement
		++current;
		return (*this);
		}

	_Myt  operator--(int)
		{	// postdecrement
		_Myt _Tmp = *this;
		++current;
		return (_Tmp);
		}

	template<class _Other>
		bool  _Equal(const reverse_iterator<_Other>& _Right) const
		{	// test for iterator equality
		return (current == _Right.base());
		}

// N.B. functions valid for random-access iterators only beyond this point

	_Myt&  operator+=(difference_type _Off)
		{	// increment by integer
		current -= _Off;
		return (*this);
		}

	_Myt  operator+(difference_type _Off) const
		{	// return this + integer
		return (_Myt(current - _Off));
		}

	_Myt&  operator-=(difference_type _Off)
		{	// decrement by integer
		current += _Off;
		return (*this);
		}

	_Myt  operator-(difference_type _Off) const
		{	// return this - integer
		return (_Myt(current + _Off));
		}

	reference  operator[](difference_type _Off) const
		{	// subscript
		return (*(*this + _Off));
		}

	template<class _Other>
		bool  _Less(const reverse_iterator<_Other>& _Right) const
		{	// test if this < _Right
		return (_Right.base() < current);
		}

	template<class _Other>
		difference_type  _Minus(const reverse_iterator<_Other>& _Right) const
		{	// return difference of iterators
		return (_Right.base() - current);
		}

protected:
	_RanIt current;	// the wrapped iterator
	};

		// reverse_iterator TEMPLATE OPERATORS
template<class _RanIt,
	class _Diff> inline
	reverse_iterator<_RanIt>  operator+(_Diff _Off,
		const reverse_iterator<_RanIt>& _Right)
	{	// return reverse_iterator + integer
	return (_Right + _Off);
	}

template<class _RanIt1,
	class _RanIt2> inline
	typename reverse_iterator<_RanIt1>::difference_type
		 operator-(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// return difference of reverse_iterators
	return (_Left._Minus(_Right));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool  operator==(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator equality
	return (_Left._Equal(_Right));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool  operator!=(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator inequality
	return (!(_Left == _Right));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool  operator<(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator < reverse_iterator
	return (_Left._Less(_Right));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool  operator>(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator > reverse_iterator
	return (_Right < _Left);
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool  operator<=(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator <= reverse_iterator
	return (!(_Right < _Left));
	}

template<class _RanIt1,
	class _RanIt2> inline
	bool  operator>=(const reverse_iterator<_RanIt1>& _Left,
		const reverse_iterator<_RanIt2>& _Right)
	{	// test for reverse_iterator >= reverse_iterator
	return (!(_Left < _Right));
	}

namespace GNamespace
{
	typedef int NType;
	NType nVar;
}

typedef GNamespace::NType GType;

GType gVar = GNamespace::nVar;

GType gFunc(GType param)
{
	typedef GType FType;
	FType lVar;
	if(FType lVar = gVar)
	{
	}
}

class GClass
{
	typedef GType CType;
	GClass(CType param) : mVar(param)
	{
	}
	CType mFunc(CType param)
	{
		typedef CType FType;
		FType fVar = param;
		if(FType cVar = mVar)
		{
			typedef FType LType;
			LType lVar;
		}

		class FClass
		{
			FClass(FType param) : mVar(param)
			{
			}
			FType mFunc(FType param)
			{
				typedef CType FType;
				FType fVar = param;
				if(FType cVar = mVar)
				{
					typedef FType LType;
					LType lVar;
				}
			}
			FType mVar;
		} fVar2;
	}
	CType mVar;

	class MClass
	{
		MClass(CType param) : mVar(param)
		{
		}
		CType mFunc(CType param)
		{
			typedef CType FType;
			FType fVar = param;
			if(FType cVar = mVar)
			{
				typedef FType LType;
				LType lVar;
			}
		}
		CType mVar;
	} mVar2;
} gVar2;
