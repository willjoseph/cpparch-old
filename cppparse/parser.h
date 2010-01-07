
#ifndef INCLUDED_CPPPARSE_PARSER_H
#define INCLUDED_CPPPARSE_PARSER_H

#include "lexer.h"


namespace cpp
{
	struct template_argument
	{
	};

	struct template_parameter
	{
	};

	struct declarator
	{
	};

	struct direct_declarator : public declarator
	{
	};

	struct declarator_id : public direct_declarator
	{
	};

	struct id_expression : public template_argument, public declarator_id
	{
	};

	struct unqualified_id : public id_expression
	{
	};

	struct qualified_id : public id_expression
	{
	};

	struct mem_initializer_id
	{
	};

	struct identifier : public unqualified_id, public mem_initializer_id
	{
		std::string value;
	};

	struct template_name : public identifier
	{
	};

	struct decl_specifier
	{
	};

	struct decl_specifier_seq
	{
		decl_specifier* item;
		decl_specifier_seq* next;
	};

	struct type_specifier : public decl_specifier
	{
	};

	struct simple_type_specifier : public type_specifier
	{
	};

	struct nested_name_specifier
	{
	};

	struct type_name : public nested_name_specifier
	{
	};

	struct class_name : public type_name
	{
	};

	struct qualified_id_default : public qualified_id
	{
		bool isGlobal;
		bool isTemplate;
		nested_name_specifier* context;
		unqualified_id* id;
	};

	struct class_key
	{
		enum { CLASS, STRUCT, UNION } value;
	};

	struct access_specifier
	{
		enum { PRIVATE, PROTECTED, PUBLIC } value;
	};

	struct base_specifier
	{
		bool isGlobal;
		access_specifier access;
		nested_name_specifier* context;
		class_name* id;
	};


	struct base_specifier_list
	{
		base_specifier* item;
		base_specifier_list* next;
	};

	struct base_clause : public base_specifier_list
	{
	};

	struct class_head
	{
		class_key key;
	};

	struct class_head_default : public class_head
	{
		identifier id; // may be empty
		base_clause* base;
	};

	struct class_head_nested : public class_head
	{
		nested_name_specifier* context;
		identifier id;
		base_clause* base;
	};

	struct template_argument_list
	{
		template_argument* item;
		template_argument_list* next;
	};

	struct template_id : public unqualified_id
	{
	};

	struct simple_template_id : public template_id
	{
		template_name id;
		template_argument_list* args;
	};

	struct class_head_template : public class_head
	{
		nested_name_specifier* context;
		simple_template_id id;
		base_clause* base;
	};

	struct cv_qualifier_seq
	{
		enum { CONST, VOLATILE } value;
		cv_qualifier_seq* next;
	};

	struct ptr_operator
	{
		bool isGlobal;
		bool isRef;
		nested_name_specifier* context;
		cv_qualifier_seq* qual;
	};

	struct type_specifier_seq
	{
		type_specifier* item;
		type_specifier_seq* next;
	};

	struct abstract_declarator
	{
	};

	struct abstract_declarator_default : public abstract_declarator
	{
		ptr_operator* op;
		abstract_declarator* decl;
	};

	struct direct_abstract_declarator : public abstract_declarator
	{
	};

	struct type_id : public template_argument
	{
		type_specifier_seq* spec;
		abstract_declarator* decl;
	};


	struct condition
	{
	};

	struct expression : public condition
	{
	};

	struct assignment_expression : public expression, public template_argument
	{
	};

	struct throw_expression : public assignment_expression
	{
		assignment_expression* expr;
	};

	struct expression_comma : public expression
	{
		expression* left;
		assignment_expression* right;
	};

	struct conditional_expression : public assignment_expression
	{
	};

	struct logical_or_expression : public conditional_expression
	{
	};

	struct logical_and_expression : public logical_or_expression
	{
	};

	struct inclusive_or_expression : public logical_and_expression
	{
	};

	struct exclusive_or_expression : public inclusive_or_expression
	{
	};

