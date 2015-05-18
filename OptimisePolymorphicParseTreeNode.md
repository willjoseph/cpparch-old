  * overhead of multiple-inherited vtables in identifier: 34 vtable pointers, 136 bytes
    * constructor calls are slow
    * is there an alternative mechanism for visiting parse-tree?
    * visiting is not required during parse!
      * removing vtable reduces overhead to 1 byte per base-class, trivial constructor
      * should be zero with empty-base-optimisation
      * remove multiple-inheritance?
      * need alternative method to enforce valid casting?