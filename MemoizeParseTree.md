  * generalised solution for re-using failed parse-tree
    * requires that re-parsing a symbol in-place behaves the same way
      * make type-name parse behave the same way inside or outside a nested-name-specifier-prefix
        * defer rejection of type names hidden by non-type/namespace names
    * pattern of swapping walker result values means only outermost walker is valid
      * make walker results a weak-reference, except for outermost walker?
        * replace Type::swap with non-copying assign?
          * same for template-arguments/params
        * splice dependents without removing from inner list?
        * store types in template-argument-list by reference?
        * DeferredSymbols?
      * avoid calling inner walker destructors?
        * walker destructors should only be called when debugging allocator?
          * do walkers own global-allocator allocations?
            * only OpaqueCopied
          * do walker destructors have side effects?
            * WalkerBase::declarations removes non-committed declarations on fail
            * DeferredSymbols
      * defer deletion of symbol-table data
    * handle ETS declaration side-effects of failed parse tree
      * defer modification of enclosing scope until..?
        * completion of statement?
      * when does ETS declaration modify enclosing scope?
        * decl-specifier-seq or parameter-declaration-clause in namespace scope -> containing namespace
        * else, smallest non-class non-function-prototype scope that contains the declaration
    * handle scope creation side-effects of failed parse tree
      * preserve allocated scopes


~~task~~: clean up caching and deferred destruction
  * clean up CachedSymbols
    * enable caching at any point in parse
    * use linear-allocator
    * remove entry from cache when alternative parse succeeds
      * otherwise, causes error when looking up template-id cached within nested-name-specifier-suffix?
    * treat cache as stack, only top item considered?
      * handle X

&lt;Y&gt;

::Z

&lt;W&gt;


    * cache more than just template-id
    * cache multiple symbols for each position
      * e.g. declarator-id -> id-expression -> unqualified-id -> template-id -> identifier
      * linear search?
      * test case
        * type-specifier -> nested-name-specifier -> nested-name -> type-name -> identifier
        * type-specifier -> type-name -> identifier
    * cache shared-prefix 'class-key'
      * type-specifier -> elaborated-type-specifier -> elaborated-type-specifier-key -> class-key
      * type-specifier -> class-specifier -> class-head -> class-key

~~task~~: clean up BacktrackCallbacks
  * use linear-allocator
  * use custom list class

task: clean up ParserAllocatorWrapper
  * defer destruction at level of container
    * need replacement for multimap?

task: further improvements to BacktrackCallbacks
  * store items grouped into frames, keyed off allocation-position stored in parser
  * specialise list class for front-insertion
