
#ifndef INCLUDED_CPPPARSE_CORE_OPERATORID_H
#define INCLUDED_CPPPARSE_CORE_OPERATORID_H

#include "Parse/Grammar.h"

// objects
extern Name gOperatorNewId;
extern Name gOperatorDeleteId;
extern Name gOperatorNewArrayId;
extern Name gOperatorDeleteArrayId;
extern Name gOperatorPlusId;
extern Name gOperatorMinusId;
extern Name gOperatorStarId;
extern Name gOperatorDivideId;
extern Name gOperatorPercentId;
extern Name gOperatorXorId;
extern Name gOperatorAndId;
extern Name gOperatorOrId;
extern Name gOperatorComplId;
extern Name gOperatorNotId;
extern Name gOperatorAssignId;
extern Name gOperatorLessId;
extern Name gOperatorGreaterId;
extern Name gOperatorPlusAssignId;
extern Name gOperatorMinusAssignId;
extern Name gOperatorStarAssignId;
extern Name gOperatorDivideAssignId;
extern Name gOperatorPercentAssignId;
extern Name gOperatorXorAssignId;
extern Name gOperatorAndAssignId;
extern Name gOperatorOrAssignId;
extern Name gOperatorShiftLeftId;
extern Name gOperatorShiftRightId;
extern Name gOperatorShiftRightAssignId;
extern Name gOperatorShiftLeftAssignId;
extern Name gOperatorEqualId;
extern Name gOperatorNotEqualId;
extern Name gOperatorLessEqualId;
extern Name gOperatorGreaterEqualId;
extern Name gOperatorAndAndId;
extern Name gOperatorOrOrId;
extern Name gOperatorPlusPlusId;
extern Name gOperatorMinusMinusId;
extern Name gOperatorCommaId;
extern Name gOperatorArrowStarId;
extern Name gOperatorArrowId;
extern Name gOperatorFunctionId;
extern Name gOperatorSubscriptId;

inline Name getOverloadableOperatorId(cpp::overloadable_operator_default* symbol)
{
	switch(symbol->id)
	{
	case cpp::overloadable_operator_default::ASSIGN: return gOperatorAssignId;
	case cpp::overloadable_operator_default::STARASSIGN: return gOperatorStarAssignId;
	case cpp::overloadable_operator_default::DIVIDEASSIGN: return gOperatorDivideAssignId;
	case cpp::overloadable_operator_default::PERCENTASSIGN: return gOperatorPercentAssignId;
	case cpp::overloadable_operator_default::PLUSASSIGN: return gOperatorPlusAssignId;
	case cpp::overloadable_operator_default::MINUSASSIGN: return gOperatorMinusAssignId;
	case cpp::overloadable_operator_default::SHIFTRIGHTASSIGN: return gOperatorShiftRightAssignId;
	case cpp::overloadable_operator_default::SHIFTLEFTASSIGN: return gOperatorShiftLeftAssignId;
	case cpp::overloadable_operator_default::ANDASSIGN: return gOperatorAndAssignId;
	case cpp::overloadable_operator_default::XORASSIGN: return gOperatorXorAssignId;
	case cpp::overloadable_operator_default::ORASSIGN: return gOperatorOrAssignId;
	case cpp::overloadable_operator_default::EQUAL: return gOperatorEqualId;
	case cpp::overloadable_operator_default::NOTEQUAL: return gOperatorNotEqualId;
	case cpp::overloadable_operator_default::LESS: return gOperatorLessId;
	case cpp::overloadable_operator_default::GREATER: return gOperatorGreaterId;
	case cpp::overloadable_operator_default::LESSEQUAL: return gOperatorLessEqualId;
	case cpp::overloadable_operator_default::GREATEREQUAL: return gOperatorGreaterEqualId;
	case cpp::overloadable_operator_default::ANDAND: return gOperatorAndAndId;
	case cpp::overloadable_operator_default::OROR: return gOperatorOrOrId;
	case cpp::overloadable_operator_default::PLUSPLUS: return gOperatorPlusPlusId;
	case cpp::overloadable_operator_default::MINUSMINUS: return gOperatorMinusMinusId;
	case cpp::overloadable_operator_default::STAR: return gOperatorStarId;
	case cpp::overloadable_operator_default::DIVIDE: return gOperatorDivideId;
	case cpp::overloadable_operator_default::PERCENT: return gOperatorPercentId;
	case cpp::overloadable_operator_default::PLUS: return gOperatorPlusId;
	case cpp::overloadable_operator_default::MINUS: return gOperatorMinusId;
	case cpp::overloadable_operator_default::SHIFTLEFT: return gOperatorShiftLeftId;
	case cpp::overloadable_operator_default::SHIFTRIGHT: return gOperatorShiftRightId;
	case cpp::overloadable_operator_default::AND: return gOperatorAndId;
	case cpp::overloadable_operator_default::OR: return gOperatorOrId;
	case cpp::overloadable_operator_default::XOR: return gOperatorXorId;
	case cpp::overloadable_operator_default::NOT: return gOperatorNotId;
	case cpp::overloadable_operator_default::COMPL: return gOperatorComplId;
	case cpp::overloadable_operator_default::ARROW: return gOperatorArrowId;
	case cpp::overloadable_operator_default::ARROWSTAR: return gOperatorArrowStarId;
	case cpp::overloadable_operator_default::COMMA: return gOperatorCommaId;
	default: break;
	}
	throw SymbolsError();
}

