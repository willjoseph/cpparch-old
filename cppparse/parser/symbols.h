
#ifndef INCLUDED_CPPPARSE_PARSER_SYMBOLS_H
#define INCLUDED_CPPPARSE_PARSER_SYMBOLS_H

#include "../parser.h"


template<typename ParserType>
inline cpp::identifier* parseSymbol(ParserType& parser, cpp::identifier* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_IDENTIFIER))
	{
		result->value.value = parser.get_value();
		result->value.position = parser.get_position();
		result->value.source = parser.get_source();
		parser.increment();
		return result;
	}
	return NULL;
}


template<typename ParserType>
inline cpp::string_literal* parseSymbol(ParserType& parser, cpp::string_literal* result)
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

template<typename ParserType>
inline cpp::numeric_literal* parseSymbol(ParserType& parser, cpp::numeric_literal* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PP_NUMBER, cpp::numeric_literal::INTEGER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INTLIT, cpp::numeric_literal::INTEGER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CHARLIT, cpp::numeric_literal::CHARACTER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FLOATLIT, cpp::numeric_literal::FLOATING);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TRUE, cpp::numeric_literal::BOOLEAN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FALSE, cpp::numeric_literal::BOOLEAN);
	return result;
}

template<typename ParserType>
inline cpp::cv_qualifier* parseSymbol(ParserType& parser, cpp::cv_qualifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONST, cpp::cv_qualifier::CONST);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VOLATILE, cpp::cv_qualifier::VOLATILE);
	return result;
}

template<typename ParserType>
inline cpp::function_specifier* parseSymbol(ParserType& parser, cpp::function_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INLINE, cpp::function_specifier::INLINE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EXPLICIT, cpp::function_specifier::EXPLICIT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VIRTUAL, cpp::function_specifier::VIRTUAL);
	return result;
}

template<typename ParserType>
inline cpp::storage_class_specifier* parseSymbol(ParserType& parser, cpp::storage_class_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REGISTER, cpp::storage_class_specifier::REGISTER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATIC, cpp::storage_class_specifier::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EXTERN, cpp::storage_class_specifier::EXTERN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MUTABLE, cpp::storage_class_specifier::MUTABLE);
	return result;
}

template<typename ParserType>
inline cpp::simple_type_specifier_builtin* parseSymbol(ParserType& parser, cpp::simple_type_specifier_builtin* result)
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

	// not handles as macros: types may be used as parameters in overloaded functions or template specialisations
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT8, cpp::simple_type_specifier_builtin::INT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT16, cpp::simple_type_specifier_builtin::INT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT32, cpp::simple_type_specifier_builtin::INT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT64, cpp::simple_type_specifier_builtin::INT);
	return result;
}

template<typename ParserType>
inline cpp::decl_specifier_default* parseSymbol(ParserType& parser, cpp::decl_specifier_default* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FRIEND, cpp::decl_specifier_default::FRIEND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPEDEF, cpp::decl_specifier_default::TYPEDEF);
	return result;
}


template<typename ParserType>
inline cpp::access_specifier* parseSymbol(ParserType& parser, cpp::access_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PRIVATE, cpp::access_specifier::PRIVATE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PROTECTED, cpp::access_specifier::PROTECTED);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PUBLIC, cpp::access_specifier::PUBLIC);
	return result;
}

template<typename ParserType>
inline cpp::class_key* parseSymbol(ParserType& parser, cpp::class_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CLASS, cpp::class_key::CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STRUCT, cpp::class_key::STRUCT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_UNION, cpp::class_key::UNION);
	return result;
}

template<typename ParserType>
inline cpp::ptr_operator_key* parseSymbol(ParserType& parser, cpp::ptr_operator_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::ptr_operator_key::PTR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::ptr_operator_key::REF);
	return result;
}

template<typename ParserType>
inline cpp::type_parameter_key* parseSymbol(ParserType& parser, cpp::type_parameter_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CLASS, cpp::type_parameter_key::CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPENAME, cpp::type_parameter_key::TYPENAME);
	return result;
}

template<typename ParserType>
inline cpp::logical_operator* parseSymbol(ParserType& parser, cpp::logical_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ANDAND, cpp::logical_operator::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_OROR, cpp::logical_operator::OR);
	return result;
}

template<typename ParserType>
inline cpp::bitwise_operator* parseSymbol(ParserType& parser, cpp::bitwise_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::bitwise_operator::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_OR, cpp::bitwise_operator::OR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_XOR, cpp::bitwise_operator::XOR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOT, cpp::bitwise_operator::NOT);
	return result;
}

template<typename ParserType>
inline cpp::unary_operator* parseSymbol(ParserType& parser, cpp::unary_operator* result)
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


template<typename ParserType>
inline cpp::member_operator* parseSymbol(ParserType& parser, cpp::member_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOT, cpp::member_operator::DOT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROW, cpp::member_operator::ARROW);
	return result;
}

