
#include "report.h"
#include "symbols.h"
#include "printer.h"
#include "lexer.h"
#include "util.h"
#include <boost/filesystem.hpp>

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
	return isDecorated(id) && id.position == id.dec.p->name->position;
}


typedef std::pair<Name, DeclarationInstance> ModuleDeclaration; // first=source, second=declaration
typedef std::set<ModuleDeclaration> ModuleDeclarationSet;
const ModuleDeclarationSet MODULEDECLARATIONSET_NULL = ModuleDeclarationSet();
typedef std::map<Name, ModuleDeclarationSet> ModuleDependencyMap; // key=source

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

void addModuleDependency(ModuleDependencyMap& moduleDependencies, const Identifier& id, const DeclarationInstance& instance)
{
	if(instance.name->source != NAME_NULL // refers to a symbol declared in a module
		&& !id.dec.deferred // name resolution not deferred
		&& id.source != instance.name->source) // refers to a symbol not declared in the current module
	{
		moduleDependencies[id.source].insert(ModuleDeclarationSet::value_type(instance.name->source, instance));
	}
}

inline bool isObjectDefinition(const Declaration& declaration)
{
	// [basic.def]
	// A declaration is a definition unless it declares a function without specifying the function's body, it
	// contains the extern specifier or a linkage specification and neither an initializer nor a
	// function body, it declares a static data member in a class declaration, it is a class name declaration,
	// or it is a typedef declaration, a using declaration, or a using-directive.
	return isObject(declaration)
		&& !(isFunction(declaration) && !declaration.isFunctionDefinition)
		&& !declaration.specifiers.isExtern
		&& !declaration.specifiers.isTypedef
		// TODO: linkage specification
		&& !isStaticMember(declaration);
}

inline bool isFunctionParameter(const Declaration& declaration)
{
	return declaration.scope != 0 && declaration.scope->type == SCOPETYPE_PROTOTYPE;
}

struct DependencyBuilder
{
	ModuleDependencyMap moduleDependencies;

	std::vector<Declaration*> functionStack;

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
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}

	void visit(cpp::identifier* symbol)
	{
		if(!isDecorated(symbol->value))
		{
			return;
		}
		const DeclarationInstance& instance = getDeclaration(symbol->value);
		if(!isPrimary(symbol->value)) // if this is not the identifier in an actual declaration
		{
			if(!isNamespace(*instance))
			{
				// the source file containing this symbol depends on all the redeclarations (of the declaration chosen by name resolution) that were visible
				// e.g. function redeclarations (excluding unchosen overloads), class forward-declarations (excluding unchosen explicit/partial-specializations)
				
				// add the most recent redeclaration
				addModuleDependency(moduleDependencies, symbol->value, instance);
			}
		}
		else
		{
			if(isObjectDefinition(*instance) // if this declaration defines an object
				&& !(isFunctionParameter(*instance) // and the object is not a function parameter..
					&& (!isDecorated(instance->scope->name) || !getDeclaration(instance->scope->name)->isFunctionDefinition))) // .. within a function declaration
			{
				UniqueTypeId type = makeUniqueType(instance->type, 0, true);
				if(isFunctionParameter(*instance))
				{
					type = adjustFunctionParameter(type);
				}
				if(type.isSimple()
					|| (type.isSimpleArray() && getArrayType(type.value).size != 0))
				{
					// the source file containing the definition depends on the type of the definition
					DeclarationInstance type = DeclarationInstance(instance->type.declaration);
					addModuleDependency(moduleDependencies, symbol->value, type);
				}
			}
		}
	}
};

void createDirectories(const char* path)
{
	try
	{
		boost::filesystem::create_directories(boost::filesystem::path(path).remove_filename());
	}
	catch(boost::filesystem::filesystem_error& e)
	{
		std::cerr << e.what() << std::endl;
		throw ReportError();
	}
}

StringRange getDrive(const char* path)
{
	if(*path == '\0'
		|| *(path + 1) != ':')
	{
		return StringRange(0, 0);
	}
	return StringRange(path, path + 1);
}

const char* getWorkingDirectory()
{
	static std::string working = boost::filesystem::initial_path().string() + "/";
	return working.c_str();
}

StringRange getRoot(const char* path)
{
	if(*path == '\0'
		|| *(path + 1) != ':')
	{
		return makeRange(getWorkingDirectory());
	}
	return StringRange(0, 0);
}


const char* removeDrive(const char* path)
{
	if(*path == '\0'
		|| *(path + 1) != ':')
	{
		return path;
	}
	return path + 2;
}

