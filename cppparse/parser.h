
#ifndef INCLUDED_CPPPARSE_PARSER_H
#define INCLUDED_CPPPARSE_PARSER_H

#include "lexer.h"
#include "cpptree.h"
#include <typeinfo>

struct ParseError
{
	ParseError()
	{
	}
};

struct GeneralError
{
	GeneralError()
	{
	}
};

#define ASSERT(condition) if(!(condition)) { throw GeneralError(); }

#define MINGLE

struct SymbolDebug
{
	bool disambiguated;
	SymbolDebug(bool disambiguated) : disambiguated(disambiguated)
	{
	}

	void visit(cpp::terminal_identifier symbol)
	{
		std::cout << symbol.value << " ";
	}

	void visit(cpp::terminal_string symbol)
	{
		std::cout << symbol.value << " ";
	}

	void visit(cpp::terminal_choice2 symbol)
	{
		std::cout << symbol.value << " ";
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
		if(symbol.value != 0)
		{
			std::cout << symbol.value << " ";
		}
	}

	template<typename T>
	void visit(T* symbol)
	{
		symbol->accept(*this);
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}

	void visit(cpp::template_argument_list* symbol)
	{
		if(disambiguated)
		{
			std::cout << "( ";
			symbol->accept(*this);
			std::cout << ") ";
		}
		else
		{
			symbol->accept(*this);
		}
	}
	void visit(cpp::relational_expression_default* symbol)
	{
		if(disambiguated)
		{
			std::cout << "( ";
			symbol->accept(*this);
			std::cout << ") ";
		}
		else
		{
			symbol->accept(*this);
		}
	}

	template<typename T>
	void visit(cpp::ambiguity<T>* symbol)
	{
		if(disambiguated)
		{
			std::cout << "[ ";
			symbol->accept(*this);
			std::cout << "] ";
		}
		else
		{
			symbol->accept(*this);
		}
	}
};

template<typename T>
void printSymbol(T* symbol, bool disambiguated = false)
{
	SymbolDebug walker(disambiguated);
	walker.visit(symbol);
}



