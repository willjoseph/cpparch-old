
#ifndef INCLUDED_CPPPARSE_AST_EXPRESSIONIMPL_H
#define INCLUDED_CPPPARSE_AST_EXPRESSIONIMPL_H

#include "Ast/Expression.h"
#include "Ast/Type.h"

struct SimpleType;
struct InstantiationContext;

struct Argument : ExpressionWrapper
{
	UniqueTypeWrapper type;
	Argument(ExpressionWrapper expression, UniqueTypeWrapper type)
		: ExpressionWrapper(expression), type(type)
	{
	}
};
typedef std::vector<Argument> Arguments;


struct IntegralConstantExpression
{
	UniqueTypeWrapper type;
	IntegralConstant value;
	IntegralConstantExpression(UniqueTypeWrapper type, IntegralConstant value)
		: type(type), value(value)
	{
	}
};

inline bool operator<(const IntegralConstantExpression& left, const IntegralConstantExpression& right)
{
	return left.type != right.type
		? left.type < right.type
		: left.value.value < right.value.value;
}


struct CastExpression
{
	UniqueTypeWrapper type;
	ExpressionWrapper operand;
	CastExpression(UniqueTypeWrapper type, ExpressionWrapper operand)
		: type(type), operand(operand)
	{
	}
};

inline bool operator<(const CastExpression& left, const CastExpression& right)
{
	return left.type != right.type
		? left.type < right.type
		: left.operand.p < right.operand.p;
}

inline bool isCastExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<CastExpression>);
}

inline const CastExpression& getCastExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isCastExpression(node));
	return static_cast<const ExpressionNodeGeneric<CastExpression>*>(node)->value;
}


struct DependentIdExpression
{
	Name name;
	UniqueTypeWrapper qualifying;
	TemplateArgumentsInstance templateArguments;
	DependentIdExpression(Name name, UniqueTypeWrapper qualifying, TemplateArgumentsInstance templateArguments)
		: name(name), qualifying(qualifying), templateArguments(templateArguments)
	{
		SYMBOLS_ASSERT(qualifying.value.p != 0);
	}
};

inline bool operator<(const DependentIdExpression& left, const DependentIdExpression& right)
{
	return left.name != right.name
		? left.name < right.name
		: left.qualifying != right.qualifying
		? left.qualifying < right.qualifying
		: left.templateArguments < right.templateArguments;
}

inline bool isDependentIdExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<DependentIdExpression>);
}

inline const DependentIdExpression& getDependentIdExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isDependentIdExpression(node));
	return static_cast<const ExpressionNodeGeneric<DependentIdExpression>*>(node)->value;
}


struct IdExpression
{
	DeclarationInstanceRef declaration;
	const SimpleType* enclosing;
	TemplateArgumentsInstance templateArguments;
	IdExpression(DeclarationInstanceRef declaration, const SimpleType* enclosing, const TemplateArgumentsInstance& templateArguments)
		: declaration(declaration), enclosing(enclosing), templateArguments(templateArguments)
	{
	}
};

inline bool operator<(const IdExpression& left, const IdExpression& right)
{
#if 1
	// TODO: check compliance: id-expressions cannot be compared for equivalence
	SYMBOLS_ASSERT(false);
#else
	return left.declaration.p != right.declaration.p
		? left.declaration.p < right.declaration.p
		: left.enclosing < right.enclosing;
#endif
}

inline bool isIdExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<IdExpression>);
}

inline const IdExpression& getIdExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isIdExpression(node));
	return static_cast<const ExpressionNodeGeneric<IdExpression>*>(node)->value;
}


struct NonTypeTemplateParameter
{
	DeclarationPtr declaration;
	NonTypeTemplateParameter(DeclarationPtr declaration)
		: declaration(declaration)
	{
	}
};

inline bool operator<(const NonTypeTemplateParameter& left, const NonTypeTemplateParameter& right)
{
	return left.declaration->scope->templateDepth != right.declaration->scope->templateDepth
		? left.declaration->scope->templateDepth < right.declaration->scope->templateDepth
		: left.declaration->templateParameter < right.declaration->templateParameter;
}

inline bool isNonTypeTemplateParameter(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<NonTypeTemplateParameter>);
}

inline const NonTypeTemplateParameter& getNonTypeTemplateParameter(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isNonTypeTemplateParameter(node));
	return static_cast<const ExpressionNodeGeneric<NonTypeTemplateParameter>*>(node)->value;
}

struct SizeofExpression
{
	// [expr.sizeof] The operand is ... an expression, which is not evaluated
	ExpressionWrapper operand;
	SizeofExpression(ExpressionWrapper operand)
		: operand(operand)
	{
	}
};

