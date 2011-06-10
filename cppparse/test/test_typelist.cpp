

namespace N114
{
	template<typename T>
	struct Base
	{
		Base(int, int)
		{
		}
	};

	template<typename T>
	struct S
		: public Base<T>
	{
		explicit S(int i, int j = 0)
			: Base<T>(i, j)
		{
		}
	};
}


template<typename T>
struct S
{
};

typedef S<int(int a)> Type1;
typedef S<struct X> Type2;

//X x;


struct TypeListEnd
{
};

template<typename ItemType, typename NextType>
struct TypeList
{
	typedef ItemType Item;
	typedef NextType Next;
};

typedef TypeList<int, TypeListEnd> Test1;

#if 0
// pathological case for parser
typedef TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int,
	TypeList<int, TypeListEnd>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
	>
> Test2;
#endif
