  * `[over.best.ics]` An implicit conversion sequence is a sequence of conversions used to convert an argument in a function
> > call to the type of the corresponding parameter of the function being called. The sequence of conversions is
> > an implicit conversion as defined in clause 4, which means it is governed by the rules for initialization of an
> > object or reference by a single expression (8.5, 8.5.3).
  * `[over.ics.user]` Since an implicit conversion sequence is an initialization, the special rules for
> > initialization by user-defined conversion apply when selecting the best user-defined conversion for a user-
> > defined conversion sequence
  * `[over.match.ctor]`, `[over.match.copy]`, `[over.match.conv]`
  * `[class.conv]` Function overload resolution selects the best conversion function to perform the conversion.
  * what prevents two user-defined conversions occurring in sequence?
  * constructor call
  * conversion function
  * semantics of initializers
    * references?
    * if destination is class-type (non-reference)
      * if source is same or derived class-type, gather converting constructors, perform overload resolution
      * else gather conversion functions and converting constructors, perform overload resolution
    * else if source is class-type (non-reference)
      * gather conversion functions, perform overload resolution
  * what second standard-conversion is required in user-defined conversion involving constructor-call?
    * is result of constructor-call an lvalue/rvalue?
