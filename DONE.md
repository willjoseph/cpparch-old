## Features ##
  * implement overload resolution for implicitly-defined `operator=` in expression `(x = y)`
  * implement overload resolution for built-in operators: OverloadResolutionForBuiltInOperators
  * correct implementation of partial ordering for function templates
    * based on C++11 [temp.deduct.partial]
  * visualise template instantiation graph to aid debugging: VisualiseTemplateInstantiation
  * template argument deduction: TemplateArgumentDeduction
  * basic support for non-type template arguments: NonTypeTemplateArguments
  * implement types for enumerators: EvaluateTypeOfExpression
  * disambiguate `a < b` as template-id when `a` names a template: TemplateIdExpressionAmbiguity
  * implement user-defined-conversion: required for overload resolution: UserDefinedConversionSequence
  * overload resolution : OverloadResolution
    * implicit object parameter/argument
    * ellipsis
    * defer if any overload is dependent
  * defer parsing of default arguments: DefaultArgumentDeferredParse
  * implement evaluation of non-dependent integer-constant-expression: ExpressionEvaluation
  * better dependent-ness tracking: DependentNames
  * C++11
    * basic support for `static_assert`: StaticAssert
    * basic support for `decltype`: [Decltype](Decltype.md)
  * treat offsetof (or similar) as constant expression for backwards compatibility with C
  * correct support for unqualified friend declaration
    * declares class/function in innermost enclosing namespace
    * existing behaviour: 'friend' specifier is stored in Declaration
      * should only be in DeclarationInstance?
      * remove 'friend' specifier at point of redeclaration?
    * name lookup should not find declaration unless separately declared in that namespace before or after friend declaration
      * ignore friend declarations during name lookup?
        * except when looking for previous declaration to check for redeclaration
      * unless lookup is during ADL
      * filter friend declaration in LookupFilterDefault
    * must also be found by ADL during overload resolution
  * lvalue determination
    * object-expression cannot defer determination of lvalueness: doesn't store actual expression?
    * don't replace actual expression when parsing
  * correct dependentness for member of current instantiation


