
#include "Profile.h"

#ifdef WIN32
#include <windows.h>


__int64 getCPUTimeElapsed()
{
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return tmp.QuadPart;
}
#endif

ProfileScope* Profiler::scope = 0;

ProfileEntry gProfileIo = ProfileEntry("io");
ProfileEntry gProfileWave = ProfileEntry("wave");
ProfileEntry gProfileLexer = ProfileEntry("lexer");
ProfileEntry gProfileParser = ProfileEntry("parser");
ProfileEntry gProfileLookup = ProfileEntry("lookup");
ProfileEntry gProfileDiagnose = ProfileEntry("diagnose");
ProfileEntry gProfileAllocator = ProfileEntry("allocator");
ProfileEntry gProfileIdentifier = ProfileEntry("identifier");
ProfileEntry gProfileTemplateId = ProfileEntry("template-id");
