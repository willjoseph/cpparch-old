
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
		//__debugbreak();
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
		if(space && isIdentifier(*(*i).value.c_str()))
		{
			std::cout << " ";
		}
		std::cout << (*i).value.c_str();
		space = isIdentifier((*i).value.c_str()[strlen((*i).value.c_str()) - 1]);
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
	const DeclarationInstance* declaration;
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

cpp::terminal_identifier& getDeclarationId(const DeclarationInstance* declaration);

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
							std::cout << id.value.c_str() << " ";
							printPosition(id.source);
						}
						else
						{
							std::cout << (*p->position).value.c_str();
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
	void (*release)(DefaultParserAllocator&, void*);

	template<typename T>
	struct ReleaseGeneric
	{
		static void apply(DefaultParserAllocator& allocator, void* p)
		{
			allocatorDelete(allocator, static_cast<T*>(p));
		}
	};

	static void releaseNull(DefaultParserAllocator&, void* p)
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
		: DefaultParserAllocator(allocator), p(allocatorNew(allocator, T(value))), release(ReleaseGeneric<T>::apply)
	{
	}
	~OpaqueCopied()
	{
		release(*this, p);
	}
	template<typename T>
	void get(T*& result)
	{
		// TODO
		//SEMANTIC_ASSERT(release == ReleaseGeneric<T>::apply);
		result = static_cast<T*>(p);
	}
};

template<typename Symbol, typename SemaT>
struct Cached
{
	size_t count;
	size_t allocation;
	Symbol* symbol;
	SemaT walker;
	Cached(Symbol* symbol, const SemaT& walker)
		: symbol(symbol), walker(walker)
	{
	}
};

template<typename T>
struct TypeInfo
{
	static const std::type_info* id;
};
template<typename T>
const std::type_info* TypeInfo<T>::id = &typeid(T);

template<typename T>
struct Opaque
{
	char data[sizeof(T)];
};

struct CachedSymbols
{
	typedef Lexer::Tokens::const_iterator Key;
	struct Value;
	typedef std::pair<Key, Value> Entry;
	typedef std::list<Entry, DefaultParserAllocator> Entries;

#ifdef _STLP_DEBUG
	struct Position
	{
		typedef Opaque<Entries::iterator> Value;
		Value value;
		Position()
		{
		}
		Position(const Entries::iterator& i)
			: value(*reinterpret_cast<const Value*>(&i))
		{
		}
		operator const Entries::iterator&() const
		{
			return *reinterpret_cast<const Entries::iterator*>(&value);
		}
	};
#else
	typedef Entries::iterator Position;
#endif