inline bool isAlphabet(char c)
{
	return (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z');
}

inline bool isNumber(char c)
{
	return (c >= '0' && c <= '9');
}

inline bool isIdentifier(char c)
{
	return isAlphabet(c) || isNumber(c) || c == '_';
}

inline bool isToken(LexTokenId token, boost::wave::token_id id)
{
	return token == id;
}

inline bool isIdentifier(LexTokenId token)
{
	return isToken(token, boost::wave::T_IDENTIFIER);
}

inline void printSequence(BacktrackBuffer& buffer, BacktrackBuffer::const_iterator first, BacktrackBuffer::const_iterator last)
{
	std::cout << "   ";
	bool space = false;
	bool endline = false;
	for( BacktrackBuffer::const_iterator i = first; i != last; i = next(buffer, i))
	{
		if(space && isIdentifier(*(*i).value))
		{
			std::cout << " ";
		}
		std::cout << (*i).value;
		space = isIdentifier((*i).value[strlen((*i).value) - 1]);
	}
}

inline void printSequence(Lexer& lexer)
{
	printSequence(lexer.history, lexer.position, lexer.error);
	std::cout << std::endl;
}

inline void printSequence(Lexer& lexer, size_t position)
{
	//position = std::min(std::min(lexer.history.size(), size_t(32)), position);
	//printSequence(lexer.position - position, lexer.position);
}

struct VisualiserNode
{
	VisualiserNode* parent;
	const char* name;
	size_t allocation;
	BacktrackBuffer::const_iterator position;
	Declaration* declaration;
};

inline void printIndent(VisualiserNode* node)
{
	for(VisualiserNode* p = node; p != 0; p = p->parent)
	{
		if(p->name != SYMBOL_NAME(cpp::declaration_seq)
			&& p->name != SYMBOL_NAME(cpp::statement_seq)
			&& p->name != SYMBOL_NAME(cpp::member_specification))
		{
			std::cout << ".";
		}
	}
}

cpp::terminal_identifier& getDeclarationId(Declaration* declaration);


struct Visualiser
{
	typedef LinearAllocator<false> Allocator;
	Allocator allocator;
	VisualiserNode* node;
	Visualiser() : node(0)
	{
	}

	void push(const char* name, BacktrackBuffer::const_iterator position)
	{
		VisualiserNode tmp = { node, name, allocator.position, position, 0 };
		node = new(allocator.allocate(sizeof(VisualiserNode))) VisualiserNode(tmp);
	}
	void pop(bool hit)
	{
		VisualiserNode* parent = node->parent;
		if(hit)
		{
			if(node->name == SYMBOL_NAME(cpp::declaration)
				|| node->name == SYMBOL_NAME(cpp::statement)
				|| node->name == SYMBOL_NAME(cpp::member_declaration))
			{
				allocator.backtrack(node->allocation);
			}
			else
			{
				node->allocation = size_t(-1);
			}
		}
		node = parent;
	}
	void pop(cpp::identifier* symbol)
	{
		node->declaration = symbol->value.dec.p;
		pop(symbol != 0);
	}
	template<typename T>
	void pop(T* symbol)
	{
		pop(symbol != 0);
	}

	void print(BacktrackBuffer& buffer)
	{
		Allocator::Pages::iterator last = allocator.pages.begin() + allocator.position / sizeof(Page);
		for(Allocator::Pages::iterator i = allocator.pages.begin(); i != allocator.pages.end(); ++i)
		{
			size_t pageSize = i == last ? allocator.position % Page::SIZE : Page::SIZE;
			VisualiserNode* first = reinterpret_cast<VisualiserNode*>((*i)->buffer);
			VisualiserNode* last = first + pageSize / sizeof(VisualiserNode);
			for(VisualiserNode* p = first; p != last; ++p)
			{
				if(p->name != SYMBOL_NAME(cpp::declaration_seq)
					&& p->name != SYMBOL_NAME(cpp::statement_seq))
				{
#if 0
					if(p->parent->position != 0
						&& p->parent->position != p->position)
					{
						printSequence(buffer, p->parent->position, p->position);
					}
					std::cout << std::endl;
#endif
					bool success = p->allocation == size_t(-1);
					printIndent(p);
					std::cout << p->name;
					std::cout << (success ? "*" : "");
					if(p->name == SYMBOL_NAME(cpp::identifier))
					{
						std::cout << " ";
						if(p->declaration != 0)
						{
							cpp::terminal_identifier& id = getDeclarationId(p->declaration);
							std::cout << id.value << " ";
							if(id.position.file != 0)
							{
								printPosition(id.position);
							}
						}
						else
						{
							std::cout << (*p->position).value;
						}
					}
					std::cout << std::endl;
				}
			}

			if(pageSize != Page::SIZE)
			{
				break;
			}
		}
		std::cout << std::endl;
	}
};




struct ParserState
{
	bool inTemplateArgumentList;

	ParserState()
		: inTemplateArgumentList(false)
	{
	}
};

struct ParserContext : Lexer
{
	Visualiser visualiser;
	ParserContext(LexContext& context, const char* path)
		: Lexer(context, path)
	{
	}
};

struct Parser : public ParserState
{
	ParserContext& lexer;
	size_t position;
	size_t allocation;

	Parser(ParserContext& lexer)
		: lexer(lexer), position(0), allocation(0)
	{
	}
	Parser(const Parser& other)
		: ParserState(other), lexer(other.lexer), position(0), allocation(lexer.allocator.position)
	{
	}

	LexTokenId get_id()
	{
		return lexer.get_id();
	}
	const char* get_value()
	{
		return lexer.get_value();
	}
	FilePosition get_position()
	{
		return lexer.get_position();
	}

	void increment()
	{
		++position;
		lexer.increment();
	}

	void backtrack(const char* symbol, bool preserveAllocation = false)
	{
		lexer.backtrack(position, symbol);
		if(!preserveAllocation)
		{
			lexer.allocator.backtrack(allocation);
		}
	}
	void advance()
	{
		if(position != 0)
		{
			lexer.advance(position);
		}
	}
};

inline void printError(Parser& parser)
{
#if 0
	for(Lexer::Positions::const_iterator i = parser.lexer.backtrace.begin(); i != parser.lexer.backtrace.end(); ++i)
	{
	}
	printPosition(parser.lexer, lexer.history[parser.lexer.stacktrace.back()].position);
#endif
	printPosition(parser.lexer.getErrorPosition());
	std::cout << "syntax error: '" << parser.lexer.getErrorValue() << "'" << std::endl;
#if 1 // TODO!
	parser.lexer.visualiser.print(parser.lexer.history);
#endif
	printSequence(parser.lexer); // rejected tokens
}

inline void printSequence(Parser& parser)
{
	printSequence(parser.lexer, parser.position);
}

#define PARSE_ERROR() throw ParseError()
#define PARSE_ASSERT(condition) if(!(condition)) { PARSE_ERROR(); }

inline void parseToken(Parser& parser, boost::wave::token_id id)
{
	PARSE_ASSERT(isToken(parser.get_id(), id));
	parser.increment();
};


#define NULLSYMBOL(T) cpp::symbol< T >(0)


struct TrueSized
{
	char m[1];
};

struct FalseSized
{
	char m[2];
};

template<typename T, typename Base>
struct IsConvertible
{
	static TrueSized test(Base*);
	static FalseSized test(...);

	static const bool RESULT = sizeof(IsConvertible<T, Base>::test(NULLSYMBOL(T))) == sizeof(TrueSized);
};

template<typename T>
struct IsConcrete
{
	static const bool RESULT = !IsConvertible<T, cpp::choice<T> >::RESULT
		&& !IsConvertible<T, cpp::terminal_choice>::RESULT;
};

template<typename T, bool isConcrete = IsConcrete<T>::RESULT >
struct SymbolAllocator;

template<typename T>
struct SymbolAllocator<T, true>
{
	LexerAllocator& allocator;
	SymbolAllocator(LexerAllocator& allocator) : allocator(allocator)
	{
	}
	T* allocate()
	{
		return new(allocator.allocate(sizeof(T))) T;
	}
	void deallocate(T* p)
	{
		p->~T();
		allocator.deallocate(p, sizeof(T));
	}
};

template<typename T>
struct SymbolAllocator<T, false>
{
	SymbolAllocator(LexerAllocator& allocator)
	{
	}
	T* allocate()
	{
		return 0;
	}
	void deallocate(T* p)
	{
	}
};

template<typename T>
T* createSymbol(Parser& parser, T*)
{
	return new(parser.lexer.allocator.allocate(sizeof(T))) T;
}

struct SymbolDelete
{
	LexerAllocator& allocator;
	SymbolDelete(LexerAllocator& allocator) : allocator(allocator)
	{
	}

	void visit(cpp::terminal_identifier symbol)
	{
	}
	void visit(cpp::terminal_string symbol)
	{
	}
	void visit(cpp::terminal_choice2 symbol)
	{
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
	}

	template<typename T>
	void visit(T* symbol)
	{
		symbol->accept(*this);
		SymbolAllocator<T, !IsConvertible<T, cpp::choice<T> >::RESULT>(allocator).deallocate(symbol);
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}
};

template<typename T>
void deleteSymbol(T* symbol, LexerAllocator& allocator)
{
#ifdef _DEBUG
	SymbolDelete walker(allocator);
	walker.visit(cpp::symbol<T>(symbol));
#endif
}

inline bool checkBacktrack(Parser& parser)
{
	if(parser.lexer.maxBacktrack)
	{
		printError(parser);
		return true;
	}
	return false;
};

template<typename ParserType, typename T>
cpp::symbol<T> parseSymbolRequired(ParserType& parser, cpp::symbol<T> symbol, size_t best = 0)
{
	PARSE_ASSERT(symbol.p == 0);
	PARSE_ASSERT(!checkBacktrack(parser));
	ParserType tmp(parser);
	parser.lexer.push();
	parser.lexer.visualiser.push(SYMBOL_NAME(T), parser.lexer.position);
	T* p = SymbolAllocator<T>(parser.lexer.allocator).allocate();
#if 0
	T* result = parseSymbol(tmp, p);
#else
	T* result = tmp.parse(p);
#endif
	parser.lexer.pop();
	if(result != 0
		&& tmp.position >= best)
	{
		parser.lexer.visualiser.pop(result);
		parser.position += tmp.position;
		return cpp::symbol<T>(result);
	}

	deleteSymbol(p, parser.lexer.allocator);
	parser.lexer.visualiser.pop(false);
	tmp.backtrack(SYMBOL_NAME(T));
	return cpp::symbol<T>(0);
}

template<typename ParserType, typename T>
cpp::symbol_optional<T> parseSymbolOptional(ParserType& parser, cpp::symbol_optional<T> symbol)
{
	return cpp::symbol_optional<T>(parseSymbolRequired(parser, symbol));
}


inline void breakpoint()
{
}


#ifdef _DEBUG
#define SYMBOLP_NAME(p) (typeid(*p).name() + 12)
#else
#define SYMBOLP_NAME(p) "cpp::$symbol"
#endif

template<typename T>
T* pruneSymbol(T* symbol)
{
	return symbol;
}

template<typename T>
inline T* pruneBinaryExpression(T* symbol)
{
	return symbol->right == 0 ? symbol->left : symbol;
}


template<LexTokenId id>
inline LexTokenId getTokenId(cpp::terminal<id>)
{
	return id;
}

inline const char* parseTerminal(Parser& parser, LexTokenId id)
{
	if(isToken(parser.get_id(), id))
	{
		const char* value = parser.get_value();
		parser.increment();
		return value;
	}
	return NULL;
}

enum ParseResult
{
	PARSERESULT_PASS,
	PARSERESULT_FAIL,
	PARSERESULT_SKIP,
};

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal<id>& result)
{
	result.value = parseTerminal(parser, getTokenId(result));
	return result.value != 0 ? PARSERESULT_PASS : PARSERESULT_FAIL;
}

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal_optional<id>& result)
{
	result.value = parseTerminal(parser, getTokenId(result));
	return PARSERESULT_PASS;
}

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal_suffix<id>& result)
{
	result.value = parseTerminal(parser, getTokenId(result));
	return result.value != 0 ? PARSERESULT_PASS : PARSERESULT_SKIP;
}


