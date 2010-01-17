
#ifndef INCLUDED_CPPPARSE_CPPTREE_H
#define INCLUDED_CPPPARSE_CPPTREE_H

#include <string>

namespace cpp
{
	template<typename T>
	struct choice
	{
		virtual ~choice()
		{
		}
	};

	struct terminal_choice
	{
	};

	struct template_argument : public choice<template_argument>
	{
	};

	struct template_parameter : public choice<template_parameter>
	{
	};

	struct exception_declarator : public choice<exception_declarator>
	{
	};

	struct declarator : public choice<declarator>, public exception_declarator
	{
	};

	struct direct_declarator_prefix : public choice<direct_declarator_prefix>, public declarator
	{
	};

	struct declarator_id : public choice<declarator_id>, public direct_declarator_prefix
	{
	};

	struct condition : public choice<condition>
	{
	};

	struct expression : public choice<expression>, public condition
	{
	};

	struct initializer : public choice<initializer>
	{
	};

	struct initializer_clause : public choice<initializer_clause>, public initializer
	{
	};

	struct assignment_expression : public expression, public template_argument, public initializer_clause
	{
	};

	struct constant_expression
	{
	};

	struct conditional_expression : public choice<conditional_expression>, public assignment_expression, public constant_expression
	{
	};

	struct logical_or_expression : public choice<logical_or_expression>, public conditional_expression
	{
	};

	struct logical_and_expression : public choice<logical_and_expression>, public logical_or_expression
	{
	};

	struct inclusive_or_expression : public choice<inclusive_or_expression>, public logical_and_expression
	{
	};

	struct exclusive_or_expression : public choice<exclusive_or_expression>, public inclusive_or_expression
	{
	};

	struct and_expression : public choice<and_expression>, public exclusive_or_expression
	{
	};

	struct equality_expression : public choice<equality_expression>, public and_expression
	{
	};

	struct relational_expression : public choice<relational_expression>, public equality_expression
	{
	};

	struct shift_expression : public choice<shift_expression>, public relational_expression
	{
	};

	struct additive_expression : public choice<additive_expression>, public shift_expression
	{
	};

	struct multiplicative_expression : public choice<multiplicative_expression>, public additive_expression
	{
	};

	struct pm_expression : public choice<pm_expression>, public multiplicative_expression
	{
	};

	struct cast_expression : public choice<cast_expression>, public pm_expression
	{
	};

	struct unary_expression : public choice<unary_expression>, public cast_expression
	{
	};

	struct postfix_expression : public choice<postfix_expression>, public unary_expression
	{
	};

	struct postfix_expression_prefix : public choice<postfix_expression_prefix>, public postfix_expression
	{
	};

	struct primary_expression : public choice<primary_expression>, public postfix_expression_prefix
	{
	};

	struct id_expression : public choice<id_expression>, public declarator_id, public primary_expression
	{
	};

	struct unqualified_id : public choice<unqualified_id>, public id_expression
	{
	};

	struct qualified_id : public choice<qualified_id>, public id_expression
	{
	};

	struct template_id : public choice<template_id>, public unqualified_id
	{
	};

	struct mem_initializer_id : public choice<mem_initializer_id>
	{
	};

	struct type_name : public choice<type_name>
	{
	};

	struct class_name : public choice<class_name>, public type_name
	{
	};

	struct identifier : public unqualified_id, public mem_initializer_id, public class_name
	{
		const char* value;
	};

	struct nested_name_specifier_prefix
	{
		type_name* id;
	};

	struct nested_name_specifier_suffix
	{
		bool isTemplate;
		class_name* id;
		nested_name_specifier_suffix* next;
	};

	struct nested_name_specifier
	{
		nested_name_specifier_prefix* prefix;
		nested_name_specifier_suffix* suffix;
	};

	struct type_specifier : public choice<type_specifier>
	{
	};

	struct type_specifier_noncv : public choice<type_specifier_noncv>, public type_specifier
	{
	};

	struct decl_specifier_suffix : public choice<decl_specifier_suffix>
	{
	};