	struct and_expression : public exclusive_or_expression
	{
	};

	struct equality_expression : public and_expression
	{
	};

	struct relational_expression : public equality_expression
	{
	};

	struct shift_expression : public relational_expression
	{
	};

	struct additive_expression : public shift_expression
	{
	};

	class multiplicative_expression : public additive_expression
	{
	};

	struct pm_expression : public multiplicative_expression
	{
	};

	struct cast_expression : public pm_expression
	{
	};

	struct unary_expression : public cast_expression
	{
	};

	struct postfix_expression : public unary_expression
	{
	};

	struct primary_expression : public postfix_expression
	{
	};

	struct expression_list
	{
		assignment_expression* item;
		expression_list* next;
	};

	struct postfix_expression_index : public postfix_expression
	{
		postfix_expression* expr;
		expression* index;
	};

	struct postfix_expression_call : public postfix_expression
	{
		postfix_expression* expr;
		expression_list* args;
	};

	struct postfix_expression_construct : public postfix_expression
	{
		simple_type_specifier* type;
		expression_list* args;
	};

	struct postfix_expression_member : public postfix_expression
	{
		postfix_expression* expr;
		id_expression* member;
	};

	struct psuedo_destructor_name
	{
		bool isGlobal;
		nested_name_specifier* context;
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

	struct postfix_expression_destructor : public postfix_expression
	{
		postfix_expression* expr;
		psuedo_destructor_name* destructor;
	};

	struct postfix_expression_default : public postfix_expression
	{
		postfix_expression* expr;
		enum { INCREMENT, DECREMENT } value;
	};

	struct postfix_expression_cast : public postfix_expression
	{
		enum { DYNAMIC, STATIC, REINTERPRET, CONST } key;
		type_id type;
		expression* expr;
	};

	struct postfix_expression_typeid : public postfix_expression
	{
		expression* expr;
	};

	struct postfix_expression_typeidtype : public postfix_expression
	{
		type_id type;
	};

	struct new_expression : public unary_expression
	{
		bool isGlobal;
		expression_list* place;
		type_id id;
		expression_list* init;
	};

	struct delete_expression : public unary_expression
	{
		bool isGlobal;
		bool isArray;
		cast_expression* expr;
	};

	struct unary_expression_pre : public unary_expression
	{
		enum { INCREMENT, DECREMENT, DEREFERENCE, ADDRESS, POSITIVE, NEGATIVE, LOGICALNOT, NOT } op;
		cast_expression* expr;
	};

	struct unary_expression_sizeof : public unary_expression
	{
		unary_expression* expr;
	};

	struct unary_expression_sizeoftype : public unary_expression
	{
		type_id id;
	};

	struct cast_expression_default : public cast_expression
	{
		type_id id;
		cast_expression* expr;
	};

	struct pm_expression_default : public pm_expression
	{
		pm_expression* left;
		cast_expression* right;
	};

	struct multiplicative_expression_default : public multiplicative_expression
	{
		multiplicative_expression* left;
		pm_expression* right;
	};

	struct additive_expression_default : public additive_expression
	{
		additive_expression* left;
		multiplicative_expression* right;
	};

	struct shift_expression_default : public shift_expression
	{
		shift_expression* left;
		additive_expression* right;
	};

	struct relational_expression_default : public relational_expression
	{
		relational_expression* left;
		shift_expression* right;
	};

	struct equality_expression_default : public equality_expression
	{
		equality_expression* left;
		relational_expression* right;
	};

	struct and_expression_default : public and_expression
	{
		and_expression* left;
		equality_expression* right;
	};

	struct exclusive_or_expression_default : public exclusive_or_expression
	{
		exclusive_or_expression* left;
		and_expression* right;
	};

	struct inclusive_or_expression_default : public inclusive_or_expression
	{
		inclusive_or_expression* left;
		exclusive_or_expression* right;
	};

	struct logical_and_expression_default : public logical_and_expression
	{
		logical_and_expression* left;
		inclusive_or_expression* right;
	};