	struct Value
	{
		const std::type_info* type;
		Position end;
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
			if((*i).second.type == TypeInfo<T>::id) // if this entry is the right type
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
		value.type = TypeInfo<T>::id;
		value.end = position;
		value.copied.~OpaqueCopied();
		new (&value.copied) OpaqueCopied(t, entries.get_allocator());
		T* p;
		value.copied.get(p);
		return *p;
	}
	void flush()
	{
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
			stats.symbol = "$symbol";//Lexer::get_value();
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
	CachedSymbols::Position cachePosition;

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
	const TokenValue& get_value()
	{
		return context.get_value();
	}
	const FilePosition& get_position()
	{
		return context.get_position();
	}
	IncludeEvents get_events()
	{
		return context.get_events();
	}
	const Source& get_source()
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

	template<typename Symbol, typename SemaT>
	bool cacheLookup(Symbol*& symbol, SemaT& walker)
	{
		CachedSymbols::Key key = context.position;
		const Cached<Symbol, SemaT>* p = 0;
		context.allocator.cachedSymbols.find(context.position, p);
		if(p != 0)
		{
			symbol = p->symbol;
			walker.~SemaT();
			new(&walker) SemaT(p->walker);

			context.allocator.position = p->allocation;
			position = p->count;
			advance();
			return true;
		}
		ASSERT(context.position == key);
		return false;
	}
	template<typename Symbol, typename SemaT>
	void cacheStore(CachedSymbols::Key key, Symbol* symbol, const SemaT& walker)
	{
#if 1
		Cached<Symbol, SemaT>& entry = context.allocator.cachedSymbols.insert(cachePosition, key, Cached<Symbol, SemaT>(symbol, walker));
		entry.count = position;
		entry.allocation = context.allocator.position; // this must be stored after all allocations performed by cachedSymbols
#endif
	}
};

inline void printError(ParserContext& context)
{
#if 0
	for(Lexer::Positions::const_iterator i = context.backtrace.begin(); i != context.backtrace.end(); ++i)
	{
	}
	printPosition(context, context.history[context.stacktrace.back()].position);
#endif
	printPosition(context.getErrorPosition());
	std::cout << "syntax error: '" << context.getErrorValue().c_str() << "'" << std::endl;
#if 1 // TODO!
	context.visualiser.print(context.history);
#endif
	printSequence(context); // rejected tokens
}

inline void printError(Parser& parser)
{
	printError(parser.context);
}

inline void printSequence(Parser& parser)
{
	printSequence(parser.context, parser.position);
}

#define PARSE_ERROR() throw ParseError()
#define PARSE_ASSERT(condition) if(!(condition)) { PARSE_ERROR(); }


template<typename T>
inline cpp::symbol_required<T> makeSymbolRequired(T* p)
{
	return cpp::symbol_required<T>(p);
}

template<typename T>
inline cpp::symbol_required<T>& checkSymbolRequired(cpp::symbol_required<T>& symbol)
{
	return symbol;
}

template<typename T>
inline cpp::symbol_optional<T> makeSymbolOptional(T* p)
{
	return cpp::symbol_optional<T>(p);
}

template<typename T>
inline cpp::symbol_optional<T>& checkSymbolOptional(cpp::symbol_optional<T>& symbol)
{
	return symbol;
}

template<typename T>
inline cpp::symbol_sequence<T>& checkSymbolSequence(cpp::symbol_sequence<T>& symbol)
{
	return symbol;
}

#define NULLSYMBOL(T) cpp::symbol_required< T >(0)
#define NULLPTR(T) static_cast<T*>(0)


template<typename T>
struct IsConcrete
{
	static const bool RESULT = !IsConvertible<T, cpp::choice<T> >::RESULT
		&& !IsConvertible<T, cpp::terminal_choice>::RESULT; // this is true within specialised parseSymbol containing PARSE_SELECT_TOKEN
};

#if 0
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
#endif

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

template<typename T>
T* parseHit(Parser& parser, T* p)
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

inline bool parseTerminal(Parser& parser, LexTokenId id, TokenValue& value)
{
	if(isToken(parser.get_id(), id))
	{
		value = parser.get_value();
		parser.increment();
		return true;
	}
	return false;
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
	return parseTerminal(parser, getTokenId(result), result.value) ? PARSERESULT_PASS : PARSERESULT_FAIL;
}

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal_optional<id>& result)
{
	parseTerminal(parser, getTokenId(result), result.value);
	return PARSERESULT_PASS;
}

template<LexTokenId id>
inline ParseResult parseTerminal(Parser& parser, cpp::terminal_suffix<id>& result)
{
	return parseTerminal(parser, getTokenId(result), result.value) ? PARSERESULT_PASS : PARSERESULT_SKIP;
}


struct CachedNull
{
};

struct CachedLeaf
{
};

struct CachedWalk
{
};

typedef CachedNull DisableCache;
typedef CachedLeaf EnableCache;

struct DeferDefault
{
	template<typename SemaT>
	static bool isDeferredParse(SemaT& walker)
	{
		return false;
	}
	template<typename SemaT, typename T>
	static T* addDeferredParse(SemaT& walker, T* symbol)
	{
		return 0;
	}
};


struct ParserOpaque : public Parser
{
	void* walker;
	ParserOpaque(ParserContext& context, void* walker)
		: Parser(context), walker(walker)
	{
	}
};






template<typename SemaT, typename T>
struct DeferredParseThunk
{
	static void* thunk(ParserContext& context, const typename SemaT::State& state, void* p)
	{
		SemaT walker(state);
		T* symbol = static_cast<T*>(p);
		ParserGeneric<SemaT> parser(context, walker);
		void* result = makeParser(symbol).parseSymbol(parser, symbol);
		return result;
	}
};

template<typename ContextType>
struct DeferredParseBase
{
	typedef void* (*Func)(ParserContext&, const ContextType&, void*);
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
	template<typename SemaT>
	ParserGeneric<SemaT>& getParser(SemaT& walker, ParserOpaque* parser)
	{
		parser->walker = &walker;  // pass walker as hidden argument to parseSymbol
		return *static_cast<ParserGeneric<SemaT>*>(parser);
	}
};


