
#ifndef INCLUDED_CPPPARSE_PROFILER_H
#define INCLUDED_CPPPARSE_PROFILER_H

#include <iostream>
#include <iomanip>

#define PROFILE_ENABLED

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
		__int64 elapsed = timer.elapsed();
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


