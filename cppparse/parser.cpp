
#include "parser.h"


// matches type-specifier: 'A', 'A::B', 'A<B>', 'A::X<Y>'
// ambiguous with id-expression
inline bool isAmbiguousTypeSpecifier(cpp::type_specifier_noncv* type)
{
	cpp::simple_type_specifier_name* name = dynamic_cast<cpp::simple_type_specifier_name*>(type);
	if(name == 0)
	{
		return false;
	}
	return true;
}

// matches type-specifier-seq: 'A', 'A::B'
// ambiguous with id-expression
inline bool isAmbiguousTypeSpecifierSeq(cpp::type_specifier_seq* spec)
{
	// 'A'
	if(spec == 0
		|| spec->prefix != 0
		|| spec->suffix != 0
		|| spec->type == 0
		|| !isAmbiguousTypeSpecifier(spec->type))
	{
		return false;
	}
	return true;
}

// matches decl-specifier-seq: 'A', 'A::B'
// ambiguous with id-expression
inline bool isAmbiguousDeclSpecifierSeq(cpp::decl_specifier_seq* spec)
{
	// 'A'
	if(spec == 0
		|| spec->prefix != 0
		|| spec->suffix != 0
		|| spec->type == 0
		|| !isAmbiguousTypeSpecifier(spec->type))
	{
		return false;
	}
	return true;
}

inline bool isAmbiguousAbstractDeclarator(cpp::abstract_declarator* symbol);

inline bool isAmbiguousParameterDeclaration(cpp::parameter_declaration_default* symbol);

// matches declarator-suffix: (x), [1], (x = 1)
// ambiguous with postfix-expression-suffix function-call/cast or array-index
// does not match: (A x), (A(B x))
inline bool isAmbiguousDeclaratorSuffix(cpp::declarator_suffix* symbol)
{
	cpp::declarator_suffix_function* function = dynamic_cast<cpp::declarator_suffix_function*>(symbol);
	if(function == 0)
	{
		cpp::declarator_suffix_array* array = dynamic_cast<cpp::declarator_suffix_array*>(symbol);
		if(array == 0
			|| array->size == 0)
		{
			return false;
		}
		return true;
	}
	if(function->params != 0)
	{
		if(function->params->isEllipsis.value != 0)
		{
			return false;
		}
		for(cpp::parameter_declaration_list* p = function->params->list; p != 0; p = p->next)
		{
			cpp::ambiguity<cpp::parameter_declaration>* ambig = dynamic_cast<cpp::ambiguity<cpp::parameter_declaration>*>(p->item.p);
			if(ambig == 0)
			{
				if(!isAmbiguousParameterDeclaration(dynamic_cast<cpp::parameter_declaration_default*>(p->item.p)))
				{
					cpp::parameter_declaration_abstract* param = dynamic_cast<cpp::parameter_declaration_abstract*>(p->item.p);
					if(param == 0
						|| param->spec == 0
						|| !isAmbiguousDeclSpecifierSeq(param->spec)
						|| !isAmbiguousAbstractDeclarator(param->decl))
					{
						return false;
					}
				}
			}
		}
	}


	return true;
}

// matches abstract-declarator: '', '(x)', '[1]', '(x)[1]', '[1](x)'
inline bool isAmbiguousAbstractDeclarator(cpp::abstract_declarator* symbol)
{
	if(symbol == 0)
	{
		// empty abstract-declarator is ambiguous!
		return true;
	}
	cpp::direct_abstract_declarator* decl = dynamic_cast<cpp::direct_abstract_declarator*>(symbol);
	if(decl == 0
		|| decl->prefix != 0)
	{
		return false;
	}
	for(cpp::declarator_suffix_seq* p = decl->suffix; p != 0; p = p->next)
	{
		if(!isAmbiguousDeclaratorSuffix(p->item))
		{
			return false;
		}
	}
	return true;
}


inline bool isAmbiguousDeclarator(cpp::declarator* decl);

// matches direct-declarator prefix: 'A', '(A)', '(*A)'
// ambiguous with primary-expression
inline bool isAmbiguousDirectDeclaratorPrefix(cpp::direct_declarator_prefix* prefix)
{
	cpp::direct_declarator_parenthesis* paren = dynamic_cast<cpp::direct_declarator_parenthesis*>(prefix);
	if(paren != 0)
	{
		return isAmbiguousDeclarator(paren->decl);
	}
	cpp::declarator_id* id = dynamic_cast<cpp::declarator_id*>(prefix);
	if(id == 0)
	{
		return false;
	}
	return true;
}

// matches direct-declarator: 'A', 'A()', 'A[1]', 'A(B())', 'A(B[1])'
// ambiguous with id-expression
inline bool isAmbiguousDirectDeclarator(cpp::declarator* decl)
{
	cpp::direct_declarator* direct = dynamic_cast<cpp::direct_declarator*>(decl);
	if(direct == 0
		|| !isAmbiguousDirectDeclaratorPrefix(direct->prefix))
	{
		return false;
	}
	for(cpp::declarator_suffix_seq* p = direct->suffix; p != 0; p = p->next)
	{
		if(!isAmbiguousDeclaratorSuffix(p->item))
		{
			return false;
		}
	}
	return true;
}

// matches declarator: 'A', '*A', '&*A', '(A)'
// ambiguous with expression
inline bool isAmbiguousDeclarator(cpp::declarator* decl)
{
	cpp::declarator_ptr* ptr = dynamic_cast<cpp::declarator_ptr*>(decl);
	if(ptr != 0)
	{
		return isAmbiguousDeclarator(ptr->decl);
	}
	return isAmbiguousDirectDeclarator(decl);
}

// matches parenthesised direct-declarator: '(A)', '(*A)', '(&*A)', '(A)()', '(A)[1]'
// ambiguous with parenthesised expression
inline bool isAmbiguousParenthesisedDeclarator(cpp::declarator* decl)
{
	cpp::direct_declarator* direct = dynamic_cast<cpp::direct_declarator*>(decl);
	if(direct == 0)
	{
		return false;
	}
	cpp::direct_declarator_parenthesis* paren = dynamic_cast<cpp::direct_declarator_parenthesis*>(direct->prefix.p);
	if(paren == 0
		|| !isAmbiguousDeclarator(paren->decl))
	{
		return false;
	}
	return true;
}

// matches initializer: '= expr'
// ambiguous with assignment-expression RHS
inline bool isAmbiguousInitializer(cpp::initializer* init)
{
	cpp::initializer_parenthesis* paren = dynamic_cast<cpp::initializer_parenthesis*>(init);
	if(paren == 0)
	{
		// '='
		cpp::initializer_default* def = dynamic_cast<cpp::initializer_default*>(init);
		if(def == 0)
		{
			return false;
		}
		// 'expr'
		cpp::assignment_expression* clause = dynamic_cast<cpp::assignment_expression*>(def->clause.p);
		if(clause == 0)
		{
			return false;
		}
	}
	return true;
}

// matches init-declarator: 'A', 'A = expr', '*A', 'A()', 'A(B())'
// ambiguous with assignment-expression
inline bool isAmbiguousInitDeclarator(cpp::init_declarator* init)
{
	// 'A'
	if(!isAmbiguousDeclarator(init->decl))
	{
		return false;
	}
	if(init->init != 0
		&& !isAmbiguousInitializer(init->init))
	{
		return false;
	}
	return true;
}

// matches simple-declaration suffix
// ';'
// ', C;'
// ', C = 0;'
// ', *C;'
// ', *C;'
// ', *C(X);'
// ', *C[X];'
inline bool isAmbiguousSimpleDeclarationSuffix(cpp::simple_declaration_suffix* suffix)
{
	if(suffix != 0)
	{
		if(suffix->init != 0
			&& !isAmbiguousInitializer(suffix->init))
		{
			return false;
		}
		for(cpp::init_declarator_list* p = suffix->next; p != 0; p = p->next)
		{
			if(!isAmbiguousInitDeclarator(p->item))
			{
				return false;
			}
		}
	}
	return true;
}


