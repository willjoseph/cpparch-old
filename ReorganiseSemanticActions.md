  * eliminate circular dependency between all walker types?
    * `commit()` requires knowing complete type of walker
  * refactor visit methods, split up?
  * declare out of line? nonmember?
    * moves related code apart: definition of walker state vs code using the state
    * semantic actions don't care about internal walker state though, only inner walker result
    * separate result type for each walker?
    * also need to instantiate inner walker: need to call constructor and allocate stack space
  * SFINAE to choose overload for specific symbol type?
```
		#define HAS_MEM_FUNC(func, name)                                        \
		template<typename T, typename Sign>                                 \
		struct name {                                                       \
			typedef char yes[1];                                            \
			typedef char no [2];                                            \
			template <typename U, U> struct type_check;                     \
			template <typename _1> static yes &chk(type_check<Sign, &_1::func> *); \
			template <typename   > static no  &chk(...);                    \
			static bool const value = sizeof(chk<T>(0)) == sizeof(yes);     \
		}
```
    * alternative way to make overload valid only if class does not contain a specific member
      * need to obtain true/false for case where it does
        * use enable-if
    * apply to default actions
    * apply to `makePolicy`
      * can't check for function with unknown return type?
      * sfinae taking SemaT and SymbolT and finding return type if overload exists
      * need to invoke overload resolution during sfinae for this member
      * appears to be impossible without decltype
      * pose on stack-overflow

  * major refactor to turn `visit` methods into actions
    * overloadable-operator matches any `T`
    * `clearQualifying` called before creating `Walker`
    * `enumerator_definition` calls stuff before creating `Walker`
    * `declareEts` for parameter declaration in `SimpleDeclarationWalker`
    * cached parse
    * declare-ets-guard
      * leaf only
      * required because decl-spec-seq may contain forward-declared class
      * and declarator parse may cause modification of `typeSequence`
    * deferred parse
      * `member_declaration`
      * declarator
      * leaf only: `default_argument`, `statement_seq_wrapper`, `mem_initializer_clause`
    * source
    * include-events (declaration)
    * operator-function source
  * dir structure
    * mixed-case names
    * cppparse
      * Common: generic data structures, util, sequence, indirect\_set, copied, allocator
      * Language: unique-type, deduce, substitute, overload, conversion, evaluate expression
      * Ast: `Declaration`, `Type`, TemplateParameter etc.. `Scope`?
        * `Identifier`, `ExpressionNode`, `DeclarationInstance`
      * Sema: walkers
      * Parse: grammar, parser
      * Lex: lexer, token
      * Report:
      * Test: tests in symbols.cpp
  * split symbols.h
    * core types: `DeclSpecifiers`, `Identifier`, `TemplateArguments`, `TemplateArgument`,
> > > `Types`, `TypeIds`, `Type`, `Scope`, `Declaration`, `Dependent`, `TemplateParameter`, `TemplateParameters`
    * `TypeId`, type-sequence
    * template-param-defaults
    * integral-constant-evaluation: `IntegralConstant`, `getBinaryIceOp`, `getUnaryIceOp`
    * abstract expression: `ExpressionNode`/visitor, `UniqueExpressions`, `ExpressionWrapper`
    * data-structure for declaration map in `Scope`: `DeclarationInstance`, `getDeclaration`
    * `UniqueNames`: names of declarations
    * `undeclare`: used when undoing semantic actions
    * `enclosesEts`, `getEnclosingClass`, `getEnclosingNamespace`
    * meta-type-of-declaration utils: `isXYZ`: deals with meta-types: e.g. arithmetic, class, enum
      * move `isAnonymous`
    * abstract unique-types: `TypeElement`/visitor, wrapper, `isEqual`, `getInner`, `isSameType`, built-in-type, `UniqueTypeGeneric`
    * concrete unique-types: `NonType`, `TemplateTemplateArgument`, `DependentType/Typename/NonType`, `Pointer/Reference/MemberPointer/Array/Function`
