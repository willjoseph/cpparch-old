
#ifndef INCLUDED_CPPPARSE_PARSE_SPECIAL_H
#define INCLUDED_CPPPARSE_PARSE_SPECIAL_H

#include "Parse.h"

struct DefaultParser
{

template<typename SemaT, typename T>
static T* parseSymbol(ParserGeneric<SemaT>& parser, T* result)
{
	if(!result->parse(parser))
	{
#if 0
		std::cout << "rejected: '" << SYMBOL_NAME(T) << "'" << std::endl;
		printSequence(parser.context); // rejected tokens
#endif
		return parser.skip ? result : 0;
	}
	return result;
}

template<typename SemaT>
inline cpp::identifier* parseSymbol(ParserGeneric<SemaT>& parser, cpp::identifier* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_IDENTIFIER))
	{
		result->value.value = parser.get_value();
		result->value.source = parser.get_source();
		parser.increment();
		return result;
	}
	return NULL;
}


template<typename SemaT>
inline cpp::string_literal* parseSymbol(ParserGeneric<SemaT>& parser, cpp::string_literal* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_STRINGLIT))
	{
		result->value.value = parser.get_value();
		parser.increment();
		PARSE_OPTIONAL(parser, result->next);
		return result;
	}
	return NULL;
}

template<typename SemaT>
inline cpp::numeric_literal* parseSymbol(ParserGeneric<SemaT>& parser, cpp::numeric_literal* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PP_NUMBER, cpp::numeric_literal::UNKNOWN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INTLIT, cpp::numeric_literal::INTEGER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CHARLIT, cpp::numeric_literal::CHARACTER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FLOATLIT, cpp::numeric_literal::FLOATING);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TRUE, cpp::numeric_literal::BOOLEAN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FALSE, cpp::numeric_literal::BOOLEAN);
	return result;
}

template<typename SemaT>
inline cpp::cv_qualifier* parseSymbol(ParserGeneric<SemaT>& parser, cpp::cv_qualifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONST, cpp::cv_qualifier::CONST);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VOLATILE, cpp::cv_qualifier::VOLATILE);
	return result;
}

template<typename SemaT>
inline cpp::function_specifier* parseSymbol(ParserGeneric<SemaT>& parser, cpp::function_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INLINE, cpp::function_specifier::INLINE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EXPLICIT, cpp::function_specifier::EXPLICIT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VIRTUAL, cpp::function_specifier::VIRTUAL);
	return result;
}

template<typename SemaT>
inline cpp::storage_class_specifier* parseSymbol(ParserGeneric<SemaT>& parser, cpp::storage_class_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REGISTER, cpp::storage_class_specifier::REGISTER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATIC, cpp::storage_class_specifier::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EXTERN, cpp::storage_class_specifier::EXTERN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MUTABLE, cpp::storage_class_specifier::MUTABLE);
	return result;
}

template<typename SemaT>
inline cpp::simple_type_specifier_builtin* parseSymbol(ParserGeneric<SemaT>& parser, cpp::simple_type_specifier_builtin* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CHAR, cpp::simple_type_specifier_builtin::CHAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_WCHART, cpp::simple_type_specifier_builtin::WCHAR_T);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_BOOL, cpp::simple_type_specifier_builtin::BOOL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHORT, cpp::simple_type_specifier_builtin::SHORT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INT, cpp::simple_type_specifier_builtin::INT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LONG, cpp::simple_type_specifier_builtin::LONG);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SIGNED, cpp::simple_type_specifier_builtin::SIGNED);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_UNSIGNED, cpp::simple_type_specifier_builtin::UNSIGNED);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FLOAT, cpp::simple_type_specifier_builtin::FLOAT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOUBLE, cpp::simple_type_specifier_builtin::DOUBLE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VOID, cpp::simple_type_specifier_builtin::VOID);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AUTO, cpp::simple_type_specifier_builtin::AUTO);

	// not handled as macros: types may be used as parameters in overloaded functions or template specialisations
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT8, cpp::simple_type_specifier_builtin::CHAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT16, cpp::simple_type_specifier_builtin::SHORT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT32, cpp::simple_type_specifier_builtin::INT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT64, cpp::simple_type_specifier_builtin::INT64);
	return result;
}

