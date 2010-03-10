
#include "profiler.h"

#include <windows.h>


ProfileScope* Profiler::scope = 0;

ProfileEntry gProfileWave = ProfileEntry("wave");
ProfileEntry gProfileLexer = ProfileEntry("lexer");
ProfileEntry gProfileParser = ProfileEntry("parser");
ProfileEntry gProfileDiagnose = ProfileEntry("diagnose");
ProfileEntry gProfileAmbiguity = ProfileEntry("ambiguity");
ProfileEntry gProfileAllocator = ProfileEntry("allocator");
ProfileEntry gProfileIdentifier = ProfileEntry("identifier");
ProfileEntry gProfileTemplateId = ProfileEntry("template-id");

__int64 getCPUTimeElapsed()
{
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	return tmp.QuadPart;
}

