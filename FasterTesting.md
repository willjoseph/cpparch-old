  * serialise preprocessed tokens?
    * is this actually faster than reading preprocessed file?
    * test for 80,000 line preprocessed input file:
      * Debug: saves around 10 seconds from total of ~1 minute
      * Release: saves around 2-3 seconds from total of 10 seconds
  * automate debugging of .prepro.cpp after failure in Release build
    * omit $predefined tokens from .prepro.cpp