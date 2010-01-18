
#ifndef INCLUDED_CPPPARSE_CPPTREE_H
#define INCLUDED_CPPPARSE_CPPTREE_H

#include <string>

#include <boost/wave/token_ids.hpp>
typedef boost::wave::token_id LexTokenId;

#define FOREACH_SIGNATURE template<typename Func> void forEach(Func func)
#define FOREACH1(a) FOREACH_SIGNATURE { func(a); }
#define FOREACH2(a, b) FOREACH_SIGNATURE { func(a); func(b); }
#define FOREACH3(a, b, c) FOREACH_SIGNATURE { func(a); func(b); func(c); }
#define FOREACH4(a, b, c, d) FOREACH_SIGNATURE { func(a); func(b); func(c); func(d); }
#define FOREACH5(a, b, c, d, e) FOREACH_SIGNATURE { func(a); func(b); func(c); func(d); func(e); }
#define FOREACH6(a, b, c, d, e, f) FOREACH_SIGNATURE { func(a); func(b); func(c); func(d); func(e); func(f); }
#define FOREACH7(a, b, c, d, e, f, g) FOREACH_SIGNATURE { func(a); func(b); func(c); func(d); func(e); func(f); func(g); }
#define FOREACH8(a, b, c, d, e, f, g, h) FOREACH_SIGNATURE { func(a); func(b); func(c); func(d); func(e); func(f); func(g); func(h); }


template<typename VisitorType, typename T>
struct VisitorThunk
{
	static void visit(void* context, T* p)
	{
		static_cast<VisitorType*>(context)->visit(p);
	}
};

template<typename T>
struct VisitorFunc
{
	typedef T Type;
	typedef void (*Thunk)(void* context, T* p);
	Thunk thunk;
	explicit VisitorFunc(Thunk thunk)
		: thunk(thunk)
	{
	}
};

template<typename VisitorFuncTableType>
struct VisitorCallback
{
	void* context;
	VisitorFuncTableType* table;

	template<typename T>
	void visit(T* p) const
	{
		static_cast<const VisitorFunc<T>*>(table)->thunk(context, p);
	}
};

template<typename T>
struct TypeId
{
	typedef T Type;
};

struct VisitorFuncListEnd
{
	template<typename VisitorType>
	explicit VisitorFuncListEnd(const TypeId<VisitorType>&)
	{
	}
};

template<typename FuncType, typename NextType>
struct VisitorFuncList : public FuncType, public NextType
{
	template<typename VisitorType>
	explicit VisitorFuncList(const TypeId<VisitorType>& visitorType) :
	FuncType(VisitorThunk<VisitorType, typename FuncType::Type>::visit),
		NextType(visitorType)
	{
	}
};

#define VISITORFUNCLIST1(T0) VisitorFuncList<VisitorFunc<T0>, VisitorFuncListEnd>
#define VISITORFUNCLIST2(T0, T1) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST1(T1)>
#define VISITORFUNCLIST3(T0, T1, T2) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST2(T1, T2)>
#define VISITORFUNCLIST4(T0, T1, T2, T3) VisitorFuncList<VisitorFunc<T0>, VISITORFUNCLIST3(T1, T2, T3)>

#define VISITABLE_BASE(Funcs) \
	typedef Funcs VisitorFuncTable; \
	typedef VisitorCallback<VisitorFuncTable> Visitor; \
	virtual void accept(const Visitor& visitor) = 0; \
	template<typename VisitorType> \
	void accept(VisitorType& visitor) \
	{ \
		static VisitorFuncTable table = VisitorFuncTable(TypeId<VisitorType>()); \
		Visitor callback = { &visitor, &table }; \
		accept(callback); \
	}

#define VISITABLE_DERIVED(Base) \
	virtual void accept(const Base::Visitor& visitor) \
	{ \
		visitor.visit(this); \
	}

struct Choice1;
struct Choice2;

struct Choice
{
	VISITABLE_BASE(VISITORFUNCLIST2(Choice1, Choice2));
};

struct Choice1 : public Choice
{
	VISITABLE_DERIVED(Choice);
};

struct Choice2 : public Choice
{
	VISITABLE_DERIVED(Choice);
};

struct MyVisitor
{
	void visit(Choice* p)
	{
		p->accept(*this);
	}
	void visit(Choice1* p)
	{
	}
	void visit(Choice2* p)
	{
	}
};

inline void test__()
{
	MyVisitor v;
	Choice2 c;
	Choice* p = &c;
	v.visit(p);
}

#if 0
#define VISITABLE_BASE(types) \
	VISITOR_ABSTRACT(types) \
	VISITOR_GENERIC(types) \
	template<typename VisitorType> \
	void accept(VisitorType& visitor) \
{ \
	VisitorGeneric<VisitorType> tmp(visitor); \
	accept(*static_cast<VisitorAbstract*>(&tmp)); \
} \
	virtual void accept(VisitorAbstract& visitor) = 0;

#define VISITABLE_DERIVED(Base) \
	void accept(Base::VisitorAbstract& visitor) \
{ \
	visitor.visit(this); \
}

#define VISIT_ABSTRACT(Type) virtual void visit(Type* p) = 0;
#define VISITOR_ABSTRACT(types) \
	struct VisitorAbstract \
	{ \
		types(VISIT_ABSTRACT) \
	};

