
#include "parser.h"


inline cpp::identifier* parseSymbol(Parser& parser, cpp::identifier* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_IDENTIFIER))
	{
		result->value = parser.get_value();
		parser.increment();
		return result;
	}
	return NULL;
}

inline cpp::declaration_seq* parseSymbol(Parser& parser, cpp::declaration_seq* result);

inline cpp::namespace_definition* parseSymbol(Parser& parser, cpp::namespace_definition* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NAMESPACE);
	PARSE_OPTIONAL(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->body);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::cv_qualifier* parseSymbol(Parser& parser, cpp::cv_qualifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONST, cpp::cv_qualifier::CONST);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VOLATILE, cpp::cv_qualifier::VOLATILE);
	return result;
}

inline cpp::function_specifier* parseSymbol(Parser& parser, cpp::function_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INLINE, cpp::function_specifier::INLINE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EXPLICIT, cpp::function_specifier::EXPLICIT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_VIRTUAL, cpp::function_specifier::VIRTUAL);
	return result;
}

inline cpp::storage_class_specifier* parseSymbol(Parser& parser, cpp::storage_class_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REGISTER, cpp::storage_class_specifier::REGISTER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATIC, cpp::storage_class_specifier::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EXTERN, cpp::storage_class_specifier::EXTERN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MUTABLE, cpp::storage_class_specifier::MUTABLE);
	return result;
}

inline cpp::simple_type_specifier_builtin* parseSymbol(Parser& parser, cpp::simple_type_specifier_builtin* result)
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

inline cpp::decl_specifier_default* parseSymbol(Parser& parser, cpp::decl_specifier_default* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FRIEND, cpp::decl_specifier_default::FRIEND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPEDEF, cpp::decl_specifier_default::TYPEDEF);
	return result;
}


inline cpp::class_name* parseSymbol(Parser& parser, cpp::class_name* result)
{
	PARSE_SELECT(parser, cpp::simple_template_id); // TODO: ambiguity: shared prefix 'identifier'
	PARSE_SELECT(parser, cpp::identifier);
	return result;
}

inline cpp::type_name* parseSymbol(Parser& parser, cpp::type_name* result)
{
	PARSE_SELECT(parser, cpp::class_name); // NOTE: not distinguishing between class/enum/typedef names
	return result;
}

inline cpp::template_argument* parseSymbol(Parser& parser, cpp::template_argument* result)
{
	PARSE_SELECT(parser, cpp::type_id); // TODO: ambiguity: 'type-id' and 'primary-expression' may both be 'identifier'. Prefer type-id to handle 'T(*)()'.
	PARSE_SELECT(parser, cpp::assignment_expression);
	PARSE_SELECT(parser, cpp::id_expression);
	return result;
}

inline cpp::template_argument_list* parseSymbol(Parser& parser, cpp::template_argument_list* result)
{
	parser.inTemplateArgumentList = true;
	PARSE_REQUIRED(parser, result->item);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::simple_template_id* parseSymbol(Parser& parser, cpp::simple_template_id* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_REQUIRED(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	return result;
}

inline cpp::nested_name_specifier_prefix* parseSymbol(Parser& parser, cpp::nested_name_specifier_prefix* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	return result;
}

inline cpp::nested_name_specifier_suffix* parseSymbol(Parser& parser, cpp::nested_name_specifier_suffix* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::nested_name_specifier* parseSymbol(Parser& parser, cpp::nested_name_specifier* result)
{
	PARSE_REQUIRED(parser, result->prefix);
	PARSE_OPTIONAL(parser, result->suffix);  // TODO: shared-prefix ambiguity between nested-name-specifier and 'identifier'
	return result;
}

inline cpp::simple_type_specifier_name* parseSymbol(Parser& parser, cpp::simple_type_specifier_name* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::simple_type_specifier_template* parseSymbol(Parser& parser, cpp::simple_type_specifier_template* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::simple_type_specifier* parseSymbol(Parser& parser, cpp::simple_type_specifier* result)
{
	PARSE_SELECT(parser, cpp::simple_type_specifier_builtin);
	PARSE_SELECT(parser, cpp::simple_type_specifier_template);
	PARSE_SELECT(parser, cpp::simple_type_specifier_name);
	return result;
}

inline cpp::using_declaration* parseSymbol(Parser& parser, cpp::using_declaration* result);
inline cpp::function_definition* parseSymbol(Parser& parser, cpp::function_definition* result);
inline cpp::parameter_declaration* parseSymbol(Parser& parser, cpp::parameter_declaration* result);

inline cpp::type_parameter_key* parseSymbol(Parser& parser, cpp::type_parameter_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CLASS, cpp::type_parameter_key::CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPENAME, cpp::type_parameter_key::TYPENAME);
	return result;
}

inline cpp::type_parameter_default* parseSymbol(Parser& parser, cpp::type_parameter_default* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_OPTIONAL(parser, result->id);
	result->init = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_ASSIGN))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->init);
	}
	return result;
}

inline cpp::type_parameter_template* parseSymbol(Parser& parser, cpp::type_parameter_template* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_REQUIRED(parser, result->params);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_CLASS);
	PARSE_OPTIONAL(parser, result->id);
	result->init = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_ASSIGN))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->init);
	}
	return result;
}

inline cpp::type_parameter* parseSymbol(Parser& parser, cpp::type_parameter* result)
{
	PARSE_SELECT(parser, cpp::type_parameter_default);
	PARSE_SELECT(parser, cpp::type_parameter_template);
	return result;
}

inline cpp::template_parameter* parseSymbol(Parser& parser, cpp::template_parameter* result)
{
	PARSE_SELECT(parser, cpp::type_parameter);
	PARSE_SELECT(parser, cpp::parameter_declaration);
	return result;
}

inline cpp::template_parameter_list* parseSymbol(Parser& parser, cpp::template_parameter_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::template_declaration* parseSymbol(Parser& parser, cpp::template_declaration* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isExport, boost::wave::T_EXPORT);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_REQUIRED(parser, result->params);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	return result;
}

inline cpp::constant_expression* parseSymbol(Parser& parser, cpp::constant_expression* result)
{
	PARSE_SELECT(parser, cpp::conditional_expression);
	return result;
}

inline cpp::pure_specifier* parseSymbol(Parser& parser, cpp::pure_specifier* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DECIMALINT); // TODO: check value is zero
	return result;
}

