
#ifndef INCLUDED_CPPPARSE_PARSER_H
#define INCLUDED_CPPPARSE_PARSER_H

#include "lexer.h"
#include "cpptree.h"
#include <typeinfo>

#include "list.h"
#include <list>

struct ParseError
{
	ParseError()
	{
		__debugbreak();
	}
};

struct GeneralError
{
	GeneralError()
	{
	}
};

#define ASSERT(condition) if(!(condition)) { throw GeneralError(); }


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

#ifdef _DEBUG
//#define PARSER_DEBUG
#endif

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



typedef LinearAllocatorWrapper<int, struct ParserAllocator> DefaultParserAllocator;
inline struct ParserAllocator& NullParserAllocator();

class OpaqueCopied : DefaultParserAllocator
{
	void* p;
	void (*release)(void*);

	template<typename T>
	struct ReleaseGeneric
	{
		static void apply(void* p)
		{
			delete static_cast<T*>(p);
		}
	};

	static void releaseNull(void* p)
	{
	}

public:
	OpaqueCopied(const DefaultParserAllocator& allocator)
		: DefaultParserAllocator(*this), p(0), release(releaseNull)
	{
	}
	OpaqueCopied(const OpaqueCopied& other)
		: DefaultParserAllocator(other), p(0), release(releaseNull)
	{
		ASSERT(other.p == NULL);
	}
	OpaqueCopied& operator=(const OpaqueCopied& other)
	{
		ASSERT(p == NULL);
		ASSERT(other.p == NULL);
	}
	template<typename T>
	OpaqueCopied(const T& value, const DefaultParserAllocator& allocator)
		: DefaultParserAllocator(allocator), p(new T(value)), release(ReleaseGeneric<T>::apply)
	{
	}
	~OpaqueCopied()
	{
		release(p);
	}
	template<typename T>
	void get(T*& result)
	{
		// TODO
		//SEMANTIC_ASSERT(release == ReleaseGeneric<T>::apply);
		result = static_cast<T*>(p);
	}
};

template<typename Symbol, typename Walker>
struct Cached
{
	size_t count;
	size_t allocation;
	Symbol* symbol;
	Walker walker;
	Cached(Symbol* symbol, const Walker& walker)
		: symbol(symbol), walker(walker)
	{
	}
};

struct CachedSymbols
{
	typedef Lexer::Tokens::const_iterator Key;
	struct Value;
	typedef std::pair<Key, Value> Entry;
	typedef std::list<Entry, DefaultParserAllocator> Entries;

	struct Value
	{
		const type_info* type;
		Entries::iterator end;
		OpaqueCopied copied;
		Value() : copied(NullParserAllocator())
		{
		}
	};

	Entries entries;
	Entries::iterator position;
	size_t hits;

	static const size_t NONE = size_t(-1);

	CachedSymbols(const DefaultParserAllocator& allocator)
		: entries(allocator), position(entries.begin()), hits(0)
	{
	}
	~CachedSymbols()
	{
		if(hits > 0)
		{
			//std::cout << "total hits: " << hits << std::endl;
		}
	}

	template<typename T>
	void find(Key key, const T*& p)
	{
		// iterate through all entries at 'key'
		// in top-down grammar order: declarator-id -> id-expression -> unqualified-id -> template-id -> identifier
		for(Entries::iterator i = position; i != entries.begin();)
		{
			--i;
			if((*i).first != key)
			{
				break;
			}
			if((*i).second.type == &typeid(T)) // if this entry is the right type
			{
				++hits;
				Value& value = (*i).second;
				value.copied.get(p);
				entries.erase(++i, position); // discard entries that match a 'more abstract' symbol
				position = value.end;
				return;
			}
		}
		p = 0;
	}
	template<typename T>
	T& insert(Entries::iterator at, Key key, const T& t)
	{
		flush();
		Value& value = (*entries.insert(at, Entries::value_type(key, Value()))).second;
		position = entries.begin();
		value.type = &typeid(T);
		value.end = position;
		value.copied.~OpaqueCopied();
		new (&value.copied) OpaqueCopied(t, entries.get_allocator());
		T* p;
		value.copied.get(p);
		return *p;
	}
	void flush()
	{
#if 0//def _DEBUG
		if(entries.begin() != position)
		{
			Key key = position == entries.end() ? 
		}
#endif
		entries.erase(entries.begin(), position);
	}
};

