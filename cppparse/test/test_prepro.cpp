
#if 0
#include "../common.h"


#if 1
#define FOREACH1(a)
#define FOREACH2(a, b)
#define FOREACH3(a, b, c)
#define FOREACH4(a, b, c, d)
#define FOREACH5(a, b, c, d, e)
#define FOREACH6(a, b, c, d, e, f)
#define FOREACH7(a, b, c, d, e, f, g)
#define FOREACH8(a, b, c, d, e, f, g, h)
#endif

#define VISITABLE_BASE(Types_) typedef Types_ Types
#define VISITABLE_DERIVED(Base) void f(const Base::Types& );
#define VISITABLE_DERIVED_TMPL(Base) void f(const typename Base::Types&);




typedef int LexTokenId;

class Declaration;

namespace cpp
{
	template<LexTokenId id>
	struct terminal
	{
		static const LexTokenId ID = id;
		const char* value;
		terminal()
			: value(0)
		{
		}
	};

	template<LexTokenId id>
	struct terminal_optional : public terminal<id>
	{
	};

	template<LexTokenId id>
	struct terminal_suffix : public terminal<id>
	{
	};

	template<typename T>
	struct symbol
	{
		typedef T Type;
		T* p;
		symbol() : p(0)
		{
		}
		explicit symbol(T* p) : p(p)
		{
		}
		T* operator->()
		{
			return p;
		}
		operator T*()
		{
			return p;
		}
	};

	template<typename T>
	struct symbol_optional : public symbol<T>
	{
		symbol_optional()
		{
		}
		explicit symbol_optional(T* p) : symbol<T>(p)
		{
		}
	};

	template<typename T>
	struct symbol_sequence : public symbol<T>
	{
		symbol_sequence()
		{
		}
		explicit symbol_sequence(T* p) : symbol<T>(p)
		{
		}
	};

	template<typename T>
	struct symbol_next : public symbol<T>
	{
		symbol_next()
		{
		}
		explicit symbol_next(T* p) : symbol<T>(p)
		{
		}
	};

#define SYMBOLFWD(T) struct T

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

	struct terminal_choice2 // TODO: replace terminal_choice
	{
		LexTokenId id;
		const char* value; // TODO: avoid storing this
	};

	struct decoration
	{
		Declaration* p;
		decoration() : p(0)
		{
		}
	};

	struct terminal_identifier
	{
		const char* value;
		FilePosition position;
		const char* source;
		decoration dec;
	};

	struct terminal_string
	{
		const char* value;
	};

	template<typename T>
	struct ambiguity : public T
	{
		VISITABLE_DERIVED_TMPL(T);
		T* first;
		T* second;
		
	};


