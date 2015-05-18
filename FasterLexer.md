  * optimise boost::wave?
    * run wave on its own to profile..
    * generally difficult - symbols are too long
    * reading files
      * copying file into string?
      * why not just read straight from buffered iostream?
        * lexer makes assumption that input iterator is a char