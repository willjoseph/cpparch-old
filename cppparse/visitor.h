
#ifndef INCLUDED_CPPPARSE_VISITOR_H
#define INCLUDED_CPPPARSE_VISITOR_H


struct TypeListEnd
{
};

template<typename ItemType, typename NextType>
struct TypeList
{
	typedef ItemType Item;
	typedef NextType Next;
};

#define TYPELIST1(T0) TypeList<T0, TypeListEnd>
#define TYPELIST2(T0, T1) TypeList<T0, TYPELIST1(T1)>
#define TYPELIST3(T0, T1, T2) TypeList<T0, TYPELIST2(T1, T2)>
#define TYPELIST4(T0, T1, T2, T3) TypeList<T0, TYPELIST3(T1, T2, T3)>
#define TYPELIST5(T0, T1, T2, T3, T4) TypeList<T0, TYPELIST4(T1, T2, T3, T4)>
#define TYPELIST6(T0, T1, T2, T3, T4, T5) TypeList<T0, TYPELIST5(T1, T2, T3, T4, T5)>
#define TYPELIST7(T0, T1, T2, T3, T4, T5, T6) TypeList<T0, TYPELIST6(T1, T2, T3, T4, T5, T6)>
#define TYPELIST8(T0, T1, T2, T3, T4, T5, T6, T7) TypeList<T0, TYPELIST7(T1, T2, T3, T4, T5, T6, T7)>
#define TYPELIST9(T0, T1, T2, T3, T4, T5, T6, T7, T8) TypeList<T0, TYPELIST8(T1, T2, T3, T4, T5, T6, T7, T8)>
#define TYPELIST10(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9) TypeList<T0, TYPELIST9(T1, T2, T3, T4, T5, T6, T7, T8, T9)>
#define TYPELIST11(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) TypeList<T0, TYPELIST10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10)>
#define TYPELIST12(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) TypeList<T0, TYPELIST11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11)>
#define TYPELIST13(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) TypeList<T0, TYPELIST12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)>
#define TYPELIST14(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) TypeList<T0, TYPELIST13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13)>
#define TYPELIST15(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) TypeList<T0, TYPELIST14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14)>
#define TYPELIST16(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) TypeList<T0, TYPELIST15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15)>



#define VISITABLE_ACCEPT template<typename VisitorType> void accept(VisitorType& visitor)
#define PARSEABLE_ACCEPT typedef TypeListEnd Choices; template<typename ParserType> bool parse(ParserType& parser)
#define FOREACH1(a) VISITABLE_ACCEPT { visitor.visit(a); } \
	PARSEABLE_ACCEPT { return parser.visit(a); }
#define FOREACH2(a, b) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); } \
	PARSEABLE_ACCEPT { return parser.visit(a) && parser.visit(b); }
#define FOREACH3(a, b, c) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); } \
	PARSEABLE_ACCEPT { return parser.visit(a) && parser.visit(b) && parser.visit(c); }
#define FOREACH4(a, b, c, d) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); } \
	PARSEABLE_ACCEPT { return parser.visit(a) && parser.visit(b) && parser.visit(c) && parser.visit(d); }
#define FOREACH5(a, b, c, d, e) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); } \
	PARSEABLE_ACCEPT { return parser.visit(a) && parser.visit(b) && parser.visit(c) && parser.visit(d) && parser.visit(e); }
#define FOREACH6(a, b, c, d, e, f) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); } \
	PARSEABLE_ACCEPT { return parser.visit(a) && parser.visit(b) && parser.visit(c) && parser.visit(d) && parser.visit(e) && parser.visit(f); }
#define FOREACH7(a, b, c, d, e, f, g) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); visitor.visit(g); } \
	PARSEABLE_ACCEPT { return parser.visit(a) && parser.visit(b) && parser.visit(c) && parser.visit(d) && parser.visit(e) && parser.visit(f) && parser.visit(g); }
#define FOREACH8(a, b, c, d, e, f, g, h) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); visitor.visit(g); visitor.visit(h); } \
	PARSEABLE_ACCEPT { return parser.visit(a) && parser.visit(b) && parser.visit(c) && parser.visit(d) && parser.visit(e) && parser.visit(f) && parser.visit(g) && parser.visit(h); }


template<typename VisitorType, typename T>
struct VisitorThunk
{
	static void visit(void* context, T p)
	{
		static_cast<VisitorType*>(context)->visit(p);
	}
};

template<typename T>
struct VisitorFunc
{
	typedef T Type;
	typedef void (*Thunk)(void* context, T p);
	Thunk thunk;
	explicit VisitorFunc(Thunk thunk)
		: thunk(thunk)
	{
	}
};

template<typename VisitorFuncTableType>
struct VisitorCallback
{
	void* context;
	VisitorFuncTableType* table;

	template<typename T>
	void visit(T p) const
	{
		static_cast<const VisitorFunc<T>*>(table)->thunk(context, p);
	}
};

template<typename T>
struct TypeId
{
	typedef T Type;
};


template<typename Types>
struct VisitorFuncGeneric : public VisitorFunc<typename Types::Item*>, public VisitorFuncGeneric<typename Types::Next>
{
	typedef VisitorFunc<typename Types::Item*> FuncType;
	typedef VisitorFuncGeneric<typename Types::Next> NextType;
	template<typename VisitorType>
	explicit VisitorFuncGeneric(const TypeId<VisitorType>& visitorType) :
		FuncType(VisitorThunk<VisitorType, typename FuncType::Type>::visit),
		NextType(visitorType)
	{
	}
};

template<>
struct VisitorFuncGeneric<TypeListEnd>
{
	template<typename VisitorType>
	explicit VisitorFuncGeneric(const TypeId<VisitorType>&)
	{
	}
};


#define VISITABLE_BASE(Types) \
	typedef Types Choices; \
	typedef VisitorFuncGeneric<Choices> VisitorFuncTable; \
	typedef VisitorCallback<VisitorFuncTable> Visitor; \
	virtual void acceptAbstract(const Visitor& visitor) = 0; \
	template<typename VisitorType> \
	void accept(VisitorType& visitor) \
	{ \
		static VisitorFuncTable table = VisitorFuncTable(TypeId<VisitorType>()); \
		Visitor callback = { &visitor, &table }; \
		acceptAbstract(callback); \
	}

#define VISITABLE_DERIVED(Base) \
	virtual void acceptAbstract(const Base::Visitor& visitor) \
	{ \
		visitor.visit(this); \
	}

#define VISITABLE_DERIVED_TMPL(Base) \
	virtual void acceptAbstract(const typename Base::Visitor& visitor) \
{ \
	visitor.visit(this); \
}

#endif