bool pathStartsWith(const char* path, const char* root)
{
	return std::equal(root, root + strlen(root), path);
}

struct AbsolutePath : public Concatenate
{
	AbsolutePath(const char* path)
		: Concatenate(getRoot(path), makeRange(path))
	{
	}
};

struct EscapedPath : public Concatenate
{
	EscapedPath(const char* path)
		: Concatenate(getDrive(path), makeRange(removeDrive(path)))
	{
	}
};


struct SourcePrinter : SymbolPrinter
{
	std::ofstream out;
	FileTokenPrinter printer;
	const char* inputRoot;
	const char* outputRoot;
	const IncludeDependencyGraph& includeGraph;
	ModuleDependencyMap& moduleDependencies;

	struct OutPath : public Concatenate
	{
		OutPath(const char* outputRoot, const char* path)
			: Concatenate(makeRange(AbsolutePath(outputRoot).c_str()), makeRange(EscapedPath(path).c_str()), makeRange(".html"))
		{
		}
	};

	SourcePrinter(const PrintSymbolArgs& args, ModuleDependencyMap& dependencies)
		: SymbolPrinter(printer),
		printer(out),
		inputRoot(args.inputRoot),
		outputRoot(args.outputRoot),
		includeGraph(args.includeGraph),
		moduleDependencies(dependencies)
	{
		includeStack.push(Source(Name(""), Name("$outer")));
		open(includeStack.top().c_str());
	}
	~SourcePrinter()
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
		OutPath tmp(outputRoot, path);
		createDirectories(tmp.c_str());
		out.open(tmp.c_str());

		out << "<html>\n"
			"<head>\n"
			"<link rel='stylesheet' type='text/css' href='file://" << AbsolutePath(outputRoot).c_str() << "../identifier.css'/>\n"
			"</style>\n"
			"</head>\n"
			"<body>\n"
			"<pre style='color:#000000;background:#ffffff;'>\n";

		out << "source: <a href='" << path << "'>" << path << "</a>\n";
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
		out.open(OutPath(outputRoot, path).c_str(), std::ios::app);
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

	typedef Stack<Source, 1024> IncludeStack;

	IncludeStack includeStack;

	void push()
	{
		REPORT_ASSERT(!includeStack.empty());
		if(includeStack.top() != SOURCE_NULL)
		{
			suspend();
		}
		includeStack.push(SOURCE_NULL);
	}

	bool isIncluded(const IncludeDependencyNodes& included, Name source)
	{
		IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(source);

		return i != includeGraph.includes.end() // should be impossible?
			&& included.find(&(*i)) != included.end();
	}

	bool isIncluded(const IncludeDependencyNodes& included, const DeclarationInstance& instance)
	{
		for(const DeclarationInstance* p = &instance; p != 0; p = p->redeclared)
		{
			if(*p->name->source.c_str() == '$' // '$outer'
				|| isIncluded(included, p->name->source))
			{
				return true;
			}
		}
		return false;
	}

	const ModuleDeclarationSet& findModuleDependencies(Name path)
	{
		ModuleDependencyMap::const_iterator i = moduleDependencies.find(path);
		if(i != moduleDependencies.end())
		{
			return (*i).second;
		}
		return MODULEDECLARATIONSET_NULL;
	}

	const MacroDeclarationSet& findModuleMacroDependencies(Name path)
	{
		MacroDependencyMap::const_iterator i = includeGraph.macros.find(path);
		if(i != includeGraph.macros.end())
		{
			return (*i).second;
		}
		return MACRODECLARATIONSET_NULL;
	}

	bool isReportable(const char* source)
	{
		typedef const char* CharConstPointer;
		const CharConstPointer exclude[] = {
			"malloc.h", "wchar.h", "cwchar", "xlocinfo.h", "xlocinfo", "cstdlib", "math.h", "time.h", "list", "cwctype"
		};
		const char* name = findFilename(source);
		for(const CharConstPointer* p = exclude; p != ARRAY_END(exclude); ++p)
		{
			if(string_equal(*p, name))
			{
				return false;
			}
		}
		return pathStartsWith(source, AbsolutePath(inputRoot).c_str());
	}

