  * required only when an operand has class or enumeration type
  * how to create infinite number of built-in overloads? e.g. match any pointer type
    * class may have multiple user-defined-conversions that match different types
    * pointer may have multiple valid conversions - e.g. `Derived->Base*`, `T*->void*`
    * in case of `operator==(T*, T*)` arguments may be incompatible but both convertible to `void*` or `Base*`
    * for each argument type, need to find intersection of potential parameters types with types to which argument can be converted
    * possible to re-use implicit-conversion code to determine types?
      * difficult to return multiple values, may be simpler to write new code
    * find superset and narrow via overload resolution?
    * initial implementation can disregard user-defined-conversions
    * `T*[volatile]&` (object)
      * from lvalue `T*` (or `T*volatile`), direct reference binding (derived not considered)
      * from `C`, direct reference binding via `operator T*` (derived not considered)
      * cannot bind rvalue to non-const reference
    * `T*` (object, any) (may be cv-qualified)
      * from nullptr, wildcard for other argument
      * from `T*`, exact match or derived/void
      * from `C`, user-defined conversion via `operator T*` (or derived/void)
    * `T*` (function) (may be cv-qualified)
      * from nullptr, wildcard for other argument
      * from `T*`, exact match
      * from `C`, user-defined conversion via `operator T*`
    * `T` (pointer to member) (may be cv-qualified)
      * from nullptr, wildcard for other argument
      * from `T`, exact match or derived
      * from `C`, user-defined conversion via `operator T` (or derived)
    * `[volatile] T&` (arithmetic)
      * from lvalue `T`, exact match (conversion not considered)
      * from C, user-defined conversion via `operator T&`
    * `[volatile] T&` (enum)
      * from lvalue `T`, exact match (conversion not considered)
      * from `C`, user-defined conversion via `operator T&`
    * `T` (arithmetic)
      * from any arithmetic (or enum), exact match or conversion
      * from `C`, user-defined conversion via conversion function to any arithmetic (or enum)
    * for a given placeholder parameter/argument pair, find set of types which can replace `T`
      * takes (to, from, source, enclosing): may cause template instantiation!
      * if reference, bind directly, otherwise perform lvalue-to-rvalue conversion
      * if pointer (or member pointer), add all cv-qualification permutations (and bases)
      * result is simple vector of types for now
      * if `T` is shared between two parameters, find the union of both sets
      * if `T` is in two parameters but not shared (e.g. `L`, `R`), find all permutations of both sets
      * generate function types and perform overload resolution
  * hide user-written candidates that match built-in overload parameters exactly
  * for enum, overload set contains built-in operators that take types that enum can be converted to
    * e.g. promote to int, convert to any integer type, convert to any floating point type, convert to bool
  * types:
```
O=object (not reference, not function, not void)
F=function
G=object-or-function
A=arithmetic
I=integral
T=type
C=class
E=enum
P=pointer
M=pointer-to-member
```
  * forms
```
O*
[const] [volatile] O*
F* (operator*)
T*
A
I
E
P
M
[volatile] I&
[volatile] E&
[volatile] M&
[volatile] A&
[volatile] A& (not bool)
O*[volatile]&
P[volatile]&
[const] [volatile] C*
[const] [volatile] G C::*
```
  * multiple conversion functions may match?
  * other ambiguities?
  * what conversions should be detected?
    * reference binding
    * exact: pointer qualification adjustment
    * promotion: `float -> double`, `I -> int`
    * conversion: `A/E -> A`
  * which match-all types required?
    * could match all T and reject later?
      * no, would not get correct conversion result
  * placeholder types
    * any
      * `T*` (unary-plus, relation, equality, assign-r)
      * `T*[volatile]&` (assign-l)
    * arithmetic
      * `[volatile]T&` (incr, decr, assign-l)
    * integral
      * `[volatile]T&` (assign-l)
    * object
      * `T*[volatile]&` (incr, decr)
      * `T*` (deref, ptr-add, subscript)
    * function
      * `T*` (deref)
    * promoted-arithmetic
      * `T` (assign-r, unary-plus, unary-minus, mul, add, relational, equality, conditional)
    * promoted-integral
      * `T` (compl, bitwise, shift, remainder)
    * class
      * `T*` (mptr-call)
    * mptr
      * `T` (mptr-call, equality, assign-r)
      * `[volatile]T&` (assign-l)
  * add overloads
    * deref: `T* -> T&`
    * unary-add: `T* -> T*` or `T -> T`
    * compl: `T -> T`
    * mptr-call: `CV1 C1*, CV2 T C2::* -> CV12 T&`
    * mul, add: `L, R -> LR`
    * relational, equality: `L, R -> bool`
    * ptr-add: `T*, ptrdiff_t -> T*`
    * subscript: `T*, ptrdiff_t -> T&`
    * ptr-sub: `T, T -> bool`
    * relational, equality: `T, T -> bool`
    * bitwise: `L, R -> LR`
    * shift: `L, R -> L`
    * assign: `[volatile]L&, R -> [volatile]L&`
    * 
  * issue: how to ensure both arguments are same type when matching `(T, T)` ?