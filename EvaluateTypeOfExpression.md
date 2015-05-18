~~task~~: implement types for enumerators

  * declare unique type for each enum
  * give its enumerators that type
  * integral promotion
> `[dcl.enum]`
> Each enumeration defines a type that is different from all other types. Following the closing brace of an
> enum-specifier, each enumerator has the type of its enumeration. Prior to the closing brace, the type of
> each enumerator is the type of its initializing value. If an initializer is specified for an enumerator, the initializing
> value has the same type as the expression. If no initializer is specified for the first enumerator, the
> type is an unspecified integral type. Otherwise the type is the same as the type of the initializing value of
> the preceding enumerator unless the incremented value is not representable in that type, in which case the
> type is an unspecified integral type sufficient to contain the incremented value.

~~task~~: array-to-pointer conversion in binary expression

  * infer that binary expression requires rvalue operands
  * therefore lvalue-to-rvalue (array to pointer) conversion is performed
> `[conv]`
> expressions with a given type will be implicitly converted to other types in several contexts:
> > - When used as operands of operators. The operator's requirements for its operands dictate the destination type


> `[expr]`
> Whenever an lvalue expression appears as an operand of an operator that expects an rvalue for that operand,
> the lvalue-to-rvalue (4.1), array-to-pointer (4.2), or function-to-pointer (4.3) standard conversions are
> applied to convert the expression to an rvalue. [Note: because cv-qualifiers are removed from the type of
> an expression of non-class type when the expression is converted to an rvalue, an lvalue expression of type
> const int can, for example, be used where an rvalue expression of type int is required. ]

> `[expr.add]`
> For addition, either both operands shall have arithmetic or enumeration type, or one operand shall be a
> pointer to a completely defined object type and the other shall have integral or enumeration type.

~~task~~: type of overloaded operators: required to determine type of expression
  * postfix `operator[]`
  * postfix `operator++`
  * `operator->`

~~task~~: remove special-cases for gEnumerator

~~issue~~: uniqued expressions required? e.g. this would mean losing the type of a template-parameter in an expression
  * used to identify redeclarations of same function
> > Two expressions involving template parameters are considered equivalent if two function definitions containing
> > the expressions would satisfy the one definition rule (3.2),
    * e.g. both expressions must contain the same stream of tokens, names must be resolved to same entities via name lookup
      * except template params may be differently named
    * however, if both expressions would evaluate to same value but differ by ODR, program is ill formed
    * in this case it should be ok to make unique expressions preserving type of template parameter
  * used in non-type template arguments to distinguish between template specializations?
    * `A<bool(0)>` names same specialization as` A<short(0)>`
    * `A<x>` may be same as `A<y>` but cannot be determined until instantiation
      * will disregard type when evaluating dependent expression as template-argument
    * so never actually comparing uniqued expressions
  * alternative way to compare expressions in template function signature?
    * compare actual token stream?
    * hash tokens? risks collisions
    * compare AST? requires AST walk

~~issue~~: when evaluating type of [non-dependent expression involving a] non-type template parameter, we only have the template-parameter index
  * must look up the template parameter in the `enclosingType`, just to get the type (not to perform substitution)
  * `enclosingType` not [currently](currently.md) available within a function template
  * this is not a problem unless evaluation is deferred.
  * does this ever need to be deferred?
    * yes: `template<class T, T x>`
    * when evaluating `sizeof(x)`
  * resolved by retaining [dependent](possibly.md) type of template parameter in `NonTypeTemplateParameter`

~~task~~: make enclosingFunction available within body of function-template, to enable resolution of Nth template parameter
  * hack: make `SimpleType` from function declaration and template params

~~issue~~: when evaluating type of id-expression within a function-call expression, cannot resolve type
  * need to know arguments (or target function type) before overload resolution can be performed
  * should type resolve to 'function overload set' ?
  * if id-expression names a function outside a function-call expression
    * e.g. `cout << std::left; // ptr-to-function`
    * type is 'function overload set'
    * return special type to indicate this?

~~issue~~: when evaluating type of id-expression within a member-access expression, cannot resolve type
  * need to know object-expression before type can be resolved
  * fix: set enclosingType to type of object expression

investigate: when do we need to evaluate an expression's type?
  * when determining whether it's a constant-expression
    * how exactly?
  * when determining whether it's a pointer-to-member-expression
  * when determining types of arguments for overload resolution
  * need to evaluate in order to discover side effects that affect name binding
    * non-dependent expressions should be immediately evaluated
    * dependent expressions should be evaluated at point of instantiation

task: determine type of all non-dependent expressions
  * overload resolution
    * unique type for dependent member of template
    * null-pointer-constant
  * template argument deduction for function template
    * along with explicit template argument specification (C++11)
    * requires support for function templates
    * substitute template params that have explicit arguments
      * ignore unspecified template params at this point
      * substitute unspecified template params with themselves?
        * much like when name of current-instantiation is used in class template definition
  * ensure that the evaluation of the type of an id-expression naming an overloaded function is deferred until we know whether or not it is a function-call

task: determine type of conditional-expression with differing second and third operands
  * write tests!

task: perform overload resolution when name of overloaded function is used as template parameter
  * test: N431