#define TOKEN_EQUAL(parser, token) isToken((parser).get_id(), token)
// TODO: avoid dependency on 'result'
#define PARSE_TERMINAL(parser, t) switch(parseTerminal(parser, t)) { case PARSERESULT_FAIL: return 0; case PARSERESULT_SKIP: return result; default: break; }

#define PARSE_TOKEN_REQUIRED(parser, token_) if(TOKEN_EQUAL(parser, token_)) { parser.increment(); } else { return 0; }
#define PARSE_TOKEN_OPTIONAL(parser, result, token) result = false; if(TOKEN_EQUAL(parser, token)) { result = true; parser.increment(); }
#define PARSE_SELECT_TOKEN(parser, p, token, value_) if(TOKEN_EQUAL(parser, token)) { p = createSymbol(parser, p); p->id = value_; p->value.id = token; p->value.value = parser.get_value(); parser.increment(); return p; }
#define PARSE_OPTIONAL(parser, p) (p) = parseSymbolOptional(parser, p)
#define PARSE_REQUIRED(parser, p) if(((p) = parseSymbolRequired(parser, p)) == 0) { return 0; }
#define PARSE_SELECT(parser, Type) if(cpp::symbol<Type> p = parseSymbolRequired(parser, NULLSYMBOL(Type))) { return p; }
#define PARSE_SELECT_UNAMBIGUOUS PARSE_SELECT