template<typename ParserType>
inline cpp::postfix_operator* parseSymbol(ParserType& parser, cpp::postfix_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSPLUS, cpp::postfix_operator::PLUSPLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSMINUS, cpp::postfix_operator::MINUSMINUS);
	return result;
}

template<typename ParserType>
inline cpp::cast_operator* parseSymbol(ParserType& parser, cpp::cast_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DYNAMICCAST, cpp::cast_operator::DYNAMIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATICCAST, cpp::cast_operator::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REINTERPRETCAST, cpp::cast_operator::REINTERPRET);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONSTCAST, cpp::cast_operator::CONST);
	return result;
}


template<typename ParserType>
inline cpp::pm_operator* parseSymbol(ParserType& parser, cpp::pm_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOTSTAR, cpp::pm_operator::DOTSTAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROWSTAR, cpp::pm_operator::ARROWSTAR);
	return result;
}

template<typename ParserType>
inline cpp::multiplicative_operator* parseSymbol(ParserType& parser, cpp::multiplicative_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::multiplicative_operator::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DIVIDE, cpp::multiplicative_operator::DIVIDE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PERCENT, cpp::multiplicative_operator::PERCENT);
	return result;
}


template<typename ParserType>
inline cpp::shift_operator* parseSymbol(ParserType& parser, cpp::shift_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTLEFT, cpp::shift_operator::SHIFTLEFT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTRIGHT, cpp::shift_operator::SHIFTRIGHT);
	return result;
}

template<typename ParserType>
inline cpp::relational_operator* parseSymbol(ParserType& parser, cpp::relational_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESS, cpp::relational_operator::LESS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATER, cpp::relational_operator::GREATER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESSEQUAL, cpp::relational_operator::LESSEQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATEREQUAL, cpp::relational_operator::GREATEREQUAL);
	return result;
}

template<typename ParserType>
inline cpp::equality_operator* parseSymbol(ParserType& parser, cpp::equality_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EQUAL, cpp::equality_operator::EQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOTEQUAL, cpp::equality_operator::NOTEQUAL);
	return result;
}

template<typename ParserType>
inline cpp::assignment_operator* parseSymbol(ParserType& parser, cpp::assignment_operator* result)
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


template<typename ParserType>
inline cpp::jump_statement_key* parseSymbol(ParserType& parser, cpp::jump_statement_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_BREAK, cpp::jump_statement_key::BREAK);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONTINUE, cpp::jump_statement_key::CONTINUE);
	return result;
}

template<typename ParserType>
inline cpp::overloadable_operator* parseSymbol(ParserType& parser, cpp::overloadable_operator* result)
{
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::assignment_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::member_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::postfix_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::unary_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::pm_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::multiplicative_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::additive_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::shift_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::relational_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::equality_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::new_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::delete_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::comma_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::function_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::array_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::bitwise_operator);
	PARSE_SELECT_UNAMBIGUOUS(parser, cpp::logical_operator);
	return result;
};

