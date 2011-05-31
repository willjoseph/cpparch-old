


#include <iostream>
#include <fstream>
#include "syelog.h"
#include <algorithm>


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

inline char* string_write(char* out, const char* s)
{
	return std::copy(s, s + strlen(s), out);
}

inline const char* string_findlast(const char* s, char c)
{
	return strrchr(s, c);
}

inline const wchar_t* string_findlast(const wchar_t* s, wchar_t c)
{
	return wcsrchr(s, c);
}

inline bool string_equal(const char* s, const char* other)
{
	return strcmp(s, other) == 0;
}

inline bool string_equal(const wchar_t* s, const wchar_t* other)
{
	return wcscmp(s, other) == 0;
}

template<typename Char>
inline const Char* findFilename(const Char* path)
{
	const Char* filename = string_findlast(path, '\\');
	if(filename == 0)
	{
		return path;
	}
	return filename + 1;
}

// "my image.exe" param1 "param 2"

inline std::ostream& operator<<(std::ostream& o, const wchar_t* s)
{
	for(; *s != '\0'; ++s)
	{
		o << char(*s);
	}
	return o;
}

// Parameters are always separated by a space or tab (multiple spaces/tabs OK)
// If the parameter does not contain any spaces, tabs, or double quotes, then all the characters in the parameter are accepted as is (there is no need to enclose the parameter in double quotes).
// Enclose spaces and tabs in a double quoted part
// A double quoted part can be anywhere within a parameter
// 2n backslashes followed by a " produce n backslashes + start/end double quoted part
// 2n+1 backslashes followed by a " produce n backslashes + a literal quotation mark
// n backslashes not followed by a quotation mark produce n backslashes
// If a closing " is followed immediately by another ", the 2nd " is accepted literally and added to the parameter 

template<typename Char>
inline const bool IsWhitespace(Char c)
{
	return c == Char(' ')
		|| c == Char('\t');
}

template<typename Char>
inline const bool IsTerminal(Char c)
{
	return c == Char('\0')
		|| c == Char('\n');
}

template<typename Char>
inline const Char* SkipWhitespace(const Char* p)
{
	for(; *p != Char('\0'); ++p)
	{
		if(!IsWhitespace(*p))
		{
			break;
		}
	}
	return p;
}


template<typename Char>
inline const Char* ParseEscaped(const Char* s, char*& out)
{
	const Char* p = s;
	for(; !IsTerminal(*p); ++p)
	{
		if(*p != '\\')
		{
			break;
		}
	}
	size_t n = p - s;
	if(*p == '\"') // escaped '"'
	{
		for(size_t i = 0; i != (n >> 1); ++i)
		{
			*out++ = '\\';
		}
		if(n & 1)
		{
			*out++ = char(*p++); // '"'
		}
	}
	else
	{
		for(size_t i = 0; i != n; ++i)
		{
			*out++ = '\\';
		}
	}
	return p;
}

template<typename Char>
inline const Char* ParseQuoted(const Char* p, char*& out)
{
	++p; // '"'
	for(; !IsTerminal(*p);)
	{
		if(*p == Char('\"'))
		{
			++p; // '"'
			break;
		}
		else if(*p == Char('\\'))
		{
			p = ParseEscaped(p, out);
		}
		else
		{
			*out++ = char(*p++);
		}
	}
	return p;
}

template<typename Char>
inline const Char* ParseArgument(const Char* p, char*& out)
{
	for(; !IsTerminal(*p);)
	{
		if(IsWhitespace(*p))
		{
			break;
		}
		if(*p == Char('\"'))
		{
			p = ParseQuoted(p, out);
		}
		else if(*p == Char('\\'))
		{
			p = ParseEscaped(p, out);
		}
		else
		{
			*out++ = char(*p++);
		}
	}
	return p;
}

typedef const char* Argument;
struct CommandLineArguments
{
	char buffer[32768];
	char* out;
	Argument elements[1024];
	size_t count;

	CommandLineArguments()
		: out(buffer), count(0)
	{
	}
};

// Where a command-line token begins with an @ sign (and is not, exceptionally,
// an argument for some earlier option), the remainder of the token names a
// command file, which the product documentation also sometimes refers to as a
// response file. The purpose of a command file is that its text is to provide
// more command-line tokens. In effect, a command-line token that names a
// command file is a command-line macro, expanding in place to whatever
// command-line tokens are found in the named file.

// The ordinary processing of command files is recursive. It is an error (D2035)
// if the recursion goes too far. The present implementation allows the recursion
// depth to reach 13 (counting the top-level command file as 1). It is an error
// (D2022) if the command file cannot be opened with read access (in text mode).
// The filename used is exactly as given in the command-line token, beginning
// immediately after the @ sign, white space included. The command file is parsed
// line by line, for as long as lines can be read. A read error for the file
// becomes an error (D2034) for CL. Parsing line by line has as an implication
// that although a line may contain multiple tokens, no token or option can carry
// beyond the line it starts on.

template<typename Char>
inline const Char* ParseCommandLine(const Char* p, CommandLineArguments& arguments)
{
	for(; !IsTerminal(*p);)
	{
		p = SkipWhitespace(p);
		arguments.elements[arguments.count++] = arguments.out;
		p = ParseArgument(p, arguments.out);
		*arguments.out++ = '\0';
	}
	return p;
}

inline bool IsCompilerCommandLineSwitch(char c)
{
	return c == '/' || c == '-';
}