template<typename SemaT, typename T>
inline DeferredParse<typename SemaT::State> makeDeferredParse(const SemaT& walker, T* symbol)
{
	DeferredParseBase<typename SemaT::State> result = { walker, symbol, DeferredParseThunk<SemaT, T>::thunk };
	return result;
}

template<typename ListType>
inline void parseDeferred(ListType& deferred, ParserContext& context)
{
	const Token* position = context.position;
	for(typename ListType::iterator i = deferred.begin(); i != deferred.end(); ++i)
	{
		typename ListType::value_type& item = (*i);

		context.history.swap(item.buffer);
		context.position = context.error = context.history.tokens;

		void* result = item.func(context, item.context, item.symbol);

		if(result == 0
			|| context.position != context.history.end() - 1)
		{
			printError(context);
		}

		context.history.swap(item.buffer);
	}
	context.position = context.error = position;
}

template<typename ListType, typename ContextType, typename T, typename Func>
inline T* addDeferredParse(Parser& parser, ListType& deferred, ContextType& walker, Func skipFunc, T* symbol)
{
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
		*buffer.position++ = Token(boost::wave::T_EOF, TokenValue(), nullPos);

		deferred.back().buffer.swap(buffer);

		return result;
	}
	return 0;
}


template<typename SemaT>
class ParserGeneric : public ParserOpaque
{
public:
	bool skip;
	ParserGeneric(ParserContext& context, SemaT& walker)
		: ParserOpaque(context, &walker), skip(false)
	{
	}
	SemaT& getWalker()
	{
		return *static_cast<SemaT*>(ParserOpaque::walker);
	}

	template<typename T, typename Base>
	inline cpp::symbol<Base> parseExpression(cpp::symbol_required<T> symbol, Base* result)
	{
		// HACK: create temporary copy of expression-symbol to get RHS-symbol
		T tmp;
		if(parse(checkSymbolRequired(tmp.right)))
		{
			result = tmp.right;
			for(;;)
			{
				// parse suffix of expression-symbol
				symbol = NULLSYMBOL(T);
				if(!parse(symbol)
					|| symbol->right == 0)
				{
					break;
				}
				symbol->left = makeSymbolRequired(result);
				result = symbol;
			}
			return makeSymbol(result);
		}
		return cpp::symbol<Base>(0);
	}

	template<LexTokenId ID>
	ParseResult parseTerminal(cpp::terminal<ID>& t)
	{
		ParseResult result = ::parseTerminal(*this, t);
		if(!t.value.empty())
		{
			SemaT& walker = getWalker();
			semaAction(walker, t);
		}
		return result;
	}

	template<typename Inner>
	static ParserGeneric<Inner>& getParser(Inner& walker, ParserOpaque* parser)
	{
		parser->walker = &walker;  // pass walker as hidden argument to parseSymbol
		return *static_cast<ParserGeneric<Inner>*>(parser);
	}

	template<typename Symbol>
	bool cacheLookup(const CachedNull&, Symbol*& symbol, SemaT& walker)
	{
		return false;
	}
	template<typename Symbol>
	void cacheStore(const CachedNull&, CachedSymbols::Key key, Symbol* symbol, const SemaT& walker)
	{
	}
	template<typename Symbol>
	bool cacheLookup(const CachedLeaf&, Symbol*& symbol, SemaT& walker)
	{
		int dummy;
		return Parser::cacheLookup(symbol, dummy);
	}
	template<typename Symbol>
	void cacheStore(const CachedLeaf&, CachedSymbols::Key key, Symbol* symbol, const SemaT& walker)
	{
		int dummy;
		Parser::cacheStore(key, symbol, dummy);
	}
	template<typename Symbol>
	bool cacheLookup(const CachedWalk&, Symbol*& symbol, SemaT& walker)
	{
		return Parser::cacheLookup(symbol, walker);
	}
	template<typename Symbol>
	void cacheStore(const CachedWalk&, CachedSymbols::Key key, Symbol* symbol, const SemaT& walker)
	{
		Parser::cacheStore(key, symbol, walker);
	}