#define VISIT_GENERIC(Type) virtual void visit(Type* p) { visitor.visit(p); }
#define VISITOR_GENERIC(types) \
	template<typename VisitorType> \
	struct VisitorGeneric : public VisitorAbstract \
	{ \
		VisitorType& visitor; \
		VisitorGeneric(VisitorType& visitor) \
		: visitor(visitor) \
		{ \
		} \
		types(VISIT_GENERIC) \
	};
#define VISITABLE_BASE(types) \
	VISITOR_ABSTRACT(types) \
	VISITOR_GENERIC(types) \
	template<typename VisitorType> \
	void accept(VisitorType& visitor) \
	{ \
		VisitorGeneric<VisitorType> tmp(visitor); \
		accept(*static_cast<VisitorAbstract*>(&tmp)); \
	} \
	virtual void accept(VisitorAbstract& visitor) = 0;

#define VISITABLE_DERIVED(Base) \
	void accept(Base::VisitorAbstract& visitor) \
	{ \
		visitor.visit(this); \
	}
#endif


namespace cpp
{
	template<LexTokenId id>
	struct terminal
	{
		static const LexTokenId ID = id;
	};

	template<LexTokenId id>
	struct terminal_optional : public terminal<id>
	{
		bool parsed;
	};

	template<LexTokenId id>
	struct terminal_suffix : public terminal<id>
	{
	};

#if 0
	template<typename T>
	struct symbol
	{
		typedef T Type;
		T* parsed;
		T* operator->()
		{
			return parsed;
		}
	};