inline cpp::constant_initializer* parseSymbol(Parser& parser, cpp::constant_initializer* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::member_initializer* parseSymbol(Parser& parser, cpp::member_initializer* result)
{
	PARSE_SELECT(parser, cpp::constant_initializer); // TODO: ambiguity here!
	PARSE_SELECT(parser, cpp::pure_specifier);
	return result;
}

inline cpp::member_declarator_bitfield* parseSymbol(Parser& parser, cpp::member_declarator_bitfield* result)
{
	PARSE_OPTIONAL(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->width);
	return result;
}

inline cpp::declarator* parseSymbol(Parser& parser, cpp::declarator* result);
inline cpp::decl_specifier_seq* parseSymbol(Parser& parser, cpp::decl_specifier_seq* result);

inline cpp::member_declarator_default* parseSymbol(Parser& parser, cpp::member_declarator_default* result)
{
	PARSE_REQUIRED(parser, result->decl);
	PARSE_OPTIONAL(parser, result->init);
	return result;
}

inline cpp::member_declarator* parseSymbol(Parser& parser, cpp::member_declarator* result)
{
	PARSE_SELECT(parser, cpp::member_declarator_bitfield); // TODO: shared prefix ambiguity: 'identifier'
	PARSE_SELECT(parser, cpp::member_declarator_default);
	return result;
}

inline cpp::member_declaration_general_bitfield* parseSymbol(Parser& parser, cpp::member_declaration_general_bitfield* result)
{
	PARSE_REQUIRED(parser, result->item);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::member_declaration_general_default* parseSymbol(Parser& parser, cpp::member_declaration_general_default* result)
{
	PARSE_OPTIONAL(parser, result->decl); // TODO: making this optional permits function-definition without declarator
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::member_declaration_general* parseSymbol(Parser& parser, cpp::member_declaration_general* result)
{
	PARSE_SELECT(parser, cpp::member_declaration_general_bitfield);
	PARSE_SELECT(parser, cpp::member_declaration_general_default);
	return result;
}

inline cpp::member_declarator_list* parseSymbol(Parser& parser, cpp::member_declarator_list* result)
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

inline cpp::member_declaration_suffix_default* parseSymbol(Parser& parser, cpp::member_declaration_suffix_default* result)
{
	PARSE_OPTIONAL(parser, result->init);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::member_declaration_suffix* parseSymbol(Parser& parser, cpp::member_declaration_suffix* result)
{
	PARSE_SELECT(parser, cpp::member_declaration_suffix_default);
	PARSE_SELECT(parser, cpp::function_definition_suffix);
	return result;
}

inline cpp::member_declaration_default* parseSymbol(Parser& parser, cpp::member_declaration_default* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::member_declaration_nested* parseSymbol(Parser& parser, cpp::member_declaration_nested* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::ctor_specifier_seq* parseSymbol(Parser& parser, cpp::ctor_specifier_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::mem_initializer* parseSymbol(Parser& parser, cpp::mem_initializer* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::mem_initializer_list* parseSymbol(Parser& parser, cpp::mem_initializer_list* result)
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

inline cpp::ctor_initializer* parseSymbol(Parser& parser, cpp::ctor_initializer* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->list);
	return result;
}

inline cpp::compound_statement* parseSymbol(Parser& parser, cpp::compound_statement* result);

inline cpp::function_body* parseSymbol(Parser& parser, cpp::function_body* result)
{
	PARSE_SELECT(parser, cpp::compound_statement);
	return result;
}

inline cpp::exception_declarator* parseSymbol(Parser& parser, cpp::exception_declarator* result)
{
	PARSE_SELECT(parser, cpp::declarator);
	PARSE_SELECT(parser, cpp::abstract_declarator);
	return result;
}

inline cpp::exception_declaration_default* parseSymbol(Parser& parser, cpp::exception_declaration_default* result)
{
	PARSE_REQUIRED(parser, result->type);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::exception_declaration_all* parseSymbol(Parser& parser, cpp::exception_declaration_all* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ELLIPSIS);
	return result;
}

inline cpp::exception_declaration* parseSymbol(Parser& parser, cpp::exception_declaration* result)
{
	PARSE_SELECT(parser, cpp::exception_declaration_all);
	PARSE_SELECT(parser, cpp::exception_declaration_default);
	return result;
}

inline cpp::handler_seq* parseSymbol(Parser& parser, cpp::handler_seq* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_CATCH);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->body);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::constructor_definition* parseSymbol(Parser& parser, cpp::constructor_definition* result)
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

inline cpp::member_declaration_inline* parseSymbol(Parser& parser, cpp::member_declaration_inline* result)
{
	PARSE_REQUIRED(parser, result->func);
	bool trailingSemicolon;
	PARSE_TOKEN_OPTIONAL(parser, trailingSemicolon, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::member_declaration_ctor* parseSymbol(Parser& parser, cpp::member_declaration_ctor* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::member_declaration* parseSymbol(Parser& parser, cpp::member_declaration* result)
{
	PARSE_SELECT(parser, cpp::using_declaration);
	PARSE_SELECT(parser, cpp::template_declaration);
	PARSE_SELECT(parser, cpp::member_declaration_inline);
	PARSE_SELECT(parser, cpp::member_declaration_ctor); // shared-prefix ambiguity:  this matches a constructor: Class(Type);
	PARSE_SELECT(parser, cpp::member_declaration_default); // this matches a member: Type(member);
	PARSE_SELECT(parser, cpp::member_declaration_nested);
	return result;
}

inline cpp::member_specification* parseSymbol(Parser& parser, cpp::member_specification* result);

inline cpp::member_specification_list* parseSymbol(Parser& parser, cpp::member_specification_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::access_specifier* parseSymbol(Parser& parser, cpp::access_specifier* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PRIVATE, cpp::access_specifier::PRIVATE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PROTECTED, cpp::access_specifier::PROTECTED);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PUBLIC, cpp::access_specifier::PUBLIC);
	return result;
}

inline cpp::member_specification_access* parseSymbol(Parser& parser, cpp::member_specification_access* result)
{
	PARSE_REQUIRED(parser, result->access);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::member_specification* parseSymbol(Parser& parser, cpp::member_specification* result)
{
	PARSE_SELECT(parser, cpp::member_specification_access);
	PARSE_SELECT(parser, cpp::member_specification_list);
	return result;
}

inline cpp::class_key* parseSymbol(Parser& parser, cpp::class_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CLASS, cpp::class_key::CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STRUCT, cpp::class_key::STRUCT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_UNION, cpp::class_key::UNION);
	return result;
}

inline cpp::base_specifier_access_virtual* parseSymbol(Parser& parser, cpp::base_specifier_access_virtual* result)
{
	PARSE_REQUIRED(parser, result->access);
	PARSE_TOKEN_OPTIONAL(parser, result->isVirtual, boost::wave::T_VIRTUAL);
	return result;
}

inline cpp::base_specifier_virtual_access* parseSymbol(Parser& parser, cpp::base_specifier_virtual_access* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_VIRTUAL);
	PARSE_OPTIONAL(parser, result->access);
	return result;
}

inline cpp::base_specifier_prefix* parseSymbol(Parser& parser, cpp::base_specifier_prefix* result)
{
	PARSE_SELECT(parser, cpp::base_specifier_access_virtual);
	PARSE_SELECT(parser, cpp::base_specifier_virtual_access);
	return result;
}

inline cpp::base_specifier* parseSymbol(Parser& parser, cpp::base_specifier* result)
{
	PARSE_OPTIONAL(parser, result->prefix);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::base_specifier_list* parseSymbol(Parser& parser, cpp::base_specifier_list* result)
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

inline cpp::base_clause* parseSymbol(Parser& parser, cpp::base_clause* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->list);
	return result;
}

inline cpp::class_head_default* parseSymbol(Parser& parser, cpp::class_head_default* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_OPTIONAL(parser, result->id);
	PARSE_OPTIONAL(parser, result->base);
	return result;
}

inline cpp::class_head_nested* parseSymbol(Parser& parser, cpp::class_head_nested* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_OPTIONAL(parser, result->base);
	return result;
}

inline cpp::class_head* parseSymbol(Parser& parser, cpp::class_head* result)
{
	PARSE_SELECT(parser, cpp::class_head_nested);
	PARSE_SELECT(parser, cpp::class_head_default);
	return result;
}

inline cpp::class_specifier* parseSymbol(Parser& parser, cpp::class_specifier* result)
{
	PARSE_REQUIRED(parser, result->head);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->members);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::enumerator_definition* parseSymbol(Parser& parser, cpp::enumerator_definition* result)
{
	PARSE_REQUIRED(parser, result->id);
	result->init = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_ASSIGN))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->init);
	}
	return result;
}

inline cpp::enumerator_list* parseSymbol(Parser& parser, cpp::enumerator_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_OPTIONAL(parser, result->next); // optional because trailing-comma is allowed
	}
	return result;
}

inline cpp::enum_specifier* parseSymbol(Parser& parser, cpp::enum_specifier* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ENUM);
	PARSE_OPTIONAL(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->values);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::enum_key* parseSymbol(Parser& parser, cpp::enum_key* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ENUM);
	return result;
}

inline cpp::elaborated_type_specifier_key* parseSymbol(Parser& parser, cpp::elaborated_type_specifier_key* result)
{
	PARSE_SELECT(parser, cpp::class_key);
	PARSE_SELECT(parser, cpp::enum_key);
	return result;
}

inline cpp::elaborated_type_specifier_default* parseSymbol(Parser& parser, cpp::elaborated_type_specifier_default* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::elaborated_type_specifier_template* parseSymbol(Parser& parser, cpp::elaborated_type_specifier_template* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::elaborated_type_specifier* parseSymbol(Parser& parser, cpp::elaborated_type_specifier* result)
{
	PARSE_SELECT(parser, cpp::elaborated_type_specifier_template); // TODO: shared-prefix ambiguity: match 'simple-template-id' before 'identifier' first
	PARSE_SELECT(parser, cpp::elaborated_type_specifier_default);
	return result;
}

inline cpp::typename_specifier* parseSymbol(Parser& parser, cpp::typename_specifier* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TYPENAME);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context); // TODO: check standard to see if this should be optional or required
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::type_specifier_noncv* parseSymbol(Parser& parser, cpp::type_specifier_noncv* result)
{
	PARSE_SELECT(parser, cpp::typename_specifier);
	PARSE_SELECT(parser, cpp::simple_type_specifier);
	PARSE_SELECT(parser, cpp::class_specifier);
	PARSE_SELECT(parser, cpp::enum_specifier);
	PARSE_SELECT(parser, cpp::elaborated_type_specifier);
	return result;
}

inline cpp::type_specifier* parseSymbol(Parser& parser, cpp::type_specifier* result)
{
	PARSE_SELECT(parser, cpp::type_specifier_noncv);
	PARSE_SELECT(parser, cpp::cv_qualifier);
	return result;
}

inline cpp::decl_specifier_nontype* parseSymbol(Parser& parser, cpp::decl_specifier_nontype* result)
{
	PARSE_SELECT(parser, cpp::storage_class_specifier);
	PARSE_SELECT(parser, cpp::decl_specifier_default);
	PARSE_SELECT(parser, cpp::function_specifier);
	PARSE_SELECT(parser, cpp::cv_qualifier);
	return result;
}

inline cpp::decl_specifier_prefix_seq* parseSymbol(Parser& parser, cpp::decl_specifier_prefix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::decl_specifier_suffix* parseSymbol(Parser& parser, cpp::decl_specifier_suffix* result)
{
	PARSE_SELECT(parser, cpp::decl_specifier_nontype);
	PARSE_SELECT(parser, cpp::simple_type_specifier_builtin);
	return result;
}

inline cpp::decl_specifier_suffix_seq* parseSymbol(Parser& parser, cpp::decl_specifier_suffix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::decl_specifier_seq* parseSymbol(Parser& parser, cpp::decl_specifier_seq* result)
{
	PARSE_OPTIONAL(parser, result->prefix);
	PARSE_REQUIRED(parser, result->type);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::cv_qualifier_seq* parseSymbol(Parser& parser, cpp::cv_qualifier_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::ptr_operator_key* parseSymbol(Parser& parser, cpp::ptr_operator_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::ptr_operator_key::PTR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_AND, cpp::ptr_operator_key::REF);
	return result;
}


inline cpp::ptr_operator* parseSymbol(Parser& parser, cpp::ptr_operator* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->key); // TODO: disallow '&' following 'nested-name-specifier'
	PARSE_OPTIONAL(parser, result->qual);
	return result;
}

inline cpp::declarator_suffix_array* parseSymbol(Parser& parser, cpp::declarator_suffix_array* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_OPTIONAL(parser, result->size);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	return result;
}

inline cpp::declarator* parseSymbol(Parser& parser, cpp::declarator* result);

inline cpp::throw_expression* parseSymbol(Parser& parser, cpp::throw_expression* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_THROW);
	PARSE_OPTIONAL(parser, result->expr);
	return result;
}

inline cpp::unary_operator* parseSymbol(Parser& parser, cpp::unary_operator* result)
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

inline cpp::cast_expression* parseSymbol(Parser& parser, cpp::cast_expression* result);

inline cpp::unary_expression_op* parseSymbol(Parser& parser, cpp::unary_expression_op* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::numeric_literal* parseSymbol(Parser& parser, cpp::numeric_literal* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PP_NUMBER, cpp::numeric_literal::INTEGER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INTLIT, cpp::numeric_literal::INTEGER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CHARLIT, cpp::numeric_literal::CHARACTER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FLOATLIT, cpp::numeric_literal::FLOATING);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TRUE, cpp::numeric_literal::BOOLEAN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FALSE, cpp::numeric_literal::BOOLEAN);
	return result;
}

inline cpp::string_literal* parseSymbol(Parser& parser, cpp::string_literal* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_STRINGLIT))
	{
		parser.increment();
		result->value = parser.get_value();
		PARSE_OPTIONAL(parser, result->next);
		return result;
	}
	return NULL;
}

