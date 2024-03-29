
#ifndef INCLUDED_CPPPARSE_COMMON_COMMON_H
#define INCLUDED_CPPPARSE_COMMON_COMMON_H

#include <cstddef> // size_t 
#include <typeinfo>

// ----------------------------------------------------------------------------

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(*array))
#define ARRAY_END(array) ((array) + ARRAY_COUNT(array))

template<typename T>
struct ArrayRange
{
	const T* first;
	const T* last;
	ArrayRange(const T* first, const T* last)
		: first(first), last(last)
	{
	}
};

template<typename T>
ArrayRange<T> makeRange(const T* first, const T* last)
{
	return ArrayRange<T>(first, last);
}

#define ARRAY_RANGE(array) makeRange(array, ARRAY_END(array))


// ----------------------------------------------------------------------------

struct TypeListEnd
{
};

template<typename ItemType, typename NextType>
struct TypeList
{
	typedef ItemType Item;
	typedef NextType Next;
};

#define TYPELIST0() TypeListEnd
#define TYPELIST1(T0) TypeList<T0, TYPELIST0()>
#define TYPELIST2(T0, T1) TypeList<T0, TYPELIST1(T1) >
#define TYPELIST3(T0, T1, T2) TypeList<T0, TYPELIST2(T1, T2) >
#define TYPELIST4(T0, T1, T2, T3) TypeList<T0, TYPELIST3(T1, T2, T3) >
#define TYPELIST5(T0, T1, T2, T3, T4) TypeList<T0, TYPELIST4(T1, T2, T3, T4) >
#define TYPELIST6(T0, T1, T2, T3, T4, T5) TypeList<T0, TYPELIST5(T1, T2, T3, T4, T5) >
#define TYPELIST7(T0, T1, T2, T3, T4, T5, T6) TypeList<T0, TYPELIST6(T1, T2, T3, T4, T5, T6) >
#define TYPELIST8(T0, T1, T2, T3, T4, T5, T6, T7) TypeList<T0, TYPELIST7(T1, T2, T3, T4, T5, T6, T7) >
#define TYPELIST9(T0, T1, T2, T3, T4, T5, T6, T7, T8) TypeList<T0, TYPELIST8(T1, T2, T3, T4, T5, T6, T7, T8) >
#define TYPELIST10(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9) TypeList<T0, TYPELIST9(T1, T2, T3, T4, T5, T6, T7, T8, T9) >
#define TYPELIST11(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) TypeList<T0, TYPELIST10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) >
#define TYPELIST12(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) TypeList<T0, TYPELIST11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) >
#define TYPELIST13(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) TypeList<T0, TYPELIST12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) >
#define TYPELIST14(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) TypeList<T0, TYPELIST13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) >
#define TYPELIST15(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) TypeList<T0, TYPELIST14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) >
#define TYPELIST16(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) TypeList<T0, TYPELIST15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) >
#define TYPELIST17(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) TypeList<T0, TYPELIST16(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16) >

template<typename Types, size_t N>
struct TypeListNth
{
	typedef typename TypeListNth<typename Types::Next, N - 1>::Result Result;
};

template<typename Types>
struct TypeListNth<Types, 0>
{
	typedef typename Types::Item Result;
};

#define TYPELIST_NTH(Types, N) typename TypeListNth<Types, N>::Result

template<typename Types>
struct TypeListCount
{
	enum { RESULT = TypeListCount<typename Types::Next>::RESULT + 1 };
};

template<>
struct TypeListCount<TypeListEnd>
{
	enum { RESULT = 0 };
};

#define TYPELIST_COUNT(Types) TypeListCount<Types>::RESULT


// ----------------------------------------------------------------------------



struct Callback0
{
	typedef void (*Function)(void* data);
	Function function;
	void* data;
	Callback0()
		: function(defaultThunk)
	{
	}
	Callback0(Function function, void* data)
		: function(function), data(data)
	{
	}
	void operator()() const
	{
		function(data);
	}
	static void defaultThunk(void*)
	{
	}
};

template<typename A>
struct Callback1
{
	typedef void (*Function)(void* data, A);
	Function function;
	void* data;
	Callback1()
		: function(defaultThunk)
	{
	}
	Callback1(Function function, void* data)
		: function(function), data(data)
	{
	}
	void operator()(A a) const
	{
		function(data, a);
	}
	static void defaultThunk(void*, A)
	{
	}
};

template<typename Object, void (Object::*member)()>
struct Member0
{
	void* object;
	Member0(Object& object)
		: object(&object)
	{
	}
	typedef Callback0 Result;
	static void thunk(void* object)
	{
		((*static_cast<Object*>(object)).*member)();
	}
};
template<typename Object, typename A, void (Object::*member)(A)>
struct Member1
{
	void* object;
	Member1(Object& object)
		: object(&object)
	{
	}
	typedef Callback1<A> Result;
	static void thunk(void* object, A a)
	{
		((*static_cast<Object*>(object)).*member)(a);
	}
};

template<typename Caller>
typename Caller::Result makeCallback(Caller caller)
{
	return typename Caller::Result(Caller::thunk, caller.object);
}




template<bool, typename = void>
struct EnableIf
{
};

template<typename T>
struct EnableIf<true, T>
{
	typedef T Type;
};


template<typename T>
struct SfinaeHelper
{
	typedef void Type;
};


template<typename T, T m>
struct SfinaeNonType
{
	typedef void Type;
};


struct TypeInfoDummy
{
	TypeInfoDummy()
	{
	}
};

template<typename>
struct TypeTag
{
	static const TypeInfoDummy c;
};

template<typename T>
const TypeInfoDummy TypeTag<T>::c;

struct TypeInfo
{
	const TypeInfoDummy* p;
	TypeInfo(const TypeInfoDummy& typeInfo)
		: p(&typeInfo)
	{
	}
};

template<typename T>
inline TypeInfo getTypeInfo()
{
	return TypeInfo(TypeTag<T>::c);
};


inline bool lessThan(const TypeInfo& l, const TypeInfo& r)
{
	return l.p < r.p;
};

inline bool isEqual(const TypeInfo& l, const TypeInfo& r)
{
	return l.p == r.p;
};

inline const TypeInfo& getTypeInfo(const TypeInfo& object)
{
	return object;
};



#endif


