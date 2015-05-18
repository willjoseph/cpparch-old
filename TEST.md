## Features ##
  * check that using-declaration is linked to correct definition: the one visible at point of using-declaration
  * check that template-arguments are 'evaluated in the context in which the entire postfix-expression occurs'
  * check that include-path search order is consistent with MSVC/GCC
  * test that the simple-template-id in simple-type-specifier-template is looked up correctly
  * test that template-arguments in a nested-name-specifier-suffix-template are considered when deciding that the NNS is dependent
  * test that names introduced into namespace by using-directive correctly hide previously-declared names
  * test parser for completeness and compliance
  * test that a name found only in a friend-declaration can be found during argument-dependent name-lookup
  * test injected-class-name
  * test integral promotion for enum
  * tests for argument-dependent-lookup
    * friend definitions
  * tests for overload resolution
    * ranking of implicit conversion sequences, all rules in [over.ics.rank]
    * choice of built-in operator over user-defined overload
      * ensure only the correct set of built-in operator overloads are used, and do not hide user-defined overloads
  * tests for partial ordering of function templates
  * tests for implicitly-declared copy-assignment operators
    * overload resolution for built-in `operator=` in expression `x = y`
    * explicit call to `a.operator=()`
  * test: correct type of expression x = y (should be type of 'x'?)
    * test overload resolution against user defined operator=


## Unit Tests ##
  * test\_include should give warning about using 'TEST\_INDIRECT'
  * test for known warnings from specific source files
  * fail on unexpected warnings


## Test Cases ##
  * document existing tests
  * ensure there are tests for:
    * nested-name-specifier-prefix: name lookup ignores object/function/enumerator names
    * type-name lookup hidden by constructor
  * test conversion function parse special-case
> > The conversion-type-id shall not represent a function type nor an array type. The conversion-type-id in a
> > conversion-function-id is the longest possible sequence of conversion-declarators. [Note: this prevents
> > ambiguities between the declarator `operator *` and its expression counterparts. [Example:
```
&ac.operator int*i; // syntax error:
// parsed as: &(ac.operator int *) i
// not as: &(ac.operator int)*i
```
> > The `*` is the pointer declarator and not the multiplication operator. ] ]
    * add test that checks for a syntax error in this case