// Type must have members 'left' and 'right', and 'typeof(left)' must by substitutable for 'Type'
#define PARSE_PREFIX(parser, Type) if(cpp::symbol<Type> p = parseSymbolRequired(parser, NULLSYMBOL(Type))) { if(p->right == 0) return p->left; return p; }

#if 0
#define PARSE_EXPRESSION PARSE_PREFIX
#else
#define PARSE_EXPRESSION PARSE_SELECT
#endif



template<typename ParserType, typename T, typename Base>
inline cpp::symbol<Base> parseExpression(ParserType& parser, cpp::symbol<T> symbol, Base* result)
{
	// HACK: create temporary copy of expression-symbol to get RHS-symbol
	result = parseSymbolRequired(parser, T().right);
	if(result != 0)
	{
		for(;;)
		{
			// parse suffix of expression-symbol
			symbol = parseSymbolRequired(parser, NULLSYMBOL(T));
			if(symbol == 0
				|| symbol->right == 0)
			{
				break;
			}
			symbol->left = makeSymbol(result);
			result = symbol;
		}
	}
	return makeSymbol(result);
}

#define PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, Type) result = parseExpression(parser, NULLSYMBOL(Type), result)

template<typename ParserType, typename T>
inline cpp::symbol_optional<T> parseSequence(ParserType& parser, cpp::symbol_optional<T>)
{
	T tmp;
	cpp::symbol_optional<T> p(&tmp);
	for(;;)
	{
		p->next = parseSymbolOptional(parser, p->next);
		if(p->next == 0)
		{
			break;
		}
		p = p->next;
	}
	return tmp.next;
}