// matches simple-declaration with redundantly-parenthesised/omitted declarator: 'A;', 'A(X);', 'A(*X);', 'X<Y>::A(X);', 'A::B(X);'
// ambiguous with expression-statement
// does not match 'A X;'
// NOTE: the form 'A<B>::C;' could be relational-expression '(A < B) > ::C;', primary-expression 'A<B>::C' or declaration 'A<B>::C'
inline bool isAmbiguousVariableDeclaration(cpp::simple_declaration* simple)
{
	// 'A'
	if(simple == 0
		|| !isAmbiguousDeclSpecifierSeq(simple->spec))
	{
		return false;
	}
	cpp::general_declaration_type* type = dynamic_cast<cpp::general_declaration_type*>(simple->affix.p);
	if(type == 0)
	{
		// '(X)'
		cpp::simple_declaration_named* named = dynamic_cast<cpp::simple_declaration_named*>(simple->affix.p);
		if(named == 0)
		{
			return false;
		}
		if(!isAmbiguousParenthesisedDeclarator(named->decl)
			|| !isAmbiguousSimpleDeclarationSuffix(named->suffix))
		{
			return false;
		}
	}
	return true;
}

template<typename T>
inline bool isAmbiguousVariableDeclaration(T* statement)
{
	return isAmbiguousVariableDeclaration(dynamic_cast<cpp::simple_declaration*>(statement));
}

// matches redundantly parenthesised variable-declaration: 'A(X)'
// ambiguous with parameter-declaration 'unnamed function returning A with param X'
inline bool isAmbiguousParameterDeclaration(cpp::parameter_declaration_default* symbol)
{
	// 'A'
	if(symbol == 0
		|| !isAmbiguousDeclSpecifierSeq(symbol->spec))
	{
		return false;
	}
	// '(X)'
	if(!isAmbiguousParenthesisedDeclarator(symbol->decl))
	{
		return false;
	}
	return true;
}

inline bool isAmbiguous(cpp::parameter_declaration* symbol)
{
	return isAmbiguousParameterDeclaration(dynamic_cast<cpp::parameter_declaration_default*>(symbol));
}

// DEPRECATED: omission of decl-specifier-seq is not allowed in simple-declaration in C++
// matches redundantly parenthesised implicit-int init-declaration '(A)(X)'
// ambiguity: '(A)(X);' could be cast-expression '(A)X;', or function-call 'A(X);'
// TODO: '(A)(X())'
// TODO: '(A)(X[1])'
// TODO: '(A)(X), (B)(X)'
inline bool isAmbiguousInitDeclaration(cpp::statement* declaration)
{
	// '(A)'
	cpp::simple_declaration* simple = dynamic_cast<cpp::simple_declaration*>(declaration);
	if(simple == 0
		|| simple->spec != 0)
	{
		return false;
	}
	cpp::simple_declaration_named* named = dynamic_cast<cpp::simple_declaration_named*>(simple->affix.p);
	if(named == 0
		|| named->suffix == 0
		|| named->suffix->init == 0)
	{
		return false;
	}
	if(!isAmbiguousParenthesisedDeclarator(named->decl))
	{
		return false;
	}
	// '(X)'
	cpp::initializer_parenthesis* init = dynamic_cast<cpp::initializer_parenthesis*>(named->suffix->init.p);
	if(init == 0
		|| init->list->item.p == 0)
	{
		return false;
	}
	return true;
}

// matches ptr-declaration 'A * B ;': type-id ptr-operator declarator-id ;
// 'A * B(X);' multiplicative-expression with function-call/cast RHS vs function-declaration 
// 'A * B[0];' multiplicative-expression with array-index RHS vs array-declaration
// 'A * B, C;' multiplicative-expression vs declarator-list
// 'A * B = 0, C = 0;'
// 'A * *B;'
// 'A * B, *C;'
// 'A * *B, *C;'
// 'A * B(X), *C(X);'
// 'A * B[X], *C[X];'
inline bool isAmbiguousPtrDeclaration(cpp::simple_declaration* simple)
{
	if(simple == 0
		|| !isAmbiguousDeclSpecifierSeq(simple->spec))
	{
		return false;
	}
	cpp::simple_declaration_named* named = dynamic_cast<cpp::simple_declaration_named*>(simple->affix.p);
	if(named == 0)
	{
		return false;
	}
	cpp::declarator_ptr* ptr = dynamic_cast<cpp::declarator_ptr*>(named->decl.p);
	if(ptr == 0
		|| !isAmbiguousDeclarator(ptr->decl)
		|| !isAmbiguousSimpleDeclarationSuffix(named->suffix))
	{
		return false;
	}
	return true;
}

template<typename T>
inline bool isAmbiguousPtrDeclaration(T* statement)
{
	return isAmbiguousPtrDeclaration(dynamic_cast<cpp::simple_declaration*>(statement));
}


// matches type-id: A, A(x), A(B(x)), A[1], A()[1]
// ambiguous with expression
// TODO: check suffix:  A(B(x))
inline bool isAmbiguousTypeId(cpp::type_id* type)
{
	if(!isAmbiguousTypeSpecifierSeq(type->spec)
		|| !isAmbiguousAbstractDeclarator(type->decl))
	{
		return false;
	}
	return true;
}

inline bool isAmbiguousTypeId(cpp::template_argument* symbol)
{
	cpp::type_id* type = dynamic_cast<cpp::type_id*>(symbol);
	if(type == 0)
	{
		return false;
	}
	return isAmbiguousTypeId(type);
}

inline bool isAmbiguous(cpp::template_argument* symbol)
{
	return isAmbiguousTypeId(symbol);
}

// matches: 'B, ', 'B && D'
// ambiguous with assignment-expression
inline bool isAmbiguousTemplateArgument(cpp::template_argument* symbol)
{
	cpp::type_id* type = dynamic_cast<cpp::type_id*>(symbol);
	if(type != 0)
	{
		if(!isAmbiguousTypeId(type))
		{
			return false;
		}
	}
	return true;
}

// matches: 'B, ', 'B && D', 'B, D'
// ambiguous with expression-list
inline bool isAmbiguousTemplateArgumentList(cpp::template_argument_list* symbol)
{
	if(!isAmbiguousTemplateArgument(symbol->item))
	{
		return false;
	}
	if(symbol->next != 0)
	{
		if(!isAmbiguousTemplateArgumentList(symbol->next))
		{
			return false;
		}
	}
	return true;
}

// matches: 'A<B>', 'A<B && D>', 'A<B, D>'
// ambiguous with nested/parenthesised relational-expression lhs
inline bool isAmbiguousTemplateId(cpp::simple_template_id* symbol)
{
	if(symbol == 0
		|| !isAmbiguousTemplateArgumentList(symbol->args))
	{
		return false;
	}
	return true;
}

// matches decl-specifier-seq: 'A<B>', 'N::A<B>'
// ambiguous with relational-expression LHS
inline bool isAmbiguousTemplateIdDeclSpecifierSeq(cpp::decl_specifier_seq* spec)
{
	// 'A'
	if(spec == 0
		|| spec->prefix != 0
		|| spec->suffix != 0
		|| spec->type == 0)
	{
		return false;
	}
	cpp::simple_type_specifier_name* name = dynamic_cast<cpp::simple_type_specifier_name*>(spec->type.p);
	if(name == 0)
	{
		return false;
	}
	cpp::simple_template_id* id = dynamic_cast<cpp::simple_template_id*>(name->id.p);
	if(id == 0
		|| !isAmbiguousTemplateId(id))
	{
		return false;
	}
	return true;
}