	struct logical_or_expression_default : public logical_or_expression
	{
		logical_or_expression* left;
		logical_and_expression* right;
	};

	struct conditional_expression_default : public conditional_expression
	{
		expression* left;
		assignment_expression* right;
	};

	struct assignment_expression_default : public assignment_expression
	{
		enum { ASSIGN, MULTIPLY, DIVIDE, MODULO, ADD, SUBTRACT, SHIFTRIGHT, SHIFTLEFT, AND, NOT, OR } op;
		logical_or_expression* left;
		assignment_expression* right;
	};

	typedef conditional_expression constant_expression;

	struct operator_function_id : public unqualified_id
	{
		// operation omitted for brevity
		template_argument_list* args; // NULL if not template
	};


	struct conversion_declarator
	{
		ptr_operator* op;
		conversion_declarator* decl;
	};

	struct conversion_type_id
	{
		type_specifier_seq* spec;
		conversion_declarator* decl;
	};

	struct conversion_function_id : public unqualified_id, public conversion_type_id
	{
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

	struct direct_declarator_function : public direct_declarator
	{
		direct_declarator* decl;
		parameter_declaration_clause* params;
		cv_qualifier_seq* qual;
		exception_specification* except;
	};

	struct direct_declarator_array : public direct_declarator
	{
		direct_declarator* decl;
		constant_expression* size;
	};

	struct direct_declarator_parenthesis : public direct_declarator
	{
		declarator* decl;
	};

	struct declarator_default : public declarator
	{
		ptr_operator* op;
		declarator* decl;
	};

	struct member_declarator
	{
	};

	struct member_declarator_default
	{
		declarator* decl;
		constant_expression* init;
	};

	struct member_declarator_bitfield
	{
		identifier id; // may be empty
		constant_expression width;
	};

	struct member_declarator_list
	{
		member_declarator* item;
		member_declarator_list* next;
	};

	struct member_declaration
	{
	};

	struct member_declaration_var : public member_declaration
	{
		decl_specifier_seq* spec;
		member_declarator_list* decl;
	};

	struct member_declaration_func : public member_declaration
	{
		bool isGlobal;
		bool isTemplate;
		nested_name_specifier* context;
		unqualified_id id;
	};

	struct member_specification
	{
		access_specifier* access;
		member_declaration* item;
		member_specification* next;
	};

	struct class_specifier : public type_specifier
	{
		class_head* head;
		member_specification* members;
	};

	struct enumerator_definition
	{
		identifier id;
		constant_expression* init;
	};

	struct enumerator_list
	{
		enumerator_definition* item;
		enumerator_list* next;
	};

	struct enum_specifier : public type_specifier
	{
		identifier id; // may be empty
		enumerator_list* values;
	};

	struct elaborated_type_specifier : public type_specifier
	{
	};

	struct elaborated_type_specifier_default : public elaborated_type_specifier
	{
		// enum/class/struct/union omitted for brevity
		bool isGlobal;
		nested_name_specifier* context;
		identifier id;
	};

	struct elaborated_type_specifier_template : public elaborated_type_specifier
	{
		class_key key;
		bool isGlobal;
		nested_name_specifier* context;
		simple_template_id id;
	};

	struct typename_specifier : public type_specifier
	{
	};

	struct typename_specifier_default : public typename_specifier
	{
		bool isGlobal;
		nested_name_specifier* context;
		identifier id;
	};

	struct typename_specifier_template : public typename_specifier
	{
		bool isGlobal;
		nested_name_specifier* context;
		simple_template_id id;
	};

	struct parameter_declaration : public template_parameter
	{
	};

	struct type_parameter : public template_parameter
	{
	};

	struct type_parameter_default : public type_parameter
	{
		enum { CLASS, TYPENAME } key;
		identifier id; // may be empty;
		type_id* init; // if NULL, no initializer
	};

	struct template_parameter_list
	{
		template_parameter* item;
		template_parameter_list* next;
	};

