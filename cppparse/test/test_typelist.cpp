
namespace N116
{
	template<typename T>
	struct Tmpl
	{
		int x[sizeof(T)];
	};

	struct S : Tmpl<int>
	{
		using Tmpl<int>::x;
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
