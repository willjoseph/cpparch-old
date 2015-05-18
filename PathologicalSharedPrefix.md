Issue: slow parsing of `cpptree.h`

  * profile parse, find out exactly what's taking so long
  * caused by pathological case: nested template-ids
    * isDependent is pathologically slow for nested template-ids
      * will always return false when invoked outside a template declaration
      * must examine all template-arguments and their bases
        * a type is dependent if it is a template-parameter visible in an enclosing scope or template-declaration
    * each iteration is parsed first as nested-name-specifier
      * cache successful parse of template-id component
  * shared-prefix: simple-type-specifier:
    * nested-name-specifier -> type-name -> simple-template-id
    * type-name -> simple-template-id
  * shared-prefix: id-expression:
    * qualified-id -> nested-name-specifier -> type-name -> simple-template-id
    * unqualified-id -> template-id -> simple-template-id
  * shared-prefix: template-argument:
    * type-id -> simple-type-specifier -> simple-template-id
    * postfix-expression -> postfix-expression-construct -> simple-type-specifier -> simple-template-id
    * id-expression -> simple-template-id
  * factor out shared-prefix between nested-name-specifier and type-name?
    * check that this is actually the cause of the slow parse?
    * problem with basic.lookup.qual: During the lookup for a name preceding the :: scope resolution operator, object, function, and enumerator names are ignored.
    * occurs: base\_specifier, mem\_initializer\_id\_base, typename\_specifier, simple\_type\_specifier\_name/template
> > > simple-type-specifier:
> > > > [::] [nested-name-specifier] type-name
> > > > [::] nested-name-specifier template simple-template-id

> > > nested-namespace-specifier:
> > > > namespace-name :: [nested-namespace-specifier-seq]

> > > nested-namespace-specifier-seq:
> > > > nested-namespace-specifier [nested-namespace-specifier-seq]

> > > qualified-type-name:
> > > > [nested-namespace-specifier-seq] type-name [qualified-type-name-suffix]

> > > qualified-type-name-suffix:
> > > > :: type-name [qualified-type-name-suffix]
  * use LR parse instead?
    * how much do context actions rely on the assumption that parsing is top-down?
  * cache the type-name parsed from the beginning of a nested-name-specifier?
    * when nested-name-specifier parse fails at '::', defer backtrack and store type-name
      * need to re-lookup type-name to check that it is not hidden by a non-type-name
      * make '::' optional in nested-name-specifier when specifying a type-name?
  * cache a successfully parsed template-argument-clause
    * only for the duration of parsing simple-type-specifier
    * cache forward-declarations within template-argument-clause
      * problem: declaration added to enclosing scope, address taken
        * store declaration by reference?
        * should declaration be added to enclosing scope?
          * name not visible in enclosing namespace scope (Comeau, msvc)
      * problem: declaration of function-type contains parameter-scope
      * avoid deallocation/destruction of cached parse-tree
        * when nested-name-specifier parse fails at '::', preserve parse-tree allocation and do not destroy template-argument-clause
        * relocate parse-tree: generate relocateSymbol()?
          * relocates from one allocator to another
            * for each node, visit children, create copy, delete original, replace original ref with copy
          * copying seems wasteful, when original could be left in-place.
          * defer deletion until next allocation, defer next allocation until successful parse
            * does anything cause allocation before cache hit?
              * empty lists allocate a node
              * isDependent causes construction of empty Scope, empty multimap allocates a node
              * ParameterDeclarationClauseWalker creates a new scope
                * create scope on stack? Scope::Declarations multimap allocates a node
                * implement scope-swap?
                  * Scope::parent/Declaration::scope pointer fixup required?
            * on successful parse, replace cache with symbol
            * perform cache lookup for each subsequent parse attempt, using rtti
            * on cache hit, return cached parse-tree, indicate that result is cached
      * defer deallocation of cached symbol table
        * problem: ETS declaration in failed parse-tree is found by lookup of subsequent parse attempt
          * move point of ets declaration to end of decl-specifier-seq?
          * flush cached parse-tree at point of declaration
          * in findDeclaration, assert that cached parse-tree does not contain declarations?
      * manage parse-tree allocations
        * defer destruction/deallocation of symbol data within allocator
          * list of successfully parsed symbols
            * with point of allocation
          * causes problem with removal of elements in std::map
            * microsoft implementation doesn't use construct() correctly
            * replace with a hash\_map?
        * need to preserve parse-tree, until next successful parse causes partial unwind
        * added complication: need to remove ETS allocations from enclosing scope
        * remove ScopeGuard and declarations/scopes in WalkerBase
        * don't use deleteSymbol/SymbolDelete
        * avoid copying parse-tree allocations
        * fix !ALLOCATOR\_DEBUG
          * don't destroy scopes/declarations
          * only undeclare ETS declarations