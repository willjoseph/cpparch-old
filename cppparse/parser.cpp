
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

	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT8, cpp::simple_type_specifier_builtin::INT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT16, cpp::simple_type_specifier_builtin::INT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT32, cpp::simple_type_specifier_builtin::INT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MSEXT_INT64, cpp::simple_type_specifier_builtin::INT);
	return NULL;
}

inline cpp::decl_specifier_default* parseNode(Parser& parser, cpp::decl_specifier_default* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FRIEND, cpp::decl_specifier_default::FRIEND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPEDEF, cpp::decl_specifier_default::TYPEDEF);
	return NULL;
}


inline cpp::class_name* parseNode(Parser& parser, cpp::class_name* result)
{
	PARSE_SELECT(parser, cpp::simple_template_id); // TODO: ambiguity: shared prefix 'identifier'
	PARSE_SELECT(parser, cpp::identifier);
	return NULL;
}

inline cpp::type_name* parseNode(Parser& parser, cpp::type_name* result)
{
	PARSE_SELECT(parser, cpp::class_name); // NOTE: not distinguishing between class/enum/typedef names
	return NULL;
}

inline cpp::template_argument* parseNode(Parser& parser, cpp::template_argument* result)
{
	PARSE_SELECT(parser, cpp::assignment_expression);
	PARSE_SELECT(parser, cpp::type_id);
	PARSE_SELECT(parser, cpp::id_expression);
	return NULL;
}

inline cpp::template_argument_list* parseNode(Parser& parser, cpp::template_argument_list* result)
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

inline cpp::simple_template_id* parseNode(Parser& parser, cpp::simple_template_id* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_REQUIRED(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	return result;
}

inline cpp::nested_name_specifier_prefix* parseNode(Parser& parser, cpp::nested_name_specifier_prefix* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	return result;
}

inline cpp::nested_name_specifier_suffix* parseNode(Parser& parser, cpp::nested_name_specifier_suffix* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
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
inline cpp::parameter_declaration* parseNode(Parser& parser, cpp::parameter_declaration* result);

inline cpp::type_parameter_key* parseNode(Parser& parser, cpp::type_parameter_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CLASS, cpp::type_parameter_key::CLASS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPENAME, cpp::type_parameter_key::TYPENAME);
	return NULL;
}

inline cpp::type_parameter_default* parseNode(Parser& parser, cpp::type_parameter_default* result)
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

inline cpp::type_parameter_template* parseNode(Parser& parser, cpp::type_parameter_template* result)
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

inline cpp::type_parameter* parseNode(Parser& parser, cpp::type_parameter* result)
{
	PARSE_SELECT(parser, cpp::type_parameter_default);
	PARSE_SELECT(parser, cpp::type_parameter_template);
	return NULL;
}

inline cpp::template_parameter* parseNode(Parser& parser, cpp::template_parameter* result)
{
	PARSE_SELECT(parser, cpp::type_parameter);
	PARSE_SELECT(parser, cpp::parameter_declaration);
	return NULL;
}

inline cpp::template_parameter_list* parseNode(Parser& parser, cpp::template_parameter_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::template_declaration* parseNode(Parser& parser, cpp::template_declaration* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isExport, boost::wave::T_EXPORT);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_REQUIRED(parser, result->params);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	return result;
}

inline cpp::constant_expression* parseNode(Parser& parser, cpp::constant_expression* result)
{
	PARSE_SELECT(parser, cpp::conditional_expression);
	return NULL;
}

inline cpp::member_declarator_pure* parseNode(Parser& parser, cpp::member_declarator_pure* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DECIMALINT); // TODO: check value is zero
	return result;
}

inline cpp::constant_initializer* parseNode(Parser& parser, cpp::constant_initializer* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASSIGN);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::member_declarator_suffix* parseNode(Parser& parser, cpp::member_declarator_suffix* result)
{
	PARSE_SELECT(parser, cpp::constant_initializer); // TODO: ambiguity here!
	PARSE_SELECT(parser, cpp::member_declarator_pure);
	return NULL;
}