	struct type_parameter_template : public type_parameter
	{
		template_parameter_list* params;
		identifier id; // may be empty;
		type_id* init; // if NULL, no initializer
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
		parameter_declaration_list* next;
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

	struct decl_specifier_builtin : public decl_specifier
	{
		enum { FRIEND, TYPEDEF } value;
	};

	struct storage_class_specifier : public decl_specifier
	{
		enum { REGISTER, STATIC, EXTERN, MUTABLE } value;
	};

	struct namespace_name : public nested_name_specifier
	{
		identifier id;
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

	struct class_name_default : public class_name, public identifier
	{
	};

	struct class_name_template : public class_name, public simple_template_id
	{
	};

	struct enum_name : public type_name, public identifier
	{
	};

	struct typedef_name : public type_name, public identifier
	{
	};

	struct simple_type_specifier_name : public simple_type_specifier
	{
		nested_name_specifier* spec;
		type_name* id;
	};

	struct simple_type_specifier_template : public simple_type_specifier
	{
	};

	struct simple_type_specifier_builtin : public simple_type_specifier
	{
		enum { CHAR, WCHAR_T, BOOL, SHORT, INT, LONG, SIGNED, UNSIGNED, FLOAT, DOUBLE, VOID, AUTO } value;
	};

	struct function_specifier : public decl_specifier
	{
		enum { INLINE, VIRTUAL, EXPLICIT } value;
	};

	typedef expression_list initializer;

	struct statement
	{	
	};

	struct declaration
	{
	};

	struct declaration_statement : public statement
	{
	};

	struct block_declaration : public declaration_statement, public declaration
	{
	};

	typedef std::string string_literal;

	struct asm_definition : public block_declaration
	{
		string_literal str;
	};


	struct namespace_alias_definition : public block_declaration
	{
		identifier id;
		bool isGlobal;
		nested_name_specifier* context;
		namespace_name* name;
	};

	struct using_declaration : public block_declaration
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
		namespace_name* name;
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

	struct simple_declaration : public block_declaration
	{
		decl_specifier_seq* spec;
		init_declarator_list decl;
	};

	struct mem_initializer_id_base : public mem_initializer_id
	{
		bool isGlobal;
		nested_name_specifier* context;
		class_name* name;
	};

	struct mem_initializer
	{
		expression_list* args;
	};

	struct mem_initializer_list
	{
		mem_initializer* item;
		mem_initializer_list* next;
	};

	struct ctor_initializer : public mem_initializer_list
	{
	};

	struct labeled_statement : public statement
	{
	};

	struct labeled_statement_default : public labeled_statement
	{
		identifier label;
		statement* body;
	};

	struct labeled_statement_case : public labeled_statement
	{
		constant_expression* label; // if NULL, then 'default'
		statement* body;
	};

	struct expression_statement : public statement
	{
		expression* expr;
	};

	struct selection_statement : public statement
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
		statement* true_;
		statement* false_;
	};

	struct selection_statement_switch : public selection_statement
	{
		condition* cond;
		statement* body;
	};

	struct iteration_statement : public statement
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
		expression_statement* expr;
		simple_declaration* decl;
		condition* cond;
		expression* incr;
		statement* body;
	};

	struct jump_statement : public statement
	{
	};

	struct jump_statement_default : public jump_statement
	{
		enum { BREAK, CONTINUE } value;
	};

	struct jump_statement_return : public jump_statement
	{
		expression* expr;
	};

	struct jump_statement_goto : public jump_statement
	{
		identifier label;
	};

	struct statement_seq
	{
		statement* item;
		statement_seq* next;
	};

	struct compound_statement : public statement
	{
		statement_seq* head;
	};

	struct function_body : public compound_statement
	{
	};

	struct exception_declaration
	{
		type_specifier_seq* type; // if NULL, all exceptions
	};

	struct exception_declaration_default : public exception_declaration
	{
		declarator* decl;
	};

	struct exception_declaration_abstract : public exception_declaration
	{
		abstract_declarator* decl;
	};

	struct handler_seq
	{
		exception_declaration* decl;
		compound_statement* body;
		handler_seq* next;
	};