template<typename SemaT>
inline cpp::decl_specifier_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::decl_specifier_default* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FRIEND, cpp::decl_specifier_default::FRIEND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPEDEF, cpp::decl_specifier_default::TYPEDEF);
	return result;
}


template<typename SemaT>
inline cpp::access_specifier* parseSymbol(ParserGeneric<SemaT>& parser, cpp::access_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PRIVATE, cpp::access_specifier::PRIVATE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PROTECTED, cpp::access_specifier::PROTECTED);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PUBLIC, cpp::access_specifier::PUBLIC);
	return result;
}

template<typename SemaT>
inline cpp::class_key* parseSymbol(ParserGeneric<SemaT>& parser, cpp::class_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CLASS, cpp::class_key::CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STRUCT, cpp::class_key::STRUCT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_UNION, cpp::class_key::UNION);
	return result;
}

template<typename SemaT>
inline cpp::ptr_operator_key* parseSymbol(ParserGeneric<SemaT>& parser, cpp::ptr_operator_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::ptr_operator_key::PTR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::ptr_operator_key::REF);
	return result;
}

template<typename SemaT>
inline cpp::type_parameter_key* parseSymbol(ParserGeneric<SemaT>& parser, cpp::type_parameter_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CLASS, cpp::type_parameter_key::CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPENAME, cpp::type_parameter_key::TYPENAME);
	return result;
}

template<typename SemaT>
inline cpp::logical_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::logical_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ANDAND, cpp::logical_operator::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_OROR, cpp::logical_operator::OR);
	return result;
}

template<typename SemaT>
inline cpp::bitwise_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::bitwise_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::bitwise_operator::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_OR, cpp::bitwise_operator::OR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_XOR, cpp::bitwise_operator::XOR);
	return result;
}

template<typename SemaT>
inline cpp::unary_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::unary_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSPLUS, cpp::unary_operator::PLUSPLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSMINUS, cpp::unary_operator::MINUSMINUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::unary_operator::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::unary_operator::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUS, cpp::unary_operator::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUS, cpp::unary_operator::MINUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOT, cpp::unary_operator::NOT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_COMPL, cpp::unary_operator::COMPL);
	return result;
}


template<typename SemaT>
inline cpp::member_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::member_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOT, cpp::member_operator::DOT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROW, cpp::member_operator::ARROW);
	return result;
}

template<typename SemaT>
inline cpp::postfix_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::postfix_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSPLUS, cpp::postfix_operator::PLUSPLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSMINUS, cpp::postfix_operator::MINUSMINUS);
	return result;
}

template<typename SemaT>
inline cpp::cast_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::cast_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DYNAMICCAST, cpp::cast_operator::DYNAMIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATICCAST, cpp::cast_operator::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REINTERPRETCAST, cpp::cast_operator::REINTERPRET);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONSTCAST, cpp::cast_operator::CONST);
	return result;
}


template<typename SemaT>
inline cpp::pm_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::pm_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOTSTAR, cpp::pm_operator::DOTSTAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROWSTAR, cpp::pm_operator::ARROWSTAR);
	return result;
}

template<typename SemaT>
inline cpp::multiplicative_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::multiplicative_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::multiplicative_operator::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DIVIDE, cpp::multiplicative_operator::DIVIDE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PERCENT, cpp::multiplicative_operator::PERCENT);
	return result;
}


template<typename SemaT>
inline cpp::shift_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::shift_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTLEFT, cpp::shift_operator::SHIFTLEFT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTRIGHT, cpp::shift_operator::SHIFTRIGHT);
	return result;
}

template<typename SemaT>
inline cpp::relational_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::relational_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESS, cpp::relational_operator::LESS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATER, cpp::relational_operator::GREATER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESSEQUAL, cpp::relational_operator::LESSEQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATEREQUAL, cpp::relational_operator::GREATEREQUAL);
	return result;
}

template<typename SemaT>
inline cpp::equality_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::equality_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EQUAL, cpp::equality_operator::EQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOTEQUAL, cpp::equality_operator::NOTEQUAL);
	return result;
}