// matches template-typed variable-declaration: 'A<B> X', 'A<B> *X', 'A<B> X, Y'
// ambiguous with relational-expression
inline bool isAmbiguousTemplateIdVariableDeclaration(cpp::simple_declaration* simple)
{
	if(simple == 0
		|| !isAmbiguousTemplateIdDeclSpecifierSeq(simple->spec))
	{
		return false;
	}
	cpp::simple_declaration_named* named = dynamic_cast<cpp::simple_declaration_named*>(simple->affix.p);
	if(named == 0)
	{
		return false;
	}
	if(!isAmbiguousDeclarator(named->decl)
		|| !isAmbiguousSimpleDeclarationSuffix(named->suffix))
	{
		return false;
	}
	return true;
}

template<typename T>
inline bool isAmbiguousTemplateIdVariableDeclaration(T* symbol)
{
	return isAmbiguousTemplateIdVariableDeclaration(dynamic_cast<cpp::simple_declaration*>(symbol));
}


inline bool isAmbiguous(cpp::statement* statement)
{
	return isAmbiguousPtrDeclaration(statement)
		|| isAmbiguousVariableDeclaration(statement)
		|| isAmbiguousTemplateIdVariableDeclaration(statement);
}

inline bool isAmbiguous(cpp::for_init_statement* statement)
{
	return isAmbiguousPtrDeclaration(statement)
		|| isAmbiguousVariableDeclaration(statement)
		|| isAmbiguousTemplateIdVariableDeclaration(statement);
}

// matches sizeof(type-id): sizeof(A), sizeof(A()), sizeof(A(B()))
// ambiguous with sizeof parenthesised-expression
inline bool isAmbiguousSizeofType(cpp::unary_expression* symbol)
{
	cpp::unary_expression_sizeoftype* expr = dynamic_cast<cpp::unary_expression_sizeoftype*>(symbol);
	if(expr == 0
		|| !isAmbiguousTypeId(expr->type))
	{
		return false;
	}
	return true;
}

// matches 'A(X);', 'A(X());', 'A(X[1]);'
// ambiguous with redundantly parenthesised member-declaration 'A X;', 'A X();', 'A X[1];'
inline bool isAmbiguousConstructor(cpp::member_declaration* symbol)
{
	cpp::member_declaration_implicit* impl = dynamic_cast<cpp::member_declaration_implicit*>(symbol);
	if(impl == 0
		|| impl->spec != 0
		|| !isAmbiguousDirectDeclarator(impl->decl->decl.p))
	{
		return false;
	}
	return true;
}


// matches '~A', '~A<X>'
inline bool isAmbiguousDestructorId(cpp::unary_expression* symbol)
{
	cpp::destructor_id* id = dynamic_cast<cpp::destructor_id*>(symbol);
	if(id == 0)
	{
		return false;
	}
	return true;
}

// matches '~A()', '~A<X>()'
inline bool isAmbiguousDestructorCall(cpp::unary_expression* symbol)
{
	cpp::postfix_expression_default* postfix = dynamic_cast<cpp::postfix_expression_default*>(symbol);
	if(postfix == 0
		|| postfix->right == 0)
	{
		return false;
	}
	cpp::postfix_expression_call* call = dynamic_cast<cpp::postfix_expression_call*>(postfix->right->item.p);
	if(call == 0)
	{
		return false;
	}
	return isAmbiguousDestructorId(postfix->left);
}

inline bool isAmbiguous(cpp::member_declaration* symbol)
{
	return isAmbiguousConstructor(symbol);
}

inline bool isAmbiguous(cpp::unary_expression* symbol)
{
	return isAmbiguousSizeofType(symbol)
		|| isAmbiguousDestructorId(symbol)
		|| isAmbiguousDestructorCall(symbol);
}


// matches: 'class A', 'class A = B', 'class A = B()', 'class A = B[1]'
inline bool isAmbiguousTypeParameter(cpp::template_parameter* symbol)
{
	cpp::type_parameter_default* type = dynamic_cast<cpp::type_parameter_default*>(symbol);
	if(type == 0)
	{
		return false;
	}
	if(type->init != 0)
	{
		if(!isAmbiguousTypeId(type->init)) // type-parameter-default looks like assignment-expression
		{
			return false;
		}
	}
	return true;
}


inline bool isAmbiguous(cpp::template_parameter* symbol)
{
	return isAmbiguousTypeParameter(symbol);
}

// matches expression '(A)', '(A, B)', '(A())', '(A[1])'
// ambiguous with function-call postfix
// also matches '(A).member', '(A)[index]', '(A)(args)'
inline bool isAmbiguousParenthesisedExpression(cpp::cast_expression* symbol)
{
	// '(A)'
	cpp::primary_expression_parenthesis* paren = dynamic_cast<cpp::primary_expression_parenthesis*>(symbol);
	if(paren == 0)
	{
		cpp::ambiguity<cpp::cast_expression>* ambig = dynamic_cast<cpp::ambiguity<cpp::cast_expression>*>(symbol);
		if(ambig != 0)
		{
			return true;
		}
		cpp::postfix_expression_default* postfix = dynamic_cast<cpp::postfix_expression_default*>(symbol);
		if(postfix == 0)
		{
			return false;
		}
		cpp::primary_expression_parenthesis* paren = dynamic_cast<cpp::primary_expression_parenthesis*>(postfix->left.p);
		if(paren == 0)
		{
			return false;
		}
	}
	return true;
}

// matches cast-expression: '(A)(X)', (A)(X())', '(A)(X[1])', '(A())(X)', '(A[1])(X)', '(A)(X, Y)'
// ambiguity with function-call/cast
// does not match: '(A)(B)(X)'
// also matches cast expression with unary-expression RHS: '(A) ++(X)'
// ambiguity with function-call (A)++ (X)
inline bool isAmbiguousCastExpression(cpp::cast_expression_default* symbol)
{
	if(symbol == 0
		|| !isAmbiguousTypeId(symbol->id))
	{
		return false;
	}
	cpp::cast_expression* rhs = symbol->expr;
	cpp::unary_expression_op* op = dynamic_cast<cpp::unary_expression_op*>(rhs);
	if(op != 0)
	{
		rhs = op->expr;
	}
	if(!isAmbiguousParenthesisedExpression(rhs))
	{
		return false;
	}
	return true;
}

inline bool isAmbiguous(cpp::cast_expression* symbol)
{
	return isAmbiguousCastExpression(dynamic_cast<cpp::cast_expression_default*>(symbol));
}



// matches type-name: 'A<B>'
// ambiguous with relational-expression prefix
inline bool isAmbiguousTemplateId(cpp::type_name* type)
{
	cpp::simple_template_id* id = dynamic_cast<cpp::simple_template_id*>(type);
	if(id == 0
		|| !isAmbiguousTemplateId(id))
	{
		return false;
	}
	return true;
}

// matches primary-expression prefix: '', 'A<B>::', 'A<B>::C<D>::'
// ambiguous with nested relational-expression
// does not match 'A<B>::C::'
inline bool isAmbiguousTemplateIdPrefix(cpp::nested_name_specifier_suffix* symbol)
{
	if(symbol != 0)
	{
		if(symbol->isTemplate.value != 0
			|| !isAmbiguousTemplateId(symbol->id)
			|| !isAmbiguousTemplateIdPrefix(symbol->next))
		{
			return false;
		}
	}
	return true;
}

// matches primary-expression prefix: 'A<B>::', 'A<B>::C<D>::'
// ambiguous with nested relational-expression
// does not match 'A<B>::C::'
inline bool isAmbiguousTemplateIdPrefix(cpp::nested_name_specifier* symbol)
{
	if(symbol == 0
		|| !isAmbiguousTemplateId(symbol->prefix->id)
		|| !isAmbiguousTemplateIdPrefix(symbol->suffix))
	{
		return false;
	}
	return true;
}

