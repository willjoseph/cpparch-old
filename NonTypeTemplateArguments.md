  * ~~store non-type argument value~~
  * ~~parse constant literals~~
  * ~~get value for constant id-expression~~
    * enumerator
  * ~~evaluate non-dependent integral constant expressions~~
    * binary
    * ternary
    * cast
    * sizeof
  * ~~deferred evaluation of dependent integral constant expressions~~
    * need to substitute non-type template parameters
      * N
      * T::value
    * need to substitute template-parameters
      * sizeof(T)
      * T(0)
    * parse expression into visitable tree
    * walk tree to perform substitution
      * is this necessary for values? can evaluate and substitute at the same time..
      * does this work when substituting types?
    * walk tree to evaluate
    * walk tree to compare for equality?
      * unique expressions?
    * ensure comma-operator evaluated left to right
    * can evaluate types with separate tree?
      * evaluating types (but not values) also required for non-integral non-constant expression
        * to determine argument types for overload resolution
        * evaluation must be deferred if expression is dependent
        * node set overlaps - some also occur in integral constant-expression
        * required in integral-constant-expression to evaluate sizeof(f())
      * sometimes type depends on value, and vice versa
        * sizeof(0) // value depends on type of '0' (but NOT value)
        * X

&lt;0&gt;

 // type depends on value of '0' (but NOT type