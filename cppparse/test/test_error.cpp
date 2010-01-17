
// TODO: long backtrack here: 'nested-name-specifier' vs 'type-id' beginning with 'simple-template-id'
template <class _Iterator>
class checked_array_iterator
	: public iterator<
	typename iterator_traits<_Iterator>::iterator_category, 
	typename iterator_traits<_Iterator>::value_type, 
	typename iterator_traits<_Iterator>::difference_type, 
	typename iterator_traits<_Iterator>::pointer, 
	typename iterator_traits<_Iterator>::reference>
{
};


typedef union _LARGE_INTEGER {
	struct {
		DWORD LowPart;
		LONG HighPart;
	};
	LONGLONG QuadPart;
} LARGE_INTEGER;


__inline struct _TEB * NtCurrentTeb( void ) { __asm mov eax, fs:[0x18] }

typedef struct _ImageArchitectureHeader {
	unsigned int AmaskValue: 1;
} IMAGE_ARCHITECTURE_HEADER;


void test()
{
	__asm int 3 __asm int 3 // one line
	__asm { int 3; } // braced
	__asm { __asm int 3; __asm int 3; }; // nested
	__asm { __asm { int 3 }; }; // nested
}

class A
{
public:
	explicit A()
		: a(0)
	{
	}
	__thiscall ~A()
	{
	}
	A& operator=(const A&)
	{
		return *this;
	}
};

class A
{
	explicit A(R(T::*m)()) // 'ptr-operator' with 'nested-name-specifier' aka ptr-to-member
	{
	}
};

inline Type<R(*)(A)> function() // ambiguity: 'template-argument' may be 'type-id' or 'primary-expression'
{
}

class basic_streambuf
{
	basic_streambuf()
		: _Plocale(new(locale)) // new-expression without placement, with parenthesised type-id
	{
	}
};

class A
{
	explicit A(const char *_Locname, size_t _Refs = 0)
		: codecvt<_Elem, _Byte, _Statype>(_Locname, _Refs) // mem-initializer with template-id
	{
	}
};


class A
{
	void member()
	{
		this->m = new X(*this->m); // new-expression without placement
	}
};

char* test = "str1" "str2"; // concatenate adjacent string-literal tokens

template<class _RanIt>
class reverse_iterator
	: public _Iterator_base_secure
{
	_Checked_iterator_base_type _Checked_iterator_base() const
	{
		typename _Checked_iterator_base_type _Base(current); // allow 'nested-name-specifier' to be omitted in 'typename-specifier'
		return _Base;
	}
};


template <class _Traits>
class _Char_traits_category_helper<_Traits, true>
{
public:
	typedef typename _Traits::_Secure_char_traits _Secure_char_traits; // typename-specifier
};

template <class _Traits>
class _Inherits_from_char_traits_base
{
	static _False _Inherits(...); // parameter-declaration-clause == ellipsis
};

template<> struct char_traits<wchar_t>:
public _Char_traits_base // base-clause
{
};

template<class _Statetype>
const _Statetype fpos<_Statetype>::_Stz = _Statetype(); // nested-name-specifier with template-id

template<class _Statetype>
class fpos
{	// store arbitrary file position
	typedef fpos<_Statetype> _Myt;
};

namespace
{
void*operator new(size_t _Size)throw()
{
	return (x < y ? x : y); // conditional-expression
}
}

void function(int arg, ...)
{
	va_list args;
	(args = p); // assignment-expression
}

void function(void(*)(void));