#ifdef MINGLE
#define PARSE_SEQUENCE PARSE_OPTIONAL
#else
#define PARSE_SEQUENCE(parser, p) p = parseSequence(parser, p)
#endif

struct True
{
};

struct False
{
};


struct ParseResultFail
{
};

struct ParseResultSkip
{
};

struct ParserOpaque : public Parser
{
	void* context;
	ParserOpaque(ParserContext& lexer, void* context)
		: Parser(lexer), context(context)
	{
	}
};

template<typename Context>
struct ParserGeneric : public ParserOpaque
{
	ParserGeneric(ParserContext& lexer, Context& context)
		: ParserOpaque(lexer, &context)
	{
	}
	Context& makeContext()
	{
		Context& context = *static_cast<Context*>(ParserOpaque::context);
		context.parser = this;
		return context;
	}
	template<typename T>
	T* parse(T* symbol)
	{
		Context& context = makeContext();
		context.visit(symbol);
		return static_cast<T*>(context.result);
	}
};

#define SYMBOL_WALK(walker, symbol) if((result = symbol = parseSymbol(getParser(walker), symbol)) == 0) return
#define PARSERCONTEXT_DEFAULT \
	template<typename T> \
	void visit(T* symbol) \
	{ \
		SYMBOL_WALK(*this, symbol); \
	} \
	template<LexTokenId id> \
	void visit(cpp::terminal<id> symbol) \
	{ \
	}

template<typename ParserType>
struct ParsingVisitor
{
	ParserType& parser;
	bool skip;
	ParsingVisitor(ParserType& parser) : parser(parser), skip(false)
	{
	}
	template<typename T>
	bool visit(cpp::symbol<T>& s)
	{
		return (s = parseSymbolRequired(parser, s)) != 0;
	}
	template<typename T>
	bool visit(cpp::symbol_optional<T>& s)
	{
		s = cpp::symbol_optional<T>(parseSymbolRequired(parser, s));
		return true;
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal<ID>& t)
	{
		bool result = parseTerminal(parser, t) == PARSERESULT_PASS;
#if 1
		if(t.value != 0)
		{
			parser.makeContext().visit(t);
		}
#endif
		return result;
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal_optional<ID>& t)
	{
		parseTerminal(parser, t);
#if 1
		if(t.value != 0)
		{
			parser.makeContext().visit(t);
		}
#endif
		return true;
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal_suffix<ID>& t)
	{
		skip = parseTerminal(parser, t) != PARSERESULT_PASS;
#if 1
		if(t.value != 0)
		{
			parser.makeContext().visit(t);
		}
#endif
		return !skip;
	}
};

template<typename ParserType, typename T>
inline T* parseSymbol(ParserType& parser, T* result, const False&)
{
	ParsingVisitor<ParserType> visitor(parser);
	if(!result->parse(visitor))
	{
#if 0
		std::cout << "rejected: '" << SYMBOL_NAME(T) << "'" << std::endl;
		printSequence(parser.lexer); // rejected tokens
#endif
		return visitor.skip ? result : 0;
	}
	return result;
}


