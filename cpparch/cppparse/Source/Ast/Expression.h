
#ifndef INCLUDED_CPPPARSE_AST_EXPRESSION_H
#define INCLUDED_CPPPARSE_AST_EXPRESSION_H

#include "Common/Allocator.h"
#include "Common/IndirectSet.h"
#include "UniqueType.h"

// [expr.const]
// An integral constant-expression can involve only literals, enumerators, const variables or static
// data members of integral or enumeration types initialized with constant expressions, non-type template
// parameters of integral or enumeration types, and sizeof expressions
struct ExpressionNodeVisitor
{
	virtual void visit(const struct IntegralConstantExpression&) = 0; // literal
	virtual void visit(const struct CastExpression&) = 0;
	virtual void visit(const struct NonTypeTemplateParameter&) = 0; // non-type template parameter
	virtual void visit(const struct DependentIdExpression&) = 0; // T::name
	virtual void visit(const struct IdExpression&) = 0; // enumerator, const variable, static data member, non-type template parameter
	virtual void visit(const struct SizeofExpression&) = 0;
	virtual void visit(const struct SizeofTypeExpression&) = 0;
	virtual void visit(const struct UnaryExpression&) = 0;
	virtual void visit(const struct BinaryExpression&) = 0;
	virtual void visit(const struct TernaryExpression&) = 0;
	virtual void visit(const struct TypeTraitsUnaryExpression&) = 0;
	virtual void visit(const struct TypeTraitsBinaryExpression&) = 0;
	virtual void visit(const struct ExplicitTypeExpression&) = 0;
	virtual void visit(const struct ObjectExpression&) = 0; // transformed 'c.'
	virtual void visit(const struct DependentObjectExpression&) = 0; // 'dependent->' or 'dependent.'
	virtual void visit(const struct ClassMemberAccessExpression&) = 0;
	virtual void visit(const struct FunctionCallExpression&) = 0;
	virtual void visit(const struct SubscriptExpression&) = 0;
	virtual void visit(const struct PostfixOperatorExpression&) = 0;
};

struct ExpressionNode : TypeInfo
{
	ExpressionNode(TypeInfo type) : TypeInfo(type)
	{
	}
	virtual ~ExpressionNode()
	{
	}
	virtual void accept(ExpressionNodeVisitor& visitor) const = 0;
	virtual bool operator<(const ExpressionNode& other) const = 0;
};

typedef SafePtr<ExpressionNode> ExpressionPtr;

template<typename T>
struct ExpressionNodeGeneric : ExpressionNode
{
	T value;
	ExpressionNodeGeneric(const T& value)
		: ExpressionNode(getTypeInfo<ExpressionNodeGeneric>()), value(value)
	{
	}
	void accept(ExpressionNodeVisitor& visitor) const
	{
		visitor.visit(value);
	}
	bool operator<(const ExpressionNodeGeneric& other) const
	{
		return value < other.value;
	}
	bool operator<(const ExpressionNode& other) const
	{
		return abstractLess(*this, other);
	}
};

typedef ExpressionNode* UniqueExpression;

typedef IndirectSet<UniqueExpression> UniqueExpressions;

extern UniqueExpressions gBuiltInExpressions;
extern UniqueExpressions gUniqueExpressions;

template<typename T>
inline UniqueExpression makeBuiltInExpression(const T& value)
{
	ExpressionNodeGeneric<T> node(value);
	return *gBuiltInExpressions.insert(node);
}

template<typename T>
inline UniqueExpression makeUniqueExpression(const T& value)
{
	ExpressionNodeGeneric<T> node(value);
	{
		UniqueExpressions::iterator i = gBuiltInExpressions.find(node);
		if(i != gBuiltInExpressions.end())
		{
			return *i;
		}
	}
	return *gUniqueExpressions.insert(node);
}




// ----------------------------------------------------------------------------
// [expr.const]
struct IntegralConstant
{
	int value;
	IntegralConstant() : value(0)
	{
	}
	explicit IntegralConstant(int value) : value(value)
	{
	}
	explicit IntegralConstant(double value) : value(int(value))
	{
	}
	explicit IntegralConstant(size_t value) : value(int(value))
	{
	}
};


struct ExpressionType : UniqueTypeWrapper
{
	bool isLvalue; // true if the expression is an lvalue
	ExpressionType() : isLvalue(false)
	{
	}
	ExpressionType(UniqueTypeWrapper type, bool isLvalue)
		: UniqueTypeWrapper(type), isLvalue(isLvalue)
	{
	}
};

struct ExpressionWrapper : ExpressionPtr
{
	ExpressionType type; // valid if this expression is not type-dependent
	IntegralConstant value; // valid if this is expression is integral-constant and not value-dependent
	bool isConstant;
	bool isTypeDependent;
	bool isValueDependent;
	bool isNullPointerConstant;
	bool isTemplateArgumentAmbiguity; // [temp.arg] In a template argument, an ambiguity between a typeid and an expression is resolved to a typeid
	bool isNonStaticMemberName;
	bool isQualifiedNonStaticMemberName;
	bool isParenthesised; // true if the expression is surrounded by one or more sets of parentheses
	ExpressionWrapper()
		: ExpressionPtr(0), isConstant(false), isTypeDependent(false), isValueDependent(false), isNullPointerConstant(false), isTemplateArgumentAmbiguity(false), isNonStaticMemberName(false), isQualifiedNonStaticMemberName(false), isParenthesised(false)
	{
	}
	explicit ExpressionWrapper(ExpressionNode* node, bool isConstant = true, bool isTypeDependent = false, bool isValueDependent = false)
		: ExpressionPtr(node), isConstant(isConstant), isTypeDependent(isTypeDependent), isValueDependent(isValueDependent), isNullPointerConstant(false), isTemplateArgumentAmbiguity(false), isNonStaticMemberName(false), isQualifiedNonStaticMemberName(false), isParenthesised(false)
	{
	}
};

#endif