typedef LinearAllocator<true> LexerAllocator;

struct BacktrackCallback
{
	typedef void (*Thunk)(void* data, LexerAllocator& allocator);
	Thunk thunk;
	void* data;

	void operator()(LexerAllocator& context) const
	{
		thunk(data, context);
	}
};

template<typename T, void (*op)(T* p, LexerAllocator& allocator)>
struct BacktrackCallbackThunk
{
	static void thunk(void* data, LexerAllocator& allocator)
	{
		op(static_cast<T*>(data), allocator);
	}
};

template<typename T>
inline void deleteObject(T* p, LexerAllocator& allocator)
{
	allocatorDelete(LinearAllocatorWrapper<int>(allocator), p);
}

template<typename T>
BacktrackCallback makeDeleteCallback(T* p)
{
	BacktrackCallback result = { BacktrackCallbackThunk<T, deleteObject>::thunk, p };
	return result;
}

template<typename T>
inline void destroyObject(T* p, LexerAllocator& allocator)
{
	LinearAllocatorWrapper<T>(allocator).destroy(p);
}

template<typename T>
BacktrackCallback makeDestroyCallback(T* p)
{
	BacktrackCallback result = { BacktrackCallbackThunk<T, destroyObject>::thunk, p };
	return result;
}

template<typename T>
inline void deallocateObject(T* p, LexerAllocator& allocator)
{
	LinearAllocatorWrapper<T>(allocator).deallocate(p, 1);
}

template<typename T>
BacktrackCallback makeDeallocateCallback(T* p)
{
	BacktrackCallback result = { BacktrackCallbackThunk<T, deallocateObject>::thunk, p };
	return result;
}


typedef std::pair<size_t, BacktrackCallback> BacktrackCallbacksElement;
typedef List<BacktrackCallbacksElement, DefaultParserAllocator> BacktrackCallbacks;


struct ParserAllocator : public LexerAllocator
{
	BacktrackCallbacks backtrackCallbacks;
	size_t position;
	CachedSymbols cachedSymbols;
	Callback1<size_t> onBacktrack;
	ParserAllocator()
		: position(0), backtrackCallbacks(DefaultParserAllocator(*this)), cachedSymbols(DefaultParserAllocator(*this))
	{
	}
	void addBacktrackCallback(size_t position, const BacktrackCallback& callback)
	{
		backtrackCallbacks.push_front(BacktrackCallbacksElement(position, callback));
	}
	void backtrack(size_t original)
	{
		ALLOCATOR_ASSERT(original <= position);
		position = original;
#if 0 // test: force immediate backtrack
		deferredBacktrack();
#endif
	}
	void* allocate(size_t size)
	{
		deferredBacktrack();
		void* result = LexerAllocator::allocate(size);
		position = LexerAllocator::position;
		return result;
	}
	void deferredBacktrack()
	{
		if(position == LexerAllocator::position)
		{
			return;
		}
#ifdef _DEBUG
		onBacktrack(LexerAllocator::position - position);
#endif

		cachedSymbols.flush();

		{
			BacktrackCallbacks::iterator i = backtrackCallbacks.begin();
			for(; i != backtrackCallbacks.end(); ++i)
			{
				if((*i).first < position)
				{
					break;
				}
				(*i).second(*this);
			}
			backtrackCallbacks.erase(backtrackCallbacks.begin(), i);
		}

		LexerAllocator::backtrack(position);
	}	
};


inline ParserAllocator& NullParserAllocator()
{
	static ParserAllocator null;
	return null;
}