template<typename SemaT>
inline cpp::assignment_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::assignment_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ASSIGN, cpp::assignment_operator::ASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STARASSIGN, cpp::assignment_operator::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DIVIDEASSIGN, cpp::assignment_operator::DIVIDE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PERCENTASSIGN, cpp::assignment_operator::PERCENT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSASSIGN, cpp::assignment_operator::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSASSIGN, cpp::assignment_operator::MINUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTRIGHTASSIGN, cpp::assignment_operator::SHIFTRIGHT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTLEFTASSIGN, cpp::assignment_operator::SHIFTLEFT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ANDASSIGN, cpp::assignment_operator::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_XORASSIGN, cpp::assignment_operator::XOR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ORASSIGN, cpp::assignment_operator::OR);
	return result;
}


template<typename SemaT>
inline cpp::jump_statement_key* parseSymbol(ParserGeneric<SemaT>& parser, cpp::jump_statement_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_BREAK, cpp::jump_statement_key::BREAK);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONTINUE, cpp::jump_statement_key::CONTINUE);
	return result;
}

template<typename SemaT>
inline cpp::overloadable_operator_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::overloadable_operator_default* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ASSIGN, cpp::overloadable_operator_default::ASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STARASSIGN, cpp::overloadable_operator_default::STARASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DIVIDEASSIGN, cpp::overloadable_operator_default::DIVIDEASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PERCENTASSIGN, cpp::overloadable_operator_default::PERCENTASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSASSIGN, cpp::overloadable_operator_default::PLUSASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSASSIGN, cpp::overloadable_operator_default::MINUSASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTRIGHTASSIGN, cpp::overloadable_operator_default::SHIFTRIGHTASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTLEFTASSIGN, cpp::overloadable_operator_default::SHIFTLEFTASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ANDASSIGN, cpp::overloadable_operator_default::ANDASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_XORASSIGN, cpp::overloadable_operator_default::XORASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ORASSIGN, cpp::overloadable_operator_default::ORASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EQUAL, cpp::overloadable_operator_default::EQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOTEQUAL, cpp::overloadable_operator_default::NOTEQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESS, cpp::overloadable_operator_default::LESS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATER, cpp::overloadable_operator_default::GREATER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESSEQUAL, cpp::overloadable_operator_default::LESSEQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATEREQUAL, cpp::overloadable_operator_default::GREATEREQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTLEFT, cpp::overloadable_operator_default::SHIFTLEFT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTRIGHT, cpp::overloadable_operator_default::SHIFTRIGHT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROWSTAR, cpp::overloadable_operator_default::ARROWSTAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROW, cpp::overloadable_operator_default::ARROW);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSPLUS, cpp::overloadable_operator_default::PLUSPLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSMINUS, cpp::overloadable_operator_default::MINUSMINUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::overloadable_operator_default::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DIVIDE, cpp::overloadable_operator_default::DIVIDE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PERCENT, cpp::overloadable_operator_default::PERCENT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::overloadable_operator_default::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUS, cpp::overloadable_operator_default::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUS, cpp::overloadable_operator_default::MINUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOT, cpp::overloadable_operator_default::NOT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_COMPL, cpp::overloadable_operator_default::COMPL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::overloadable_operator_default::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_OR, cpp::overloadable_operator_default::OR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_XOR, cpp::overloadable_operator_default::XOR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOT, cpp::overloadable_operator_default::NOT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ANDAND, cpp::overloadable_operator_default::ANDAND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_OROR, cpp::overloadable_operator_default::OROR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_COMMA, cpp::overloadable_operator_default::COMMA);
	return result;
};

template<typename SemaT>
inline cpp::overloadable_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::overloadable_operator* result)
{
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::overloadable_operator_default);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::new_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::delete_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::function_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::array_operator);
	return result;
};

