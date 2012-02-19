
#ifndef INCLUDED_CPPPARSE_UTIL_H
#define INCLUDED_CPPPARSE_UTIL_H


#include <string.h>
#include <vector>


inline const char* findFilename(const char* path)
{
	const char* result = strrchr(path, '\\');
	if(result == 0)
	{
		result = strrchr(path, '/');
		if(result == 0)
		{
			return path;
		}
	}
	return ++result;
}

inline const char* findFilenameSafe(const char* path)
{
	if(path != 0)
	{
		return findFilename(path);
	}
	return "<error>";
}

inline const char* findExtension(const char* path)
{
	const char* result = strrchr(path, '.');
	return result == 0 ? path + strlen(path) : result;
}

inline bool string_equal(const char* string, const char* other)
{
	return strcmp(string, other) == 0;
}

inline bool string_equal_nocase(const char* string, const char* other)
{
#ifdef WIN32
	return _stricmp(string, other) == 0;
#else
	return strcasecmp(string, other) == 0;
#endif
}

inline bool string_equal_prefix(const char* left, const char* right)
{
	for(; *left != '\0' && *right != '\0'; ++left, ++right)
	{
		if(*left != *right)
		{
			return false;
		}
	}
	return true;
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

inline StringRange makeRange(const std::vector<char>& buffer)
{
	return buffer.empty() ? StringRange(0, 0) : StringRange(&(*buffer.begin()), &(*buffer.begin()) + buffer.size());
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

template<typename T, size_t CAPACITY>
struct Stack
{
	Stack()
		: depth(0)
	{
	}
	typedef T value_type;
	typedef value_type* iterator;
	typedef const value_type* const_iterator;

	value_type stack[CAPACITY];
	size_t depth;

	bool empty() const
	{
		return depth == 0;
	}
	value_type& top()
	{
		return stack[depth - 1];
	}
	const value_type& top() const
	{
		return stack[depth - 1];
	}
	void push(value_type v)
	{
		stack[depth++] = v;
	}
	void pop()
	{
		--depth;
	}
	const_iterator begin() const
	{
		return stack;
	}
	const_iterator end() const
	{
		return stack + depth;
	}
};

typedef Stack<const char*, 1024> StringStack;

#endif