inline void Report(const char* caption, const char* text)
{
#if 0
	Syelog(SYELOG_SEVERITY_INFORMATION, "%s: %s\n", caption, text);
#else
	std::cout << caption << ": " << text << std::endl;
#endif
}

inline void Report(const char* caption, const wchar_t* text)
{
#if 0
	Syelog(SYELOG_SEVERITY_INFORMATION, "%s: %ls\n", caption, text);
#else
	std::cout << caption << ": " << text << std::endl;
#endif
}

#include <vector>

template<typename Char>
struct NextLine
{
	std::vector<Char> line;
	NextLine(std::ifstream& file, bool wide)
	{
		line.reserve(1024);
		int prev = 0;
		for(; file.good();)
		{
			int c = file.get();
			if(file.eof())
			{
				break;
			}
			if(wide)
			{
				c |= (file.get() << 8);
			}

			if(prev == '\r'
				&& c == '\n')
			{
				break;
			}
			prev = c;

			if(c != '\r')
			{
				line.push_back(Char(c));
			}
		}
		line.push_back(Char('\0'));
	}
	const Char* c_str() const
	{
		return &(*line.begin());
	}
};

#define ARRAY_COUNT(array) (sizeof(array) / sizeof(*array))
#define ARRAY_END(array) ((array) + ARRAY_COUNT(array))

typedef const char* Option;

const Option CL_OPTIONS_ARGREQUIRED[] = {
	"AI",
	"B1_5",
	"B1",
	"B2",
	"Bl",
	"Bp1",
	"Bp2",
	"Bpl",
	"Bpx",
	"Bx",
	"bC",
	"bS",
	"d1_5",
	"d1",
	"d2",
	"D",
	"FI",
	"FU",
	"F",
	"f",
	"H",
	"I",
	"il",
	"MP",
	"nl",
	"o",
	"Tc",
	"To",
	"Tp",
	"pc",
	"V",
	"W",
	"w1",
	"w2",
	"w3",
	"w4",
	"wd",
	"we",
	"wo",
};

const Option CL_OPTIONS_ARG[] = {
	"AI",
	"B1_5",
	"B1",
	"B2",
	"BK",
	"Bk",
	"Bl",
	"Bp1",
	"Bp2",
	"Bpl",
	"Bpx",
	"Bx",
	"bC",
	"bS",
	"D",
	"d1_5",
	"d1",
	"d2",
	"FA",
	"Fa",
	"Fb",
	"Fc",
	"Fd",
	"Fe",
	"FI",
	"Fl",
	"Fm",
	"Fo",
	"FP",
	"Fp",
	"FR",
	"Fr",
	"Fs",
	"FU",
	"F",
	"f",
	"GE",
	"Gp",
	"Gs",
	"Gt",
	"H",
	"I",
	"il",
	"MP",
	"nl",
	"OV",
	"o",
	"Tc",
	"To",
	"Tp",
	"pc",
	"V",
	"vd",
	"Wp",
	"W",
	"w1",
	"w2",
	"w3",
	"w4",
	"wd",
	"we",
	"wo",
	"Yc",
	"Yl",
	"Yu",
	"YX",
	"ZB",
	"Zm",
	"Zp",
	"ZX",
};


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

struct CompilerOptionEqual
{
	Option option;
	CompilerOptionEqual(Option option)
		: option(option)
	{
	}
	bool operator()(Option option) const
	{
		return string_equal_prefix(this->option, option);
	}
};

const Argument* ParseCompilerOption(const Argument* first, const Argument* last, std::ofstream& out)
{
	Argument a = *first;
	if(IsCompilerCommandLineSwitch(*a))
	{
		const char* option = a + 1;

		if(string_equal(option, "link"))
		{
			// skip this and all remaining options
			return last;
		}

		const char* value = "";
		const Option* found = std::find_if(CL_OPTIONS_ARG, ARRAY_END(CL_OPTIONS_ARG), CompilerOptionEqual(option));
		if(found != ARRAY_END(CL_OPTIONS_ARG))
		{
			value = option + strlen(*found);
			option = *found;
			if(*value == '\0')
			{
				++first;
				// the value for this option is given by the next argument
				value = first == last ? "" : *first;
			}
		}

		out << "/" << option << " " << value << std::endl;
	}
	else
	{
		out << a << std::endl;
	}

	return first + 1;
}


template<typename Char>
inline void ParseCompilerCommandLine(const Char* commandline, std::ofstream& out)
{
	CommandLineArguments arguments;
	ParseCommandLine(commandline, arguments);

	for(const Argument* i = arguments.elements; i != arguments.elements + arguments.count;)
	{
		Argument a = *i;
		if(*a == '@')
		{
			const char* path = a + 1;

			char signature[2];
			std::ifstream file(path, std::ios::binary);
			if(file.is_open())
			{
				file.read(signature, 2);

				bool wide = signature[0] == -1 && signature[1] == -2;
				if(!wide)
				{
					file.putback(signature[1]);
					file.putback(signature[0]);
				}
			
				while(file.good())
				{
					if(wide)
					{
						ParseCompilerCommandLine(NextLine<wchar_t>(file, wide).c_str(), out);
					}
					else
					{
						ParseCompilerCommandLine(NextLine<char>(file, wide).c_str(), out);
					}
				}
				++i;
				continue;
			}
			else
			{
				Report("FAILED", path);
			}
		}

		Report("ARGUMENT", a);

		i = ParseCompilerOption(i, arguments.elements + arguments.count, out);
	}
}