#define GENERIC_ITERATE1(i, op) op(i);
#define GENERIC_ITERATE2(i, op) op(i); GENERIC_ITERATE1(i + 1, op)
#define GENERIC_ITERATE3(i, op) op(i); GENERIC_ITERATE2(i + 1, op)
#define GENERIC_ITERATE4(i, op) op(i); GENERIC_ITERATE3(i + 1, op)
#define GENERIC_ITERATE5(i, op) op(i); GENERIC_ITERATE4(i + 1, op)
#define GENERIC_ITERATE6(i, op) op(i); GENERIC_ITERATE5(i + 1, op)
#define GENERIC_ITERATE7(i, op) op(i); GENERIC_ITERATE6(i + 1, op)
#define GENERIC_ITERATE8(i, op) op(i); GENERIC_ITERATE7(i + 1, op)
#define GENERIC_ITERATE9(i, op) op(i); GENERIC_ITERATE8(i + 1, op)
#define GENERIC_ITERATE10(i, op) op(i); GENERIC_ITERATE9(i + 1, op)
#define GENERIC_ITERATE11(i, op) op(i); GENERIC_ITERATE10(i + 1, op)
#define GENERIC_ITERATE12(i, op) op(i); GENERIC_ITERATE11(i + 1, op)
#define GENERIC_ITERATE13(i, op) op(i); GENERIC_ITERATE12(i + 1, op)
#define GENERIC_ITERATE14(i, op) op(i); GENERIC_ITERATE13(i + 1, op)
#define GENERIC_ITERATE15(i, op) op(i); GENERIC_ITERATE14(i + 1, op)
#define GENERIC_ITERATE16(i, op) op(i); GENERIC_ITERATE15(i + 1, op)
#define GENERIC_ITERATE17(i, op) op(i); GENERIC_ITERATE16(i + 1, op)

template<typename T>
struct TypeListCount<TYPELIST1(cpp::ambiguity<T>)>
{
	enum { RESULT = 0 };
};

template<typename ParserType, typename T, size_t N>
struct ChoiceParser
{
};

#define DEFINE_CHOICEPARSER(N) \
	template<typename ParserType, typename T> \
	struct ChoiceParser<ParserType, T, N> \
	{ \
		static T* parseSymbol(ParserType& parser, T* result) \
		{ \
			GENERIC_ITERATE##N(0, CHOICEPARSER_OP) \
			return result; \
		} \
	}

#define CHOICEPARSER_OP(N) if(result = parseSymbolRequired(parser, NULLSYMBOL(TYPELIST_NTH(typename T::Choices, N)))) return result
DEFINE_CHOICEPARSER(1);
DEFINE_CHOICEPARSER(2);
DEFINE_CHOICEPARSER(3);
DEFINE_CHOICEPARSER(4);
DEFINE_CHOICEPARSER(5);
DEFINE_CHOICEPARSER(6);
DEFINE_CHOICEPARSER(7);
DEFINE_CHOICEPARSER(8);
DEFINE_CHOICEPARSER(9);
DEFINE_CHOICEPARSER(10);
DEFINE_CHOICEPARSER(11);
DEFINE_CHOICEPARSER(12);
DEFINE_CHOICEPARSER(13);
DEFINE_CHOICEPARSER(14);
DEFINE_CHOICEPARSER(15);
DEFINE_CHOICEPARSER(16);
DEFINE_CHOICEPARSER(17);
#undef CHOICEPARSER_OP


template<typename ParserType, typename T>
inline T* parseSymbol(ParserType& parser, T* result, const True&)
{
	return ChoiceParser<ParserType, T, TYPELIST_COUNT(typename T::Choices)>::parseSymbol(parser, result);
}

template<typename T>
struct IsChoice
{
	typedef True Result;
};

template<>
struct IsChoice<TypeListEnd>
{
	typedef False Result;
};

template<typename ParserType, typename T>
inline T* parseSymbol(ParserType& parser, T* result)
{
	return parseSymbol(parser, result, typename IsChoice<typename T::Choices>::Result());
}


