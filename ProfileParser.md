  * 10%: construction/destruction of multi-inherited interfaces
    * remove virtual-destructors <-- 10% speedup
    * non-virtual concrete symbols?
  * 6%: lex-names
    * 5%: char\_traits<>::compare
    * 1%: makeFilename: 0.5%: convert forward-slash to backslash
      * avoid slash-conversion unless absolutely necessary (assert correct convention)
    * customise lex-token for fast comparison and share between lexer and parser?
    * use hash-table?
  * 1% findDeclaration


  * instrument code to profile only parser
  * profile with /Oy (omit frame pointer)
    * seems to reduce overhead of function calls
    * tests not conclusive, need to measure more accurately
  * ~~rtti in comparison of `UniqueType`~~
    * `abstractLess`: `std::type_info::before()` is very slow.
      * comparing `type_info` ptr is faster, but not standard compliant
      * 300ms -> 120ms
      * many comparisons occurring
    * `isClass/isEnum`: `std::type_info::operator==()` is slow.
    * measure cost of `typeid()`
      * halving number of calls : 120ms -> 70ms
      * typeid() cost is 100ms out of 120ms
  * ~~`deferredBacktrack`~~ (~7th in profile)
    * large overhead to call, only to find that no backtrack is required
    * move rarely-taken branch into separate function
  * ~~`isDependent`~~
    * type substitution for dependent-typename
    * `isDependent(*context.enclosingType)`
    * this is a temporary workaround?
    * required in asserts generally
    * could cache in unique-type
  * ~~`makeInnerWalker` is not inlined?~~
    * refactor to `SemaInner`
  * compare performance with/without exception handling
    * remove try/catch
    * remove throw
  * `deduceFunctionCall`
    * throwing exception for deduction failure is slow
  * frequent exception throwing
    * `TypeError`
    * occurs during substitution
  * rb-tree search in `pushUniqueType`
    * replace with hashtable
  * `findDeclaration` (7% of total)
    * upper\_bound is expensive
    * replace with hashtable
    * needs to be ordered?
      * no, but needs to allow duplicate keys and be stable
  * allocations for vector are expensive
    * ensure all vectors are reserved up front
    * implement immutable shared vector
      * with custom allocator, never need to delete?
      * except when used as a temporary array?
    * alternative: unique vector
      * never need to delete
      * fast comparisons
      * hackable with std containers?
    * temporary vectors before uniquing:
      * use custom allocator?
        * requires keeping ptr to custom allocator
          * not a dealbreaker, but touches a lot of code
        * single global instance?
          * works for stack allocator
          * not multithread-friendly, but that's ok
        * inline allocator: optimise for <= 8 elements?
          * can't swap?
        * how to enforce safe usage of stack allocator?
      * vector with different allocator cannot be trivially added to std::set
        * breaks insert/find/upper\_bound/key\_comp
    * will be using custom hashtable, but still want to preserve std interface
      * values are ranges without ownership
      * custom always-growing allocator: no need to delete
  * `pushUniqueType<SimpleType>()`
    * in `addOverload`: `makeUniqueSimpleType(*context.enclosingType)`
    * in `typeOfFunctionCallExpression`: `makeUniqueSimpleType(classType)`
    * copying non-trivial `SimpleType` is expensive
      * when copying childInstantiations!
        * should not be copied during substitution: only relevant for dependent types
      * muliple copies may occur during pushUniqueType
      * only need to copy the immutable key info: primary, template-arguments, enclosing
    * faster way to get unique-type for enclosingType?
    * prevent substitution for non-dependent type
    * substitution occurring unnecessarily?
      * need to memoize substitution result?
      * substitution occurs for type `X<T>` for each set of template-arguments
  * in `SemaTemplateArgumentList::commit()`: `ListReference::push_front` is slow
    * copying of `TemplateArgument`
  * `isReference/isClass` etc still showing in profile (though inlined)
    * value of `typeid(T)` is not compile-time-constant
    * use new technique instead
  * `parseSymbol(simple_type_specifier_builtin)`
    * common types (char, long, double, short) showing up in profile
    * `PARSE_SELECT_TOKEN`: duplication of large chunk of code for 'hit' branch?
    * could be noise: e.g. refill(), overhead of disabling profiling
    * overhead for createSymbol?
      * `simple_type_specifier_builtin` has less than 10 bases
  * allocations during overload resolution
    * `CandidateFunction` members
    * when replacing an existing best candidate
    * when destroying an `OverloadResolver`
    * allocate enough mem for N arguments on stack?
  * `SemaState` constructor
  * general `pushUniqueType` stuff during type-substitution
    * in key-compare
    * problem because of rtti during find?



~~task~~: optimise parser allocations
  * ensure allocator uses good block size: 128kb?
  * use parse-tree allocator where possible
    * scopes
    * declarations
    * types
  * avoid allocations where possible
    * don't allocate symbol until successful parse
      * measure cost
    * avoid copying 'qualifying' when creating WalkerState: e.g. parsing nested-name-specifier
      * ref instead of copy
    * don't track list of scopes/declarations unless debugging

~~task~~: optimise name-lookup
  * hashtable?
  * single global id table, id:scopes?
    * how to sort scopes?
  * trace name-lookup pattern
  * went with a std::multimap