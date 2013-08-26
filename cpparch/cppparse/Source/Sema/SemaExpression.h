
#ifndef INCLUDED_CPPPARSE_SEMA_SEMAEXPRESSION_H
#define INCLUDED_CPPPARSE_SEMA_SEMAEXPRESSION_H

#include "SemaCommon.h"
#include "SemaPostfixExpression.h"

struct SemaExplicitTypeExpression : public SemaBase, SemaExplicitTypeExpressionResult
{
	SEMA_BOILERPLATE;

	SemaExplicitTypeExpression(const SemaState& state)
		: SemaBase(state), SemaExplicitTypeExpressionResult(context)
	{
	}
	SEMA_POLICY(cpp::simple_type_specifier, SemaPolicyPush<struct SemaTypeSpecifier>)
	void action(cpp::simple_type_specifier* symbol, const SemaTypeSpecifierResult& walker)
	{
		type = walker.type;
		if(type.declaration == 0)
		{
			type = getFundamentalType(walker.fundamental);
		}
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type);
	}
	SEMA_POLICY(cpp::typename_specifier, SemaPolicyPush<struct SemaTypenameSpecifier>)
	void action(cpp::typename_specifier* symbol, const SemaTypenameSpecifierResult& walker)
	{
		type = walker.type;
		addDependent(typeDependent, type);
		makeUniqueTypeSafe(type);
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		type = walker.type;
		addDependent(typeDependent, type);
	}
	SEMA_POLICY(cpp::new_type, SemaPolicyPush<struct SemaNewType>)
	void action(cpp::new_type* symbol, const SemaNewTypeResult& walker)
	{
		type = walker.type;
		addDependent(typeDependent, type);
		addDependent(typeDependent, walker.valueDependent);
		makeUniqueTypeSafe(type);
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
	SEMA_POLICY(cpp::cast_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::cast_expression* symbol, const SemaExpressionResult& walker)
	{
		expression = walker.expression;
		addDependent(valueDependent, walker.valueDependent);
	}
};


struct SemaSizeofTypeExpression : public SemaBase
{
	SEMA_BOILERPLATE;

	Dependent valueDependent;
	ExpressionWrapper expression;
	SemaSizeofTypeExpression(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::type_id, SemaPolicyPushCommit<struct SemaTypeId>)
	void action(cpp::type_id* symbol, const SemaTypeIdResult& walker)
	{
		walker.committed.test();
		// [temp.dep.expr] Expressions of the following form [sizeof(T)] are never type-dependent (because the type of the expression cannot be dependent)
		// [temp.dep.constexpr] Expressions of the following form [sizeof(T)] are value-dependent if ... the type-id is dependent
		addDependent(valueDependent, walker.type);

		UniqueTypeId type = getUniqueTypeSafe(walker.type);
		setExpressionType(symbol, type);

		expression = makeExpression(SizeofTypeExpression(type), true, false, isDependent(valueDependent));
	}
};

struct SemaConditionalExpression : public SemaBase
{
	SEMA_BOILERPLATE;

	Dependent typeDependent;
	Dependent valueDependent;
	ExpressionWrapper left;
	ExpressionWrapper right;
	UniqueTypeWrapper leftType;
	UniqueTypeWrapper rightType;
	SemaConditionalExpression(const SemaState& state)
		: SemaBase(state)
	{
	}
	SEMA_POLICY(cpp::expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::expression* symbol, const SemaExpressionResult& walker)
	{
		left = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		leftType = walker.type;
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPush<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker)
	{
		right = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		rightType = walker.type;
	}
};

struct SemaExpression : public SemaBase, SemaExpressionResult
{
	SEMA_BOILERPLATE;

	SemaExpression(const SemaState& state)
		: SemaBase(state)
	{
	}