	struct decl_specifier_nontype : public choice<decl_specifier_nontype>, public decl_specifier_suffix
	{
	};

	struct decl_specifier_prefix_seq
	{
		decl_specifier_nontype* item;
		decl_specifier_prefix_seq* next;
	};

	struct decl_specifier_suffix_seq
	{
		decl_specifier_suffix* item;
		decl_specifier_suffix_seq* next;
	};

	struct decl_specifier_seq
	{
		decl_specifier_prefix_seq* prefix;
		type_specifier_noncv* type;
		decl_specifier_suffix_seq* suffix;
	};

	struct simple_type_specifier : public choice<simple_type_specifier>, public type_specifier_noncv
	{
	};

	struct template_argument_list
	{
		template_argument* item;
		template_argument_list* next;
	};

	struct overloadable_operator : public choice<overloadable_operator>
	{
	};

	struct array_operator : public choice<array_operator>, public overloadable_operator
	{
	};

	struct function_operator : public choice<function_operator>, public overloadable_operator
	{
	};

	struct comma_operator : public choice<comma_operator>, public overloadable_operator
	{
	};

	struct new_operator : public overloadable_operator
	{
		array_operator* array;
	};

	struct delete_operator : public overloadable_operator
	{
		array_operator* array;
	};

	struct operator_function_id : public unqualified_id
	{
		overloadable_operator* op;
		template_argument_list* args; // NULL if not template
	};

	struct template_id_operator_function : public template_id
	{
		operator_function_id* id;
		template_argument_list* args;
	};

	struct qualified_id_default : public qualified_id
	{
		bool isGlobal;
		bool isTemplate;
		nested_name_specifier* context;
		unqualified_id* id;
	};

	struct qualified_id_global : public qualified_id
	{
		identifier* id;
	};

	struct qualified_id_global_template : public qualified_id
	{
		template_id* id;
	};

	struct qualified_id_global_op_func : public qualified_id
	{
		operator_function_id* id;
	};

	struct elaborated_type_specifier_key : public choice<elaborated_type_specifier_key>
	{
	};

	struct class_key : public terminal_choice, public elaborated_type_specifier_key
	{
		enum { CLASS, STRUCT, UNION } value;
	};

	struct enum_key : public elaborated_type_specifier_key
	{
		// always 'enum'
	};

	struct access_specifier : public terminal_choice
	{
		enum { PRIVATE, PROTECTED, PUBLIC } value;
	};

	struct base_specifier_prefix : public choice<base_specifier_prefix>
	{
	};

	struct base_specifier_access_virtual : public base_specifier_prefix
	{
		access_specifier* access; // required
		bool isVirtual;
	};

	struct base_specifier_virtual_access : public base_specifier_prefix
	{
		// always virtual
		access_specifier* access; // optional
	};

	struct base_specifier
	{
		base_specifier_prefix* prefix;
		bool isGlobal;
		nested_name_specifier* context;
		class_name* id;
	};


	struct base_specifier_list
	{
		base_specifier* item;
		base_specifier_list* next;
	};

	struct base_clause
	{
		base_specifier_list* list;
	};

	struct class_head : public choice<class_head>
	{
	};

	struct class_head_default : public class_head
	{
		class_key* key;
		identifier* id;
		base_clause* base;
	};

	struct class_head_nested : public class_head
	{
		class_key* key;
		nested_name_specifier* context;
		class_name* id;
		base_clause* base;
	};

	struct simple_template_id : public template_id, public class_name
	{
		identifier* id;
		template_argument_list* args;
	};

	struct class_head_template : public class_head
	{
		nested_name_specifier* context;
		simple_template_id id;
		base_clause* base;
	};

	struct type_specifier_suffix : public choice<type_specifier_suffix>
	{
	};

	struct cv_qualifier : public terminal_choice, public type_specifier, public decl_specifier_nontype, public type_specifier_suffix
	{
		enum { CONST, VOLATILE } value;
	};

	struct cv_qualifier_seq
	{
		cv_qualifier* item;
		cv_qualifier_seq* next;
	};

