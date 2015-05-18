  * e.g. `dependent.A::B::C::m`
  * `A` and `B` could be namespace or type
  * if `A`, `B` or `C` are types, each may be a template-id
  * namespace always comes before type
  * `C` always names a type which is a base of `dependent`
  * `m` always names a member of the type of `dependent` via the base `C`
  * current deferred lookup for `typename T::Type` relies on namespace lookup not being deferred
    * assumes lefthand side always names a type that can be substituted in context of instantiation
  * need type element similar to `DependentTypename`
    * contains list of qualifying ids that are either namespace-name or class-name
      * up to first template-id?
    * contains enclosing-scope for lookup of first nested name specifier
    * contains type of dependent object expression
    * when evaluated
      * substitute dependent object expression
      * lookup first name in both enclosing scope and type of object expression
        * lookup in enclosing scope should search all base-classes, not just non-dependent?
  * simpler just to wrap namespace as a `UniqueType`?
    * evaluating new `DependentTypename` may then result in a namespace
    * could use `SimpleType`, or `NamespaceType`
    * or could
  * add special polymorphic type for nested-name-specifier
    * abstract wrapper for a (possibly qualified) qualifier
    * equivalent of `Qualifying`, but uniqued
    * can be substituted to yield concrete type/namespace in which name can be looked up
    * either
      * class-member-access expression + context
      * qualifying + identifier (class or namespace name)
      * qualifying + template-id
  * include context when uniquing DependentTypename
    * both `objectExpression` and `enclosingScope`
    * if nested-name-specifier prefix is dependent, allow empty qualifying
    * allow finding namespace
  * closer analysis of the standard suggests that nested name specifier prefix in class-member-access cannot be dependent