	template<typename T>
	struct symbol_optional : public symbol
	{
	};
#endif

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
		VISITABLE_BASE(VISITORFUNCLIST3(
			struct assignment_expression,
			struct type_id,
			struct id_expression
		));
	};

	struct template_parameter : public choice<template_parameter>
	{
		VISITABLE_BASE(VISITORFUNCLIST2(
			struct parameter_declaration,
			struct type_parameter
		));
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
		VISITABLE_DERIVED(template_argument);
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
		VISITABLE_DERIVED(template_argument);
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
		terminal<boost::wave::T_COLON_COLON> scope;
		FOREACH2(id, scope);
	};

	struct nested_name_specifier_suffix
	{
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		class_name* id;
		terminal<boost::wave::T_COLON_COLON> scope;
		nested_name_specifier_suffix* next;
		FOREACH4(isTemplate, id, scope, next);
	};

	struct nested_name_specifier
	{
		nested_name_specifier_prefix* prefix;
		nested_name_specifier_suffix* suffix;
		FOREACH2(prefix, suffix);
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
		FOREACH2(item, next);
	};

	struct decl_specifier_suffix_seq
	{
		decl_specifier_suffix* item;
		decl_specifier_suffix_seq* next;
		FOREACH2(item, next);
	};

	struct decl_specifier_seq
	{
		decl_specifier_prefix_seq* prefix;
		type_specifier_noncv* type;
		decl_specifier_suffix_seq* suffix;
		FOREACH3(prefix, type, suffix);
	};

	struct simple_type_specifier : public choice<simple_type_specifier>, public type_specifier_noncv
	{
	};

	struct template_argument_list
	{
		template_argument* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		template_argument_list* next;
		FOREACH3(item, comma, next);
	};

	struct overloadable_operator : public choice<overloadable_operator>
	{
	};

	struct array_operator : public overloadable_operator
	{
		// always '[]'
		terminal<boost::wave::T_LEFTBRACKET> ls;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		FOREACH2(ls, rs);
	};

	struct function_operator : public overloadable_operator
	{
		// always '()'
		terminal<boost::wave::T_LEFTPAREN> lp;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH2(lp, rp);
	};

	struct comma_operator : public overloadable_operator
	{
		// always ','
		terminal<boost::wave::T_COMMA> key;
		FOREACH1(key);
	};

	struct new_operator : public overloadable_operator
	{
		terminal<boost::wave::T_NEW> key;
		array_operator* array;
		FOREACH2(key, array);
	};

	struct delete_operator : public overloadable_operator
	{
		terminal<boost::wave::T_DELETE> key;
		array_operator* array;
		FOREACH2(key, array);
	};

	struct operator_function_id_suffix
	{
		terminal<boost::wave::T_LESS> lt;
		template_argument_list* args;
		terminal<boost::wave::T_GREATER> gt;
		FOREACH3(lt, args, gt);
	};

	struct operator_function_id : public unqualified_id
	{
		terminal<boost::wave::T_OPERATOR> key;
		overloadable_operator* op;
		operator_function_id_suffix* suffix;
		FOREACH3(key, op, args);
	};

	struct template_id_operator_function : public template_id
	{
		operator_function_id* id;
		terminal<boost::wave::T_LESS> lt;
		template_argument_list* args;
		terminal<boost::wave::T_GREATER> gt;
		FOREACH4(id, lt, args, gt);
	};

	struct qualified_id_default : public qualified_id
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		unqualified_id* id;
		FOREACH4(isGlobal, context, isTemplate, id);
	};

	struct qualified_id_global : public qualified_id
	{
		terminal<boost::wave::T_COLON_COLON> scope;
		identifier* id;
		FOREACH2(scope, id);
	};

	struct qualified_id_global_template : public qualified_id
	{
		terminal<boost::wave::T_COLON_COLON> scope;
		template_id* id;
		FOREACH2(scope, id);
	};

	struct qualified_id_global_op_func : public qualified_id
	{
		terminal<boost::wave::T_COLON_COLON> scope;
		operator_function_id* id;
		FOREACH2(scope, id);
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
		terminal<boost::wave::T_ENUM> key;
		FOREACH1(key);
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
		terminal_optional<boost::wave::T_VIRTUAL> isVirtual;
		FOREACH2(access, isVirtual);
	};

	struct base_specifier_virtual_access : public base_specifier_prefix
	{
		terminal<boost::wave::T_VIRTUAL> isVirtual;
		access_specifier* access; // optional
		FOREACH2(isVirtual, access);
	};

	struct base_specifier
	{
		base_specifier_prefix* prefix;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		class_name* id;
		FOREACH4(prefix, isGlobal, context, id);
	};


	struct base_specifier_list
	{
		base_specifier* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		base_specifier_list* next;
		FOREACH3(item, comma, next);
	};

	struct base_clause
	{
		terminal<boost::wave::T_COLON> colon;
		base_specifier_list* list;
		FOREACH2(colon, list);
	};

	struct class_head : public choice<class_head>
	{
	};

	struct class_head_default : public class_head
	{
		class_key* key;
		identifier* id;
		base_clause* base;
		FOREACH3(key, id, base);
	};

	struct class_head_nested : public class_head
	{
		class_key* key;
		nested_name_specifier* context;
		class_name* id;
		base_clause* base;
		FOREACH4(key, context, id, base);
	};

	struct simple_template_id : public template_id, public class_name
	{
		identifier* id;
		terminal<boost::wave::T_LESS> lb;
		template_argument_list* args;
		terminal<boost::wave::T_GREATER> rb;
		FOREACH4(id, lb, args, rb);
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
		FOREACH2(item, next);
	};

	struct ptr_operator_key : public terminal_choice
	{
		enum { PTR, REF } value;
	};

	struct ptr_operator
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		ptr_operator_key* key;
		cv_qualifier_seq* qual;
		FOREACH4(isGlobal, context, key, qual);
	};

	struct type_specifier_prefix_seq
	{
		cv_qualifier* item;
		type_specifier_prefix_seq* next;
		FOREACH2(item, next);
	};

	struct type_specifier_suffix_seq
	{
		type_specifier_suffix* item;
		type_specifier_suffix_seq* next;
		FOREACH2(item, next);
	};

	struct type_specifier_seq
	{
		type_specifier_prefix_seq* prefix;
		type_specifier_noncv* type;
		type_specifier_suffix_seq* suffix;
		FOREACH3(prefix, type, suffix);
	};

	struct abstract_declarator : public choice<abstract_declarator>, public exception_declarator
	{
	};

	struct abstract_declarator_ptr : public abstract_declarator
	{
		ptr_operator* op;
		abstract_declarator* decl;
		FOREACH2(op, decl);
	};

	struct abstract_declarator_parenthesis : public abstract_declarator
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		abstract_declarator* decl;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, decl, rp);
	};

	struct declarator_suffix_seq;

	struct direct_abstract_declarator : public abstract_declarator
	{
		abstract_declarator_parenthesis* prefix;
		declarator_suffix_seq* suffix;
		FOREACH2(prefix, suffix);
	};

	struct type_id : public template_argument
	{
		type_specifier_seq* spec;
		abstract_declarator* decl;
		FOREACH2(spec, decl);
		VISITABLE_DERIVED(template_argument);
	};

	struct throw_expression : public assignment_expression
	{
		terminal<boost::wave::T_THROW> key;
		assignment_expression* expr;
		FOREACH2(key, expr);
	};

	struct expression_comma : public expression
	{
		assignment_expression* left;
		terminal_suffix<boost::wave::T_COMMA> comma;
		expression_comma* right;
		FOREACH3(left, comma, right);
	};

	struct member_initializer : public choice<member_initializer>
	{
	};

	struct constant_initializer : public member_initializer
	{
		terminal<boost::wave::T_ASSIGN> assign;
		constant_expression* expr;
		FOREACH2(assign, expr);
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
		FOREACH2(value, next);
	};

	struct primary_expression_builtin : public primary_expression
	{
		// always 'this'
		terminal<boost::wave::T_THIS> key;
	};

	struct primary_expression_parenthesis : public primary_expression
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression* expr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, expr, rp);
	};

	struct initializer_list
	{
		initializer_clause* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		initializer_list* next;
		FOREACH3(item, comma, next);
	};

	struct initializer_clause_list : public initializer_clause
	{
		terminal<boost::wave::T_LEFTBRACE> lb;
		initializer_list* list;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH3(lb, list, rb);
	};

	struct initializer_clause_empty : public initializer_clause
	{
		terminal<boost::wave::T_LEFTBRACE> lb;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH2(lb, rb);
	};

	struct expression_list : public initializer
	{
		assignment_expression* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		expression_list* next;
		FOREACH3(item, comma, next);
	};

	struct initializer_default : public initializer
	{
		terminal<boost::wave::T_ASSIGN> assign;
		initializer_clause* clause;
		FOREACH2(assign, clause);
	};

	struct initializer_parenthesis : public initializer
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression_list* list;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, list, rp);
	};

	struct postfix_expression_suffix : public choice<postfix_expression_suffix>
	{
	};

	struct postfix_expression_suffix_seq
	{
		postfix_expression_suffix* item;
		postfix_expression_suffix_seq* next;
		FOREACH2(item, next);
	};

	struct postfix_expression_default : public postfix_expression
	{
		postfix_expression_prefix* expr;
		postfix_expression_suffix_seq* suffix;
		FOREACH2(expr, suffix);
	};

	struct postfix_expression_index : public postfix_expression_suffix
	{
		terminal<boost::wave::T_LEFTBRACKET> ls;
		expression* index;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		FOREACH3(ls, index, rs);
	};

	struct postfix_expression_call : public postfix_expression_suffix
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression_list* args;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, args, rp);
	};

	struct member_operator : public terminal_choice, public overloadable_operator
	{
		enum { DOT, ARROW } value;
	};

	struct postfix_expression_member : public postfix_expression_suffix
	{
		member_operator* op;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		id_expression* id;
		FOREACH3(op, isTemplate, id);
	};

	struct postfix_expression_destructor : public postfix_expression_suffix
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		terminal<boost::wave::T_COMPL> compl;
		type_name* type;
		FOREACH4(isGlobal, context, compl, type);
	};

	struct postfix_operator : public terminal_choice, public postfix_expression_suffix, public overloadable_operator
	{
		enum { PLUSPLUS, MINUSMINUS } value;
	};

	struct postfix_expression_construct : public postfix_expression_prefix
	{
		simple_type_specifier* type;
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression_list* args;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(type, lp, args, rp);
	};

	struct cast_operator : public terminal_choice
	{
		enum { DYNAMIC, STATIC, REINTERPRET, CONST } value;
	};

	struct postfix_expression_cast : public postfix_expression_prefix
	{
		cast_operator* op;
		terminal<boost::wave::T_LESS> lt;
		type_id* type;
		terminal<boost::wave::T_GREATER> gt;
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression* expr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH7(op, lt, type, gt, lp, expr, rp);
	};

	struct postfix_expression_typeid : public postfix_expression_prefix
	{
		terminal<boost::wave::T_TYPEID> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression* expr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(key, lp, expr, rp);
	};

	struct postfix_expression_typeidtype : public postfix_expression_prefix
	{
		terminal<boost::wave::T_TYPEID> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		type_id* type;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(key, lp, type, rp);
	};

	struct new_type : public choice<new_type>
	{
	};

	struct new_declarator : public choice<new_declarator>
	{
	};

	struct new_declarator_suffix
	{
		terminal<boost::wave::T_LEFTBRACKET> ls;
		constant_expression* expr;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		new_declarator_suffix* next;
		FOREACH4(ls, expr, rs, next);
	};

	struct direct_new_declarator : public new_declarator
	{
		terminal<boost::wave::T_LEFTBRACKET> ls;
		expression* expr;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		new_declarator_suffix* suffix;
		FOREACH4(ls, expr, rs, suffix);
	};

	struct new_declarator_ptr : public new_declarator
	{
		ptr_operator* op;
		new_declarator* decl;
		FOREACH2(op, decl);
	};

	struct new_type_default : public new_type
	{
		type_specifier_seq* spec;
		new_declarator* decl;
		FOREACH2(spec, decl);
	};

	struct new_type_parenthesis : public new_type
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		type_id* id;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, id, rp);
	};

	struct new_initializer
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression_list* expr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, expr, rp);
	};

	struct new_expression : public choice<new_expression>, public unary_expression
	{
	};

	struct new_expression_placement : public new_expression
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		terminal<boost::wave::T_NEW> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression_list* place;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		new_type* type;
		new_initializer* init;
		FOREACH7(isGlobal, key, lp, place, rp, type, init);
	};

	struct new_expression_default : public new_expression
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		terminal<boost::wave::T_NEW> key;
		new_type* type;
		new_initializer* init;
		FOREACH4(isGlobal, key, type, init);
	};

	struct delete_expression : public unary_expression
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		terminal<boost::wave::T_DELETE> key;
		array_operator* op;
		cast_expression* expr;
		FOREACH4(isGlobal, key, op, expr);
	};

	struct unary_operator : public terminal_choice, public unary_expression, public overloadable_operator
	{
		enum { PLUSPLUS, MINUSMINUS, STAR, AND, PLUS, MINUS, NOT, COMPL } value;
	};

	struct unary_expression_op : public unary_expression
	{
		unary_operator* op;
		cast_expression* expr;
		FOREACH2(op, expr);
	};

	struct unary_expression_sizeof : public unary_expression
	{
		terminal<boost::wave::T_SIZEOF> key;
		unary_expression* expr;
		FOREACH2(key, expr);
	};

	struct unary_expression_sizeoftype : public unary_expression
	{
		terminal<boost::wave::T_SIZEOF> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		type_id* type;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(key, lp, type, rp);
	};

	struct cast_expression_default : public cast_expression
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		type_id* id;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		cast_expression* expr;
		FOREACH4(lp, id, rp, expr);
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
		FOREACH3(left, op, right);
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
		FOREACH3(left, op, right);
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
		FOREACH3(left, op, right);
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
		FOREACH3(left, op, right);
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
		FOREACH3(left, op, right);
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
		FOREACH3(left, op, right);
	};

	struct and_expression_default : public and_expression
	{
		equality_expression* left;
		terminal_suffix<boost::wave::T_AND> op;
		and_expression* right;
		FOREACH3(left, op, right);
	};

	struct exclusive_or_expression_default : public exclusive_or_expression
	{
		and_expression* left;
		terminal_suffix<boost::wave::T_XOR> op;
		exclusive_or_expression* right;
		FOREACH3(left, op, right);
	};

	struct inclusive_or_expression_default : public inclusive_or_expression
	{
		exclusive_or_expression* left;
		terminal_suffix<boost::wave::T_OR> op;
		inclusive_or_expression* right;
		FOREACH3(left, op, right);
	};

	struct logical_and_expression_default : public logical_and_expression
	{
		inclusive_or_expression* left;
		terminal_suffix<boost::wave::T_ANDAND> op;
		logical_and_expression* right;
		FOREACH3(left, op, right);
	};

	struct logical_or_expression_default : public logical_or_expression
	{
		logical_and_expression* left;
		terminal_suffix<boost::wave::T_OROR> op;
		logical_or_expression* right;
		FOREACH3(left, op, right);
	};

	struct logical_or_expression_suffix : public choice<logical_or_expression_suffix>
	{
	};

	struct conditional_expression_rhs : public logical_or_expression_suffix
	{
		terminal<boost::wave::T_QUESTION_MARK> key;
		expression* mid;
		terminal<boost::wave::T_COLON> colon;
		assignment_expression* right;
		FOREACH4(key, mid, colon, right);
	};

	struct conditional_expression_default : public conditional_expression
	{
		logical_or_expression* left;
		conditional_expression_rhs* right;
		FOREACH2(left, right);
	};

	struct logical_or_expression_precedent : public conditional_expression
	{
		logical_or_expression* left;
		logical_or_expression_suffix* right;
		FOREACH2(left, right);
	};

	struct assignment_operator : public terminal_choice, public overloadable_operator
	{
		enum { ASSIGN, STAR, DIVIDE, PERCENT, PLUS, MINUS, SHIFTRIGHT, SHIFTLEFT, AND, XOR, OR } value;
	};

	struct assignment_expression_rhs : public logical_or_expression_suffix
	{
		assignment_operator* op;
		assignment_expression* right;
		FOREACH2(op, right);
	};




	struct conversion_declarator
	{
		ptr_operator* op;
		conversion_declarator* decl;
		FOREACH2(op, decl);
	};

	struct conversion_function_id : public unqualified_id
	{
		terminal<boost::wave::T_OPERATOR> key;
		type_specifier_seq* spec;
		conversion_declarator* decl;
		FOREACH3(key, spec, decl);
	};

	struct destructor_id : public unqualified_id 
	{
		terminal<boost::wave::T_COMPL> compl;
		class_name* name;
		FOREACH2(compl, name);
	};

	struct parameter_declaration_clause;

	struct exception_type_list : public choice<exception_type_list>
	{
	};

	struct exception_type_all : public exception_type_list
	{
		terminal<boost::wave::T_ELLIPSIS> key;
		FOREACH1(key);
	};

	struct type_id_list : public exception_type_list
	{
		type_id* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		type_id_list* next;
		FOREACH3(item, comma, next);
	};

	struct exception_specification
	{
		terminal<boost::wave::T_THROW> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		exception_type_list* types;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH4(key, lp, types, rp);
	};

	struct declarator_suffix : public choice<declarator_suffix>
	{
	};

	struct declarator_suffix_function : public declarator_suffix
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		parameter_declaration_clause* params;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		cv_qualifier_seq* qual;
		exception_specification* except;
		FOREACH5(lp, params, rp, qual, except);
	};

	struct declarator_suffix_array : public declarator_suffix
	{
		terminal<boost::wave::T_LEFTBRACKET> ls;
		constant_expression* size;
		terminal<boost::wave::T_RIGHTBRACKET> rs;
		FOREACH3(ls, size, rs);
	};

	struct declarator_suffix_seq : public declarator_suffix
	{
		declarator_suffix* item;
		declarator_suffix_seq* next;
		FOREACH2(item, next);
	};

	struct direct_declarator : public declarator
	{
		direct_declarator_prefix* prefix;
		declarator_suffix_seq* suffix;
		FOREACH2(prefix, suffix);
	};

	struct direct_declarator_parenthesis : public direct_declarator_prefix
	{
		terminal<boost::wave::T_LEFTPAREN> lp;
		declarator* decl;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH3(lp, decl, rp);
	};

	struct declarator_ptr : public declarator
	{
		ptr_operator* op;
		declarator* decl;
		FOREACH2(op, decl);
	};

	struct statement : public choice<statement>
	{
	};

	struct statement_seq
	{
		statement* item;
		statement_seq* next;
		FOREACH2(item, next);
	};

	struct function_body : public choice<function_body>
	{
	};

	struct compound_statement : public statement, public function_body
	{
		terminal<boost::wave::T_LEFTBRACE> lb;
		statement_seq* body;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH3(lb, body, rb);
	};

	struct exception_declaration : public choice<exception_declaration>
	{
	};

	struct exception_declaration_default : public exception_declaration
	{
		type_specifier_seq* type;
		exception_declarator* decl;
		FOREACH2(type, decl);
	};

	struct exception_declaration_all : public exception_declaration
	{
		terminal<boost::wave::T_ELLIPSIS> key;
		FOREACH1(key);
	};

	struct handler_seq
	{
		terminal<boost::wave::T_CATCH> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		exception_declaration* decl;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		compound_statement* body;
		handler_seq* next;
		FOREACH6(key, lp, decl, rp, body, next);
	};

	struct linkage_specification_suffix : public choice<linkage_specification_suffix>
	{
	};

	struct declaration : public choice<declaration>, public linkage_specification_suffix
	{
	};

	struct mem_initializer
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		class_name* id;
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression_list* args;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		FOREACH6(isGlobal, context, id, lp, args, rp);
	};

	struct mem_initializer_list
	{
		mem_initializer* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		mem_initializer_list* next;
		FOREACH3(item, comma, next);
	};

	struct ctor_initializer
	{
		terminal<boost::wave::T_COLON> colon;
		mem_initializer_list* list;
		FOREACH2(colon, list);
	};

	struct general_declaration_suffix : public choice<general_declaration_suffix>
	{
	};

	struct general_declaration : public declaration
	{
		decl_specifier_seq* spec;
		declarator* decl;
		general_declaration_suffix* suffix;
		FOREACH3(spec, decl, suffix);
	};

	struct member_declaration_suffix : public choice<member_declaration_suffix>
	{
	};

	struct function_definition_suffix : public general_declaration_suffix, public member_declaration_suffix
	{
		function_body* body;
		handler_seq* handlers;
		FOREACH2(body, handlers);
	};

	struct function_definition : public declaration
	{
		decl_specifier_seq* spec;
		declarator* decl;
		function_definition_suffix* suffix;
		FOREACH3(spec, decl, suffix);
	};


	struct member_declarator : public choice<member_declarator>
	{
	};

	struct pure_specifier : public member_initializer
	{
		// always '= 0'
		terminal<boost::wave::T_ASSIGN> assign;
		terminal<boost::wave::T_DECIMALINT> zero; // TODO: check value is zero
		FOREACH2(assign, zero);
	};

	struct member_declarator_default : public member_declarator
	{
		declarator* decl;
		member_initializer* init;
		FOREACH2(decl, init);
	};

	struct member_declarator_bitfield : public member_declarator
	{
		identifier* id;
		terminal<boost::wave::T_COLON> colon;
		constant_expression* width;
		FOREACH3(id, colon, width);
	};

	struct member_declarator_list
	{
		member_declarator* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		member_declarator_list* next;
		FOREACH3(item, comma, next);
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
		FOREACH2(spec, decl);
	};

	struct member_declaration_general_bitfield : public member_declaration_general
	{
		member_declarator_bitfield* item;
		terminal_optional<boost::wave::T_COMMA> comma;
		member_declarator_list* next;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH4(item, comma, next, semicolon);
	};

	struct member_declaration_general_default : public member_declaration_general
	{
		declarator* decl;
		member_declaration_suffix* suffix;
		FOREACH2(decl, suffix);
	};

	struct member_declaration_suffix_default : public member_declaration_suffix
	{
		member_initializer* init;
		terminal_optional<boost::wave::T_COMMA> comma;
		member_declarator_list* next;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH4(init, comma, next, semicolon);
	};

	struct member_declaration_nested : public member_declaration
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		unqualified_id* id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH5(isGlobal, context, isTemplate, id, semicolon);
	};

	struct function_specifier : public terminal_choice, public decl_specifier_nontype
	{
		enum { INLINE, VIRTUAL, EXPLICIT } value;
	};

	struct ctor_specifier_seq
	{
		function_specifier* item;
		ctor_specifier_seq* next;
		FOREACH2(item, next);
	};

	struct constructor_definition : public declaration
	{
		ctor_specifier_seq* spec;
		declarator* decl;
		ctor_initializer* init;
		function_body* body;
		handler_seq* handlers;
		FOREACH5(spec, decl, init, body, handlers);
	};

	struct member_declaration_inline : public member_declaration
	{
		constructor_definition* func;
		terminal_optional<boost::wave::T_SEMICOLON> semicolon;
		FOREACH2(func, semicolon);
	};

	struct member_declaration_ctor : public member_declaration
	{
		ctor_specifier_seq* spec;
		declarator* decl;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(spec, decl, semicolon);
	};

	struct member_specification : public choice<member_specification>
	{
	};

	struct member_specification_list : public member_specification
	{
		member_declaration* item;
		member_specification* next;
		FOREACH2(item, next);
	};

	struct member_specification_access : public member_specification
	{
		access_specifier* access;
		terminal<boost::wave::T_COLON> colon;
		member_specification* next;
		FOREACH3(access, colon, next);
	};

	struct class_specifier : public type_specifier_noncv
	{
		class_head* head;
		terminal<boost::wave::T_LEFTBRACE> lb;
		member_specification* members;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH4(head, lb, members, rb);
	};

	struct enumerator_definition
	{
		identifier* id;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		constant_expression* init;
		FOREACH3(id, assign, init);
	};

	struct enumerator_list
	{
		enumerator_definition* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		enumerator_list* next;
		FOREACH3(item, comma, next);
	};

	struct enum_specifier : public type_specifier_noncv
	{
		terminal<boost::wave::T_ENUM> key;
		identifier* id; // may be empty
		terminal<boost::wave::T_LEFTBRACE> lb;
		enumerator_list* values;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH5(key, id, lp, values, rb);
	};

	struct elaborated_type_specifier : public choice<elaborated_type_specifier>, public type_specifier_noncv
	{
	};

	struct elaborated_type_specifier_default : public elaborated_type_specifier
	{
		elaborated_type_specifier_key* key;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		identifier* id;
		FOREACH4(key, isGlobal, context, id);
	};

	struct elaborated_type_specifier_template : public elaborated_type_specifier
	{
		class_key* key;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		simple_template_id* id;
		FOREACH5(key, isGlobal, context, isTemplate, id);
	};

	struct typename_specifier : public type_specifier_noncv
	{
		terminal<boost::wave::T_TYPENAME> key;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		terminal_optional<boost::wave::T_TEMPLATE> isTemplate;
		type_name* id; // NOTE: only 'identifier' is allowed if 'isTemplate' is true
		FOREACH5(key, isGlobal, context, isTemplate, id);
	};

	struct parameter_declaration : public choice<parameter_declaration>, public template_parameter
	{
		VISITABLE_DERIVED(template_parameter);
	};

	struct type_parameter : public choice<type_parameter>, public template_parameter
	{
		VISITABLE_DERIVED(template_parameter);
	};

	struct type_parameter_key : public terminal_choice
	{
		enum { CLASS, TYPENAME } value;
	};

	struct type_parameter_default : public type_parameter
	{
		type_parameter_key* key;
		identifier* id;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		type_id* init;
		FOREACH4(key, id, assign, init);
	};

	struct template_parameter_list
	{
		template_parameter* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		template_parameter_list* next;
		FOREACH3(item, comma, next);
	};

	struct type_parameter_template : public type_parameter
	{
		terminal<boost::wave::T_TEMPLATE> key;
		terminal<boost::wave::T_LESS> lt;
		template_parameter_list* params;
		terminal<boost::wave::T_GREATER> gt;
		terminal<boost::wave::T_CLASS> key2;
		identifier* id;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		id_expression* init;
		FOREACH8(key, lt, params, gt, key2, id, assign, init);
	};

	struct parameter_declaration_default : public parameter_declaration
	{
		decl_specifier_seq* spec;
		declarator* decl;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		assignment_expression* init;
		FOREACH4(spec, decl, assign, init);
	};

	struct parameter_declaration_abstract : public parameter_declaration
	{
		decl_specifier_seq* spec;
		abstract_declarator* decl;
		terminal_suffix<boost::wave::T_ASSIGN> assign;
		assignment_expression* init;
		FOREACH4(spec, decl, assign, init);
	};

	struct parameter_declaration_list
	{
		parameter_declaration* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		parameter_declaration_list* next;
		FOREACH3(item, comma, next);
	};

	struct parameter_declaration_clause
	{
		terminal_optional<boost::wave::T_ELLIPSIS> isEllipsis;
		parameter_declaration_list* list;
		FOREACH2(isEllipsis, list);
	};

	struct direct_abstract_declarator_function : public direct_abstract_declarator
	{
		direct_abstract_declarator* decl;
		parameter_declaration_clause* params;
		cv_qualifier_seq* qual;
		exception_specification* except;
		FOREACH4(decl, params, qual, except);
	};

	struct direct_abstract_declarator_array : public direct_abstract_declarator
	{
		direct_abstract_declarator* decl;
		constant_expression* size;
		FOREACH2(decl, size);
	};

	struct direct_abstract_declarator_parenthesis : public direct_abstract_declarator
	{
		abstract_declarator* decl;
		FOREACH1(decl);
	};

	struct template_id_operator : public template_id
	{
		operator_function_id* id;
		template_argument_list* args;
		FOREACH2(id, args);
	};

	struct decl_specifier_default : public terminal_choice, public decl_specifier_nontype
	{
		enum { FRIEND, TYPEDEF } value;
	};

	struct storage_class_specifier : public terminal_choice, public decl_specifier_nontype
	{
		enum { REGISTER, STATIC, EXTERN, MUTABLE } value;
	};

	struct simple_type_specifier_name : public simple_type_specifier
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		type_name* id;
		FOREACH3(isGlobal, context, id);
	};

	struct simple_type_specifier_template : public simple_type_specifier
	{
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		terminal<boost::wave::T_TEMPLATE> key;
		simple_template_id* id;
		FOREACH4(isGlobal, context, key, id);
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
		terminal<boost::wave::T_ASM> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		string_literal* str;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH5(key, lp, str, rp, semicolon);
	};

	struct msext_asm_statement : public statement
	{
		terminal<boost::wave::T_MSEXT_ASM> key;
		msext_asm_statement* inner;
		terminal_optional<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(key, inner, semicolon);
	};


	struct namespace_alias_definition : public block_declaration
	{
		terminal<boost::wave::T_NAMESPACE> key;
		identifier* alias;
		terminal<boost::wave::T_ASSIGN> assign;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		identifier* id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH7(key, alias, assign, isGlobal, context, id, semicolon);
	};

	struct using_declaration : public choice<using_declaration>, public block_declaration, public member_declaration
	{
	};

	struct using_declaration_global : public using_declaration
	{
		terminal<boost::wave::T_USING> key;
		terminal<boost::wave::T_COLON_COLON> scope;
		unqualified_id* id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH4(key, scope, id, semicolon);
	};

	struct using_declaration_nested : public using_declaration
	{
		terminal<boost::wave::T_USING> key;
		terminal_optional<boost::wave::T_TYPENAME> isTypename;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		unqualified_id* id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH6(key, isTypename, isGlobal, context, id, semicolon);
	};

	struct using_directive : public block_declaration
	{
		terminal<boost::wave::T_USING> key;
		terminal<boost::wave::T_NAMESPACE> key2;
		terminal_optional<boost::wave::T_COLON_COLON> isGlobal;
		nested_name_specifier* context;
		identifier* id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH6(key, key2, isGlobal, context, id, semicolon);
	};

	struct for_init_statement : public choice<for_init_statement>
	{
	};

	struct init_declarator
	{
		declarator* decl;
		initializer* init;
		FOREACH2(decl, init);
	};

	struct init_declarator_list
	{
		init_declarator* item;
		terminal_suffix<boost::wave::T_COMMA> comma;
		init_declarator_list* next;
		FOREACH3(item, comma, next);
	};

	struct simple_declaration_suffix : public general_declaration_suffix
	{
		initializer* init;
		terminal_optional<boost::wave::T_COMMA> comma;
		init_declarator_list* next;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(init, next, semicolon);
	};

	struct simple_declaration : public block_declaration, public for_init_statement
	{
		decl_specifier_seq* spec;
		declarator* decl;
		simple_declaration_suffix* suffix;
		FOREACH3(spec, decl, suffix);
	};

	struct labeled_statement : public choice<labeled_statement>, public statement
	{
	};

	struct labeled_statement_id : public labeled_statement
	{
		identifier* label;
		terminal<boost::wave::T_COLON> colon;
		statement* body;
		FOREACH3(label, colon, body);
	};

	struct labeled_statement_case : public labeled_statement
	{
		terminal<boost::wave::T_CASE> key;
		constant_expression* label;
		terminal<boost::wave::T_COLON> colon;
		statement* body;
		FOREACH4(key, label, colon, body);
	};

	struct labeled_statement_default : public labeled_statement
	{
		terminal<boost::wave::T_DEFAULT> key;
		terminal<boost::wave::T_COLON> colon;
		statement* body;
		FOREACH3(key, colon, body);
	};

	struct expression_statement : public statement, public for_init_statement
	{
		expression* expr;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH2(expr, semicolon);
	};

	struct selection_statement : public choice<selection_statement>, public statement
	{
	};

	struct condition_init : public condition
	{
		type_specifier_seq* type;
		declarator* decl;
		terminal<boost::wave::T_ASSIGN> assign;
		assignment_expression* init;
		FOREACH4(type, decl, assign, init);
	};

	struct selection_statement_if : public selection_statement
	{
		terminal<boost::wave::T_IF> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		condition* cond;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		statement* body;
		terminal_suffix<boost::wave::T_ELSE> key2;
		statement* fail;
		FOREACH7(key, lp, cond, rp, body, key2, fail);
	};

	struct selection_statement_switch : public selection_statement
	{
		terminal<boost::wave::T_SWITCH> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		condition* cond;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		statement* body;
		FOREACH5(key, lp, cond, rp, body);
	};

	struct iteration_statement : public choice<iteration_statement>, public statement
	{
	};

	struct iteration_statement_while : public iteration_statement
	{
		terminal<boost::wave::T_WHILE> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		condition* cond;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		statement* body;
		FOREACH5(key, lp, cond, rp, body);
	};

	struct iteration_statement_dowhile : public iteration_statement
	{
		terminal<boost::wave::T_DO> key;
		statement* body;
		terminal<boost::wave::T_WHILE> key2;
		terminal<boost::wave::T_LEFTPAREN> lp;
		expression* cond;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH7(key, body, key2, lp, cond, rp, semicolon);
	};

	struct iteration_statement_for : public iteration_statement
	{
		terminal<boost::wave::T_FOR> key;
		terminal<boost::wave::T_LEFTPAREN> lp;
		for_init_statement* init;
		condition* cond;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		expression* incr;
		terminal<boost::wave::T_RIGHTPAREN> rp;
		statement* body;
		FOREACH8(key, lp, init, cond, semicolon, incr, rp, body);
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
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH2(key, semicolon);
	};

	struct jump_statement_return : public jump_statement
	{
		terminal<boost::wave::T_RETURN> key;
		expression* expr;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(key, expr, semicolon);
	};

	struct jump_statement_goto : public jump_statement
	{
		terminal<boost::wave::T_GOTO> key;
		identifier* id;
		terminal<boost::wave::T_SEMICOLON> semicolon;
		FOREACH3(key, id, semicolon);
	};

	struct try_block : public statement
	{
		terminal<boost::wave::T_TRY> key;
		compound_statement* body;
		handler_seq* handlers;
		FOREACH3(key, body, handlers);
	};

	struct declaration_seq
	{
		declaration* item;
		declaration_seq* next;
		FOREACH2(item, next);
	};

	struct template_declaration : public declaration, public member_declaration
	{
		terminal_optional<boost::wave::T_EXPORT> isExport;
		terminal<boost::wave::T_TEMPLATE> key;
		terminal<boost::wave::T_LESS> lt;
		template_parameter_list* params;
		terminal<boost::wave::T_GREATER> gt;
		declaration* decl;
		FOREACH6(isExport, key, lt, params, gt, decl);
	};

	struct explicit_instantiation : public declaration
	{
		terminal_optional<boost::wave::T_EXTERN> isExtern;
		terminal<boost::wave::T_TEMPLATE> key;
		declaration* decl;
		FOREACH3(isExtern, key, decl);
	};

	struct explicit_specialization : public declaration
	{
		terminal<boost::wave::T_TEMPLATE> key;
		terminal<boost::wave::T_LESS> lt;
		terminal<boost::wave::T_GREATER> gt;
		declaration* decl;
		FOREACH4(key, lt, gt, decl);
	};

	struct linkage_specification_compound : public linkage_specification_suffix
	{
		terminal<boost::wave::T_LEFTBRACE> lb;
		declaration_seq* decl;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH3(lb, decl, rb);
	};

	struct linkage_specification : public declaration
	{
		terminal<boost::wave::T_EXTERN> key;
		string_literal* str;
		linkage_specification_suffix* suffix;
		FOREACH3(key, str, suffix);
	};

	typedef declaration_seq namespace_body;

	struct namespace_definition : public declaration
	{
		terminal<boost::wave::T_NAMESPACE> key;
		identifier* id;
		terminal<boost::wave::T_LEFTBRACE> lb;
		namespace_body* body;
		terminal<boost::wave::T_RIGHTBRACE> rb;
		FOREACH5(key, id, lb, body, rb);
	};
}

#endif