	struct template_argument : public choice<template_argument>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(type_id), // TODO: ambiguity: 'type-id' and 'primary-expression' may both be 'identifier'. Prefer type-id to handle 'T(*)()'.
			SYMBOLFWD(assignment_expression),
			//SYMBOLFWD(id_expression), // TODO: assignment-expression contains id-expression
			ambiguity<template_argument>
			));
	};

	struct template_parameter : public choice<template_parameter>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(type_parameter), // TODO: ambiguity 'typename T' could be typename-specifier or type-parameter
			SYMBOLFWD(parameter_declaration), // TODO: ambiguity 'class C' could be elaborated-type-specifier or type-parameter
			ambiguity<template_parameter>
			));
	};

	struct exception_declarator : public choice<exception_declarator>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(declarator),
			SYMBOLFWD(abstract_declarator)
			));
	};

	struct declarator : public choice<declarator>, public exception_declarator
	{
		VISITABLE_DERIVED(exception_declarator);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(declarator_ptr),
			SYMBOLFWD(direct_declarator)
			));
	};

	struct direct_declarator_prefix : public choice<direct_declarator_prefix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(direct_declarator_parenthesis),
			SYMBOLFWD(declarator_id)
			));
	};

	struct declarator_id : public choice<declarator_id>, public direct_declarator_prefix
	{
		VISITABLE_DERIVED(direct_declarator_prefix);
		VISITABLE_BASE(TYPELIST1(
			SYMBOLFWD(id_expression)
			));
	};

	struct condition : public choice<condition>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(condition_init),
			SYMBOLFWD(expression)
			));
	};

	struct expression : public choice<expression>, public condition
	{
		VISITABLE_DERIVED(condition);
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(expression_list),
			SYMBOLFWD(assignment_expression),
			ambiguity<expression>
			));
	};

	struct initializer : public choice<initializer>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(initializer_default),
			SYMBOLFWD(initializer_parenthesis)
			));
	};

	struct initializer_clause : public choice<initializer_clause>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(initializer_clause_empty),
			SYMBOLFWD(initializer_clause_list),
			SYMBOLFWD(assignment_expression)
			));
	};

	struct assignment_expression : public choice<assignment_expression>, public expression, public template_argument, public initializer_clause
	{
		VISITABLE_DERIVED(expression);
		VISITABLE_DERIVED(template_argument);
		VISITABLE_DERIVED(initializer_clause);
		VISITABLE_BASE(TYPELIST4(
			SYMBOLFWD(throw_expression),
			SYMBOLFWD(logical_or_expression_precedent),
			SYMBOLFWD(conditional_expression),
			ambiguity<assignment_expression>
			));
	};

	struct constant_expression : public choice<constant_expression>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(conditional_expression),
			ambiguity<constant_expression>
			));
	};

	struct conditional_expression : public choice<conditional_expression>, public assignment_expression, public constant_expression
	{
		VISITABLE_DERIVED(assignment_expression);
		VISITABLE_DERIVED(constant_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(conditional_expression_default),
			SYMBOLFWD(logical_or_expression)
			));
	};

	struct logical_or_expression : public choice<logical_or_expression>, public conditional_expression
	{
		VISITABLE_DERIVED(conditional_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(logical_or_expression_default),
			SYMBOLFWD(logical_and_expression)
			));
	};

	struct logical_and_expression : public choice<logical_and_expression>, public logical_or_expression
	{
		VISITABLE_DERIVED(logical_or_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(logical_and_expression_default),
			SYMBOLFWD(inclusive_or_expression)
			));
	};

	struct inclusive_or_expression : public choice<inclusive_or_expression>, public logical_and_expression
	{
		VISITABLE_DERIVED(logical_and_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(inclusive_or_expression_default),
			SYMBOLFWD(exclusive_or_expression)
			));
	};

	struct exclusive_or_expression : public choice<exclusive_or_expression>, public inclusive_or_expression
	{
		VISITABLE_DERIVED(inclusive_or_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(exclusive_or_expression_default),
			SYMBOLFWD(and_expression)
			));
	};

	struct and_expression : public choice<and_expression>, public exclusive_or_expression
	{
		VISITABLE_DERIVED(exclusive_or_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(and_expression_default),
			SYMBOLFWD(equality_expression)
			));
	};

	struct equality_expression : public choice<equality_expression>, public and_expression
	{
		VISITABLE_DERIVED(and_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(equality_expression_default),
			SYMBOLFWD(relational_expression)
			));
	};

	struct relational_expression : public choice<relational_expression>, public equality_expression
	{
		VISITABLE_DERIVED(equality_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(relational_expression_default),
			SYMBOLFWD(shift_expression)
			));
	};

	struct shift_expression : public choice<shift_expression>, public relational_expression
	{
		VISITABLE_DERIVED(relational_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(shift_expression_default),
			SYMBOLFWD(additive_expression)
			));
	};

	struct additive_expression : public choice<additive_expression>, public shift_expression
	{
		VISITABLE_DERIVED(shift_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(additive_expression_default),
			SYMBOLFWD(multiplicative_expression)
			));
	};

	struct multiplicative_expression : public choice<multiplicative_expression>, public additive_expression
	{
		VISITABLE_DERIVED(additive_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(multiplicative_expression_default),
			SYMBOLFWD(pm_expression)
			));
	};

	struct pm_expression : public choice<pm_expression>, public multiplicative_expression
	{
		VISITABLE_DERIVED(multiplicative_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(pm_expression_default),
			SYMBOLFWD(cast_expression)
			));
	};

	struct cast_expression : public choice<cast_expression>, public pm_expression
	{
		VISITABLE_DERIVED(pm_expression);
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(cast_expression_default),
			SYMBOLFWD(unary_expression),
			ambiguity<cast_expression>
			));
	};

	struct unary_expression : public choice<unary_expression>, public cast_expression
	{
		VISITABLE_DERIVED(cast_expression);
		VISITABLE_BASE(TYPELIST7(
			SYMBOLFWD(postfix_expression),
			SYMBOLFWD(unary_expression_sizeoftype),
			SYMBOLFWD(unary_expression_sizeof),
			SYMBOLFWD(unary_expression_op),
			SYMBOLFWD(new_expression),
			SYMBOLFWD(delete_expression),
			ambiguity<unary_expression>
			));
	};

	struct postfix_expression : public choice<postfix_expression>, public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(postfix_expression_default),
			SYMBOLFWD(postfix_expression_prefix)
			));
	};

	struct postfix_expression_prefix : public choice<postfix_expression_prefix>, public postfix_expression
	{
		VISITABLE_DERIVED(postfix_expression);
		VISITABLE_BASE(TYPELIST7(
			SYMBOLFWD(primary_expression),
			SYMBOLFWD(postfix_expression_disambiguate),
			SYMBOLFWD(postfix_expression_construct),
			SYMBOLFWD(postfix_expression_cast),
			SYMBOLFWD(postfix_expression_typeid),
			SYMBOLFWD(postfix_expression_typeidtype),
			ambiguity<postfix_expression_prefix>
			));
	};

	struct primary_expression : public choice<primary_expression>, public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		VISITABLE_BASE(TYPELIST4(
			SYMBOLFWD(id_expression),
			SYMBOLFWD(literal),
			SYMBOLFWD(primary_expression_builtin),
			SYMBOLFWD(primary_expression_parenthesis)
			));
	};

	struct id_expression : public choice<id_expression>, public declarator_id, public primary_expression
	{
		VISITABLE_DERIVED(declarator_id);
		VISITABLE_DERIVED(primary_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(qualified_id), // TODO: shared prefix ambiguity: 'identifier' vs 'nested-name-specifier'
			SYMBOLFWD(unqualified_id)
			));
	};

	struct unqualified_id : public choice<unqualified_id>, public id_expression
	{
		VISITABLE_DERIVED(id_expression);
		VISITABLE_BASE(TYPELIST5(
			SYMBOLFWD(template_id),
			SYMBOLFWD(identifier),
			SYMBOLFWD(operator_function_id),
			SYMBOLFWD(conversion_function_id),
			SYMBOLFWD(destructor_id)
			));
	};

	struct qualified_id : public choice<qualified_id>, public id_expression
	{
		VISITABLE_DERIVED(id_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(qualified_id_default),
			SYMBOLFWD(qualified_id_global)
			));
	};

	struct qualified_id_suffix : public choice<qualified_id_suffix>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(identifier),
			SYMBOLFWD(template_id),
			SYMBOLFWD(operator_function_id)
			));
	};

	struct template_id : public choice<template_id>, public unqualified_id, public qualified_id_suffix
	{
		VISITABLE_DERIVED(unqualified_id);
		VISITABLE_DERIVED(qualified_id_suffix);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(simple_template_id),
			SYMBOLFWD(template_id_operator_function)
			));
	};

	struct nested_name : public choice<nested_name>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(type_name),
			SYMBOLFWD(namespace_name),
			ambiguity<nested_name>
			));
	};

	struct type_name : public choice<type_name>, public nested_name
	{
		VISITABLE_DERIVED(nested_name);
		VISITABLE_BASE(TYPELIST1(
			SYMBOLFWD(class_name)
			));
	};

	struct class_name : public choice<class_name>, public type_name
	{
		VISITABLE_DERIVED(type_name);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(simple_template_id), // TODO: ambiguity: shared prefix 'identifier'
			SYMBOLFWD(identifier)
			));
	};

	struct namespace_name : public choice<namespace_name>, public nested_name
	{
		VISITABLE_DERIVED(nested_name);
		VISITABLE_BASE(TYPELIST1(
			SYMBOLFWD(identifier)
			));
	};

	struct mem_initializer_id : public choice<mem_initializer_id>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(mem_initializer_id_base),
			SYMBOLFWD(identifier),
			ambiguity<mem_initializer_id>
			));
	};

	struct identifier : public class_name, public namespace_name, public unqualified_id, public qualified_id_suffix, public mem_initializer_id
	{
		VISITABLE_DERIVED(class_name);
		VISITABLE_DERIVED(namespace_name);
		VISITABLE_DERIVED(unqualified_id);
		VISITABLE_DERIVED(qualified_id_suffix);
		VISITABLE_DERIVED(mem_initializer_id);
		terminal_identifier value;
		
	};

	struct template_argument_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct template_argument_clause_disambiguate : public choice<template_argument_clause_disambiguate>
	{
		VISITABLE_BASE(TYPELIST1(
			SYMBOLFWD(template_argument_clause) // disambiguates: < CONSTANT_EXPRESSION < 0 >
			));
	};

	struct template_argument_clause : public template_argument_clause_disambiguate
	{
		VISITABLE_DERIVED(template_argument_clause_disambiguate);
		
		
		
		
	};

	struct simple_template_id : public template_id, public class_name
	{
		VISITABLE_DERIVED(class_name);
		VISITABLE_DERIVED(template_id);
		
		
		
	};

	struct nested_name_specifier_prefix
	{
		
		
		
	};

	struct nested_name_specifier_suffix : public choice<nested_name_specifier_suffix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(nested_name_specifier_suffix_template),
			SYMBOLFWD(nested_name_specifier_suffix_default)
			));
	};

	struct nested_name_specifier_suffix_default : public nested_name_specifier_suffix
	{
		VISITABLE_DERIVED(nested_name_specifier_suffix);
		
		
		
	};

	struct nested_name_specifier_suffix_template : public nested_name_specifier_suffix
	{
		VISITABLE_DERIVED(nested_name_specifier_suffix);
		
		
		
		
	};

	struct nested_name_specifier_suffix_seq
	{
		
		
		
	};

	struct nested_name_specifier
	{
		
		
		
	};

	struct type_specifier : public choice<type_specifier>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(type_specifier_noncv),
			SYMBOLFWD(cv_qualifier)
			));
	};

	struct type_specifier_noncv : public choice<type_specifier_noncv>, public type_specifier
	{
		VISITABLE_DERIVED(type_specifier);
		VISITABLE_BASE(TYPELIST5(
			SYMBOLFWD(typename_specifier),
			SYMBOLFWD(simple_type_specifier),
			SYMBOLFWD(class_specifier),
			SYMBOLFWD(enum_specifier),
			SYMBOLFWD(elaborated_type_specifier)
			));
	};

	struct decl_specifier_suffix : public choice<decl_specifier_suffix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(decl_specifier_nontype),
			SYMBOLFWD(simple_type_specifier_builtin)
			));
	};

	struct decl_specifier_nontype : public choice<decl_specifier_nontype>, public decl_specifier_suffix
	{
		VISITABLE_DERIVED(decl_specifier_suffix);
		VISITABLE_BASE(TYPELIST4(
			SYMBOLFWD(storage_class_specifier),
			SYMBOLFWD(decl_specifier_default),
			SYMBOLFWD(function_specifier),
			SYMBOLFWD(cv_qualifier)
			));
	};

	struct decl_specifier_prefix_seq
	{
		
		
		
	};

	struct decl_specifier_suffix_seq
	{
		
		
		
	};

	struct decl_specifier_seq
	{
		
		
		
		
	};

	struct postfix_expression_type_specifier : public choice<postfix_expression_type_specifier>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(simple_type_specifier),
			SYMBOLFWD(typename_specifier)
			));
	};

	struct simple_type_specifier : public choice<simple_type_specifier>, public type_specifier_noncv, public postfix_expression_type_specifier
	{
		VISITABLE_DERIVED(postfix_expression_type_specifier);
		VISITABLE_DERIVED(type_specifier_noncv);
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(simple_type_specifier_builtin),
			SYMBOLFWD(simple_type_specifier_template),
			SYMBOLFWD(simple_type_specifier_name)
			));
	};

	struct overloadable_operator : public choice<overloadable_operator>
	{
#if 0
		VISITABLE_BASE(TYPELIST17(
			SYMBOLFWD(assignment_operator),
			SYMBOLFWD(member_operator),
			SYMBOLFWD(postfix_operator),
			SYMBOLFWD(unary_operator),
			SYMBOLFWD(pm_operator),
			SYMBOLFWD(multiplicative_operator),
			SYMBOLFWD(additive_operator),
			SYMBOLFWD(shift_operator),
			SYMBOLFWD(relational_operator),
			SYMBOLFWD(equality_operator),
			SYMBOLFWD(new_operator),
			SYMBOLFWD(delete_operator),
			SYMBOLFWD(comma_operator),
			SYMBOLFWD(function_operator),
			SYMBOLFWD(array_operator),
			SYMBOLFWD(bitwise_operator),
			SYMBOLFWD(logical_operator)
			));
#endif
	};

	struct bitwise_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { AND, OR, XOR, NOT } id;
		terminal_choice2 value;
		
	};

	struct logical_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { AND, OR } id;
		terminal_choice2 value;
		
	};

	struct array_operator : public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		
		
		
	};

	struct function_operator : public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		
		
		
	};

	struct comma_operator : public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		
		
	};

	struct new_operator : public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		
		
		
	};

	struct delete_operator : public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		
		
		
	};

	struct operator_function_id : public unqualified_id, public qualified_id_suffix
	{
		VISITABLE_DERIVED(unqualified_id);
		VISITABLE_DERIVED(qualified_id_suffix);
		
		
		
	};

	struct template_id_operator_function : public template_id
	{
		VISITABLE_DERIVED(template_id);
		
		
		
	};

	struct qualified_id_default : public qualified_id
	{
		VISITABLE_DERIVED(qualified_id);
		
		
		
		
		
	};

	struct qualified_id_global : public qualified_id
	{
		VISITABLE_DERIVED(qualified_id);
		
		
		
	};

	struct elaborated_type_specifier_key : public choice<elaborated_type_specifier_key>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(class_key),
			SYMBOLFWD(enum_key)
			));
	};

	struct class_key : public terminal_choice, public elaborated_type_specifier_key
	{
		VISITABLE_DERIVED(class_key);
		enum { CLASS, STRUCT, UNION } id;
		terminal_choice2 value;
		
	};

	struct enum_key : public elaborated_type_specifier_key
	{
		VISITABLE_DERIVED(elaborated_type_specifier_key);
		
		
	};

	struct access_specifier : public terminal_choice
	{
		enum { PRIVATE, PROTECTED, PUBLIC } id;
		terminal_choice2 value;
		
	};

	struct base_specifier_prefix : public choice<base_specifier_prefix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(base_specifier_access_virtual),
			SYMBOLFWD(base_specifier_virtual_access)
			));
	};

	struct base_specifier_access_virtual : public base_specifier_prefix
	{
		VISITABLE_DERIVED(base_specifier_prefix);
		symbol<access_specifier> access; // required
		
		
	};

	struct base_specifier_virtual_access : public base_specifier_prefix
	{
		VISITABLE_DERIVED(base_specifier_prefix);
		
		symbol_optional<access_specifier> access; // optional
		
	};

	struct base_specifier
	{
		
		
		
		terminal_optional<0> isTemplate; // TODO: disallow 'template' followed by non-template-id
		
		
	};


	struct base_specifier_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct base_clause
	{
		
		
		
	};

	struct class_head : public choice<class_head>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(class_head_default),
			SYMBOLFWD(class_head_unnamed)
			));
	};

	struct class_head_unnamed : public class_head
	{
		VISITABLE_DERIVED(class_head);
		
		
		
	};

	struct class_head_default : public class_head
	{
		VISITABLE_DERIVED(class_head);
		
		
		
		
		
	};

	struct type_specifier_suffix : public choice<type_specifier_suffix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(cv_qualifier),
			SYMBOLFWD(simple_type_specifier_builtin)
			));
	};

	struct cv_qualifier : public terminal_choice, public type_specifier, public decl_specifier_nontype, public type_specifier_suffix
	{
		VISITABLE_DERIVED(type_specifier);
		VISITABLE_DERIVED(decl_specifier_nontype);
		VISITABLE_DERIVED(type_specifier_suffix);
		enum { CONST, VOLATILE } id;
		terminal_choice2 value;
		
	};

	struct cv_qualifier_seq
	{
		
		
		
	};

	struct ptr_operator_key : public terminal_choice
	{
		enum { PTR, REF } id;
		terminal_choice2 value;
		
	};

	struct ptr_operator
	{
		
		
		
		
		
	};

	struct type_specifier_prefix_seq
	{
		
		
		
	};

	struct type_specifier_suffix_seq
	{
		
		
		
	};

	struct type_specifier_seq
	{
		
		
		
		
	};

	struct abstract_declarator : public choice<abstract_declarator>, public exception_declarator
	{
		VISITABLE_DERIVED(exception_declarator);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(abstract_declarator_ptr),
			SYMBOLFWD(direct_abstract_declarator)
			));
	};

	struct abstract_declarator_ptr : public abstract_declarator
	{
		VISITABLE_DERIVED(abstract_declarator);
		
		
		
	};

	struct abstract_declarator_parenthesis
	{
		
		
		
		
	};

	struct declarator_suffix;

	struct direct_abstract_declarator : public abstract_declarator
	{
		VISITABLE_DERIVED(abstract_declarator);
		
		
		
	};

	struct type_id : public template_argument
	{
		VISITABLE_DERIVED(template_argument);
		
		
		
	};

	struct throw_expression : public assignment_expression
	{
		VISITABLE_DERIVED(assignment_expression);
		
		
		
	};

	struct expression_list : public expression
	{
		VISITABLE_DERIVED(expression);
		
		terminal_suffix<0> comma;
		
		
	};

	struct member_initializer : public choice<member_initializer>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(constant_initializer), // TODO: ambiguity here!
			SYMBOLFWD(pure_specifier)
			));
	};

	struct constant_initializer : public member_initializer
	{
		VISITABLE_DERIVED(member_initializer);
		
		
		
	};

	struct literal : public choice<literal>, public primary_expression
	{
		VISITABLE_DERIVED(primary_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(numeric_literal),
			SYMBOLFWD(string_literal)
			));
	};

	struct numeric_literal : public terminal_choice, public literal
	{
		VISITABLE_DERIVED(literal);
		enum { INTEGER, CHARACTER, FLOATING, BOOLEAN } id;
		terminal_choice2 value;
		
	};

	struct string_literal : public literal
	{
		VISITABLE_DERIVED(literal);
		terminal_string value;
		
		
	};

	struct primary_expression_builtin : public primary_expression
	{
		VISITABLE_DERIVED(primary_expression);
		
		
	};

	struct primary_expression_parenthesis : public primary_expression
	{
		VISITABLE_DERIVED(primary_expression);
		
		
		
		
	};

	struct initializer_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct initializer_clause_list : public initializer_clause
	{
		VISITABLE_DERIVED(initializer_clause);
		
		
		
		
	};

	struct initializer_clause_empty : public initializer_clause
	{
		VISITABLE_DERIVED(initializer_clause);
		
		
		
	};

	struct initializer_default : public initializer
	{
		VISITABLE_DERIVED(initializer);
		
		
		
	};

	struct initializer_parenthesis : public initializer
	{
		VISITABLE_DERIVED(initializer);
		
		
		
		
	};

	struct postfix_expression_suffix : public choice<postfix_expression_suffix>
	{
		VISITABLE_BASE(TYPELIST5(
			SYMBOLFWD(postfix_expression_index),
			SYMBOLFWD(postfix_expression_call),
			SYMBOLFWD(postfix_expression_member),
			SYMBOLFWD(postfix_expression_destructor),
			SYMBOLFWD(postfix_operator)
			));
	};

	struct postfix_expression_suffix_seq
	{
		
		
		
	};

	struct postfix_expression_default : public postfix_expression
	{
		VISITABLE_DERIVED(postfix_expression);
		
		
		
	};

	struct postfix_expression_index : public postfix_expression_suffix
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		
		
		
		
	};

	struct postfix_expression_call : public postfix_expression_suffix
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		
		
		
		
	};

	struct postfix_expression_disambiguate : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		
		
		
	};

	struct member_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { DOT, ARROW } id;
		terminal_choice2 value;
		
	};

	struct postfix_expression_member : public postfix_expression_suffix
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		
		
		
		
	};

	struct postfix_expression_destructor : public postfix_expression_suffix
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		
		
		//
		
		
	};

	struct postfix_operator : public terminal_choice, public postfix_expression_suffix, public overloadable_operator
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		//VISITABLE_DERIVED(overloadable_operator);
		enum { PLUSPLUS, MINUSMINUS } id;
		terminal_choice2 value;
		
	};

	struct postfix_expression_construct : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		
		
		
		
		
	};

	struct cast_operator : public terminal_choice
	{
		enum { DYNAMIC, STATIC, REINTERPRET, CONST } id;
		terminal_choice2 value;
		
	};

	struct postfix_expression_cast : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		
		
		
		
		
		
		
		
	};

	struct postfix_expression_typeid : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		
		
		
		
		
	};

	struct postfix_expression_typeidtype : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		
		
		
		
		
	};

	struct new_type : public choice<new_type>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(new_type_parenthesis),
			SYMBOLFWD(new_type_default)
			));
	};

	struct new_declarator : public choice<new_declarator>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(direct_new_declarator),
			SYMBOLFWD(new_declarator_ptr)
			));
	};

	struct new_declarator_suffix
	{
		
		
		
		
		
	};

	struct direct_new_declarator : public new_declarator
	{
		VISITABLE_DERIVED(new_declarator);
		
		
		
		
		
	};

	struct new_declarator_ptr : public new_declarator
	{
		VISITABLE_DERIVED(new_declarator);
		
		
		
	};

	struct new_type_default : public new_type
	{
		VISITABLE_DERIVED(new_type);
		
		
		
	};

	struct new_type_parenthesis : public new_type
	{
		VISITABLE_DERIVED(new_type);
		
		
		
		
	};

	struct new_initializer
	{
		
		
		
		
	};

	struct new_expression : public choice<new_expression>, public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(new_expression_placement), // TODO: ambiguity: 'new-placement' vs parenthesised 'type-id'
			SYMBOLFWD(new_expression_default)
			));
	};

	struct new_expression_placement : public new_expression
	{
		VISITABLE_DERIVED(new_expression);
		
		
		
		
		
		
		
		
	};

	struct new_expression_default : public new_expression
	{
		VISITABLE_DERIVED(new_expression);
		
		
		
		
		
	};

	struct delete_expression : public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		
		
		
		
		
	};

	struct unary_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { PLUSPLUS, MINUSMINUS, STAR, AND, PLUS, MINUS, NOT, COMPL } id;
		terminal_choice2 value;
		
	};

	struct unary_expression_op : public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		
		
		
	};

	struct unary_expression_sizeof : public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		
		
		
	};

	struct unary_expression_sizeoftype : public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		
		
		
		
		
	};

	struct cast_expression_default : public cast_expression
	{
		VISITABLE_DERIVED(cast_expression);
		
		
		
		
		
	};

	struct pm_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { DOTSTAR, ARROWSTAR } id;
		terminal_choice2 value;
		
	};

	struct pm_expression_default : public pm_expression
	{
		VISITABLE_DERIVED(pm_expression);
		
		
		
		
	};

	struct multiplicative_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { STAR, DIVIDE, PERCENT } id;
		terminal_choice2 value;
		
	};

	struct multiplicative_expression_default : public multiplicative_expression
	{
		VISITABLE_DERIVED(multiplicative_expression);
		
		
		
		
	};

	struct additive_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { PLUS, MINUS } id;
		terminal_choice2 value;
		
	};

	struct additive_expression_default : public additive_expression
	{
		VISITABLE_DERIVED(additive_expression);
		
		
		
		
	};

	struct shift_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { SHIFTLEFT, SHIFTRIGHT } id;
		terminal_choice2 value;
		
	};

	struct shift_expression_default : public shift_expression
	{
		VISITABLE_DERIVED(shift_expression);
		
		
		
		
	};

	struct relational_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { LESS, GREATER, LESSEQUAL, GREATEREQUAL } id;
		terminal_choice2 value;
		
	};

	struct relational_expression_default : public relational_expression
	{
		VISITABLE_DERIVED(relational_expression);
		
		
		
		
	};

	struct equality_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { EQUAL, NOTEQUAL } id;
		terminal_choice2 value;
		
	};

	struct equality_expression_default : public equality_expression
	{
		VISITABLE_DERIVED(equality_expression);
		
		
		
		
	};

	struct and_expression_default : public and_expression
	{
		VISITABLE_DERIVED(and_expression);
		
		
		
		
	};

	struct exclusive_or_expression_default : public exclusive_or_expression
	{
		VISITABLE_DERIVED(exclusive_or_expression);
		
		
		
		
	};

	struct inclusive_or_expression_default : public inclusive_or_expression
	{
		VISITABLE_DERIVED(inclusive_or_expression);
		
		
		
		
	};

	struct logical_and_expression_default : public logical_and_expression
	{
		VISITABLE_DERIVED(logical_and_expression);
		
		
		
		
	};

	struct logical_or_expression_default : public logical_or_expression
	{
		VISITABLE_DERIVED(logical_or_expression);
		
		
		
		
	};

	struct logical_or_expression_suffix : public choice<logical_or_expression_suffix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(conditional_expression_rhs),
			SYMBOLFWD(assignment_expression_rhs)
			));
	};

	struct conditional_expression_rhs : public logical_or_expression_suffix
	{
		VISITABLE_DERIVED(logical_or_expression_suffix);
		
		
		
		
		
	};

	struct conditional_expression_default : public conditional_expression
	{
		VISITABLE_DERIVED(conditional_expression);
		
		
		
	};

	struct logical_or_expression_precedent : public assignment_expression
	{
		VISITABLE_DERIVED(assignment_expression);
		
		
		
	};

	struct assignment_operator : public terminal_choice, public overloadable_operator
	{
		//VISITABLE_DERIVED(overloadable_operator);
		enum { ASSIGN, STAR, DIVIDE, PERCENT, PLUS, MINUS, SHIFTRIGHT, SHIFTLEFT, AND, XOR, OR } id;
		terminal_choice2 value;
		
	};

	struct assignment_expression_rhs : public logical_or_expression_suffix
	{
		VISITABLE_DERIVED(logical_or_expression_suffix);
		
		
		
	};




	struct conversion_declarator
	{
		
		
		
	};

	struct conversion_function_id : public unqualified_id
	{
		VISITABLE_DERIVED(unqualified_id);
		
		
		
		
	};

	struct destructor_id : public unqualified_id 
	{
		VISITABLE_DERIVED(unqualified_id);
		//
		
		
	};

	struct parameter_declaration_clause;

	struct exception_type_list : public choice<exception_type_list>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(exception_type_all),
			SYMBOLFWD(type_id_list)
			));
	};

	struct exception_type_all : public exception_type_list
	{
		VISITABLE_DERIVED(exception_type_list);
		
		
	};

	struct type_id_list : public exception_type_list
	{
		VISITABLE_DERIVED(exception_type_list);
		
		terminal_suffix<0> comma;
		
		
	};

	struct exception_specification
	{
		
		
		
		
		
	};

	struct declarator_suffix : public choice<declarator_suffix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(declarator_suffix_array),
			SYMBOLFWD(declarator_suffix_function)
			));
	};

	struct declarator_suffix_function : public declarator_suffix
	{
		VISITABLE_DERIVED(declarator_suffix);
		
		
		
		
		
		
	};

	struct declarator_suffix_array : public declarator_suffix
	{
		VISITABLE_DERIVED(declarator_suffix);
		
		
		
		
		
	};

	struct direct_declarator : public declarator
	{
		VISITABLE_DERIVED(declarator);
		
		
		
	};

	struct direct_declarator_parenthesis : public direct_declarator_prefix
	{
		VISITABLE_DERIVED(direct_declarator_prefix);
		
		
		
		
	};

	struct declarator_ptr : public declarator
	{
		VISITABLE_DERIVED(declarator);
		
		
		
	};

	struct statement : public choice<statement>
	{
		VISITABLE_BASE(TYPELIST11(
			SYMBOLFWD(msext_asm_statement_braced), // TODO: shared-prefix ambiguity: braced and unbraced start with '__asm'
			SYMBOLFWD(msext_asm_statement),
			SYMBOLFWD(compound_statement),
			SYMBOLFWD(declaration_statement),
			SYMBOLFWD(labeled_statement),
			SYMBOLFWD(expression_statement),
			SYMBOLFWD(selection_statement),
			SYMBOLFWD(iteration_statement),
			SYMBOLFWD(jump_statement),
			SYMBOLFWD(try_block),
			ambiguity<statement>
			));
	};

	struct statement_seq
	{
		
		symbol_next<statement_seq> next;
		
	};

	// HACK: allows statement_seq parse to be deferred
	struct statement_seq_wrapper
	{
		symbol_sequence<statement_seq> wrapped;
		
	};

	struct function_body : public choice<function_body>
	{
		VISITABLE_BASE(TYPELIST1(
			SYMBOLFWD(compound_statement)
			));
	};

	struct compound_statement : public statement, public function_body
	{
		VISITABLE_DERIVED(statement);
		VISITABLE_DERIVED(function_body);
		
		
		
		
	};

	struct exception_declaration : public choice<exception_declaration>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(exception_declaration_all),
			SYMBOLFWD(exception_declaration_default)
			));
	};

	struct exception_declaration_default : public exception_declaration
	{
		VISITABLE_DERIVED(exception_declaration);
		
		
		
	};

	struct exception_declaration_all : public exception_declaration
	{
		VISITABLE_DERIVED(exception_declaration);
		
		
	};

	struct handler
	{
		
		
		
		
		
		
	};

	struct handler_seq
	{
		
		
		
	};

	struct linkage_specification_suffix : public choice<linkage_specification_suffix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(linkage_specification_compound),
			SYMBOLFWD(declaration)
			));
	};

	struct declaration : public choice<declaration>, public linkage_specification_suffix
	{
		IncludeEvents events;
		const char* source;
		VISITABLE_DERIVED(linkage_specification_suffix);
		VISITABLE_BASE(TYPELIST8(
			SYMBOLFWD(linkage_specification),
			SYMBOLFWD(explicit_instantiation),
			SYMBOLFWD(template_declaration),
			SYMBOLFWD(explicit_specialization),
			SYMBOLFWD(namespace_definition),
			SYMBOLFWD(general_declaration),
			SYMBOLFWD(constructor_definition),
			SYMBOLFWD(block_declaration)
			));
	};

	struct mem_initializer_id_base : public mem_initializer_id
	{
		VISITABLE_DERIVED(mem_initializer_id);
		
		
		
		
	};

	struct mem_initializer
	{
		
		
		
		
		
	};

	struct mem_initializer_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct mem_initializer_clause
	{
		
		
	};

	struct ctor_initializer
	{
		
		
		
	};

	struct general_declaration_suffix : public choice<general_declaration_suffix>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(type_declaration_suffix),
			SYMBOLFWD(simple_declaration_named),
			SYMBOLFWD(function_definition)
			));
	};

	struct simple_declaration_suffix : public choice<simple_declaration_suffix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(type_declaration_suffix),
			SYMBOLFWD(simple_declaration_named)
			));
	};

	struct member_declaration_suffix : public choice<member_declaration_suffix>
	{
		VISITABLE_BASE(TYPELIST4(
			SYMBOLFWD(type_declaration_suffix),
			SYMBOLFWD(member_declaration_bitfield),
			SYMBOLFWD(member_declaration_named),
			SYMBOLFWD(function_definition)
			));
	};

	struct type_declaration_suffix : public general_declaration_suffix, public simple_declaration_suffix, public member_declaration_suffix
	{
		VISITABLE_DERIVED(general_declaration_suffix);
		VISITABLE_DERIVED(simple_declaration_suffix);
		VISITABLE_DERIVED(member_declaration_suffix);
		
		
	};

	struct general_declaration : public declaration
	{
		VISITABLE_DERIVED(declaration);
		
		
		
	};

	struct function_definition_suffix : public choice<function_definition_suffix>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(function_try_block),
			SYMBOLFWD(function_definition_suffix_default)
			));
	};

	struct function_definition_suffix_default : public function_definition_suffix
	{
		VISITABLE_DERIVED(function_definition_suffix);
		
		
		
	};

	struct function_try_block : public function_definition_suffix
	{
		VISITABLE_DERIVED(function_definition_suffix);
		
		
		
		
		
	};

	struct function_definition : public general_declaration_suffix, public member_declaration_suffix
	{
		VISITABLE_DERIVED(general_declaration_suffix);
		VISITABLE_DERIVED(member_declaration_suffix);
		
		
		
	};

	struct member_declarator : public choice<member_declarator>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(member_declarator_bitfield), // TODO: shared prefix ambiguity: 'identifier'
			SYMBOLFWD(member_declarator_default)
			));
	};

	struct pure_specifier : public member_initializer
	{
		VISITABLE_DERIVED(member_initializer);
		
		terminal<0> zero; // TODO: check value is zero
		
	};

	struct member_declarator_default : public member_declarator
	{
		VISITABLE_DERIVED(member_declarator);
		
		
		
	};

	struct member_declarator_bitfield : public member_declarator
	{
		VISITABLE_DERIVED(member_declarator);
		
		
		
		
	};

	struct member_declarator_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct member_declaration : public choice<member_declaration>
	{
		VISITABLE_BASE(TYPELIST6(
			SYMBOLFWD(using_declaration),
			SYMBOLFWD(member_template_declaration),
			SYMBOLFWD(member_declaration_implicit), // shared-prefix ambiguity:  this matches a constructor: Class(Type);
			SYMBOLFWD(member_declaration_default), // this matches a member: Type(member);
			SYMBOLFWD(member_declaration_nested),
			ambiguity<member_declaration>
			));
	};

	struct member_declaration_default : public member_declaration
	{
		VISITABLE_DERIVED(member_declaration);
		
		
		
	};

	struct member_declaration_bitfield : public member_declaration_suffix
	{
		VISITABLE_DERIVED(member_declaration_suffix);
		
		
		
		
		
	};

	struct member_declaration_named : public member_declaration_suffix
	{
		VISITABLE_DERIVED(member_declaration_suffix);
		
		
		
	};

	struct member_declaration_nested : public member_declaration
	{
		VISITABLE_DERIVED(member_declaration);
		
		
		
		
		
		
	};

	struct function_specifier : public terminal_choice, public decl_specifier_nontype
	{
		VISITABLE_DERIVED(decl_specifier_nontype);
		enum { INLINE, VIRTUAL, EXPLICIT } id;
		terminal_choice2 value;
		
	};

	struct function_specifier_seq
	{
		
		
		
	};

	struct constructor_definition : public declaration
	{
		VISITABLE_DERIVED(declaration);
		
		
		
	};

	struct member_declaration_implicit : public member_declaration
	{
		VISITABLE_DERIVED(member_declaration);
		
		
		
	};

	struct member_specification : public choice<member_specification>
	{
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(member_specification_access),
			SYMBOLFWD(member_specification_list)
			));
	};

	struct member_specification_list : public member_specification
	{
		VISITABLE_DERIVED(member_specification);
		
		
		
	};

	struct member_specification_access : public member_specification
	{
		VISITABLE_DERIVED(member_specification);
		
		
		
		
	};

	struct class_specifier : public type_specifier_noncv
	{
		VISITABLE_DERIVED(type_specifier_noncv);
		
		
		
		
		
	};

	struct enumerator_definition
	{
		
		terminal_suffix<0> assign;
		
		
	};

	struct enumerator_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct enum_specifier : public type_specifier_noncv
	{
		VISITABLE_DERIVED(type_specifier_noncv);
		
		symbol_optional<identifier> id; // may be empty
		
		
		
		
	};

	struct elaborated_type_specifier : public choice<elaborated_type_specifier>, public type_specifier_noncv
	{
		VISITABLE_DERIVED(type_specifier_noncv);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(elaborated_type_specifier_template), // TODO: shared-prefix ambiguity: match 'simple-template-id' before 'identifier'
			SYMBOLFWD(elaborated_type_specifier_default)
			));
	};

	struct elaborated_type_specifier_default : public elaborated_type_specifier
	{
		VISITABLE_DERIVED(elaborated_type_specifier);
		
		
		
		
		
	};

	struct elaborated_type_specifier_template : public elaborated_type_specifier
	{
		VISITABLE_DERIVED(elaborated_type_specifier);
		
		
		
		
		
		
	};

	struct typename_specifier : public type_specifier_noncv, public postfix_expression_type_specifier
	{
		VISITABLE_DERIVED(type_specifier_noncv);
		VISITABLE_DERIVED(postfix_expression_type_specifier);
		
		
		
		
		symbol<type_name> id; // NOTE: only 'identifier' is allowed if 'isTemplate' is true
		
	};

	struct parameter_declaration : public choice<parameter_declaration>, public template_parameter
	{
		VISITABLE_DERIVED(template_parameter);
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(parameter_declaration_default), // TODO: ambiguity: 'C::A(X)' could be 'C::A X' or 'C::A(*)(X)'
			SYMBOLFWD(parameter_declaration_abstract),
			ambiguity<parameter_declaration>
			));
	};

	struct type_parameter : public choice<type_parameter>, public template_parameter
	{
		VISITABLE_DERIVED(template_parameter);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(type_parameter_default),
			SYMBOLFWD(type_parameter_template)
			));
	};

	struct type_parameter_key : public terminal_choice
	{
		enum { CLASS, TYPENAME } id;
		terminal_choice2 value;
		
	};

	struct type_parameter_default : public type_parameter
	{
		VISITABLE_DERIVED(type_parameter);
		
		
		terminal_suffix<0> assign;
		
		
	};

	struct template_parameter_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct type_parameter_template : public type_parameter
	{
		VISITABLE_DERIVED(type_parameter);
		
		
		
		
		
		
		terminal_suffix<0> assign;
		
		
	};

	struct default_parameter
	{
		
		
	};

	struct parameter_declaration_default : public parameter_declaration
	{
		VISITABLE_DERIVED(parameter_declaration);
		
		
		terminal_suffix<0> assign;
		
		
	};

	struct parameter_declaration_abstract : public parameter_declaration
	{
		VISITABLE_DERIVED(parameter_declaration);
		
		
		terminal_suffix<0> assign;
		
		
	};

	struct parameter_declaration_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct parameter_declaration_clause
	{
		
		
		
	};

	struct direct_abstract_declarator_function : public direct_abstract_declarator
	{
		
		
		
		
		
	};

	struct direct_abstract_declarator_array : public direct_abstract_declarator
	{
		
		
		
	};

	struct direct_abstract_declarator_parenthesis : public direct_abstract_declarator
	{
		
		
	};

	struct decl_specifier_default : public terminal_choice, public decl_specifier_nontype
	{
		VISITABLE_DERIVED(decl_specifier_nontype);
		enum { FRIEND, TYPEDEF } id;
		terminal_choice2 value;
		
	};

	struct storage_class_specifier : public terminal_choice, public decl_specifier_nontype
	{
		VISITABLE_DERIVED(decl_specifier_nontype);
		enum { REGISTER, STATIC, EXTERN, MUTABLE } id;
		terminal_choice2 value;
		
	};

	struct simple_type_specifier_name : public simple_type_specifier
	{
		VISITABLE_DERIVED(simple_type_specifier);
		
		
		
		
	};

	struct simple_type_specifier_template : public simple_type_specifier
	{
		VISITABLE_DERIVED(simple_type_specifier);
		
		
		
		
		
	};

	struct simple_type_specifier_builtin : public terminal_choice, public simple_type_specifier, public decl_specifier_suffix, public type_specifier_suffix
	{
		VISITABLE_DERIVED(simple_type_specifier);
		VISITABLE_DERIVED(decl_specifier_suffix);
		VISITABLE_DERIVED(type_specifier_suffix);
		enum { CHAR, WCHAR_T, BOOL, SHORT, INT, LONG, SIGNED, UNSIGNED, FLOAT, DOUBLE, VOID, AUTO } id;
		terminal_choice2 value;
		
	};

	struct declaration_statement : public choice<declaration_statement>, public statement
	{
		VISITABLE_DERIVED(statement);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(block_declaration),
			SYMBOLFWD(simple_declaration)
			));
	};

	struct block_declaration : public choice<block_declaration>, public declaration_statement, public declaration
	{
		VISITABLE_DERIVED(declaration_statement);
		VISITABLE_DERIVED(declaration);
		VISITABLE_BASE(TYPELIST4(
			SYMBOLFWD(asm_definition),
			SYMBOLFWD(namespace_alias_definition),
			SYMBOLFWD(using_declaration),
			SYMBOLFWD(using_directive)
			));
	};

	struct asm_definition : public block_declaration
	{
		VISITABLE_DERIVED(block_declaration);
		
		
		
		
		
		
	};

	struct msext_asm_element : public choice<msext_asm_element>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(msext_asm_statement_braced), // TODO: shared-prefix ambiguity: braced and unbraced start with '__asm'
			SYMBOLFWD(msext_asm_statement),
			SYMBOLFWD(msext_asm_terminal)
			));
	};

	struct msext_asm_terminal : public msext_asm_element
	{
		VISITABLE_DERIVED(msext_asm_element);
		terminal_identifier value;
		
	};

	struct msext_asm_element_list
	{
		
		
		
		
	};

	struct msext_asm_element_list_inline
	{
		
		
		
		
	};

	struct msext_asm_statement : public msext_asm_element, public statement
	{
		VISITABLE_DERIVED(msext_asm_element);
		VISITABLE_DERIVED(statement);
		
		
		
		
	};

	struct msext_asm_statement_braced : public msext_asm_element, public statement
	{
		VISITABLE_DERIVED(msext_asm_element);
		VISITABLE_DERIVED(statement);
		
		
		
		
		
		
	};


	struct namespace_alias_definition : public block_declaration
	{
		VISITABLE_DERIVED(block_declaration);
		
		
		
		
		
		
		
		
	};

	struct using_declaration : public choice<using_declaration>, public block_declaration, public member_declaration
	{
		VISITABLE_DERIVED(block_declaration);
		VISITABLE_DERIVED(member_declaration);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(using_declaration_global),
			SYMBOLFWD(using_declaration_nested)
			));
	};

	struct using_declaration_global : public using_declaration
	{
		VISITABLE_DERIVED(using_declaration);
		
		
		
		
		
	};

	struct using_declaration_nested : public using_declaration
	{
		VISITABLE_DERIVED(using_declaration);
		
		
		
		
		
		
		
	};

	struct using_directive : public block_declaration
	{
		VISITABLE_DERIVED(block_declaration);
		
		
		
		
		
		
		
	};

	struct for_init_statement : public choice<for_init_statement>
	{
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(expression_statement),
			SYMBOLFWD(simple_declaration),
			ambiguity<for_init_statement>
			));
	};

	struct init_declarator
	{
		
		
		
	};

	struct init_declarator_list
	{
		
		terminal_suffix<0> comma;
		
		
	};

	struct simple_declaration_named : public general_declaration_suffix, public simple_declaration_suffix
	{
		VISITABLE_DERIVED(simple_declaration_suffix);
		VISITABLE_DERIVED(general_declaration_suffix);
		
		
		
	};

	struct simple_declaration : public declaration_statement, public for_init_statement
	{
		VISITABLE_DERIVED(declaration_statement);
		VISITABLE_DERIVED(for_init_statement);
		symbol<decl_specifier_seq> spec; // 7-1: Only in function declarations for constructors, destructors, and type conversions can the decl-specifier-seq be omitted.
		
		
	};

	struct labeled_statement : public choice<labeled_statement>, public statement
	{
		VISITABLE_DERIVED(statement);
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(labeled_statement_id),
			SYMBOLFWD(labeled_statement_case),
			SYMBOLFWD(labeled_statement_default)
			));
	};

	struct labeled_statement_id : public labeled_statement
	{
		VISITABLE_DERIVED(labeled_statement);
		
		
		
		
	};

	struct labeled_statement_case : public labeled_statement
	{
		VISITABLE_DERIVED(labeled_statement);
		
		
		
		
		
	};

	struct labeled_statement_default : public labeled_statement
	{
		VISITABLE_DERIVED(labeled_statement);
		
		
		
		
	};

	struct expression_statement : public statement, public for_init_statement
	{
		VISITABLE_DERIVED(for_init_statement);
		VISITABLE_DERIVED(statement);
		
		
		
	};

	struct selection_statement : public choice<selection_statement>, public statement
	{
		VISITABLE_DERIVED(statement);
		VISITABLE_BASE(TYPELIST2(
			SYMBOLFWD(selection_statement_if),
			SYMBOLFWD(selection_statement_switch)
			));
	};

	struct condition_init : public condition
	{
		VISITABLE_DERIVED(condition);
		
		
		
		
		
	};

	struct selection_statement_if : public selection_statement
	{
		VISITABLE_DERIVED(selection_statement);
		
		
		
		
		
		terminal_suffix<0> key2;
		
		
	};

	struct selection_statement_switch : public selection_statement
	{
		VISITABLE_DERIVED(selection_statement);
		
		
		
		
		
		
	};

	struct iteration_statement : public choice<iteration_statement>, public statement
	{
		VISITABLE_DERIVED(statement);
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(iteration_statement_for),
			SYMBOLFWD(iteration_statement_while),
			SYMBOLFWD(iteration_statement_dowhile)
			));
	};

	struct iteration_statement_while : public iteration_statement
	{
		VISITABLE_DERIVED(iteration_statement);
		
		
		
		
		
		
	};

	struct iteration_statement_dowhile : public iteration_statement
	{
		VISITABLE_DERIVED(iteration_statement);
		
		
		
		
		
		
		
		
	};

	struct iteration_statement_for : public iteration_statement
	{
		VISITABLE_DERIVED(iteration_statement);
		
		
		
		
		
		
		
		
		
	};

	struct jump_statement : public choice<jump_statement>, public statement
	{
		VISITABLE_DERIVED(statement);
		VISITABLE_BASE(TYPELIST3(
			SYMBOLFWD(jump_statement_simple),
			SYMBOLFWD(jump_statement_return),
			SYMBOLFWD(jump_statement_goto)
			));
	};

	struct jump_statement_key : public terminal_choice
	{
		enum { BREAK, CONTINUE } id;
		terminal_choice2 value;
		
	};

	struct jump_statement_simple : public jump_statement
	{
		VISITABLE_DERIVED(jump_statement);
		
		
		
	};

	struct jump_statement_return : public jump_statement
	{
		VISITABLE_DERIVED(jump_statement);
		
		
		
		
	};

	struct jump_statement_goto : public jump_statement
	{
		VISITABLE_DERIVED(jump_statement);
		
		
		
		
	};

	struct try_block : public statement
	{
		VISITABLE_DERIVED(statement);
		
		
		
		
	};

	struct declaration_seq
	{
		
		symbol_next<declaration_seq> next;
		
	};

	struct template_declaration_prefix
	{
		
		
		
		
		
		
	};

	struct template_declaration : public declaration
	{
		VISITABLE_DERIVED(declaration);
		
		
		
	};

	struct member_template_declaration : public member_declaration
	{
		VISITABLE_DERIVED(member_declaration);
		
		
		
	};

	struct explicit_instantiation : public declaration
	{
		VISITABLE_DERIVED(declaration);
		
		
		
		
	};

	struct explicit_specialization : public declaration
	{
		VISITABLE_DERIVED(declaration);
		
		
		
		
		
	};

	struct linkage_specification_compound : public linkage_specification_suffix
	{
		VISITABLE_DERIVED(linkage_specification_suffix);
		
		symbol_sequence<declaration_seq> decl;
		
		
	};

	struct linkage_specification : public declaration
	{
		VISITABLE_DERIVED(declaration);
		
		
		
		
	};

	typedef declaration_seq namespace_body;

	struct namespace_definition : public declaration
	{
		VISITABLE_DERIVED(declaration);
		
		
		
		symbol_sequence<namespace_body> body;
		
		
	};
}

template<typename T>
inline cpp::symbol<T> makeSymbol(T* p)
{
	return cpp::symbol<T>(p);
}


#ifdef _DEBUG
#define SYMBOL_NAME(T) (typeid(T).name() + 12)
#else
#define SYMBOL_NAME(T) "$symbol"
#endif

#endif
