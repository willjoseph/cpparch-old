  * don't crash when backtracking after parse error!
  * if member-initializer parse fails, member declaration is already committed
    * committed declaration cannot be deallocated (unless forward-declaration), causes allocator check failure
    * need to commit at point of declaration: failure to parse member-initializer is fatal
    * detect that this is a fatal error and don't clean up symbols?
    * for now, track all declarations and clean up if parse fails..
      * doubles parse time, optimise!
      * don't track declarations (except forward-decl) unless ALLOCATOR\_DEBUG