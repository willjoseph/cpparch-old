
#if defined X
X()
#endif

#include <crtdbg.h>
#include <malloc.h>

#include "include/unused.h"
#include "include/empty.h"
#include "include/used.h"
#include "include/used_duplicate.h"


const int TEST = USED;

void test()
{
	used();
}