## Bugs ##
  * fix type of conversion function declaration: declarator-ptr not correctly parsed
  * failing to complete self-parse since refactoring parser
    * failing in ParserGeneric::Visit
    * deferred parse, error location seems uninformative?
      * SemaInner is undeclared?
  * reports error in declaration `int x[offsetof(A, m)]` because expecting integer-constant-expression
    * make non-fatal warning, or special case for `&(((A*)0)->m)`
    * WinNT.h line 2235 (doesn't use offsetof)
      * quick hack: treat `&(((A*)0)->m)` as a constant expression
    * clang has special implementation of offsetof as builtin\_offsetof

## Optimisation ##
  * Reduce cost of backtracking and reparsing due to shared prefixes in grammer: PathologicalSharedPrefix, MemoizeParseTree
  * profile with vtune to check for low hanging fruit: ProfileParser

## Research ##
  * Correct behaviour of qualified name lookup within currently-instantiating class: QualifiedNameLookupWithinIncompleteType
  * `T->T` and `T->const T&` are indistinguishable conversion sequences? ConversionBindToConstReference
  * deferred name lookup for nested name specifier in dependent class member access: DependentQualifiedClassMemberAccess
  * refactoring makes avoidance of double-parsing of default-argument unnecessary in the following scenario?
    * while parsing simple-declaration-named declarator which contains deferred default-argument expression,
> > > on finding `{`, we backtrack and try parsing function-definition.
> > > symbols may be deferred during attempt to parse shared-prefix declarator: `f(int i = j)`
> > > first parsed as member\_declaration\_named, backtracks on reaching `{`
    * no longer necessary because Declarator parse is successfully cached


## Refactoring ##
  * refactor semantic actions to allow splitting across multiple files: ReorganiseSemanticActions
  * replace `swap()` with assignment for shared-list types
  * get rid of old `printParameters(DeclarationList)`
    * note that this needs to print (possibly dependent) parameters of an uninstantiated template function
  * `makeUniqueType`: return more appropriate object than `gBaseClass` when failing to resolve dependent type with `allowDependent=true`
  * clean up unused expression evaluation code in SemaExpression
    * remove `type` where it is replaced by `ExpressionWrapper::type`
    * evaluate type of id-expression within class-member-access immediately
    * rename `IdExpression::enclosing` to `qualifying`
    * rename `SemaState::enclosingScope` to `enclosingScope`
    * remove idEnclosing
  * memoize evaluation of non-dependent expressions
  * inline evaluation of type/value of non-dependent expression

## Workflow ##
  * When debugging, single-step is very slow with large call-stack: FasterDebuggerSingleStep

## Documentation ##
  * remove template\_argument\_clause\_disambiguate
    * disambiguates: `< CONSTANT_EXPRESSION < 0 >`
    * how?
    * only required when template-id appears in an expression
    * no longer required, possibly because of accurate checking of whether 'identifier' is a template-name

## Other ##
  * implement single-pass semantic analysis
    * walk parse-tree and build symbol table
    * use symbol-table to disambiguate ambiguous parse-tree nodes
      * when ambiguous node is found, reject if type-name/template-name lookup fails
        * handle qualified-id and nested-name

  * parser is slow
    * profile
    * try a different lexer/preprocessor

  * handle template-id vs relational-expression ambiguity
    * parse expression twice: preferring relational-less to template-id first time to handle ambiguity:

> > <pre>A<B>(C)</pre>
> > the branch must occur at the level of 'equality-expression' to handle:
> > <pre>A<B>(C) == A<B>(C)</pre>
    * the branch must occur at the level of 'expression' to handle:
> > <pre>A < B, C > 0</pre>
> > When branching at the level of 'relational-expression' the two interpretations are:
> > <pre>A < B, C > // relational-expression -> primary-expression -> simple-template-id</pre>
> > <pre>A < B // relational-expression</pre>


+ clean up main()
+ fixed mergeIncludes crash in lexer.cpp: failed to handle cyclic includes
+ fixed crash in dependency analysis: declareEts not rolled back completely after parse fail - 'type' was left modified
+ fixed crash parsing cppparse.cpp, caused by friend-declaration shortcut
+ fixed false-positive order-dependent includes warnings
+ fixed skipping default-argument with nested parentheses
+ check that correct std::swap() is called for Copied
+ remove 'original' in LookupResult
+ remove ambiguity

&lt;T&gt;


+ move List into separate file
+ remove init\_declarator\_disambiguate
+ build against stlport for performance
+ buffer tokens from lexer: huge speed improvement!
+ allow source file + includes to be specified on commandline or config file
+ faster debugging: parse declaration-seq iteratively
+ fixed: template-function template-param scope not renamed from $template
+ bug: infinite loop when source file ends without newline?
+ remove 'else' after 'return reportIdentifierMismatch'
+ shared prefix: declarator

> - simple-declaration-named -> init-declarator-list -> init-declarator -> declarator
> - function-definition -> declarator
+ bug: without ALLOCATOR\_DEBUG: stomped global-scope declarations
> - occurs only when deferring delete of declarations
> - stomped entry contains ets-declaration in failed parse-tree
> - onBacktrack not called when ALLOCATER\_DEBUG enabled
+ enabled syntax-highlighted output from mingled parse
+ replaced std::list with custom List class
> - custom tree-allocator with std::list doesn't work: allocation pattern is implementation-defined
+ in parseSymbolRequired, defer allocation of symbol until successful parse
> - requires stack-allocation and copying of parse-tree node
> - when parsing identifier, we store a persistent reference to symbol
> > - allocate within visit()?
> > > - if parseSymbol succeeds

> - when deferring parse, we store a persistent reference to symbol
+ avoid copying Type in addDependent/Dependent
> - use list allocated with tree-allocator
> > - use reference-counting to ensure nodes not freed before release
+ make using-declaration link to named declaration
+ parse multi-dimensional array types