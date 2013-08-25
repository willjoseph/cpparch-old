
#include "Report.h"
#include "Ast/Ast.h"
#include "Lex/TokenPrinter.h"
#include "Lex/Lex.h"
#include "Common/Util.h"
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
			symbol->accept(*this);
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
	return isDecorated(id) && id.source == id.dec.p->name->source;
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

void addModuleDependency(ModuleDependencyMap& moduleDependencies, const Name module, const DeclarationInstance& instance)
{
	if(instance.name->source.absolute != NAME_NULL // refers to a symbol declared in a module
		&& module != instance.name->source.absolute) // refers to a symbol not declared in the current module
	{
		moduleDependencies[module].insert(ModuleDeclaration(instance.name->source.absolute, instance));
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
	ModuleDependencyMap& moduleDependencies;
	DependencyBuilder(ModuleDependencyMap& moduleDependencies)
		: moduleDependencies(moduleDependencies)
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
	}

	template<typename T>
	void visit(cpp::symbol<T> symbol)
	{
		if(symbol.p != 0)
		{
			visit(symbol.p);
		}
	}

	void addTypeDependency(UniqueTypeId type, Name module)
	{
		if(type.isSimple())
		{
			const SimpleType& objectType = getSimpleType(type.value);
			if(isClass(*objectType.declaration))
			{
				//TODO SYMBOLS_ASSERT(objectType.instantiated);
				addModuleDependency(moduleDependencies, module, DeclarationInstance(objectType.declaration));
			}
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
			if(!isNamespace(*instance)
				&& !symbol->value.dec.deferred) // name resolution not deferred
			{
				// the source file containing this symbol depends on all the redeclarations (of the declaration chosen by name resolution) that were visible
				// e.g. function redeclarations (excluding unchosen overloads), class forward-declarations (excluding unchosen explicit/partial-specializations)
				
				// add the most recent redeclaration
				addModuleDependency(moduleDependencies, symbol->value.source.absolute, instance);
			}
		}
		else if(!instance->type.isDependent)
		{
			if(isObjectDefinition(*instance) // if this declaration defines an object
				&& !(isFunctionParameter(*instance) // and the object is not a function parameter..
					&& (!isDecorated(instance->scope->name) || !getDeclaration(instance->scope->name)->isFunctionDefinition))) // .. within a function declaration
			{
				REPORT_ASSERT(instance->type.unique != 0);
				UniqueTypeId type(instance->type.unique);
				if(isFunctionParameter(*instance))
				{
					type = adjustFunctionParameter(type);
				}
				while(type.isArray()
					&& getArrayType(type.value).size != 0)
				{
					type.pop_front(); // arrays of known size are object types
				}
				addTypeDependency(type, symbol->value.source.absolute);
			}
		}
	}

	void addCastDependency(cpp::expression* symbol)
	{
		// [basic.lval] An expression which holds a temporary object resulting from a cast to a non-reference type is an rvalue
		addTypeDependency(getExpressionType(symbol), symbol->source.absolute);
	}
	void visit(cpp::cast_expression_default* symbol)
	{
		addCastDependency(symbol);
		symbol->accept(*this);
	}
	void visit(cpp::postfix_expression_construct* symbol)
	{
		addCastDependency(symbol);
		symbol->accept(*this);
	}
	void visit(cpp::postfix_expression_cast* symbol)
	{
		addCastDependency(symbol);
		symbol->accept(*this);
	}

	void addNewExpressionDependency(cpp::expression* symbol)
	{
		// [expr.new] The new expression attempts to create an object of the type-id or new-type-id to which it is applied. The type shall be a complete type...
		UniqueTypeWrapper type = getExpressionType(symbol);
		type.pop_front();
		addTypeDependency(type, symbol->source.absolute);
	}
	void visit(cpp::new_expression_default* symbol)
	{
		addNewExpressionDependency(symbol);
		symbol->accept(*this);
	}
	void visit(cpp::new_expression_placement* symbol)
	{
		addNewExpressionDependency(symbol);
		symbol->accept(*this);
	}
	void visit(cpp::assignment_expression_default* symbol)
	{
		if(symbol->right != 0
			&& typeid(*symbol->right) == typeid(cpp::assignment_expression_suffix))
		{
			cpp::assignment_expression_suffix* suffix = static_cast<cpp::assignment_expression_suffix*>(symbol->right.p);

			UniqueTypeId type = getExpressionType(symbol->left);
			if(type.isReference())
			{
				// [expr.ass] When the left operand of an assignment operator denotes a reference to T, the operation assigns to the object of type T denoted by the reference.
				type.pop_front();
			}
			// [expr.ass] If the left operand is of class type, the class shall be complete.
			addTypeDependency(type, symbol->source.absolute);

			// [expr.ass] In += and -= [the left operand] shall have either arithmetic type or be a pointer to a possibly cv-qualified completely defined object type.
			if(suffix->op->id == cpp::assignment_operator::PLUS
				|| suffix->op->id == cpp::assignment_operator::MINUS)
			{
				if(type.isPointer())
				{
					type.pop_front();
					addTypeDependency(type, symbol->source.absolute);
				}
			}
		}

		symbol->accept(*this);
	}
	
	void visit(cpp::postfix_expression_member* symbol)
	{
		UniqueTypeId type = getExpressionType(symbol);
		if(type.isReference())
		{
			type.pop_front(); // [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		}
		if(symbol->op->id == cpp::member_operator::ARROW
			&& type.isPointer())
		{
			type.pop_front();
		}
		// [expr.ref] [the type of the object-expression shall be complete]
		addTypeDependency(type, symbol->source.absolute);
		symbol->accept(*this);
	}

	void visit(cpp::postfix_expression_subscript* symbol)
	{
		UniqueTypeId type = getExpressionType(symbol);
		if(type.isReference())
		{
			type.pop_front(); // [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		}
		// [expr.sub] The result is an lvalue of type T. The type "T" shall be a completely defined object type.
		addTypeDependency(type, symbol->source.absolute);
		symbol->accept(*this);
	}

	void visit(cpp::unary_expression_sizeof* symbol)
	{
		UniqueTypeId type = getExpressionType(symbol->expr);
		if(type.isReference())
		{
			type.pop_front(); // [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		}
		// [expr.sizeof] The sizeof operator shall not be applied to an expression that has function or incomplete type.
		addTypeDependency(type, symbol->source.absolute);
		symbol->accept(*this);
	}

	void visit(cpp::unary_expression_sizeoftype* symbol)
	{
		UniqueTypeId type = getExpressionType(symbol->type);
		if(type.isReference())
		{
			type.pop_front(); // [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		}
		// [expr.sizeof] The sizeof operator shall not be applied to an expression that has function or incomplete type... or to the parenthesized name of such types.
		addTypeDependency(type, symbol->source.absolute);
		symbol->accept(*this);
	}

	void visit(cpp::unary_expression_op* symbol)
	{
		UniqueTypeId type = getExpressionType(symbol->expr);
		if(type.isReference())
		{
			type.pop_front(); // [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		}
		if(symbol->op->id == cpp::unary_operator::PLUSPLUS
			|| symbol->op->id == cpp::unary_operator::MINUSMINUS)
		{
			// [expr.pre.incr] The type of the operand shall be an arithmetic type or a pointer to a completely-defined object type.
			if(type.isPointer())
			{
				type.pop_front();
				addTypeDependency(type, symbol->source.absolute);
			}
		}
		symbol->accept(*this);
	}

	void visit(cpp::postfix_operator* symbol)
	{
		UniqueTypeId type = getExpressionType(symbol);
		if(type.isReference())
		{
			type.pop_front(); // [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		}
		if(symbol->id == cpp::unary_operator::PLUSPLUS
			|| symbol->id == cpp::unary_operator::MINUSMINUS)
		{
			// [expr.post.incr] The type of the operand shall be an arithmetic type or a pointer to a complete object type.
			if(type.isPointer())
			{
				type.pop_front();
				addTypeDependency(type, symbol->source.absolute);
			}
		}
		symbol->accept(*this);
	}

	void visit(cpp::additive_expression_default* symbol)
	{
		UniqueTypeId type = getExpressionType(symbol->left);
		if(type.isReference())
		{
			type.pop_front(); // [expr] If an expression initially has the type "reference to T", the type is adjusted to "T" prior to any further analysis.
		}
		// [expr.add] the left operand is a pointer to a completely defined object type [or] both operands are pointers to the same completely defined object type.
		if(type.isPointer())
		{
			type.pop_front();
			addTypeDependency(type, symbol->source.absolute);
		}

		symbol->accept(*this);
	}

	void visit(cpp::base_specifier* symbol)
	{
		UniqueTypeId type = getExpressionType(symbol);
		// [class.derived] The class-name in a base-specifier shall not be an incompletely defined class.
		addTypeDependency(type, symbol->source.absolute);

		symbol->accept(*this);
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

inline const char* getDeclarationType(const Declaration& declaration)
{
	if(isNamespace(declaration))
	{
		return "namespace";
	}
	if(isType(declaration))
	{
		return declaration.isTemplate ? "templateName" : "type";
	}
	return "object";
}


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
		: SymbolPrinter(printer, true),
		printer(out),
		inputRoot(args.inputRoot),
		outputRoot(args.outputRoot),
		includeGraph(args.includeGraph),
		moduleDependencies(dependencies)
	{
		includeStack.push(Path(Name(""), Name("$outer")));
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

	typedef Stack<Path, 1024> IncludeStack;

	IncludeStack includeStack;

	void push()
	{
		REPORT_ASSERT(!includeStack.empty());
		if(includeStack.top() != PATH_NULL)
		{
			suspend();
		}
		includeStack.push(PATH_NULL);
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
			if(*p->name->source.absolute.c_str() == '$' // '$outer'
				|| isIncluded(included, p->name->source.absolute))
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
				const ModuleDeclaration& declaration = *i;

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
			printer.out << ":" << instance.name->source.line << "," << instance.name->source.column;
		}
	}
	bool printAnchorStart(const DeclarationInstance& instance, bool deferred = false)
	{
		if(instance.name != 0
			&& instance.name->source.absolute == NAME_NULL) // this identifier is a dependent-name
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
#if 0
	bool printAnchorStart(Declaration* declaration)
	{
		return printAnchorStart(DeclarationInstance(declaration));
	}
#endif

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
		if(includeStack.top() != PATH_NULL)
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
			&& includeStack.top() != PATH_NULL)
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
		if(includeStack.top() == PATH_NULL)
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
							MacroDeclarationSet::const_iterator j = macros.lower_bound(MacroDeclaration((*i)->name, 0));
							for(; j != macros.end(); ++j)
							{
								const MacroDeclaration& declaration = *j;
								if(declaration.first != (*i)->name)
								{
									continue;
								}
								printer.out << "  ";
								printer.out << declaration.second;
								printer.out << std::endl;
							}
						}
						{
							ModuleDeclarationSet::const_iterator j = dependencies.lower_bound(ModuleDeclaration((*i)->name, DeclarationInstance()));
							for(; j != dependencies.end(); ++j)
							{
								const ModuleDeclaration& declaration = *j;
								if(declaration.first != (*i)->name)
								{
									continue;
								}
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
		: SymbolPrinter(printer, true), printer(out)
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
			return UniqueTypeId(getDeclaration(symbol->value)->type.unique);
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
		ModuleDependencyMap moduleDependencies;
		{
			DependencyBuilder builder(moduleDependencies);
			builder.visit(makeSymbol(p));
		}
		{
			ParseTreePrinter printer(args.outputRoot);
			printer.visit(makeSymbol(p));
		}
		{
			SourcePrinter printer(args, moduleDependencies);
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
		ModuleDependencyMap moduleDependencies;
		{
			DependencyBuilder builder(moduleDependencies);
			builder.visit(makeSymbol(p));
		}
		{
			ParseTreePrinter printer(args.outputRoot);
			printer.visit(makeSymbol(p));
		}
		{
			SourcePrinter printer(args, moduleDependencies);
			printer.visit(makeSymbol(p));
		}
	}
	catch(ReportError&)
	{
	}

}

#endif