	struct ptr_operator_key : public terminal_choice
	{
		enum { PTR, REF } value;
	};

	struct ptr_operator
	{
		bool isGlobal;
		nested_name_specifier* context;
		ptr_operator_key* key;
		cv_qualifier_seq* qual;
	};

	struct type_specifier_prefix_seq
	{
		cv_qualifier* item;
		type_specifier_prefix_seq* next;
	};

	struct type_specifier_suffix_seq
	{
		type_specifier_suffix* item;
		type_specifier_suffix_seq* next;
	};

	struct type_specifier_seq
	{
		type_specifier_prefix_seq* prefix;
		type_specifier_noncv* type;
		type_specifier_suffix_seq* suffix;
	};

	struct abstract_declarator : public choice<abstract_declarator>, public exception_declarator
	{
	};

	struct abstract_declarator_ptr : public abstract_declarator
	{
		ptr_operator* op;
		abstract_declarator* decl;
	};

	struct abstract_declarator_parenthesis : public abstract_declarator
	{
		abstract_declarator* decl;
	};

	struct declarator_suffix_seq;

	struct direct_abstract_declarator : public abstract_declarator
	{
		abstract_declarator_parenthesis* prefix;
		declarator_suffix_seq* suffix;
	};

	struct type_id : public template_argument
	{
		type_specifier_seq* spec;
		abstract_declarator* decl;
	};

	struct throw_expression : public assignment_expression
	{
		assignment_expression* expr;
	};

	struct expression_comma : public expression
	{
		assignment_expression* left;
		expression_comma* right;
	};

	struct member_initializer : public choice<member_initializer>
	{
	};

	struct constant_initializer : public member_initializer
	{
		constant_expression* expr;
	};

	struct literal : public choice<literal>, public primary_expression
	{
	};

	struct numeric_literal : public terminal_choice, public literal
	{
		enum { INTEGER, CHARACTER, FLOATING, BOOLEAN } value;
	};

	struct string_literal : public literal
	{
		const char* value;
		string_literal* next;
	};

	struct primary_expression_builtin : public primary_expression
	{
		// always 'this'
	};

	struct primary_expression_parenthesis : public primary_expression
	{
		expression* expr;
	};

	struct initializer_list
	{
		initializer_clause* item;
		initializer_list* next;
	};

	struct initializer_clause_list : public initializer_clause
	{
		initializer_list* list;
	};

	struct expression_list : public initializer
	{
		assignment_expression* item;
		expression_list* next;
	};

	struct initializer_default : public initializer
	{
		initializer_clause* clause;
	};

	struct initializer_parenthesis : public initializer
	{
		expression_list* list;
	};

	struct postfix_expression_suffix : public choice<postfix_expression_suffix>
	{
	};

	struct postfix_expression_suffix_seq
	{
		postfix_expression_suffix* item;
		postfix_expression_suffix_seq* next;
	};

	struct postfix_expression_default : public postfix_expression
	{
		postfix_expression_prefix* expr;
		postfix_expression_suffix_seq* suffix;
	};

	struct postfix_expression_index : public postfix_expression_suffix
	{
		expression* index;
	};

	struct postfix_expression_call : public postfix_expression_suffix
	{
		expression_list* args;
	};

	struct member_operator : public terminal_choice, public overloadable_operator
	{
		enum { DOT, ARROW } value;
	};

	struct postfix_expression_member : public postfix_expression_suffix
	{
		member_operator* op;
		bool isTemplate;
		id_expression* id;
	};

	struct psuedo_destructor_name
	{
	};

	struct psuedo_destructor_name_default : public psuedo_destructor_name
	{
		type_name* type;
	};

	struct psuedo_destructor_name_template : public psuedo_destructor_name
	{
		simple_template_id id;
		type_name* type;
	};

	struct postfix_expression_destructor : public postfix_expression_suffix
	{
		bool isGlobal;
		nested_name_specifier* context;
		type_name* type;
		//psuedo_destructor_name* destructor;
	};

