  * should a type involving a dependent decltype be uniqued?
    * what happens when testing two dependent decltype-specifiers for equivalence?
> > > `[temp.type]`
> > > If an expression e involves a template parameter, decltype(e) denotes a unique dependent type. Two such
> > > decltype-specifiers refer to the same type only if their expressions are equivalent
  * cache result of evaluation of member constants, avoid re-evaluation
  * evaluate expression type in context of (enclosing scope, enclosing type, location)
  * dependent vs nondependent
    * can evaluate nondependent immediately, no need to store expression tree?
      * no, need to defer evaluation of type
    * need to do some evaluation of nondependent in order to do name lookup
    * simple example: id-expression with dependent qualifying-type
      * can't perform name lookup, assume name is non-type
      * still need to perform overload resolution later
    * for now, can assume non-dependent operands when performing overload resolution
  * id-expression
    * deferred lookup for dependent nested-name-specifier
    * store qualifying type/namespace as unique-type
    * template-arguments?
      * possibly dependent
    * dependent id-expression naming a non-static member should be transformed at point of instantiation
      * if id-expression is dependent, cannot determine this until instantiation
  * binary-expression
    * `operator`
    * `left`, `right`: the operand expressions
  * unary-expression
    * operator
    * operand: the operand expression
  * function-call expression
    * `id`: the name in a call-to-named-function
      * overload-set (`getDeclaration/findOverloaded`)
      * name-lookup (ADL)
      * only if postfix-expression is an id-expression (possibly class-member-access) (possibly dependent)
      * if class-member-access, argument list is augmented by left operand of class-member-access expression
    * `type`: type of the postfix-expression, the context in which `operator()` is found
    * `memberClass`: if this is the righthand side of a member access expression, the class type of the left-hand side - provides implicit object argument for nonstatic member.
    * `arguments`: the expression-list for the function call.
    * `templateArguments`: the explicitly-specified template arguments for the function call.
    * `idEnclosing`: the class of which the name is a member, required if the name is a member of a template. Provides implicit object argument for static member
  * member-access
    * `memberType`?
      * valid only when expression is not dependent
      * represents type of expression on left hand side of class-member access
      * store objectExpression instead
    * inputs:
      * `id`: the name in a call-to-named-function
      * `qualifying`: the optional nested-name-specifier as in `x.C::m`
    * outputs
      * `idEnclosing`: the class of which the name is a member, used when uniquing the type of a member of a template class
      * `type`: the type of the member access expression
      * `memberClass`: if the type is a function, the class type of the lefthand side - provides implicit object argument for overload resolution
  * subscript
    * `type`: type of the lefthand postfix-expression, the context in which `operator[]` is found
    * `argument`: expression found within `[]`
  * explicit-type: cast, construct
    * `type`


~~task~~: make `enclosingType` valid for out-of-line member definitions

task: deferred evaluation of type of dependent expression
  * requires two-phase name lookup for unqualified names within expression. e.g. `f(dependent)`
    * for now: `[temp.dep.res]` Declarations that are visible at the point of definition of the template.
    * if operand is dependent, must store unqualified-id and point-of-definition.
      * point-of-definition is location of original template
    * could do unqualified lookup (but not overload resolution) at point of parse?
    * lookup of member operator overloads within the operand must be deferred, to allow searching dependent base class

task: determine type of dependent pointer-to-member expression
  * for SFINAE during substitution of nontype pointer-to-member parameter `&T::member` -> `int T::*member`

task: defer evaluation of dependent decltype
  * determine that decltype-specifier is dependent if its expression is dependent
  * difficult because of legacy isDependent implementation
    * stores reference to the enclosing class template that contains the template parameter depended upon
    * useful for debugging?
    * implement for expressions?
      * using typeDependent/valueDependent

task: refactor evaluation of type of expressions
  * some expression node types don't work with dependent expressions
    * need dependent version of class-member-access

task: test SFINAE for `d.m`
task: tests for type of class-member-access with const/mutable permutations
task: tests for type of class-member-access static/non-static member
~~task:~~ type of id-expression naming member: correct cv-qualifiers and value category
task: correctly determine that id-expression transformed into class-member-access is dependent if 'this' is dependent
  * should not be transformed (and therefore not be dependent) if id-expression forms pointer-to-member
  * when parsing, can't detect whether id-expression forms pointer-to-member until too late.

task: type of pointer-to-member expression `&C::m`
  * can always be determined to be ptr-to-member at point of parse? e.g. this cannot be dependent on a template parameter
  * only if naming a non-static member, otherwise `&C::m` is the address of the static member

investigate: type of class-member-access expression:

> If `E2` is a (possibly overloaded) member function, function overload resolution (13.3) is used to determine
> whether `E1.E2` refers to a static or a non-static member function.
  * how is overload resolution used in this context?
