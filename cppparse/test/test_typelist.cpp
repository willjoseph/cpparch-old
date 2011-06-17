
namespace N68
{
	template<typename X>
	struct Tmpl
	{
	};
	template<>
	struct Tmpl<char>
	{
		static const char VALUE = 0;
	};

	const char Tmpl<char>::VALUE;

	template<>
	struct Tmpl<int>
	{
		static const int VALUE = 0;
	};
	const int Tmpl<int>::VALUE; // Tmpl<int>::VALUE should be distinct from Tmpl<char>::VALUE
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
