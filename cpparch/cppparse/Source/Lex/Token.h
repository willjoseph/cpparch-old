
#ifndef INCLUDED_CPPPARSE_LEX_TOKEN_H
#define INCLUDED_CPPPARSE_LEX_TOKEN_H

#include <boost/wave/token_ids.hpp>

struct Name
{
	const char* value;
	Name()
		: value(0)
	{
	}
	explicit Name(const char* value)
		: value(value)
	{
	}
	bool empty() const
	{
		return value == 0;
	}
	const char* c_str() const
	{
		return value;
	}
	void clear()
	{
		value = 0;
	}
};

#define NAME_NULL Name(0)

inline bool operator==(const Name& left, const Name& right)
{
	return left.value == right.value;
}

inline bool operator!=(const Name& left, const Name& right)
{
	return left.value != right.value;
}

inline bool operator<(const Name& left, const Name& right)
{
	return left.value < right.value;
}

#if 1
typedef Name TokenValue;
#else
struct TokenValue : BOOST_WAVE_STRINGTYPE
{
	typedef BOOST_WAVE_STRINGTYPE Base;
	TokenValue()
	{
	}
	explicit TokenValue(const char* value)
		: Base(value)
	{
	}
	explicit TokenValue(const Base& value)
		: Base(value)
	{
	}
	bool empty() const
	{
		return Base::empty();
	}
	const char* c_str() const
	{
		return Base::c_str();
	}
};
#endif

const TokenValue TOKENVALUE_NULL;

typedef BOOST_WAVE_STRINGTYPE Filename;

struct FilePosition
{
	Filename file;
	unsigned int line;
	unsigned int column;
};

inline bool operator==(const FilePosition& left, const FilePosition& right)
{
	return left.file == right.file
		&& left.line == right.line
		&& left.column == right.column;
}

struct IncludeEvents
{
	unsigned short push;
	unsigned short pop;
	IncludeEvents()
		: push(0), pop(0)
	{
	}
};

struct Path
{
	Name relative;
	Name absolute;

	Path()
	{
	}
	Path(Name relative, Name absolute)
		: relative(relative), absolute(absolute)
	{
	}
	const char* c_str() const
	{
		return absolute.c_str();
	}
};


inline bool operator==(const Path& left, const Path& right)
{
	return left.absolute == right.absolute;
}

inline bool operator!=(const Path& left, const Path& right)
{
	return left.absolute != right.absolute;
}

inline bool operator<(const Path& left, const Path& right)
{
	return left.absolute < right.absolute;
}

#define PATH_NULL Path(Name(0), Name(0))


struct Source : Path
{
	unsigned int line;
	unsigned int column;

	Source()
	{
	}
	Source(Path path, unsigned int line, unsigned int column)
		: Path(path), line(line), column(column)
	{
	}
};

inline bool operator==(const Source& left, const Source& right)
{
	return left.absolute == right.absolute
		&& left.line == right.line
		&& left.column == right.column;
}

inline bool operator!=(const Source& left, const Source& right)
{
	return !operator==(left, right);
}

#endif


