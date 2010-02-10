
namespace N
{
	class X;
}

namespace N
{
	X x;
}

typedef struct _MIDL_SYNTAX_INFO MIDL_SYNTAX_INFO;

typedef struct _MIDL_SYNTAX_INFO
{
} MIDL_SYNTAX_INFO;

struct S3
{
	typedef S3 Type;
	typedef int T;
};


template<typename T>
class Template3
{
	void f()
	{
		typename T::dependent x;
	}
};



template<typename T>
class Template2;

template<typename T>
class Template2
{
	static int x;
};

template<typename T>
int Template2<T>::x;


struct Incomplete;

template<typename T>
class Template
{
	void f()
	{
		Incomplete::X(); // should interpret this as a function-call
	};
};

int f(int a)
{
	int t(a); // function-declaration or variable declaration with init
	typedef int T;
	int (*x)(T)(f);
	int (*x)(T)[1];
	// illegal
	// int t[1](a, b);
	// int t(a)(b);
	// int t(a)[1];
}

template<class T>
class Template1
{
	friend class Template1;
	typedef Template1<int> Type;
};

typedef struct { typedef int T; } S1;
typedef struct { typedef int T; } S2;

S1::T s1t;
S2::T s2t;


void f()
{
	struct S;
	S *p;
}

namespace std
{

template<class _Ty> inline
	void swap(_Ty& _Left, _Ty& _Right)
	{	// exchange values stored at _Left and _Right
	_Ty _Tmp = _Left;
	_Left = _Right, _Right = _Tmp;
	}

		// TEMPLATE STRUCT pair
template<class _Ty1,
	class _Ty2> struct pair
	{	// store a pair of values
	typedef pair<_Ty1, _Ty2> _Myt;
	typedef _Ty1 first_type;
	typedef _Ty2 second_type;

	void swap(_Myt& _Right)
		{	// exchange contents with _Right
		std::swap(first, _Right.first);
		std::swap(second, _Right.second);
		}

	_Ty1 first;	// the first stored value
	_Ty2 second;	// the second stored value
};
}

class C3
{
	void reset(C3*)
	{
	}
	void f()
	{
		C3 *_Ptr;
		reset(_Ptr);
		return (*this);
	}
};


template<typename T>
class Tmpl
{
	template<typename Q>
	void func()
	{
		Q q;
	}
};

enum { ENUM1 };
enum { ENUM2 };

template<typename T>
class C1
{
	static T t;
};

template<typename T>
T C1::t;

template<>
class C1<int>
{
};

namespace std
{
	class _Lockit
	{
	public:
		explicit _Lockit();
		explicit _Lockit(int);
		~_Lockit();
		static void _Lockit_ctor(int);
		static void _Lockit_dtor(int);
	private:
		static void _Lockit_ctor(_Lockit*);
		static void _Lockit_ctor(_Lockit*,int);
		static void _Lockit_dtor(_Lockit*);
		_Lockit(const _Lockit&);
		_Lockit&operator=(const _Lockit&);
		int _Locktype;
	};

	class _Mutex
	{	// lock under program control

	};
}

typedef struct _div_t {
} div_t;

typedef struct _ldiv_t {
} ldiv_t;

inline ldiv_t  div()
{
}

div_t  __cdecl div();


struct threadlocaleinfostruct;
typedef struct threadlocaleinfostruct * pthreadlocinfo;

typedef enum IMAGE_AUX_SYMBOL_TYPE {
	IMAGE_AUX_SYMBOL_TYPE_TOKEN_DEF = 1,
} IMAGE_AUX_SYMBOL_TYPE;

typedef struct fd_set
{
} fd_set;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;


class type_info;

const type_info& type;


template<class T>
class C
{
	friend class C;
	C()
	{
	}
	~C()
	{
	}
};


class A
{
	void f();
};

class B
{
	friend class A;
};

void A::f()
{
}




typedef unsigned int size_t;

template<size_t _Size>
inline char*gets_s(char(&_Buffer)[_Size])
{
}

template<size_t _Size>
inline int _snprintf_s(char(&_Dest)[_Size], size_t __Size, const char*_Format, ...)
{
};


template<typename A>
struct X
{
	template<typename B>
	struct Y
	{
		template<typename C>
		struct Z
		{
			A a;
			B b;
			C c;
		};
	};
};

X<int>::Y<int>::Z<int> p;


template<typename A>
struct Y
{
};



