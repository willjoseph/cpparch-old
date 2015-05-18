issue: 'context' is still being instantiated while looking up names within it

```
	LexContext
		-> context
		-> macromap<context>::name_iterator (instantiate macromap<context>)
		-> ContextT::token_type token_type (instantiate ContextT)
```

  * correct behaviour left unspecified, see DR 287
    * "This was discussed at length. The consensus was that the template case should be treated the same as the non-template class case
> > > in terms of the order in which members get declared/defined and classes get completed."
  * during qualified name lookup within instantiation for a member of a class which is partially instantiated, only names declared before the point of instantiation are visible
    * including dependent base classes!
    * but names in bases of partially instantiated class cannot be accessed from within instantiation of its base
  * track point of instantiation for each declaration in WalkerState
    * automatically rolls back if parse fails
    * per instance, because redeclaration may make name visible after being hidden
  * during parse of template definition, when complete type is required
    * if non-dependent, instantiate type and store point of instantiation
    * else add type to list of child instantiations and store declaration visibility
    * when instantiating template, use stored declaration visibility when instantiating child
  * in what contexts do child instantiations occur?
    * type of member definition required to be complete
    * type of base required to be complete
    * type named in member/base type-id
      * nested-name-specifier
    * type in expression required to be complete
      * static-const initializer
      * template-argument expression
        * in specialization arguments