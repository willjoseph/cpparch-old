  * reduce size of call-stack
    * modify parse-tree: convert recursive list parse rules to iterative using 'sequence'
    * sample of small call-stack within expression-parsing
      * 233 levels
      * `parseExpression`: 11
      * ~~`parseSymbol`~~: 37
      * `parseSymbolRequired`: 38
      * ~~`ParserGeneric::parse`~~: 38
      * `cpp::[symbol]::parse`: 13
      * `ParsingVisitor::visit`: 13
      * `Walker::visit`: 26
      * `Default.ChoiceParser::parseSymbol`: 22
  * try vc10
    * build boost, stlport
    * single-step faster if not looking at call-stack window