> > http://stackoverflow.com/questions/16542026/overload-resolution-without-a-parameter-list-on-class-member-access
    * possibly refers to (13.4) "A use of an overloaded function name without arguments is resolved in certain contexts to a function, a pointer to function or a pointer to member function for a specific function from the overload set"

investigate: how early can we determine that class-member-access transform should be ignored?
  * in unparenthesised decltype: known at point of parse
  * in pointer-to-member: not known until done parsing unary-expression
    * therefore cannot determine type of id-expression at point of parse
    * where does id-expression occur?
      * primary-expression
    * either suppress transform in special cases, or do transform only in other cases


investigate: can class-member-access ever be constant-expression?
  * only in C++11, if it refers to an enumerator or const static data member of integral or enumeration type
  * does the object expression play any part in determining the type or value of such an expression?
    * it determines the type if the member's type depends on a template parameter
    * it determines the value if the member's value depends on a template parameter
    * both only require the type of the object expression, not the value
    * object expression can be arbitrarily complex, or even type-dependent
      * cannot contain function-call

investigate: if ICE-ness of expression depends on expression type, how can we choose whether to unique the expression or not?
  * if expression is type-dependent, we cannot determine ICE-ness: evaluation of type-dependent expressions must be deferred
  * if such an expression appears in a non-type template argument, we do not unique it: e.g. `void f(A<x>);` where `x` is a value-dependent id-expression
  * perform additional ICE-ness check after evaluating type?
  * solution: unique all sub-expressions which are potentially ICE
    * optimisation: only if sub-expression is type-dependent OR ICE
  * replace isUniqueExpression with uniquing only in context where required?
    * e.g. template argument
    * also required in initializer of integral-constant that is later used as template argument
      * cannot distinguish this from any other expression
  * evaluate expression value only in a context when a constant expression is expected?
    * e.g. template argument, array size, bitfield, enumerator, case-statement
    * also expected in initializer of integral-constant that is later used in a constant expression
      * cannot distinguish this from any other expression

investigate: theoretically, type/value of subexpression may differ in different context

> - due to overload resolution
> - can't safely memoise using unique-expression as key
> - not an issue in practice for C++98 : no overload resolution in constant expression

investigate: substitute dependent types only once, at point of instantiation
> - 'dependentIndex' member of type
> - defer substitution for type of member
> - other cases:
> > - type of dependent base class
> > - type of default template-argument
> > > - substitution occurs whenever template specialization is named!
> > > > - evaluation should occur while uniquing the SimpleType

> > > - container for dependent constructs found in default-arguments in template-parameter-clause?
> > > > - no, don't want to indiscriminately substitute all default-arguments, only when used

> > > - result is already memoised in the form of the template arguments, don't bother?

> > - type of inline friend function definition
> > - type of member template function (likely to be dependent on its own parameters)
> > > - we assume that dependent type is either dependent or not
> > > - parameters/return-type may be dependent on own parameters, enclosing class, or both
> > > - if dependent, must attempt to substitute, but allow finding template parameters in dependent enclosingFunction
> > > > - not yet declared at point of parse, must use dummy?
> > > > - could defer dependent-expressions to deferred-substitutions at point of declaration instead?

> - consider: substitute dependent expressions when substituting type?
> > - runs into same issue with type of member template function
> > - would need to store full ExpressionWrapper
> > - doesn't substitiute dependent sub-expressions if overall expression is not value-dependent: `sizeof(sizeof(T))`

> - instead, gather dependent expressions into batch for each type, then add-deferred at point of declaration
> - forms: `int m[N]`, `A<N> m`
> - need to in turn gather expressions from within pending-instantiation types (may not be type-dependent!) used within expressions
> > - e.g. `int m[sizeof(A<sizeof(sizeof(T))>)]`
> > - expression is not guaranteed to be unique, could be function-call
> > - can't take shortcuts, may as well just gather all into enclosing container?
> > > - need to associate expressions found within a declaration with that declaration
> > > - declaration does not exist at point of parse of expression
> > > - gather all into one container, then associate with declaration later?
> > > > - gathers more than we intended: enums, anonymous unions

> - how to test?
> > - need to check that all dependent sub-expressions were substituted
> > - check that `sizeof(sizeof(T))` requires T to be complete
> > > - need intrinsic?

investigate: when evaluating dependent sub-expressions, we need to track which sub-expressions to evaluate - not just the root

> - to handle `sizeof(T)`: doesn't have a dependent type!
> - could memoise a type/value keyed by index into set of unique sub-expressions within enclosing template class/function
> > - store in ExpressionWrapper?
> > - uniqued expressions contain key (e.g. binary-expression first/second) which may differ depending on context
> > - need to include enclosing template class/function scope when uniquing expressions

