
#include "include/cycle.h"

#if defined X
X()
#endif

#include <crtdbg.h>
#include <malloc.h>

#include "include/unused.h"
#include "include/empty.h"
#include "include/used.h"
#include "include/used_duplicate.h"
#include "include/used_indirect.h"


const int TEST = USED;
const int TEST_INDIRECT = INDIRECT;
const Indirect TEST_INDIRECT_TYPEDEF = 0;

void test()
{
	used();
	indirect();
}