inline cpp::literal* parseSymbol(Parser& parser, cpp::literal* result)
{
	PARSE_SELECT(parser, cpp::numeric_literal);
	PARSE_SELECT(parser, cpp::string_literal);
	return result;
}

inline cpp::primary_expression_builtin* parseSymbol(Parser& parser, cpp::primary_expression_builtin* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_THIS);
	return result;
}

inline cpp::assignment_expression* parseSymbol(Parser& parser, cpp::assignment_expression* result);

inline cpp::expression_comma* parseSymbol(Parser& parser, cpp::expression_comma* result)
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

inline cpp::expression* parseSymbol(Parser& parser, cpp::expression* result)
{
	PARSE_PREFIX(parser, cpp::expression_comma);
	return result;
}

inline cpp::primary_expression_parenthesis* parseSymbol(Parser& parser, cpp::primary_expression_parenthesis* result)
{
	parser.inTemplateArgumentList = false;
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::id_expression* parseSymbol(Parser& parser, cpp::id_expression* result);

inline cpp::primary_expression* parseSymbol(Parser& parser, cpp::primary_expression* result)
{
	PARSE_SELECT(parser, cpp::literal);
	PARSE_SELECT(parser, cpp::primary_expression_builtin);
	PARSE_SELECT(parser, cpp::primary_expression_parenthesis);
	PARSE_SELECT(parser, cpp::id_expression);
	return result;
}

inline cpp::postfix_expression_index* parseSymbol(Parser& parser, cpp::postfix_expression_index* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_REQUIRED(parser, result->index);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	return result;
}

inline cpp::postfix_expression_call* parseSymbol(Parser& parser, cpp::postfix_expression_call* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::member_operator* parseSymbol(Parser& parser, cpp::member_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOT, cpp::member_operator::DOT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROW, cpp::member_operator::ARROW);
	return result;
}

inline cpp::postfix_expression_member* parseSymbol(Parser& parser, cpp::postfix_expression_member* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::postfix_expression_destructor* parseSymbol(Parser& parser, cpp::postfix_expression_destructor* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COMPL);
	PARSE_REQUIRED(parser, result->type);
	return result;
}

inline cpp::postfix_operator* parseSymbol(Parser& parser, cpp::postfix_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSPLUS, cpp::postfix_operator::PLUSPLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSMINUS, cpp::postfix_operator::MINUSMINUS);
	return result;
}