	struct postfix_operator : public terminal_choice, public postfix_expression_suffix, public overloadable_operator
	{
		enum { PLUSPLUS, MINUSMINUS } value;
	};

	struct postfix_expression_construct : public postfix_expression_prefix
	{
		simple_type_specifier* type;
		expression_list* args;
	};

	struct cast_operator : public terminal_choice
	{
		enum { DYNAMIC, STATIC, REINTERPRET, CONST } value;
	};

	struct postfix_expression_cast : public postfix_expression_prefix
	{
		cast_operator* op;
		type_id* type;
		expression* expr;
	};

	struct postfix_expression_typeid : public postfix_expression_prefix
	{
		expression* expr;
	};

	struct postfix_expression_typeidtype : public postfix_expression_prefix
	{
		type_id* type;
	};

	struct new_type : public choice<new_type>
	{
	};

	struct new_declarator : public choice<new_declarator>
	{
	};

	struct new_declarator_suffix
	{
		constant_expression* expr;
		new_declarator_suffix* next;
	};

	struct direct_new_declarator : public new_declarator
	{
		expression* expr;
		new_declarator_suffix* suffix;
	};

	struct new_declarator_ptr : public new_declarator
	{
		ptr_operator* op;
		new_declarator* decl;
	};

	struct new_type_default : public new_type
	{
		type_specifier_seq* spec;
		new_declarator* decl;
	};

	struct new_type_parenthesis : public new_type
	{
		type_id* id;
	};

	struct new_initializer
	{
		expression_list* expr;
	};

	struct new_expression : public choice<new_expression>, public unary_expression
	{
	};

	struct new_expression_placement : public new_expression
	{
		bool isGlobal;
		expression_list* place;
		new_type* type;
		new_initializer* init;
	};

	struct new_expression_default : public new_expression
	{
		bool isGlobal;
		new_type* type;
		new_initializer* init;
	};

	struct delete_expression : public unary_expression
	{
		bool isGlobal;
		bool isArray;
		cast_expression* expr;
	};

	struct unary_operator : public terminal_choice, public unary_expression, public overloadable_operator
	{
		enum { PLUSPLUS, MINUSMINUS, STAR, AND, PLUS, MINUS, NOT, COMPL } value;
	};

	struct unary_expression_op : public unary_expression
	{
		unary_operator* op;
		cast_expression* expr;
	};

	struct unary_expression_sizeof : public unary_expression
	{
		unary_expression* expr;
	};

	struct unary_expression_sizeoftype : public unary_expression
	{
		type_id* type;
	};

	struct cast_expression_default : public cast_expression
	{
		type_id* id;
		cast_expression* expr;
	};

	struct pm_operator : public terminal_choice, public overloadable_operator
	{
		enum { DOTSTAR, ARROWSTAR } value;
	};

	struct pm_expression_default : public pm_expression
	{
		cast_expression* left;
		pm_operator* op;
		pm_expression* right;
	};

	struct multiplicative_operator : public terminal_choice, public overloadable_operator
	{
		enum { STAR, DIVIDE, PERCENT } value;
	};

	struct multiplicative_expression_default : public multiplicative_expression
	{
		pm_expression* left;
		multiplicative_operator* op;
		multiplicative_expression* right;
	};

	struct additive_operator : public terminal_choice, public overloadable_operator
	{
		enum { PLUS, MINUS } value;
	};

	struct additive_expression_default : public additive_expression
	{
		multiplicative_expression* left;
		additive_operator* op;
		additive_expression* right;
	};

	struct shift_operator : public terminal_choice, public overloadable_operator
	{
		enum { SHIFTLEFT, SHIFTRIGHT } value;
	};

	struct shift_expression_default : public shift_expression
	{
		additive_expression* left;
		shift_operator* op;
		shift_expression* right;
	};

	struct relational_operator : public terminal_choice, public overloadable_operator
	{
		enum { LESS, GREATER, LESSEQUAL, GREATEREQUAL } value;
	};

	struct relational_expression_default : public relational_expression
	{
		shift_expression* left;
		relational_operator* op;
		relational_expression* right;
	};

