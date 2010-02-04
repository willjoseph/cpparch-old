
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

namespace N
{
	class X;
}

namespace N
{
	X x;
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