template<typename ParserType>
inline cpp::template_argument_list* parseSymbol(ParserType& parser, cpp::template_argument_list* result)
{
	parser.inTemplateArgumentList = true;
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

template<typename ParserType>
inline cpp::template_argument_clause_disambiguate* parseSymbol(ParserType& parser, cpp::template_argument_clause_disambiguate* result)
{
	PARSE_SELECT(parser, cpp::template_argument_clause);
	return result;
}

template<typename ParserType>
inline cpp::simple_template_id* parseSymbol(ParserType& parser, cpp::simple_template_id* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_REQUIRED(parser, result->args);
	return result;
}


template<typename ParserType>
inline cpp::member_declaration_bitfield* parseSymbol(ParserType& parser, cpp::member_declaration_bitfield* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	if(result->comma.value != 0)
	{
		PARSE_REQUIRED(parser, result->next);
	}
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}


inline cpp::expression* pruneSymbol(cpp::expression_list* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}


template<typename ParserType>
inline cpp::expression* parseSymbol(ParserType& parser, cpp::expression* result)
{
	PARSE_SELECT(parser, cpp::expression_list);
	return result;
}

template<typename ParserType>
inline cpp::primary_expression_parenthesis* parseSymbol(ParserType& parser, cpp::primary_expression_parenthesis* result)
{
	parser.inTemplateArgumentList = false;
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

inline cpp::postfix_expression* pruneSymbol(cpp::postfix_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::postfix_expression_default* parseSymbol(ParserType& parser, cpp::postfix_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::postfix_expression* parseSymbol(ParserType& parser, cpp::postfix_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::postfix_expression_default);
	return result;
}

inline cpp::pm_expression* pruneSymbol(cpp::pm_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}


template<typename ParserType>
inline cpp::pm_expression_default* parseSymbol(ParserType& parser, cpp::pm_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::pm_expression* parseSymbol(ParserType& parser, cpp::pm_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::pm_expression_default);
	return result;
}

inline cpp::multiplicative_expression* pruneSymbol(cpp::multiplicative_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::multiplicative_expression_default* parseSymbol(ParserType& parser, cpp::multiplicative_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::multiplicative_expression* parseSymbol(ParserType& parser, cpp::multiplicative_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::multiplicative_expression_default);
	return result;
}

template<typename ParserType>
inline cpp::additive_operator* parseSymbol(ParserType& parser, cpp::additive_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUS, cpp::additive_operator::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUS, cpp::additive_operator::MINUS);
	return result;
}

inline cpp::additive_expression* pruneSymbol(cpp::additive_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::additive_expression_default* parseSymbol(ParserType& parser, cpp::additive_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::additive_expression* parseSymbol(ParserType& parser, cpp::additive_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::additive_expression_default);
	return result;
}

inline cpp::shift_expression* pruneSymbol(cpp::shift_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::shift_expression_default* parseSymbol(ParserType& parser, cpp::shift_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::shift_expression* parseSymbol(ParserType& parser, cpp::shift_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::shift_expression_default);
	return result;
}

inline cpp::relational_expression* pruneSymbol(cpp::relational_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::relational_expression_default* parseSymbol(ParserType& parser, cpp::relational_expression_default* result)
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

template<typename ParserType>
inline cpp::relational_expression* parseSymbol(ParserType& parser, cpp::relational_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::relational_expression_default);
	return result;
}

inline cpp::equality_expression* pruneSymbol(cpp::equality_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::equality_expression_default* parseSymbol(ParserType& parser, cpp::equality_expression_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::equality_expression* parseSymbol(ParserType& parser, cpp::equality_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::equality_expression_default);
	return result;
}

inline cpp::and_expression* pruneSymbol(cpp::and_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::and_expression_default* parseSymbol(ParserType& parser, cpp::and_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::and_expression* parseSymbol(ParserType& parser, cpp::and_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::and_expression_default);
	return result;
}

inline cpp::exclusive_or_expression* pruneSymbol(cpp::exclusive_or_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::exclusive_or_expression_default* parseSymbol(ParserType& parser, cpp::exclusive_or_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::exclusive_or_expression* parseSymbol(ParserType& parser, cpp::exclusive_or_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::exclusive_or_expression_default);
	return result;
}

inline cpp::inclusive_or_expression* pruneSymbol(cpp::inclusive_or_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::inclusive_or_expression_default* parseSymbol(ParserType& parser, cpp::inclusive_or_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::inclusive_or_expression* parseSymbol(ParserType& parser, cpp::inclusive_or_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::inclusive_or_expression_default);
	return result;
}

inline cpp::logical_and_expression* pruneSymbol(cpp::logical_and_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::logical_and_expression_default* parseSymbol(ParserType& parser, cpp::logical_and_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::logical_and_expression* parseSymbol(ParserType& parser, cpp::logical_and_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::logical_and_expression_default);
	return result;
}

inline cpp::logical_or_expression* pruneSymbol(cpp::logical_or_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::logical_or_expression_default* parseSymbol(ParserType& parser, cpp::logical_or_expression_default* result)
{
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

template<typename ParserType>
inline cpp::logical_or_expression* parseSymbol(ParserType& parser, cpp::logical_or_expression* result)
{
	PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, cpp::logical_or_expression_default);
	return result;
}


inline cpp::conditional_expression* pruneSymbol(cpp::conditional_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::conditional_expression_default* parseSymbol(ParserType& parser, cpp::conditional_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

inline cpp::assignment_expression* pruneSymbol(cpp::logical_or_expression_precedent* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

template<typename ParserType>
inline cpp::conditional_expression* parseSymbol(ParserType& parser, cpp::conditional_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::conditional_expression_default);
	return result;
}


template<typename ParserType>
inline cpp::assignment_expression* parseSymbol(ParserType& parser, cpp::assignment_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::logical_or_expression_precedent);
	PARSE_SELECT(parser, cpp::throw_expression);
	return result;
}

template<typename ParserType>
inline cpp::parameter_declaration_list* parseSymbol(ParserType& parser, cpp::parameter_declaration_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	if(!TOKEN_EQUAL(parser, boost::wave::T_ELLIPSIS))
	{
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

template<typename ParserType>
inline cpp::msext_asm_terminal* parseSymbol(ParserType& parser, cpp::msext_asm_terminal* result)
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

template<typename ParserType>
inline cpp::msext_asm_element_list_inline* parseSymbol(ParserType& parser, cpp::msext_asm_element_list_inline* result)
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
		result->semicolon.value = 0;
	}
	return result;
}



#endif