inline Name getOverloadableOperatorId(cpp::new_operator* symbol)
{
	if(symbol->array.get() != 0)
	{
		return gOperatorNewArrayId;
	}
	return gOperatorNewId;
}

inline Name getOverloadableOperatorId(cpp::delete_operator* symbol)
{
	if(symbol->array.get() != 0)
	{
		return gOperatorDeleteArrayId;
	}
	return gOperatorDeleteId;
}

inline Name getOverloadableOperatorId(cpp::function_operator* symbol)
{
	return gOperatorFunctionId;
}

inline Name getOverloadableOperatorId(cpp::array_operator* symbol)
{
	return gOperatorSubscriptId;
}

inline Name getOverloadedOperatorId(cpp::postfix_operator* symbol)
{
	switch(symbol->id)
	{
	case cpp::postfix_operator::PLUSPLUS: return gOperatorPlusPlusId;
	case cpp::postfix_operator::MINUSMINUS: return gOperatorMinusMinusId;
	default: break;
	}
	throw SymbolsError();
}

inline Name getOverloadedOperatorId(cpp::unary_operator* symbol)
{
	switch(symbol->id)
	{
	case cpp::unary_operator::PLUSPLUS: return gOperatorPlusPlusId;
	case cpp::unary_operator::MINUSMINUS: return gOperatorMinusMinusId;
	case cpp::unary_operator::STAR: return gOperatorStarId;
	case cpp::unary_operator::AND: return gOperatorAndId;
	case cpp::unary_operator::PLUS: return gOperatorPlusId;
	case cpp::unary_operator::MINUS: return gOperatorMinusId;
	case cpp::unary_operator::NOT: return gOperatorNotId;
	case cpp::unary_operator::COMPL: return gOperatorComplId;
	default: break;
	}
	throw SymbolsError();
}

inline Name getOverloadedOperatorId(cpp::pm_expression_default* symbol)
{
	if(symbol->op->id == cpp::pm_operator::ARROWSTAR)
	{
		return gOperatorArrowStarId;
	}
	return Name();
}
inline Name getOverloadedOperatorId(cpp::multiplicative_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::multiplicative_operator::STAR: return gOperatorStarId;
	case cpp::multiplicative_operator::DIVIDE: return gOperatorDivideId;
	case cpp::multiplicative_operator::PERCENT: return gOperatorPercentId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::additive_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::additive_operator::PLUS: return gOperatorPlusId;
	case cpp::additive_operator::MINUS: return gOperatorMinusId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::shift_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::shift_operator::SHIFTLEFT: return gOperatorShiftLeftId;
	case cpp::shift_operator::SHIFTRIGHT: return gOperatorShiftRightId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::relational_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::relational_operator::LESS: return gOperatorLessId;
	case cpp::relational_operator::GREATER: return gOperatorGreaterId;
	case cpp::relational_operator::LESSEQUAL: return gOperatorLessEqualId;
	case cpp::relational_operator::GREATEREQUAL: return gOperatorGreaterEqualId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::equality_expression_default* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::equality_operator::EQUAL: return gOperatorEqualId;
	case cpp::equality_operator::NOTEQUAL: return gOperatorNotEqualId;
	default: break;
	}
	throw SymbolsError();
}
inline Name getOverloadedOperatorId(cpp::and_expression_default* symbol)
{
	return gOperatorAndId;
}
inline Name getOverloadedOperatorId(cpp::exclusive_or_expression_default* symbol)
{
	return gOperatorXorId;
}

inline Name getOverloadedOperatorId(cpp::inclusive_or_expression_default* symbol)
{
	return gOperatorOrId;
}

inline Name getOverloadedOperatorId(cpp::logical_and_expression_default* symbol)
{
	return gOperatorAndAndId;
}

inline Name getOverloadedOperatorId(cpp::logical_or_expression_default* symbol)
{
	return gOperatorOrOrId;
}

inline Name getOverloadedOperatorId(cpp::assignment_expression_suffix* symbol)
{
	switch(symbol->op->id)
	{
	case cpp::assignment_operator::ASSIGN: return gOperatorAssignId;
	case cpp::assignment_operator::STAR: return gOperatorStarAssignId;
	case cpp::assignment_operator::DIVIDE: return gOperatorDivideAssignId;
	case cpp::assignment_operator::PERCENT: return gOperatorPercentAssignId;
	case cpp::assignment_operator::PLUS: return gOperatorPlusAssignId;
	case cpp::assignment_operator::MINUS: return gOperatorMinusAssignId;
	case cpp::assignment_operator::SHIFTRIGHT: return gOperatorShiftRightAssignId;
	case cpp::assignment_operator::SHIFTLEFT: return gOperatorShiftLeftAssignId;
	case cpp::assignment_operator::AND: return gOperatorAndAssignId;
	case cpp::assignment_operator::XOR: return gOperatorXorAssignId;
	case cpp::assignment_operator::OR: return gOperatorOrAssignId;
	default: break;
	}
	throw SymbolsError();
}

#endif
