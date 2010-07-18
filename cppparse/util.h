
#ifndef INCLUDED_CPPPARSE_UTIL_H
#define INCLUDED_CPPPARSE_UTIL_H


#include <string.h>
#include <vector>


inline const char* findFilename(const char* path)
{
	const char* result = strrchr(path, '\\');
	if(result == 0)
	{
		result = path;
	}
	else
	{
		++result;
	}
	return result;
}

inline const char* findFilenameSafe(const char* path)
{
	if(path != 0)
	{
		return findFilename(path);
	}
	return "<unknown>";
}

inline const char* findExtension(const char* path)
{
	const char* result = strrchr(path, '.');
	return result == 0 ? path + strlen(path) : result;
}

inline bool string_equal_nocase(const char* string, const char* other)
{
#ifdef WIN32
	return _stricmp(string, other) == 0;
#else
	return strcasecmp(string, other) == 0;
#endif
}





struct StringRange
{
	const char* first;
	const char* last;
	StringRange(const char* first, const char* last)
		: first(first), last(last)
	{
	}
};

inline StringRange makeRange(const char* s)
{
	return StringRange(s, s + strlen(s));
}

inline size_t getLength(const StringRange& range)
{
	return range.last - range.first;
}


struct Concatenate
{
	typedef std::vector<char> Buffer;
	Buffer buffer;
	Concatenate(const StringRange& left, const StringRange& right)
	{
		buffer.reserve(getLength(left) + getLength(right) + 1);
		append(left);
		append(right);
		buffer.push_back('\0');
	}
	Concatenate(const StringRange& left, const StringRange& mid, const StringRange& right)
	{
		buffer.reserve(getLength(left) + getLength(mid) + getLength(right) + 1);
		append(left);
		append(mid);
		append(right);
		buffer.push_back('\0');
	}
	void append(const StringRange& range)
	{
		buffer.insert(buffer.end(), range.first, range.last);
	}
	const char* c_str() const
	{
		return &(*buffer.begin());
	}
};

#endif