#if 0
#define ParserAllocatorWrapper LinearAllocatorWrapper
#else
template<class T>
class ParserAllocatorWrapper
{
public:
	ParserAllocator& instance;

	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef const T* const_pointer;
	typedef const T& const_reference;

	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template<class OtherT>
	struct rebind
	{
		typedef ParserAllocatorWrapper<OtherT> other;
	};
	ParserAllocatorWrapper() : instance(NullParserAllocator())
	{
		throw AllocatorError();
	}
	ParserAllocatorWrapper(ParserAllocator& instance) : instance(instance)
	{
	}
	ParserAllocatorWrapper(const ParserAllocatorWrapper<T>& other) : instance(other.instance)
	{
	}
	template<class OtherT>
	ParserAllocatorWrapper(const ParserAllocatorWrapper<OtherT>& other) : instance(other.instance)
	{
	}
	template<class OtherT>
	ParserAllocatorWrapper<T>& operator=(const ParserAllocatorWrapper<OtherT>& other)
	{
		if(this != &other)
		{
			this->~ParserAllocatorWrapper();
			new(this) ParserAllocatorWrapper(other);
		}
		// do nothing!
		return (*this);
	}

	void deallocate(pointer p, size_type count)
	{
		//std::cout << "deallocate: " << p << std::endl; 
	}

	pointer allocate(size_type count)
	{
		size_t position = instance.position;
		pointer p = pointer(instance.allocate(count * sizeof(T)
#ifdef ALLOCATOR_DEBUG
			+ sizeof(size_t)
#endif
			));
#ifdef ALLOCATOR_DEBUG
		*reinterpret_cast<size_t*>(p + count) = position;
#endif
		//std::cout << "allocate: " << p << std::endl;
#ifdef ALLOCATOR_DEBUG
		*reinterpret_cast<size_t*>(p) = position;
		instance.addBacktrackCallback(position, makeDeallocateCallback(p));
#endif
		return p;
	}

	pointer allocate(size_type count, const void* hint)
	{
		return allocate(count);
	}

	void construct(pointer p, const T& value)
	{
#ifdef ALLOCATOR_DEBUG
		instance.addBacktrackCallback(*reinterpret_cast<size_t*>(p), makeDestroyCallback(p));
#endif
		new(p) T(value);
	}

	void destroy(pointer p)
	{
	}

	size_type max_size() const
	{
		size_type _Count = size_type(-1) / sizeof(T);
		return (0 < _Count ? _Count : 1);
	}
};

template<class T,
class OtherT>
	inline bool operator==(const ParserAllocatorWrapper<T>&, const ParserAllocatorWrapper<OtherT>&)
{
	return true;
}

template<class T,
class OtherT>
	inline bool operator!=(const ParserAllocatorWrapper<T>&, const ParserAllocatorWrapper<OtherT>&)
{
	return false;
}

#endif

struct ParserContext : Lexer
{
	Visualiser visualiser;
	ParserAllocator allocator;
	BacktrackStats stats;

	ParserContext(LexContext& context, const char* path)
		: Lexer(context, path)
	{
#ifdef _DEBUG
		allocator.onBacktrack = makeCallback(Member1<ParserContext, size_t, &ParserContext::onBacktrack>(*this));
#endif
	}

#ifdef _DEBUG
	void onBacktrack(size_t count)
	{
		if(count > stats.count)
		{
			stats.count = count;
			stats.symbol = Lexer::get_value();
			stats.position = Lexer::get_position();
		}
#if 0 // spam!
		printPosition(Lexer::get_position());
		std::cout << "backtrack: " << Lexer::get_value() << ": " << count << std::endl;
#endif
	}
#endif

