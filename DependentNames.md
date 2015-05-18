  * name lookup for 'this->m' finds 'm' in enclosing scope
    * see [basic.lookup.classref]
    * reject names not found in enclosing class when not considering nested-name-specifier
    * memberClass not set during scope of out-of-line member definition, member lookup fails
    * check when object-expr is dependent and skip lookup, rather than reverting to unqualified lookup

~~task~~: better dependent-ness tracking
  * rules
    * a template parameter is dependent on its scope
    * an unqualified typedef is always dependent on the dependent-scope of the target type
    * a qualified typedef is dependent on the dependent-scope of its qualifying type, or of the target-type (only if the target type is dependent)
    * an unqualified template-id is dependent on the dependent-scope of its template-arguments (or its scope if it names a template-parameter)
  * implementation
    * Type::dependent refers to the innermost dependent-scope at parse-time (or 0 if not dependent)
      * parser maintains dependent-scope reference, persists in Declaration::type::dependent
      * should this be property of Type?
        * allows it to encode template-arguments/qualifying
    * when querying..?

task: distinguish name-of-template-itself as not dependent
  * 14.6 (temp.res) Three kinds of names can be used within a template definition:
    * The name of the template itself, and names declared within the template itself
    * Names dependent on a template-parameter
    * Names from scopes which are visible within the template-definition.


task: remove old isDependent/addDependent framework
  * simplify determination of dependent-ness
    * id-expression: determine declaration has a dependent type when declared
      * can't be dependent if qualified by non-dependent type-name?
      * function template: mark dependent only if depending on template parameter of class template
        * exception: when function template calls itself: name is dependent if within own template declaration
  * clean up makeUniqueType
    * remove allowDependent
    * determine dependentness

