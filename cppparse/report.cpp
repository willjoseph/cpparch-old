
#include "report.h"
#include "symbols.h"
#include "printer.h"
#include "lexer.h"
#include "util.h"

#include <fstream>
#include <set>

#define REPORT_ASSERT ALLOCATOR_ASSERT
typedef AllocatorError ReportError;

#ifndef CPPTREE_VIRTUAL


void printSymbol(cpp::declaration_seq* p, const PrintSymbolArgs& args)
{
}

void printSymbol(cpp::statement_seq* p, const PrintSymbolArgs& args)
{
}

#else

template<typename OutputStreamType>
struct TreePrinter // TODO: better name
{
	OutputStreamType& out;
	bool visited;
	TreePrinter(OutputStreamType& out)
		: out(out), visited(false)
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
		if(typeid(T) != typeid(*symbol)) // if abstract
		{
			TREEWALKER_LEAF(symbol);
		}
		else
		{
			if(visited)
			{
				out << ", ";
			}
			visited = true;
			out << SYMBOL_NAME(*symbol);
#if 0 // don't print children
			out << '(';
			TreePrinter<OutputStreamType> tmp(out);
			symbol->accept(tmp);
			out << ')';
#endif
		}
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}

	void visit(cpp::declaration* symbol)
	{
	}

	void visit(cpp::member_declaration* symbol)
	{
	}

	void visit(cpp::statement* symbol)
	{
	}
};

typedef TokenPrinter<std::ofstream> FileTokenPrinter;

struct PrintingWalker
{
	FileTokenPrinter& printer;
	PrintingWalker(FileTokenPrinter& printer)
		: printer(printer)
	{
	}
#if 0
	template<typename T>
	void printSymbol(T* symbol)
	{
		SymbolPrinter walker(getState());
		TREEWALKER_WALK(walker, symbol);
	}
#endif
	void printName(Scope* scope)
	{
		if(scope != 0
			&& scope->parent != 0)
		{
			printName(scope->parent);
			printer.out << getValue(scope->name) << ".";
		}
	}

	void printName(Declaration* name)
	{
		if(name == 0)
		{
			printer.out << "<unknown>";
		}
		else
		{
			printName(name->scope);
			printer.out << getValue(name->getName());
		}
	}

	void printName(const char* caption, Declaration* type, Declaration* name)
	{
		printer.out << "/* ";
		printer.out << caption;
		printName(type);
		printer.out << ": ";
		printName(name);
		printer.out << " */";
	}

};

const char* escapeTerminal(LexTokenId id, const char* value)
{
	switch(id)
	{
	case boost::wave::T_LESS: return "&lt;";
	case boost::wave::T_LESSEQUAL: return "&lt;=";
	case boost::wave::T_SHIFTLEFT: return "&lt;&lt;";
	case boost::wave::T_SHIFTLEFTASSIGN: return "&lt;&lt;=";
	case boost::wave::T_AND_ALT:
	case boost::wave::T_AND: return "&amp;";
	case boost::wave::T_ANDAND_ALT:
	case boost::wave::T_ANDAND: return "&amp;&amp;";
	case boost::wave::T_ANDASSIGN_ALT:
	case boost::wave::T_ANDASSIGN: return "&amp;=";
	default: break;
	}

	return value;
}

template<LexTokenId id> 
const char* escapeTerminal(cpp::terminal<id> symbol)
{
	return escapeTerminal(id, symbol.value.c_str());
}


const char* escapeTerminal(cpp::terminal_choice2 symbol)
{
	return escapeTerminal(symbol.id, symbol.value.c_str());
}


bool isPrimary(const Identifier& id)
{
	// TODO: optimise
	return id.dec.p != 0 && id.position == id.dec.p->getName().position;
}

typedef std::pair<Name, Declaration*> ModuleDeclaration; // first=source, second=declaration

typedef std::set<ModuleDeclaration> DeclarationSet;

typedef std::map<Name, DeclarationSet> ModuleDependencyMap; // key=source