// matches primary-expression: 'A<B>::C', '::A<B>::C'
// ambiguous with nested relational-expression
// does not match 'A<B>::template C', 'A<B>::C<D>'
inline bool isAmbiguousTemplateIdPrefix(cpp::qualified_id_default* symbol)
{
	if(symbol == 0
		|| !isAmbiguousTemplateIdPrefix(symbol->context))
	{
		return false;
	}
	cpp::identifier* id = dynamic_cast<cpp::identifier*>(symbol->id.p);
	if(id == 0)
	{
		return false;
	}
	return true;
}

// matches: 'A<B>(C)', 'A<B && D>(C)', 'A<B>(C())'
// ambiguous with nested/parenthesised relational-expression
inline bool isAmbiguousTemplateIdPrefix(cpp::postfix_expression_construct* symbol)
{
	if(symbol == 0
		|| symbol->args == 0)
	{
		return false;
	}
	cpp::simple_type_specifier_name* name = dynamic_cast<cpp::simple_type_specifier_name*>(symbol->type.p);
	if(name == 0
		|| !isAmbiguousTemplateId(dynamic_cast<cpp::simple_template_id*>(name->id.p)))
	{
		return false;
	}
	return true;
}

template<typename T>
inline bool isAmbiguousTemplateIdPrefixExpr(T* symbol)
{
	if(symbol == 0
		|| !( isAmbiguousTemplateIdPrefix(dynamic_cast<cpp::qualified_id_default*>(symbol->left.p))
			|| isAmbiguousTemplateId(dynamic_cast<cpp::simple_template_id*>(symbol->left.p))
			|| isAmbiguousTemplateIdPrefix(dynamic_cast<cpp::postfix_expression_construct*>(symbol->left.p))))
	{
		return false;
	}
	return true;
}

template<typename T>
inline bool isAmbiguousTemplateIdPrefix(T* symbol)
{
	return isAmbiguousTemplateIdPrefix(dynamic_cast<cpp::qualified_id_default*>(symbol))
		|| isAmbiguousTemplateIdPrefix(dynamic_cast<cpp::postfix_expression_construct*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::conditional_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::logical_or_expression_precedent*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::logical_or_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::logical_and_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::inclusive_or_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::exclusive_or_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::and_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::equality_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::relational_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::shift_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::additive_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::multiplicative_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::pm_expression_default*>(symbol))
		|| isAmbiguousTemplateIdPrefixExpr(dynamic_cast<cpp::postfix_expression_default*>(symbol));
}

inline bool isAmbiguous(cpp::expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::assignment_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::conditional_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::logical_or_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::logical_and_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::inclusive_or_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::exclusive_or_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::and_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::equality_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::relational_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::shift_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::additive_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::multiplicative_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}

inline bool isAmbiguous(cpp::pm_expression* symbol)
{
	return isAmbiguousTemplateIdPrefix(symbol);
}







inline cpp::identifier* parseSymbol(Parser& parser, cpp::identifier* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_IDENTIFIER))
	{
		result->value.value = parser.get_value();
		parser.increment();
		return result;
	}
	return NULL;
}

inline cpp::declaration_seq* parseSymbol(Parser& parser, cpp::declaration_seq* result);

inline cpp::namespace_definition* parseSymbol(Parser& parser, cpp::namespace_definition* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_OPTIONAL(parser, result->id);
	PARSE_TERMINAL(parser, result->lb);
	PARSE_OPTIONAL(parser, result->body);
	PARSE_TERMINAL(parser, result->rb);
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
	return result;
}

inline cpp::template_argument_list* parseSymbol(Parser& parser, cpp::template_argument_list* result)
{
	parser.inTemplateArgumentList = true;
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

inline cpp::simple_template_id* parseSymbol(Parser& parser, cpp::simple_template_id* result)
{
	if(parser.ambiguity != 0
		&& peekTemplateIdAmbiguity(parser)
		&& parser.ambiguity->nextDepth())
	{
		return 0;
	}
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->lb);
	PARSE_REQUIRED(parser, result->args);
	PARSE_TERMINAL(parser, result->rb);
	return result;
}

inline cpp::nested_name_specifier_prefix* parseSymbol(Parser& parser, cpp::nested_name_specifier_prefix* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->scope);
	return result;
}

inline cpp::nested_name_specifier_suffix* parseSymbol(Parser& parser, cpp::nested_name_specifier_suffix* result)
{
	PARSE_TERMINAL(parser, result->isTemplate);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->scope);
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
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::simple_type_specifier_template* parseSymbol(Parser& parser, cpp::simple_type_specifier_template* result)
{
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TERMINAL(parser, result->key);
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
	PARSE_TERMINAL(parser, result->assign);
	PARSE_REQUIRED(parser, result->init);
	return result;
}

inline cpp::type_parameter_template* parseSymbol(Parser& parser, cpp::type_parameter_template* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lt);
	PARSE_REQUIRED(parser, result->params);
	PARSE_TERMINAL(parser, result->gt);
	PARSE_TERMINAL(parser, result->key2);
	PARSE_OPTIONAL(parser, result->id);
	PARSE_TERMINAL(parser, result->assign);
	PARSE_REQUIRED(parser, result->init);
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
	PARSE_SELECT(parser, cpp::type_parameter); // TODO: ambiguity 'class C' could be elaborated-type-specifier or type-parameter
	PARSE_SELECT(parser, cpp::parameter_declaration);
	return result;
}

inline cpp::template_parameter_list* parseSymbol(Parser& parser, cpp::template_parameter_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

inline cpp::template_declaration* parseSymbol(Parser& parser, cpp::template_declaration* result)
{
	PARSE_TERMINAL(parser, result->isExport);
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lt);
	PARSE_REQUIRED(parser, result->params);
	PARSE_TERMINAL(parser, result->gt);
	return result;
}

inline cpp::constant_expression* parseSymbol(Parser& parser, cpp::constant_expression* result)
{
	PARSE_SELECT(parser, cpp::conditional_expression);
	return result;
}

inline cpp::pure_specifier* parseSymbol(Parser& parser, cpp::pure_specifier* result)
{
	PARSE_TERMINAL(parser, result->assign);
	PARSE_TERMINAL(parser, result->zero); // TODO: check value is zero
	return result;
}

