## Important ##

## Optimisation ##
  * reduce cost of initialising multiple-inherited vtables in parse-tree nodes: OptimisePolymorphicParseTreeNode
  * avoid constructing complex `*_NULL` constants on the fly
  * use custom list class for `DeferredSymbols`?
  * custom allocator for unique-types
  * specialised expression parsing: http://www.engr.mun.ca/~theo/Misc/exp_parsing.htm
  * runs slower in debugger than outside?
    * turn of debug heap: env var _NO\_DEBUG\_HEAP=1_

## Features ##
  * Declare copy-assignment operator if not already declared: ImplicitlyDeclaredCopyAssignmentOperator
  * **Correct evaluation of type for all possible expressions**: EvaluateTypeOfExpression
    * conditional
  * evaluate implicit conversion to return-type in return statement (may require complete class)
  * evaluate explicit conversions in cast expression (may require complete class)
    * e.g. may invoke user-defined conversion
  * determine correct lvalueness for all expressions
  * evaluate expression immediately if possible: EvaluateTypeOfExpression
  * **Deferred evaluation of expression type**: EvaluateTypeOfDependentExpression
    * static assert should check whether convertible to bool, and evaluate constant expression
    * other expressions should perform deferred instantiation for types that are required to be complete
      * scenarios?
  * **Template Instantiation** : ClassTemplateInstantiation
    * disallow implicit instantiation of a template within its own definition
  * C++11
    * support `>>` in nested template-argument-list
    * implement `auto`
    * defer evaluation of dependent `decltype(x)`: StaticAssert
    * defer evaluation of dependent `static_assert(x, "")`: [Decltype](Decltype.md)
  * gracefully handle all parse errors without crashing: GracefullyHandleParseErrors
  * implement function template explicit specialization
  * function parameter type transformations during template argument deduction: TemplateArgumentDeduction
  * merge default-arguments across multiple declarations
    * watch out for types that refer to local template-param declaration
  * Correct name lookup behaviour for dependent names: DependentNames
    * remove old isDependent/addDependent framework
    * distinguish name-of-template-itself as not dependent
  * properly implement sizeof
    * for dependent types
      * defer initialisation of SimpleType::layout until instantiation
      * must require complete type for non-dependent non-static data members at point of declaration
      * add layout of non-dependent members to list in SimpleType
    * for unions
  * disallow 'template' followed by non-template-id
  * basic recovery from errors: to allow some tests to fail without preventing others running
    * difficult if we expect to be able to safely recover from any error
  * error reporting and recovery
    * if recoverable error is encountered while parsing declaration or statement, skip it (resynchronise)
    * replace declaration with..?
    * what if parsing function-definition or namespace-definition which don't end in semicolon?
    * or declaration that follows linkage-specification ?
    * statement always ends in semicolon
  * ignore double-reference introduced by typedef or decltype
  * implement sizeof for POD types
    * alignment, padding
    * array, pointer, pointer to member, enum, union
    * anonymous union
  * implement offsetof for POD types
    * dependent offsetof: if type is dependent, cannot be evaluated
  * list symbols depended on by each module: FindSymbolDependencies
  * correct behaviour for using-declaration
    * dependent
    * template-name
    * used in overload resolution
  * explicit instantiation: instantiate all functions and static members when class is explicitly instantiated
  * explicit specialization of function template
  * explicit specialization of static member
  * explicit specialization of member function
  * correct dependentness for local class
    * http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html

## Bugs ##
  * `decltype(0ll)` should be `long long`
  * offsetof expression `&(((A*)0)->m)` should be usable in a template argument
    * class-member-access should be uniqued?
    * detect `&(((A*)0)->m)` and replace?
  * type-name: shared prefix between identifier and template-id leads to template-id being knocked out of cache
    * occurs theoretically when type-name parse fails twice
      * not a major problem, if we assume type-name parse will always subsequently fail..
    * when identifier is successfully looked up, template-id is removed
      * more-abstract entries at same position are invalidated by successful lookup
        * implies that identifier is part of a competing choice
      * necessary because entries are allocated via linear allocator: next non-cached symbol will cause backtrack
      * would be preferable to defer removal until actual backtrack
    * could change grammar to remove shared prefix, but this is undesirable..
    * reversing order of identifier and template-id breaks parse: template-id parsed as expression

## Tests ##
  * test that various declaration forms are correctly detected to be equivalent
    * friend redeclared as non-friend
    * function redeclared with same argument types
  * test trick using overload resolution within sizeof
    * ensure reported size of type is correct
  * test expression type evaluation
    * integral promotions
    * usual arithmetic conversions
    * conditional expression
      * null vs pointer and null vs pointer-to-function
      * composite pointer type
      * user-defined conversions
    * conversion operator
> > > http://stackoverflow.com/questions/25241516/different-cast-operator-called-by-different-compilers

> http://stackoverflow.com/questions/26658776/why-cant-i-access-a-default-argument-in-its-initializer-in-gcc
> http://stackoverflow.com/questions/26681873/interaction-between-decltype-and-class-member-name-shadowing-an-external-name

## Research ##
  * defect: class-or-decltype should contain optional 'template' keyword?
    * seems unnecessary since a qualified name can always assumed to name a type, and there is no ambiguity to resolve
  * in `if(struct S* p = 0)` what is the scope of `S` and in what namespace is `S` declared?
> > `[basic.scope.pdecl]`
> > if the elaborated-type-specifier is used in the decl-specifier-seq or parameter-declaration-clause of a
> > function defined in namespace scope, the identifier is declared as a class-name in the namespace that
> > contains the declaration; otherwise, except as a friend declaration, the identifier is declared in the
> > smallest namespace or block scope that contains the declaration.

## Workflow ##
  * Debugging/testing eats a lot of time, even when running on preprocessed source files: FasterTesting
  * Automated build and testing: AutomatedBuildAndTest
  * Better error reporting from tests?
  * Long iteration times when modifying Sema.cpp: FasterBuild
  * Debugger can be very slow (VS2010)
    * run-to-cursor inside a templated function causes long hitch
    * single-step with a deep call-stack causes long hitch
  * Add unity build to see if it affects link time after small changes.
  * natvis for custom container types

## Documentation ##
  * document `boost::wave` modification: `resolved_include_file`
  * document TypeSubstitute

## Refactoring ##
  * clean up duplicated html string escape in `SourcePrinter`/`ParseTreePrinter`
  * clean up error handling - multiple assert exception types?
  * clean up test code: ensure it compiles in LLVM
  * make gUniqueNames a member of Scope?
  * move Callback stuff into separate file
  * generate remaining handwritten parseSymbol functions
    * `PARSE_SELECT_TOKEN`, `PARSE_EXPRESSION_LEFTASSOCIATIVE`
    * `inTemplateArgumentList`
    * `overloadable_operator`
  * use custom list class for `CachedSymbols`
  * take advantage of memoization to simplify grammar (allowing more shared-prefix)
    * for shared-prefix of assignment-expression/conditional-expression/constant-expression
    * shared-prefix of expression/expression-list/assignment-expression
  * general renaming
    * `UniqueTypeWrapper`
    * remove UniqueTypeId
  * issue: declaration specifiers may differ in different redeclarations, but there is only one copy stored for all of them
  * remove declarationList: doesn't appear to be required
  * split template-param part of findEnclosingType into separate function
  * split up ExpressionEvaluate.h to reduce size
  * fix circular dependency between evaluateExpression and OverloadResolve.h
  * two identical versions of makeUniqueTemplateArguments?
  * unify various versions of isComplete, isFunction etc
  * two places to parse constant\_expression in bitfield declaration?
    * both are used..