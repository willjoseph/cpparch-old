
// testing shared-prefix declarator(_suffix)
void f()
{
}

struct R
{
};

// testing shared-prefix identifier
R i;

template<typename T>
struct S
{
};

// testing shared-prefix template-id
typedef S<struct X> Type2;
typedef S<int(int a)> Type1;




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

#if 1
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