inline cpp::constant_initializer* parseSymbol(Parser& parser, cpp::constant_initializer* result)
{
	PARSE_TERMINAL(parser, result->assign);
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
	PARSE_TERMINAL(parser, result->colon);
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
	PARSE_TERMINAL(parser, result->comma);
	if(result->comma.value != 0)
	{
		PARSE_REQUIRED(parser, result->next);
	}
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::member_declaration_general_default* parseSymbol(Parser& parser, cpp::member_declaration_general_default* result)
{
	PARSE_REQUIRED(parser, result->decl);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::member_declaration_general* parseSymbol(Parser& parser, cpp::member_declaration_general* result)
{
	PARSE_SELECT(parser, cpp::member_declaration_general_bitfield);
	PARSE_SELECT(parser, cpp::member_declaration_general_default);
	PARSE_SELECT(parser, cpp::member_declaration_suffix_default);
	return result;
}

inline cpp::member_declarator_list* parseSymbol(Parser& parser, cpp::member_declarator_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

inline cpp::member_declaration_suffix_default* parseSymbol(Parser& parser, cpp::member_declaration_suffix_default* result)
{
	PARSE_OPTIONAL(parser, result->init);
	PARSE_TERMINAL(parser, result->comma);
	if(result->comma.value != 0)
	{
		PARSE_REQUIRED(parser, result->next);
	}
	PARSE_TERMINAL(parser, result->semicolon);
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
	PARSE_REQUIRED(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::member_declaration_nested* parseSymbol(Parser& parser, cpp::member_declaration_nested* result)
{
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TERMINAL(parser, result->isTemplate);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::function_specifier_seq* parseSymbol(Parser& parser, cpp::function_specifier_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::mem_initializer* parseSymbol(Parser& parser, cpp::mem_initializer* result)
{
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

inline cpp::mem_initializer_list* parseSymbol(Parser& parser, cpp::mem_initializer_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

inline cpp::ctor_initializer* parseSymbol(Parser& parser, cpp::ctor_initializer* result)
{
	PARSE_TERMINAL(parser, result->colon);
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
	PARSE_TERMINAL(parser, result->key);
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
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TERMINAL(parser, result->rp);
	PARSE_REQUIRED(parser, result->body);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::constructor_definition* parseSymbol(Parser& parser, cpp::constructor_definition* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::member_declaration_implicit* parseSymbol(Parser& parser, cpp::member_declaration_implicit* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::member_declaration* parseSymbol(Parser& parser, cpp::member_declaration* result)
{
	PARSE_SELECT(parser, cpp::using_declaration);
	PARSE_SELECT(parser, cpp::template_declaration);
	PARSE_SELECT(parser, cpp::member_declaration_implicit); // TODO: ambiguity:  this matches a constructor: "Class(Type);"
	PARSE_SELECT(parser, cpp::member_declaration_default); // .. this matches a member: "Type(member);"
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
	PARSE_TERMINAL(parser, result->colon);
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
	PARSE_TERMINAL(parser, result->isVirtual);
	return result;
}

inline cpp::base_specifier_virtual_access* parseSymbol(Parser& parser, cpp::base_specifier_virtual_access* result)
{
	PARSE_TERMINAL(parser, result->isVirtual);
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
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::base_specifier_list* parseSymbol(Parser& parser, cpp::base_specifier_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

inline cpp::base_clause* parseSymbol(Parser& parser, cpp::base_clause* result)
{
	PARSE_TERMINAL(parser, result->colon);
	PARSE_REQUIRED(parser, result->list);
	return result;
}

inline cpp::class_head_anonymous* parseSymbol(Parser& parser, cpp::class_head_anonymous* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_OPTIONAL(parser, result->base);
	return result;
}

inline cpp::class_head_default* parseSymbol(Parser& parser, cpp::class_head_default* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_OPTIONAL(parser, result->base);
	return result;
}

inline cpp::class_head* parseSymbol(Parser& parser, cpp::class_head* result)
{
	PARSE_SELECT(parser, cpp::class_head_default); // TODO: shared prefix ambiguity: both start with 'class-key'
	PARSE_SELECT(parser, cpp::class_head_anonymous);
	return result;
}

inline cpp::class_specifier* parseSymbol(Parser& parser, cpp::class_specifier* result)
{
	PARSE_REQUIRED(parser, result->head);
	PARSE_TERMINAL(parser, result->lb);
	PARSE_OPTIONAL(parser, result->members);
	PARSE_TERMINAL(parser, result->rb);
	return result;
}

inline cpp::enumerator_definition* parseSymbol(Parser& parser, cpp::enumerator_definition* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->assign);
	PARSE_REQUIRED(parser, result->init);
	return result;
}

inline cpp::enumerator_list* parseSymbol(Parser& parser, cpp::enumerator_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_OPTIONAL(parser, result->next); // optional because trailing-comma is allowed
	return result;
}

inline cpp::enum_specifier* parseSymbol(Parser& parser, cpp::enum_specifier* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_OPTIONAL(parser, result->id);
	PARSE_TERMINAL(parser, result->lb);
	PARSE_OPTIONAL(parser, result->values);
	PARSE_TERMINAL(parser, result->rb);
	return result;
}

inline cpp::enum_key* parseSymbol(Parser& parser, cpp::enum_key* result)
{
	PARSE_TERMINAL(parser, result->key);
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
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::elaborated_type_specifier_template* parseSymbol(Parser& parser, cpp::elaborated_type_specifier_template* result)
{
	PARSE_REQUIRED(parser, result->key);
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_TERMINAL(parser, result->isTemplate);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::elaborated_type_specifier* parseSymbol(Parser& parser, cpp::elaborated_type_specifier* result)
{
	PARSE_SELECT(parser, cpp::elaborated_type_specifier_template); // TODO: shared-prefix ambiguity: match 'simple-template-id' before 'identifier'
	PARSE_SELECT(parser, cpp::elaborated_type_specifier_default);
	return result;
}

inline cpp::typename_specifier* parseSymbol(Parser& parser, cpp::typename_specifier* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context); // TODO: check standard to see if this should be optional or required
	PARSE_TERMINAL(parser, result->isTemplate);
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
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->key); // TODO: disallow '&' following 'nested-name-specifier'
	PARSE_OPTIONAL(parser, result->qual);
	return result;
}

inline cpp::declarator_suffix_array* parseSymbol(Parser& parser, cpp::declarator_suffix_array* result)
{
	PARSE_TERMINAL(parser, result->ls);
	PARSE_OPTIONAL(parser, result->size);
	PARSE_TERMINAL(parser, result->rs);
	return result;
}

inline cpp::declarator* parseSymbol(Parser& parser, cpp::declarator* result);

inline cpp::throw_expression* parseSymbol(Parser& parser, cpp::throw_expression* result)
{
	PARSE_TERMINAL(parser, result->key);
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
		result->value.value = parser.get_value();
		parser.increment();
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
	PARSE_TERMINAL(parser, result->key);
	return result;
}

inline cpp::assignment_expression* parseSymbol(Parser& parser, cpp::assignment_expression* result);

inline cpp::expression* pruneSymbol(cpp::expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::expression_default* parseSymbol(Parser& parser, cpp::expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::expression* parseSymbol(Parser& parser, cpp::expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::expression_default);
	return result;
}

inline cpp::primary_expression_parenthesis* parseSymbol(Parser& parser, cpp::primary_expression_parenthesis* result)
{
	parser.inTemplateArgumentList = false;
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TERMINAL(parser, result->rp);
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
	PARSE_TERMINAL(parser, result->ls);
	PARSE_REQUIRED(parser, result->index);
	PARSE_TERMINAL(parser, result->rs);
	return result;
}

inline cpp::postfix_expression_call* parseSymbol(Parser& parser, cpp::postfix_expression_call* result)
{
	PARSE_TERMINAL(parser, result->lp);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TERMINAL(parser, result->rp);
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
	PARSE_TERMINAL(parser, result->isTemplate);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::postfix_expression_destructor* parseSymbol(Parser& parser, cpp::postfix_expression_destructor* result)
{
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_TERMINAL(parser, result->compl);
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
	PARSE_TERMINAL(parser, result->lp);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TERMINAL(parser, result->rp);
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
	PARSE_TERMINAL(parser, result->lt);
	PARSE_REQUIRED(parser, result->type);
	PARSE_TERMINAL(parser, result->gt);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

inline cpp::postfix_expression_typeid* parseSymbol(Parser& parser, cpp::postfix_expression_typeid* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

inline cpp::postfix_expression_typeidtype* parseSymbol(Parser& parser, cpp::postfix_expression_typeidtype* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->type);
	PARSE_TERMINAL(parser, result->rp);
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

cpp::postfix_expression* pruneSymbol(cpp::postfix_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::postfix_expression_default* parseSymbol(Parser& parser, cpp::postfix_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->right);
	return result;
}

inline cpp::postfix_expression* parseSymbol(Parser& parser, cpp::postfix_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::postfix_expression_default);
	return result;
}

inline cpp::unary_expression_sizeoftype* parseSymbol(Parser& parser, cpp::unary_expression_sizeoftype* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->type);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

inline cpp::unary_expression_sizeof* parseSymbol(Parser& parser, cpp::unary_expression_sizeof* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::new_declarator_suffix* parseSymbol(Parser& parser, cpp::new_declarator_suffix* result)
{
	PARSE_TERMINAL(parser, result->ls);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TERMINAL(parser, result->rs);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::direct_new_declarator* parseSymbol(Parser& parser, cpp::direct_new_declarator* result)
{
	PARSE_TERMINAL(parser, result->ls);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TERMINAL(parser, result->rs);
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
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->rp);
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
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->expr);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

inline cpp::new_expression_placement* parseSymbol(Parser& parser, cpp::new_expression_placement* result)
{
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->place);
	PARSE_TERMINAL(parser, result->rp);
	PARSE_REQUIRED(parser, result->type);
	PARSE_OPTIONAL(parser, result->init);
	return result;
}

inline cpp::new_expression_default* parseSymbol(Parser& parser, cpp::new_expression_default* result)
{
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_TERMINAL(parser, result->key);
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
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_TERMINAL(parser, result->key);
	PARSE_OPTIONAL(parser, result->op);
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
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->rp);
	PARSE_REQUIRED(parser, result->expr);
	return result;
}

inline cpp::cast_expression* parseSymbol(Parser& parser, cpp::cast_expression* result)
{
	PARSE_SELECT(parser, cpp::cast_expression_default); // ambiguity: '(A)(X)' could be cast-expression '(A)X' or function-call 'A(X)'
	PARSE_SELECT(parser, cpp::unary_expression);
	return result;
}

inline cpp::pm_operator* parseSymbol(Parser& parser, cpp::pm_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DOTSTAR, cpp::pm_operator::DOTSTAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_ARROWSTAR, cpp::pm_operator::ARROWSTAR);
	return result;
}

cpp::pm_expression* pruneSymbol(cpp::pm_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::pm_expression_default* parseSymbol(Parser& parser, cpp::pm_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::pm_expression* parseSymbol(Parser& parser, cpp::pm_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::pm_expression_default);
	return result;
}

inline cpp::multiplicative_operator* parseSymbol(Parser& parser, cpp::multiplicative_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_STAR, cpp::multiplicative_operator::STAR);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_DIVIDE, cpp::multiplicative_operator::DIVIDE);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PERCENT, cpp::multiplicative_operator::PERCENT);
	return result;
}

cpp::multiplicative_expression* pruneSymbol(cpp::multiplicative_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::multiplicative_expression_default* parseSymbol(Parser& parser, cpp::multiplicative_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::multiplicative_expression* parseSymbol(Parser& parser, cpp::multiplicative_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::multiplicative_expression_default);
	return result;
}

inline cpp::additive_operator* parseSymbol(Parser& parser, cpp::additive_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_PLUS, cpp::additive_operator::PLUS);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_MINUS, cpp::additive_operator::MINUS);
	return result;
}

cpp::additive_expression* pruneSymbol(cpp::additive_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::additive_expression_default* parseSymbol(Parser& parser, cpp::additive_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::additive_expression* parseSymbol(Parser& parser, cpp::additive_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::additive_expression_default);
	return result;
}

inline cpp::shift_operator* parseSymbol(Parser& parser, cpp::shift_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTLEFT, cpp::shift_operator::SHIFTLEFT);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_SHIFTRIGHT, cpp::shift_operator::SHIFTRIGHT);
	return result;
}

cpp::shift_expression* pruneSymbol(cpp::shift_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::shift_expression_default* parseSymbol(Parser& parser, cpp::shift_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::shift_expression* parseSymbol(Parser& parser, cpp::shift_expression* result)
{
	PARSE_EXPRESSION(parser, cpp::shift_expression_default);
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

cpp::relational_expression* pruneSymbol(cpp::relational_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::relational_expression_default* parseSymbol(Parser& parser, cpp::relational_expression_default* result)
{
	bool ignoreRelationalLess = parser.ambiguity != 0
		&& peekTemplateIdAmbiguity(parser)
		&& !parser.ambiguity->nextDepth();

	PARSE_REQUIRED(parser, result->left);
	if(!(ignoreRelationalLess
			&& TOKEN_EQUAL(parser, boost::wave::T_LESS))) // '<' begins template-argument-list
	{
		if(!(parser.inTemplateArgumentList
				&& TOKEN_EQUAL(parser, boost::wave::T_GREATER))) // '>' terminates template-argument-list
		{
			PARSE_OPTIONAL(parser, result->op);
			if(result->op != NULL)
			{
				PARSE_REQUIRED(parser, result->right);
			}
		}
	}
	return result;
}

inline cpp::relational_expression* parseSymbol(Parser& parser, cpp::relational_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::relational_expression_default);
	return result;
}

inline cpp::equality_operator* parseSymbol(Parser& parser, cpp::equality_operator* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_EQUAL, cpp::equality_operator::EQUAL);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_NOTEQUAL, cpp::equality_operator::NOTEQUAL);
	return result;
}

cpp::equality_expression* pruneSymbol(cpp::equality_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::equality_expression_default* parseSymbol(Parser& parser, cpp::equality_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_OPTIONAL(parser, result->op);
	if(result->op != NULL)
	{
		PARSE_REQUIRED(parser, result->right);
	}
	return result;
}

inline cpp::equality_expression* parseSymbol(Parser& parser, cpp::equality_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::equality_expression_default);
	return result;
}

cpp::and_expression* pruneSymbol(cpp::and_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::and_expression_default* parseSymbol(Parser& parser, cpp::and_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::and_expression* parseSymbol(Parser& parser, cpp::and_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::and_expression_default);
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

inline cpp::exclusive_or_expression_default* parseSymbol(Parser& parser, cpp::exclusive_or_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::exclusive_or_expression* parseSymbol(Parser& parser, cpp::exclusive_or_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::exclusive_or_expression_default);
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

inline cpp::inclusive_or_expression_default* parseSymbol(Parser& parser, cpp::inclusive_or_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::inclusive_or_expression* parseSymbol(Parser& parser, cpp::inclusive_or_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::inclusive_or_expression_default);
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

inline cpp::logical_and_expression_default* parseSymbol(Parser& parser, cpp::logical_and_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::logical_and_expression* parseSymbol(Parser& parser, cpp::logical_and_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::logical_and_expression_default);
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

inline cpp::logical_or_expression_default* parseSymbol(Parser& parser, cpp::logical_or_expression_default* result)
{
	PARSE_REQUIRED(parser, result->left);
	PARSE_TERMINAL(parser, result->op);
	PARSE_REQUIRED(parser, result->right);
	return result;
}

inline cpp::logical_or_expression* parseSymbol(Parser& parser, cpp::logical_or_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::logical_or_expression_default);
	return result;
}

inline cpp::conditional_expression_rhs* parseSymbol(Parser& parser, cpp::conditional_expression_rhs* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->mid);
	PARSE_TERMINAL(parser, result->colon);
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

inline cpp::conditional_expression* pruneSymbol(cpp::conditional_expression_default* symbol)
{
	if(symbol->right == 0)
	{
		return symbol->left;
	}
	return symbol;
}

inline cpp::conditional_expression_default* parseSymbol(Parser& parser, cpp::conditional_expression_default* result)
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

inline cpp::conditional_expression* parseSymbol(Parser& parser, cpp::conditional_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::conditional_expression_default);
	return result;
}


inline cpp::assignment_expression* parseSymbol(Parser& parser, cpp::assignment_expression* result)
{
	PARSE_EXPRESSION_SPECIAL(parser, cpp::logical_or_expression_precedent); // TODO: handle template-id / relation-expression ambiguity
	PARSE_SELECT(parser, cpp::throw_expression); // NOTE: PARSE_EXPRESSION must be the first statement - it relies on 'parser' being untouched
	return result;
}

inline cpp::parameter_declaration_default* parseSymbol(Parser& parser, cpp::parameter_declaration_default* result)
{
	PARSE_REQUIRED(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TERMINAL(parser, result->assign);
	PARSE_REQUIRED(parser, result->init);
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
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TERMINAL(parser, result->rp);
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
	PARSE_TERMINAL(parser, result->assign);
	PARSE_REQUIRED(parser, result->init);
	return result;
}

inline cpp::parameter_declaration* parseSymbol(Parser& parser, cpp::parameter_declaration* result)
{
	PARSE_SELECT(parser, cpp::parameter_declaration_default); // TODO: ambiguity: 'C::A(X)' could be 'C::A X' or 'C::A(*)(X)'
	PARSE_SELECT(parser, cpp::parameter_declaration_abstract);
	return result;
}

inline cpp::parameter_declaration_list* parseSymbol(Parser& parser, cpp::parameter_declaration_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	if(!TOKEN_EQUAL(parser, boost::wave::T_ELLIPSIS))
	{
		PARSE_REQUIRED(parser, result->next);
	}
	return result;
}

inline cpp::parameter_declaration_clause* parseSymbol(Parser& parser, cpp::parameter_declaration_clause* result)
{
	PARSE_OPTIONAL(parser, result->list);
	PARSE_TERMINAL(parser, result->isEllipsis);
	return result;
}

inline cpp::exception_type_all* parseSymbol(Parser& parser, cpp::exception_type_all* result)
{
	PARSE_TERMINAL(parser, result->key);
	return result;
}

inline cpp::type_id_list* parseSymbol(Parser& parser, cpp::type_id_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

inline cpp::exception_type_list* parseSymbol(Parser& parser, cpp::exception_type_list* result)
{
	PARSE_SELECT(parser, cpp::exception_type_all);
	PARSE_SELECT(parser, cpp::type_id_list);
	return result;
}

inline cpp::exception_specification* parseSymbol(Parser& parser, cpp::exception_specification* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_OPTIONAL(parser, result->types);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

inline cpp::declarator_suffix_function* parseSymbol(Parser& parser, cpp::declarator_suffix_function* result)
{
	PARSE_TERMINAL(parser, result->lp);
	PARSE_OPTIONAL(parser, result->params);
	PARSE_TERMINAL(parser, result->rp);
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
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->decl);
	PARSE_TERMINAL(parser, result->rp);
	return result;
}

inline cpp::comma_operator* parseSymbol(Parser& parser, cpp::comma_operator* result)
{
	PARSE_TERMINAL(parser, result->key);
	return result;
};

inline cpp::function_operator* parseSymbol(Parser& parser, cpp::function_operator* result)
{
	PARSE_TERMINAL(parser, result->lp);
	PARSE_TERMINAL(parser, result->rp);
	return result;
};

inline cpp::array_operator* parseSymbol(Parser& parser, cpp::array_operator* result)
{
	PARSE_TERMINAL(parser, result->ls);
	PARSE_TERMINAL(parser, result->rs);
	return result;
};

inline cpp::new_operator* parseSymbol(Parser& parser, cpp::new_operator* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_OPTIONAL(parser, result->array);
	return result;
};

inline cpp::delete_operator* parseSymbol(Parser& parser, cpp::delete_operator* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_OPTIONAL(parser, result->array);
	return result;
};

inline cpp::overloadable_operator* parseSymbol(Parser& parser, cpp::overloadable_operator* result)
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
	return result;
};

inline cpp::operator_function_id_suffix* parseSymbol(Parser& parser, cpp::operator_function_id_suffix* result)
{
	PARSE_TERMINAL(parser, result->lt);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TERMINAL(parser, result->gt);
	return result;
}

inline cpp::operator_function_id* parseSymbol(Parser& parser, cpp::operator_function_id* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->op);
	PARSE_OPTIONAL(parser, result->suffix);
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
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->spec);
	PARSE_OPTIONAL(parser, result->decl);
	return result;
}

inline cpp::template_id_operator_function* parseSymbol(Parser& parser, cpp::template_id_operator_function* result)
{
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->lt);
	PARSE_OPTIONAL(parser, result->args);
	PARSE_TERMINAL(parser, result->gt);
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
	PARSE_TERMINAL(parser, result->compl);
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
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_REQUIRED(parser, result->context);
	PARSE_TERMINAL(parser, result->isTemplate);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id_suffix* parseSymbol(Parser& parser, cpp::qualified_id_suffix* result)
{
	PARSE_SELECT(parser, cpp::template_id); // todo: shared-prefix ambiguity: 'template-id' vs 'identifier'
	PARSE_SELECT(parser, cpp::identifier);
	PARSE_SELECT(parser, cpp::operator_function_id);
	return result;
}

inline cpp::qualified_id_global* parseSymbol(Parser& parser, cpp::qualified_id_global* result)
{
	PARSE_TERMINAL(parser, result->scope);
	PARSE_REQUIRED(parser, result->id);
	return result;
}

inline cpp::qualified_id* parseSymbol(Parser& parser, cpp::qualified_id* result)
{
	PARSE_SELECT(parser, cpp::qualified_id_default);
	PARSE_SELECT(parser, cpp::qualified_id_global);
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
	// TODO
	bool isTry;
	PARSE_TOKEN_OPTIONAL(parser, isTry, boost::wave::T_TRY);
	PARSE_OPTIONAL(parser, result->init);
	PARSE_REQUIRED(parser, result->body);
	if(isTry)
	{
		PARSE_REQUIRED(parser, result->handlers);
	}
	return result;
}

inline cpp::linkage_specification_compound* parseSymbol(Parser& parser, cpp::linkage_specification_compound* result)
{
	PARSE_TERMINAL(parser, result->lb);
	PARSE_OPTIONAL(parser, result->decl);
	PARSE_TERMINAL(parser, result->rb);
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
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->str);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::explicit_instantiation* parseSymbol(Parser& parser, cpp::explicit_instantiation* result)
{
	PARSE_TERMINAL(parser, result->isExtern);
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::explicit_specialization* parseSymbol(Parser& parser, cpp::explicit_specialization* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lt);
	PARSE_TERMINAL(parser, result->gt);
	PARSE_REQUIRED(parser, result->decl);
	return result;
}

inline cpp::expression_list* parseSymbol(Parser& parser, cpp::expression_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

inline cpp::initializer_clause* parseSymbol(Parser& parser, cpp::initializer_clause* result);

inline cpp::initializer_list* parseSymbol(Parser& parser, cpp::initializer_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_TERMINAL(parser, result->comma);
	PARSE_OPTIONAL(parser, result->next); // optional because trailing comma is allowed
	return result;
}

inline cpp::initializer_clause_list* parseSymbol(Parser& parser, cpp::initializer_clause_list* result)
{
	PARSE_TERMINAL(parser, result->lb);
	PARSE_REQUIRED(parser, result->list);
	PARSE_TERMINAL(parser, result->rb);
	return result;
}

inline cpp::initializer_clause_empty* parseSymbol(Parser& parser, cpp::initializer_clause_empty* result)
{
	PARSE_TERMINAL(parser, result->lb);
	PARSE_TERMINAL(parser, result->rb);
	return result;
}

inline cpp::initializer_clause* parseSymbol(Parser& parser, cpp::initializer_clause* result)
{
	PARSE_SELECT(parser, cpp::initializer_clause_empty);
	PARSE_SELECT(parser, cpp::initializer_clause_list);
	PARSE_SELECT(parser, cpp::assignment_expression);
	return result;
}

inline cpp::initializer_default* parseSymbol(Parser& parser, cpp::initializer_default* result)
{
	PARSE_TERMINAL(parser, result->assign);
	PARSE_REQUIRED(parser, result->clause);
	return result;
}

inline cpp::initializer_parenthesis* parseSymbol(Parser& parser, cpp::initializer_parenthesis* result)
{
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->list);
	PARSE_TERMINAL(parser, result->rp);
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
	PARSE_TERMINAL(parser, result->comma);
	PARSE_REQUIRED(parser, result->next);
	return result;
}

inline cpp::simple_declaration_suffix* parseSymbol(Parser& parser, cpp::simple_declaration_suffix* result)
{
	PARSE_OPTIONAL(parser, result->init);
	PARSE_TERMINAL(parser, result->comma);
	if(result->comma.value != 0)
	{
		PARSE_REQUIRED(parser, result->next);
	}
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::simple_declaration_named* parseSymbol(Parser& parser, cpp::simple_declaration_named* result)
{
	PARSE_REQUIRED(parser, result->decl);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::simple_declaration_affix* parseSymbol(Parser& parser, cpp::simple_declaration_affix* result)
{
	PARSE_SELECT(parser, cpp::general_declaration_type);
	PARSE_SELECT(parser, cpp::simple_declaration_named);
	return result;
}

inline cpp::simple_declaration* parseSymbol(Parser& parser, cpp::simple_declaration* result)
{
	PARSE_REQUIRED(parser, result->spec);
	PARSE_REQUIRED(parser, result->affix);
	return result;
}

inline cpp::asm_definition* parseSymbol(Parser& parser, cpp::asm_definition* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->str);
	PARSE_TERMINAL(parser, result->rp);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::namespace_alias_definition* parseSymbol(Parser& parser, cpp::namespace_alias_definition* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->alias);
	PARSE_TERMINAL(parser, result->assign);
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::using_declaration_global* parseSymbol(Parser& parser, cpp::using_declaration_global* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->scope);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::using_declaration_nested* parseSymbol(Parser& parser, cpp::using_declaration_nested* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->isTypename);
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_REQUIRED(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->semicolon);
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
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->key2);
	PARSE_TERMINAL(parser, result->isGlobal);
	PARSE_OPTIONAL(parser, result->context);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::block_declaration* parseSymbol(Parser& parser, cpp::block_declaration* result)
{
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

inline cpp::general_declaration_named* parseSymbol(Parser& parser, cpp::general_declaration_named* result)
{
	PARSE_REQUIRED(parser, result->decl);
	PARSE_REQUIRED(parser, result->suffix);
	return result;
}

inline cpp::general_declaration_type* parseSymbol(Parser& parser, cpp::general_declaration_type* result)
{
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::general_declaration_affix* parseSymbol(Parser& parser, cpp::general_declaration_affix* result)
{
	PARSE_SELECT(parser, cpp::general_declaration_type);
	PARSE_SELECT(parser, cpp::general_declaration_named);
	return result;
}

inline cpp::general_declaration* parseSymbol(Parser& parser, cpp::general_declaration* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->affix);
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
	PARSE_TERMINAL(parser, result->lb);
	PARSE_OPTIONAL(parser, result->body);
	PARSE_TERMINAL(parser, result->rb);
	return result;
}

inline cpp::declaration_statement* parseSymbol(Parser& parser, cpp::declaration_statement* result)
{
	PARSE_SELECT(parser, cpp::simple_declaration);
	PARSE_SELECT(parser, cpp::block_declaration);
	return result;
}

inline cpp::labeled_statement_id* parseSymbol(Parser& parser, cpp::labeled_statement_id* result)
{
	PARSE_REQUIRED(parser, result->label);
	PARSE_TERMINAL(parser, result->colon);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::labeled_statement_case* parseSymbol(Parser& parser, cpp::labeled_statement_case* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->label);
	PARSE_TERMINAL(parser, result->colon);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::labeled_statement_default* parseSymbol(Parser& parser, cpp::labeled_statement_default* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->colon);
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
	PARSE_TERMINAL(parser, result->assign);
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
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TERMINAL(parser, result->rp);
	PARSE_REQUIRED(parser, result->body);
	PARSE_TERMINAL(parser, result->key2);
	PARSE_REQUIRED(parser, result->fail);
	return result;
}

inline cpp::selection_statement_switch* parseSymbol(Parser& parser, cpp::selection_statement_switch* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TERMINAL(parser, result->rp);
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
	PARSE_SELECT(parser, cpp::simple_declaration);
	PARSE_SELECT(parser, cpp::expression_statement);
	return result;
}

inline cpp::iteration_statement_for* parseSymbol(Parser& parser, cpp::iteration_statement_for* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->init);
	PARSE_OPTIONAL(parser, result->cond);
	PARSE_TERMINAL(parser, result->semicolon);
	PARSE_OPTIONAL(parser, result->incr);
	PARSE_TERMINAL(parser, result->rp);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::iteration_statement_while* parseSymbol(Parser& parser, cpp::iteration_statement_while* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TERMINAL(parser, result->rp);
	PARSE_REQUIRED(parser, result->body);
	return result;
}

inline cpp::iteration_statement_dowhile* parseSymbol(Parser& parser, cpp::iteration_statement_dowhile* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->body);
	PARSE_TERMINAL(parser, result->key2);
	PARSE_TERMINAL(parser, result->lp);
	PARSE_REQUIRED(parser, result->cond);
	PARSE_TERMINAL(parser, result->rp);
	PARSE_TERMINAL(parser, result->semicolon);
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
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::jump_statement_return* parseSymbol(Parser& parser, cpp::jump_statement_return* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_OPTIONAL(parser, result->expr);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::jump_statement_goto* parseSymbol(Parser& parser, cpp::jump_statement_goto* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->id);
	PARSE_TERMINAL(parser, result->semicolon);
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
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->body);
	PARSE_REQUIRED(parser, result->handlers);
	return result;
}

inline cpp::expression_statement* parseSymbol(Parser& parser, cpp::expression_statement* result)
{
	PARSE_OPTIONAL(parser, result->expr);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::msext_asm_terminal* parseSymbol(Parser& parser, cpp::msext_asm_terminal* result)
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

inline cpp::msext_asm_element* parseSymbol(Parser& parser, cpp::msext_asm_element* result);

inline cpp::msext_asm_element_list* parseSymbol(Parser& parser, cpp::msext_asm_element_list* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::msext_asm_element_list_inline* parseSymbol(Parser& parser, cpp::msext_asm_element_list_inline* result)
{
	size_t line = parser.get_position().get_line();
	PARSE_REQUIRED(parser, result->item);
	if(parser.get_position().get_line() == line) // HACK: only continue until end of line
	{
		PARSE_OPTIONAL(parser, result->next);
		PARSE_TERMINAL(parser, result->semicolon);
	}
	return result;
}

inline cpp::msext_asm_statement* parseSymbol(Parser& parser, cpp::msext_asm_statement* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_REQUIRED(parser, result->list);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::msext_asm_statement_braced* parseSymbol(Parser& parser, cpp::msext_asm_statement_braced* result)
{
	PARSE_TERMINAL(parser, result->key);
	PARSE_TERMINAL(parser, result->lb);
	PARSE_REQUIRED(parser, result->list);
	PARSE_TERMINAL(parser, result->rb);
	PARSE_TERMINAL(parser, result->semicolon);
	return result;
}

inline cpp::msext_asm_element* parseSymbol(Parser& parser, cpp::msext_asm_element* result)
{
	PARSE_SELECT(parser, cpp::msext_asm_statement_braced); // TODO: shared-prefix ambiguity: braced and unbraced start with '__asm'
	PARSE_SELECT(parser, cpp::msext_asm_statement);
	PARSE_SELECT(parser, cpp::msext_asm_terminal); // will eat anything!
	return result;
}

inline cpp::statement* parseSymbol(Parser& parser, cpp::statement* result)
{
	PARSE_SELECT(parser, cpp::msext_asm_statement_braced); // TODO: shared-prefix ambiguity: braced and unbraced start with '__asm'
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


cpp::declaration_seq* parseFile(Lexer& lexer)
{
	Parser parser(lexer);

	cpp::symbol_optional<cpp::declaration_seq> result(NULL);
	try
	{
		PARSE_OPTIONAL(parser, result);
	}
	catch(ParseError&)
	{
	}
	if(!lexer.finished())
	{
		printError(parser);
	}
	return result;
}

cpp::statement_seq* parseFunction(Lexer& lexer)
{
	Parser parser(lexer);

	cpp::symbol_optional<cpp::statement_seq> result(NULL);
	try
	{
		PARSE_OPTIONAL(parser, result);
	}
	catch(ParseError&)
	{
	}
	if(!lexer.finished())
	{
		printError(parser);
	}
	return result;
}