	struct equality_operator : public terminal_choice, public overloadable_operator
	{
		enum { EQUAL, NOTEQUAL } value;
	};

	struct equality_expression_default : public equality_expression
	{
		relational_expression* left;
		equality_operator* op;
		equality_expression* right;
	};

	struct and_expression_default : public and_expression
	{
		equality_expression* left;
		and_expression* right;
	};

	struct exclusive_or_expression_default : public exclusive_or_expression
	{
		and_expression* left;
		exclusive_or_expression* right;
	};

	struct inclusive_or_expression_default : public inclusive_or_expression
	{
		exclusive_or_expression* left;
		inclusive_or_expression* right;
	};

	struct logical_and_expression_default : public logical_and_expression
	{
		inclusive_or_expression* left;
		logical_and_expression* right;
	};

	struct logical_or_expression_default : public logical_or_expression
	{
		logical_and_expression* left;
		logical_or_expression* right;
	};

	struct logical_or_expression_suffix : public choice<logical_or_expression_suffix>
	{
	};

	struct conditional_expression_rhs : public logical_or_expression_suffix
	{
		expression* mid;
		assignment_expression* right;
	};

	struct conditional_expression_default : public conditional_expression
	{
		logical_or_expression* left;
		conditional_expression_rhs* right;
	};

	struct logical_or_expression_precedent : public conditional_expression
	{
		logical_or_expression* left;
		logical_or_expression_suffix* right;
	};

	struct assignment_operator : public terminal_choice, public overloadable_operator
	{
		enum { ASSIGN, STAR, DIVIDE, PERCENT, PLUS, MINUS, SHIFTRIGHT, SHIFTLEFT, AND, XOR, OR } value;
	};

	struct assignment_expression_rhs : public logical_or_expression_suffix
	{
		assignment_operator* op;
		assignment_expression* right;
	};




	struct conversion_declarator
	{
		ptr_operator* op;
		conversion_declarator* decl;
	};

	struct conversion_function_id : public unqualified_id
	{
		type_specifier_seq* spec;
		conversion_declarator* decl;
	};

	struct destructor_id : public unqualified_id 
	{
		class_name* name;
	};

	struct declarator_id_nested : public declarator_id
	{
		bool isGlobal;
		nested_name_specifier* context;
		class_name* name;
	};

	struct parameter_declaration_clause;

	struct type_id_list
	{
		type_id* item;
		type_id_list* next;
	};

	struct exception_specification
	{
		type_id_list* types;
	};

	struct declarator_suffix : public choice<declarator_suffix>
	{
	};

	struct declarator_suffix_function : public declarator_suffix
	{
		parameter_declaration_clause* params;
		cv_qualifier_seq* qual;
		exception_specification* except;
	};

	struct declarator_suffix_array : public declarator_suffix
	{
		constant_expression* size;
	};

	struct declarator_suffix_seq : public declarator_suffix
	{
		declarator_suffix* item;
		declarator_suffix_seq* next;
	};

	struct direct_declarator : public declarator
	{
		direct_declarator_prefix* prefix;
		declarator_suffix_seq* suffix;
	};

	struct direct_declarator_parenthesis : public direct_declarator_prefix
	{
		declarator* decl;
	};

	struct declarator_ptr : public declarator
	{
		ptr_operator* op;
		declarator* decl;
	};

	struct statement : public choice<statement>
	{
	};

	struct statement_seq
	{
		statement* item;
		statement_seq* next;
	};

	struct function_body : public choice<function_body>
	{
	};

	struct compound_statement : public statement, public function_body
	{
		statement_seq* body;
	};

	struct exception_declaration : public choice<exception_declaration>
	{
	};

	struct exception_declaration_default : public exception_declaration
	{
		type_specifier_seq* type;
		exception_declarator* decl;
	};

	struct exception_declaration_all : public exception_declaration
	{
		// always '...'
	};

	struct handler_seq
	{
		exception_declaration* decl;
		compound_statement* body;
		handler_seq* next;
	};