> > > SimpleType
    * instantiateClass: `addBase`, `dumpTemplateInstantiations`
    * declaration-equivalence: compare function parameters at point of declaration
    * `Parameter/Parameters`
    * Location (source + point of instantiation)
    * `findPrimaryTemplate`, `findLastDeclaration`, `findPrimaryTemplateLastDeclaration`
    * `InstantiationContext`: used when evaluating type/value of expression, or subsituting types
    * concrete expression node types: `*Expression`
    * `typeOfExpression`
    * `findEnclosingTemplate`: given a template parameter and an enclosing template, find the enclosing template that declares the template parameter
    * `findEnclosingPrimaryTemplate`: used when determining if name is name of enclosing template definition
    * `getEnclosingType`: used only when adding child instantiations
    * `evaluateExpression`
    * `isDependent`
    * `deduce`
    * `TypeError` objects: generally used in substitution
    * `substitute`
    * `makeUniqueType/getUniqueType/makeUniqueQualifying/makeUniqueEnclosing/makeUniqueTemplateArguments/makeUniqueTemplateParameters`
    * `getEnclosingTemplate`, `findScope`, `getClassDeclaration`, `findEnclosingClassTemplate`
    * `isDependentImpl`
    * findOverloaded
    * requireCompleteObjectType
    * matchTemplatePartialSpecialization/findTemplateSpecialization
    * findEnclosingType
    * expression type annotation helper
    * built-in/fundamental type construction
    * literal parsing
    * getOverloadableOperatorId
    * unique-type utils: isXYZ: requires built-in types
    * type-adjustment: function-param-adjust, remove-reference, removePointer
    * implicit conversions: array-to-pointer, funtion-to-pointer, integral-promotions, usual-arithmetic-conversions, lvalue-to-rvalue, qualification-adjustment
      * standard-conversion
      * ranking conversion sequences
      * depends on findBestConversionFunction (circular!)
    * overload-resolver:
      * ranking candidate functions
      * addOverload, addConversionFunctionOverloads, findBestConversionFunction
    * declaration-error, getPrimaryDeclaration
    * symbol-printer
    * `TypeTraits.h`
      * type-traits intrinsic
    * `NameLookup.h`
      * lookup-filter + implementations
      * lookup-result, declaration-instance-ref, lookup-result-ref
      * name-lookup: `findDeclaration(SimpleType)`
      * name-lookup: findDeclaration overloads
    * non-unique `findTemplateSpecialization` implementation (used only in `getDeclaratorQualifying`)
  * move common code from `semantic.cpp` into header(s)
    * cull unused code?
    * declaration-equivalence
    * koenig lookup
    * overload-gathering
    * overload resolution for builtin operators
    * expression type determination
    * `SemaContext/SemaState/SemaBase`
    * deferred-symbols-list
    * sema-result types
    * sema-qualified
    * sema policy generic types
    * sema policy concrete types
    * sema policy macros
    * is-hidden filters: type/namespace
    * sema walker types: group by chapter?
    * prim
      * `[expr.prim]` primary-expression, id-expression, unqualified-id, qualified-id, nested-name-specifier
    * post
      * `[expr.post]` postfix-expression, expression-list, psuedo-destructor-name
    * new-delete
      * `[expr.new]` new-expression, new-placement, new-type-id, new-declarator, new-initializer
      * `[expr.delete]` delete-expression
    * expr
      * `[expr.cast]` cast-expression
      * `[expr.mptr.oper]` pm-expression
      * `[expr.mul/add/shift/rel/eq/and/xor/or/log.and/log.or/cond/ass/comma]` etc
    * stmt
      * `[stmt.stmt]` statement, labeled-statement, expression-statement, compound-statement, statement-seq,
> > > > selection-statement, condition, iteration-statement, for-init-statement, jump-statement, declaration-statement
    * dcl
      * `[dcl.dcl]` declaration-seq, declaration, block-declaration, simple-declaration
    * spec
      * `[dcl.spec]` decl-specifier, decl-specifier-seq, storage-class-specifier, function-specifier, typedef-name
      * `[dcl.type]` type-specifier, simple-type-specifier, type-name, elaborated-type-specifier
    * enum
      * `[dcl.enum]` enum-specifier, enumerator-definition
    * namespace
      * `[basic.namespace]` namespace-definition
      * `[namespace.alias]`
      * `[namespace.udecl]` using-declaration
      * `[namespace.udir]` using.directive
    * asm
      * `[dcl.asm]` asm-definition
      * `[dcl.link]` linkage-specification
    * decl
      * `[dcl.decl]` init-declarator, declarator, ptr-operator
    * type
      * `[dcl.name]` type-id, type-specifier-seq, abstract-declarator
    * class
      * `[class]` class-head, class-specifier
      * `[class.derived]` base-specifier, access-specifier
      * `[class.base.init]` mem-initializer
    * conv
      * `[class.conv.fct]` conversion-type-id, conversion-function-id, conversion-declarator
    * temp
      * `[temp]` template-declaration, template-parameter-list, template-parameter, type-parameter
    * temp.names
      * `[temp.names]` template-id, template-argument-list, template-argument
    * except
      * `[except]` try-block, handler-seq, handler, exception-declaration, throw-expression
    * n/a
      * `[class.mem]` member-declaration, member-declarator
      * `[dcl.fct.def]` function-definition
      * `[dcl.init]` initializer

