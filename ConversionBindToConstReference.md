  * T->T: identity
  * T->const T&:
    * also an identity sequence?
    * qualification adjustment?
  * When a parameter of reference type binds directly (8.5.3) to an argument expression, the implicit conversion
> > sequence is the identity conversion, unless the argument expression has a type that is a derived class of
> > the parameter type, in which case the implicit conversion sequence is a derived-to-base Conversion.
  * The rank of a conversion sequence is determined by considering the rank of each conversion in the sequence and the rank of any reference binding
  * Any difference in top-level cv-qualification is
> > subsumed by the initialization itself and does not constitute a conversion. [ Example: a parameter of type A
> > can be initialized from an argument of type const A. The implicit conversion sequence for that case is the
> > identity sequence; it contains no �conversion� from const A to A. �end example ]
  * If no conversions are required to match an argument to a parameter type, the implicit conversion sequence
> > is the standard conversion sequence consisting of the identity conversion.
