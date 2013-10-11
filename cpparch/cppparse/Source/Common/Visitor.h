
#ifndef INCLUDED_CPPPARSE_COMMON_VISITOR_H
#define INCLUDED_CPPPARSE_COMMON_VISITOR_H

#include "Common.h"


template<typename T>
struct Visitable;


#define VISITABLE_ACCEPT template<typename VisitorType> void accept(VisitorType& visitor)
#define PARSEABLE_ACCEPT typedef TypeListEnd Choices; template<typename SemaT> bool parse(ParserGeneric<SemaT>& parser)
#define FOREACH1(a) VISITABLE_ACCEPT { visitor.visit(a); } \
	PARSEABLE_ACCEPT { return parser.parse(a); }
#define FOREACH2(a, b) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); } \
	PARSEABLE_ACCEPT { return parser.parse(a) && parser.parse(b); }
#define FOREACH3(a, b, c) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); } \
	PARSEABLE_ACCEPT { return parser.parse(a) && parser.parse(b) && parser.parse(c); }
#define FOREACH4(a, b, c, d) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); } \
	PARSEABLE_ACCEPT { return parser.parse(a) && parser.parse(b) && parser.parse(c) && parser.parse(d); }
#define FOREACH5(a, b, c, d, e) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); } \
	PARSEABLE_ACCEPT { return parser.parse(a) && parser.parse(b) && parser.parse(c) && parser.parse(d) && parser.parse(e); }
#define FOREACH6(a, b, c, d, e, f) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); } \
	PARSEABLE_ACCEPT { return parser.parse(a) && parser.parse(b) && parser.parse(c) && parser.parse(d) && parser.parse(e) && parser.parse(f); }
#define FOREACH7(a, b, c, d, e, f, g) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); visitor.visit(g); } \
	PARSEABLE_ACCEPT { return parser.parse(a) && parser.parse(b) && parser.parse(c) && parser.parse(d) && parser.parse(e) && parser.parse(f) && parser.parse(g); }
#define FOREACH8(a, b, c, d, e, f, g, h) VISITABLE_ACCEPT { visitor.visit(a); visitor.visit(b); visitor.visit(c); visitor.visit(d); visitor.visit(e); visitor.visit(f); visitor.visit(g); visitor.visit(h); } \
	PARSEABLE_ACCEPT { return parser.parse(a) && parser.parse(b) && parser.parse(c) && parser.parse(d) && parser.parse(e) && parser.parse(f) && parser.parse(g) && parser.parse(h); }


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
	typedef T* Type;
	typedef void (*Thunk)(void* context, Type p);
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
	void visit(Visitable<T>* p) const
	{
		static_cast<const VisitorFunc<T>*>(table)->thunk(context, p);
	}
};

template<typename T>
struct VisitorTypeId
{
	typedef T Type;
};


template<typename Types>
struct VisitorFuncGeneric : public VisitorFunc<typename Types::Item>, public VisitorFuncGeneric<typename Types::Next>
{
	typedef VisitorFunc<typename Types::Item> FuncType;
	typedef VisitorFuncGeneric<typename Types::Next> NextType;
	template<typename VisitorType>
	explicit VisitorFuncGeneric(const VisitorTypeId<VisitorType>& visitorType) :
		FuncType(VisitorThunk<VisitorType, typename FuncType::Type>::visit),
		NextType(visitorType)
	{
	}
};

template<>
struct VisitorFuncGeneric<TypeListEnd>
{
	template<typename VisitorType>
	explicit VisitorFuncGeneric(const VisitorTypeId<VisitorType>&)
	{
	}
};

#define CPPTREE_VIRTUAL
#ifdef CPPTREE_VIRTUAL

#if 0
template<typename T>
struct VisitableBase
{
	typedef typename T::Types Types;
	typedef VisitorFuncGeneric<Types> VisitorFuncTable;
	typedef VisitorCallback<VisitorFuncTable> Visitor;
	virtual void acceptAbstract(const Visitor& visitor) = 0;
	template<typename VisitorType>
	void accept(VisitorType& visitor)
	{
		static VisitorFuncTable table = VisitorFuncTable(VisitorTypeId<VisitorType>());
		Visitor callback = { &visitor, &table };
		acceptAbstract(callback);
	}
};
#endif


template<typename Op, typename Bases>
struct Compose : Op::template Apply<typename Bases::Item>::Type, Compose<Op, typename Bases::Next>
{
};

template<typename Op>
struct Compose<Op, TypeListEnd>
{
};

template<typename T, typename Base>
struct VisitableBase : Visitable<Base>
{
	virtual void acceptAbstract(const typename Base::Visitor& visitor)
	{
		visitor.visit(static_cast<Visitable<T>*>(this));
	}
};

template<typename T>
struct MakeVisitableBase
{
	template<typename Base>
	struct Apply
	{
		typedef VisitableBase<T, Base> Type;
	};
};

template<typename T, typename U = void>
struct GetBases
{
	typedef TypeListEnd Type;
};

template<typename T>
struct GetBases<T, typename SfinaeHelper<typename T::Bases>::Type>
{
	typedef typename T::Bases Type;
};


template<typename T>
struct Visitable : Compose<MakeVisitableBase<T>, typename GetBases<T>::Type>, T
{
	Visitable()
	{
	}
	Visitable(const T& value) : T(value)
	{
	}
	template<typename VisitorType>
	void accept(VisitorType& visitor)
	{
		static_cast<T*>(this)->accept(visitor);
	}
};


#define VISITABLE_BASE_IMPL \
	typedef VisitorFuncGeneric<Types> VisitorFuncTable; \
	typedef VisitorCallback<VisitorFuncTable> Visitor; \
	virtual void acceptAbstract(const Visitor& visitor) = 0; \
	template<typename VisitorType> \
	void accept(VisitorType& visitor) \
	{ \
		static VisitorFuncTable table = VisitorFuncTable(VisitorTypeId<VisitorType>()); \
		Visitor callback = { &visitor, &table }; \
		acceptAbstract(callback); \
	}

#define VISITABLE_BASE(Types_) typedef Types_ Types; typedef Types Choices; VISITABLE_BASE_IMPL
#define VISITABLE_BASE_DEFAULT(Types_) typedef Types_ Types; typedef TypeListEnd Choices; VISITABLE_BASE_IMPL


#define VISITABLE_DERIVED(Base) \
	virtual void acceptAbstract(const Base::Visitor& visitor) \
	{ \
		visitor.visit(makeVisitable(this)); \
	}

#define VISITABLE_DERIVED_TMPL(Base) \
	virtual void acceptAbstract(const typename Base::Visitor& visitor) \
	{ \
		visitor.visit(makeVisitable(this)); \
	}

#else

#define VISITABLE_BASE_DEFAULT(Types) typedef TypeListEnd Choices
#define VISITABLE_BASE(Types) typedef Types Choices
#define VISITABLE_DERIVED(Base)
#define VISITABLE_DERIVED_TMPL(Base)

#endif

#endif


