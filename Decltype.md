~~task~~: basic support for `decltype`
  * add to lexer
  * add to grammar
    * simple-type-specifier
    * destructor-name
    * nested-name-specifier
    * base-specifier
    * mem-initializer
  * add semantic actions
    * type-specifier: how to construct Type?
      * add ExpressionWrapper member if representing decltype(x)
  * dependent?
    * need to defer evaluation

task: defer evaluation of dependent `decltype(x)`