// skips a braced token sequence
inline void skipBraced(Parser& parser)
{
	while(!TOKEN_EQUAL(parser, boost::wave::T_RIGHTBRACE))
	{
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
		if(TOKEN_EQUAL(parser, boost::wave::T_LEFTBRACE))
		{
			parser.increment();
			skipBraced(parser);
			parser.increment();
		}
		else
		{
			parser.increment();
		}
	}
}

// skips a parenthesised token sequence
inline void skipParenthesised(Parser& parser)
{
	while(!TOKEN_EQUAL(parser, boost::wave::T_RIGHTPAREN))
	{
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_SEMICOLON));
		if(TOKEN_EQUAL(parser, boost::wave::T_LEFTPAREN))
		{
			parser.increment();
			skipBraced(parser);
			parser.increment();
		}
		else
		{
			parser.increment();
		}
	}
}

// skips a template argument list
inline void skipTemplateArgumentList(Parser& parser)
{
	while(!TOKEN_EQUAL(parser, boost::wave::T_GREATER))
	{
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
		if(TOKEN_EQUAL(parser, boost::wave::T_LEFTPAREN))
		{
			parser.increment();
			skipParenthesised(parser);
			parser.increment();
		}
		else
		{
			parser.increment();
		}
	}
}

// skips a parenthesised expression
template<typename Declare>
struct SkipParenthesised
{
	Declare declare;
	SkipParenthesised(Declare declare) : declare(declare)
	{
	}
	inline void operator()(Parser& parser) const
	{
		while(!TOKEN_EQUAL(parser, boost::wave::T_RIGHTPAREN))
		{
			PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
			PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_SEMICOLON));

#if 1
			if(TOKEN_EQUAL(parser, boost::wave::T_CLASS)
				|| TOKEN_EQUAL(parser, boost::wave::T_STRUCT)
				|| TOKEN_EQUAL(parser, boost::wave::T_UNION))
			{
				// elaborated-type-specifier of the form 'class-key identifier' declares 'identifier' in enclosing scope
				parser.increment();
				PARSE_ASSERT(TOKEN_EQUAL(parser, boost::wave::T_IDENTIFIER));
				cpp::terminal_identifier id = { parser.get_value(), parser.get_position() };
				parser.increment();
				if(!TOKEN_EQUAL(parser, boost::wave::T_LESS) // template-id
					&& !TOKEN_EQUAL(parser, boost::wave::T_COLON_COLON)) // nested-name-specifier
				{
					declare(id);
				}
			}
			else 
#endif
			if(TOKEN_EQUAL(parser, boost::wave::T_LEFTPAREN))
			{
				parser.increment();
				(*this)(parser);
				parser.increment();
			}
			else
			{
				parser.increment();
			}
		}
	}
};

template<typename Declare>
inline SkipParenthesised<Declare> makeSkipParenthesised(Declare declare)
{
	return SkipParenthesised<Declare>(declare);
}

// skips a default-argument
template<typename IsTemplateName>
struct SkipDefaultArgument
{
	IsTemplateName isTemplateName;
	SkipDefaultArgument(IsTemplateName isTemplateName) : isTemplateName(isTemplateName)
	{
	}
	void operator()(Parser& parser) const
	{
		while(!TOKEN_EQUAL(parser, boost::wave::T_RIGHTPAREN)
			&& !TOKEN_EQUAL(parser, boost::wave::T_COMMA))
		{
			PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
			PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_SEMICOLON));

			if(TOKEN_EQUAL(parser, boost::wave::T_IDENTIFIER))
			{
				// may be template-name 
				cpp::terminal_identifier id = { parser.get_value(), parser.get_position() };
				parser.increment();
				if(TOKEN_EQUAL(parser, boost::wave::T_LESS)
					&& isTemplateName(id))
				{
					parser.increment();
					skipTemplateArgumentList(parser);
					parser.increment();
				}
			}
			else if(TOKEN_EQUAL(parser, boost::wave::T_LEFTPAREN))
			{
				parser.increment();
				skipParenthesised(parser);
				parser.increment();
			}
			else
			{
				parser.increment();
			}
		}
	}
};