```
Operator: SemaOverloadableOperator, SemaOperatorFunctionId
PrimaryExpression: SemaUnqualifiedId, SemaQualifiedId, SemaIdExpression, SemaLiteral, SemaPrimaryExpression
PostfixExpression: SemaPostfixExpression, SemaArgumentList, SemaSubscript, SemaPostfixExpressionMember, SemaTypeTraitsIntrinsic
Expression: SemaExpression, SemaExplicitTypeExpression, SemaSizeofTypeExpression, SemaConditionalExpression
NestedNameSpecifier: SemaNestedNameSpecifierSuffix, SemaNestedNameSpecifierPrefix, SemaNestedNameSpecifier
TypeId: SemaTypeName, SemaTypeSpecifier, SemaElaboratedTypeSpecifier, SemaTypeId, SemaNewType
DeclaratorFunction: SemaParameterDeclarationList, SemaParameterDeclarationClause, SemaExceptionSpecification, SemaDeclaratorFunction, SemaCvQualifierSeq
Declarator: SemaUnqualifiedDeclaratorId, SemaQualifiedDeclaratorId, SemaDeclaratorId, SemaPtrOperator, SemaDeclaratorArray, SemaDeclarator, SemaInitializer
Class: SemaBaseSpecifier, SemaClassHead, SemaClassSpecifier, SemaQualifiedTypeName, SemaMemInitializer, SemaMemInitializerClause, SemaMemberDeclaratorBitfield, SemaMemberDeclaration
Namespace: SemaNamespaceName, SemaUsingDeclaration, SemaUsingDirective, SemaNamespace, SemaNamespaceAliasDefinition
Enum: SemaEnumeratorDefinition, SemaEnumSpecifier
Statement: SemaLabeledStatement, SemaStatement, SemaControlStatement, SemaCompoundStatement, SemaStatementSeq
Exception: SemaTryBlock, SemaHandler, SemaHandlerSeq
Declaration: SemaDeclSpecifierSeq, SemaDeclarationSuffix, SemaSimpleDeclaration, SemaDeclaration
TemplateDeclaration: SemaTypeParameter, SemaTemplateParameterList, SemaTemplateParameterClause, SemaTemplateDeclaration
TemplateName: SemaTemplateId, SemaTemplateArgumentList, SemaTypenameSpecifier, SemaExplicitInstantiation
```



~~investigate~~: what's going on with SimpleDeclarationWalker
  * complex
  * used with:
    * decl-spec-seq 

&lt;suffix&gt;



> > > simple\_declaration -> simple\_declaration\_suffix: type\_declaration\_suffix/simple\_declaration\_named
> > > general\_declaration -> general\_declaration\_suffix: type\_declaration\_suffix/simple\_declaration\_named/function\_definition
> > > member\_declaration\_default -> member\_declaration\_suffix: type\_declaration\_suffix/member\_declaration\_bitfield/member\_declaration\_named/function\_definition
> > > parameter\_declaration -> parameter\_declaration\_suffix
    * type-specifier-seq 

&lt;suffix&gt;


> > > condition\_init -> condition\_declarator
> > > exception\_declaration\_default -> exception\_declarator
    * function-specifier-seq 

&lt;suffix&gt;


> > > constructor\_definition -> function\_definition
> > > member\_declaration\_implicit -> member\_declaration\_suffix
  * all variations contain some type of declarator
  * decl-spec/type-spec may contain forward-declaration
    * declare immediately after decl-spec?
    * target scope depends on whether this is an explicit forward-declaration or occurs within another declaration
  * split into prefix/suffix?