inline cpp::member_declarator_bitfield* parseNode(Parser& parser, cpp::member_declarator_bitfield* result)
{
	PARSE_OPTIONAL(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->width);
	return result;
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
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::ctor_specifier_seq* parseNode(Parser& parser, cpp::ctor_specifier_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::mem_initializer* parseNode(Parser& parser, cpp::mem_initializer* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::mem_initializer_list* parseNode(Parser& parser, cpp::mem_initializer_list* result)
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

inline cpp::ctor_initializer* parseNode(Parser& parser, cpp::ctor_initializer* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->list);
	return result;
}

inline cpp::compound_statement* parseNode(Parser& parser, cpp::compound_statement* result);

inline cpp::function_body* parseNode(Parser& parser, cpp::function_body* result)
{
	PARSE_SELECT(parser, cpp::compound_statement);
	return NULL;
}

inline cpp::exception_declarator* parseNode(Parser& parser, cpp::exception_declarator* result)
{
	PARSE_SELECT(parser, cpp::declarator);
	PARSE_SELECT(parser, cpp::abstract_declarator);
	return NULL;
}

inline cpp::exception_declaration_default* parseNode(Parser& parser, cpp::exception_declaration_default* result)
{
	PARSE_REQUIRED(parser, result->type);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::exception_declaration_all* parseNode(Parser& parser, cpp::exception_declaration_all* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ELLIPSIS);
	return result;
}

inline cpp::exception_declaration* parseNode(Parser& parser, cpp::exception_declaration* result)
{
	PARSE_SELECT(parser, cpp::exception_declaration_all);
	PARSE_SELECT(parser, cpp::exception_declaration_default);
	return NULL;
}

inline cpp::handler_seq* parseNode(Parser& parser, cpp::handler_seq* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_CATCH);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_REQUIRED(parser, result->body);
	PARSE_OPTIONAL(parser, result->next);
	return result;
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
	PARSE_SELECT(parser, cpp::member_declaration_inline); // shared-prefix ambiguity: 
	PARSE_SELECT(parser, cpp::member_declaration_ctor); // this matches a constructor: Class(Type);
	PARSE_SELECT(parser, cpp::member_declaration_default); // this matches a member: Type(member);
	PARSE_SELECT(parser, cpp::member_declaration_nested);
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
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
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

inline cpp::base_specifier_access_virtual* parseNode(Parser& parser, cpp::base_specifier_access_virtual* result)
{
	PARSE_REQUIRED(parser, result->access);
	PARSE_TOKEN_OPTIONAL(parser, result->isVirtual, boost::wave::T_VIRTUAL);
	return result;
}

inline cpp::base_specifier_virtual_access* parseNode(Parser& parser, cpp::base_specifier_virtual_access* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_VIRTUAL);
	PARSE_OPTIONAL(parser, result->access);
	return result;
}

inline cpp::base_specifier_prefix* parseNode(Parser& parser, cpp::base_specifier_prefix* result)
{
	PARSE_SELECT(parser, cpp::base_specifier_access_virtual);
	PARSE_SELECT(parser, cpp::base_specifier_virtual_access);
	return NULL;
}

inline cpp::base_specifier* parseNode(Parser& parser, cpp::base_specifier* result)
{
	PARSE_OPTIONAL(parser, result->prefix);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::base_specifier_list* parseNode(Parser& parser, cpp::base_specifier_list* result)
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

inline cpp::base_clause* parseNode(Parser& parser, cpp::base_clause* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->list);
	return result;
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
	PARSE_OPTIONAL(parser, result->context);
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

inline cpp::enumerator_definition* parseNode(Parser& parser, cpp::enumerator_definition* result)
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

inline cpp::enumerator_list* parseNode(Parser& parser, cpp::enumerator_list* result)
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

inline cpp::enum_specifier* parseNode(Parser& parser, cpp::enum_specifier* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ENUM);
	PARSE_OPTIONAL(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->values);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::enum_key* parseNode(Parser& parser, cpp::enum_key* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ENUM);
	return result;
}

inline cpp::elaborated_type_specifier_key* parseNode(Parser& parser, cpp::elaborated_type_specifier_key* result)
{
	PARSE_SELECT(parser, cpp::class_key);
	PARSE_SELECT(parser, cpp::enum_key);
	return NULL;
}

inline cpp::elaborated_type_specifier_default* parseNode(Parser& parser, cpp::elaborated_type_specifier_default* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::elaborated_type_specifier_template* parseNode(Parser& parser, cpp::elaborated_type_specifier_template* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::elaborated_type_specifier* parseNode(Parser& parser, cpp::elaborated_type_specifier* result)
{
	PARSE_SELECT(parser, cpp::elaborated_type_specifier_template); // TODO: shared-prefix ambiguity: match 'simple-template-id' before 'identifier' first
	PARSE_SELECT(parser, cpp::elaborated_type_specifier_default);
	return NULL;
}

inline cpp::typename_specifier* parseNode(Parser& parser, cpp::typename_specifier* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TYPENAME);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context); // TODO: check standard to see if this should be optional or required
	PARSE_TOKEN_OPTIONAL(parser, result->isTemplate, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::type_specifier_noncv* parseNode(Parser& parser, cpp::type_specifier_noncv* result)
{
	PARSE_SELECT(parser, cpp::typename_specifier);
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

inline cpp::extended_decl_modifier_simple* parseNode(Parser& parser, cpp::extended_decl_modifier_simple* result)
{
	// TODO
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_IDENTIFIER, cpp::extended_decl_modifier_simple::NAKED);
	return NULL;
}

inline cpp::extended_decl_modifier_align* parseNode(Parser& parser, cpp::extended_decl_modifier_align* result)
{
	// TODO
	return NULL;
}

inline cpp::extended_decl_modifier_allocate* parseNode(Parser& parser, cpp::extended_decl_modifier_allocate* result)
{
	// TODO
	return NULL;
}

inline cpp::extended_decl_modifier_uuid* parseNode(Parser& parser, cpp::extended_decl_modifier_uuid* result)
{
	// TODO
	return NULL;
}

inline cpp::extended_decl_modifier_property* parseNode(Parser& parser, cpp::extended_decl_modifier_property* result)
{
	// TODO
	return NULL;
}

inline cpp::extended_decl_modifier* parseNode(Parser& parser, cpp::extended_decl_modifier* result)
{
	PARSE_SELECT(parser, cpp::extended_decl_modifier_simple);
	PARSE_SELECT(parser, cpp::extended_decl_modifier_align);
	PARSE_SELECT(parser, cpp::extended_decl_modifier_allocate);
	PARSE_SELECT(parser, cpp::extended_decl_modifier_uuid);
	PARSE_SELECT(parser, cpp::extended_decl_modifier_property);
	return NULL;
}

inline cpp::extended_decl_modifier_seq* parseNode(Parser& parser, cpp::extended_decl_modifier_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::extended_decl_specifier* parseNode(Parser& parser, cpp::extended_decl_specifier* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_MSEXT_DECLSPEC);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->mods);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::decl_specifier_nontype* parseNode(Parser& parser, cpp::decl_specifier_nontype* result)
{
	PARSE_SELECT(parser, cpp::storage_class_specifier);
	PARSE_SELECT(parser, cpp::decl_specifier_default);
	PARSE_SELECT(parser, cpp::function_specifier);
	PARSE_SELECT(parser, cpp::cv_qualifier);
	PARSE_SELECT(parser, cpp::extended_decl_specifier);
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
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_THROW);
	PARSE_OPTIONAL(parser, result->expr);
	return result;
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

inline cpp::numeric_literal* parseNode(Parser& parser, cpp::numeric_literal* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PP_NUMBER, cpp::numeric_literal::INTEGER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_INTLIT, cpp::numeric_literal::INTEGER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CHARLIT, cpp::numeric_literal::CHARACTER);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FLOATLIT, cpp::numeric_literal::FLOATING);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TRUE, cpp::numeric_literal::BOOLEAN);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FALSE, cpp::numeric_literal::BOOLEAN);
	return NULL;
}