inline cpp::postfix_expression_construct* parseSymbol(Parser& parser, cpp::postfix_expression_construct* result)
{
	PARSE_REQUIRED(parser, result->type);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::cast_operator* parseSymbol(Parser& parser, cpp::cast_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DYNAMICCAST, cpp::cast_operator::DYNAMIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATICCAST, cpp::cast_operator::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REINTERPRETCAST, cpp::cast_operator::REINTERPRET);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONSTCAST, cpp::cast_operator::CONST);
	return result;
}

inline cpp::postfix_expression_cast* parseSymbol(Parser& parser, cpp::postfix_expression_cast* result)
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

inline cpp::postfix_expression_typeid* parseSymbol(Parser& parser, cpp::postfix_expression_typeid* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TYPEID);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::postfix_expression_typeidtype* parseSymbol(Parser& parser, cpp::postfix_expression_typeidtype* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TYPEID);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->type);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::postfix_expression_suffix* parseSymbol(Parser& parser, cpp::postfix_expression_suffix* result)
{
	PARSE_SELECT(parser, cpp::postfix_expression_index);
	PARSE_SELECT(parser, cpp::postfix_expression_call);
	PARSE_SELECT(parser, cpp::postfix_expression_member);
	PARSE_SELECT(parser, cpp::postfix_expression_destructor);
	PARSE_SELECT(parser, cpp::postfix_operator);
	return result;
}

inline cpp::postfix_expression_prefix* parseSymbol(Parser& parser, cpp::postfix_expression_prefix* result)
{
	PARSE_SELECT(parser, cpp::primary_expression);
	PARSE_SELECT(parser, cpp::postfix_expression_construct);
	PARSE_SELECT(parser, cpp::postfix_expression_cast);
	PARSE_SELECT(parser, cpp::postfix_expression_typeid);
	PARSE_SELECT(parser, cpp::postfix_expression_typeidtype);
	return result;
}