	bool printDependencies(Name path)
	{
		bool warnings = false;

		bool isHeader = !string_equal_nocase(findExtension(path.c_str()), ".inl");
		bool reportable = isReportable(path.c_str());

		IncludeDependencyNodes included;
		{
			IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(path);
			if(i != includeGraph.includes.end())
			{
				const IncludeDependencyNode& graph = *i;
				mergeIncludes(included, graph);
#if 0
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
			const ModuleDeclarationSet& d = findModuleDependencies(path);
			for(ModuleDeclarationSet::const_iterator i = d.begin(); i != d.end(); ++i)
			{
				const ModuleDeclarationSet::value_type& declaration = *i;

				printer.out << (declaration.first != NAME_NULL ? declaration.first : Name("<unknown>")).c_str() << ": ";
				printName(declaration.second);
				printer.out << std::endl;

				if(reportable
					&& declaration.first != NAME_NULL
					&& declaration.second != 0
					&& isHeader
					&& !isIncluded(included, declaration.second))
				{
					printer.out << "WARNING: depending on file that was not (in)directly included: " << declaration.first.c_str() << std::endl;
					warnings = true;
				}
			}
		}
		{
			const MacroDeclarationSet& d = findModuleMacroDependencies(path);
			for(MacroDeclarationSet::const_iterator i = d.begin(); i != d.end(); ++i)
			{
				const MacroDeclarationSet::value_type& declaration = *i;
				printer.out << declaration.first.c_str() << ": " << declaration.second << std::endl;

				if(reportable
					&& *declaration.first.c_str() != '<' // <command line>
					&& isHeader
					&& !string_equal(declaration.second, "NULL")// TEMP HACK
					&& !isIncluded(included, declaration.first)) 
				{
					printer.out << "WARNING: depending on file that was not (in)directly included: " << declaration.first.c_str() << std::endl;
					warnings = true;
				}
			}
		}

		return warnings;
	}

	void printAnchorName(const DeclarationInstance& instance, bool deferred = false)
	{
		printName(instance);
		if(deferred)
		{
			printer.out << "?";
		}
		if(instance.name != 0)
		{
			const FilePosition& position =  instance.name->position;
			printer.out << ":" << position.line << "," << position.column;
		}
	}
	bool printAnchorStart(const DeclarationInstance& instance, bool deferred = false)
	{
		if(instance.name != 0
			&& instance.name->source == NAME_NULL) // this identifier is a dependent-name
		{
			return false; // don't make this identifier a link
		}

		printer.out << "<a href='";
		if(instance.name != 0)
		{
			printer.out << OutPath(outputRoot, instance.name->source.c_str()).c_str();
		}
		printer.out << "#";
		printAnchorName(instance, deferred);
		printer.out << "'>";
		return true;
	}
	bool printAnchorStart(Declaration* declaration)
	{
		return printAnchorStart(DeclarationInstance(declaration));
	}

	void printIdentifier(const Identifier& identifier)
	{
		bool anchor = false;
		DeclarationInstance declaration = isDecorated(identifier) ? getDeclaration(identifier) : DeclarationInstance();
		if(isPrimary(identifier))
		{
			printer.out << "<a name='";
			printAnchorName(declaration, identifier.dec.deferred);
			printer.out << "'></a>";
		}
		else
		{
			anchor = printAnchorStart(declaration, identifier.dec.deferred);
		}
		const char* type = declaration != 0
			? getDeclarationType(*declaration) : "unknown";
		printer.out << "<" << type << ">";
		printer.out << getValue(identifier);
		printer.out << "</" << type << ">";
		if(anchor)
		{
			printer.out << "</a>";
		}
	}

	void pop()
	{
		REPORT_ASSERT(!includeStack.empty());
		if(includeStack.top() != SOURCE_NULL)
		{
			close();

			out.open(Concatenate(makeRange(outputRoot), makeRange(EscapedPath(includeStack.top().c_str()).c_str()), makeRange(".d")).c_str());

			bool warnings = printDependencies(includeStack.top().absolute);
			out.close();

			if(warnings)
			{
				std::cout << "warnings found: " << includeStack.top().c_str() << std::endl;
			}
		}
		includeStack.pop();
		if(!includeStack.empty()
			&& includeStack.top() != SOURCE_NULL)
		{
			resume(includeStack.top().c_str());
		}
	}

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
		if(includeStack.top() == SOURCE_NULL)
		{
			includeStack.top() = symbol->source;
			open(includeStack.top().c_str());

			{
				const ModuleDeclarationSet& dependencies = findModuleDependencies(symbol->source.absolute);
				const MacroDeclarationSet& macros = findModuleMacroDependencies(symbol->source.absolute);

				IncludeDependencyGraph::Includes::const_iterator i = includeGraph.includes.find(symbol->source.absolute);
				if(i != includeGraph.includes.end())
				{
					const IncludeDependencyNode& d = *i;
					for(IncludeDependencyNode::const_iterator i = d.begin(); i != d.end(); ++i)
					{
						//printer.out << (void*)(*i)->name.c_str();
						printer.out << "<a href='" << OutPath(outputRoot, (*i)->name.c_str()).c_str() << "'>" << (*i)->name.c_str() << "</a>" << std::endl;

						{
							MacroDeclarationSet::const_iterator j = macros.lower_bound(MacroDeclarationSet::value_type((*i)->name, 0));
							for(; j != macros.end() && (*j).first == (*i)->name; ++j)
							{
								const MacroDeclarationSet::value_type& declaration = *j;
								printer.out << "  ";
								printer.out << declaration.second;
								printer.out << std::endl;
							}
						}
						{
							ModuleDeclarationSet::const_iterator j = dependencies.lower_bound(ModuleDeclarationSet::value_type((*i)->name, DeclarationInstance()));
							for(; j != dependencies.end() && (*j).first == (*i)->name; ++j)
							{
								const ModuleDeclarationSet::value_type& declaration = *j;
								printer.out << "  ";
								printAnchorStart(declaration.second);
								printName(declaration.second);
								printer.out << "</a>";
								printer.out << std::endl;
							}
						}
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

#if 0 // this fails for mid-declaration includes
		REPORT_ASSERT(symbol->value.source == includeStack.top()); // identifiers within the current source file should be tagged with the same source path
#endif

		printIdentifier(symbol->value);
	}

	void visit(cpp::operator_function_id* symbol)
	{
		printer.formatToken(boost::wave::T_IDENTIFIER);

#if 0 // this fails for mid-declaration includes
		REPORT_ASSERT(symbol->value.source == includeStack.top()); // identifiers within the current source file should be tagged with the same source path
#endif

		printIdentifier(symbol->value); // TODO: format appropriately (handling 'new' or 'delete' differently to '*')
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

struct ParseTreePrinter : SymbolPrinter
{
	std::ofstream out;
	FileTokenPrinter printer;
	ParseTreePrinter(const char* outputRoot)
		: SymbolPrinter(printer), printer(out)
	{
		OutPath tmp(outputRoot);
		createDirectories(tmp.c_str());
		out.open(tmp.c_str());

		out << "<html>\n"
			"<head>\n"
			"</head>\n"
			"<body>\n"
			"<pre style='color:#000000;background:#ffffff;'>\n";
	}
	~ParseTreePrinter()
	{
		REPORT_ASSERT(out.is_open());

		out << "</pre>\n"
			"</body>\n"
			"</html>\n";
	}

	struct OutPath : public Concatenate
	{
		OutPath(const char* out)
			: Concatenate(makeRange(out), makeRange("parse.html"))
		{
		}
	};

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

	static UniqueTypeId getSymbolType(cpp::identifier* symbol)
	{
		if(isPrimary(symbol->value))
		{
			return makeUniqueType(getDeclaration(symbol->value)->type, 0, true);
		}
		return gUniqueTypeNull;
	}

	template<typename T>
	static UniqueTypeId getSymbolType(T* symbol)
	{
		return ExpressionType<T>::get(symbol);
	}

	template<typename T>
	void visit(T* symbol)
	{
		if(typeid(T) != typeid(*symbol) // if abstract
			|| typeid(T) == typeid(cpp::declaration_seq)) // or nested sequence
		{
			// don't print name
			symbol->accept(*this);
		}
		else
		{
			UniqueTypeId type = getSymbolType(symbol);
			out << "<span title=\"" << SYMBOL_NAME(T);
			if(type.value != 0
				&& type.value != UNIQUETYPE_NULL)
			{
				out << "|";
				printType(type);
			}
			out << "\">";
			symbol->accept(*this);
			out << "</span>";
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
};


void printSymbol(cpp::declaration_seq* p, const PrintSymbolArgs& args)
{
	try
	{
		DependencyBuilder builder;
		builder.visit(makeSymbol(p));
		{
			ParseTreePrinter printer(args.outputRoot);
			printer.visit(makeSymbol(p));
		}
		{
			SourcePrinter printer(args, builder.moduleDependencies);
			printer.visit(makeSymbol(p));
		}
	}
	catch(ReportError&)
	{
	}
}

void printSymbol(cpp::statement_seq* p, const PrintSymbolArgs& args)
{
	try
	{
		DependencyBuilder builder;
		builder.visit(makeSymbol(p));
		{
			ParseTreePrinter printer(args.outputRoot);
			printer.visit(makeSymbol(p));
		}
		{
			SourcePrinter printer(args, builder.moduleDependencies);
			printer.visit(makeSymbol(p));
		}
	}
	catch(ReportError&)
	{
	}

}

#endif
