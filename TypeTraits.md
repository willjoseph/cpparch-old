  * gcc: https://gcc.gnu.org/onlinedocs/gcc/Type-Traits.html
  * cpp: http://en.cppreference.com/w/cpp/types
  * msvc: http://msdn.microsoft.com/en-us/library/ms177194.aspx

  * issue: function-specifiers are lost for member declarations that are not constructors
    * state of 'seq' member of SemaSimpleDeclaration is lost..
    * possibly during cached parse of shared-prefix of member\_declaration\_implicit and member\_declaration\_named?
      * not cached!