> - separate sets for value-dependent and type-dependent?
> > - no need to record which (or both) to evaluate

> - separate sets for non-dependent and dependent?
> > - just need to evaluate dependent set: also needs to store references to expression node

> - later, could also store pointer to evaluation function
> - ideally would use array, but size is not known until after parse
> - add evaluation callbacks to list
> > - issue: backtracking causes expression to be 'unparsed'
> > - need 'undeclare' for expressions?
> > - how often does this happen?

> - alternative: gather lists of deferred expressions in SemaExpressionResult
> > - splice into enclosing template only when safe to commit

> - issue: cannot evaluate sub-expression without context
> > - e.g. id-expression in class-member-access or function-call
> > - solution: defer only when expression type/value would otherwise be 'non-dependent'
> > > - e.g. `sizeof(sizeof(T))` - defer evaluation of value of second dependent sizeof, and in turn type of `T`
> > > - e.g. `throw int(f(t))` - defer evaluation of type of dependent `f(t)`, and in turn type/value of `t`

> - alternative: substitute expression for non-dependent version
> > - necessary if any sub-expression is type-dependent or value-dependent
> > - simplifies evaluation
> > - can memoise substituted expression
> > - single pass through expression
> > - stages to evaluate expression:
> > > - if necessary, substitute dependent types in expression
> > > - determine dependentness
> > > - allocate copy of substituted expression sub-graph
> > > - id-expression: if not type dependent or value dependent, mark object/function as used
> > > - sizeof-type: if not value dependent, mark type as required to be complete

> - need to track names used by instantiation
> > - and types required to be complete
> > - and functions required to be instantiated
> > > - result of overload resolution

> > - gathering during dependent expression substitution?

> - rather than substitition, achieve same by converting deferred expression-wrapper to fully-evaluated
> > - need to traverse entire expression?
> > - mark expression as dependent if it contains a dependent sub-expression
> > - do same for dependent type-ids
> > - necessary to evaluate value-dependent expression if value is not used at compile time?
> > > - e.g. `new T[T::value]`

> - necessary to build fully connected expression AST?
> > - alternative: keep track of dependent disconnected subtrees
> > > - perhaps later as optimisation..?

> > - comma-separated expression parse may not keep track of multiple expressions
> > > - need expression-list node?
> > > - expression:
> > > > assignment-expression
> > > > expression-list

> > > - expression-list:
> > > > assignment-expression , expression

> - potential optimisation: avoid duplicated work when substituting multiple identical sub-expressions?
> - gathering full expressions poses same problem as disconnected sub-expressions
> > - must avoid changes to enclosing scope in case of backtrack
> > - store full expression in template argument list?
> > - gather expressions in type during make-unique?
> > > - backtrack can occur

> > - unique all expressions?
> > > - still need to track which occur in a given template class/function

> > - add guard which reverts changes to enclosing scope on backtrack?
> > > - add debug guard to catch changes to enclosing scope on backtrack?
> > > - complicated..

> > - defer committing changes to enclosing scope until after potential backtrack?
> > > - may be many cases of backtracking?

> - gather dependent member typedefs that should be checked at point of instantiation
> > - dependent `Type` can store index into array of member typedefs in instance
> > - getUniqueType can avoid substitution

> - gather dependent stuff: typedefs, expressions, declarations
> > - within template class/function/static-member
> > - into enclosing instantiation: Declaration


> - optimisation: Arguments allocates from heap using std::vector!

**investigate: evaluate dependent using-declaration
  * implement in-class using-declaration
    * pulls in all overloads of name
    * currently implemented as second instance of declaration, loses overload graph
    * also hidden by function of same signature in derived class
    * redeclare all overloads?**

  * declaration graph
> > f(A) // 1: instance.overloaded = 0, instance.redeclared = 0, declaration.overloaded = 0
> > f(A) // 2: instance.overloaded = 1, instance.redeclared = 1, declaration.overloaded = 0
> > f(B) // 3: instance.overloaded = 2, instance.redeclared = 0, declaration.overloaded = 1
> > f(A) // 4: instance.overloaded = 3, instance.redeclared = 1, declaration.overloaded = 0
> > f(C) // 5: instance.overloaded = 4, instance.redeclared = 0, declaration.overloaded = 3

  * support dependent using declaration naming a template
  * id-expression naming using-declaration: isNonStaticMemberName is incorrectly determined
  * improve makeUniqueType for using-declaration naming a type
    * type-name should generate typedef
    * if dependent?
    * template-name?



When a using-declaration brings names from a base class into a derived class scope, member functions in
the derived class override and/or hide member functions with the same name and parameter types in a base
class (rather than conflicting).