inline cpp::postfix_expression_suffix_seq* parseSymbol(Parser& parser, cpp::postfix_expression_suffix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::postfix_expression_default* parseSymbol(Parser& parser, cpp::postfix_expression_default* result)
{
	PARSE_REQUIRED(parser, result->expr);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::postfix_expression* parseSymbol(Parser& parser, cpp::postfix_expression* result)
{
	PARSE_SELECT(parser, cpp::postfix_expression_default);
	PARSE_SELECT(parser, cpp::postfix_expression_prefix);
	return result;
}

inline cpp::unary_expression_sizeoftype* parseSymbol(Parser& parser, cpp::unary_expression_sizeoftype* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SIZEOF);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->type);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::unary_expression_sizeof* parseSymbol(Parser& parser, cpp::unary_expression_sizeof* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SIZEOF);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::new_declarator_suffix* parseSymbol(Parser& parser, cpp::new_declarator_suffix* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::direct_new_declarator* parseSymbol(Parser& parser, cpp::direct_new_declarator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::new_declarator_ptr* parseSymbol(Parser& parser, cpp::new_declarator_ptr* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::new_declarator* parseSymbol(Parser& parser, cpp::new_declarator* result)
{
	PARSE_SELECT(parser, cpp::direct_new_declarator);
	PARSE_SELECT(parser, cpp::new_declarator_ptr);
	return result;
}

inline cpp::new_type_default* parseSymbol(Parser& parser, cpp::new_type_default* result)
{
	PARSE_REQUIRED(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::new_type_parenthesis* parseSymbol(Parser& parser, cpp::new_type_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::new_type* parseSymbol(Parser& parser, cpp::new_type* result)
{
	PARSE_SELECT(parser, cpp::new_type_parenthesis);
	PARSE_SELECT(parser, cpp::new_type_default);
	return result;
}

inline cpp::new_initializer* parseSymbol(Parser& parser, cpp::new_initializer* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::new_expression_placement* parseSymbol(Parser& parser, cpp::new_expression_placement* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NEW);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->place);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->type);
	PARSE_OPTIONAL(parser, result->init);
	return result;
}

inline cpp::new_expression_default* parseSymbol(Parser& parser, cpp::new_expression_default* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NEW);
	PARSE_REQUIRED(parser, result->type);
	PARSE_OPTIONAL(parser, result->init);
	return result;
}

inline cpp::new_expression* parseSymbol(Parser& parser, cpp::new_expression* result)
{
	PARSE_SELECT(parser, cpp::new_expression_placement); // TODO: ambiguity: 'new-placement' vs parenthesised 'type-id'
	PARSE_SELECT(parser, cpp::new_expression_default);
	return result;
}

inline cpp::delete_expression* parseSymbol(Parser& parser, cpp::delete_expression* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DELETE);
	PARSE_TOKEN_OPTIONAL(parser, result->isArray, boost::wave::T_LEFTBRACKET);
	if(result->isArray)
	{
		PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	}
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::unary_expression* parseSymbol(Parser& parser, cpp::unary_expression* result)
{
	PARSE_SELECT(parser, cpp::postfix_expression);
	PARSE_SELECT(parser, cpp::unary_expression_sizeoftype);
	PARSE_SELECT(parser, cpp::unary_expression_sizeof);
	PARSE_SELECT(parser, cpp::unary_expression_op);
	PARSE_SELECT(parser, cpp::new_expression);
	PARSE_SELECT(parser, cpp::delete_expression);
	return result;
}

inline cpp::type_id* parseSymbol(Parser& parser, cpp::type_id* result)
{
	PARSE_REQUIRED(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::cast_expression_default* parseSymbol(Parser& parser, cpp::cast_expression_default* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::cast_expression* parseSymbol(Parser& parser, cpp::cast_expression* result)
{
	PARSE_SELECT(parser, cpp::cast_expression_default);
	PARSE_SELECT(parser, cpp::unary_expression);
	return result;
}

inline cpp::pm_operator* parseSymbol(Parser& parser, cpp::pm_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOTSTAR, cpp::pm_operator::DOTSTAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROWSTAR, cpp::pm_operator::ARROWSTAR);
	return result;
}

inline cpp::pm_expression_default* parseSymbol(Parser& parser, cpp::pm_expression_default* result)
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

inline cpp::pm_expression* parseSymbol(Parser& parser, cpp::pm_expression* result)
{
	PARSE_PREFIX(parser, cpp::pm_expression_default);
	return result;
}

inline cpp::multiplicative_operator* parseSymbol(Parser& parser, cpp::multiplicative_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::multiplicative_operator::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DIVIDE, cpp::multiplicative_operator::DIVIDE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PERCENT, cpp::multiplicative_operator::PERCENT);
	return result;
}

inline cpp::multiplicative_expression_default* parseSymbol(Parser& parser, cpp::multiplicative_expression_default* result)
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

inline cpp::multiplicative_expression* parseSymbol(Parser& parser, cpp::multiplicative_expression* result)
{
	PARSE_PREFIX(parser, cpp::multiplicative_expression_default);
	return result;
}

inline cpp::additive_operator* parseSymbol(Parser& parser, cpp::additive_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUS, cpp::additive_operator::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUS, cpp::additive_operator::MINUS);
	return result;
}

inline cpp::additive_expression_default* parseSymbol(Parser& parser, cpp::additive_expression_default* result)
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

inline cpp::additive_expression* parseSymbol(Parser& parser, cpp::additive_expression* result)
{
	PARSE_PREFIX(parser, cpp::additive_expression_default);
	return result;
}

inline cpp::shift_operator* parseSymbol(Parser& parser, cpp::shift_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTLEFT, cpp::shift_operator::SHIFTLEFT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTRIGHT, cpp::shift_operator::SHIFTRIGHT);
	return result;
}

inline cpp::shift_expression_default* parseSymbol(Parser& parser, cpp::shift_expression_default* result)
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

inline cpp::shift_expression* parseSymbol(Parser& parser, cpp::shift_expression* result)
{
	PARSE_PREFIX(parser, cpp::shift_expression_default);
	return result;
}

inline cpp::relational_operator* parseSymbol(Parser& parser, cpp::relational_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESS, cpp::relational_operator::LESS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATER, cpp::relational_operator::GREATER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_LESSEQUAL, cpp::relational_operator::LESSEQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_GREATEREQUAL, cpp::relational_operator::GREATEREQUAL);
	return result;
}

inline cpp::relational_expression_default* parseSymbol(Parser& parser, cpp::relational_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	result->right = NULL;
	if(!parser.inTemplateArgumentList
		|| !TOKEN_EQUAL(parser, boost::wave::T_GREATER)) // '>' terminates template-argument-list
	{
		PARSE_OPTIONAL(parser, result->op);
		if(result->op != NULL)
		{
			PARSE_REQUIRED(parser, result->right);
		}
	}
	return result;
}

inline cpp::relational_expression* parseSymbol(Parser& parser, cpp::relational_expression* result)
{
	PARSE_PREFIX(parser, cpp::relational_expression_default);
	return result;
}

inline cpp::equality_operator* parseSymbol(Parser& parser, cpp::equality_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EQUAL, cpp::equality_operator::EQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOTEQUAL, cpp::equality_operator::NOTEQUAL);
	return result;
}

inline cpp::equality_expression_default* parseSymbol(Parser& parser, cpp::equality_expression_default* result)
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

inline cpp::equality_expression* parseSymbol(Parser& parser, cpp::equality_expression* result)
{
	PARSE_PREFIX(parser, cpp::equality_expression_default);
	return result;
}

inline cpp::and_expression_default* parseSymbol(Parser& parser, cpp::and_expression_default* result)
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

inline cpp::and_expression* parseSymbol(Parser& parser, cpp::and_expression* result)
{
	PARSE_PREFIX(parser, cpp::and_expression_default);
	return result;
}

inline cpp::exclusive_or_expression_default* parseSymbol(Parser& parser, cpp::exclusive_or_expression_default* result)
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

inline cpp::exclusive_or_expression* parseSymbol(Parser& parser, cpp::exclusive_or_expression* result)
{
	PARSE_PREFIX(parser, cpp::exclusive_or_expression_default);
	return result;
}

inline cpp::inclusive_or_expression_default* parseSymbol(Parser& parser, cpp::inclusive_or_expression_default* result)
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

inline cpp::inclusive_or_expression* parseSymbol(Parser& parser, cpp::inclusive_or_expression* result)
{
	PARSE_PREFIX(parser, cpp::inclusive_or_expression_default);
	return result;
}

inline cpp::logical_and_expression_default* parseSymbol(Parser& parser, cpp::logical_and_expression_default* result)
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

inline cpp::logical_and_expression* parseSymbol(Parser& parser, cpp::logical_and_expression* result)
{
	PARSE_PREFIX(parser, cpp::logical_and_expression_default);
	return result;
}

inline cpp::logical_or_expression_default* parseSymbol(Parser& parser, cpp::logical_or_expression_default* result)
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

inline cpp::logical_or_expression* parseSymbol(Parser& parser, cpp::logical_or_expression* result)
{
	PARSE_PREFIX(parser, cpp::logical_or_expression_default);
	return result;
}

inline cpp::conditional_expression_rhs* parseSymbol(Parser& parser, cpp::conditional_expression_rhs* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_QUESTION_MARK);
	PARSE_REQUIRED(parser, result->mid);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::assignment_operator* parseSymbol(Parser& parser, cpp::assignment_operator* result)
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

inline cpp::assignment_expression* parseSymbol(Parser& parser, cpp::assignment_expression* result);

inline cpp::assignment_expression_rhs* parseSymbol(Parser& parser, cpp::assignment_expression_rhs* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::logical_or_expression_suffix* parseSymbol(Parser& parser, cpp::logical_or_expression_suffix* result)
{
	PARSE_SELECT(parser, cpp::conditional_expression_rhs);
	PARSE_SELECT(parser, cpp::assignment_expression_rhs);
	return result;
}

inline cpp::logical_or_expression_precedent* parseSymbol(Parser& parser, cpp::logical_or_expression_precedent* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

inline cpp::conditional_expression_default* parseSymbol(Parser& parser, cpp::conditional_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

inline cpp::conditional_expression* parseSymbol(Parser& parser, cpp::conditional_expression* result)
{
	PARSE_PREFIX(parser, cpp::conditional_expression_default);
	return result;
}


inline cpp::assignment_expression* parseSymbol(Parser& parser, cpp::assignment_expression* result)
{
	PARSE_SELECT(parser, cpp::throw_expression);
	PARSE_PREFIX(parser, cpp::logical_or_expression_precedent);
	return result;
}

inline cpp::parameter_declaration_default* parseSymbol(Parser& parser, cpp::parameter_declaration_default* result)
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

inline cpp::abstract_declarator* parseSymbol(Parser& parser, cpp::abstract_declarator* result);

inline cpp::abstract_declarator_ptr* parseSymbol(Parser& parser, cpp::abstract_declarator_ptr* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::abstract_declarator_parenthesis* parseSymbol(Parser& parser, cpp::abstract_declarator_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::direct_abstract_declarator* parseSymbol(Parser& parser, cpp::direct_abstract_declarator* result)
{
	PARSE_OPTIONAL(parser, result->prefix);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::abstract_declarator* parseSymbol(Parser& parser, cpp::abstract_declarator* result)
{
	PARSE_SELECT(parser, cpp::abstract_declarator_ptr);
	PARSE_SELECT(parser, cpp::direct_abstract_declarator);
	return result;
}

inline cpp::parameter_declaration_abstract* parseSymbol(Parser& parser, cpp::parameter_declaration_abstract* result)
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

inline cpp::parameter_declaration* parseSymbol(Parser& parser, cpp::parameter_declaration* result)
{
	PARSE_SELECT(parser, cpp::parameter_declaration_default);
	PARSE_SELECT(parser, cpp::parameter_declaration_abstract);
	return result;
}

inline cpp::parameter_declaration_list* parseSymbol(Parser& parser, cpp::parameter_declaration_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		if(!TOKEN_EQUAL(parser, boost::wave::T_ELLIPSIS))
		{
			PARSE_REQUIRED(parser, result->next);
		}
	}
	return result;
}

inline cpp::parameter_declaration_clause* parseSymbol(Parser& parser, cpp::parameter_declaration_clause* result)
{
	PARSE_OPTIONAL(parser, result->list);
	PARSE_TOKEN_OPTIONAL(parser, result->isEllipsis, boost::wave::T_ELLIPSIS);
	return result;
}

inline cpp::type_id_list* parseSymbol(Parser& parser, cpp::type_id_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::exception_specification* parseSymbol(Parser& parser, cpp::exception_specification* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_THROW);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	if(TOKEN_EQUAL(parser, boost::wave::T_ELLIPSIS)) // TODO: used in msvc stdc, may not be standard?
	{
		result->types = NULL;
		parser.increment();
	}
	else
	{
		PARSE_OPTIONAL(parser, result->types);
	}
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::declarator_suffix_function* parseSymbol(Parser& parser, cpp::declarator_suffix_function* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->params);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_OPTIONAL(parser, result->qual);
	PARSE_OPTIONAL(parser, result->except);
	return result;
}

inline cpp::declarator_suffix* parseSymbol(Parser& parser, cpp::declarator_suffix* result)
{
	PARSE_SELECT(parser, cpp::declarator_suffix_array);
	PARSE_SELECT(parser, cpp::declarator_suffix_function);
	return result;
}

inline cpp::declarator_suffix_seq* parseSymbol(Parser& parser, cpp::declarator_suffix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::direct_declarator_parenthesis* parseSymbol(Parser& parser, cpp::direct_declarator_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::comma_operator* parseSymbol(Parser& parser, cpp::comma_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COMMA);
	return result;
};

inline cpp::function_operator* parseSymbol(Parser& parser, cpp::function_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
};

inline cpp::array_operator* parseSymbol(Parser& parser, cpp::array_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	return result;
};

inline cpp::new_operator* parseSymbol(Parser& parser, cpp::new_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NEW);
	PARSE_OPTIONAL(parser, result->array);
	return result;
};

inline cpp::delete_operator* parseSymbol(Parser& parser, cpp::delete_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DELETE);
	PARSE_OPTIONAL(parser, result->array);
	return result;
};

inline cpp::overloadable_operator* parseSymbol(Parser& parser, cpp::overloadable_operator* result)
{
	PARSE_SELECT(parser, cpp::assignment_operator);
	PARSE_SELECT(parser, cpp::member_operator);
	PARSE_SELECT(parser, cpp::postfix_operator);
	PARSE_SELECT(parser, cpp::unary_operator);
	PARSE_SELECT(parser, cpp::pm_operator);
	PARSE_SELECT(parser, cpp::multiplicative_operator);
	PARSE_SELECT(parser, cpp::additive_operator);
	PARSE_SELECT(parser, cpp::shift_operator);
	PARSE_SELECT(parser, cpp::relational_operator);
	PARSE_SELECT(parser, cpp::equality_operator);
	PARSE_SELECT(parser, cpp::multiplicative_operator);
	PARSE_SELECT(parser, cpp::new_operator);
	PARSE_SELECT(parser, cpp::delete_operator);
	PARSE_SELECT(parser, cpp::comma_operator);
	PARSE_SELECT(parser, cpp::function_operator);
	PARSE_SELECT(parser, cpp::array_operator);
	return result;
};

inline cpp::operator_function_id* parseSymbol(Parser& parser, cpp::operator_function_id* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_OPERATOR);
	PARSE_REQUIRED(parser, result->op);
	if(TOKEN_EQUAL(parser, boost::wave::T_LESS))
	{
		parser.increment();
		PARSE_OPTIONAL(parser, result->args);
		PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	}
	return result;
}

inline cpp::conversion_declarator* parseSymbol(Parser& parser, cpp::conversion_declarator* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::conversion_function_id* parseSymbol(Parser& parser, cpp::conversion_function_id* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_OPERATOR);
	PARSE_REQUIRED(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::template_id_operator_function* parseSymbol(Parser& parser, cpp::template_id_operator_function* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	return result;
}

inline cpp::template_id* parseSymbol(Parser& parser, cpp::template_id* result)
{
	PARSE_SELECT(parser, cpp::simple_template_id);
	PARSE_SELECT(parser, cpp::template_id_operator_function);
	return result;
}

inline cpp::destructor_id* parseSymbol(Parser& parser, cpp::destructor_id* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COMPL);
	PARSE_REQUIRED(parser, result->name);
	return result;
}

inline cpp::unqualified_id* parseSymbol(Parser& parser, cpp::unqualified_id* result)
{
	PARSE_SELECT(parser, cpp::template_id); // todo: shared-prefix ambiguity: 'template-id' vs 'identifier'
	PARSE_SELECT(parser, cpp::identifier);
	PARSE_SELECT(parser, cpp::operator_function_id);
	PARSE_SELECT(parser, cpp::conversion_function_id);
	PARSE_SELECT(parser, cpp::destructor_id);
	return result;
}

inline cpp::qualified_id_default* parseSymbol(Parser& parser, cpp::qualified_id_default* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id_global* parseSymbol(Parser& parser, cpp::qualified_id_global* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id_global_template* parseSymbol(Parser& parser, cpp::qualified_id_global_template* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id_global_op_func* parseSymbol(Parser& parser, cpp::qualified_id_global_op_func* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id* parseSymbol(Parser& parser, cpp::qualified_id* result)
{
	PARSE_SELECT(parser, cpp::qualified_id_default);
	PARSE_SELECT(parser, cpp::qualified_id_global);
	PARSE_SELECT(parser, cpp::qualified_id_global_template);
	PARSE_SELECT(parser, cpp::qualified_id_global_op_func);
	return result;
}

inline cpp::id_expression* parseSymbol(Parser& parser, cpp::id_expression* result)
{
	PARSE_SELECT(parser, cpp::qualified_id); // TODO: shared prefix ambiguity: 'identifier' vs 'nested-name-specifier'
	PARSE_SELECT(parser, cpp::unqualified_id);
	return result;
}

inline cpp::declarator_id* parseSymbol(Parser& parser, cpp::declarator_id* result)
{
	PARSE_SELECT(parser, cpp::id_expression);
	// TODO: qualified form
	return result;
}

inline cpp::direct_declarator_prefix* parseSymbol(Parser& parser, cpp::direct_declarator_prefix* result)
{
	PARSE_SELECT(parser, cpp::direct_declarator_parenthesis);
	PARSE_SELECT(parser, cpp::declarator_id);
	return result;
}

inline cpp::direct_declarator* parseSymbol(Parser& parser, cpp::direct_declarator* result)
{
	PARSE_REQUIRED(parser, result->prefix);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::declarator_ptr* parseSymbol(Parser& parser, cpp::declarator_ptr* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::declarator* parseSymbol(Parser& parser, cpp::declarator* result)
{
	PARSE_SELECT(parser, cpp::declarator_ptr);
	PARSE_SELECT(parser, cpp::direct_declarator);
	return result;
}

inline cpp::function_definition_suffix* parseSymbol(Parser& parser, cpp::function_definition_suffix* result)
{
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

inline cpp::function_definition* parseSymbol(Parser& parser, cpp::function_definition* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::linkage_specification_compound* parseSymbol(Parser& parser, cpp::linkage_specification_compound* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::linkage_specification_suffix* parseSymbol(Parser& parser, cpp::linkage_specification_suffix* result)
{
	PARSE_SELECT(parser, cpp::linkage_specification_compound);
	PARSE_SELECT(parser, cpp::declaration);
	return result;
}

inline cpp::linkage_specification* parseSymbol(Parser& parser, cpp::linkage_specification* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_EXTERN);
	PARSE_REQUIRED(parser, result->str);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::explicit_instantiation* parseSymbol(Parser& parser, cpp::explicit_instantiation* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isExtern, boost::wave::T_EXTERN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::explicit_specialization* parseSymbol(Parser& parser, cpp::explicit_specialization* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::expression_list* parseSymbol(Parser& parser, cpp::expression_list* result)
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

inline cpp::initializer_clause* parseSymbol(Parser& parser, cpp::initializer_clause* result);

inline cpp::initializer_list* parseSymbol(Parser& parser, cpp::initializer_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_OPTIONAL(parser, result->next);
	}
	return result;
}

inline cpp::initializer_clause_list* parseSymbol(Parser& parser, cpp::initializer_clause_list* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	if(TOKEN_EQUAL(parser, boost::wave::T_RIGHTBRACE))
	{
		parser.increment();
		result->list = NULL;
		return result;
	}
	PARSE_REQUIRED(parser, result->list);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::initializer_clause* parseSymbol(Parser& parser, cpp::initializer_clause* result)
{
	PARSE_SELECT(parser, cpp::initializer_clause_list);
	PARSE_SELECT(parser, cpp::assignment_expression);
	return result;
}

inline cpp::initializer_default* parseSymbol(Parser& parser, cpp::initializer_default* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_REQUIRED(parser, result->clause);
	return result;
}

inline cpp::initializer_parenthesis* parseSymbol(Parser& parser, cpp::initializer_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->list);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::initializer* parseSymbol(Parser& parser, cpp::initializer* result)
{
	PARSE_SELECT(parser, cpp::initializer_default);
	PARSE_SELECT(parser, cpp::initializer_parenthesis);
	return result;
}

inline cpp::init_declarator* parseSymbol(Parser& parser, cpp::init_declarator* result)
{
	PARSE_REQUIRED(parser, result->decl);
	PARSE_OPTIONAL(parser, result->init);
	return result;
}

inline cpp::init_declarator_list* parseSymbol(Parser& parser, cpp::init_declarator_list* result)
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

inline cpp::simple_declaration_suffix* parseSymbol(Parser& parser, cpp::simple_declaration_suffix* result)
{
	PARSE_OPTIONAL(parser, result->init);
	result->next = NULL;
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::simple_declaration* parseSymbol(Parser& parser, cpp::simple_declaration* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::asm_definition* parseSymbol(Parser& parser, cpp::asm_definition* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASM);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->str);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::namespace_alias_definition* parseSymbol(Parser& parser, cpp::namespace_alias_definition* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NAMESPACE);
	PARSE_REQUIRED(parser, result->alias);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::using_declaration_global* parseSymbol(Parser& parser, cpp::using_declaration_global* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_USING);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::using_declaration_nested* parseSymbol(Parser& parser, cpp::using_declaration_nested* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_USING);
	PARSE_TOKEN_OPTIONAL(parser, result->isTypename, boost::wave::T_TYPENAME);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::using_declaration* parseSymbol(Parser& parser, cpp::using_declaration* result)
{
	PARSE_SELECT(parser, cpp::using_declaration_global);
	PARSE_SELECT(parser, cpp::using_declaration_nested);
	return result;
}

inline cpp::using_directive* parseSymbol(Parser& parser, cpp::using_directive* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_USING);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NAMESPACE);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::block_declaration* parseSymbol(Parser& parser, cpp::block_declaration* result)
{
	PARSE_SELECT(parser, cpp::simple_declaration);
	PARSE_SELECT(parser, cpp::asm_definition);
	PARSE_SELECT(parser, cpp::namespace_alias_definition);
	PARSE_SELECT(parser, cpp::using_declaration);
	PARSE_SELECT(parser, cpp::using_directive);
	return result;
}

inline cpp::general_declaration_suffix* parseSymbol(Parser& parser, cpp::general_declaration_suffix* result)
{
	PARSE_SELECT(parser, cpp::function_definition_suffix);
	PARSE_SELECT(parser, cpp::simple_declaration_suffix);
	return result;
}

inline cpp::general_declaration* parseSymbol(Parser& parser, cpp::general_declaration* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl); // TODO: making this optional permits parsing function-definition with no declarator
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::declaration* parseSymbol(Parser& parser, cpp::declaration* result)
{
	PARSE_SELECT(parser, cpp::linkage_specification);
	PARSE_SELECT(parser, cpp::explicit_instantiation);
	PARSE_SELECT(parser, cpp::template_declaration);
	PARSE_SELECT(parser, cpp::explicit_specialization);
	PARSE_SELECT(parser, cpp::namespace_definition);
	PARSE_SELECT(parser, cpp::general_declaration);
	PARSE_SELECT(parser, cpp::constructor_definition);
	PARSE_SELECT(parser, cpp::block_declaration);
	return result;
}

inline cpp::declaration_seq* parseSymbol(Parser& parser, cpp::declaration_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::statement_seq* parseSymbol(Parser& parser, cpp::statement_seq* result);

inline cpp::compound_statement* parseSymbol(Parser& parser, cpp::compound_statement* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->body);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::declaration_statement* parseSymbol(Parser& parser, cpp::declaration_statement* result)
{
	PARSE_SELECT(parser, cpp::block_declaration);
	return result;
}

inline cpp::labeled_statement_id* parseSymbol(Parser& parser, cpp::labeled_statement_id* result)
{
	PARSE_REQUIRED(parser, result->label);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::labeled_statement_case* parseSymbol(Parser& parser, cpp::labeled_statement_case* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_CASE);
	PARSE_REQUIRED(parser, result->label);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::labeled_statement_default* parseSymbol(Parser& parser, cpp::labeled_statement_default* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DEFAULT);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::labeled_statement* parseSymbol(Parser& parser, cpp::labeled_statement* result)
{
	PARSE_SELECT(parser, cpp::labeled_statement_id);
	PARSE_SELECT(parser, cpp::labeled_statement_case);
	PARSE_SELECT(parser, cpp::labeled_statement_default);
	return result;
}

inline cpp::type_specifier_prefix_seq* parseSymbol(Parser& parser, cpp::type_specifier_prefix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::type_specifier_suffix* parseSymbol(Parser& parser, cpp::type_specifier_suffix* result)
{
	PARSE_SELECT(parser, cpp::cv_qualifier);
	PARSE_SELECT(parser, cpp::simple_type_specifier_builtin);
	return result;
}

inline cpp::type_specifier_suffix_seq* parseSymbol(Parser& parser, cpp::type_specifier_suffix_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::type_specifier_seq* parseSymbol(Parser& parser, cpp::type_specifier_seq* result)
{
	PARSE_OPTIONAL(parser, result->prefix);
	PARSE_REQUIRED(parser, result->type);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}


inline cpp::condition_init* parseSymbol(Parser& parser, cpp::condition_init* result)
{
	PARSE_REQUIRED(parser, result->type);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_REQUIRED(parser, result->init);
	return result;
}

inline cpp::condition* parseSymbol(Parser& parser, cpp::condition* result)
{
	PARSE_SELECT(parser, cpp::condition_init); // ambiguity: 'Type x = y': match init statement first, because expression resolves to 'primary-expression'
	PARSE_SELECT(parser, cpp::expression);
	return result;
}

inline cpp::selection_statement_if* parseSymbol(Parser& parser, cpp::selection_statement_if* result)
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

inline cpp::selection_statement_switch* parseSymbol(Parser& parser, cpp::selection_statement_switch* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SWITCH);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::selection_statement* parseSymbol(Parser& parser, cpp::selection_statement* result)
{
	PARSE_SELECT(parser, cpp::selection_statement_if);
	PARSE_SELECT(parser, cpp::selection_statement_switch);
	return result;
}

inline cpp::for_init_statement* parseSymbol(Parser& parser, cpp::for_init_statement* result)
{
	PARSE_SELECT(parser, cpp::expression_statement);
	PARSE_SELECT(parser, cpp::simple_declaration);
	return result;
}

inline cpp::iteration_statement_for* parseSymbol(Parser& parser, cpp::iteration_statement_for* result)
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

inline cpp::iteration_statement_while* parseSymbol(Parser& parser, cpp::iteration_statement_while* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_WHILE);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::iteration_statement_dowhile* parseSymbol(Parser& parser, cpp::iteration_statement_dowhile* result)
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

inline cpp::iteration_statement* parseSymbol(Parser& parser, cpp::iteration_statement* result)
{
	PARSE_SELECT(parser, cpp::iteration_statement_for);
	PARSE_SELECT(parser, cpp::iteration_statement_while);
	PARSE_SELECT(parser, cpp::iteration_statement_dowhile);
	return result;
}

inline cpp::jump_statement_key* parseSymbol(Parser& parser, cpp::jump_statement_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_BREAK, cpp::jump_statement_key::BREAK);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONTINUE, cpp::jump_statement_key::CONTINUE);
	return result;
}

inline cpp::jump_statement_simple* parseSymbol(Parser& parser, cpp::jump_statement_simple* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::jump_statement_return* parseSymbol(Parser& parser, cpp::jump_statement_return* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RETURN);
	PARSE_OPTIONAL(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::jump_statement_goto* parseSymbol(Parser& parser, cpp::jump_statement_goto* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GOTO);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::jump_statement* parseSymbol(Parser& parser, cpp::jump_statement* result)
{
	PARSE_SELECT(parser, cpp::jump_statement_simple);
	PARSE_SELECT(parser, cpp::jump_statement_return);
	PARSE_SELECT(parser, cpp::jump_statement_goto);
	return result;
}

inline cpp::try_block* parseSymbol(Parser& parser, cpp::try_block* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TRY);
	PARSE_REQUIRED(parser, result->body);
	PARSE_REQUIRED(parser, result->handlers);
	return result;
}

inline cpp::expression_statement* parseSymbol(Parser& parser, cpp::expression_statement* result)
{
	PARSE_OPTIONAL(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::msext_asm_statement* parseSymbol(Parser& parser, cpp::msext_asm_statement* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_MSEXT_ASM);
	bool leadingBrace;
	PARSE_TOKEN_OPTIONAL(parser, leadingBrace, boost::wave::T_LEFTBRACE);
	size_t line = parser.get_position().get_line();
	size_t depth = size_t(leadingBrace);
	for(;;)
	{
		if(!leadingBrace
			&& parser.get_position().get_line() != line) // HACK!
		{
			break;
		}
		if(depth == 0
			&& TOKEN_EQUAL(parser, boost::wave::T_SEMICOLON))
		{
			break;
		}
		if(depth == size_t(leadingBrace)
			&& TOKEN_EQUAL(parser, boost::wave::T_RIGHTBRACE))
		{
			if(leadingBrace)
			{
				parser.increment();
			}
			break;
		}
		if(TOKEN_EQUAL(parser, boost::wave::T_LEFTBRACE))
		{
			parser.increment();
			++depth;
		}
		else if(TOKEN_EQUAL(parser, boost::wave::T_RIGHTBRACE))
		{
			parser.increment();
			--depth;
		}
		else if(TOKEN_EQUAL(parser, boost::wave::T_MSEXT_ASM))
		{
			PARSE_REQUIRED(parser, result->inner);
		}
		else
		{
			parser.increment();
		}
	}
	bool trailingSemicolon;
	PARSE_TOKEN_OPTIONAL(parser, trailingSemicolon, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::statement* parseSymbol(Parser& parser, cpp::statement* result)
{
	PARSE_SELECT(parser, cpp::msext_asm_statement);
	PARSE_SELECT(parser, cpp::compound_statement);
	PARSE_SELECT(parser, cpp::declaration_statement);
	PARSE_SELECT(parser, cpp::labeled_statement);
	PARSE_SELECT(parser, cpp::expression_statement);
	PARSE_SELECT(parser, cpp::selection_statement);
	PARSE_SELECT(parser, cpp::iteration_statement);
	PARSE_SELECT(parser, cpp::jump_statement);
	PARSE_SELECT(parser, cpp::try_block);
	return result;
}

inline cpp::statement_seq* parseSymbol(Parser& parser, cpp::statement_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}


cpp::declaration_seq* parseFile(Scanner& scanner)
{
	Parser parser(scanner);

	cpp::declaration_seq* result = NULL;
	PARSE_OPTIONAL(parser, result);
	if(!scanner.finished())
	{
		printError(parser);
	}
	return result;
}

cpp::statement_seq* parseFunction(Scanner& scanner)
{
	Parser parser(scanner);

	cpp::statement_seq* result = NULL;
	PARSE_OPTIONAL(parser, result);
	if(!scanner.finished())
	{
		printError(parser);
	}
	return result;
}