template<typename IsTemplateName>
inline SkipDefaultArgument<IsTemplateName> makeSkipDefaultArgument(IsTemplateName isTemplateName)
{
	return SkipDefaultArgument<IsTemplateName>(isTemplateName);
}

// skips a mem-initializer-list
inline void skipMemInitializerClause(Parser& parser)
{
	parser.increment();
	while(!TOKEN_EQUAL(parser, boost::wave::T_LEFTBRACE))
	{
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_EOF));
		PARSE_ASSERT(!TOKEN_EQUAL(parser, boost::wave::T_SEMICOLON));
		parser.increment();
	}
}



#include <list>



template<typename ContextType, typename T>
struct DeferredParseThunk
{
	static void* thunk(const typename ContextType::Base& base, void* p)
	{
		ContextType walker(base);
		T* symbol = static_cast<T*>(p);
		return parseSymbol(walker.getParser(walker), symbol);
	}
};

template<typename ContextType>
struct DeferredParseBase
{
	typedef void* (*Func)(ContextType&, void*);
	ContextType context;
	void* symbol;
	Func func;
};

template<typename ContextType>
struct DeferredParse : public DeferredParseBase<ContextType>
{
	BacktrackBuffer buffer;

	// hack!
	DeferredParse(const DeferredParseBase<ContextType>& base)
		: buffer(), DeferredParseBase<ContextType>(base)
	{
	}
	DeferredParse(const DeferredParse& other)
		: buffer(), DeferredParseBase<ContextType>(other)
	{
	}
	DeferredParse& operator=(const DeferredParse& other)
	{
		if(&other != this)
		{
			this->~DeferredParse();
			new(this) DeferredParse(other);
		}
		return *this;
	}
};

struct ContextBase
{
	ParserOpaque* parser;
	void* result;

	template<typename ContextType>
	ParserGeneric<ContextType>& getParser(ContextType& context)
	{
		parser->context = &context;
		return *static_cast<ParserGeneric<ContextType>*>(parser);
	}
};


template<typename Walker, typename T>
inline DeferredParse<typename Walker::Base> makeDeferredParse(const Walker& context, T* symbol)
{
	DeferredParseBase<typename Walker::Base> result = { context, symbol, DeferredParse<typename Walker::Base>::Func(DeferredParseThunk<Walker, T>::thunk) };
	return result;
}

template<typename ListType>
inline void parseDeferred(ListType& deferred, ParserOpaque& parser)
{
	const Token* position = parser.lexer.position;
	for(ListType::iterator i = deferred.begin(); i != deferred.end(); ++i)
	{
		typename ListType::value_type& item = (*i);

		parser.lexer.history.swap(item.buffer);
		parser.lexer.position = parser.lexer.error = parser.lexer.history.tokens;
		item.context.parser = &parser;

		void* result = item.func(item.context, item.symbol);

		if(result == 0
			|| parser.lexer.position != parser.lexer.history.end() - 1)
		{
			printError(parser);
		}

		parser.lexer.history.swap(item.buffer);
	}
	parser.lexer.position = parser.lexer.error = position;
}

template<typename ListType, typename ContextType, typename T, typename Func>
inline T* defer(ListType& deferred, ContextType& walker, Func skipFunc, T* symbol)
{
	Parser& parser = *walker.parser;
	const Token* first = parser.lexer.position;

	skipFunc(parser);

	size_t count = ::distance(parser.lexer.history, first, parser.lexer.position);
	if(count != 0)
	{
		deferred.push_back(makeDeferredParse(walker, symbol));

		BacktrackBuffer buffer;
		buffer.resize(count + 2); // adding 1 for EOF and 1 to allow use as circular buffer
		for(const Token* p = first; p != parser.lexer.position; p = ::next(parser.lexer.history, p))
		{
			*buffer.position++ = *first++;
		}
		FilePosition nullPos = { "$null.cpp", 0, 0 };
		*buffer.position++ = Token(boost::wave::T_EOF, "", nullPos);

		deferred.back().buffer.swap(buffer);

		return symbol;
	}
	return 0;
}


cpp::declaration_seq* parseFile(ParserContext& lexer);
cpp::statement_seq* parseFunction(ParserContext& lexer);


#endif


