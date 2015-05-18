  * name lookup fails for id-expression 'operator=' when it has not been declared
  * correct behaviour: name lookup for 'operator=' within class should always return default, if not found
    * need a proper declaration to find enclosing class, but implicit declaration has no associated identifier?
      * too complex to manufacture declaration from nothing
    * avoid conflict with overload declarations
  * cannot determine whether copy-assign is already declared until class is instantiated
  * need to include declaration in overload set for call to operator=
  * can't declare in base declarations for type, must synthesise special declaration?
  * consider name-lookup of operator= while inside a template definition
    * name lookup will be deferred if operands are dependent