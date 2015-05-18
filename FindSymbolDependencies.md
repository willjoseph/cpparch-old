~task~: exclude predefined symbols
  * pass contents of predefined\_msvc.h directly to lexer, rather than via #include

**use consistent path-separator/root in filenames
  * how does wave derive the absolute root from the relative include-path?** including macro definitions
**support overloaded-operators** ignore namespace names
**perform type-dependent name-lookup for template instantiations** handle multiple identical typedef/forward-declaration in different modules
**if symbol is forward-declared, link to primary declaration if visible**

**when module depends on name of overloaded function, it only depends on selected overload
  * other overloads could be removed and meaning of program would be the same**

investigate: rules for which overloads are visible at point of instantiation
  * only those that were visible at point of declaration of template?
> [temp.res]
> Three kinds of names can be used within a template definition:
> - The name of the template itself, and names declared within the template itself.
> - Names dependent on a template-parameter.
> - Names from scopes which are visible within the template definition.
> [temp.res]
> Inside a template, some constructs have semantics which may differ from one instantiation to another. Such a
> construct depends on the template parameters. In particular, types and expressions may depend on the type
> and/or value of template parameters (as determined by the template arguments) and this determines the
> context for name lookup for certain names.
> Such names are unbound and are looked up at the point of the template instantiation in both the
> context of the template definition and the context of the point of instantiation.
> [temp.dep.res]
> In resolving dependent names, names from the following sources are considered:
> - Declarations that are visible at the point of definition of the template.
> - Declarations from namespaces associated with the types of the function arguments both from the
> instantiation context and from the definition context.
> [temp.dep.candidate]
> For a function call that depends on a template parameter, the candidate functions are found using the usual
> lookup rules (3.4.1, 3.4.2, 3.4.3) except that:
> - For the part of the lookup using unqualified name lookup (3.4.1) or qualified name lookup (3.4.3), only
> function declarations from the template definition context are found.
> - For the part of the lookup using associated namespaces (3.4.2), only function declarations found in
> either the template definition context or the template instantiation context are found.
> If the function name is an unqualified-id and the call would be ill-formed or would find a better match had
> the lookup within the associated namespaces considered all the function declarations with external linkage
> introduced in those namespaces in all translation units, not just considering those declarations found in the
> template definition and template instantiation contexts, then the program has undefined behavior.

  * during instantiation, argument dependent lookup may find names that were declared after the definition of the template!
    * but only those declared before the point of instantiation
    * this means that the dependencies may differ depending on the point of instantiation
      * this is undefined behaviour according to the standard
      * would be good to detect this and warn?
      * could detect declarations after point of instantiation that would otherwise have been used
  * if module A requires instantiation of a template specialization, dependencies may differ depending on whether that specialization was previously instantiated by module B
    * only if better candidate would be found by ADL in the latter instantiation
    * detect and warn?
      * if any instantiation would find a better candidate via ADL
      * is there a valid reason to make use of this undefined behaviour?
    * in this scenario, module A effectively depends on the instantiation in module B
      * if the point of instantiation changed, the meaning of the program would change
  * in a well formed program with no UB, unqualified names used in a template must be defined before the first point of instantiation
    * an instantiation can therefore only depend on unqualified names declared earlier
