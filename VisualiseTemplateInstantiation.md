  * for a given instantiation, report which instantiations it requires
    * track required instantiations
    * add to list in enclosing-type when calling instantiateClass
      * contains instance+location
      * reversal of existing graph?
        * except that existing graph does not store location
    * instantiation graph: maps enclosing-type to children
      * multimap, key of enclosing type, data of instance+location?
        * hard to debug
  * for each required instantiation
    * link to instantiation page
    * point of instantiation: file/line/column
  * display point of instantiation in source code
    * source file? can't auto scroll without inserting markup
    * inline only selected line(s)?
      * how to display long lines?
      * performance if file is large?
        * create index of source lines during prepro?
          * unnecessary performance hit?
        * ideally could get pointer to location in input file buffer for each token
        * performance not critical for error reporting
      * source lines should only be within template definition?