  * implement class-template instantiation
    * specializations are chosen at point of instantiation
    * dependent-names/types are bound at point of instantiation-
      * SimpleType: contains list of declarations, one for each dependent-name in the template
        * including base classes
    * partial-ordering of class template specialisations



investigate: template instantiation
  * perform substitution on dependent unique-types instead of Type
```
	template<typename T>
	struct S
	{
		T m; // 'S<T>' contains dependent unique-type 'T'

		template<typename U>
		struct V
		{
			U m; // 'S<T>::V<U>' contains dependent unique-type 'U'
		};
	};

	S<int> s; // instantiate S<T> -> S<int>
	S<int>::V<int> v; // instantiate S::<T>::V<U> -> S<int>::V<int>
```
  * `S<T>` is `TypeInstance`, with one dependent template-argument
  * unique:
  * instantiate:

investigate: what is instantiated when instantiating a class?
  * direct member declarations: yes
  * member-functions: no
  * member-classes: no

task: build list of dependent types for deferred uniquing/instantiation in instantiateClass
  * cannot instantiate until unique-type is known, should hold reference to placeholder 'deferred' unique-type
  * avoid duplicates: use dependent unique-type to distinguish?
  * do this whenever complete type is required, but type is dependent

issue: for class-template, out of line definition of static member is (incorrectly) added to list of child instantiations
  * not usually a problem, except that class-template is 'partially instantiated' during instantiation of its children
    * side effects of child instantiation may require name-lookup within the partially-instantiated class, will fail if names do not
  * static members should not be instantiated until they are used?

task: disallow implicit instantiation of a template within its own definition
