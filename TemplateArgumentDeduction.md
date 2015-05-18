  * what is "type deduction"?
    * the process of determining the template arguments, from a set of dependent types (e.g. function parameters) and a set of non-dependent types (e.g. function-call arguments)
    * involves 'substitution' of 'substition-failure'
    * `[temp.deduct]` When an explicit template argument list is specified, the template arguments must be compatible with the
> > > template parameter list and must result in a valid function type.
> > > If a substitution in a template parameter or in the function type of the function template results in an invalid type, type deduction fails.
    * unspecified arguments must be deduced before substitution
    * for each dependent type, substitute template-parameters with template-arguments; fail if the result is an invalid type
    * perform function parameter type adjustments
  * steps in argument deduction for function template
    * for each function-parameter, find template-parameters (for allowed forms, ignoring nondeduced contexts) and fill in template-arguments with corresponding type from function-call arguments
  * type substitution for name qualified by dependent type
    * similar process to what currently happens in makeUniqueType
    * look up declaration in enclosing type
    * perform substitution in context of enclosing type
  * partial ordering of partial-specializations
    * compare specialization `A` to specialisation `B`
    * use `A`'s specialization-arguments to match `B`
    * e.g. `A<T*>` matches `B<T>`, but `A<T>` does not match `B<T*>`
    * if A matches `B` but `B` does not match `A`, `A` is more specialized
  * what causes `<const T>` and `int` to fail deduction?
    * likewise, what causes `<const T>` and `const int` to deduce `int` ?
  * deduction for non-type
  * substitution for non-type
  * substitute while deducing?

> > `[temp.deduct]` When all template arguments have been deduced, all uses of template parameters in
> > nondeduced contexts are replaced with the corresponding deduced argument values. If the substitution
> > results in an invalid type, as described above, type deduction fails.

~~task~~: template argument deduction for function templates

~~task~~: template argument deduction for class template partial-specializations
  * specialization is matched if its arguments can be deduced from the template arguments vs the specialization argument list

task: function parameter type transformations during template argument deduction

> `[dcl.fct]` After determining the
> type of each parameter, any parameter of type "array of `T`" or "function returning `T`" is adjusted to be
> "pointer to `T`" or "pointer to function returning `T`," respectively. After producing the list of parameter
> types, several transformations take place upon these types to determine the function type. Any cv-qualifier
> modifying a parameter type is deleted. The resulting list of transformed parameter types is the function's parameter
> type list.

> `[temp.deduct]`
> After this substitution is performed, the function parameter type adjustments described in 8.3.5 are performed.
> [Example: A parameter type of "`void ()(const int, int[5])`" becomes "`void(*)(int,int*)`". ]

