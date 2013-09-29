
#ifndef INCLUDED_CPPPARSE_COMMON_PROFILER_H
#define INCLUDED_CPPPARSE_COMMON_PROFILER_H

#include <iostream>
#include <iomanip>

#ifdef WIN32
#if 0 // enable vtune pause/resume
#include "ittnotify.h"

#pragma comment(lib, "libittnotify.lib")

struct ProfileScopeEnableCollection
{
	ProfileScopeEnableCollection()
	{
		__itt_resume();
	}
	~ProfileScopeEnableCollection()
	{
		__itt_pause();
	}
};

struct ProfileScopeDisableCollection
{
	ProfileScopeDisableCollection()
	{
		__itt_pause();
	}
	~ProfileScopeDisableCollection()
	{
		__itt_resume();
	}
};

#else

struct ProfileScopeEnableCollection
{
};

struct ProfileScopeDisableCollection
{
};

#endif

#endif // WIN32


//#define PROFILE_ENABLED
#ifdef WIN32

typedef __int64 CPUTime;

CPUTime getCPUTimeElapsed();


#else

typedef int CPUTime;

inline CPUTime getCPUTimeElapsed()
{
    return 0;
}

#endif


struct ProfileEntry
{
	const char* name;
	CPUTime elapsed;
	CPUTime elapsedChild;
	ProfileEntry(const char* name) : name(name), elapsed(0), elapsedChild(0)
	{
	}
};

struct Timer
{
	CPUTime time;
	void start()
	{
		time = getCPUTimeElapsed();
	}
	CPUTime elapsed() const
	{
		return getCPUTimeElapsed() - time;
	}
};

struct Profiler
{
	static struct ProfileScope* scope;
};



#ifdef PROFILE_ENABLED

struct ProfileScope
{
	ProfileScope* parent;
	ProfileEntry& entry;
	Timer timer;
	ProfileScope(ProfileEntry& entry)
		: parent(Profiler::scope), entry(entry)
	{
		Profiler::scope = this;
		timer.start();
	}
	~ProfileScope()
	{
		Profiler::scope = parent;
		CPUTime elapsed = timer.elapsed();
		entry.elapsed += elapsed;
		if(Profiler::scope != 0)
		{
			Profiler::scope->entry.elapsedChild += elapsed;
		}
	}
};

#else

struct ProfileScope
{
	ProfileScope(ProfileEntry& entry)
	{
	}
};

#endif

inline void dumpProfile(ProfileEntry& entry)
{
	std::cout << std::setw(12) << std::left << entry.name
		<< std::setw(0) << " "
		<< std::right << std::setw(20) << (entry.elapsed - entry.elapsedChild)
		<< std::setw(0) << "\n";
}

extern ProfileEntry gProfileIo;
extern ProfileEntry gProfileWave;
extern ProfileEntry gProfileLexer;
extern ProfileEntry gProfileParser;
extern ProfileEntry gProfileLookup;
extern ProfileEntry gProfileDiagnose;
extern ProfileEntry gProfileAllocator;
extern ProfileEntry gProfileIdentifier;
extern ProfileEntry gProfileTemplateId;

#endif