	void backtrack(size_t count, const char* symbol = 0)
	{
		Lexer::backtrack(count, symbol);
	}
	void addBacktrackCallback(size_t position, const BacktrackCallback& callback)
	{
		allocator.addBacktrackCallback(position, callback);
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

struct Parser : public ParserState
{
	ParserContext& context;
	size_t position;
	size_t allocation;
	CachedSymbols::Entries::iterator cachePosition;

	Parser(ParserContext& context)
		: context(context), position(0), allocation(0)
	{
	}
	Parser(const Parser& other)
		: ParserState(other), context(other.context), position(0), allocation(context.allocator.position), cachePosition(context.allocator.cachedSymbols.position)
	{
	}

	LexTokenId get_id()
	{
		return context.get_id();
	}
	const char* get_value()
	{
		return context.get_value();
	}
	FilePosition get_position()
	{
		return context.get_position();
	}
	IncludeEvents get_events()
	{
		return context.get_events();
	}
	const char* get_source()
	{
		return context.get_source();
	}

	void increment()
	{
		++position;
		context.increment();
	}

	void backtrack(const char* symbol, bool preserveAllocation = false)
	{
		if(!preserveAllocation)
		{
			context.allocator.backtrack(allocation);
		}
		context.backtrack(position, symbol);
		context.allocator.cachedSymbols.position = cachePosition;
	}
	void advance()
	{
		if(position != 0)
		{
			context.advance(position);
		}
	}

	void addBacktrackCallback(const BacktrackCallback& callback)
	{
		context.allocator.addBacktrackCallback(allocation, callback);
	}

	template<typename Symbol, typename Walker>
	bool cacheLookup(Symbol*& symbol, Walker& walker)
	{
		CachedSymbols::Key key = context.position;
		const Cached<Symbol, Walker>* p = 0;
		context.allocator.cachedSymbols.find(context.position, p);
		if(p != 0)
		{
			symbol = p->symbol;
			walker.~Walker();
			new(&walker) Walker(p->walker);

			context.allocator.position = p->allocation;
			position = p->count;
			advance();
			return true;
		}
		ASSERT(context.position == key);
		return false;
	}
	template<typename Symbol, typename Walker>
	void cacheStore(CachedSymbols::Key key, Symbol* symbol, const Walker& walker)
	{
#if 1
		Cached<Symbol, Walker>& entry = context.allocator.cachedSymbols.insert(cachePosition, key, Cached<Symbol, Walker>(symbol, walker));
		entry.count = position;
		entry.allocation = context.allocator.position; // this must be stored after all allocations performed by cachedSymbols
#endif
	}
};

inline void printError(Parser& parser)
{
#if 0
	for(Lexer::Positions::const_iterator i = parser.context.backtrace.begin(); i != parser.context.backtrace.end(); ++i)
	{
	}
	printPosition(parser.context, context.history[parser.context.stacktrace.back()].position);
#endif
	printPosition(parser.context.getErrorPosition());
	std::cout << "syntax error: '" << parser.context.getErrorValue() << "'" << std::endl;
#if 1 // TODO!
	parser.context.visualiser.print(parser.context.history);
#endif
	printSequence(parser.context); // rejected tokens
}

inline void printSequence(Parser& parser)
{
	printSequence(parser.context, parser.position);
}

#define PARSE_ERROR() throw ParseError()
#define PARSE_ASSERT(condition) if(!(condition)) { PARSE_ERROR(); }

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
		&& !IsConvertible<T, cpp::terminal_choice>::RESULT; // this is true within specialised parseSymbol containing PARSE_SELECT_TOKEN
};

template<typename T, bool isConcrete = IsConcrete<T>::RESULT >
struct SymbolAllocator;

template<typename T>
struct SymbolAllocator<T, true>
{
	ParserContext& context;
	SymbolAllocator(ParserContext& context)
		: context(context)
	{
	}
	T* allocate()
	{
		return allocatorNew(ParserAllocatorWrapper<int>(context.allocator), T());
	}
	void deallocate(T* p)
	{
		allocatorDelete(ParserAllocatorWrapper<int>(context.allocator), p);
	}
};

template<typename T>
struct SymbolAllocator<T, false>
{
	SymbolAllocator(ParserContext& context)
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
	return allocatorNew(ParserAllocatorWrapper<int>(parser.context.allocator), T());
}

template<typename T, bool isConcrete = IsConcrete<T>::RESULT >
struct SymbolHolder;

template<typename T>
struct SymbolHolder<T, true>
{
	ParserContext& context;
	SymbolHolder(ParserContext& context)
		: context(context)
	{
	}
	T value;
	T* get()
	{
		return &value;
	}
	static T* hit(T* result, ParserContext& context)
	{
		return allocatorNew(ParserAllocatorWrapper<int>(context.allocator), T(*result));
	}
};

template<typename T>
struct SymbolHolder<T, false>
{
	SymbolHolder(ParserContext& context)
	{
	}
	T* get()
	{
		return 0;
	}
	static T* hit(T* result, ParserContext& context)
	{
		return result;
	}
};

template<typename ParserType, typename T>
T* parseHit(ParserType& parser, T* p)
{
	return SymbolHolder<T>::hit(p, parser.context);
}


inline bool checkBacktrack(Parser& parser)
{
	if(parser.context.maxBacktrack)
	{
		printError(parser);
		return true;
	}
	return false;
};

template<typename ParserType, typename T>
cpp::symbol<T> parseSymbolRequired(ParserType& parser, cpp::symbol<T> symbol)
{
#ifdef _DEBUG
	PARSE_ASSERT(symbol.p == 0);
	PARSE_ASSERT(!checkBacktrack(parser));
#endif
	ParserType tmp(parser);
#ifdef PARSER_DEBUG
	parser.context.visualiser.push(SYMBOL_NAME(T), parser.context.position);
#endif
	SymbolHolder<T> holder(parser.context);
#if 0
	T* result = parseSymbol(tmp, holder.get());
#else
	T* result = tmp.parse(holder.get());
#endif
	if(result != 0)
	{
#ifdef PARSER_DEBUG
		parser.context.visualiser.pop(result);
#endif
		parser.position += tmp.position;
		return cpp::symbol<T>(result);
	}

#ifdef PARSER_DEBUG
	parser.context.visualiser.pop(false);
#endif
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
inline cpp::symbol_sequence<T> parseSequence(ParserType& parser, cpp::symbol_sequence<T>)
{
	T tmp;
	cpp::symbol_next<T> p(&tmp);
	for(;;)
	{
		p->next = cpp::symbol_next<T>(parseSymbolRequired(parser, p->next));
		if(p->next == 0)
		{
			break;
		}
		p = p->next;
	}
	return cpp::symbol_sequence<T>(tmp.next);
}

#define PARSE_SEQUENCE(parser, p) p = parseSequence(parser, p)


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
	void* walker;
	ParserOpaque(ParserContext& context, void* walker)
		: Parser(context), walker(walker)
	{
	}
};

template<typename WalkerType>
class ParserGeneric : public ParserOpaque
{
	WalkerType& getWalker()
	{
		return *static_cast<WalkerType*>(ParserOpaque::walker);
	}
public:
	ParserGeneric(ParserContext& context, WalkerType& walker)
		: ParserOpaque(context, &walker)
	{
	}
	template<typename T>
	T* parse(T* symbol)
	{
		WalkerType& walker = getWalker();
		ParserOpaque* tmp = walker.parser;
		walker.parser = this; // pass parser as hidden argument to WalkerType::visit 
		walker.visit(symbol);
		walker.parser = tmp;
		return static_cast<T*>(walker.result);
	}
	template<LexTokenId ID>
	ParseResult parse(cpp::terminal<ID>& t)
	{
		ParseResult result = parseTerminal(*this, t);
		if(t.value != 0)
		{
			WalkerType& walker = getWalker();
			ParserOpaque* tmp = walker.parser;
			walker.parser = this; // pass parser as hidden argument to WalkerType::visit 
			walker.visit(t);
			walker.parser = tmp;
		}
		return result;
	}
};

#define SYMBOL_WALK(walker, symbol) if((result = symbol = parseSymbol(getParser(walker), symbol)) == 0) return; result = symbol = parseHit(getParser(walker), symbol)
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
	template<typename T>
	bool visit(cpp::symbol_sequence<T>& s)
	{
		s = parseSequence(parser, s);
		return true;
	}
	template<typename T>
	bool visit(cpp::symbol_next<T>& s)
	{
		return true; // this is handled by parseSequence
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal<ID>& t)
	{
		return parser.parse(t) == PARSERESULT_PASS;
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal_optional<ID>& t)
	{
		parser.parse(t);
		return true;
	}
	template<LexTokenId ID>
	bool visit(cpp::terminal_suffix<ID>& t)
	{
		skip = parser.parse(t) != PARSERESULT_PASS;
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
		printSequence(parser.context); // rejected tokens
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
			skipParenthesised(parser);
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



template<typename Walker, typename T>
struct DeferredParseThunk
{
	static void* thunk(typename Walker::Base& base, const typename Walker::State& state, void* p)
	{
		Walker walker(state);
		T* symbol = static_cast<T*>(p);
		void* result = parseSymbol(walker.getParser(walker), symbol);
		return result;
	}
};

template<typename Base, typename ContextType>
struct DeferredParseBase
{
	typedef void* (*Func)(Base&, const ContextType&, void*);
	typename ContextType context;
	void* symbol;
	Func func;
};

template<typename Base, typename ContextType>
struct DeferredParse : public DeferredParseBase<Base, ContextType>
{
	BacktrackBuffer buffer;

	// hack!
	DeferredParse(const DeferredParseBase<Base, ContextType>& base)
		: buffer(), DeferredParseBase<Base, ContextType>(base)
	{
	}
	DeferredParse(const DeferredParse& other)
		: buffer(), DeferredParseBase<Base, ContextType>(other)
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

	template<typename WalkerType>
	ParserGeneric<WalkerType>& getParser(WalkerType& walker)
	{
		parser->walker = &walker;  // pass walker as hidden argument to parseSymbol
		return *static_cast<ParserGeneric<WalkerType>*>(parser);
	}
};


template<typename Walker, typename T>
inline DeferredParse<typename Walker::Base, typename Walker::State> makeDeferredParse(const Walker& walker, T* symbol)
{
	DeferredParseBase<typename Walker::Base, typename Walker::State> result = { walker, symbol, DeferredParseThunk<Walker, T>::thunk };
	return result;
}

template<typename ListType, typename Walker>
inline void parseDeferred(ListType& deferred, Walker& walker)
{
	ParserOpaque& parser = *walker.parser;
	const Token* position = parser.context.position;
	for(typename ListType::iterator i = deferred.begin(); i != deferred.end(); ++i)
	{
		typename ListType::value_type& item = (*i);

		parser.context.history.swap(item.buffer);
		parser.context.position = parser.context.error = parser.context.history.tokens;
		item.context.parser = &parser;

		void* result = item.func(walker, item.context, item.symbol);

		if(result == 0
			|| parser.context.position != parser.context.history.end() - 1)
		{
			printError(parser);
		}

		parser.context.history.swap(item.buffer);
	}
	parser.context.position = parser.context.error = position;
}

template<typename ListType, typename ContextType, typename T, typename Func>
inline T* defer(ListType& deferred, ContextType& walker, Func skipFunc, T* symbol)
{
	Parser& parser = *walker.parser;
	const Token* first = parser.context.position;

	skipFunc(parser);

	size_t count = ::distance(parser.context.history, first, parser.context.position);
	if(count != 0)
	{
		T* result = parseHit(parser, symbol);
		deferred.push_back(makeDeferredParse(walker, result));

		BacktrackBuffer buffer;
		buffer.resize(count + 2); // adding 1 for EOF and 1 to allow use as circular buffer
		for(const Token* p = first; p != parser.context.position; p = ::next(parser.context.history, p))
		{
			*buffer.position++ = *p;
		}
		FilePosition nullPos = { "$null.cpp", 0, 0 };
		*buffer.position++ = Token(boost::wave::T_EOF, "", nullPos);

		deferred.back().buffer.swap(buffer);

		return result;
	}
	return 0;
}

#endif


