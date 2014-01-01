
#if !BUILD_STANDALONE
#undef BUILD_STANDALONE
#define BUILD_STANDALONE 1

#include "Fundamental.cpp"
#include "KoenigLookup.cpp"
#include "Literal.cpp"
#include "NameLookup.cpp"
#include "OperatorId.cpp"
#include "OverloadResolve.cpp"
#include "Special.cpp"
#include "TemplateDeduce.cpp"
#include "TypeConvert.cpp"
#include "TypeInstantiate.cpp"
#include "TypeSubstitute.cpp"
#include "TypeTraits.cpp"
#include "TypeUnique.cpp"


#endif