	struct linkage_specification_suffix : public choice<linkage_specification_suffix>
	{
	};

	struct declaration : public choice<declaration>, public linkage_specification_suffix
	{
	};

	struct mem_initializer
	{
		bool isGlobal;
		nested_name_specifier* context;
		class_name* id;
		expression_list* args;
	};

	struct mem_initializer_list
	{
		mem_initializer* item;
		mem_initializer_list* next;
	};

	struct ctor_initializer
	{
		mem_initializer_list* list;
	};

	struct general_declaration_suffix : public choice<general_declaration_suffix>
	{
	};

	struct general_declaration : public declaration
	{
		decl_specifier_seq* spec;
		declarator* decl;
		general_declaration_suffix* suffix;
	};

	struct member_declaration_suffix : public choice<member_declaration_suffix>
	{
	};

	struct function_definition_suffix : public general_declaration_suffix, public member_declaration_suffix
	{
		function_body* body;
		handler_seq* handlers;
	};

	struct function_definition : public declaration
	{
		decl_specifier_seq* spec;
		declarator* decl;
		function_definition_suffix* suffix;
	};


	struct member_declarator : public choice<member_declarator>
	{
	};

	struct pure_specifier : public member_initializer
	{
		// always '= 0'
	};

	struct member_declarator_default : public member_declarator
	{
		declarator* decl;
		member_initializer* init;
	};

	struct member_declarator_bitfield : public member_declarator
	{
		identifier* id;
		constant_expression* width;
	};

	struct member_declarator_list
	{
		member_declarator* item;
		member_declarator_list* next;
	};

	struct member_declaration : public choice<member_declaration>
	{
	};

	struct member_declaration_general : public choice<member_declaration_general>
	{
	};

	struct member_declaration_default : public member_declaration
	{
		decl_specifier_seq* spec;
		member_declaration_general* decl;
	};

	struct member_declaration_general_bitfield : public member_declaration_general
	{
		member_declarator_bitfield* item;
		member_declarator_list* next;
	};

	struct member_declaration_general_default : public member_declaration_general
	{
		declarator* decl;
		member_declaration_suffix* suffix;
	};

	struct member_declaration_suffix_default : public member_declaration_suffix
	{
		member_initializer* init;
		member_declarator_list* next;
	};

	struct member_declaration_nested : public member_declaration
	{
		bool isGlobal;
		bool isTemplate;
		nested_name_specifier* context;
		unqualified_id* id;
	};

	struct function_specifier : public terminal_choice, public decl_specifier_nontype
	{
		enum { INLINE, VIRTUAL, EXPLICIT } value;
	};

	struct ctor_specifier_seq
	{
		function_specifier* item;
		ctor_specifier_seq* next;
	};

	struct constructor_definition : public declaration
	{
		ctor_specifier_seq* spec;
		declarator* decl;
		ctor_initializer* init;
		function_body* body;
		handler_seq* handlers;
	};

	struct member_declaration_inline : public member_declaration
	{
		constructor_definition* func;
	};

	struct member_declaration_ctor : public member_declaration
	{
		ctor_specifier_seq* spec;
		declarator* decl;
	};

	struct member_specification : public choice<member_specification>
	{
	};

	struct member_specification_list : public member_specification
	{
		member_declaration* item;
		member_specification* next;
	};

	struct member_specification_access : public member_specification
	{
		access_specifier* access;
		member_specification* next;
	};

	struct class_specifier : public type_specifier_noncv
	{
		class_head* head;
		member_specification* members;
	};

	struct enumerator_definition
	{
		identifier* id;
		constant_expression* init;
	};

	struct enumerator_list
	{
		enumerator_definition* item;
		enumerator_list* next;
	};

	struct enum_specifier : public type_specifier_noncv
	{
		identifier* id; // may be empty
		enumerator_list* values;
	};

	struct elaborated_type_specifier : public choice<elaborated_type_specifier>, public type_specifier_noncv
	{
	};

