  * refactor evaluation of class-member-access
    * id-expression naming non-static-member cannot exist outside class-member-access: will always be transformed
    * don't evaluate standalone id-expression naming non-static-member
    * in case of dependent id-expression `d.m` need to avoid substituting standalone `m`
  * DependentIdExpression::qualifying: same as isQualifying?
  * ~general renaming of expression types~
    * ~IdExpression::qualifying -> IdExpression::enclosing~
    * ~getIdExpressionClass -> getIdExpressionEnclosing~
    * ~SimpleType -> Instance~
    * ~getSimpleType -> getInstance~
    * ~makeUniqueSimpleType -> makeUniqueInstance~
    * ~InstantiationContext::enclosingType -> InstantiationContext::enclosingInstance~
    * ~SemaBase::enclosingType -> SemaBase::enclosingInstance~
    * ~QualifiedDeclaration::enclosing -> QualifiedDeclaration::enclosingInstance~
    * ~QualifiedDeclaration -> ResolvedDeclaration~
    * ~resolveQualifiedDeclaration -> resolveUsingDeclaration~
    * ~makeUniqueEnclosing -> makeEnclosingClassInstance~
    * ~setEnclosingType -> setEnclosingInstance~
    * ~setEnclosingTypeSafe -> setEnclosingInstanceSafe~
    * ~findEnclosingType -> findEnclosingClass~
    * ~remove InstantiationContext::enclosingFunction and SemaBase::enclosingFunction~
    * ~isEnclosingType -> isEnclosingClass~
    * ~SemaBase::memberClass -> SemaBase::objectExpressionClass~
    * ~classType -> classInstance~
    * ~enclosingClass -> enclosingInstance (as appropriate)~
    * memberEnclosing/idEnclosing
  * refactor expression substitution
  * substitute all dependent types/expressions within class template at point of instantiation
    * substitute member function template declaration
      * handle expression containing non-type template parameter
      * complication: must also partially substitute dependent types/expressions when enclosing class template is instantiated
        * don't defer substitution until enclosing template is instantiated?
      * need to store dependent types/expressions in instance of declaration
        * `template<typename T> class A { template<typename U> void f(X<T, U>); };`
        * `A<T>::f<U>(X<T, U>) -> A<int>::f<U>(X<int, U>) -> A<int>::f<int>(X<int, int>)`
        * type of parameter gets substituted twice; first time `U` is not substituted
        * problem: dependentIndex in type of parameter may refer to outer class template or inner function template
          * need two?
          * can identify instance of declaration via enclosing specialization
          * index could be relative to `f` rather than enclosing class.. would still require same set of substituted types
          * template parameter can either belong to directly enclosing template, or an outer template
          * would generally be good to independently track inner/outer dependentness
          * store depth of innermost and outermost template parameter
        * problem: when substitution fails for a given specialization (SFINAE), instance is left in an invalid state
          * remember that substitution failed, don't retry
    * substitute member static\_assert
    * substitute member typedef: `T m[sizeof(sizeof(T))]`
    * substitute static member definition when used
    * substitute function definition when used
    * subsitute default argument when used
  * substitute dependent types/expressions within non-member function template declaration during overload resolution
    * groups to be substituted
      * class template declaration: default arguments in template-parameter-clause
        * substitute when specialization is named
      * class template definition: member declarations
        * substitute at point of instantiation
      * function template declaration: return type, arguments, default-arguments
        * substitute when used in overload resolution
      * function template definition: statements
        * substitute at point of instantiation
    * when function template is at namespace scope, gather dependent types/expressions
      * store in Declaration::declarationDependent
      * substitute when function is used in overload resolution
    * when function template (or class template) is member of class template, its declaration may depend on outer template parameters
      * when outer class is instantiated, substitute types/expressions dependent upon outer template-parameters
      * build new substitution group when outer class is instantiated, containing only types/expressions which depend on inner template parameters
    * collect dependent types/expressions in template-declaration
  * substitute dependent types/expressions within function template at point of instantiation
    * does point of instantiation have any effect on name lookup?
      * yes, unqualified lookup may find names declared after definition of template, before point of instantiation
    * InstantiationContext::enclosingScope seems wrong: should be the scope in which the expression/type was found
      * used for unqualified name lookup "in the context of the expression"
      * used to determine whether `this` is valid, and for `getEnclosingFunction`
    * what stops instantiation of unqualified member function call?
      * qualified by `(*this).` which has dependent type
  * memoise template-id in case of `N::Tmpl<expr>`
    * template-id is partially recognised as suffix, but fails at missing `::`
    * cache is flushed when `N::` is accepted as nested-name-specifier