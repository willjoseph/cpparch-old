

template<typename T>
struct S
{
};

typedef S<struct X> Type;

//Type::X x;


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
typedef TypeList<int(int a), TypeListEnd> Test3;
#endif

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
> Test2;
#endif