inline bool operator<(const SizeofExpression& left, const SizeofExpression& right)
{
	return left.operand.p < right.operand.p;
}

struct SizeofTypeExpression
{
	// [expr.sizeof] The operand is ... a parenthesized type-id
	UniqueTypeWrapper type;
	SizeofTypeExpression(UniqueTypeWrapper type)
		: type(type)
	{
	}
};

inline bool operator<(const SizeofTypeExpression& left, const SizeofTypeExpression& right)
{
	return left.type < right.type;
}


typedef IntegralConstant (*UnaryIceOp)(IntegralConstant);

struct UnaryExpression
{
	Name operatorName;
	UnaryIceOp operation;
	ExpressionWrapper first;
	UnaryExpression(Name operatorName, UnaryIceOp operation, ExpressionWrapper first)
		: operatorName(operatorName), operation(operation), first(first)
	{
	}
};

inline bool operator<(const UnaryExpression& left, const UnaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.first.p < right.first.p;
}

inline bool isUnaryExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<UnaryExpression>);
}

inline const UnaryExpression& getUnaryExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isUnaryExpression(node));
	return static_cast<const ExpressionNodeGeneric<UnaryExpression>*>(node)->value;
}

typedef IntegralConstant (*BinaryIceOp)(IntegralConstant, IntegralConstant);
typedef UniqueTypeWrapper (*BinaryTypeOp)(Name operatorName, Argument first, Argument second, const InstantiationContext& context);

struct BinaryExpression
{
	Name operatorName;
	BinaryIceOp operation;
	BinaryTypeOp type;
	ExpressionWrapper first;
	ExpressionWrapper second;
	BinaryExpression(Name operatorName, BinaryIceOp operation, BinaryTypeOp type, ExpressionWrapper first, ExpressionWrapper second)
		: operatorName(operatorName), operation(operation), type(type), first(first), second(second)
	{
	}
};

inline bool operator<(const BinaryExpression& left, const BinaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.type != right.type
		? left.type < right.type
		: left.first.p != right.first.p
		? left.first.p < right.first.p
		: left.second.p < right.second.p;
}

typedef IntegralConstant (*TernaryIceOp)(IntegralConstant, IntegralConstant, IntegralConstant);

struct TernaryExpression
{
	TernaryIceOp operation;
	ExpressionWrapper first;
	ExpressionWrapper second;
	ExpressionWrapper third;
	TernaryExpression(TernaryIceOp operation, ExpressionWrapper first, ExpressionWrapper second, ExpressionWrapper third)
		: operation(operation), first(first), second(second), third(third)
	{
	}
};

inline bool operator<(const TernaryExpression& left, const TernaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.first.p != right.first.p
		? left.first.p < right.first.p
		: left.second.p != right.second.p
		? left.second.p < right.second.p
		: left.third.p < right.third.p;
}

typedef bool (*UnaryTypeTraitsOp)(UniqueTypeWrapper);
typedef bool (*BinaryTypeTraitsOp)(UniqueTypeWrapper, UniqueTypeWrapper, const InstantiationContext& context);

struct TypeTraitsUnaryExpression
{
	Name traitName;
	UnaryTypeTraitsOp operation;
	UniqueTypeWrapper type;
	TypeTraitsUnaryExpression(Name traitName, UnaryTypeTraitsOp operation, UniqueTypeWrapper type)
		: traitName(traitName), operation(operation), type(type)
	{
	}
};

inline bool operator<(const TypeTraitsUnaryExpression& left, const TypeTraitsUnaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.type < right.type;
}

struct TypeTraitsBinaryExpression
{
	Name traitName;
	BinaryTypeTraitsOp operation;
	UniqueTypeWrapper first;
	UniqueTypeWrapper second;
	TypeTraitsBinaryExpression(Name traitName, BinaryTypeTraitsOp operation, UniqueTypeWrapper first, UniqueTypeWrapper second)
		: traitName(traitName), operation(operation), first(first), second(second)
	{
		SYMBOLS_ASSERT(first != gUniqueTypeNull);
		SYMBOLS_ASSERT(second != gUniqueTypeNull);
	}
};

inline bool operator<(const TypeTraitsBinaryExpression& left, const TypeTraitsBinaryExpression& right)
{
	return left.operation != right.operation
		? left.operation < right.operation
		: left.first != right.first
		? left.first < right.first
		: left.second < right.second;
}



struct ExplicitTypeExpression
{
	UniqueTypeWrapper type;
	ExplicitTypeExpression(UniqueTypeWrapper type)
		: type(type)
	{
	}
};

inline bool operator<(const ExplicitTypeExpression& left, const ExplicitTypeExpression& right)
{
	return left.type < right.type;
}