	// this path handles the right-hand side of a binary expression
	// it is assumed that 'type' already contains the type of the left-hand side
	template<BuiltInBinaryTypeOp typeOp, typename T>
	void walkBinaryExpression(T*& symbol, const SemaExpressionResult& walker)
	{
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		// TODO: SEMANTIC_ASSERT(type.declaration != 0 && walker.type.declaration != 0);
		BinaryIceOp iceOp = getBinaryIceOp(symbol);
		ExpressionWrapper leftExpression = expression;
		expression = makeExpression(BinaryExpression(getOverloadedOperatorId(symbol), iceOp, typeOfBinaryExpression<typeOp>, expression, walker.expression),
			expression.isConstant && walker.expression.isConstant && iceOp != 0,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		if(!expression.isTypeDependent)
		{
			UniqueTypeWrapper left = removeReference(type);
			UniqueTypeWrapper right = removeReference(walker.type);
			type = typeOfBinaryExpression<typeOp>(getOverloadedOperatorId(symbol),
				Argument(leftExpression, left), Argument(walker.expression, right),
				getInstantiationContext());
			SYMBOLS_ASSERT(type != gUniqueTypeNull);
		}
		ExpressionType<T>::set(symbol, type);
	}
	template<typename T>
	void walkBinaryArithmeticExpression(T* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorArithmeticType>(symbol, walker);
	}
	template<typename T>
	void walkBinaryAdditiveExpression(T* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorAdditiveType>(symbol, walker);
	}
	template<typename T>
	void walkBinaryIntegralExpression(T* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorIntegralType>(symbol, walker);
	}
	template<typename T>
	void walkBinaryBooleanExpression(T* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorBoolean>(symbol, walker);
	}
	SEMA_POLICY(cpp::assignment_expression_suffix, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::assignment_expression_suffix* symbol, const SemaExpressionResult& walker)
	{
		// 5.1.7 Assignment operators
		// the type of an assignment expression is that of its left operand
		walkBinaryExpression<binaryOperatorAssignment>(symbol, walker);
	}
	SEMA_POLICY(cpp::conditional_expression_suffix, SemaPolicyPush<struct SemaConditionalExpression>)
	void action(cpp::conditional_expression_suffix* symbol, const SemaConditionalExpression& walker)
	{
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = makeExpression(TernaryExpression(conditional, expression, walker.left, walker.right),
			expression.isConstant && walker.left.isConstant && walker.right.isConstant,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		if(!expression.isTypeDependent)
		{
			type = getConditionalOperatorType(removeReference(walker.leftType), removeReference(walker.rightType));
		}
	}
	SEMA_POLICY(cpp::logical_or_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::logical_or_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::logical_and_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::logical_and_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryBooleanExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::inclusive_or_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::inclusive_or_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::exclusive_or_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::exclusive_or_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::and_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::and_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::equality_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::equality_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryBooleanExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::relational_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::relational_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryBooleanExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::shift_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::shift_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryIntegralExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::additive_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::additive_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryAdditiveExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::multiplicative_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::multiplicative_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryArithmeticExpression(symbol, walker);
	}
	SEMA_POLICY(cpp::pm_expression_default, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::pm_expression_default* symbol, const SemaExpressionResult& walker)
	{
		walkBinaryExpression<binaryOperatorMemberPointer>(symbol, walker);
		id = 0; // not a parenthesised id-expression, expression is not 'call to named function' [over.call.func]
	}
	SEMA_POLICY(cpp::assignment_expression, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::assignment_expression* symbol, const SemaExpressionResult& walker) // expression_list, assignment_expression_suffix, conditional_expression_suffix
	{
		// [expr.comma] The type and value of the result are the type and value of the right operand
		expression = walker.expression;
		type = walker.type;
		id = walker.id;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		setExpressionType(symbol, type);
	}
	SEMA_POLICY(cpp::expression_list, SemaPolicyIdentity)
	void action(cpp::expression_list* symbol) // a comma-separated list of assignment_expression
	{
		setExpressionType(symbol, type);
	}
	SEMA_POLICY(cpp::postfix_expression, SemaPolicyPushSrcChecked<struct SemaPostfixExpression>)
	bool action(cpp::postfix_expression* symbol, const SemaPostfixExpression& walker)
	{
		if(walker.isUndeclared)
		{
			return reportIdentifierMismatch(symbol, *id, &gUndeclared, "object-name");
		}
		id = walker.id;
		type = walker.type;
		expression = walker.expression;
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		//setDependent(dependent, walker.dependent); // TODO:
		setExpressionType(symbol, type);
		return true;
	}
	SEMA_POLICY(cpp::unary_expression_op, SemaPolicyIdentity)
	void action(cpp::unary_expression_op* symbol)
	{
		id = 0; // not a parenthesised id-expression, expression is not 'call to named function' [over.call.func]
		if(!isDependent(typeDependent))
		{
			SEMANTIC_ASSERT(type != gUniqueTypeNull);
			SEMANTIC_ASSERT(!::isDependent(type)); // can't resolve operator overloads if type is dependent
			SEMANTIC_ASSERT(getQualifyingScope() == 0);
			SEMANTIC_ASSERT(!(objectExpression.p != 0 && memberClass != 0)); // unary expression should not occur during class-member-access

			type = removeReference(type);
			type = typeOfUnaryExpression(
				getOverloadedOperatorId(symbol->op),
				Argument(expression, type),
				getInstantiationContext());
			// TODO: decorate parse-tree with declaration
		}
		else
		{
			type = gUniqueTypeNull;
		}


		UnaryIceOp iceOp = getUnaryIceOp(symbol);
		expression = makeExpression(UnaryExpression(getOverloadedOperatorId(symbol->op), iceOp, expression),
			expression.isConstant && iceOp != 0,
			isDependent(typeDependent),
			isDependent(valueDependent)
		);
		setExpressionType(symbol, type);
	}
	/* 14.6.2.2-3
	Expressions of the following forms are type-dependent only if the type specified by the type-id, simple-type-specifier
	or new-type-id is dependent, even if any subexpression is type-dependent:
	- postfix-expression-construct
	- new-expression
	- postfix-expression-cast
	- cast-expression
	*/
	/* temp.dep.constexpr
	Expressions of the following form are value-dependent if either the type-id or simple-type-specifier is dependent or the
	expression or cast-expression is value-dependent:
	simple-type-specifier ( expression-listopt )
	static_cast < type-id > ( expression )
	const_cast < type-id > ( expression )
	reinterpret_cast < type-id > ( expression )
	( type-id ) cast-expression
	*/
	SEMA_POLICY(cpp::new_expression_placement, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::new_expression_placement* symbol, const SemaExplicitTypeExpressionResult& walker)
	{
		type = getUniqueTypeSafe(walker.type);
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		requireCompleteObjectType(type, getInstantiationContext());
		type.push_front(PointerType());
		addDependent(typeDependent, walker.typeDependent);
		expression = makeExpression(ExplicitTypeExpression(type), false, isDependent(typeDependent));
		setExpressionType(symbol, type);
	}
	SEMA_POLICY(cpp::new_expression_default, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::new_expression_default* symbol, const SemaExplicitTypeExpressionResult& walker)
	{
		type = getUniqueTypeSafe(walker.type);
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		requireCompleteObjectType(type, getInstantiationContext());
		type.push_front(PointerType());
		addDependent(typeDependent, walker.typeDependent);
		expression = makeExpression(ExplicitTypeExpression(type), false, isDependent(typeDependent));
		setExpressionType(symbol, type);
	}
	SEMA_POLICY(cpp::cast_expression_default, SemaPolicyPushSrc<struct SemaExplicitTypeExpression>)
	void action(cpp::cast_expression_default* symbol, const SemaExplicitTypeExpressionResult& walker)
	{
		type = getUniqueTypeSafe(walker.type);
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		requireCompleteObjectType(type, getInstantiationContext());
		Dependent tmp(walker.typeDependent);
		addDependent(valueDependent, tmp);
		addDependent(typeDependent, walker.typeDependent);
		addDependent(valueDependent, walker.valueDependent);
		expression = makeExpression(CastExpression(type, walker.expression), walker.expression.isConstant, isDependent(typeDependent), isDependent(valueDependent));
		setExpressionType(symbol, type);
	}
	/* 14.6.2.2-4
	Expressions of the following forms are never type-dependent (because the type of the expression cannot be
	dependent):
	literal
	postfix-expression . pseudo-destructor-name
	postfix-expression -> pseudo-destructor-name
	sizeof unary-expression
	sizeof ( type-id )
	sizeof ... ( identifier )
	alignof ( type-id )
	typeid ( expression )
	typeid ( type-id )
	::opt delete cast-expression
	::opt delete [ ] cast-expression
	throw assignment-expressionopt
	*/
	// TODO: destructor-call is not dependent
	/* temp.dep.constexpr
	Expressions of the following form are value-dependent if the unary-expression is type-dependent or the type-id is dependent
	(even if sizeof unary-expression and sizeof ( type-id ) are not type-dependent):
	sizeof unary-expression
	sizeof ( type-id )
	*/
	SEMA_POLICY(cpp::unary_expression_sizeof, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::unary_expression_sizeof* symbol, const SemaExpressionResult& walker)
	{
		// [temp.dep.expr] Expressions of the following form [sizeof(expr)] are never type-dependent (because the type of the expression cannot be dependent)
		// [temp.dep.constexpr] Expressions of the following form [sizeof(expr)] are value-dependent if the unary-expression is type-dependent
		addDependent(valueDependent, walker.typeDependent);
		type = gUnsignedInt;
		setExpressionType(symbol, type);
		expression = makeExpression(SizeofExpression(walker.expression), true, false, isDependent(valueDependent));
	}
	SEMA_POLICY(cpp::unary_expression_sizeoftype, SemaPolicyPushSrc<struct SemaSizeofTypeExpression>)
	void action(cpp::unary_expression_sizeoftype* symbol, const SemaSizeofTypeExpression& walker)
	{
		addDependent(valueDependent, walker.valueDependent);
		type = gUnsignedInt;
		setExpressionType(symbol, type);
		expression = walker.expression;
	}
	SEMA_POLICY(cpp::delete_expression, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::delete_expression* symbol, const SemaExpressionResult& walker)
	{
		type = gVoid; // TODO: check compliance: type of delete-expression
		setExpressionType(symbol, type);
		expression = ExpressionWrapper();
	}
	SEMA_POLICY(cpp::throw_expression, SemaPolicyPushSrc<struct SemaExpression>)
	void action(cpp::throw_expression* symbol, const SemaExpressionResult& walker)
	{
		type = gVoid; // [except] A throw-expression is of type void.
		expression = ExpressionWrapper();
	}
};

#endif
