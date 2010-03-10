
#ifndef INCLUDED_CPPPARSE_PROFILER_H
#define INCLUDED_CPPPARSE_PROFILER_H

#include <iostream>
#include <iomanip>

__int64 getCPUTimeElapsed();

struct ProfileEntry
{
	const char* name;
	__int64 elapsed;
	__int64 elapsedChild;
	ProfileEntry(const char* name) : name(name), elapsed(0), elapsedChild(0)
	{
	}
};

struct Timer
{
	__int64 time;
	void start()
	{
		time = getCPUTimeElapsed();
	}
	__int64 elapsed() const
	{
		return getCPUTimeElapsed() - time;
	}
};

struct Profiler
{
	static struct ProfileScope* scope;
};


struct ProfileScope
{
	ProfileScope* parent;
	ProfileEntry& entry;
#if PROFILE_ENABLED
	Timer timer;
#endif
	ProfileScope(ProfileEntry& entry)
		: parent(Profiler::scope), entry(entry)
	{
		Profiler::scope = this;
#if PROFILE_ENABLED
		timer.start();
#endif
	}
	~ProfileScope()
	{
		Profiler::scope = parent;
#if PROFILE_ENABLED
		__int64 elapsed = timer.elapsed();
		entry.elapsed += elapsed;
		if(Profiler::scope != 0)
		{
			Profiler::scope->entry.elapsedChild += elapsed;
		}
#endif
	}
};

inline void dumpProfile(ProfileEntry& entry)
{
	std::cout << std::setw(10) << std::left << entry.name
		<< std::setw(0) << " "
		<< std::right << std::setw(20) << (entry.elapsed - entry.elapsedChild)
		<< std::setw(0) << "\n";
}

extern ProfileEntry gProfileWave;
extern ProfileEntry gProfileLexer;
extern ProfileEntry gProfileParser;
extern ProfileEntry gProfileDiagnose;
extern ProfileEntry gProfileAmbiguity;
extern ProfileEntry gProfileAllocator;
extern ProfileEntry gProfileIdentifier;

#endif


