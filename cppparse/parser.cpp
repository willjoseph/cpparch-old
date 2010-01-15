
#include "parser.h"


inline cpp::identifier* parseNode(Parser& parser, cpp::identifier* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_IDENTIFIER))
	{
		result->value = parser.get_value();
		parser.increment();
		return result;
	}
	return NULL;
}

inline cpp::declaration_seq* parseNode(Parser& parser, cpp::declaration_seq* result);

inline cpp::namespace_definition* parseNode(Parser& parser, cpp::namespace_definition* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_NAMESPACE))
	{
		PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NAMESPACE);
		PARSE_OPTIONAL(parser, result->id);
		PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
		PARSE_OPTIONAL(parser, result->body);
		PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
		return result;
	}
	return NULL;
}

inline cpp::cv_qualifier* parseNode(Parser& parser, cpp::cv_qualifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONST, cpp::cv_qualifier::CONST);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VOLATILE, cpp::cv_qualifier::VOLATILE);
	return NULL;
}

inline cpp::function_specifier* parseNode(Parser& parser, cpp::function_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INLINE, cpp::function_specifier::INLINE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EXPLICIT, cpp::function_specifier::EXPLICIT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VIRTUAL, cpp::function_specifier::VIRTUAL);
	return NULL;
}

inline cpp::storage_class_specifier* parseNode(Parser& parser, cpp::storage_class_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REGISTER, cpp::storage_class_specifier::REGISTER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATIC, cpp::storage_class_specifier::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EXTERN, cpp::storage_class_specifier::EXTERN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MUTABLE, cpp::storage_class_specifier::MUTABLE);
	return NULL;
}

inline cpp::simple_type_specifier_builtin* parseNode(Parser& parser, cpp::simple_type_specifier_builtin* result)
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
	return NULL;
}

inline cpp::decl_specifier_default* parseNode(Parser& parser, cpp::decl_specifier_default* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FRIEND, cpp::decl_specifier_default::FRIEND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPEDEF, cpp::decl_specifier_default::TYPEDEF);
	return NULL;
}


inline cpp::type_name* parseNode(Parser& parser, cpp::type_name* result)
{
	PARSE_SELECT(parser, cpp::identifier);
	return NULL;
}

inline cpp::class_name* parseNode(Parser& parser, cpp::class_name* result)
{
	PARSE_SELECT(parser, cpp::identifier);
	return NULL;
}

inline cpp::simple_template_id* parseNode(Parser& parser, cpp::simple_template_id* result)
{
	// TODO
	return NULL;
}

inline cpp::nested_name_specifier_prefix* parseNode(Parser& parser, cpp::nested_name_specifier_prefix* result)
{
	PARSE_REQUIRED(parser, result->name);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	return result;
}

