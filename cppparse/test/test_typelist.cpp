
struct TypeListEnd
{
};

template<typename ItemType, typename NextType>
struct TypeList
{
	typedef ItemType Item;
	typedef NextType Next;
};

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
> Test;