For the purpose of overload resolution, the functions which are introduced by a using-declaration into a
derived class will be treated as though they were members of the derived class. In particular, the implicit
this parameter shall be treated as if it were a pointer to the derived class rather than to the base class.
This has no effect on the type of the function, and in all other respects the function remains a member of
the base class.


**overload resolution failed when matching arguments
(boost.mpl.s\_item<boost.wave.util.from,boost.mpl.set0<boost.mpl.na>>, boost.mpl.aux.type\_wrapper<boost.multi\_index.detail.duplicate\_tag\_mark>**)
candidates for boost.mpl.s\_item.is\_masked**(const boost.mpl.s\_item<boost.wave.util.from,boost.mpl.set0<boost.mpl.na>>&, boost.mpl.aux.type\_wrapper<boost.wave.util.from>_)
not viable!
  * s\_item<boost.wave.util.from,boost.mpl.set0<boost.mpl.na>>::is\_masked_();
    * s\_item<>::is\_masked_returns no\_tag: has\_key = true
    * set0<>::is\_masked_ returns yes\_tag: has\_key = false
    * fold should produce a set?
      * duplicate\_tag\_marker result is always s\_item?**


**investigate: instantiation of member function or static data member

> [temp.inst]
> Unless a member of a class template or a member template has been explicitly instantiated or explicitly
> specialized, the specialization of the member is implicitly instantiated when the specialization is referenced
> in a context that requires the member definition to exist
> Unless a function template specialization has been explicitly instantiated or explicitly specialized, the function
> template specialization is implicitly instantiated when the specialization is referenced in a context that
> requires a function definition to exist
  * clang instantiates initializer of static data member if found inline
  * use same model as `instantiateClass`
  * unique based on enclosing type, primary declaration and template arguments
  * select explicit/partial specialization at point of instantiation
  * don't need `original`
  * instantiate when function/member is required to exist
    * when odr-used in an expression (excluding in unevaluated context)
    * constructor is odr-used during initialization, may call base constructor
    * copy-constructor may be odr-used
> > [basic.def.odr]
> > An expression is potentially evaluated unless it is an unevaluated operand or a subexpression
> > thereof. A variable whose name appears as a potentially-evaluated expression is odr-used unless it is an
> > object that satisfies the requirements for appearing in a constant expression and the lvalue-to-rvalue
> > conversion is immediately applied.
> > http://stackoverflow.com/questions/5565522/can-anyone-explain-this-paragraph-of-the-current-c0x-standard-draft
  * can use `SimpleType` for now
    * at point of odr-use, find unique instance for id-expression and instantiate
  * later: default argument instantiation**


**~~issue~~: must determine whether type-dependent expression is constant - in order to determine whether it is a null-pointer constant expression
  * if expression is type-dependent, it is also value-dependent!
    * `(T)0` is value-dependent
    * `value` is value-dependent if `value` has a dependent type
    * `T::value` is value-dependent
  * it follows that an expression's type is always known when determining its value
    * must know type in order to determine whether expression can be evaluated
  * evaluate expression type, then is-constant, then value**

**issue: `decltype((m))` returns same type as preceding `decltype(m)`
  * dependent-decltype contains ExpressionWrapper but does not compare `isParenthesised` when uniquing
  * store only ExpressionPtr + isParenthesised?
  * typeOfDecltypeSpecifier can reuse memoised type/value if expression is not dependent**


investigate: is `decltype(&A::f<int>)` ill-formed?
  * requires overload resolution

task: SFINAE for dependent pointer to member expression naming function template: `&T::f<U>`
  * may fail if `f<U>` does not identify a single function template specialization

task: avoid evaluating type of id-expression naming function template within function-call
  * e.g. `f<B>(a)` where template argument deduction is required before type can be determined
  * defer evaluation of all id-expressions naming functions, even if not overloaded?
  * function-call doesn't require knowledge of type, performs overload resolution
  * can determine type when resolving without arguments as in [over.over]
  * `decltype(f<int>) -> int()` or `decltype(&f<int>) -> int(*)()`
  * template is considered to be overloaded function and therefore should not be evaluated!

task: determine type of template-id which identifies a single function template specialization `f<int>`
  * in case of `C::mf<int>` the signature should not include an implicit object argument!
  * does implicit object parameter participate in template argument deduction?

~~issue~~: call to member template function in sizeof fails to substitute
  * substitite FunctionCallExpression's arguments before overload resolution

~~issue~~: `struct B::A : C` name lookup for base-specifier fails to find `C` in `B`

~~issue~~: `struct B::A : C<sizeof(f())>` when evaluating `f()`, instantiation-context is `::A` but should be `B::A` so that `f` is found in `B`