	struct function_try_block
	{
		ctor_initializer* init;
		function_body* body;
		handler_seq* handlers;
	};


	struct declaration_seq
	{
		declaration* item;
		declaration_seq* next;
	};

	struct function_definition : public declaration, public member_declaration
	{
		decl_specifier_seq* spec;
		declarator* decl;
		ctor_initializer* init;
		function_body* body;
	};


	struct template_declaration : public declaration
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

	struct linkage_specification : public declaration
	{
		string_literal str;
		declaration_seq* decl;
	};

	typedef declaration_seq namespace_body;

	struct namespace_definition : public declaration
	{
		identifier id; // may be empty
		namespace_body* body;
	};
}


inline bool isEOF(const LexToken& token)
{
	return IS_CATEGORY(get_id(token), boost::wave::EOFTokenType);
}

inline bool isWhiteSpace(const LexToken& token)
{
	return IS_CATEGORY(get_id(token), boost::wave::WhiteSpaceTokenType)
		|| IS_CATEGORY(get_id(token), boost::wave::EOLTokenType);
}

struct ParserContext
{
	LexContext& context;
	LexIterator& first;
	LexIterator& last;

	ParserContext(std::string& instring, const char* input)
		: context(createContext(instring, input)), first(createBegin(context)), last(createEnd(context))
	{
		if(isWhiteSpace(dereference()))
		{
			increment();
		}
	}
	~ParserContext()
	{
		release(first);
		release(last);
		release(context);
	}

	const LexToken& dereference()
	{
		return ::dereference(first);
	}

	void increment()
	{
		for(;;)
		{
			if(first == last
				|| isEOF(dereference()))
			{
				throw LexError();
			}
			::increment(first);
			if(!isWhiteSpace(dereference()))
			{
				break;
			}
		}
	}
};

#define PARSE_ERROR() throw LexError()
#define PARSE_ASSERT(condition) if(!(condition)) { PARSE_ERROR(); }

inline bool isToken(const LexToken& token, boost::wave::token_id id)
{
	return get_id(token) == id;
}

inline bool isIdentifier(const LexToken& token)
{
	return isToken(token, boost::wave::T_IDENTIFIER);
}

inline std::string parseIdentifier(ParserContext& parser)
{
	PARSE_ASSERT(isIdentifier(parser.dereference()));
	std::string result(get_value(parser.dereference()));
	parser.increment();
	return result;
};

inline void parseToken(ParserContext& parser, boost::wave::token_id id)
{
	PARSE_ASSERT(isToken(parser.dereference(), id));
	parser.increment();
};

inline cpp::declaration_seq* parseDeclarationSeq(ParserContext& parser);

inline cpp::declaration* parseDeclaration(ParserContext& parser)
{
	if(get_id(parser.dereference()) == boost::wave::T_NAMESPACE)
	{
		cpp::namespace_definition* result = new cpp::namespace_definition;
		parseToken(parser, boost::wave::T_NAMESPACE);
		if(get_id(parser.dereference()) != boost::wave::T_LEFTBRACE)
		{
			result->id.value = parseIdentifier(parser);
		}
		parseToken(parser, boost::wave::T_LEFTBRACE);
		result->body = parseDeclarationSeq(parser);
		parseToken(parser, boost::wave::T_RIGHTBRACE);
		return result;
	}
	return NULL; // not a declaration
}

inline cpp::declaration_seq* parseDeclarationSeq(ParserContext& parser)
{
	cpp::declaration_seq* declarations = NULL;

	for(;;)
	{
		cpp::declaration* decl = parseDeclaration(parser);
		if(decl == NULL)
		{
			return declarations;
		}
		cpp::declaration_seq* node = new cpp::declaration_seq;
		node->item = decl;
		node->next = declarations;
		declarations = node;
	}
}


inline void parseFile(std::string& instring, const char* input)
{
	ParserContext parser(instring, input);

	cpp::declaration_seq* declarations = parseDeclarationSeq(parser);
}


#endif


