investigate: `TemplateIdWalker` doesn't do name lookup, leads to unnecessary failed parse when expression begins with `a < b`
  * in case of `a < b || c` causes error evaluating expression type if `b` is not convertible to bool
  * ideally, defer evaluation of expression type until ambiguity is resolved.
  * temporary hack to avoid?
  * conformant solution (see below): look up identifier during template-id parse, and reject if it does not name a template

don't need to handle template-id ambiguity between id-expression and type-name?
  * can template-name ever be hidden?
  * always look up template-name immediately and reject if first name found is not template?
    * test implementation works well
  * what if hiding name is non-template?
    * gcc, clang, msvc all reject
  * what if within class-member-access?
    * lookup first in class of object-expression, then in enclosing scope
    * if object expression is dependent, name lookup should be deferred when looking for non-type name?
      * only if preceded by 'template'
    * if not preceded by 'template', first look in class of object expression (if present and non-dependent), then in enclosing scope
  * what if within declarator?
    * e.g. `C<T>::C()` vs `C<T> f()`
      * still ok to look up 'C' to find whether it's a template
    * e.g. `C<T>::M<U>::M()` vs `typename C<T>::template M<U> f()`
      * different behaviour when in declarator?
        * `C<T>` is known to be the current instantiation, therefore lookup of `M` is not deferred
      * otherwise, lookup of second `M` deferred because qualified by dependent
      * still ok to look up template name if not preceded by `template`?
        * even if qualified by dependent type name?
          * don't bother with lookup in this case?
          * what if `C<T>::M` names a non-type and therefore `<` could be less-than?
          * impossible, always preceded by `template<>`?
    * e.g. `template<> template<> C<int>::M<int>::M()` vs `template<> template<> C<int>::M<int> C<int>::M<int>::f()`
      * different behaviour when in declarator?
      * still ok to look up template name if not preceded by `template`
      * if qualified, look up in qualifying
    * issueL within declarator, `qualifyingClass` is not valid even when type was not dependent