	struct elaborated_type_specifier_default : public elaborated_type_specifier
	{
		elaborated_type_specifier_key* key;
		bool isGlobal;
		nested_name_specifier* context;
		identifier* id;
	};

	struct elaborated_type_specifier_template : public elaborated_type_specifier
	{
		class_key* key;
		bool isGlobal;
		nested_name_specifier* context;
		bool isTemplate;
		simple_template_id* id;
	};

	struct typename_specifier : public type_specifier_noncv
	{
		bool isGlobal;
		nested_name_specifier* context;
		bool isTemplate;
		type_name* id; // NOTE: only 'identifier' is allowed if 'isTemplate' is true
	};

	struct parameter_declaration : public choice<parameter_declaration>, public template_parameter
	{
	};

	struct type_parameter : public choice<type_parameter>, public template_parameter
	{
	};

	struct type_parameter_key : public terminal_choice
	{
		enum { CLASS, TYPENAME } value;
	};

	struct type_parameter_default : public type_parameter
	{
		type_parameter_key* key;
		identifier* id;
		type_id* init;
	};

	struct template_parameter_list
	{
		template_parameter* item;
		template_parameter_list* next;
	};

	struct type_parameter_template : public type_parameter
	{
		template_parameter_list* params;
		identifier* id;
		id_expression* init;
	};

	struct parameter_declaration_default : public parameter_declaration
	{
		decl_specifier_seq* spec;
		declarator* decl;
		assignment_expression* init;
	};

	struct parameter_declaration_abstract : public parameter_declaration
	{
		decl_specifier_seq* spec;
		abstract_declarator* decl;
		assignment_expression* init;
	};

	struct parameter_declaration_list
	{
		parameter_declaration* item;
		parameter_declaration_list* next;
	};

	struct parameter_declaration_clause
	{
		bool isEllipsis;
		parameter_declaration_list* list;
	};

	struct direct_abstract_declarator_function : public direct_abstract_declarator
	{
		direct_abstract_declarator* decl;
		parameter_declaration_clause* params;
		cv_qualifier_seq* qual;
		exception_specification* except;
	};

	struct direct_abstract_declarator_array : public direct_abstract_declarator
	{
		direct_abstract_declarator* decl;
		constant_expression* size;
	};

	struct direct_abstract_declarator_parenthesis : public direct_abstract_declarator
	{
		abstract_declarator* decl;
	};

	struct template_id_operator : public template_id
	{
		operator_function_id* id;
		template_argument_list* args;
	};

	struct decl_specifier_default : public terminal_choice, public decl_specifier_nontype
	{
		enum { FRIEND, TYPEDEF } value;
	};

	struct storage_class_specifier : public terminal_choice, public decl_specifier_nontype
	{
		enum { REGISTER, STATIC, EXTERN, MUTABLE } value;
	};

	struct nested_name_specifier_nested : public nested_name_specifier
	{
		nested_name_specifier* context;
		identifier id;
	};

	struct nested_name_specifier_template : public nested_name_specifier
	{
		nested_name_specifier* context;
		simple_template_id id;
	};

	struct simple_type_specifier_name : public simple_type_specifier
	{
		bool isGlobal;
		nested_name_specifier* context;
		type_name* id;
	};

	struct simple_type_specifier_template : public simple_type_specifier
	{
		bool isGlobal;
		nested_name_specifier* context;
		simple_template_id* id;
	};

	struct simple_type_specifier_builtin : public terminal_choice, public simple_type_specifier, public decl_specifier_suffix, public type_specifier_suffix
	{
		enum { CHAR, WCHAR_T, BOOL, SHORT, INT, LONG, SIGNED, UNSIGNED, FLOAT, DOUBLE, VOID, AUTO } value;
	};

	struct declaration_statement : public choice<declaration_statement>, public statement
	{
	};

	struct block_declaration : public choice<block_declaration>, public declaration_statement, public declaration
	{
	};

	struct asm_definition : public block_declaration
	{
		string_literal* str;
	};

	struct msext_asm_statement : public statement
	{
		msext_asm_statement* inner;
	};


