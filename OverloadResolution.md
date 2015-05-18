  * across using-directive
  * ( identifier ) ( expression-list )
  * object-of-class-type ( expression-list )
  * record argument types for function-call
> > + record complex types
> > > + pointer, array, function
> > > + member-ptr, const
    * implicit object argument
      * const
    * print complex types
> > > + pointer, array, function
      * member-ptr, const
    * ellipsis
    * null-pointer-constant
  * evaluate type of expression
    * expression-list
      * choose last
      * found in: parenthesis, array-index, cast, typeid, new-array, conditional-second, expression-statement, condition, iteration-cond+incr, return,
    * primary
      * identifier
    * postfix
      * member
    * cast
    * binary-operator
> > > + handle out-of-order parse for left-associative expressions
    * unary-operator
      * address
      * dereference
    * ternary-operator
    * overloaded-operator
> > > + star
      * member vs non-member lookup
      * overload resolution against built-in operators
    * promotions
    * numeric-literal: double, float, hex, oct, modifiers
    * string-literal: wide
  * implicit-conversion
    * can be identity
    * can be ambiguous
    * left-operand of assignment and implicit-object param cannot be temporaries
    * standard
      * up to one of each of lvalue transform, promotion/conversion, qualification adjustment, applied in order
      * ranked as exact, promotion or conversion
        * T&->T vs T->T is ambiguous
        * 0->T**vs 0->float ?
    * user-defined
    * ellipsis
  * testing framework
    * generate unique-type from C++ type
  * consider redeclarations with same signature as a single candidate
  * perform template-argument type-deduction**

~~task~~: overload resolution: implicit object parameter/argument

> `[over.match.funcs]` a member function is considered to have an extra parameter, called the implicit object parameter, which
> represents the object for which the member function has been called. For the purposes of overload resolution,
> both static and non-static member functions have an implicit object parameter, but constructors do not.
> Similarly, when appropriate, the context can construct an argument list that contains an implied object
> argument to denote the object to be operated on.
  * when calling a member function, must always supply an implicit-object-argument
    * when that member is static and is called without a class object, the implied-object-argument is the type of the class containing the member

~~task~~: ellipsis in overload resolution
  * if more arguments than parameters, excess arguments become ellipsis-conversion-sequence

~~task~~: remove check for `isMemberOfTemplate` that guards overload resolution: `findBestOverloadedFunction`
  * investigate: is it possible to do overload resolution and choose a function that has a dependent return type?
    * example: calling an overloaded static member of a template, within `sizeof()`, to initialise a static-const member
    * should defer overload resolution if any overload is declared with a dependent type
    * defect 541: http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html
  * required because member-functions of template classes may have dependent arguments
    * conformant solution: function-call expression is dependent if function has any overloads that are declared with a dependent type
    * don't perform overload resolution if function-call expression is dependent