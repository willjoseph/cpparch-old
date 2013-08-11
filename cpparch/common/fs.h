
#ifndef INCLUDED_COMMON_FS_H
#define INCLUDED_COMMON_FS_H

#include <direct.h> // _mkdir, _rmdir
#include <io.h> // _access

inline bool dir_create(const char* path)
{
	return _mkdir(path) == 0;
}

inline bool dir_remove(const char* path)
{
	return _rmdir(path) == 0;
}

inline bool file_exists(const char* path)
{
	return _access(path, 0) == 0;
}

#include <io.h>
#include <stdlib.h>
#include <string.h>

struct Dir
{
	_finddata_t data;
	intptr_t handle;

	Dir(const char* path)
	{
		char tmp[1024];
		strcpy(tmp, path);
		strcat(tmp, "*");
		handle = _findfirst(tmp, &data);
	}
	~Dir()
	{
		if(handle != -1)
		{
			_findclose(handle);
		}
	}
	bool good() const
	{
		return handle != -1;
	}
	void next()
	{
		if(_findnext(handle, &data) != 0)
		{
			_findclose(handle);
			handle = -1;
		}
	}
	const char* name() const
	{
		return data.name;
	}
};


#endif