	struct namespace_alias_definition : public block_declaration
	{
		identifier* alias;
		bool isGlobal;
		nested_name_specifier* context;
		identifier* id;
	};

	struct using_declaration : public choice<using_declaration>, public block_declaration, public member_declaration
	{
	};

	struct using_declaration_global : public using_declaration
	{
		unqualified_id* id;
	};

	struct using_declaration_nested : public using_declaration
	{
		bool isTypename;
		bool isGlobal;
		nested_name_specifier* context;
		unqualified_id* id;
	};

	struct using_directive : public block_declaration
	{
		bool isGlobal;
		nested_name_specifier* context;
		identifier* id;
	};

	struct for_init_statement : public choice<for_init_statement>
	{
	};

	struct init_declarator
	{
		declarator* decl;
		initializer* init;
	};

	struct init_declarator_list
	{
		init_declarator* item;
		init_declarator_list* next;
	};

	struct simple_declaration_suffix : public general_declaration_suffix
	{
		initializer* init;
		init_declarator_list* next;
	};

	struct simple_declaration : public block_declaration, public for_init_statement
	{
		decl_specifier_seq* spec;
		declarator* decl;
		simple_declaration_suffix* suffix;
	};

	struct mem_initializer_id_base : public mem_initializer_id
	{
		bool isGlobal;
		nested_name_specifier* context;
		class_name* name;
	};

	struct labeled_statement : public choice<labeled_statement>, public statement
	{
	};

	struct labeled_statement_id : public labeled_statement
	{
		identifier* label;
		statement* body;
	};

	struct labeled_statement_case : public labeled_statement
	{
		constant_expression* label;
		statement* body;
	};

	struct labeled_statement_default : public labeled_statement
	{
		statement* body;
	};

	struct expression_statement : public statement, public for_init_statement
	{
		expression* expr;
	};

	struct selection_statement : public choice<selection_statement>, public statement
	{
	};

	struct condition_init : public condition
	{
		type_specifier_seq* type;
		declarator* decl;
		assignment_expression* init;
	};

	struct selection_statement_if : public selection_statement
	{
		condition* cond;
		statement* body;
		statement* fail;
	};

	struct selection_statement_switch : public selection_statement
	{
		condition* cond;
		statement* body;
	};

	struct iteration_statement : public choice<iteration_statement>, public statement
	{
	};

	struct iteration_statement_while : public iteration_statement
	{
		condition* cond;
		statement* body;
	};

	struct iteration_statement_dowhile : public iteration_statement
	{
		expression* cond;
		statement* body;
	};

	struct iteration_statement_for : public iteration_statement
	{
		for_init_statement* init;
		condition* cond;
		expression* incr;
		statement* body;
	};

	struct jump_statement : public choice<jump_statement>, public statement
	{
	};

	struct jump_statement_key : public terminal_choice
	{
		enum { BREAK, CONTINUE } value;
	};

	struct jump_statement_simple : public jump_statement
	{
		jump_statement_key* key;
	};

	struct jump_statement_return : public jump_statement
	{
		expression* expr;
	};

	struct jump_statement_goto : public jump_statement
	{
		identifier* id;
	};

	struct try_block : public statement
	{
		compound_statement* body;
		handler_seq* handlers;
	};

	struct declaration_seq
	{
		declaration* item;
		declaration_seq* next;
	};

	struct template_declaration : public declaration, public member_declaration
	{
		bool isExport;
		template_parameter_list* params;
		declaration* decl;
	};

	struct explicit_instantiation : public declaration
	{
		bool isExtern;
		declaration* decl;
	};

	struct explicit_specialization : public declaration
	{
		declaration* decl;
	};

	struct linkage_specification_compound : public linkage_specification_suffix
	{
		declaration_seq* decl;
	};

	struct linkage_specification : public declaration
	{
		string_literal* str;
		linkage_specification_suffix* suffix;
	};

	typedef declaration_seq namespace_body;

	struct namespace_definition : public declaration
	{
		identifier* id;
		namespace_body* body;
	};
}

#endif