inline bool isExplicitTypeExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<ExplicitTypeExpression>);
}

inline const ExplicitTypeExpression& getExplicitTypeExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isExplicitTypeExpression(node));
	return static_cast<const ExpressionNodeGeneric<ExplicitTypeExpression>*>(node)->value;
}


struct DependentObjectExpression
{
	ExpressionWrapper left;
	bool isArrow;
	DependentObjectExpression(ExpressionWrapper left, bool isArrow)
		: left(left), isArrow(isArrow)
	{
	}
};

inline bool operator<(const DependentObjectExpression& left, const DependentObjectExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}

inline bool isDependentObjectExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<DependentObjectExpression>);
}

inline const DependentObjectExpression& getDependentObjectExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isDependentObjectExpression(node));
	return static_cast<const ExpressionNodeGeneric<DependentObjectExpression>*>(node)->value;
}


struct ObjectExpression
{
	const SimpleType* classType;
	ObjectExpression(const SimpleType* classType)
		: classType(classType)
	{
	}
};

inline bool operator<(const ObjectExpression& left, const ObjectExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}

inline bool isObjectExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<ObjectExpression>);
}

inline const ObjectExpression& getObjectExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isObjectExpression(node));
	return static_cast<const ExpressionNodeGeneric<ObjectExpression>*>(node)->value;
}


struct ClassMemberAccessExpression
{
	ExpressionWrapper left; // ObjectExpression or DependentObjectExpression
	ExpressionWrapper right; // IdExpression or DependentIdExpression
	ClassMemberAccessExpression(ExpressionWrapper left, ExpressionWrapper right)
		: left(left), right(right)
	{
	}
};

inline bool operator<(const ClassMemberAccessExpression& left, const ClassMemberAccessExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}

inline bool isClassMemberAccessExpression(ExpressionNode* node)
{
	return typeid(*node) == typeid(ExpressionNodeGeneric<ClassMemberAccessExpression>);
}

inline const ClassMemberAccessExpression& getClassMemberAccessExpression(ExpressionNode* node)
{
	SYMBOLS_ASSERT(isClassMemberAccessExpression(node));
	return static_cast<const ExpressionNodeGeneric<ClassMemberAccessExpression>*>(node)->value;
}


// id-expression ( expression-list )
// overload resolution is required if the lefthand side is
// - a (parenthesised) id-expression
// - of class type
struct FunctionCallExpression
{
	ExpressionWrapper left; // TODO: extract memberClass, id, idEnclosing, type, templateArguments
	Arguments arguments;
	FunctionCallExpression(ExpressionWrapper left, Arguments arguments)
		: left(left), arguments(arguments)
	{
	}
};

inline bool operator<(const FunctionCallExpression& left, const FunctionCallExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}


struct SubscriptExpression
{
	ExpressionWrapper left;
	ExpressionWrapper right;
	SubscriptExpression(ExpressionWrapper left, ExpressionWrapper right)
		: left(left), right(right)
	{
	}
};

inline bool operator<(const SubscriptExpression& left, const SubscriptExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}


struct PostfixOperatorExpression
{
	Name operatorName;
	ExpressionWrapper operand;
	PostfixOperatorExpression(Name operatorName, ExpressionWrapper operand)
		: operatorName(operatorName), operand(operand)
	{
	}
};

inline bool operator<(const PostfixOperatorExpression& left, const PostfixOperatorExpression& right)
{
	SYMBOLS_ASSERT(false);
	return false;
}


inline UniqueTypeWrapper typeOfExpression(ExpressionNode* node, const InstantiationContext& context);

inline bool isPointerToMemberExpression(ExpressionNode* expression)
{
	if(!isUnaryExpression(expression))
	{
		return false;
	}
	const UnaryExpression& unary = getUnaryExpression(expression);
	extern Name gOperatorAndId;
	if(unary.operatorName != gOperatorAndId
		|| !isIdExpression(unary.first))
	{
		return false;
	}
	return getIdExpression(unary.first).enclosing != 0;
}

inline bool isPointerToFunctionExpression(ExpressionNode* expression)
{
	if(isUnaryExpression(expression))
	{
		expression = getUnaryExpression(expression).first;
	}
	if(!isIdExpression(expression))
	{
		return false;
	}
	const IdExpression node = getIdExpression(expression);
	return UniqueTypeWrapper(node.declaration->type.unique).isFunction();
}

inline bool isDependentPointerToMemberExpression(ExpressionNode* expression)
{
	extern Name gOperatorAndId;

	return isUnaryExpression(expression)
		&& getUnaryExpression(expression).operatorName == gOperatorAndId
		&& isDependentIdExpression(getUnaryExpression(expression).first);
}


#endif
