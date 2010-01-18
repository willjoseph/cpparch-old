
#ifndef INCLUDED_CPPPARSE_VISITOR_H
#define INCLUDED_CPPPARSE_VISITOR_H

#define FOREACH_SIGNATURE template<typename VisitorType> void accept(VisitorType& visitor)
#define FOREACH1(a) FOREACH_SIGNATURE { visitor.visit(a); }
#define FOREACH2(a, b) FOREACH_SIGNATURE { visitor.visit(a); visitor.visit(b); }
#define FOREACH3(a, b, c) FOREACH_SIGNATURE { visitor.visit(a); visitor.visit(b); visitor.visit(c); }
#define FOREACH4(a, b, c, d) FOREACH_SIGNATURE { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); }
#define FOREACH5(a, b, c, d, e) FOREACH_SIGNATURE { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); }
#define FOREACH6(a, b, c, d, e, f) FOREACH_SIGNATURE { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); }
#define FOREACH7(a, b, c, d, e, f, g) FOREACH_SIGNATURE { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); visitor.visit(g); }
#define FOREACH8(a, b, c, d, e, f, g, h) FOREACH_SIGNATURE { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); visitor.visit(g); visitor.visit(h); }


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

struct VisitorFuncListEnd
{
	template<typename VisitorType>
	explicit VisitorFuncListEnd(const TypeId<VisitorType>&)
	{
	}
};

template<typename FuncType, typename NextType>
struct VisitorFuncList : public FuncType, public NextType
{
	template<typename VisitorType>
	explicit VisitorFuncList(const TypeId<VisitorType>& visitorType) :
	FuncType(VisitorThunk<VisitorType, typename FuncType::Type>::visit),
		NextType(visitorType)
	{
	}
};

#define VISITORFUNCLIST1(T0) VisitorFuncList<VisitorFunc<T0>, VisitorFuncListEnd>
#define VISITORFUNCLIST2(T0, T1) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST1(T1)>
#define VISITORFUNCLIST3(T0, T1, T2) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST2(T1, T2)>
#define VISITORFUNCLIST4(T0, T1, T2, T3) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST3(T1, T2, T3)>
#define VISITORFUNCLIST5(T0, T1, T2, T3, T4) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST4(T1, T2, T3, T4)>
#define VISITORFUNCLIST6(T0, T1, T2, T3, T4, T5) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST5(T1, T2, T3, T4, T5)>
#define VISITORFUNCLIST7(T0, T1, T2, T3, T4, T5, T6) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST6(T1, T2, T3, T4, T5, T6)>
#define VISITORFUNCLIST8(T0, T1, T2, T3, T4, T5, T6, T7) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST7(T1, T2, T3, T4, T5, T6, T7)>
#define VISITORFUNCLIST9(T0, T1, T2, T3, T4, T5, T6, T7, T8) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST8(T1, T2, T3, T4, T5, T6, T7, T8)>
#define VISITORFUNCLIST10(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST9(T1, T2, T3, T4, T5, T6, T7, T8, T9)>
#define VISITORFUNCLIST11(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST10(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10)>
#define VISITORFUNCLIST12(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST11(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11)>
#define VISITORFUNCLIST13(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST12(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12)>
#define VISITORFUNCLIST14(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST13(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13)>
#define VISITORFUNCLIST15(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST14(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14)>
#define VISITORFUNCLIST16(T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST15(T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15)>

#define VISITABLE_BASE(Funcs) \
	typedef Funcs VisitorFuncTable; \
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


#endif