	bool isDeferredParse(const DeferDefault&, SemaT& walker)
	{
		return false;
	}
	template<typename T>
	T* addDeferredParse(const DeferDefault&, SemaT& walker, T* symbol)
	{
		return 0;
	}
	template<typename Defer>
	bool isDeferredParse(const Defer&, SemaT& walker)
	{
		return Defer::isDeferredParse(walker);
	}
	template<typename Defer, typename T>
	T* addDeferredParse(const Defer&, SemaT& walker, T* symbol)
	{
		return ::addDeferredParse(*this, Defer::getDeferredSymbolsList(walker), walker, Defer::getSkipFunc(walker), symbol);
	}

	template<typename T, typename InnerWalker, typename Inner, typename Annotate, typename Action, typename Cache, typename Defer>
	T* visit(SemaT& walker, T* symbol, const InnerWalker& innerConst, const Inner& inner, const Annotate&, const Action& action, const Cache& cache, const Defer& defer)
	{
		InnerWalker& innerWalker = *const_cast<InnerWalker*>(&innerConst); // 'innerConst' is a temporary with lifetime longer than this function.
		ParserGeneric<InnerWalker>& innerParser = getParser(innerWalker, this);
		typename Annotate::Data data = Annotate::makeData(*context.position);
		if(innerParser.isDeferredParse(defer, innerWalker)) // if the parse of this symbol is to be deferred (e.g. body of an inline member function)
		{
			symbol = innerParser.addDeferredParse(defer, innerWalker, symbol); // skip the tokens that comprise the symbol, add an entry to the appropriate deferred parse list
			if(symbol == 0)
			{
				return 0; // indicate that the symbol was not matched because there were no tokens to skip (e.g. empty function body)
			}
		}
		// if enabled, check whether the symbol was previously found by a failed production that shares the same prefix
		else if(!innerParser.cacheLookup(cache, symbol, innerWalker)) // if found, skip the tokens that comprise the symbol, and restore state of both symbol and innerWalker!
		{
			CachedSymbols::Key key = context.position; // record the location of the symbol we are about to parse
			symbol = makeParser(symbol).parseSymbol(innerParser, symbol);
			if(symbol == 0)
			{
				return 0;
			}
			symbol = parseHit(*this, symbol); // if the parse succeeded, return a persistent version of the symbol.
			semaCommit(innerWalker, inner); // if enabled, call the sema object's commit()
			innerParser.cacheStore(cache, key, symbol, innerWalker); // if enabled, save the state of both symbol and innerWalker for later re-use
		}
		// Report success if the parse was successful and the current walker's associated semantic action also passed.
		bool success = Action::invokeAction(walker, symbol, innerWalker);
		if(success)
		{
			Annotate::annotate(symbol, data);
			return symbol;
		}
		return 0;
	}	

	template<typename T, bool required>
	bool parse(cpp::symbol_generic<T, required>& s)
	{
#ifdef _DEBUG
		PARSE_ASSERT(s.p == 0);
		PARSE_ASSERT(!checkBacktrack(*this));
#endif
		ParserGeneric<SemaT> tmp(*this);
#ifdef PARSER_DEBUG
		context.visualiser.push(SYMBOL_NAME(T), context.position);
#endif
		SymbolHolder<T> holder(context);
		SemaT& walker = getWalker();

		// Construct an inner walker from the current walker, based on the current walker's policy for this symbol.
		// Try to parse the symbol using the inner walker.
		T* result = tmp.visit(walker, holder.get(),
			makeInnerWalker(walker, walker.makePolicy(NULLPTR(T)).getInnerPolicy()),
			walker.makePolicy(NULLPTR(T)).getInnerPolicy(),
			walker.makePolicy(NULLPTR(T)).getAnnotatePolicy(),
			walker.makePolicy(NULLPTR(T)).getActionPolicy(),
			walker.makePolicy(NULLPTR(T)).getCachePolicy(),
			walker.makePolicy(NULLPTR(T)).getDeferPolicy());

		if(result != 0)
		{
#ifdef PARSER_DEBUG
			context.visualiser.pop(result);
#endif
			position += tmp.position;
			s = cpp::symbol_generic<T, required>(result);
			return true;
		}
#ifdef PARSER_DEBUG
		context.visualiser.pop(false);
#endif
		tmp.backtrack(SYMBOL_NAME(T));
		s = cpp::symbol_generic<T, required>(0);
		return !required;
	}
	template<typename T>
	bool parse(cpp::symbol_sequence<T>& s)
	{
		T tmp;
		cpp::symbol_next<T> p(&tmp);
		for(;;)
		{
			cpp::symbol_required<T> next = NULLSYMBOL(T);
			if(!parse(next))
			{
				break;
			}
			p->next = cpp::symbol_next<T>(next);
			p = p->next;
		}
		s = cpp::symbol_sequence<T>(tmp.next);
		return true;
	}
	template<typename T>
	bool parse(cpp::symbol_next<T>& s)
	{
		return true; // this is handled by parse(cpp::symbol_sequence<T>)
	}
	template<LexTokenId ID>
	bool parse(cpp::terminal<ID>& t)
	{
		return parseTerminal(t) == PARSERESULT_PASS;
	}
	template<LexTokenId ID>
	bool parse(cpp::terminal_optional<ID>& t)
	{
		parseTerminal(t);
		return true;
	}
	template<LexTokenId ID>
	bool parse(cpp::terminal_suffix<ID>& t)
	{
		skip = parseTerminal(t) != PARSERESULT_PASS;
		return !skip;
	}
};


