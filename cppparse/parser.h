
#ifndef INCLUDED_CPPPARSE_PARSER_H
#define INCLUDED_CPPPARSE_PARSER_H

#include "lexer.h"
#include "cpptree.h"

inline bool isEOF(const LexToken& token)
{
	return IS_CATEGORY(get_id(token), boost::wave::EOFTokenType);
}

inline bool isWhiteSpace(const LexToken& token)
{
	return IS_CATEGORY(get_id(token), boost::wave::WhiteSpaceTokenType)
		|| IS_CATEGORY(get_id(token), boost::wave::EOLTokenType);
}

struct Parser
{
	LexIterator& first;
	LexIterator& last;

	Parser(LexContext& context)
		: first(createBegin(context)), last(createEnd(context))
	{
		if(isWhiteSpace(dereference()))
		{
			increment();
		}
	}
	~Parser()
	{
		release(first);
		release(last);
	}
	Parser(const Parser& other)
		: first(cloneIterator(other.first)), last(cloneIterator(other.last))
	{
	}
	Parser& operator=(const Parser& other)
	{
		assignIterator(first, other.first);
		assignIterator(last, other.last);
		return *this;
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

inline std::string parseIdentifier(Parser& parser)
{
	PARSE_ASSERT(isIdentifier(parser.dereference()));
	std::string result(get_value(parser.dereference()));
	parser.increment();
	return result;
};

inline void parseToken(Parser& parser, boost::wave::token_id id)
{
	PARSE_ASSERT(isToken(parser.dereference(), id));
	parser.increment();
};

template<typename T>
T* createNode(T*)
{
	return new T;
}

#define TOKEN_EQUAL(parser, token) isToken(parser.dereference(), token)
#define PARSE_TOKEN_REQUIRED(parser, token) if(TOKEN_EQUAL(parser, token)) { parser.increment(); } else { return NULL; }
#define PARSE_TOKEN_OPTIONAL(parser, result, token) result = false; if(TOKEN_EQUAL(parser, token)) { result = true; parser.increment(); }
#define PARSE_OPTIONAL(parser, p) { Parser tmp_(parser); if((p) = parseNode((parser), createNode(p))) { parser = tmp_; } }
#define PARSE_REQUIRED(parser, p) if(((p) = parseNode((parser), createNode(p))) == NULL) { return NULL; }
#define PARSE_SELECT(parser, Type) { Parser tmp_(parser); if(Type* p = parseNode(tmp_, new Type)) { parser = tmp_; return p; } }
#define PARSE_SELECT_TOKEN(parser, p, token, value_) if(get_id(parser.dereference()) == token) { p->value = value_; parser.increment(); return p; }

inline cpp::declaration_seq* parseNode(Parser& parser, cpp::declaration_seq* result);

inline cpp::namespace_definition* parseNode(Parser& parser, cpp::namespace_definition* result)
{
	if(TOKEN_EQUAL(parser, boost::wave::T_NAMESPACE))
	{
		PARSE_TOKEN_REQUIRED(parser, boost::wave::T_NAMESPACE);
		if(!TOKEN_EQUAL(parser, boost::wave::T_LEFTBRACE))
		{
			result->id.value = parseIdentifier(parser);
		}
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

inline cpp::decl_specifier_builtin* parseNode(Parser& parser, cpp::decl_specifier_builtin* result)
{
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_FRIEND, cpp::decl_specifier_builtin::FRIEND);
	PARSE_SELECT_TOKEN(parser, result, boost::wave::T_TYPEDEF, cpp::decl_specifier_builtin::TYPEDEF);
	return NULL;
}


inline cpp::type_name* parseNode(Parser& parser, cpp::type_name* result)
{
	// TODO
	return NULL;
}

inline cpp::simple_template_id* parseNode(Parser& parser, cpp::simple_template_id* result)
{
	// TODO
	return NULL;
}

inline cpp::nested_name_specifier* parseNode(Parser& parser, cpp::nested_name_specifier* result)
{
	// TODO
	return NULL;
}

inline cpp::simple_type_specifier_name* parseNode(Parser& parser, cpp::simple_type_specifier_name* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->id);
	return NULL;
}

inline cpp::simple_type_specifier_template* parseNode(Parser& parser, cpp::simple_type_specifier_template* result)
{
	PARSE_TOKEN_OPTIONAL(parser, result->isGlobal, boost::wave::T_COLON_COLON);
	PARSE_REQUIRED(parser, result->spec);
	PARSE_TOKEN_REQUIRED(parser, boost::wave::T_TEMPLATE);
	PARSE_REQUIRED(parser, result->id);
	return NULL;
}

inline cpp::simple_type_specifier* parseNode(Parser& parser, cpp::simple_type_specifier* result)
{
	PARSE_SELECT(parser, cpp::simple_type_specifier_builtin);
	PARSE_SELECT(parser, cpp::simple_type_specifier_template);
	PARSE_SELECT(parser, cpp::simple_type_specifier_name);
	return NULL;
}

inline cpp::class_specifier* parseNode(Parser& parser, cpp::class_specifier* result)
{
	// TODO
	return NULL;
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

inline cpp::type_specifier* parseNode(Parser& parser, cpp::type_specifier* result)
{
	PARSE_SELECT(parser, cpp::simple_type_specifier);
	PARSE_SELECT(parser, cpp::class_specifier);
	PARSE_SELECT(parser, cpp::enum_specifier);
	PARSE_SELECT(parser, cpp::elaborated_type_specifier);
	PARSE_SELECT(parser, cpp::cv_qualifier);
	return NULL;
}

inline cpp::decl_specifier* parseNode(Parser& parser, cpp::decl_specifier* result)
{
	PARSE_SELECT(parser, cpp::storage_class_specifier);
	PARSE_SELECT(parser, cpp::decl_specifier_builtin);
	PARSE_SELECT(parser, cpp::function_specifier);
	PARSE_SELECT(parser, cpp::type_specifier);
	return NULL;
}

inline cpp::decl_specifier_seq* parseNode(Parser& parser, cpp::decl_specifier_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}

inline cpp::declarator* parseNode(Parser& parser, cpp::declarator* result)
{
	// TODO
	return NULL;
}

inline cpp::ctor_initializer* parseNode(Parser& parser, cpp::ctor_initializer* result)
{
	// TODO
	return NULL;
}

inline cpp::function_body* parseNode(Parser& parser, cpp::function_body* result)
{
	// TODO
	return NULL;
}

inline cpp::handler_seq* parseNode(Parser& parser, cpp::handler_seq* result)
{
	// TODO
	return NULL;
}

inline cpp::function_definition* parseNode(Parser& parser, cpp::function_definition* result)
{
	PARSE_OPTIONAL(parser, result->spec);
	PARSE_REQUIRED(parser, result->decl);
	bool isTry;
	PARSE_TOKEN_OPTIONAL(parser, isTry, boost::wave::T_TRY)
	PARSE_OPTIONAL(parser, result->init);
	PARSE_REQUIRED(parser, result->body);
	result->handlers = NULL;
	if(isTry)
	{
		PARSE_REQUIRED(parser, result->handlers);
	}
	return result;
}


inline cpp::declaration* parseNode(Parser& parser, cpp::declaration* result)
{
	PARSE_SELECT(parser, cpp::namespace_definition);
	PARSE_SELECT(parser, cpp::function_definition);
	return NULL;
}

inline cpp::declaration_seq* parseNode(Parser& parser, cpp::declaration_seq* result)
{
	PARSE_REQUIRED(parser, result->item);
	PARSE_OPTIONAL(parser, result->next);
	return result;
}


inline cpp::declaration_seq* parseFile(std::string& instring, const char* input)
{
	LexContext& context = createContext(instring, input);

	Parser parser(context);

	return parseNode(parser, new cpp::declaration_seq);
}


#endif