inline cpp::string_literal* parseNode(Parser& parser, cpp::string_literal* result)
{
	cpp::string_literal* p = NULL;
	for(;;)
	{
		if(!TOKEN_EQUAL(parser, boost::wave::T_STRINGLIT))
		{
			return p;
		}
		parser.increment();
		result->value += parser.get_value();
		p = result;
	}
	return result;
}

inline cpp::literal* parseNode(Parser& parser, cpp::literal* result)
{
	PARSE_SELECT(parser, cpp::numeric_literal);
	PARSE_SELECT(parser, cpp::string_literal);
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
	parser.inTemplateArgumentList = false;
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
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
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COMPL);
	PARSE_REQUIRED(parser, result->type);
	return result;
}

inline cpp::postfix_operator* parseNode(Parser& parser, cpp::postfix_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUSPLUS, cpp::postfix_operator::PLUSPLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUSMINUS, cpp::postfix_operator::MINUSMINUS);
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

inline cpp::cast_operator* parseNode(Parser& parser, cpp::cast_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DYNAMICCAST, cpp::cast_operator::DYNAMIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STATICCAST, cpp::cast_operator::STATIC);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_REINTERPRETCAST, cpp::cast_operator::REINTERPRET);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONSTCAST, cpp::cast_operator::CONST);
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
	PARSE_SELECT(parser, cpp::postfix_operator);
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
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SIZEOF);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->type);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::unary_expression_sizeof* parseNode(Parser& parser, cpp::unary_expression_sizeof* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SIZEOF);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::new_declarator_suffix* parseNode(Parser& parser, cpp::new_declarator_suffix* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::direct_new_declarator* parseNode(Parser& parser, cpp::direct_new_declarator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	PARSE_OPTIONAL(parser, result->suffix);
	return result;
}

inline cpp::new_declarator_ptr* parseNode(Parser& parser, cpp::new_declarator_ptr* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::new_declarator* parseNode(Parser& parser, cpp::new_declarator* result)
{
	PARSE_SELECT(parser, cpp::direct_new_declarator);
	PARSE_SELECT(parser, cpp::new_declarator_ptr);
	return NULL;
}

inline cpp::new_type_default* parseNode(Parser& parser, cpp::new_type_default* result)
{
	PARSE_REQUIRED(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::new_type_parenthesis* parseNode(Parser& parser, cpp::new_type_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::new_type* parseNode(Parser& parser, cpp::new_type* result)
{
	PARSE_SELECT(parser, cpp::new_type_parenthesis);
	PARSE_SELECT(parser, cpp::new_type_default);
	return NULL;
}

inline cpp::new_initializer* parseNode(Parser& parser, cpp::new_initializer* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::new_expression_placement* parseNode(Parser& parser, cpp::new_expression_placement* result)
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

inline cpp::new_expression_default* parseNode(Parser& parser, cpp::new_expression_default* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NEW);
	PARSE_REQUIRED(parser, result->type);
	PARSE_OPTIONAL(parser, result->init);
	return result;
}

inline cpp::new_expression* parseNode(Parser& parser, cpp::new_expression* result)
{
	PARSE_SELECT(parser, cpp::new_expression_placement); // TODO: ambiguity: 'new-placement' vs parenthesised 'type-id'
	PARSE_SELECT(parser, cpp::new_expression_default);
	return NULL;
}

inline cpp::delete_expression* parseNode(Parser& parser, cpp::delete_expression* result)
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
	PARSE_REQUIRED(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
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

inline cpp::conditional_expression_rhs* parseNode(Parser& parser, cpp::conditional_expression_rhs* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_QUESTION_MARK);
	PARSE_REQUIRED(parser, result->mid);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON);
	PARSE_REQUIRED(parser, result->right);
	return result;
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

inline cpp::assignment_expression_rhs* parseNode(Parser& parser, cpp::assignment_expression_rhs* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::logical_or_expression_suffix* parseNode(Parser& parser, cpp::logical_or_expression_suffix* result)
{
	PARSE_SELECT(parser, cpp::conditional_expression_rhs);
	PARSE_SELECT(parser, cpp::assignment_expression_rhs);
	return NULL;
}

inline cpp::logical_or_expression_precedent* parseNode(Parser& parser, cpp::logical_or_expression_precedent* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

inline cpp::conditional_expression_default* parseNode(Parser& parser, cpp::conditional_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

inline cpp::conditional_expression* parseNode(Parser& parser, cpp::conditional_expression* result)
{
	PARSE_PREFIX(parser, cpp::conditional_expression_default);
	return NULL;
}


inline cpp::assignment_expression* parseNode(Parser& parser, cpp::assignment_expression* result)
{
	PARSE_SELECT(parser, cpp::throw_expression);
	PARSE_SELECT(parser, cpp::logical_or_expression_precedent);
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

inline cpp::abstract_declarator_ptr* parseNode(Parser& parser, cpp::abstract_declarator_ptr* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::abstract_declarator_parenthesis* parseNode(Parser& parser, cpp::abstract_declarator_parenthesis* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
}

inline cpp::direct_abstract_declarator* parseNode(Parser& parser, cpp::direct_abstract_declarator* result)
{
	PARSE_OPTIONAL(parser, result->prefix);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::abstract_declarator* parseNode(Parser& parser, cpp::abstract_declarator* result)
{
	PARSE_SELECT(parser, cpp::abstract_declarator_ptr);
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
		if(!TOKEN_EQUAL(parser, boost::wave::T_ELLIPSIS))
		{
			PARSE_REQUIRED(parser, result->next);
		}
	}
	return result;
}

inline cpp::parameter_declaration_clause* parseNode(Parser& parser, cpp::parameter_declaration_clause* result)
{
	PARSE_OPTIONAL(parser, result->list);
	PARSE_TOKEN_OPTIONAL(parser, result->isEllipsis, boost::wave::T_ELLIPSIS);
	return result;
}

inline cpp::type_id_list* parseNode(Parser& parser, cpp::type_id_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	if(TOKEN_EQUAL(parser, boost::wave::T_COMMA))
	{
		parser.increment();
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::exception_specification* parseNode(Parser& parser, cpp::exception_specification* result)
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

inline cpp::declarator_suffix_function* parseNode(Parser& parser, cpp::declarator_suffix_function* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_OPTIONAL(parser, result->params);
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

inline cpp::comma_operator* parseNode(Parser& parser, cpp::comma_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COMMA);
	return result;
};

inline cpp::function_operator* parseNode(Parser& parser, cpp::function_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	return result;
};

inline cpp::array_operator* parseNode(Parser& parser, cpp::array_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACKET);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACKET);
	return result;
};

inline cpp::new_operator* parseNode(Parser& parser, cpp::new_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NEW);
	PARSE_OPTIONAL(parser, result->array);
	return result;
};

inline cpp::delete_operator* parseNode(Parser& parser, cpp::delete_operator* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_DELETE);
	PARSE_OPTIONAL(parser, result->array);
	return result;
};

inline cpp::overloadable_operator* parseNode(Parser& parser, cpp::overloadable_operator* result)
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
	return NULL;
};

inline cpp::operator_function_id* parseNode(Parser& parser, cpp::operator_function_id* result)
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

inline cpp::conversion_declarator* parseNode(Parser& parser, cpp::conversion_declarator* result)
{
	PARSE_REQUIRED(parser, result->op);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::conversion_function_id* parseNode(Parser& parser, cpp::conversion_function_id* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_OPERATOR);
	PARSE_REQUIRED(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::template_id_operator_function* parseNode(Parser& parser, cpp::template_id_operator_function* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	return result;
}

inline cpp::template_id* parseNode(Parser& parser, cpp::template_id* result)
{
	PARSE_SELECT(parser, cpp::simple_template_id);
	PARSE_SELECT(parser, cpp::template_id_operator_function);
	return NULL;
}

inline cpp::destructor_id* parseNode(Parser& parser, cpp::destructor_id* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COMPL);
	PARSE_REQUIRED(parser, result->name);
	return result;
}

inline cpp::unqualified_id* parseNode(Parser& parser, cpp::unqualified_id* result)
{
	PARSE_SELECT(parser, cpp::template_id); // todo: shared-prefix ambiguity: 'template-id' vs 'identifier'
	PARSE_SELECT(parser, cpp::identifier);
	PARSE_SELECT(parser, cpp::operator_function_id);
	PARSE_SELECT(parser, cpp::conversion_function_id);
	PARSE_SELECT(parser, cpp::destructor_id);
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
	PARSE_SELECT(parser, cpp::qualified_id); // TODO: shared prefix ambiguity: 'identifier' vs 'nested-name-specifier'
	PARSE_SELECT(parser, cpp::unqualified_id);
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

inline cpp::linkage_specification_compound* parseNode(Parser& parser, cpp::linkage_specification_compound* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	PARSE_OPTIONAL(parser, result->decl);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
}

inline cpp::linkage_specification_suffix* parseNode(Parser& parser, cpp::linkage_specification_suffix* result)
{
	PARSE_SELECT(parser, cpp::linkage_specification_compound);
	PARSE_SELECT(parser, cpp::declaration);
	return NULL;
}

inline cpp::linkage_specification* parseNode(Parser& parser, cpp::linkage_specification* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_EXTERN);
	PARSE_REQUIRED(parser, result->str);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::explicit_instantiation* parseNode(Parser& parser, cpp::explicit_instantiation* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isExtern, boost::wave::T_EXTERN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::explicit_specialization* parseNode(Parser& parser, cpp::explicit_specialization* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LESS);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GREATER);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::expression_list* parseNode(Parser& parser, cpp::expression_list* result)
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

inline cpp::initializer_clause* parseNode(Parser& parser, cpp::initializer_clause* result);

inline cpp::initializer_list* parseNode(Parser& parser, cpp::initializer_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::initializer_clause_list* parseNode(Parser& parser, cpp::initializer_clause_list* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTBRACE);
	if(TOKEN_EQUAL(parser, boost::wave::T_RIGHTBRACE))
	{
		parser.increment();
		result->list = NULL;
		return result;
	}
	PARSE_REQUIRED(parser, result->list);
	bool trailingComma;
	PARSE_TOKEN_OPTIONAL(parser, trailingComma, boost::wave::T_COMMA);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTBRACE);
	return result;
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
		parser.increment();
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
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_ASM);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_LEFTPAREN);
	PARSE_REQUIRED(parser, result->str);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RIGHTPAREN);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::namespace_alias_definition* parseNode(Parser& parser, cpp::namespace_alias_definition* result)
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

inline cpp::using_declaration_global* parseNode(Parser& parser, cpp::using_declaration_global* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_USING);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::using_declaration_nested* parseNode(Parser& parser, cpp::using_declaration_nested* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_USING);
	PARSE_TOKEN_OPTIONAL(parser, result->isTypename, boost::wave::T_TYPENAME);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::using_declaration* parseNode(Parser& parser, cpp::using_declaration* result)
{
	PARSE_SELECT(parser, cpp::using_declaration_global);
	PARSE_SELECT(parser, cpp::using_declaration_nested);
	return NULL;
}

inline cpp::using_directive* parseNode(Parser& parser, cpp::using_directive* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_USING);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NAMESPACE);
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
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

inline cpp::jump_statement_key* parseNode(Parser& parser, cpp::jump_statement_key* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_BREAK, cpp::jump_statement_key::BREAK);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_CONTINUE, cpp::jump_statement_key::CONTINUE);
	return NULL;
}

inline cpp::jump_statement_simple* parseNode(Parser& parser, cpp::jump_statement_simple* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::jump_statement_return* parseNode(Parser& parser, cpp::jump_statement_return* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_RETURN);
	PARSE_OPTIONAL(parser, result->expr);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::jump_statement_goto* parseNode(Parser& parser, cpp::jump_statement_goto* result)
{
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_GOTO);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_SEMICOLON);
	return result;
}

inline cpp::jump_statement* parseNode(Parser& parser, cpp::jump_statement* result)
{
	PARSE_SELECT(parser, cpp::jump_statement_simple);
	PARSE_SELECT(parser, cpp::jump_statement_return);
	PARSE_SELECT(parser, cpp::jump_statement_goto);
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


cpp::declaration_seq* parseFile(LexContext& context)
{
	Scanner scanner(context);
	Parser parser(scanner);

	cpp::declaration_seq* result = NULL;
	PARSE_OPTIONAL(parser, result);
#ifdef _DEBUG
	if(!scanner.finished())
	{
		printError(parser);
	}
#endif
	return result;
}

cpp::statement_seq* parseFunction(LexContext& context)
{
	Scanner scanner(context);
	Parser parser(scanner);

	cpp::statement_seq* result = NULL;
	PARSE_OPTIONAL(parser, result);
#ifdef _DEBUG
	if(!scanner.finished())
	{
		printError(parser);
	}
#endif
	return result;
}