template<typename SemaT>
inline cpp::template_argument_list* parseSymbol(ParserGeneric<SemaT>& parser, cpp::template_argument_list* result)
{
	parser.inTemplateArgumentList = true;
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

template<typename SemaT>
inline cpp::member_declaration_bitfield* parseSymbol(ParserGeneric<SemaT>& parser, cpp::member_declaration_bitfield* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	if(!result->comma.value.empty())
	{
		PARSE_REQUIRED(parser, result->next);
	}
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

#if 0
inline cpp::expression* pruneSymbol(cpp::expression_list* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::expression* result)
{
	PARSE_SELECT(parser, cpp::expression_list);
	return result;
}

template<typename SemaT>
inline cpp::primary_expression_parenthesis* parseSymbol(ParserGeneric<SemaT>& parser, cpp::primary_expression_parenthesis* result)
{
	parser.inTemplateArgumentList = false;
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

#if 0
inline cpp::postfix_expression* pruneSymbol(cpp::postfix_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::postfix_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::postfix_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::postfix_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::postfix_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::postfix_expression_default);
	return result;
}

#if 0
inline cpp::pm_expression* pruneSymbol(cpp::pm_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif


template<typename SemaT>
inline cpp::pm_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::pm_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::pm_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::pm_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::pm_expression_default);
	return result;
}

#if 0
inline cpp::multiplicative_expression* pruneSymbol(cpp::multiplicative_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::multiplicative_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::multiplicative_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::multiplicative_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::multiplicative_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::multiplicative_expression_default);
	return result;
}

template<typename SemaT>
inline cpp::additive_operator* parseSymbol(ParserGeneric<SemaT>& parser, cpp::additive_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUS, cpp::additive_operator::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUS, cpp::additive_operator::MINUS);
	return result;
}

#if 0
inline cpp::additive_expression* pruneSymbol(cpp::additive_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::additive_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::additive_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::additive_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::additive_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::additive_expression_default);
	return result;
}

#if 0
inline cpp::shift_expression* pruneSymbol(cpp::shift_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::shift_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::shift_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::shift_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::shift_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::shift_expression_default);
	return result;
}

#if 0
inline cpp::relational_expression* pruneSymbol(cpp::relational_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::relational_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::relational_expression_default* result)
{
	if(parser.inTemplateArgumentList
		&& TOKEN_EQUAL(parser, boost::wave::T_GREATER)) // '>' terminates template-argument-list
	{
		return 0;
	}

	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::relational_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::relational_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::relational_expression_default);
	return result;
}

#if 0
inline cpp::equality_expression* pruneSymbol(cpp::equality_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::equality_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::equality_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::equality_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::equality_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::equality_expression_default);
	return result;
}

#if 0
inline cpp::and_expression* pruneSymbol(cpp::and_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::and_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::and_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::and_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::and_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::and_expression_default);
	return result;
}

#if 0
inline cpp::exclusive_or_expression* pruneSymbol(cpp::exclusive_or_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::exclusive_or_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::exclusive_or_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::exclusive_or_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::exclusive_or_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::exclusive_or_expression_default);
	return result;
}

#if 0
inline cpp::inclusive_or_expression* pruneSymbol(cpp::inclusive_or_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::inclusive_or_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::inclusive_or_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::inclusive_or_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::inclusive_or_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::inclusive_or_expression_default);
	return result;
}

#if 0
inline cpp::logical_and_expression* pruneSymbol(cpp::logical_and_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::logical_and_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::logical_and_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::logical_and_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::logical_and_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::logical_and_expression_default);
	return result;
}

#if 0
inline cpp::logical_or_expression* pruneSymbol(cpp::logical_or_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::logical_or_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::logical_or_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename SemaT>
inline cpp::logical_or_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::logical_or_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::logical_or_expression_default);
	return result;
}


#if 0
inline cpp::conditional_expression* pruneSymbol(cpp::conditional_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::conditional_expression_default* parseSymbol(ParserGeneric<SemaT>& parser, cpp::conditional_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

#if 0
inline cpp::assignment_expression* pruneSymbol(cpp::assignment_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}
#endif

template<typename SemaT>
inline cpp::conditional_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::conditional_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::conditional_expression_default);
	return result;
}


template<typename SemaT>
inline cpp::assignment_expression* parseSymbol(ParserGeneric<SemaT>& parser, cpp::assignment_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::assignment_expression_default);
	PARSE_SELECT(parser, cpp::throw_expression);
	return result;
}

