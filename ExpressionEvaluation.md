~~task~~: implement evaluation of non-dependent integer-constant-expression
  * required by is-null-pointer-constant check in overload resolution
  * required by template-partial-specialization resolution

task: evaluate expression immediately if possible
  * if only type-dependent, find value
  * if only value-dependent, find type
  * if neither, find both
  * substitute evaluated nodes into expression tree?
    * need to preserve tree if only value-dependent or only type-dependent?
    * e.g. sizeof(T) is only value-dependent
    * is only value-dependent even possible?
  * for now, store evaluated type/value in ExpressionWrapper

~~investigate~~: function call: correct expression to pass into `findBestOverloadedFunction`
  * either member-call: `object-expression id-expression ( expression-list )`
    * either explicit class-member-access: `a.m()`
    * or implicit: `(*this).m()`
    * or static: `m()`
  * or ordinary-call: `id-expression ( expression-list )`
  * other forms do not require overload resolution
    * pointer to function
    * pointer to member function