inline cpp::nested_name_specifier_suffix* parseNode(Parser& parser, cpp::nested_name_specifier_suffix* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->name);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::nested_name_specifier* parseNode(Parser& parser, cpp::nested_name_specifier* result)
{
	PARSE_REQUIRED(parser, result->prefix);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::simple_type_specifier_name* parseNode(Parser& parser, cpp::simple_type_specifier_name* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::simple_type_specifier_template* parseNode(Parser& parser, cpp::simple_type_specifier_template* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::simple_type_specifier* parseNode(Parser& parser, cpp::simple_type_specifier* result)
{
	PARSE_SELECT(parser, cpp::simple_type_specifier_builtin);
	PARSE_SELECT(parser, cpp::simple_type_specifier_template);
	PARSE_SELECT(parser, cpp::simple_type_specifier_name);
	return NULL;
}

inline cpp::using_declaration* parseNode(Parser& parser, cpp::using_declaration* result);
inline cpp::function_definition* parseNode(Parser& parser, cpp::function_definition* result);

inline cpp::template_declaration* parseNode(Parser& parser, cpp::template_declaration* result)
{
	// TODO
	return NULL;
}

inline cpp::constant_expression* parseNode(Parser& parser, cpp::constant_expression* result)
{
	// TODO
	return NULL;
}

inline cpp::member_declarator_pure* parseNode(Parser& parser, cpp::member_declarator_pure* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DECIMALINT); // TODO: check value is zero
	return NULL;
}

inline cpp::constant_initializer* parseNode(Parser& parser, cpp::constant_initializer* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_REQUIRED(parser, result->expr);
	return NULL;
}

inline cpp::member_declarator_suffix* parseNode(Parser& parser, cpp::member_declarator_suffix* result)
{
	PARSE_SELECT(parser, cpp::constant_initializer); // TODO: ambiguity here!
	PARSE_SELECT(parser, cpp::member_declarator_pure);
	return NULL;
}

inline cpp::member_declarator_bitfield* parseNode(Parser& parser, cpp::member_declarator_bitfield* result)
{
	// TODO
	return NULL;
}

inline cpp::declarator* parseNode(Parser& parser, cpp::declarator* result);
inline cpp::decl_specifier_seq* parseNode(Parser& parser, cpp::decl_specifier_seq* result);

inline cpp::member_declarator_default* parseNode(Parser& parser, cpp::member_declarator_default* result)
{
	PARSE_REQUIRED(parser, result->decl);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::member_declarator* parseNode(Parser& parser, cpp::member_declarator* result)
{
	PARSE_SELECT(parser, cpp::member_declarator_default);
	PARSE_SELECT(parser, cpp::member_declarator_bitfield);
	return NULL;
}

inline cpp::member_declarator_list* parseNode(Parser& parser, cpp::member_declarator_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::member_declaration_default* parseNode(Parser& parser, cpp::member_declaration_default* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::member_declaration_nested* parseNode(Parser& parser, cpp::member_declaration_nested* result)
{
	// TODO
	return NULL;
}

inline cpp::ctor_specifier_seq* parseNode(Parser& parser, cpp::ctor_specifier_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::ctor_initializer* parseNode(Parser& parser, cpp::ctor_initializer* result)
{
	// TODO
	return NULL;
}

inline cpp::compound_statement* parseNode(Parser& parser, cpp::compound_statement* result);

inline cpp::function_body* parseNode(Parser& parser, cpp::function_body* result)
{
	PARSE_SELECT(parser, cpp::compound_statement);
	return NULL;
}

inline cpp::handler_seq* parseNode(Parser& parser, cpp::handler_seq* result)
{
	// TODO
	return NULL;
}

inline cpp::constructor_definition* parseNode(Parser& parser, cpp::constructor_definition* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	bool isTry;
	PARSE_TOKEN_OPTIONAL(parser, isTry, boost::wave::T_TRY);
	PARSE_OPTIONAL(parser, result->init);
	PARSE_REQUIRED(parser, result->body);
	result->handlers = NULL;
	if(isTry)
	{
		PARSE_REQUIRED(parser, result->handlers);
	}
	return result;
}

inline cpp::member_function_definition* parseNode(Parser& parser, cpp::member_function_definition* result)
{
	PARSE_SELECT(parser, cpp::constructor_definition); // TODO: check for ambiguity
	PARSE_SELECT(parser, cpp::function_definition);
	return NULL;
}

inline cpp::member_declaration_inline* parseNode(Parser& parser, cpp::member_declaration_inline* result)
{
	PARSE_REQUIRED(parser, result->func);
	bool trailingSemicolon;
	PARSE_TOKEN_OPTIONAL(parser, trailingSemicolon, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::member_declaration_ctor* parseNode(Parser& parser, cpp::member_declaration_ctor* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::member_declaration* parseNode(Parser& parser, cpp::member_declaration* result)
{
	PARSE_SELECT(parser, cpp::using_declaration);
	PARSE_SELECT(parser, cpp::template_declaration);
	PARSE_SELECT(parser, cpp::member_declaration_ctor); // this matches a constructor: Class(Type);
	PARSE_SELECT(parser, cpp::member_declaration_default); // this matches a member: Type(member);
	PARSE_SELECT(parser, cpp::member_declaration_nested);
	PARSE_SELECT(parser, cpp::member_declaration_inline);
	return NULL;
}

inline cpp::member_specification* parseNode(Parser& parser, cpp::member_specification* result);

inline cpp::member_specification_list* parseNode(Parser& parser, cpp::member_specification_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::access_specifier* parseNode(Parser& parser, cpp::access_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PRIVATE, cpp::access_specifier::PRIVATE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PROTECTED, cpp::access_specifier::PROTECTED);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PUBLIC, cpp::access_specifier::PUBLIC);
	return NULL;
}

inline cpp::member_specification_access* parseNode(Parser& parser, cpp::member_specification_access* result)
{
	PARSE_REQUIRED(parser, result->access);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::member_specification* parseNode(Parser& parser, cpp::member_specification* result)
{
	PARSE_SELECT(parser, cpp::member_specification_access);
	PARSE_SELECT(parser, cpp::member_specification_list);
	return NULL;
}

inline cpp::class_key* parseNode(Parser& parser, cpp::class_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CLASS, cpp::class_key::CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STRUCT, cpp::class_key::STRUCT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_UNION, cpp::class_key::UNION);
	return NULL;
}

inline cpp::base_clause* parseNode(Parser& parser, cpp::base_clause* result)
{
	// TODO
	return NULL;
}

inline cpp::class_head_default* parseNode(Parser& parser, cpp::class_head_default* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_OPTIONAL(parser, result->id);
	PARSE_OPTIONAL(parser, result->base);
	return result;
}

inline cpp::class_head_nested* parseNode(Parser& parser, cpp::class_head_nested* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_REQUIRED(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_OPTIONAL(parser, result->base);
	return result;
}

inline cpp::class_head* parseNode(Parser& parser, cpp::class_head* result)
{
	PARSE_SELECT(parser, cpp::class_head_nested);
	PARSE_SELECT(parser, cpp::class_head_default);
	return NULL;
}

inline cpp::class_specifier* parseNode(Parser& parser, cpp::class_specifier* result)
{
	PARSE_REQUIRED(parser, result->head);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->members);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::enum_specifier* parseNode(Parser& parser, cpp::enum_specifier* result)
{
	// TODO
	return NULL;
}

inline cpp::elaborated_type_specifier* parseNode(Parser& parser, cpp::elaborated_type_specifier* result)
{
	// TODO
	return NULL;
}

inline cpp::type_specifier_noncv* parseNode(Parser& parser, cpp::type_specifier_noncv* result)
{
	PARSE_SELECT(parser, cpp::simple_type_specifier);
	PARSE_SELECT(parser, cpp::class_specifier);
	PARSE_SELECT(parser, cpp::enum_specifier);
	PARSE_SELECT(parser, cpp::elaborated_type_specifier);
	return NULL;
}

inline cpp::type_specifier* parseNode(Parser& parser, cpp::type_specifier* result)
{
	PARSE_SELECT(parser, cpp::type_specifier_noncv);
	PARSE_SELECT(parser, cpp::cv_qualifier);
	return NULL;
}

inline cpp::decl_specifier_nontype* parseNode(Parser& parser, cpp::decl_specifier_nontype* result)
{
	PARSE_SELECT(parser, cpp::storage_class_specifier);
	PARSE_SELECT(parser, cpp::decl_specifier_default);
	PARSE_SELECT(parser, cpp::function_specifier);
	PARSE_SELECT(parser, cpp::cv_qualifier);
	return NULL;
}

inline cpp::decl_specifier_prefix_seq* parseNode(Parser& parser, cpp::decl_specifier_prefix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::decl_specifier_suffix* parseNode(Parser& parser, cpp::decl_specifier_suffix* result)
{
	PARSE_SELECT(parser, cpp::decl_specifier_nontype);
	PARSE_SELECT(parser, cpp::simple_type_specifier_builtin);
	return NULL;
}

inline cpp::decl_specifier_suffix_seq* parseNode(Parser& parser, cpp::decl_specifier_suffix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::decl_specifier_seq* parseNode(Parser& parser, cpp::decl_specifier_seq* result)
{
	PARSE_OPTIONAL(parser, result->prefix);
	PARSE_REQUIRED(parser, result->type);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::cv_qualifier_seq* parseNode(Parser& parser, cpp::cv_qualifier_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::ptr_operator* parseNode(Parser& parser, cpp::ptr_operator* result)
{
	// TODO: 'ClassName*' form
	result->isGlobal = false;
	result->isRef = TOKEN_EQUAL(parser, boost::wave::T_AND);
	result->context = NULL;
	if(result->isRef)
	{
		parser.increment();
		result->qual = NULL;
		return result;
	}
	if(TOKEN_EQUAL(parser, boost::wave::T_STAR))
	{
		parser.increment();
		PARSE_OPTIONAL(parser, result->qual);
		return result;
	}
	return NULL;
}

inline cpp::declarator_suffix_array* parseNode(Parser& parser, cpp::declarator_suffix_array* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_OPTIONAL(parser, result->size);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	return result;
}

inline cpp::declarator* parseNode(Parser& parser, cpp::declarator* result);

inline cpp::throw_expression* parseNode(Parser& parser, cpp::throw_expression* result)
{
	// TODO
	return NULL;
}

inline cpp::unary_operator* parseNode(Parser& parser, cpp::unary_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSPLUS, cpp::unary_operator::PLUSPLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSMINUS, cpp::unary_operator::MINUSMINUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::unary_operator::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::unary_operator::AND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUS, cpp::unary_operator::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUS, cpp::unary_operator::MINUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOT, cpp::unary_operator::NOT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_COMPL, cpp::unary_operator::COMPL);
	return NULL;
}

inline cpp::cast_expression* parseNode(Parser& parser, cpp::cast_expression* result);

inline cpp::unary_expression_op* parseNode(Parser& parser, cpp::unary_expression_op* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::literal* parseNode(Parser& parser, cpp::literal* result)
{
	// TODO
	return NULL;
}

inline cpp::primary_expression_builtin* parseNode(Parser& parser, cpp::primary_expression_builtin* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_THIS);
	return result;
}

inline cpp::assignment_expression* parseNode(Parser& parser, cpp::assignment_expression* result);

inline cpp::expression_comma* parseNode(Parser& parser, cpp::expression_comma* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::expression* parseNode(Parser& parser, cpp::expression* result)
{
	PARSE_PREFIX(parser, cpp::expression_comma);
	return NULL;
}

inline cpp::primary_expression_parenthesis* parseNode(Parser& parser, cpp::primary_expression_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return NULL;
}

inline cpp::id_expression* parseNode(Parser& parser, cpp::id_expression* result);

inline cpp::primary_expression* parseNode(Parser& parser, cpp::primary_expression* result)
{
	PARSE_SELECT(parser, cpp::literal);
	PARSE_SELECT(parser, cpp::primary_expression_builtin);
	PARSE_SELECT(parser, cpp::primary_expression_parenthesis);
	PARSE_SELECT(parser, cpp::id_expression);
	return NULL;
}

inline cpp::postfix_expression_index* parseNode(Parser& parser, cpp::postfix_expression_index* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_REQUIRED(parser, result->index);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	return result;
}

inline cpp::postfix_expression_call* parseNode(Parser& parser, cpp::postfix_expression_call* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::member_operator* parseNode(Parser& parser, cpp::member_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOT, cpp::member_operator::DOT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROW, cpp::member_operator::ARROW);
	return NULL;
}

inline cpp::postfix_expression_member* parseNode(Parser& parser, cpp::postfix_expression_member* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::postfix_expression_destructor* parseNode(Parser& parser, cpp::postfix_expression_destructor* result)
{
	// TODO
	return NULL;
}

inline cpp::postfix_expression_simple* parseNode(Parser& parser, cpp::postfix_expression_simple* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSPLUS, cpp::postfix_expression_simple::PLUSPLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSMINUS, cpp::postfix_expression_simple::MINUSMINUS);
	return NULL;
}

inline cpp::postfix_expression_construct* parseNode(Parser& parser, cpp::postfix_expression_construct* result)
{
	PARSE_REQUIRED(parser, result->type);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::cast_operation* parseNode(Parser& parser, cpp::cast_operation* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DYNAMICCAST, cpp::cast_operation::DYNAMIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATICCAST, cpp::cast_operation::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REINTERPRETCAST, cpp::cast_operation::REINTERPRET);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONSTCAST, cpp::cast_operation::CONST);
	return NULL;
}

inline cpp::postfix_expression_cast* parseNode(Parser& parser, cpp::postfix_expression_cast* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_REQUIRED(parser, result->type);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::postfix_expression_typeid* parseNode(Parser& parser, cpp::postfix_expression_typeid* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TYPEID);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::postfix_expression_typeidtype* parseNode(Parser& parser, cpp::postfix_expression_typeidtype* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TYPEID);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->type);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::postfix_expression_suffix* parseNode(Parser& parser, cpp::postfix_expression_suffix* result)
{
	PARSE_SELECT(parser, cpp::postfix_expression_index);
	PARSE_SELECT(parser, cpp::postfix_expression_call);
	PARSE_SELECT(parser, cpp::postfix_expression_member);
	PARSE_SELECT(parser, cpp::postfix_expression_destructor);
	PARSE_SELECT(parser, cpp::postfix_expression_simple);
	return NULL;
}

inline cpp::postfix_expression_prefix* parseNode(Parser& parser, cpp::postfix_expression_prefix* result)
{
	PARSE_SELECT(parser, cpp::primary_expression);
	PARSE_SELECT(parser, cpp::postfix_expression_construct);
	PARSE_SELECT(parser, cpp::postfix_expression_cast);
	PARSE_SELECT(parser, cpp::postfix_expression_typeid);
	PARSE_SELECT(parser, cpp::postfix_expression_typeidtype);
	return NULL;
}

inline cpp::postfix_expression_suffix_seq* parseNode(Parser& parser, cpp::postfix_expression_suffix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::postfix_expression_default* parseNode(Parser& parser, cpp::postfix_expression_default* result)
{
	PARSE_REQUIRED(parser, result->expr);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::postfix_expression* parseNode(Parser& parser, cpp::postfix_expression* result)
{
	PARSE_SELECT(parser, cpp::postfix_expression_default);
	PARSE_SELECT(parser, cpp::postfix_expression_prefix);
	return NULL;
}

inline cpp::unary_expression_sizeoftype* parseNode(Parser& parser, cpp::unary_expression_sizeoftype* result)
{
	// TODO
	return NULL;
}

inline cpp::unary_expression_sizeof* parseNode(Parser& parser, cpp::unary_expression_sizeof* result)
{
	// TODO
	return NULL;
}

inline cpp::new_expression* parseNode(Parser& parser, cpp::new_expression* result)
{
	// TODO
	return NULL;
}

inline cpp::delete_expression* parseNode(Parser& parser, cpp::delete_expression* result)
{
	// TODO
	return NULL;
}

inline cpp::unary_expression* parseNode(Parser& parser, cpp::unary_expression* result)
{
	PARSE_SELECT(parser, cpp::postfix_expression);
	PARSE_SELECT(parser, cpp::unary_expression_sizeoftype);
	PARSE_SELECT(parser, cpp::unary_expression_sizeof);
	PARSE_SELECT(parser, cpp::unary_expression_op);
	PARSE_SELECT(parser, cpp::new_expression);
	PARSE_SELECT(parser, cpp::delete_expression);
	return NULL;
}

inline cpp::type_id* parseNode(Parser& parser, cpp::type_id* result)
{
	// TODO
	return NULL;
}

inline cpp::cast_expression_default* parseNode(Parser& parser, cpp::cast_expression_default* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::cast_expression* parseNode(Parser& parser, cpp::cast_expression* result)
{
	PARSE_SELECT(parser, cpp::cast_expression_default);
	PARSE_SELECT(parser, cpp::unary_expression);
	return NULL;
}

inline cpp::pm_operator* parseNode(Parser& parser, cpp::pm_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOTSTAR, cpp::pm_operator::DOTSTAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROWSTAR, cpp::pm_operator::ARROWSTAR);
	return NULL;
}

inline cpp::pm_expression_default* parseNode(Parser& parser, cpp::pm_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::pm_expression* parseNode(Parser& parser, cpp::pm_expression* result)
{
	PARSE_PREFIX(parser, cpp::pm_expression_default);
	return NULL;
}

inline cpp::multiplicative_operator* parseNode(Parser& parser, cpp::multiplicative_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::multiplicative_operator::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DIVIDE, cpp::multiplicative_operator::DIVIDE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PERCENT, cpp::multiplicative_operator::PERCENT);
	return NULL;
}

inline cpp::multiplicative_expression_default* parseNode(Parser& parser, cpp::multiplicative_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::multiplicative_expression* parseNode(Parser& parser, cpp::multiplicative_expression* result)
{
	PARSE_PREFIX(parser, cpp::multiplicative_expression_default);
	return NULL;
}

inline cpp::additive_operator* parseNode(Parser& parser, cpp::additive_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUS, cpp::additive_operator::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUS, cpp::additive_operator::MINUS);
	return NULL;
}

inline cpp::additive_expression_default* parseNode(Parser& parser, cpp::additive_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::additive_expression* parseNode(Parser& parser, cpp::additive_expression* result)
{
	PARSE_PREFIX(parser, cpp::additive_expression_default);
	return NULL;
}

inline cpp::shift_operator* parseNode(Parser& parser, cpp::shift_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTLEFT, cpp::shift_operator::SHIFTLEFT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTRIGHT, cpp::shift_operator::SHIFTRIGHT);
	return NULL;
}

inline cpp::shift_expression_default* parseNode(Parser& parser, cpp::shift_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::shift_expression* parseNode(Parser& parser, cpp::shift_expression* result)
{
	PARSE_PREFIX(parser, cpp::shift_expression_default);
	return NULL;
}

inline cpp::relational_operator* parseNode(Parser& parser, cpp::relational_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESS, cpp::relational_operator::LESS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATER, cpp::relational_operator::GREATER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESSEQUAL, cpp::relational_operator::LESSEQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATEREQUAL, cpp::relational_operator::GREATEREQUAL);
	return NULL;
}

inline cpp::relational_expression_default* parseNode(Parser& parser, cpp::relational_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::relational_expression* parseNode(Parser& parser, cpp::relational_expression* result)
{
	PARSE_PREFIX(parser, cpp::relational_expression_default);
	return NULL;
}

inline cpp::equality_operator* parseNode(Parser& parser, cpp::equality_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EQUAL, cpp::equality_operator::EQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOTEQUAL, cpp::equality_operator::NOTEQUAL);
	return NULL;
}

inline cpp::equality_expression_default* parseNode(Parser& parser, cpp::equality_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::equality_expression* parseNode(Parser& parser, cpp::equality_expression* result)
{
	PARSE_PREFIX(parser, cpp::equality_expression_default);
	return NULL;
}

inline cpp::and_expression_default* parseNode(Parser& parser, cpp::and_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_AND))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::and_expression* parseNode(Parser& parser, cpp::and_expression* result)
{
	PARSE_PREFIX(parser, cpp::and_expression_default);
	return NULL;
}

inline cpp::exclusive_or_expression_default* parseNode(Parser& parser, cpp::exclusive_or_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_XOR))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::exclusive_or_expression* parseNode(Parser& parser, cpp::exclusive_or_expression* result)
{
	PARSE_PREFIX(parser, cpp::exclusive_or_expression_default);
	return NULL;
}

inline cpp::inclusive_or_expression_default* parseNode(Parser& parser, cpp::inclusive_or_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_OR))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::inclusive_or_expression* parseNode(Parser& parser, cpp::inclusive_or_expression* result)
{
	PARSE_PREFIX(parser, cpp::inclusive_or_expression_default);
	return NULL;
}

inline cpp::logical_and_expression_default* parseNode(Parser& parser, cpp::logical_and_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_ANDAND))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::logical_and_expression* parseNode(Parser& parser, cpp::logical_and_expression* result)
{
	PARSE_PREFIX(parser, cpp::logical_and_expression_default);
	return NULL;
}

inline cpp::logical_or_expression_default* parseNode(Parser& parser, cpp::logical_or_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_OROR))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::logical_or_expression* parseNode(Parser& parser, cpp::logical_or_expression* result)
{
	PARSE_PREFIX(parser, cpp::logical_or_expression_default);
	return NULL;
}

inline cpp::conditional_expression_default* parseNode(Parser& parser, cpp::conditional_expression_default* result)
{
	PARSE_REQUIRED(parser, result->test);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_QUESTION_MARK);
	PARSE_REQUIRED(parser, result->pass);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->fail);
	return result;
}

inline cpp::conditional_expression* parseNode(Parser& parser, cpp::conditional_expression* result)
{
	PARSE_SELECT(parser, cpp::logical_or_expression);
	PARSE_SELECT(parser, cpp::conditional_expression_default);
	return NULL;
}

inline cpp::assignment_operator* parseNode(Parser& parser, cpp::assignment_operator* result)
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
	return NULL;
}

inline cpp::assignment_expression* parseNode(Parser& parser, cpp::assignment_expression* result);

inline cpp::assignment_expression_default* parseNode(Parser& parser, cpp::assignment_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::assignment_expression* parseNode(Parser& parser, cpp::assignment_expression* result)
{
	PARSE_SELECT(parser, cpp::conditional_expression);
	PARSE_SELECT(parser, cpp::assignment_expression_default);
	PARSE_SELECT(parser, cpp::throw_expression);
	return NULL;
}

inline cpp::parameter_declaration_default* parseNode(Parser& parser, cpp::parameter_declaration_default* result)
{
	PARSE_REQUIRED(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	if(TOKEN_EQUAL(parser, boost::wave::T_ASSIGN))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->init);
	}
	return result;
}

inline cpp::abstract_declarator* parseNode(Parser& parser, cpp::abstract_declarator* result);

inline cpp::abstract_declarator_default* parseNode(Parser& parser, cpp::abstract_declarator_default* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::direct_abstract_declarator* parseNode(Parser& parser, cpp::direct_abstract_declarator* result)
{
	// TODO
	return NULL;
}

inline cpp::abstract_declarator* parseNode(Parser& parser, cpp::abstract_declarator* result)
{
	PARSE_SELECT(parser, cpp::abstract_declarator_default);
	PARSE_SELECT(parser, cpp::direct_abstract_declarator);
	return NULL;
}

inline cpp::parameter_declaration_abstract* parseNode(Parser& parser, cpp::parameter_declaration_abstract* result)
{
	PARSE_REQUIRED(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	if(TOKEN_EQUAL(parser, boost::wave::T_ASSIGN))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->init);
	}
	return result;
}

inline cpp::parameter_declaration* parseNode(Parser& parser, cpp::parameter_declaration* result)
{
	PARSE_SELECT(parser, cpp::parameter_declaration_default);
	PARSE_SELECT(parser, cpp::parameter_declaration_abstract);
	return NULL;
}

inline cpp::parameter_declaration_list* parseNode(Parser& parser, cpp::parameter_declaration_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::parameter_declaration_clause* parseNode(Parser& parser, cpp::parameter_declaration_clause* result)
{
	PARSE_OPTIONAL(parser, result->list);
	if(result->list != NULL
		&& TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ELLIPSIS);
		result->isEllipsis = true;
	}
	else
	{
		PARSE_TOKEN_OPTIONAL(parser, result->isEllipsis, boost::wave::T_ELLIPSIS);
	}
	return result;
}

inline cpp::exception_specification* parseNode(Parser& parser, cpp::exception_specification* result)
{
	// TODO
	return NULL;
}

inline cpp::declarator_suffix_function* parseNode(Parser& parser, cpp::declarator_suffix_function* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->params);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_OPTIONAL(parser, result->qual);
	PARSE_OPTIONAL(parser, result->except);
	return result;
}

inline cpp::declarator_suffix* parseNode(Parser& parser, cpp::declarator_suffix* result)
{
	PARSE_SELECT(parser, cpp::declarator_suffix_array);
	PARSE_SELECT(parser, cpp::declarator_suffix_function);
	return NULL;
}

inline cpp::declarator_suffix_seq* parseNode(Parser& parser, cpp::declarator_suffix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::direct_declarator_parenthesis* parseNode(Parser& parser, cpp::direct_declarator_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::operator_function_id* parseNode(Parser& parser, cpp::operator_function_id* result)
{
	// TODO
	return NULL;
}

inline cpp::conversion_function_id* parseNode(Parser& parser, cpp::conversion_function_id* result)
{
	// TODO
	return NULL;
}

inline cpp::template_id* parseNode(Parser& parser, cpp::template_id* result)
{
	// TODO
	return NULL;
}

inline cpp::unqualified_id* parseNode(Parser& parser, cpp::unqualified_id* result)
{
	PARSE_SELECT(parser, cpp::identifier);
	PARSE_SELECT(parser, cpp::operator_function_id);
	PARSE_SELECT(parser, cpp::conversion_function_id);
	//TODO PARSE_SELECT(parser, cpp::destructor_id);
	PARSE_SELECT(parser, cpp::template_id);
	return NULL;
}

inline cpp::qualified_id_default* parseNode(Parser& parser, cpp::qualified_id_default* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id_global* parseNode(Parser& parser, cpp::qualified_id_global* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id_global_template* parseNode(Parser& parser, cpp::qualified_id_global_template* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id_global_op_func* parseNode(Parser& parser, cpp::qualified_id_global_op_func* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id* parseNode(Parser& parser, cpp::qualified_id* result)
{
	PARSE_SELECT(parser, cpp::qualified_id_default);
	PARSE_SELECT(parser, cpp::qualified_id_global);
	PARSE_SELECT(parser, cpp::qualified_id_global_template);
	PARSE_SELECT(parser, cpp::qualified_id_global_op_func);
	return NULL;
}

inline cpp::id_expression* parseNode(Parser& parser, cpp::id_expression* result)
{
	PARSE_SELECT(parser, cpp::unqualified_id);
	PARSE_SELECT(parser, cpp::qualified_id);
	return NULL;
}

inline cpp::declarator_id* parseNode(Parser& parser, cpp::declarator_id* result)
{
	PARSE_SELECT(parser, cpp::id_expression);
	// TODO: qualified form
	return NULL;
}

inline cpp::direct_declarator_prefix* parseNode(Parser& parser, cpp::direct_declarator_prefix* result)
{
	PARSE_SELECT(parser, cpp::direct_declarator_parenthesis);
	PARSE_SELECT(parser, cpp::declarator_id);
	return NULL;
}

inline cpp::direct_declarator* parseNode(Parser& parser, cpp::direct_declarator* result)
{
	PARSE_REQUIRED(parser, result->prefix);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::declarator_ptr* parseNode(Parser& parser, cpp::declarator_ptr* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::declarator* parseNode(Parser& parser, cpp::declarator* result)
{
	PARSE_SELECT(parser, cpp::declarator_ptr);
	PARSE_SELECT(parser, cpp::direct_declarator);
	return NULL;
}

inline cpp::function_definition* parseNode(Parser& parser, cpp::function_definition* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	bool isTry;
	PARSE_TOKEN_OPTIONAL(parser, isTry, boost::wave::T_TRY);
	PARSE_REQUIRED(parser, result->body);
	result->handlers = NULL;
	if(isTry)
	{
		PARSE_REQUIRED(parser, result->handlers);
	}
	return result;
}

inline cpp::linkage_specification* parseNode(Parser& parser, cpp::linkage_specification* result)
{
	// TODO
	return NULL;
}

inline cpp::explicit_instantiation* parseNode(Parser& parser, cpp::explicit_instantiation* result)
{
	// TODO
	return NULL;
}

inline cpp::explicit_specialization* parseNode(Parser& parser, cpp::explicit_specialization* result)
{
	// TODO
	return NULL;
}

inline cpp::expression_list* parseNode(Parser& parser, cpp::expression_list* result)
{
	// TODO
	return NULL;
}

inline cpp::initializer_clause* parseNode(Parser& parser, cpp::initializer_clause* result);

inline cpp::initializer_list* parseNode(Parser& parser, cpp::initializer_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return NULL;
}

inline cpp::initializer_clause_list* parseNode(Parser& parser, cpp::initializer_clause_list* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	if(TOKEN_EQUAL(parser, boost::wave::T_RIGHTBRACE))
	{
		parser.increment();
		result->list = 0;
		return result;
	}
	PARSE_REQUIRED(parser, result->list);
	bool trailingComma;
	PARSE_TOKEN_OPTIONAL(parser, trailingComma, boost::wave::T_COMMA);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return NULL;
}

inline cpp::initializer_clause* parseNode(Parser& parser, cpp::initializer_clause* result)
{
	PARSE_SELECT(parser, cpp::initializer_clause_list);
	PARSE_SELECT(parser, cpp::assignment_expression);
	return NULL;
}

inline cpp::initializer_default* parseNode(Parser& parser, cpp::initializer_default* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_REQUIRED(parser, result->clause);
	return result;
}

inline cpp::initializer_parenthesis* parseNode(Parser& parser, cpp::initializer_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->list);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::initializer* parseNode(Parser& parser, cpp::initializer* result)
{
	PARSE_SELECT(parser, cpp::initializer_default);
	PARSE_SELECT(parser, cpp::initializer_parenthesis);
	return NULL;
}

inline cpp::init_declarator* parseNode(Parser& parser, cpp::init_declarator* result)
{
	PARSE_REQUIRED(parser, result->decl);
	PARSE_OPTIONAL(parser, result->init);
	return result;
}

inline cpp::init_declarator_list* parseNode(Parser& parser, cpp::init_declarator_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::simple_declaration* parseNode(Parser& parser, cpp::simple_declaration* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::asm_definition* parseNode(Parser& parser, cpp::asm_definition* result)
{
	// TODO
	return NULL;
}

inline cpp::namespace_alias_definition* parseNode(Parser& parser, cpp::namespace_alias_definition* result)
{
	// TODO
	return NULL;
}

inline cpp::using_declaration* parseNode(Parser& parser, cpp::using_declaration* result)
{
	// TODO
	return NULL;
}

inline cpp::using_directive* parseNode(Parser& parser, cpp::using_directive* result)
{
	// TODO
	return NULL;
}

inline cpp::block_declaration* parseNode(Parser& parser, cpp::block_declaration* result)
{
	PARSE_SELECT(parser, cpp::simple_declaration);
	PARSE_SELECT(parser, cpp::asm_definition);
	PARSE_SELECT(parser, cpp::namespace_alias_definition);
	PARSE_SELECT(parser, cpp::using_declaration);
	PARSE_SELECT(parser, cpp::using_directive);
	return NULL;
}

inline cpp::declaration* parseNode(Parser& parser, cpp::declaration* result)
{
	PARSE_SELECT(parser, cpp::linkage_specification);
	PARSE_SELECT(parser, cpp::explicit_instantiation);
	PARSE_SELECT(parser, cpp::template_declaration);
	PARSE_SELECT(parser, cpp::explicit_specialization);
	PARSE_SELECT(parser, cpp::namespace_definition);
	PARSE_SELECT(parser, cpp::function_definition);
	PARSE_SELECT(parser, cpp::constructor_definition);
	PARSE_SELECT(parser, cpp::block_declaration);
	return NULL;
}

inline cpp::declaration_seq* parseNode(Parser& parser, cpp::declaration_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::statement_seq* parseNode(Parser& parser, cpp::statement_seq* result);

inline cpp::compound_statement* parseNode(Parser& parser, cpp::compound_statement* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->body);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::declaration_statement* parseNode(Parser& parser, cpp::declaration_statement* result)
{
	PARSE_SELECT(parser, cpp::block_declaration);
	return NULL;
}

inline cpp::labeled_statement_id* parseNode(Parser& parser, cpp::labeled_statement_id* result)
{
	PARSE_REQUIRED(parser, result->label);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::labeled_statement_case* parseNode(Parser& parser, cpp::labeled_statement_case* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_CASE);
	PARSE_REQUIRED(parser, result->label);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::labeled_statement_default* parseNode(Parser& parser, cpp::labeled_statement_default* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DEFAULT);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::labeled_statement* parseNode(Parser& parser, cpp::labeled_statement* result)
{
	PARSE_SELECT(parser, cpp::labeled_statement_id);
	PARSE_SELECT(parser, cpp::labeled_statement_case);
	PARSE_SELECT(parser, cpp::labeled_statement_default);
	return NULL;
}

inline cpp::type_specifier_prefix_seq* parseNode(Parser& parser, cpp::type_specifier_prefix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::type_specifier_suffix* parseNode(Parser& parser, cpp::type_specifier_suffix* result)
{
	PARSE_SELECT(parser, cpp::cv_qualifier);
	PARSE_SELECT(parser, cpp::simple_type_specifier_builtin);
	return NULL;
}

inline cpp::type_specifier_suffix_seq* parseNode(Parser& parser, cpp::type_specifier_suffix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::type_specifier_seq* parseNode(Parser& parser, cpp::type_specifier_seq* result)
{
	PARSE_OPTIONAL(parser, result->prefix);
	PARSE_REQUIRED(parser, result->type);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}


inline cpp::condition_init* parseNode(Parser& parser, cpp::condition_init* result)
{
	PARSE_REQUIRED(parser, result->type);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_REQUIRED(parser, result->init);
	return result;
}

inline cpp::condition* parseNode(Parser& parser, cpp::condition* result)
{
	PARSE_SELECT(parser, cpp::condition_init); // ambiguity: 'Type x = y': match init statement first, because expression resolves to 'primary-expression'
	PARSE_SELECT(parser, cpp::expression);
	return NULL;
}

inline cpp::selection_statement_if* parseNode(Parser& parser, cpp::selection_statement_if* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_IF);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->body);
	result->fail = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_ELSE))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->fail);
	}
	return result;
}

inline cpp::selection_statement_switch* parseNode(Parser& parser, cpp::selection_statement_switch* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SWITCH);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::selection_statement* parseNode(Parser& parser, cpp::selection_statement* result)
{
	PARSE_SELECT(parser, cpp::selection_statement_if);
	PARSE_SELECT(parser, cpp::selection_statement_switch);
	return NULL;
}

inline cpp::for_init_statement* parseNode(Parser& parser, cpp::for_init_statement* result)
{
	PARSE_SELECT(parser, cpp::expression_statement);
	PARSE_SELECT(parser, cpp::simple_declaration);
	return NULL;
}

inline cpp::iteration_statement_for* parseNode(Parser& parser, cpp::iteration_statement_for* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_FOR);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->init);
	PARSE_OPTIONAL(parser, result->cond);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	PARSE_OPTIONAL(parser, result->incr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::iteration_statement_while* parseNode(Parser& parser, cpp::iteration_statement_while* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_WHILE);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::iteration_statement_dowhile* parseNode(Parser& parser, cpp::iteration_statement_dowhile* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DO);
	PARSE_REQUIRED(parser, result->body);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_WHILE);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::iteration_statement* parseNode(Parser& parser, cpp::iteration_statement* result)
{
	PARSE_SELECT(parser, cpp::iteration_statement_for);
	PARSE_SELECT(parser, cpp::iteration_statement_while);
	PARSE_SELECT(parser, cpp::iteration_statement_dowhile);
	return NULL;
}

inline cpp::jump_statement* parseNode(Parser& parser, cpp::jump_statement* result)
{
	// TODO
	return NULL;
}

inline cpp::try_block* parseNode(Parser& parser, cpp::try_block* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TRY);
	PARSE_REQUIRED(parser, result->body);
	PARSE_REQUIRED(parser, result->handlers);
	return result;
}

inline cpp::expression_statement* parseNode(Parser& parser, cpp::expression_statement* result)
{
	PARSE_OPTIONAL(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::statement* parseNode(Parser& parser, cpp::statement* result)
{
	PARSE_SELECT(parser, cpp::compound_statement);
	PARSE_SELECT(parser, cpp::declaration_statement);
	PARSE_SELECT(parser, cpp::labeled_statement);
	PARSE_SELECT(parser, cpp::expression_statement);
	PARSE_SELECT(parser, cpp::selection_statement);
	PARSE_SELECT(parser, cpp::iteration_statement);
	PARSE_SELECT(parser, cpp::jump_statement);
	PARSE_SELECT(parser, cpp::try_block);
	return NULL;
}

inline cpp::statement_seq* parseNode(Parser& parser, cpp::statement_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}


cpp::declaration_seq* parseFile(std::string& instring, const char* input)
{
	LexContext& context = createContext(instring, input);

	Scanner scanner(context);
	Parser parser(scanner);

	cpp::declaration_seq* result = parseNode(parser, new cpp::declaration_seq);
#ifdef _DEBUG
	if(result == 0)
	{
		int bleh = 0;
	}
#endif
	return result;
}

cpp::statement_seq* parseFunction(std::string& instring, const char* input)
{
	LexContext& context = createContext(instring, input);

	Scanner scanner(context);
	Parser parser(scanner);

	cpp::statement_seq* result = parseNode(parser, new cpp::statement_seq);
#ifdef _DEBUG
	if(result == 0)
	{
		int bleh = 0;
	}
#endif
	return result;
}