#define TOKEN_EQUAL(parser, token) isToken((parser).get_id(), token)
// TODO: avoid dependency on 'result'
#define PARSE_TERMINAL(parser, t) switch(parseTerminal(parser, t)) { case PARSERESULT_FAIL: return 0; case PARSERESULT_SKIP: return result; default: break; }

#define PARSE_TOKEN_REQUIRED(parser, token_) if(TOKEN_EQUAL(parser, token_)) { parser.increment(); } else { return 0; }
#define PARSE_TOKEN_OPTIONAL(parser, result, token) result = false; if(TOKEN_EQUAL(parser, token)) { result = true; parser.increment(); }
#define PARSE_SELECT_TOKEN(parser, p, token, value_) if(TOKEN_EQUAL(parser, token)) { p = createSymbol(parser, p); p->id = value_; p->value.id = token; p->value.value = parser.get_value(); parser.increment(); return p; }
#define PARSE_OPTIONAL(parser, p) parser.parse(checkSymbolOptional(p))
#define PARSE_REQUIRED(parser, p) if(parser.parse(checkSymbolRequired(p)) == 0) { return 0; }
#define PARSE_SELECT(parser, Type) { cpp::symbol_required<Type> result; if(parser.parse(result)) { return result; } }
#define PARSE_SELECT_UNAMBIGUOUS PARSE_SELECT

#if 0
// Type must have members 'left' and 'right', and 'typeof(left)' must by substitutable for 'Type'
#define PARSE_PREFIX(parser, Type) if(cpp::symbol_required<Type> p = parser.parseSymbolRequired(NULLSYMBOL(Type))) { if(p->right == 0) return p->left; return p; }
#endif

#if 0
#define PARSE_EXPRESSION PARSE_PREFIX
#else
#define PARSE_EXPRESSION PARSE_SELECT
#endif

#define PARSE_EXPRESSION_LEFTASSOCIATIVE(parser, Type) result = parser.parseExpression(NULLSYMBOL(Type), result)
#define PARSE_SEQUENCE(parser, p) parser.parse(checkSymbolSequence(p))


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


template<typename T, size_t N>
struct ChoiceParser
{
};

#define DEFINE_CHOICEPARSER(N) \
	template<typename T> \
	struct ChoiceParser<T, N> \
	{ \
		template<typename SemaT> \
		static T* parseSymbol(ParserGeneric<SemaT>& parser, T* result) \
		{ \
			GENERIC_ITERATE##N(0, CHOICEPARSER_OP) \
			return result; \
		} \
	}

#define CHOICEPARSER_OP(N) { cpp::symbol_required<TYPELIST_NTH(typename T::Choices, N)> result; if(parser.parse(result)) return result; }
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


template<typename T, typename Choices>
struct MakeParser
{
	typedef ChoiceParser<T, TYPELIST_COUNT(typename T::Choices)> Type;
};

struct DefaultParser;

template<typename T>
struct MakeParser<T, TypeListEnd>
{
	typedef DefaultParser Type;
};

template<typename T>
typename MakeParser<T, typename T::Choices>::Type makeParser(T*)
{
	return typename MakeParser<T, typename T::Choices>::Type();
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
				cpp::terminal_identifier id = { parser.get_value(), parser.get_source() };
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
// We cannot correctly skip a template-id in a default-argument if it refers to a template declared later in the class.
// This is considered to be correct: http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_active.html#325
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
				cpp::terminal_identifier id = { parser.get_value(), parser.get_source() };
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


#endif