template<typename SemaT>
inline cpp::parameter_declaration_list* parseSymbol(ParserGeneric<SemaT>& parser, cpp::parameter_declaration_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	if(!TOKEN_EQUAL(parser, boost::wave::T_ELLIPSIS))
	{
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

template<typename SemaT>
inline cpp::msext_asm_terminal* parseSymbol(ParserGeneric<SemaT>& parser, cpp::msext_asm_terminal* result)
{
	if(!TOKEN_EQUAL(parser, boost::wave::T_RIGHTBRACE)
		&& !TOKEN_EQUAL(parser, boost::wave::T_SEMICOLON)
		&& !TOKEN_EQUAL(parser, boost::wave::T_LEFTBRACE))
	{
		result->value.value = parser.get_value();
		parser.increment();
		return result;
	}
	return NULL;
}

template<typename SemaT>
inline cpp::msext_asm_element_list_inline* parseSymbol(ParserGeneric<SemaT>& parser, cpp::msext_asm_element_list_inline* result)
{
	size_t line = parser.get_position().line;
	PARSE_REQUIRED(parser, result->item);
	if(parser.get_position().line == line) // HACK: only continue until end of line
	{
		PARSE_OPTIONAL(parser, result->next);
		PARSE_TERMINAL(parser, result->semicolon);
	}
	else
	{
		result->semicolon.value.clear();
	}
	return result;
}



template<typename SemaT>
inline cpp::typetraits_unary* parseSymbol(ParserGeneric<SemaT>& parser, cpp::typetraits_unary* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_HAS_NOTHROW_CONSTRUCTOR, cpp::typetraits_unary::HAS_NOTHROW_CONSTRUCTOR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_HAS_NOTHROW_COPY, cpp::typetraits_unary::HAS_NOTHROW_COPY);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_HAS_TRIVIAL_ASSIGN, cpp::typetraits_unary::HAS_TRIVIAL_ASSIGN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_HAS_TRIVIAL_CONSTRUCTOR, cpp::typetraits_unary::HAS_TRIVIAL_CONSTRUCTOR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_HAS_TRIVIAL_COPY, cpp::typetraits_unary::HAS_TRIVIAL_COPY);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_HAS_TRIVIAL_DESTRUCTOR, cpp::typetraits_unary::HAS_TRIVIAL_DESTRUCTOR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_HAS_VIRTUAL_DESTRUCTOR, cpp::typetraits_unary::HAS_VIRTUAL_DESTRUCTOR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_ABSTRACT, cpp::typetraits_unary::IS_ABSTRACT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_CLASS, cpp::typetraits_unary::IS_CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_EMPTY, cpp::typetraits_unary::IS_EMPTY);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_ENUM, cpp::typetraits_unary::IS_ENUM);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_POD, cpp::typetraits_unary::IS_POD);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_POLYMORPHIC, cpp::typetraits_unary::IS_POLYMORPHIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_UNION, cpp::typetraits_unary::IS_UNION);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_TRIVIALLY_COPYABLE, cpp::typetraits_unary::IS_TRIVIALLY_COPYABLE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_STANDARD_LAYOUT, cpp::typetraits_unary::IS_STANDARD_LAYOUT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_LITERAL_TYPE, cpp::typetraits_unary::IS_LITERAL_TYPE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_UNDERLYING_TYPE, cpp::typetraits_unary::UNDERLYING_TYPE);
	return result;
}

template<typename SemaT>
inline cpp::typetraits_binary* parseSymbol(ParserGeneric<SemaT>& parser, cpp::typetraits_binary* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_BASE_OF, cpp::typetraits_binary::IS_BASE_OF);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_CONVERTIBLE_TO, cpp::typetraits_binary::IS_CONVERTIBLE_TO);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_TRIVIALLY_CONSTRUCTIBLE, cpp::typetraits_binary::IS_TRIVIALLY_CONSTRUCTIBLE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IS_TRIVIALLY_ASSIGNABLE, cpp::typetraits_binary::IS_TRIVIALLY_ASSIGNABLE);
	return result;
}

};


#endif


