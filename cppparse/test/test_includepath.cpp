
#include "includepath/header.h"
#ifdef _CPPP_TEST // won't build in msvc?
#include "../msvc-8.0/include/ctrdefs.h" // escape from '.'
#endif
#include <../include/stdarg.h> // escape from msvc stdcpp include-path
#include <../cpparch/cppparse/test/includepath/header4.h> // escape from boost_14_1_0 include-path