// recursively merges all source files in the \p graph into \p includes
void mergeIncludes(IncludeDependencyNodes& includes, const IncludeDependencyNode& graph)
{
	if(graph.empty())
	{
		return;
	}
	IncludeDependencyNodes tmp;

	std::set_difference(
		graph.begin(), graph.end(),
		includes.begin(), includes.end(),
		std::inserter(tmp, tmp.begin())
	);

	for(IncludeDependencyNode::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
	{
		includes.insert(*i);
		mergeIncludes(includes, *(*i));
	}
}

struct SymbolPrinter : PrintingWalker
{
	std::ofstream out;
	FileTokenPrinter printer;
	const char* root;
	const IncludeDependencyGraph& includeGraph;
	ModuleDependencyMap moduleDependencies;

	SymbolPrinter(const PrintSymbolArgs& args)
		: PrintingWalker(printer),
		printer(out),
		root(args.path),
		includeGraph(args.includeGraph)
	{
		includeStack.push(Name("$outer"));
		open(includeStack.top().c_str());
	}
	~SymbolPrinter()
	{
		while(!includeStack.empty())
		{
			pop();
		}
		REPORT_ASSERT(!out.is_open());
	}

	void open(const char* path)
	{
		REPORT_ASSERT(!out.is_open());
		out.open(OutPath(root, path).c_str());

		out << "<html>\n"
			"<head>\n"
			"<link rel='stylesheet' type='text/css' href='identifier.css'/>\n"
			"</style>\n"
			"</head>\n"
			"<body>\n"
			"<pre style='color:#000000;background:#ffffff;'>\n";
	}
	void close()
	{
		REPORT_ASSERT(out.is_open());

		out << "</pre>\n"
			"</body>\n"
			"</html>\n";

		out.close();
	}
	void suspend()
	{
		REPORT_ASSERT(out.is_open());
		out.close();
	}
	void resume(const char* path)
	{
		REPORT_ASSERT(!out.is_open());
		out.open(OutPath(root, path).c_str(), std::ios::app);
	}

	void visit(cpp::terminal_identifier symbol)
	{
		printer.printToken(boost::wave::T_IDENTIFIER, symbol.value.c_str());
	}

	void visit(cpp::terminal_string symbol)
	{
#if 1
		printer.formatToken(boost::wave::T_STRINGLIT);
		for(const char* p = symbol.value.c_str(); *p != '\0'; ++p)
		{
			char c = *p;
			switch(c)
			{
			case '"': printer.out << "&quot;"; break;
			case '&': printer.out << "&amp;"; break;
			case '<': printer.out << "&lt;"; break;
			case '>': printer.out << "&gt;"; break;
			default: printer.out << c; break;
			}
		}
#else
		printer.printToken(boost::wave::T_STRINGLIT, symbol.value.c_str());
#endif
	}

	void visit(cpp::terminal_choice2 symbol)
	{
		printer.printToken(symbol.id, escapeTerminal(symbol));
	}

	template<LexTokenId id>
	void visit(cpp::terminal<id> symbol)
	{
		if(!symbol.value.empty())
		{
			printer.printToken(id, escapeTerminal(symbol));
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

	typedef Stack<Name, 1024> NameStack;
	typedef NameStack IncludeStack;

	IncludeStack includeStack;

	void push()
	{
		REPORT_ASSERT(!includeStack.empty());
		if(includeStack.top() != NAME_NULL)
		{
			suspend();
		}
		includeStack.push(NAME_NULL);
	}

	bool isIncluded(const IncludeDependencyNodes& included, Name source)
	{
		IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(source);

		return i != includeGraph.includes.end() // should be impossible?
			&& included.find(&(*i)) != included.end();
	}

	bool isIncluded(const IncludeDependencyNodes& included, Declaration* declaration)
	{
		for(; declaration != 0; declaration = declaration->overloaded)
		{
			if(*declaration->getName().source.c_str() == '$' // '$outer'
				|| isIncluded(included, declaration->getName().source))
			{
				return true;
			}
		}
		return false;
	}

	void pop()
	{
		REPORT_ASSERT(!includeStack.empty());
		if(includeStack.top() != NAME_NULL)
		{
			close();

			out.open(Concatenate(makeRange(root), makeRange(findFilenameSafe(includeStack.top().c_str())), makeRange(".d")).c_str());

			bool warnings = false;

			bool isHeader = !string_equal_nocase(findExtension(includeStack.top().c_str()), ".inl");

			IncludeDependencyNodes included;
			{
				IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(includeStack.top());
				if(i != includeGraph.includes.end())
				{
					const IncludeDependencyNode& graph = *i;
					mergeIncludes(included, graph);
#if 1
					for(IncludeDependencyNodes::const_iterator i = graph.begin(); i != graph.end(); ++i)
					{
						printer.out << "direct: " << (*i)->name.c_str() << std::endl;
					}

					for(IncludeDependencyNodes::const_iterator i = included.begin(); i != included.end(); ++i)
					{
						printer.out << "indirect: " << (*i)->name.c_str() << std::endl;
					}
#endif
				}
			}
			{
				ModuleDependencyMap::const_iterator i = moduleDependencies.find(includeStack.top());
				if(i != moduleDependencies.end())
				{
					const DeclarationSet& d = (*i).second;
					for(DeclarationSet::const_iterator i = d.begin(); i != d.end(); ++i)
					{
						const ModuleDeclaration& declaration = *i;

						printer.out << (declaration.first != NAME_NULL ? declaration.first : Name("<unknown>")).c_str() << ": ";
						printName(declaration.second);
						printer.out << std::endl;

						if(declaration.first != NAME_NULL
							&& declaration.second != 0
							&& isHeader
							&& !isIncluded(included, declaration.second))
						{
							printer.out << "WARNING: depending on file that was not (in)directly included: " << declaration.first.c_str() << std::endl;
							warnings = true;
						}
					}
				}
			}
			{
				MacroDependencyMap::const_iterator i = includeGraph.macros.find(includeStack.top());
				if(i != includeGraph.macros.end())
				{
					const MacroDeclarationSet& d = (*i).second;
					for(MacroDeclarationSet::const_iterator i = d.begin(); i != d.end(); ++i)
					{
						const MacroDeclaration& declaration = *i;
						printer.out << declaration.first.c_str() << ": " << declaration.second << std::endl;

						if(*declaration.first.c_str() != '<' // <command line>
							&& isHeader
							&& !string_equal(declaration.second, "NULL")// TEMP HACK
							&& !isIncluded(included, declaration.first)) 
						{
							printer.out << "WARNING: depending on file that was not (in)directly included: " << declaration.first.c_str() << std::endl;
							warnings = true;
						}
					}
				}
			}
			out.close();

			if(warnings)
			{
				std::cout << "warnings found: " << includeStack.top().c_str() << std::endl;
			}
		}
		includeStack.pop();
		if(!includeStack.empty()
			&& includeStack.top() != NAME_NULL)
		{
			resume(includeStack.top().c_str());
		}
	}

	struct OutPath : public Concatenate
	{
		OutPath(const char* root, const char* path)
			: Concatenate(makeRange(root), makeRange(findFilenameSafe(path)), makeRange(".html"))
		{
		}
	};

	void visit(cpp::declaration* symbol)
	{
		for(unsigned short i = 0; i != symbol->events.pop; ++i)
		{
			pop();
		}
		for(unsigned short i = 0; i != symbol->events.push; ++i)
		{
			push();
		}
		if(includeStack.top() == NAME_NULL)
		{
			includeStack.top() = symbol->source;
			open(includeStack.top().c_str());

			{
				IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(symbol->source);
				if(i != includeGraph.includes.end())
				{
					const IncludeDependencyNode& d = *i;
					for(IncludeDependencyNode::const_iterator i = d.begin(); i != d.end(); ++i)
					{
						printer.out << "<a href='" << OutPath("", (*i)->name.c_str()).c_str() << "'>" << (*i)->name.c_str() << "</a>" << std::endl;
					}
				}
			}

		}

		symbol->accept(*this);
	}
#if 0
	void visit(cpp::declaration* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}

	void visit(cpp::member_declaration* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}

	void visit(cpp::statement* symbol)
	{
		symbol->accept(*this);
		printer.out << " // ";
		TreePrinter<std::ofstream> tmp(printer.out);
		symbol->accept(tmp);
	}
#endif

	void visit(cpp::identifier* symbol)
	{
		printer.formatToken(boost::wave::T_IDENTIFIER);

		const char* type = symbol->value.dec.p != 0 ? getDeclarationType(*symbol->value.dec.p) : "unknown";
		if(isPrimary(symbol->value))
		{
			printer.out << "<a name='";
			printName(symbol->value.dec.p);
			printer.out << "'></a>";
		}
		else
		{
			printer.out << "<a href='";
			if(symbol->value.dec.p != 0)
			{
				printer.out << OutPath(root, symbol->value.dec.p->getName().source.c_str()).c_str() + 4; // HACK! Remove 'out/' from path
			}
			printer.out << "#";
			printName(symbol->value.dec.p);
			printer.out << "'>";

			if(symbol->value.dec.p != 0
				&& !isNamespace(*symbol->value.dec.p)
				&& symbol->value.dec.p->getName().source != NAME_NULL // refers to a symbol declared in a module
				&& symbol->value.source != symbol->value.dec.p->getName().source) // refers to a symbol not declared in the current module
			{
				moduleDependencies[symbol->value.source].insert(ModuleDeclaration(symbol->value.dec.p->getName().source, symbol->value.dec.p));
			}
		}
		printer.out << "<" << type << ">";
		printer.out << getValue(symbol->value);
		printer.out << "</" << type << ">";
		if(!isPrimary(symbol->value))
		{
			printer.out << "</a>";
		}
	}

	void visit(cpp::simple_type_specifier_builtin* symbol)
	{
		printer.formatToken(symbol->value.id);

		printer.out << "<type>";
		printer.out << symbol->value.value.c_str();
		printer.out << "</type>";
	}
	void visit(cpp::primary_expression_builtin* symbol)
	{
		printer.formatToken(symbol->key.ID);

		printer.out << "<object>";
		printer.out << symbol->key.value.c_str();
		printer.out << "</object>";
	}
};




void printSymbol(cpp::declaration_seq* p, const PrintSymbolArgs& args)
{
	try
	{
		SymbolPrinter printer(args);
		printer.visit(makeSymbol(p));
	}
	catch(ReportError&)
	{
	}
}

void printSymbol(cpp::statement_seq* p, const PrintSymbolArgs& args)
{
	try
	{
		SymbolPrinter printer(args);
		printer.visit(makeSymbol(p));
	}
	catch(ReportError&)
	{
	}

}

#endif
