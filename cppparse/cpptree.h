
#ifndef INCLUDED_CPPPARSE_CPPTREE_H
#define INCLUDED_CPPPARSE_CPPTREE_H

#include "visitor.h"
#include "allocator.h"

#include <boost/wave/token_ids.hpp>

typedef boost::wave::token_id LexTokenId;


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
#ifdef ALLOCATOR_DEBUG
			ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
			return p;
		}
		operator T*()
		{
#ifdef ALLOCATOR_DEBUG
			ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
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
		SafePtr<Declaration> p;
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
		FOREACH1(first);
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
		FOREACH1(value);
	};

	struct template_argument_list
	{
		symbol<template_argument> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<template_argument_list> next;
		FOREACH3(item, comma, next);
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
		terminal<boost::wave::T_LESS> lb;
		symbol_optional<template_argument_list> args;
		terminal<boost::wave::T_GREATER> rb;
		FOREACH3(lb, args, rb);
	};

	struct simple_template_id : public template_id, public class_name
	{
		VISITABLE_DERIVED(class_name);
		VISITABLE_DERIVED(template_id);
		symbol<identifier> id;
		symbol<template_argument_clause_disambiguate> args;
		FOREACH2(id, args);
	};

	struct nested_name_specifier_prefix
	{
		symbol<nested_name> id;
		terminal<boost::wave::T_COLON_COLON> scope;
		FOREACH2(id, scope);
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
		symbol<identifier> id;
		terminal<boost::wave::T_COLON_COLON> scope;
		FOREACH2(id, scope);
	};

	struct nested_name_specifier_suffix_template : public nested_name_specifier_suffix
	{
		VISITABLE_DERIVED(nested_name_specifier_suffix);
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		symbol<simple_template_id> id;
		terminal<boost::wave::T_COLON_COLON> scope;
		FOREACH3(isTemplate, id, scope);
	};

	struct nested_name_specifier_suffix_seq
	{
		symbol<nested_name_specifier_suffix> item;
		symbol_optional<nested_name_specifier_suffix_seq> next;
		FOREACH2(item, next);
	};

	struct nested_name_specifier
	{
		symbol<nested_name_specifier_prefix> prefix;
		symbol_optional<nested_name_specifier_suffix_seq> suffix;
		FOREACH2(prefix, suffix);
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
		symbol<decl_specifier_nontype> item;
		symbol_optional<decl_specifier_prefix_seq> next;
		FOREACH2(item, next);
	};

	struct decl_specifier_suffix_seq
	{
		symbol<decl_specifier_suffix> item;
		symbol_optional<decl_specifier_suffix_seq> next;
		FOREACH2(item, next);
	};

	struct decl_specifier_seq
	{
		symbol_optional<decl_specifier_prefix_seq> prefix;
		symbol<type_specifier_noncv> type;
		symbol_optional<decl_specifier_suffix_seq> suffix;
		FOREACH3(prefix, type, suffix);
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
	};

	struct bitwise_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { AND, OR, XOR, NOT } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct logical_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { AND, OR } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct array_operator : public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		terminal<boost::wave::T_LEFTBRACKET> ls;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		FOREACH2(ls, rs);
	};

	struct function_operator : public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		terminal<boost::wave::T_LEFTPAREN> lp;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH2(lp, rp);
	};

	struct comma_operator : public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		terminal<boost::wave::T_COMMA> key;
		FOREACH1(key);
	};

	struct new_operator : public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		terminal<boost::wave::T_NEW> key;
		symbol_optional<array_operator> array;
		FOREACH2(key, array);
	};

	struct delete_operator : public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		terminal<boost::wave::T_DELETE> key;
		symbol_optional<array_operator> array;
		FOREACH2(key, array);
	};

	struct operator_function_id : public unqualified_id, public qualified_id_suffix
	{
		VISITABLE_DERIVED(unqualified_id);
		VISITABLE_DERIVED(qualified_id_suffix);
		terminal<boost::wave::T_OPERATOR> key;
		symbol<overloadable_operator> op;
		FOREACH2(key, op);
	};

	struct template_id_operator_function : public template_id
	{
		VISITABLE_DERIVED(template_id);
		symbol<operator_function_id> id;
		symbol<template_argument_clause_disambiguate> args;
		FOREACH2(id, args);
	};

	struct qualified_id_default : public qualified_id
	{
		VISITABLE_DERIVED(qualified_id);
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol<nested_name_specifier> context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		symbol<unqualified_id> id;
		FOREACH4(isGlobal, context, isTemplate, id);
	};

	struct qualified_id_global : public qualified_id
	{
		VISITABLE_DERIVED(qualified_id);
		terminal<boost::wave::T_COLON_COLON> isGlobal;
		symbol<qualified_id_suffix> id;
		FOREACH2(isGlobal, id);
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
		FOREACH1(value);
	};

	struct enum_key : public elaborated_type_specifier_key
	{
		VISITABLE_DERIVED(elaborated_type_specifier_key);
		terminal<boost::wave::T_ENUM> key;
		FOREACH1(key);
	};

	struct access_specifier : public terminal_choice
	{
		enum { PRIVATE, PROTECTED, PUBLIC } id;
		terminal_choice2 value;
		FOREACH1(value);
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
		terminal_optional<boost::wave::T_VIRTUAL> isVirtual;
		FOREACH2(access, isVirtual);
	};

	struct base_specifier_virtual_access : public base_specifier_prefix
	{
		VISITABLE_DERIVED(base_specifier_prefix);
		terminal<boost::wave::T_VIRTUAL> isVirtual;
		symbol_optional<access_specifier> access; // optional
		FOREACH2(isVirtual, access);
	};

	struct base_specifier
	{
		symbol_optional<base_specifier_prefix> prefix;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate; // TODO: disallow 'template' followed by non-template-id
		symbol<class_name> id;
		FOREACH5(prefix, isGlobal, context, isTemplate, id);
	};


	struct base_specifier_list
	{
		symbol<base_specifier> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<base_specifier_list> next;
		FOREACH3(item, comma, next);
	};

	struct base_clause
	{
		terminal<boost::wave::T_COLON> colon;
		symbol<base_specifier_list> list;
		FOREACH2(colon, list);
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
		symbol<class_key> key;
		symbol_optional<base_clause> base;
		FOREACH2(key, base);
	};

	struct class_head_default : public class_head
	{
		VISITABLE_DERIVED(class_head);
		symbol<class_key> key;
		symbol_optional<nested_name_specifier> context;
		symbol<class_name> id;
		symbol_optional<base_clause> base;
		FOREACH4(key, context, id, base);
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
		FOREACH1(value);
	};

	struct cv_qualifier_seq
	{
		symbol<cv_qualifier> item;
		symbol_optional<cv_qualifier_seq> next;
		FOREACH2(item, next);
	};

	struct ptr_operator_key : public terminal_choice
	{
		enum { PTR, REF } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct ptr_operator
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		symbol<ptr_operator_key> key;
		symbol_optional<cv_qualifier_seq> qual;
		FOREACH4(isGlobal, context, key, qual);
	};

	struct type_specifier_prefix_seq
	{
		symbol<cv_qualifier> item;
		symbol_optional<type_specifier_prefix_seq> next;
		FOREACH2(item, next);
	};

	struct type_specifier_suffix_seq
	{
		symbol<type_specifier_suffix> item;
		symbol_optional<type_specifier_suffix_seq> next;
		FOREACH2(item, next);
	};

	struct type_specifier_seq
	{
		symbol_optional<type_specifier_prefix_seq> prefix;
		symbol<type_specifier_noncv> type;
		symbol_optional<type_specifier_suffix_seq> suffix;
		FOREACH3(prefix, type, suffix);
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
		symbol<ptr_operator> op;
		symbol_optional<abstract_declarator> decl;
		FOREACH2(op, decl);
	};

	struct abstract_declarator_parenthesis
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<abstract_declarator> decl;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, decl, rp);
	};

	struct declarator_suffix;

	struct direct_abstract_declarator : public abstract_declarator
	{
		VISITABLE_DERIVED(abstract_declarator);
		symbol_optional<abstract_declarator_parenthesis> prefix;
		symbol<declarator_suffix> suffix;
		FOREACH2(prefix, suffix);
	};

	struct type_id : public template_argument
	{
		VISITABLE_DERIVED(template_argument);
		symbol<type_specifier_seq> spec;
		symbol_optional<abstract_declarator> decl;
		FOREACH2(spec, decl);
	};

	struct throw_expression : public assignment_expression
	{
		VISITABLE_DERIVED(assignment_expression);
		terminal<boost::wave::T_THROW> key;
		symbol_optional<assignment_expression> expr;
		FOREACH2(key, expr);
	};

	struct expression_list : public expression
	{
		VISITABLE_DERIVED(expression);
		symbol<assignment_expression> left;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<expression> right;
		FOREACH3(left, comma, right);
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
		terminal<boost::wave::T_ASSIGN> assign;
		symbol<constant_expression> expr;
		FOREACH2(assign, expr);
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
		FOREACH1(value);
	};

	struct string_literal : public literal
	{
		VISITABLE_DERIVED(literal);
		terminal_string value;
		symbol_optional<string_literal> next;
		FOREACH2(value, next);
	};

	struct primary_expression_builtin : public primary_expression
	{
		VISITABLE_DERIVED(primary_expression);
		terminal<boost::wave::T_THIS> key;
		FOREACH1(key);
	};

	struct primary_expression_parenthesis : public primary_expression
	{
		VISITABLE_DERIVED(primary_expression);
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<expression> expr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, expr, rp);
	};

	struct initializer_list
	{
		symbol<initializer_clause> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol_optional<initializer_list> next;
		FOREACH3(item, comma, next);
	};

	struct initializer_clause_list : public initializer_clause
	{
		VISITABLE_DERIVED(initializer_clause);
		terminal<boost::wave::T_LEFTBRACE> lb;
		symbol<initializer_list> list;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH3(lb, list, rb);
	};

	struct initializer_clause_empty : public initializer_clause
	{
		VISITABLE_DERIVED(initializer_clause);
		terminal<boost::wave::T_LEFTBRACE> lb;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH2(lb, rb);
	};

	struct initializer_default : public initializer
	{
		VISITABLE_DERIVED(initializer);
		terminal<boost::wave::T_ASSIGN> assign;
		symbol<initializer_clause> clause;
		FOREACH2(assign, clause);
	};

	struct initializer_parenthesis : public initializer
	{
		VISITABLE_DERIVED(initializer);
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<expression_list> list;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, list, rp);
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
		symbol<postfix_expression_suffix> item;
		symbol_optional<postfix_expression_suffix_seq> next;
		FOREACH2(item, next);
	};

	struct postfix_expression_default : public postfix_expression
	{
		VISITABLE_DERIVED(postfix_expression);
		symbol<postfix_expression_prefix> left;
		symbol_optional<postfix_expression_suffix_seq> right;
		FOREACH2(left, right);
	};

	struct postfix_expression_index : public postfix_expression_suffix
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		terminal<boost::wave::T_LEFTBRACKET> ls;
		symbol<expression> index;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		FOREACH3(ls, index, rs);
	};

	struct postfix_expression_call : public postfix_expression_suffix
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol_optional<expression_list> args;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, args, rp);
	};

	struct postfix_expression_disambiguate : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		symbol<primary_expression> left;
		symbol<postfix_expression_call> right;
		FOREACH2(left, right);
	};

	struct member_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { DOT, ARROW } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct postfix_expression_member : public postfix_expression_suffix
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		symbol<member_operator> op;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		symbol<id_expression> id;
		FOREACH3(op, isTemplate, id);
	};

	struct postfix_expression_destructor : public postfix_expression_suffix
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		terminal<boost::wave::T_COMPL> compl;
		symbol<type_name> type;
		FOREACH4(isGlobal, context, compl, type);
	};

	struct postfix_operator : public terminal_choice, public postfix_expression_suffix, public overloadable_operator
	{
		VISITABLE_DERIVED(postfix_expression_suffix);
		VISITABLE_DERIVED(overloadable_operator);
		enum { PLUSPLUS, MINUSMINUS } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct postfix_expression_construct : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		symbol<postfix_expression_type_specifier> type;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol_optional<expression_list> args;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(type, lp, args, rp);
	};

	struct cast_operator : public terminal_choice
	{
		enum { DYNAMIC, STATIC, REINTERPRET, CONST } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct postfix_expression_cast : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		symbol<cast_operator> op;
		terminal<boost::wave::T_LESS> lt;
		symbol<type_id> type;
		terminal<boost::wave::T_GREATER> gt;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<expression> expr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH7(op, lt, type, gt, lp, expr, rp);
	};

	struct postfix_expression_typeid : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		terminal<boost::wave::T_TYPEID> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<expression> expr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(key, lp, expr, rp);
	};

	struct postfix_expression_typeidtype : public postfix_expression_prefix
	{
		VISITABLE_DERIVED(postfix_expression_prefix);
		terminal<boost::wave::T_TYPEID> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<type_id> type;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(key, lp, type, rp);
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
		terminal<boost::wave::T_LEFTBRACKET> ls;
		symbol<constant_expression> expr;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		symbol_optional<new_declarator_suffix> next;
		FOREACH4(ls, expr, rs, next);
	};

	struct direct_new_declarator : public new_declarator
	{
		VISITABLE_DERIVED(new_declarator);
		terminal<boost::wave::T_LEFTBRACKET> ls;
		symbol<expression> expr;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		symbol_optional<new_declarator_suffix> suffix;
		FOREACH4(ls, expr, rs, suffix);
	};

	struct new_declarator_ptr : public new_declarator
	{
		VISITABLE_DERIVED(new_declarator);
		symbol<ptr_operator> op;
		symbol_optional<new_declarator> decl;
		FOREACH2(op, decl);
	};

	struct new_type_default : public new_type
	{
		VISITABLE_DERIVED(new_type);
		symbol<type_specifier_seq> spec;
		symbol_optional<new_declarator> decl;
		FOREACH2(spec, decl);
	};

	struct new_type_parenthesis : public new_type
	{
		VISITABLE_DERIVED(new_type);
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<type_id> id;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, id, rp);
	};

	struct new_initializer
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol_optional<expression_list> expr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, expr, rp);
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
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		terminal<boost::wave::T_NEW> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<expression_list> place;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		symbol<new_type> type;
		symbol_optional<new_initializer> init;
		FOREACH7(isGlobal, key, lp, place, rp, type, init);
	};

	struct new_expression_default : public new_expression
	{
		VISITABLE_DERIVED(new_expression);
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		terminal<boost::wave::T_NEW> key;
		symbol<new_type> type;
		symbol_optional<new_initializer> init;
		FOREACH4(isGlobal, key, type, init);
	};

	struct delete_expression : public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		terminal<boost::wave::T_DELETE> key;
		symbol_optional<array_operator> op;
		symbol<cast_expression> expr;
		FOREACH4(isGlobal, key, op, expr);
	};

	struct unary_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { PLUSPLUS, MINUSMINUS, STAR, AND, PLUS, MINUS, NOT, COMPL } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct unary_expression_op : public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		symbol<unary_operator> op;
		symbol<cast_expression> expr;
		FOREACH2(op, expr);
	};

	struct unary_expression_sizeof : public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		terminal<boost::wave::T_SIZEOF> key;
		symbol<unary_expression> expr;
		FOREACH2(key, expr);
	};

	struct unary_expression_sizeoftype : public unary_expression
	{
		VISITABLE_DERIVED(unary_expression);
		terminal<boost::wave::T_SIZEOF> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<type_id> type;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(key, lp, type, rp);
	};

	struct cast_expression_default : public cast_expression
	{
		VISITABLE_DERIVED(cast_expression);
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<type_id> id;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		symbol<cast_expression> expr;
		FOREACH4(lp, id, rp, expr);
	};

	struct pm_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { DOTSTAR, ARROWSTAR } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct pm_expression_default : public pm_expression
	{
		VISITABLE_DERIVED(pm_expression);
		symbol<pm_expression> left;
		symbol<pm_operator> op;
		symbol<cast_expression> right;
		FOREACH3(left, op, right);
	};

	struct multiplicative_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { STAR, DIVIDE, PERCENT } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct multiplicative_expression_default : public multiplicative_expression
	{
		VISITABLE_DERIVED(multiplicative_expression);
		symbol<multiplicative_expression> left;
		symbol<multiplicative_operator> op;
		symbol<pm_expression> right;
		FOREACH3(left, op, right);
	};

	struct additive_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { PLUS, MINUS } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct additive_expression_default : public additive_expression
	{
		VISITABLE_DERIVED(additive_expression);
		symbol<additive_expression> left;
		symbol<additive_operator> op;
		symbol<multiplicative_expression> right;
		FOREACH3(left, op, right);
	};

	struct shift_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { SHIFTLEFT, SHIFTRIGHT } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct shift_expression_default : public shift_expression
	{
		VISITABLE_DERIVED(shift_expression);
		symbol<shift_expression> left;
		symbol<shift_operator> op;
		symbol<additive_expression> right;
		FOREACH3(left, op, right);
	};

	struct relational_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { LESS, GREATER, LESSEQUAL, GREATEREQUAL } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct relational_expression_default : public relational_expression
	{
		VISITABLE_DERIVED(relational_expression);
		symbol<relational_expression> left;
		symbol<relational_operator> op;
		symbol<shift_expression> right;
		FOREACH3(left, op, right);
	};

	struct equality_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { EQUAL, NOTEQUAL } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct equality_expression_default : public equality_expression
	{
		VISITABLE_DERIVED(equality_expression);
		symbol<equality_expression> left;
		symbol<equality_operator> op;
		symbol<relational_expression> right;
		FOREACH3(left, op, right);
	};

	struct and_expression_default : public and_expression
	{
		VISITABLE_DERIVED(and_expression);
		symbol<and_expression> left;
		terminal<boost::wave::T_AND> op;
		symbol<equality_expression> right;
		FOREACH3(left, op, right);
	};

	struct exclusive_or_expression_default : public exclusive_or_expression
	{
		VISITABLE_DERIVED(exclusive_or_expression);
		symbol<exclusive_or_expression> left;
		terminal<boost::wave::T_XOR> op;
		symbol<and_expression> right;
		FOREACH3(left, op, right);
	};

	struct inclusive_or_expression_default : public inclusive_or_expression
	{
		VISITABLE_DERIVED(inclusive_or_expression);
		symbol<inclusive_or_expression> left;
		terminal<boost::wave::T_OR> op;
		symbol<exclusive_or_expression> right;
		FOREACH3(left, op, right);
	};

	struct logical_and_expression_default : public logical_and_expression
	{
		VISITABLE_DERIVED(logical_and_expression);
		symbol<logical_and_expression> left;
		terminal<boost::wave::T_ANDAND> op;
		symbol<inclusive_or_expression> right;
		FOREACH3(left, op, right);
	};

	struct logical_or_expression_default : public logical_or_expression
	{
		VISITABLE_DERIVED(logical_or_expression);
		symbol<logical_or_expression> left;
		terminal<boost::wave::T_OROR> op;
		symbol<logical_and_expression> right;
		FOREACH3(left, op, right);
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
		terminal<boost::wave::T_QUESTION_MARK> key;
		symbol<expression> mid;
		terminal<boost::wave::T_COLON> colon;
		symbol<assignment_expression> right;
		FOREACH4(key, mid, colon, right);
	};

	struct conditional_expression_default : public conditional_expression
	{
		VISITABLE_DERIVED(conditional_expression);
		symbol<logical_or_expression> left;
		symbol_optional<conditional_expression_rhs> right;
		FOREACH2(left, right);
	};

	struct logical_or_expression_precedent : public assignment_expression
	{
		VISITABLE_DERIVED(assignment_expression);
		symbol<logical_or_expression> left;
		symbol_optional<logical_or_expression_suffix> right;
		FOREACH2(left, right);
	};

	struct assignment_operator : public terminal_choice, public overloadable_operator
	{
		VISITABLE_DERIVED(overloadable_operator);
		enum { ASSIGN, STAR, DIVIDE, PERCENT, PLUS, MINUS, SHIFTRIGHT, SHIFTLEFT, AND, XOR, OR } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct assignment_expression_rhs : public logical_or_expression_suffix
	{
		VISITABLE_DERIVED(logical_or_expression_suffix);
		symbol<assignment_operator> op;
		symbol<assignment_expression> right;
		FOREACH2(op, right);
	};




	struct conversion_declarator
	{
		symbol<ptr_operator> op;
		symbol_optional<conversion_declarator> decl;
		FOREACH2(op, decl);
	};

	struct conversion_function_id : public unqualified_id
	{
		VISITABLE_DERIVED(unqualified_id);
		terminal<boost::wave::T_OPERATOR> key;
		symbol<type_specifier_seq> spec;
		symbol_optional<conversion_declarator> decl;
		FOREACH3(key, spec, decl);
	};

	struct destructor_id : public unqualified_id 
	{
		VISITABLE_DERIVED(unqualified_id);
		terminal<boost::wave::T_COMPL> compl;
		symbol<identifier> name;
		FOREACH2(compl, name);
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
		terminal<boost::wave::T_ELLIPSIS> key;
		FOREACH1(key);
	};

	struct type_id_list : public exception_type_list
	{
		VISITABLE_DERIVED(exception_type_list);
		symbol<type_id> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<type_id_list> next;
		FOREACH3(item, comma, next);
	};

	struct exception_specification
	{
		terminal<boost::wave::T_THROW> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol_optional<exception_type_list> types;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(key, lp, types, rp);
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
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol_optional<parameter_declaration_clause> params;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		symbol_optional<cv_qualifier_seq> qual;
		symbol_optional<exception_specification> except;
		FOREACH5(lp, params, rp, qual, except);
	};

	struct declarator_suffix_array : public declarator_suffix
	{
		VISITABLE_DERIVED(declarator_suffix);
		terminal<boost::wave::T_LEFTBRACKET> ls;
		symbol_optional<constant_expression> size;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		symbol_optional<declarator_suffix_array> next;
		FOREACH4(ls, size, rs, next);
	};

	struct direct_declarator : public declarator
	{
		VISITABLE_DERIVED(declarator);
		symbol<direct_declarator_prefix> prefix;
		symbol_optional<declarator_suffix> suffix;
		FOREACH2(prefix, suffix);
	};

	struct direct_declarator_parenthesis : public direct_declarator_prefix
	{
		VISITABLE_DERIVED(direct_declarator_prefix);
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<declarator> decl;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, decl, rp);
	};

	struct declarator_ptr : public declarator
	{
		VISITABLE_DERIVED(declarator);
		symbol<ptr_operator> op;
		symbol<declarator> decl;
		FOREACH2(op, decl);
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
		symbol<statement> item;
		symbol_next<statement_seq> next;
		FOREACH2(item, next);
	};

	// HACK: allows statement_seq parse to be deferred
	struct statement_seq_wrapper
	{
		symbol_sequence<statement_seq> wrapped;
		FOREACH1(wrapped);
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
		terminal<boost::wave::T_LEFTBRACE> lb;
		symbol_optional<statement_seq_wrapper> body;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH3(lb, body, rb);
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
		symbol<type_specifier_seq> type;
		symbol<exception_declarator> decl;
		FOREACH2(type, decl);
	};

	struct exception_declaration_all : public exception_declaration
	{
		VISITABLE_DERIVED(exception_declaration);
		terminal<boost::wave::T_ELLIPSIS> key;
		FOREACH1(key);
	};

	struct handler
	{
		terminal<boost::wave::T_CATCH> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<exception_declaration> decl;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		symbol<compound_statement> body;
		FOREACH5(key, lp, decl, rp, body);
	};

	struct handler_seq
	{
		symbol<handler> item;
		symbol_optional<handler_seq> next;
		FOREACH2(item, next);
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
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		symbol<class_name> type;
		FOREACH3(isGlobal, context, type);
	};

	struct mem_initializer
	{
		symbol<mem_initializer_id> id;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol_optional<expression_list> args;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(id, lp, args, rp);
	};

	struct mem_initializer_list
	{
		symbol<mem_initializer> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<mem_initializer_list> next;
		FOREACH3(item, comma, next);
	};

	struct mem_initializer_clause
	{
		symbol<mem_initializer_list> list;
		FOREACH1(list);
	};

	struct ctor_initializer
	{
		terminal<boost::wave::T_COLON> colon;
		symbol<mem_initializer_clause> list;
		FOREACH2(colon, list);
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
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH1(semicolon);
	};

	struct general_declaration : public declaration
	{
		VISITABLE_DERIVED(declaration);
		symbol_optional<decl_specifier_seq> spec;
		symbol<general_declaration_suffix> suffix;
		FOREACH2(spec, suffix);
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
		symbol_optional<ctor_initializer> init;
		symbol<function_body> body;
		FOREACH2(init, body);
	};

	struct function_try_block : public function_definition_suffix
	{
		VISITABLE_DERIVED(function_definition_suffix);
		terminal<boost::wave::T_TRY> isTry;
		symbol_optional<ctor_initializer> init;
		symbol<function_body> body;
		symbol<handler_seq> handlers;
		FOREACH4(isTry, init, body, handlers);
	};

	struct function_definition : public general_declaration_suffix, public member_declaration_suffix
	{
		VISITABLE_DERIVED(general_declaration_suffix);
		VISITABLE_DERIVED(member_declaration_suffix);
		symbol<declarator> decl;
		symbol<function_definition_suffix> suffix;
		FOREACH2(decl, suffix);
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
		terminal<boost::wave::T_ASSIGN> assign;
		terminal<boost::wave::T_DECIMALINT> zero; // TODO: check value is zero
		FOREACH2(assign, zero);
	};

	struct member_declarator_default : public member_declarator
	{
		VISITABLE_DERIVED(member_declarator);
		symbol<declarator> decl;
		symbol_optional<member_initializer> init;
		FOREACH2(decl, init);
	};

	struct member_declarator_bitfield : public member_declarator
	{
		VISITABLE_DERIVED(member_declarator);
		symbol_optional<identifier> id;
		terminal<boost::wave::T_COLON> colon;
		symbol<constant_expression> width;
		FOREACH3(id, colon, width);
	};

	struct member_declarator_list
	{
		symbol<member_declarator> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<member_declarator_list> next;
		FOREACH3(item, comma, next);
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
		symbol<decl_specifier_seq> spec;
		symbol<member_declaration_suffix> suffix;
		FOREACH2(spec, suffix);
	};

	struct member_declaration_bitfield : public member_declaration_suffix
	{
		VISITABLE_DERIVED(member_declaration_suffix);
		symbol<member_declarator_bitfield> item;
		terminal_optional<boost::wave::T_COMMA> comma;
		symbol<member_declarator_list> next;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH4(item, comma, next, semicolon);
	};

	struct member_declaration_named : public member_declaration_suffix
	{
		VISITABLE_DERIVED(member_declaration_suffix);
		symbol<member_declarator_list> decl;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH2(decl, semicolon);
	};

	struct member_declaration_nested : public member_declaration
	{
		VISITABLE_DERIVED(member_declaration);
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol<nested_name_specifier> context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		symbol<unqualified_id> id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH5(isGlobal, context, isTemplate, id, semicolon);
	};

	struct function_specifier : public terminal_choice, public decl_specifier_nontype
	{
		VISITABLE_DERIVED(decl_specifier_nontype);
		enum { INLINE, VIRTUAL, EXPLICIT } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct function_specifier_seq
	{
		symbol<function_specifier> item;
		symbol_optional<function_specifier_seq> next;
		FOREACH2(item, next);
	};

	struct constructor_definition : public declaration
	{
		VISITABLE_DERIVED(declaration);
		symbol_optional<function_specifier_seq> spec;
		symbol<function_definition> suffix;
		FOREACH2(spec, suffix);
	};

	struct member_declaration_implicit : public member_declaration
	{
		VISITABLE_DERIVED(member_declaration);
		symbol_optional<function_specifier_seq> spec;
		symbol<member_declaration_suffix> suffix;
		FOREACH2(spec, suffix);
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
		symbol<member_declaration> item;
		symbol_optional<member_specification> next;
		FOREACH2(item, next);
	};

	struct member_specification_access : public member_specification
	{
		VISITABLE_DERIVED(member_specification);
		symbol<access_specifier> access;
		terminal<boost::wave::T_COLON> colon;
		symbol_optional<member_specification> next;
		FOREACH3(access, colon, next);
	};

	struct class_specifier : public type_specifier_noncv
	{
		VISITABLE_DERIVED(type_specifier_noncv);
		symbol<class_head> head;
		terminal<boost::wave::T_LEFTBRACE> lb;
		symbol_optional<member_specification> members;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH4(head, lb, members, rb);
	};

	struct enumerator_definition
	{
		symbol<identifier> id;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		symbol<constant_expression> init;
		FOREACH3(id, assign, init);
	};

	struct enumerator_list
	{
		symbol<enumerator_definition> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol_optional<enumerator_list> next;
		FOREACH3(item, comma, next);
	};

	struct enum_specifier : public type_specifier_noncv
	{
		VISITABLE_DERIVED(type_specifier_noncv);
		terminal<boost::wave::T_ENUM> key;
		symbol_optional<identifier> id; // may be empty
		terminal<boost::wave::T_LEFTBRACE> lb;
		symbol_optional<enumerator_list> values;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH5(key, id, lb, values, rb);
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
		symbol<elaborated_type_specifier_key> key;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		symbol<identifier> id;
		FOREACH4(key, isGlobal, context, id);
	};

	struct elaborated_type_specifier_template : public elaborated_type_specifier
	{
		VISITABLE_DERIVED(elaborated_type_specifier);
		symbol<class_key> key;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		symbol<simple_template_id> id;
		FOREACH5(key, isGlobal, context, isTemplate, id);
	};

	struct typename_specifier : public type_specifier_noncv, public postfix_expression_type_specifier
	{
		VISITABLE_DERIVED(type_specifier_noncv);
		VISITABLE_DERIVED(postfix_expression_type_specifier);
		terminal<boost::wave::T_TYPENAME> key;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		symbol<type_name> id; // NOTE: only 'identifier' is allowed if 'isTemplate' is true
		FOREACH5(key, isGlobal, context, isTemplate, id);
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
		FOREACH1(value);
	};

	struct type_parameter_default : public type_parameter
	{
		VISITABLE_DERIVED(type_parameter);
		symbol<type_parameter_key> key;
		symbol_optional<identifier> id;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		symbol<type_id> init;
		FOREACH4(key, id, assign, init);
	};

	struct template_parameter_list
	{
		symbol<template_parameter> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<template_parameter_list> next;
		FOREACH3(item, comma, next);
	};

	struct type_parameter_template : public type_parameter
	{
		VISITABLE_DERIVED(type_parameter);
		terminal<boost::wave::T_TEMPLATE> key;
		terminal<boost::wave::T_LESS> lt;
		symbol<template_parameter_list> params;
		terminal<boost::wave::T_GREATER> gt;
		terminal<boost::wave::T_CLASS> key2;
		symbol_optional<identifier> id;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		symbol<id_expression> init;
		FOREACH8(key, lt, params, gt, key2, id, assign, init);
	};

	struct default_parameter
	{
		symbol<assignment_expression> expr;
		FOREACH1(expr);
	};

	struct parameter_declaration_default : public parameter_declaration
	{
		VISITABLE_DERIVED(parameter_declaration);
		symbol<decl_specifier_seq> spec;
		symbol<declarator> decl;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		symbol<default_parameter> init;
		FOREACH4(spec, decl, assign, init);
	};

	struct parameter_declaration_abstract : public parameter_declaration
	{
		VISITABLE_DERIVED(parameter_declaration);
		symbol<decl_specifier_seq> spec;
		symbol_optional<abstract_declarator> decl;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		symbol<default_parameter> init;
		FOREACH4(spec, decl, assign, init);
	};

	struct parameter_declaration_list
	{
		symbol<parameter_declaration> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<parameter_declaration_list> next;
		FOREACH3(item, comma, next);
	};

	struct parameter_declaration_clause
	{
		symbol_optional<parameter_declaration_list> list;
		terminal_optional<boost::wave::T_ELLIPSIS> isEllipsis;
		FOREACH2(list, isEllipsis);
	};

	struct direct_abstract_declarator_function : public direct_abstract_declarator
	{
		symbol<direct_abstract_declarator> decl;
		symbol<parameter_declaration_clause> params;
		symbol<cv_qualifier_seq> qual;
		symbol<exception_specification> except;
		FOREACH4(decl, params, qual, except);
	};

	struct direct_abstract_declarator_array : public direct_abstract_declarator
	{
		symbol<direct_abstract_declarator> decl;
		symbol<constant_expression> size;
		FOREACH2(decl, size);
	};

	struct direct_abstract_declarator_parenthesis : public direct_abstract_declarator
	{
		symbol<abstract_declarator> decl;
		FOREACH1(decl);
	};

	struct decl_specifier_default : public terminal_choice, public decl_specifier_nontype
	{
		VISITABLE_DERIVED(decl_specifier_nontype);
		enum { FRIEND, TYPEDEF } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct storage_class_specifier : public terminal_choice, public decl_specifier_nontype
	{
		VISITABLE_DERIVED(decl_specifier_nontype);
		enum { REGISTER, STATIC, EXTERN, MUTABLE } id;
		terminal_choice2 value;
		FOREACH1(value);
	};

	struct simple_type_specifier_name : public simple_type_specifier
	{
		VISITABLE_DERIVED(simple_type_specifier);
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		symbol<type_name> id;
		FOREACH3(isGlobal, context, id);
	};

	struct simple_type_specifier_template : public simple_type_specifier
	{
		VISITABLE_DERIVED(simple_type_specifier);
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol<nested_name_specifier> context;
		terminal<boost::wave::T_TEMPLATE> key;
		symbol<simple_template_id> id;
		FOREACH4(isGlobal, context, key, id);
	};

	struct simple_type_specifier_builtin : public terminal_choice, public simple_type_specifier, public decl_specifier_suffix, public type_specifier_suffix
	{
		VISITABLE_DERIVED(simple_type_specifier);
		VISITABLE_DERIVED(decl_specifier_suffix);
		VISITABLE_DERIVED(type_specifier_suffix);
		enum { CHAR, WCHAR_T, BOOL, SHORT, INT, LONG, SIGNED, UNSIGNED, FLOAT, DOUBLE, VOID, AUTO } id;
		terminal_choice2 value;
		FOREACH1(value);
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
		terminal<boost::wave::T_ASM> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<string_literal> str;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH5(key, lp, str, rp, semicolon);
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
		FOREACH1(value);
	};

	struct msext_asm_element_list
	{
		symbol<msext_asm_element> item;
		symbol_optional<msext_asm_element_list> next;
		terminal_optional<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(item, next, semicolon);
	};

	struct msext_asm_element_list_inline
	{
		symbol<msext_asm_element> item;
		symbol_optional<msext_asm_element_list_inline> next;
		terminal_optional<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(item, next, semicolon);
	};

	struct msext_asm_statement : public msext_asm_element, public statement
	{
		VISITABLE_DERIVED(msext_asm_element);
		VISITABLE_DERIVED(statement);
		terminal<boost::wave::T_MSEXT_ASM> key;
		symbol<msext_asm_element_list_inline> list;
		terminal_optional<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(key, list, semicolon);
	};

	struct msext_asm_statement_braced : public msext_asm_element, public statement
	{
		VISITABLE_DERIVED(msext_asm_element);
		VISITABLE_DERIVED(statement);
		terminal<boost::wave::T_MSEXT_ASM> key;
		terminal<boost::wave::T_LEFTBRACE> lb;
		symbol<msext_asm_element_list> list;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		terminal_optional<boost::wave::T_SEMICOLON> semicolon;
		FOREACH5(key, lb, list, rb, semicolon);
	};


	struct namespace_alias_definition : public block_declaration
	{
		VISITABLE_DERIVED(block_declaration);
		terminal<boost::wave::T_NAMESPACE> key;
		symbol<identifier> alias;
		terminal<boost::wave::T_ASSIGN> assign;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		symbol<identifier> id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH7(key, alias, assign, isGlobal, context, id, semicolon);
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
		terminal<boost::wave::T_USING> key;
		terminal<boost::wave::T_COLON_COLON> scope;
		symbol<unqualified_id> id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH4(key, scope, id, semicolon);
	};

	struct using_declaration_nested : public using_declaration
	{
		VISITABLE_DERIVED(using_declaration);
		terminal<boost::wave::T_USING> key;
		terminal_optional<boost::wave::T_TYPENAME> isTypename;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol<nested_name_specifier> context;
		symbol<unqualified_id> id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH6(key, isTypename, isGlobal, context, id, semicolon);
	};

	struct using_directive : public block_declaration
	{
		VISITABLE_DERIVED(block_declaration);
		terminal<boost::wave::T_USING> key;
		terminal<boost::wave::T_NAMESPACE> key2;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		symbol_optional<nested_name_specifier> context;
		symbol<namespace_name> id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH6(key, key2, isGlobal, context, id, semicolon);
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
		symbol<declarator> decl;
		symbol_optional<initializer> init;
		FOREACH2(decl, init);
	};

	struct init_declarator_list
	{
		symbol<init_declarator> item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		symbol<init_declarator_list> next;
		FOREACH3(item, comma, next);
	};

	struct simple_declaration_named : public general_declaration_suffix, public simple_declaration_suffix
	{
		VISITABLE_DERIVED(simple_declaration_suffix);
		VISITABLE_DERIVED(general_declaration_suffix);
		symbol<init_declarator_list> decl;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH2(decl, semicolon);
	};

	struct simple_declaration : public declaration_statement, public for_init_statement
	{
		VISITABLE_DERIVED(declaration_statement);
		VISITABLE_DERIVED(for_init_statement);
		symbol<decl_specifier_seq> spec; // 7-1: Only in function declarations for constructors, destructors, and type conversions can the decl-specifier-seq be omitted.
		symbol<simple_declaration_suffix> suffix;
		FOREACH2(spec, suffix);
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
		symbol<identifier> label;
		terminal<boost::wave::T_COLON> colon;
		symbol<statement> body;
		FOREACH3(label, colon, body);
	};

	struct labeled_statement_case : public labeled_statement
	{
		VISITABLE_DERIVED(labeled_statement);
		terminal<boost::wave::T_CASE> key;
		symbol<constant_expression> label;
		terminal<boost::wave::T_COLON> colon;
		symbol<statement> body;
		FOREACH4(key, label, colon, body);
	};

	struct labeled_statement_default : public labeled_statement
	{
		VISITABLE_DERIVED(labeled_statement);
		terminal<boost::wave::T_DEFAULT> key;
		terminal<boost::wave::T_COLON> colon;
		symbol<statement> body;
		FOREACH3(key, colon, body);
	};

	struct expression_statement : public statement, public for_init_statement
	{
		VISITABLE_DERIVED(for_init_statement);
		VISITABLE_DERIVED(statement);
		symbol_optional<expression> expr;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH2(expr, semicolon);
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
		symbol<type_specifier_seq> type;
		symbol<declarator> decl;
		terminal<boost::wave::T_ASSIGN> assign;
		symbol<assignment_expression> init;
		FOREACH4(type, decl, assign, init);
	};

	struct selection_statement_if : public selection_statement
	{
		VISITABLE_DERIVED(selection_statement);
		terminal<boost::wave::T_IF> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<condition> cond;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		symbol<statement> body;
		terminal_suffix<boost::wave::T_ELSE> key2;
		symbol<statement> fail;
		FOREACH7(key, lp, cond, rp, body, key2, fail);
	};

	struct selection_statement_switch : public selection_statement
	{
		VISITABLE_DERIVED(selection_statement);
		terminal<boost::wave::T_SWITCH> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<condition> cond;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		symbol<statement> body;
		FOREACH5(key, lp, cond, rp, body);
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
		terminal<boost::wave::T_WHILE> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<condition> cond;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		symbol<statement> body;
		FOREACH5(key, lp, cond, rp, body);
	};

	struct iteration_statement_dowhile : public iteration_statement
	{
		VISITABLE_DERIVED(iteration_statement);
		terminal<boost::wave::T_DO> key;
		symbol<statement> body;
		terminal<boost::wave::T_WHILE> key2;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<expression> cond;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH7(key, body, key2, lp, cond, rp, semicolon);
	};

	struct iteration_statement_for : public iteration_statement
	{
		VISITABLE_DERIVED(iteration_statement);
		terminal<boost::wave::T_FOR> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		symbol<for_init_statement> init;
		symbol_optional<condition> cond;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		symbol_optional<expression> incr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		symbol<statement> body;
		FOREACH8(key, lp, init, cond, semicolon, incr, rp, body);
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
		FOREACH1(value);
	};

	struct jump_statement_simple : public jump_statement
	{
		VISITABLE_DERIVED(jump_statement);
		symbol<jump_statement_key> key;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH2(key, semicolon);
	};

	struct jump_statement_return : public jump_statement
	{
		VISITABLE_DERIVED(jump_statement);
		terminal<boost::wave::T_RETURN> key;
		symbol_optional<expression> expr;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(key, expr, semicolon);
	};

	struct jump_statement_goto : public jump_statement
	{
		VISITABLE_DERIVED(jump_statement);
		terminal<boost::wave::T_GOTO> key;
		symbol<identifier> id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(key, id, semicolon);
	};

	struct try_block : public statement
	{
		VISITABLE_DERIVED(statement);
		terminal<boost::wave::T_TRY> key;
		symbol<compound_statement> body;
		symbol<handler_seq> handlers;
		FOREACH3(key, body, handlers);
	};

	struct declaration_seq
	{
		symbol<declaration> item;
		symbol_next<declaration_seq> next;
		FOREACH2(item, next);
	};

	struct template_declaration_prefix
	{
		terminal_optional<boost::wave::T_EXPORT> isExport;
		terminal<boost::wave::T_TEMPLATE> key;
		terminal<boost::wave::T_LESS> lt;
		symbol<template_parameter_list> params;
		terminal<boost::wave::T_GREATER> gt;
		FOREACH5(isExport, key, lt, params, gt);
	};

	struct template_declaration : public declaration
	{
		VISITABLE_DERIVED(declaration);
		symbol<template_declaration_prefix> prefix;
		symbol<declaration> decl;
		FOREACH2(prefix, decl);
	};

	struct member_template_declaration : public member_declaration
	{
		VISITABLE_DERIVED(member_declaration);
		symbol<template_declaration_prefix> prefix;
		symbol<member_declaration> decl;
		FOREACH2(prefix, decl);
	};

	struct explicit_instantiation : public declaration
	{
		VISITABLE_DERIVED(declaration);
		terminal_optional<boost::wave::T_EXTERN> isExtern;
		terminal<boost::wave::T_TEMPLATE> key;
		symbol<declaration> decl;
		FOREACH3(isExtern, key, decl);
	};

	struct explicit_specialization : public declaration
	{
		VISITABLE_DERIVED(declaration);
		terminal<boost::wave::T_TEMPLATE> key;
		terminal<boost::wave::T_LESS> lt;
		terminal<boost::wave::T_GREATER> gt;
		symbol<declaration> decl;
		FOREACH4(key, lt, gt, decl);
	};

	struct linkage_specification_compound : public linkage_specification_suffix
	{
		VISITABLE_DERIVED(linkage_specification_suffix);
		terminal<boost::wave::T_LEFTBRACE> lb;
		symbol_sequence<declaration_seq> decl;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH3(lb, decl, rb);
	};

	struct linkage_specification : public declaration
	{
		VISITABLE_DERIVED(declaration);
		terminal<boost::wave::T_EXTERN> key;
		symbol<string_literal> str;
		symbol<linkage_specification_suffix> suffix;
		FOREACH3(key, str, suffix);
	};

	typedef declaration_seq namespace_body;

	struct namespace_definition : public declaration
	{
		VISITABLE_DERIVED(declaration);
		terminal<boost::wave::T_NAMESPACE> key;
		symbol_optional<identifier> id;
		terminal<boost::wave::T_LEFTBRACE> lb;
		symbol_sequence<namespace_body> body;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH5(key, id, lb, body, rb);
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


