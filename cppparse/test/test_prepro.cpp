
extern "C"
{
	typedef unsigned int uintptr_t;
	typedef char*va_list;
}
extern "C"
{
	typedef unsigned int size_t;
	typedef size_t rsize_t;
	typedef int intptr_t;
	typedef int ptrdiff_t;
	typedef unsigned short wint_t;
	typedef unsigned short wctype_t;
	typedef int errcode;
	typedef int errno_t;
	typedef long __time32_t;
	typedef __int64 __time64_t;
	typedef __time64_t time_t;
	struct threadlocaleinfostruct;
	struct threadmbcinfostruct;
	typedef struct threadlocaleinfostruct*pthreadlocinfo;
	typedef struct threadmbcinfostruct*pthreadmbcinfo;
	struct __lc_time_data;
	typedef struct localeinfo_struct
	{
		pthreadlocinfo locinfo;
		pthreadmbcinfo mbcinfo;
	}
	_locale_tstruct, *_locale_t;
	typedef struct tagLC_ID
	{
		unsigned short wLanguage;
		unsigned short wCountry;
		unsigned short wCodePage;
	}
	LC_ID, *LPLC_ID;
	typedef struct threadlocaleinfostruct
	{
		int refcount;
		unsigned int lc_codepage;
		unsigned int lc_collate_cp;
		unsigned long lc_handle[6];
		LC_ID lc_id[6];
		struct
		{
			char*locale;
			wchar_t*wlocale;
			int*refcount;
			int*wrefcount;
		}
		lc_category[6];
		int lc_clike;
		int mb_cur_max;
		int*lconv_intl_refcount;
		int*lconv_num_refcount;
		int*lconv_mon_refcount;
		struct lconv*lconv;
		int*ctype1_refcount;
		unsigned short*ctype1;
		const unsigned short*pctype;
		const unsigned char*pclmap;
		const unsigned char*pcumap;
		struct __lc_time_data*lc_time_curr;
	}
	threadlocinfo;
}
extern "C" void _invalid_parameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t);
namespace std
{
	typedef bool _Bool;
}
typedef __int64 _Longlong;
typedef unsigned __int64 _ULonglong;
namespace std
{
	class _Lockit
	{
	public:
		explicit _Lockit();
		explicit _Lockit(int);
		~_Lockit();
		static void _Lockit_ctor(int);
		static void _Lockit_dtor(int);
	private:
		static void _Lockit_ctor(_Lockit*);
		static void _Lockit_ctor(_Lockit*, int);
		static void _Lockit_dtor(_Lockit*);
		_Lockit(const _Lockit&);
		_Lockit&operator=(const _Lockit&);
		int _Locktype;
	};
	class _Mutex
	{
	public:
		_Mutex();
		~_Mutex();
		void _Lock();
		void _Unlock();
	private:
		static void _Mutex_ctor(_Mutex*);
		static void _Mutex_dtor(_Mutex*);
		static void _Mutex_Lock(_Mutex*);
		static void _Mutex_Unlock(_Mutex*);
		_Mutex(const _Mutex&);
		_Mutex&operator=(const _Mutex&);
		void*_Mtx;
	};
	class _Init_locks
	{
	public:
		_Init_locks();
		~_Init_locks();
	private:
		static void _Init_locks_ctor(_Init_locks*);
		static void _Init_locks_dtor(_Init_locks*);
	};
}
void _Atexit(void(*)(void));
typedef int _Mbstatet;
extern "C"
{
	struct _iobuf
	{
		char*_ptr;
		int _cnt;
		char*_base;
		int _flag;
		int _file;
		int _charbuf;
		int _bufsiz;
		char*_tmpfname;
	};
	typedef struct _iobuf FILE;
	FILE*__iob_func(void);
	typedef long long fpos_t;
	int _filbuf(FILE*_File);
	int _flsbuf(int _Ch, FILE*_File);
	FILE*_fsopen(const char*_Filename, const char*_Mode, int _ShFlag);
	void clearerr(FILE*_File);
	errno_t clearerr_s(FILE*_File);
	int fclose(FILE*_File);
	int _fcloseall(void);
	FILE*_fdopen(int _FileHandle, const char*_Mode);
	int feof(FILE*_File);
	int ferror(FILE*_File);
	int fflush(FILE*_File);
	int fgetc(FILE*_File);
	int _fgetchar(void);
	int fgetpos(FILE*_File, fpos_t*_Pos);
	char*fgets(char*_Buf, int _MaxCount, FILE*_File);
	int _fileno(FILE*_File);
	char*_tempnam(const char*_DirName, const char*_FilePrefix);
	int _flushall(void);
	FILE*fopen(const char*_Filename, const char*_Mode);
	errno_t fopen_s(FILE**_File, const char*_Filename, const char*_Mode);
	int fprintf(FILE*_File, const char*_Format, ...);
	int fprintf_s(FILE*_File, const char*_Format, ...);
	int fputc(int _Ch, FILE*_File);
	int _fputchar(int _Ch);
	int fputs(const char*_Str, FILE*_File);
	size_t fread(void*_DstBuf, size_t _ElementSize, size_t _Count, FILE*_File);
	size_t fread_s(void*_DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE*_File);
	FILE*freopen(const char*_Filename, const char*_Mode, FILE*_File);
	errno_t freopen_s(FILE**_File, const char*_Filename, const char*_Mode, FILE*_OldFile);
	int fscanf(FILE*_File, const char*_Format, ...);
	int _fscanf_l(FILE*_File, const char*_Format, _locale_t _Locale, ...);
	int fscanf_s(FILE*_File, const char*_Format, ...);
	int _fscanf_s_l(FILE*_File, const char*_Format, _locale_t _Locale, ...);
	int fsetpos(FILE*_File, const fpos_t*_Pos);
	int fseek(FILE*_File, long _Offset, int _Origin);
	long ftell(FILE*_File);
	int _fseeki64(FILE*_File, __int64 _Offset, int _Origin);
	__int64 _ftelli64(FILE*_File);
	size_t fwrite(const void*_Str, size_t _Size, size_t _Count, FILE*_File);
	int getc(FILE*_File);
	int getchar(void);
	int _getmaxstdio(void);
	char*gets_s(char*_Buf, rsize_t _Size);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline char*gets_s(char(&_Buffer)[_SizeFIXED])
		{
			return gets_s(_Buffer, _SizeFIXED);
		}
	}
	char*gets(char*_Buffer);
	int _getw(FILE*_File);
	void perror(const char*_ErrMsg);
	int _pclose(FILE*_File);
	FILE*_popen(const char*_Command, const char*_Mode);
	int printf(const char*_Format, ...);
	int printf_s(const char*_Format, ...);
	int putc(int _Ch, FILE*_File);
	int putchar(int _Ch);
	int puts(const char*_Str);
	int _putw(int _Word, FILE*_File);
	int remove(const char*_Filename);
	int rename(const char*_OldFilename, const char*_NewFilename);
	int _unlink(const char*_Filename);
	void rewind(FILE*_File);
	int _rmtmp(void);
	int scanf(const char*_Format, ...);
	int _scanf_l(const char*_Format, _locale_t _Locale, ...);
	int scanf_s(const char*_Format, ...);
	int _scanf_s_l(const char*_Format, _locale_t _Locale, ...);
	void setbuf(FILE*_File, char*_Buffer);
	int _setmaxstdio(int _Max);
	unsigned int _set_output_format(unsigned int _Format);
	unsigned int _get_output_format(void);
	int setvbuf(FILE*_File, char*_Buf, int _Mode, size_t _Size);
	int _snprintf_s(char*_DstBuf, size_t _DstSize, size_t _MaxCount, const char*_Format, ...);
	int sprintf_s(char*_DstBuf, size_t _DstSize, const char*_Format, ...);
	int _scprintf(const char*_Format, ...);
	int sscanf(const char*_Src, const char*_Format, ...);
	int _sscanf_l(const char*_Src, const char*_Format, _locale_t _Locale, ...);
	int sscanf_s(const char*_Src, const char*_Format, ...);
	int _sscanf_s_l(const char*_Src, const char*_Format, _locale_t _Locale, ...);
	int _snscanf(const char*_Src, size_t _MaxCount, const char*_Format, ...);
	int _snscanf_l(const char*_Src, size_t _MaxCount, const char*_Format, _locale_t _Locale, ...);
	int _snscanf_s(const char*_Src, size_t _MaxCount, const char*_Format, ...);
	int _snscanf_s_l(const char*_Src, size_t _MaxCount, const char*_Format, _locale_t _Locale, ...);
	FILE*tmpfile(void);
	errno_t tmpfile_s(FILE**_File);
	errno_t tmpnam_s(char*_Buf, rsize_t _Size);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t tmpnam_s(char(&_Buf)[_SizeFIXED])
		{
			return tmpnam_s(_Buf, _SizeFIXED);
		}
	}
	char*tmpnam(char*_Buffer);
	int ungetc(int _Ch, FILE*_File);
	int vfprintf(FILE*_File, const char*_Format, va_list _ArgList);
	int vfprintf_s(FILE*_File, const char*_Format, va_list _ArgList);
	int vprintf(const char*_Format, va_list _ArgList);
	int vprintf_s(const char*_Format, va_list _ArgList);
	int vsnprintf(char*_DstBuf, size_t _MaxCount, const char*_Format, va_list _ArgList);
	int vsnprintf_s(char*_DstBuf, size_t _DstSize, size_t _MaxCount, const char*_Format, va_list _ArgList);
	int _vsnprintf_s(char*_DstBuf, size_t _DstSize, size_t _MaxCount, const char*_Format, va_list _ArgList);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline int _vsnprintf_s(char(&_Dest)[_SizeFIXED], size_t _Size, const char*_Format, va_list _Args)
		{
			return _vsnprintf_s(_Dest, _SizeFIXED, _Size, _Format, _Args);
		}
	}
	int _snprintf(char*_Dest, size_t _Count, const char*_Format, ...);
	int _vsnprintf(char*_Dest, size_t _Count, const char*_Format, va_list _Args);
	int vsprintf_s(char*_DstBuf, size_t _Size, const char*_Format, va_list _ArgList);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline int vsprintf_s(char(&_Dest)[_SizeFIXED], const char*_Format, va_list _Args)
		{
			return vsprintf_s(_Dest, _SizeFIXED, _Format, _Args);
		}
	}
	int sprintf(char*_Dest, const char*_Format, ...);
	int vsprintf(char*_Dest, const char*_Format, va_list _Args);
	extern "C++"
	{
		;
		;
		template<size_t _SizeFIXED>
		inline int _snprintf_s(char(&_Dest)[_SizeFIXED], size_t _Size, const char*_Format, ...)
		{
			va_list _ArgList;
			(_ArgList=(va_list)(&reinterpret_cast<const char&>(_Format))+((sizeof(_Format)+sizeof(int)-1)&~(sizeof(int)-1)));
			return _vsnprintf_s(_Dest, _SizeFIXED, _Size, _Format, _ArgList);
		};
	}
	extern "C++"
	{
		;
		;
		template<size_t _SizeFIXED>
		inline int sprintf_s(char(&_Dest)[_SizeFIXED], const char*_Format, ...)
		{
			va_list _ArgList;
			(_ArgList=(va_list)(&reinterpret_cast<const char&>(_Format))+((sizeof(_Format)+sizeof(int)-1)&~(sizeof(int)-1)));
			return vsprintf_s(_Dest, _SizeFIXED, _Format, _ArgList);
		};
	}
	int _vscprintf(const char*_Format, va_list _ArgList);
	int _snprintf_c(char*_DstBuf, size_t _MaxCount, const char*_Format, ...);
	int _vsnprintf_c(char*_DstBuf, size_t _MaxCount, const char*_Format, va_list _ArgList);
	int _fprintf_p(FILE*_File, const char*_Format, ...);
	int _printf_p(const char*_Format, ...);
	int _sprintf_p(char*_Dst, size_t _MaxCount, const char*_Format, ...);
	int _vfprintf_p(FILE*_File, const char*_Format, va_list _ArgList);
	int _vprintf_p(const char*_Format, va_list _ArgList);
	int _vsprintf_p(char*_Dst, size_t _MaxCount, const char*_Format, va_list _ArgList);
	int _scprintf_p(const char*_Format, ...);
	int _vscprintf_p(const char*_Format, va_list _ArgList);
	int _set_printf_count_output(int _Value);
	int _get_printf_count_output();
	int _printf_l(const char*_Format, _locale_t _Locale, ...);
	int _printf_p_l(const char*_Format, _locale_t _Locale, ...);
	int _printf_s_l(const char*_Format, _locale_t _Locale, ...);
	int _vprintf_l(const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _vprintf_p_l(const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _vprintf_s_l(const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _fprintf_l(FILE*_File, const char*_Format, _locale_t _Locale, ...);
	int _fprintf_p_l(FILE*_File, const char*_Format, _locale_t _Locale, ...);
	int _fprintf_s_l(FILE*_File, const char*_Format, _locale_t _Locale, ...);
	int _vfprintf_l(FILE*_File, const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _vfprintf_p_l(FILE*_File, const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _vfprintf_s_l(FILE*_File, const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _sprintf_l(char*_DstBuf, const char*_Format, _locale_t _Locale, ...);
	int _sprintf_p_l(char*_DstBuf, size_t _MaxCount, const char*_Format, _locale_t _Locale, ...);
	int _sprintf_s_l(char*_DstBuf, size_t _DstSize, const char*_Format, _locale_t _Locale, ...);
	int _vsprintf_l(char*_DstBuf, const char*_Format, _locale_t, va_list _ArgList);
	int _vsprintf_p_l(char*_DstBuf, size_t _MaxCount, const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _vsprintf_s_l(char*_DstBuf, size_t _DstSize, const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _scprintf_l(const char*_Format, _locale_t _Locale, ...);
	int _scprintf_p_l(const char*_Format, _locale_t _Locale, ...);
	int _vscprintf_l(const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _vscprintf_p_l(const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _snprintf_l(char*_DstBuf, size_t _MaxCount, const char*_Format, _locale_t _Locale, ...);
	int _snprintf_c_l(char*_DstBuf, size_t _MaxCount, const char*_Format, _locale_t _Locale, ...);
	int _snprintf_s_l(char*_DstBuf, size_t _DstSize, size_t _MaxCount, const char*_Format, _locale_t _Locale, ...);
	int _vsnprintf_l(char*_DstBuf, size_t _MaxCount, const char*_Format, _locale_t _Locale, va_list _ArgList);
	int _vsnprintf_c_l(char*_DstBuf, size_t _MaxCount, const char*, _locale_t _Locale, va_list _ArgList);
	int _vsnprintf_s_l(char*_DstBuf, size_t _DstSize, size_t _MaxCount, const char*_Format, _locale_t _Locale, va_list _ArgList);
	FILE*_wfsopen(const wchar_t*_Filename, const wchar_t*_Mode, int _ShFlag);
	wint_t fgetwc(FILE*_File);
	wint_t _fgetwchar(void);
	wint_t fputwc(wchar_t _Ch, FILE*_File);
	wint_t _fputwchar(wchar_t _Ch);
	wint_t getwc(FILE*_File);
	wint_t getwchar(void);
	wint_t putwc(wchar_t _Ch, FILE*_File);
	wint_t putwchar(wchar_t _Ch);
	wint_t ungetwc(wint_t _Ch, FILE*_File);
	wchar_t*fgetws(wchar_t*_Dst, int _SizeInWords, FILE*_File);
	int fputws(const wchar_t*_Str, FILE*_File);
	wchar_t*_getws_s(wchar_t*_Str, size_t _SizeInWords);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline wchar_t*_getws_s(wchar_t(&_String)[_SizeFIXED])
		{
			return _getws_s(_String, _SizeFIXED);
		}
	}
	wchar_t*_getws(wchar_t*_String);
	int _putws(const wchar_t*_Str);
	int fwprintf(FILE*_File, const wchar_t*_Format, ...);
	int fwprintf_s(FILE*_File, const wchar_t*_Format, ...);
	int wprintf(const wchar_t*_Format, ...);
	int wprintf_s(const wchar_t*_Format, ...);
	int _scwprintf(const wchar_t*_Format, ...);
	int vfwprintf(FILE*_File, const wchar_t*_Format, va_list _ArgList);
	int vfwprintf_s(FILE*_File, const wchar_t*_Format, va_list _ArgList);
	int vwprintf(const wchar_t*_Format, va_list _ArgList);
	int vwprintf_s(const wchar_t*_Format, va_list _ArgList);
	int swprintf_s(wchar_t*_Dst, size_t _SizeInWords, const wchar_t*_Format, ...);
	int vswprintf_s(wchar_t*_Dst, size_t _SizeInWords, const wchar_t*_Format, va_list _ArgList);
	extern "C++"
	{
		;
		;
		template<size_t _SizeFIXED>
		inline int swprintf_s(wchar_t(&_Dest)[_SizeFIXED], const wchar_t*_Format, ...)
		{
			va_list _ArgList;
			(_ArgList=(va_list)(&reinterpret_cast<const char&>(_Format))+((sizeof(_Format)+sizeof(int)-1)&~(sizeof(int)-1)));
			return vswprintf_s(_Dest, _SizeFIXED, _Format, _ArgList);
		};
	}
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline int vswprintf_s(wchar_t(&_Dest)[_SizeFIXED], const wchar_t*_Format, va_list _Args)
		{
			return vswprintf_s(_Dest, _SizeFIXED, _Format, _Args);
		}
	}
	int _swprintf_c(wchar_t*_DstBuf, size_t _SizeInWords, const wchar_t*_Format, ...);
	int _vswprintf_c(wchar_t*_DstBuf, size_t _SizeInWords, const wchar_t*_Format, va_list _ArgList);
	int _snwprintf_s(wchar_t*_DstBuf, size_t _DstSizeInWords, size_t _MaxCount, const wchar_t*_Format, ...);
	int _vsnwprintf_s(wchar_t*_DstBuf, size_t _DstSizeInWords, size_t _MaxCount, const wchar_t*_Format, va_list _ArgList);
	extern "C++"
	{
		;
		;
		template<size_t _SizeFIXED>
		inline int _snwprintf_s(wchar_t(&_Dest)[_SizeFIXED], size_t _Count, const wchar_t*_Format, ...)
		{
			va_list _ArgList;
			(_ArgList=(va_list)(&reinterpret_cast<const char&>(_Format))+((sizeof(_Format)+sizeof(int)-1)&~(sizeof(int)-1)));
			return _vsnwprintf_s(_Dest, _SizeFIXED, _Count, _Format, _ArgList);
		};
	}
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline int _vsnwprintf_s(wchar_t(&_Dest)[_SizeFIXED], size_t _Count, const wchar_t*_Format, va_list _Args)
		{
			return _vsnwprintf_s(_Dest, _SizeFIXED, _Count, _Format, _Args);
		}
	}
	int _snwprintf(wchar_t*_Dest, size_t _Count, const wchar_t*_Format, ...);
	int _vsnwprintf(wchar_t*_Dest, size_t _Count, const wchar_t*_Format, va_list _Args);
	int _fwprintf_p(FILE*_File, const wchar_t*_Format, ...);
	int _wprintf_p(const wchar_t*_Format, ...);
	int _vfwprintf_p(FILE*_File, const wchar_t*_Format, va_list _ArgList);
	int _vwprintf_p(const wchar_t*_Format, va_list _ArgList);
	int _swprintf_p(wchar_t*_DstBuf, size_t _MaxCount, const wchar_t*_Format, ...);
	int _vswprintf_p(wchar_t*_DstBuf, size_t _MaxCount, const wchar_t*_Format, va_list _ArgList);
	int _scwprintf_p(const wchar_t*_Format, ...);
	int _vscwprintf_p(const wchar_t*_Format, va_list _ArgList);
	int _wprintf_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _wprintf_p_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _wprintf_s_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _vwprintf_l(const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _vwprintf_p_l(const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _vwprintf_s_l(const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _fwprintf_l(FILE*_File, const wchar_t*_Format, _locale_t _Locale, ...);
	int _fwprintf_p_l(FILE*_File, const wchar_t*_Format, _locale_t _Locale, ...);
	int _fwprintf_s_l(FILE*_File, const wchar_t*_Format, _locale_t _Locale, ...);
	int _vfwprintf_l(FILE*_File, const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _vfwprintf_p_l(FILE*_File, const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _vfwprintf_s_l(FILE*_File, const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _swprintf_c_l(wchar_t*_DstBuf, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, ...);
	int _swprintf_p_l(wchar_t*_DstBuf, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, ...);
	int _swprintf_s_l(wchar_t*_DstBuf, size_t _DstSize, const wchar_t*_Format, _locale_t _Locale, ...);
	int _vswprintf_c_l(wchar_t*_DstBuf, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _vswprintf_p_l(wchar_t*_DstBuf, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _vswprintf_s_l(wchar_t*_DstBuf, size_t _DstSize, const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _scwprintf_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _scwprintf_p_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _vscwprintf_p_l(const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _snwprintf_l(wchar_t*_DstBuf, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, ...);
	int _snwprintf_s_l(wchar_t*_DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, ...);
	int _vsnwprintf_l(wchar_t*_DstBuf, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _vsnwprintf_s_l(wchar_t*_DstBuf, size_t _DstSize, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _swprintf(wchar_t*_Dest, const wchar_t*_Format, ...);
	int _vswprintf(wchar_t*_Dest, const wchar_t*_Format, va_list _Args);
	int __swprintf_l(wchar_t*_Dest, const wchar_t*_Format, _locale_t _Plocinfo, ...);
	int __vswprintf_l(wchar_t*_Dest, const wchar_t*_Format, _locale_t _Plocinfo, va_list _Args);
	static int swprintf(wchar_t*_String, size_t _Count, const wchar_t*_Format, ...)
	{
		va_list _Arglist;
		int _Ret;
		(_Arglist=(va_list)(&reinterpret_cast<const char&>(_Format))+((sizeof(_Format)+sizeof(int)-1)&~(sizeof(int)-1)));
		_Ret=_vswprintf_c_l(_String, _Count, _Format, 0, _Arglist);
		(_Arglist=(va_list)0);
		return _Ret;
	}
	static int vswprintf(wchar_t*_String, size_t _Count, const wchar_t*_Format, va_list _Ap)
	{
		return _vswprintf_c_l(_String, _Count, _Format, 0, _Ap);
	}
	static int _swprintf_l(wchar_t*_String, size_t _Count, const wchar_t*_Format, _locale_t _Plocinfo, ...)
	{
		va_list _Arglist;
		int _Ret;
		(_Arglist=(va_list)(&reinterpret_cast<const char&>(_Plocinfo))+((sizeof(_Plocinfo)+sizeof(int)-1)&~(sizeof(int)-1)));
		_Ret=_vswprintf_c_l(_String, _Count, _Format, _Plocinfo, _Arglist);
		(_Arglist=(va_list)0);
		return _Ret;
	}
	static int _vswprintf_l(wchar_t*_String, size_t _Count, const wchar_t*_Format, _locale_t _Plocinfo, va_list _Ap)
	{
		return _vswprintf_c_l(_String, _Count, _Format, _Plocinfo, _Ap);
	}
	extern "C++" int swprintf(wchar_t*_String, const wchar_t*_Format, ...)
	{
		va_list _Arglist;
		(_Arglist=(va_list)(&reinterpret_cast<const char&>(_Format))+((sizeof(_Format)+sizeof(int)-1)&~(sizeof(int)-1)));
		int _Ret=_vswprintf(_String, _Format, _Arglist);
		(_Arglist=(va_list)0);
		return _Ret;
	}
	extern "C++" int vswprintf(wchar_t*_String, const wchar_t*_Format, va_list _Ap)
	{
		return _vswprintf(_String, _Format, _Ap);
	}
	extern "C++" int _swprintf_l(wchar_t*_String, const wchar_t*_Format, _locale_t _Plocinfo, ...)
	{
		va_list _Arglist;
		(_Arglist=(va_list)(&reinterpret_cast<const char&>(_Plocinfo))+((sizeof(_Plocinfo)+sizeof(int)-1)&~(sizeof(int)-1)));
		int _Ret=__vswprintf_l(_String, _Format, _Plocinfo, _Arglist);
		(_Arglist=(va_list)0);
		return _Ret;
	}
	extern "C++" int _vswprintf_l(wchar_t*_String, const wchar_t*_Format, _locale_t _Plocinfo, va_list _Ap)
	{
		return __vswprintf_l(_String, _Format, _Plocinfo, _Ap);
	}
	wchar_t*_wtempnam(const wchar_t*_Directory, const wchar_t*_FilePrefix);
	int _vscwprintf(const wchar_t*_Format, va_list _ArgList);
	int _vscwprintf_l(const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int fwscanf(FILE*_File, const wchar_t*_Format, ...);
	int _fwscanf_l(FILE*_File, const wchar_t*_Format, _locale_t _Locale, ...);
	int fwscanf_s(FILE*_File, const wchar_t*_Format, ...);
	int _fwscanf_s_l(FILE*_File, const wchar_t*_Format, _locale_t _Locale, ...);
	int swscanf(const wchar_t*_Src, const wchar_t*_Format, ...);
	int _swscanf_l(const wchar_t*_Src, const wchar_t*_Format, _locale_t _Locale, ...);
	int swscanf_s(const wchar_t*_Src, const wchar_t*_Format, ...);
	int _swscanf_s_l(const wchar_t*_Src, const wchar_t*_Format, _locale_t _Locale, ...);
	int _snwscanf(const wchar_t*_Src, size_t _MaxCount, const wchar_t*_Format, ...);
	int _snwscanf_l(const wchar_t*_Src, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, ...);
	int _snwscanf_s(const wchar_t*_Src, size_t _MaxCount, const wchar_t*_Format, ...);
	int _snwscanf_s_l(const wchar_t*_Src, size_t _MaxCount, const wchar_t*_Format, _locale_t _Locale, ...);
	int wscanf(const wchar_t*_Format, ...);
	int _wscanf_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int wscanf_s(const wchar_t*_Format, ...);
	int _wscanf_s_l(const wchar_t*_Format, _locale_t _Locale, ...);
	FILE*_wfdopen(int _FileHandle, const wchar_t*_Mode);
	FILE*_wfopen(const wchar_t*_Filename, const wchar_t*_Mode);
	errno_t _wfopen_s(FILE**_File, const wchar_t*_Filename, const wchar_t*_Mode);
	FILE*_wfreopen(const wchar_t*_Filename, const wchar_t*_Mode, FILE*_OldFile);
	errno_t _wfreopen_s(FILE**_File, const wchar_t*_Filename, const wchar_t*_Mode, FILE*_OldFile);
	void _wperror(const wchar_t*_ErrMsg);
	FILE*_wpopen(const wchar_t*_Command, const wchar_t*_Mode);
	int _wremove(const wchar_t*_Filename);
	errno_t _wtmpnam_s(wchar_t*_DstBuf, size_t _SizeInWords);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wtmpnam_s(wchar_t(&_Buffer)[_SizeFIXED])
		{
			return _wtmpnam_s(_Buffer, _SizeFIXED);
		}
	}
	wchar_t*_wtmpnam(wchar_t*_Buffer);
	wint_t _fgetwc_nolock(FILE*_File);
	wint_t _fputwc_nolock(wchar_t _Ch, FILE*_File);
	wint_t _ungetwc_nolock(wint_t _Ch, FILE*_File);
	inline wint_t getwchar()
	{
		return (fgetwc((&__iob_func()[0])));
	}
	inline wint_t putwchar(wchar_t _C)
	{
		return (fputwc(_C, (&__iob_func()[1])));
	}
	int _fclose_nolock(FILE*_File);
	int _fflush_nolock(FILE*_File);
	size_t _fread_nolock(void*_DstBuf, size_t _ElementSize, size_t _Count, FILE*_File);
	size_t _fread_nolock_s(void*_DstBuf, size_t _DstSize, size_t _ElementSize, size_t _Count, FILE*_File);
	int _fseek_nolock(FILE*_File, long _Offset, int _Origin);
	long _ftell_nolock(FILE*_File);
	int _fseeki64_nolock(FILE*_File, __int64 _Offset, int _Origin);
	__int64 _ftelli64_nolock(FILE*_File);
	size_t _fwrite_nolock(const void*_DstBuf, size_t _Size, size_t _Count, FILE*_File);
	int _ungetc_nolock(int _Ch, FILE*_File);
}
namespace std
{
	using::size_t;
	using::fpos_t;
	using::FILE;
	using::clearerr;
	using::fclose;
	using::feof;
	using::ferror;
	using::fflush;
	using::fgetc;
	using::fgetpos;
	using::fgets;
	using::fopen;
	using::fprintf;
	using::fputc;
	using::fputs;
	using::fread;
	using::freopen;
	using::fscanf;
	using::fseek;
	using::fsetpos;
	using::ftell;
	using::fwrite;
	using::getc;
	using::getchar;
	using::gets;
	using::perror;
	using::putc;
	using::putchar;
	using::printf;
	using::puts;
	using::remove;
	using::rename;
	using::rewind;
	using::scanf;
	using::setbuf;
	using::setvbuf;
	using::sprintf;
	using::sscanf;
	using::tmpfile;
	using::tmpnam;
	using::ungetc;
	using::vfprintf;
	using::vprintf;
	using::vsprintf;
}
extern "C"
{
	typedef int(*_onexit_t)(void);
	typedef struct _div_t
	{
		int quot;
		int rem;
	}
	div_t;
	typedef struct _ldiv_t
	{
		long quot;
		long rem;
	}
	ldiv_t;
	typedef struct
	{
		unsigned char ld[10];
	}
	_LDOUBLE;
	typedef struct
	{
		double x;
	}
	_CRT_DOUBLE;
	typedef struct
	{
		float f;
	}
	_CRT_FLOAT;
	typedef struct
	{
		long double x;
	}
	_LONGDOUBLE;
	typedef struct
	{
		unsigned char ld12[12];
	}
	_LDBL12;
	extern int __mb_cur_max;
	int ___mb_cur_max_func(void);
	int ___mb_cur_max_l_func(_locale_t);
	typedef void(*_purecall_handler)(void);
	_purecall_handler _set_purecall_handler(_purecall_handler _Handler);
	_purecall_handler _get_purecall_handler();
	extern "C++"
	{
	}
	typedef void(*_invalid_parameter_handler)(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t);
	_invalid_parameter_handler _set_invalid_parameter_handler(_invalid_parameter_handler _Handler);
	_invalid_parameter_handler _get_invalid_parameter_handler(void);
	extern "C++"
	{
	}
	extern int*_errno(void);
	errno_t _set_errno(int _Value);
	errno_t _get_errno(int*_Value);
	unsigned long*__doserrno(void);
	errno_t _set_doserrno(unsigned long _Value);
	errno_t _get_doserrno(unsigned long*_Value);
	char**__sys_errlist(void);
	int*__sys_nerr(void);
	extern int __argc;
	extern char**__argv;
	extern wchar_t**__wargv;
	extern char**_environ;
	extern wchar_t**_wenviron;
	extern char*_pgmptr;
	extern wchar_t*_wpgmptr;
	errno_t _get_pgmptr(char**_Value);
	errno_t _get_wpgmptr(wchar_t**_Value);
	extern int _fmode;
	errno_t _set_fmode(int _Mode);
	errno_t _get_fmode(int*_PMode);
	extern unsigned int _osplatform;
	extern unsigned int _osver;
	extern unsigned int _winver;
	extern unsigned int _winmajor;
	extern unsigned int _winminor;
	errno_t _get_osplatform(unsigned int*_Value);
	errno_t _get_osver(unsigned int*_Value);
	errno_t _get_winver(unsigned int*_Value);
	errno_t _get_winmajor(unsigned int*_Value);
	errno_t _get_winminor(unsigned int*_Value);
	extern "C++"
	{
		template<typename _CountofType, size_t _SizeOfArray>
		char(*__countof_helper(_CountofType(&_Array)[_SizeOfArray]))[_SizeOfArray];
	}
	void exit(int _Code);
	void _exit(int _Code);
	void abort(void);
	unsigned int _set_abort_behavior(unsigned int _Flags, unsigned int _Mask);
	int abs(int _X);
	long labs(long _X);
	__int64 _abs64(__int64);
	int atexit(void(*)(void));
	double atof(const char*_String);
	double _atof_l(const char*_String, _locale_t _Locale);
	int atoi(const char*_Str);
	int _atoi_l(const char*_Str, _locale_t _Locale);
	long atol(const char*_Str);
	long _atol_l(const char*_Str, _locale_t _Locale);
	void*bsearch_s(const void*_Key, const void*_Base, rsize_t _NumOfElements, rsize_t _SizeOfElements, int(*_PtFuncCompare)(void*, const void*, const void*), void*_Context);
	void*bsearch(const void*_Key, const void*_Base, size_t _NumOfElements, size_t _SizeOfElements, int(*_PtFuncCompare)(const void*, const void*));
	void qsort_s(void*_Base, rsize_t _NumOfElements, rsize_t _SizeOfElements, int(*_PtFuncCompare)(void*, const void*, const void*), void*_Context);
	void qsort(void*_Base, size_t _NumOfElements, size_t _SizeOfElements, int(*_PtFuncCompare)(const void*, const void*));
	unsigned short _byteswap_ushort(unsigned short _Short);
	unsigned long _byteswap_ulong(unsigned long _Long);
	unsigned __int64 _byteswap_uint64(unsigned __int64 _Int64);
	div_t div(int _Numerator, int _Denominator);
	char*getenv(const char*_VarName);
	errno_t getenv_s(size_t*_ReturnSize, char*_DstBuf, rsize_t _DstSize, const char*_VarName);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t getenv_s(size_t*_ReturnSize, char(&_Dest)[_SizeFIXED], const char*_VarName)
		{
			return getenv_s(_ReturnSize, _Dest, _SizeFIXED, _VarName);
		}
	}
	errno_t _dupenv_s(char**_PBuffer, size_t*_PBufferSizeInBytes, const char*_VarName);
	errno_t _itoa_s(int _Value, char*_DstBuf, size_t _Size, int _Radix);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _itoa_s(int _Value, char(&_Dest)[_SizeFIXED], int _Radix)
		{
			return _itoa_s(_Value, _Dest, _SizeFIXED, _Radix);
		}
	}
	char*_itoa(int _Value, char*_Dest, int _Radix);
	errno_t _i64toa_s(__int64 _Val, char*_DstBuf, size_t _Size, int _Radix);
	char*_i64toa(__int64 _Val, char*_DstBuf, int _Radix);
	errno_t _ui64toa_s(unsigned __int64 _Val, char*_DstBuf, size_t _Size, int _Radix);
	char*_ui64toa(unsigned __int64 _Val, char*_DstBuf, int _Radix);
	__int64 _atoi64(const char*_String);
	__int64 _atoi64_l(const char*_String, _locale_t _Locale);
	__int64 _strtoi64(const char*_String, char**_EndPtr, int _Radix);
	__int64 _strtoi64_l(const char*_String, char**_EndPtr, int _Radix, _locale_t _Locale);
	unsigned __int64 _strtoui64(const char*_String, char**_EndPtr, int _Radix);
	unsigned __int64 _strtoui64_l(const char*_String, char**_EndPtr, int _Radix, _locale_t _Locale);
	ldiv_t ldiv(long _Numerator, long _Denominator);
	extern "C++"
	{
		inline ldiv_t div(long _A1, long _A2)
		{
			return ldiv(_A1, _A2);
		}
	}
	errno_t _ltoa_s(long _Val, char*_DstBuf, size_t _Size, int _Radix);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _ltoa_s(long _Value, char(&_Dest)[_SizeFIXED], int _Radix)
		{
			return _ltoa_s(_Value, _Dest, _SizeFIXED, _Radix);
		}
	}
	char*_ltoa(long _Value, char*_Dest, int _Radix);
	int mblen(const char*_Ch, size_t _MaxCount);
	int _mblen_l(const char*_Ch, size_t _MaxCount, _locale_t _Locale);
	size_t _mbstrlen(const char*_Str);
	size_t _mbstrlen_l(const char*_Str, _locale_t _Locale);
	size_t _mbstrnlen(const char*_Str, size_t _MaxCount);
	size_t _mbstrnlen_l(const char*_Str, size_t _MaxCount, _locale_t _Locale);
	int mbtowc(wchar_t*_DstCh, const char*_SrcCh, size_t _SrcSizeInBytes);
	int _mbtowc_l(wchar_t*_DstCh, const char*_SrcCh, size_t _SrcSizeInBytes, _locale_t _Locale);
	errno_t mbstowcs_s(size_t*_PtNumOfCharConverted, wchar_t*_DstBuf, size_t _SizeInWords, const char*_SrcBuf, size_t _MaxCount);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t mbstowcs_s(size_t*_PtNumOfCharConverted, wchar_t(&_Dest)[_SizeFIXED], const char*_Source, size_t _MaxCount)
		{
			return mbstowcs_s(_PtNumOfCharConverted, _Dest, _SizeFIXED, _Source, _MaxCount);
		}
	}
	size_t mbstowcs(wchar_t*_Dest, const char*_Source, size_t _MaxCount);
	errno_t _mbstowcs_s_l(size_t*_PtNumOfCharConverted, wchar_t*_DstBuf, size_t _SizeInWords, const char*_SrcBuf, size_t _MaxCount, _locale_t _Locale);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _mbstowcs_s_l(size_t*_PtNumOfCharConverted, wchar_t(&_Dest)[_SizeFIXED], const char*_Source, size_t _MaxCount, _locale_t _Locale)
		{
			return _mbstowcs_s_l(_PtNumOfCharConverted, _Dest, _SizeFIXED, _Source, _MaxCount, _Locale);
		}
	}
	size_t _mbstowcs_l(wchar_t*_Dest, const char*_Source, size_t _MaxCount, _locale_t _Locale);
	int rand(void);
	int _set_error_mode(int _Mode);
	void srand(unsigned int _Seed);
	double strtod(const char*_Str, char**_EndPtr);
	double _strtod_l(const char*_Str, char**_EndPtr, _locale_t _Locale);
	long strtol(const char*_Str, char**_EndPtr, int _Radix);
	long _strtol_l(const char*_Str, char**_EndPtr, int _Radix, _locale_t _Locale);
	unsigned long strtoul(const char*_Str, char**_EndPtr, int _Radix);
	unsigned long _strtoul_l(const char*_Str, char**_EndPtr, int _Radix, _locale_t _Locale);
	int system(const char*_Command);
	errno_t _ultoa_s(unsigned long _Val, char*_DstBuf, size_t _Size, int _Radix);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _ultoa_s(unsigned long _Value, char(&_Dest)[_SizeFIXED], int _Radix)
		{
			return _ultoa_s(_Value, _Dest, _SizeFIXED, _Radix);
		}
	}
	char*_ultoa(unsigned long _Value, char*_Dest, int _Radix);
	int wctomb(char*_MbCh, wchar_t _WCh);
	int _wctomb_l(char*_MbCh, wchar_t _WCh, _locale_t _Locale);
	errno_t wctomb_s(int*_SizeConverted, char*_MbCh, rsize_t _SizeInBytes, wchar_t _WCh);
	errno_t _wctomb_s_l(int*_SizeConverted, char*_MbCh, size_t _SizeInBytes, wchar_t _WCh, _locale_t _Locale);
	errno_t wcstombs_s(size_t*_PtNumOfCharConverted, char*_Dst, size_t _DstSizeInBytes, const wchar_t*_Src, size_t _MaxCountInBytes);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t wcstombs_s(size_t*_PtNumOfCharConverted, char(&_Dest)[_SizeFIXED], const wchar_t*_Source, size_t _MaxCount)
		{
			return wcstombs_s(_PtNumOfCharConverted, _Dest, _SizeFIXED, _Source, _MaxCount);
		}
	}
	size_t wcstombs(char*_Dest, const wchar_t*_Source, size_t _MaxCount);
	errno_t _wcstombs_s_l(size_t*_PtNumOfCharConverted, char*_Dst, size_t _DstSizeInBytes, const wchar_t*_Src, size_t _MaxCountInBytes, _locale_t _Locale);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wcstombs_s_l(size_t*_PtNumOfCharConverted, char(&_Dest)[_SizeFIXED], const wchar_t*_Source, size_t _MaxCount, _locale_t _Locale)
		{
			return _wcstombs_s_l(_PtNumOfCharConverted, _Dest, _SizeFIXED, _Source, _MaxCount, _Locale);
		}
	}
	size_t _wcstombs_l(char*_Dest, const wchar_t*_Source, size_t _MaxCount, _locale_t _Locale);
	void*calloc(size_t _NumOfElements, size_t _SizeOfElements);
	void free(void*_Memory);
	void*malloc(size_t _Size);
	void*realloc(void*_Memory, size_t _NewSize);
	void*_recalloc(void*_Memory, size_t _Count, size_t _Size);
	void _aligned_free(void*_Memory);
	void*_aligned_malloc(size_t _Size, size_t _Alignment);
	void*_aligned_offset_malloc(size_t _Size, size_t _Alignment, size_t _Offset);
	void*_aligned_realloc(void*_Memory, size_t _Size, size_t _Alignment);
	void*_aligned_recalloc(void*_Memory, size_t _Count, size_t _Size, size_t _Alignment);
	void*_aligned_offset_realloc(void*_Memory, size_t _Size, size_t _Alignment, size_t _Offset);
	void*_aligned_offset_recalloc(void*_Memory, size_t _Count, size_t _Size, size_t _Alignment, size_t _Offset);
	errno_t _itow_s(int _Val, wchar_t*_DstBuf, size_t _SizeInWords, int _Radix);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _itow_s(int _Value, wchar_t(&_Dest)[_SizeFIXED], int _Radix)
		{
			return _itow_s(_Value, _Dest, _SizeFIXED, _Radix);
		}
	}
	wchar_t*_itow(int _Value, wchar_t*_Dest, int _Radix);
	errno_t _ltow_s(long _Val, wchar_t*_DstBuf, size_t _SizeInWords, int _Radix);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _ltow_s(long _Value, wchar_t(&_Dest)[_SizeFIXED], int _Radix)
		{
			return _ltow_s(_Value, _Dest, _SizeFIXED, _Radix);
		}
	}
	wchar_t*_ltow(long _Value, wchar_t*_Dest, int _Radix);
	errno_t _ultow_s(unsigned long _Val, wchar_t*_DstBuf, size_t _SizeInWords, int _Radix);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _ultow_s(unsigned long _Value, wchar_t(&_Dest)[_SizeFIXED], int _Radix)
		{
			return _ultow_s(_Value, _Dest, _SizeFIXED, _Radix);
		}
	}
	wchar_t*_ultow(unsigned long _Value, wchar_t*_Dest, int _Radix);
	double wcstod(const wchar_t*_Str, wchar_t**_EndPtr);
	double _wcstod_l(const wchar_t*_Str, wchar_t**_EndPtr, _locale_t _Locale);
	long wcstol(const wchar_t*_Str, wchar_t**_EndPtr, int _Radix);
	long _wcstol_l(const wchar_t*_Str, wchar_t**_EndPtr, int _Radix, _locale_t _Locale);
	unsigned long wcstoul(const wchar_t*_Str, wchar_t**_EndPtr, int _Radix);
	unsigned long _wcstoul_l(const wchar_t*_Str, wchar_t**_EndPtr, int _Radix, _locale_t _Locale);
	wchar_t*_wgetenv(const wchar_t*_VarName);
	errno_t _wgetenv_s(size_t*_ReturnSize, wchar_t*_DstBuf, size_t _DstSizeInWords, const wchar_t*_VarName);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wgetenv_s(size_t*_ReturnSize, wchar_t(&_Dest)[_SizeFIXED], const wchar_t*_VarName)
		{
			return _wgetenv_s(_ReturnSize, _Dest, _SizeFIXED, _VarName);
		}
	}
	errno_t _wdupenv_s(wchar_t**_Buffer, size_t*_BufferSizeInWords, const wchar_t*_VarName);
	int _wsystem(const wchar_t*_Command);
	double _wtof(const wchar_t*_Str);
	double _wtof_l(const wchar_t*_Str, _locale_t _Locale);
	int _wtoi(const wchar_t*_Str);
	int _wtoi_l(const wchar_t*_Str, _locale_t _Locale);
	long _wtol(const wchar_t*_Str);
	long _wtol_l(const wchar_t*_Str, _locale_t _Locale);
	errno_t _i64tow_s(__int64 _Val, wchar_t*_DstBuf, size_t _SizeInWords, int _Radix);
	wchar_t*_i64tow(__int64 _Val, wchar_t*_DstBuf, int _Radix);
	errno_t _ui64tow_s(unsigned __int64 _Val, wchar_t*_DstBuf, size_t _SizeInWords, int _Radix);
	wchar_t*_ui64tow(unsigned __int64 _Val, wchar_t*_DstBuf, int _Radix);
	__int64 _wtoi64(const wchar_t*_Str);
	__int64 _wtoi64_l(const wchar_t*_Str, _locale_t _Locale);
	__int64 _wcstoi64(const wchar_t*_Str, wchar_t**_EndPtr, int _Radix);
	__int64 _wcstoi64_l(const wchar_t*_Str, wchar_t**_EndPtr, int _Radix, _locale_t _Locale);
	unsigned __int64 _wcstoui64(const wchar_t*_Str, wchar_t**_EndPtr, int _Radix);
	unsigned __int64 _wcstoui64_l(const wchar_t*_Str, wchar_t**_EndPtr, int _Radix, _locale_t _Locale);
	char*_fullpath(char*_FullPath, const char*_Path, size_t _SizeInBytes);
	errno_t _ecvt_s(char*_DstBuf, size_t _Size, double _Val, int _NumOfDights, int*_PtDec, int*_PtSign);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _ecvt_s(char(&_Dest)[_SizeFIXED], double _Value, int _NumOfDigits, int*_PtDec, int*_PtSign)
		{
			return _ecvt_s(_Dest, _SizeFIXED, _Value, _NumOfDigits, _PtDec, _PtSign);
		}
	}
	char*_ecvt(double _Val, int _NumOfDigits, int*_PtDec, int*_PtSign);
	errno_t _fcvt_s(char*_DstBuf, size_t _Size, double _Val, int _NumOfDec, int*_PtDec, int*_PtSign);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _fcvt_s(char(&_Dest)[_SizeFIXED], double _Value, int _NumOfDigits, int*_PtDec, int*_PtSign)
		{
			return _fcvt_s(_Dest, _SizeFIXED, _Value, _NumOfDigits, _PtDec, _PtSign);
		}
	}
	char*_fcvt(double _Val, int _NumOfDec, int*_PtDec, int*_PtSign);
	errno_t _gcvt_s(char*_DstBuf, size_t _Size, double _Val, int _NumOfDigits);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _gcvt_s(char(&_Dest)[_SizeFIXED], double _Value, int _NumOfDigits)
		{
			return _gcvt_s(_Dest, _SizeFIXED, _Value, _NumOfDigits);
		}
	}
	char*_gcvt(double _Val, int _NumOfDigits, char*_DstBuf);
	int _atodbl(_CRT_DOUBLE*_Result, char*_Str);
	int _atoldbl(_LDOUBLE*_Result, char*_Str);
	int _atoflt(_CRT_FLOAT*_Result, char*_Str);
	int _atodbl_l(_CRT_DOUBLE*_Result, char*_Str, _locale_t _Locale);
	int _atoldbl_l(_LDOUBLE*_Result, char*_Str, _locale_t _Locale);
	int _atoflt_l(_CRT_FLOAT*_Result, char*_Str, _locale_t _Locale);
	unsigned long _lrotl(unsigned long _Val, int _Shift);
	unsigned long _lrotr(unsigned long _Val, int _Shift);
	errno_t _makepath_s(char*_PathResult, size_t _Size, const char*_Drive, const char*_Dir, const char*_Filename, const char*_Ext);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _makepath_s(char(&_Path)[_SizeFIXED], const char*_Drive, const char*_Dir, const char*_Filename, const char*_Ext)
		{
			return _makepath_s(_Path, _SizeFIXED, _Drive, _Dir, _Filename, _Ext);
		}
	}
	void _makepath(char*_Path, const char*_Drive, const char*_Dir, const char*_Filename, const char*_Ext);
	_onexit_t _onexit(_onexit_t _Func);
	int _putenv(const char*_EnvString);
	errno_t _putenv_s(const char*_Name, const char*_Value);
	unsigned int _rotl(unsigned int _Val, int _Shift);
	unsigned __int64 _rotl64(unsigned __int64 _Val, int _Shift);
	unsigned int _rotr(unsigned int _Val, int _Shift);
	unsigned __int64 _rotr64(unsigned __int64 _Val, int _Shift);
	errno_t _searchenv_s(const char*_Filename, const char*_EnvVar, char*_ResultPath, size_t _SizeInBytes);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _searchenv_s(const char*_Filename, const char*_EnvVar, char(&_ResultPath)[_SizeFIXED])
		{
			return _searchenv_s(_Filename, _EnvVar, _ResultPath, _SizeFIXED);
		}
	}
	void _searchenv(const char*_Filename, const char*_EnvVar, char*_ResultPath);
	void _splitpath(const char*_FullPath, char*_Drive, char*_Dir, char*_Filename, char*_Ext);
	errno_t _splitpath_s(const char*_FullPath, char*_Drive, size_t _DriveSize, char*_Dir, size_t _DirSize, char*_Filename, size_t _FilenameSize, char*_Ext, size_t _ExtSize);
	extern "C++"
	{
		template<size_t _DriveSize, size_t _DirSize, size_t _NameSize, size_t _ExtSize>
		inline errno_t _splitpath_s(const char*_Dest, char(&_Drive)[_DriveSize], char(&_Dir)[_DirSize], char(&_Name)[_NameSize], char(&_Ext)[_ExtSize])
		{
			return _splitpath_s(_Dest, _Drive, _DriveSize, _Dir, _DirSize, _Name, _NameSize, _Ext, _ExtSize);
		}
	}
	void _swab(char*_Buf1, char*_Buf2, int _SizeInBytes);
	wchar_t*_wfullpath(wchar_t*_FullPath, const wchar_t*_Path, size_t _SizeInWords);
	errno_t _wmakepath_s(wchar_t*_PathResult, size_t _SizeInWords, const wchar_t*_Drive, const wchar_t*_Dir, const wchar_t*_Filename, const wchar_t*_Ext);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wmakepath_s(wchar_t(&_ResultPath)[_SizeFIXED], const wchar_t*_Drive, const wchar_t*_Dir, const wchar_t*_Filename, const wchar_t*_Ext)
		{
			return _wmakepath_s(_ResultPath, _SizeFIXED, _Drive, _Dir, _Filename, _Ext);
		}
	}
	void _wmakepath(wchar_t*_ResultPath, const wchar_t*_Drive, const wchar_t*_Dir, const wchar_t*_Filename, const wchar_t*_Ext);
	int _wputenv(const wchar_t*_EnvString);
	errno_t _wputenv_s(const wchar_t*_Name, const wchar_t*_Value);
	errno_t _wsearchenv_s(const wchar_t*_Filename, const wchar_t*_EnvVar, wchar_t*_ResultPath, size_t _SizeInWords);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wsearchenv_s(const wchar_t*_Filename, const wchar_t*_EnvVar, wchar_t(&_ResultPath)[_SizeFIXED])
		{
			return _wsearchenv_s(_Filename, _EnvVar, _ResultPath, _SizeFIXED);
		}
	}
	void _wsearchenv(const wchar_t*_Filename, const wchar_t*_EnvVar, wchar_t*_ResultPath);
	void _wsplitpath(const wchar_t*_FullPath, wchar_t*_Drive, wchar_t*_Dir, wchar_t*_Filename, wchar_t*_Ext);
	errno_t _wsplitpath_s(const wchar_t*_FullPath, wchar_t*_Drive, size_t _DriveSizeInWords, wchar_t*_Dir, size_t _DirSizeInWords, wchar_t*_Filename, size_t _FilenameSizeInWords, wchar_t*_Ext, size_t _ExtSizeInWords);
	extern "C++"
	{
		template<size_t _DriveSize, size_t _DirSize, size_t _NameSize, size_t _ExtSize>
		inline errno_t _wsplitpath_s(const wchar_t*_Path, wchar_t(&_Drive)[_DriveSize], wchar_t(&_Dir)[_DirSize], wchar_t(&_Name)[_NameSize], wchar_t(&_Ext)[_ExtSize])
		{
			return _wsplitpath_s(_Path, _Drive, _DriveSize, _Dir, _DirSize, _Name, _NameSize, _Ext, _ExtSize);
		}
	}
	void _seterrormode(int _Mode);
	void _beep(unsigned _Frequency, unsigned _Duration);
	void _sleep(unsigned long _Duration);
}
namespace std
{
	using::size_t;
	using::div_t;
	using::ldiv_t;
	using::abort;
	using::abs;
	using::atexit;
	using::atof;
	using::atoi;
	using::atol;
	using::bsearch;
	using::calloc;
	using::div;
	using::exit;
	using::free;
	using::getenv;
	using::labs;
	using::ldiv;
	using::malloc;
	using::mblen;
	using::mbstowcs;
	using::mbtowc;
	using::qsort;
	using::rand;
	using::realloc;
	using::srand;
	using::strtod;
	using::strtol;
	using::strtoul;
	using::system;
	using::wcstombs;
	using::wctomb;
}
extern "C"
{
	void*_memccpy(void*_Dst, const void*_Src, int _Val, size_t _MaxCount);
	const void*memchr(const void*_Buf, int _Val, size_t _MaxCount);
	int _memicmp(const void*_Buf1, const void*_Buf2, size_t _Size);
	int _memicmp_l(const void*_Buf1, const void*_Buf2, size_t _Size, _locale_t _Locale);
	int memcmp(const void*_Buf1, const void*_Buf2, size_t _Size);
	void*memcpy(void*_Dst, const void*_Src, size_t _Size);
	errno_t memcpy_s(void*_Dst, rsize_t _DstSize, const void*_Src, rsize_t _MaxCount);
	void*memset(void*_Dst, int _Val, size_t _Size);
	char*_strset(char*_Str, int _Val);
	errno_t _strset_s(char*_Dst, size_t _DstSize, int _Value);
	errno_t strcpy_s(char*_Dst, rsize_t _DstSize, const char*_Src);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t strcpy_s(char(&_Dest)[_SizeFIXED], const char*_Source)
		{
			return strcpy_s(_Dest, _SizeFIXED, _Source);
		}
	}
	char*strcpy(char*_Dest, const char*_Source);
	errno_t strcat_s(char*_Dst, rsize_t _DstSize, const char*_Src);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t strcat_s(char(&_Dest)[_SizeFIXED], const char*_Source)
		{
			return strcat_s(_Dest, _SizeFIXED, _Source);
		}
	}
	char*strcat(char*_Dest, const char*_Source);
	int strcmp(const char*_Str1, const char*_Str2);
	size_t strlen(const char*_Str);
	size_t strnlen(const char*_Str, size_t _MaxCount);
	static size_t strnlen_s(const char*_Str, size_t _MaxCount)
	{
		return strnlen(_Str, _MaxCount);
	}
	errno_t memmove_s(void*_Dst, rsize_t _DstSize, const void*_Src, rsize_t _MaxCount);
	void*memmove(void*_Dst, const void*_Src, size_t _Size);
	char*_strdup(const char*_Src);
	const char*strchr(const char*_Str, int _Val);
	int _stricmp(const char*_Str1, const char*_Str2);
	int _strcmpi(const char*_Str1, const char*_Str2);
	int _stricmp_l(const char*_Str1, const char*_Str2, _locale_t _Locale);
	int strcoll(const char*_Str1, const char*_Str2);
	int _strcoll_l(const char*_Str1, const char*_Str2, _locale_t _Locale);
	int _stricoll(const char*_Str1, const char*_Str2);
	int _stricoll_l(const char*_Str1, const char*_Str2, _locale_t _Locale);
	int _strncoll(const char*_Str1, const char*_Str2, size_t _MaxCount);
	int _strncoll_l(const char*_Str1, const char*_Str2, size_t _MaxCount, _locale_t _Locale);
	int _strnicoll(const char*_Str1, const char*_Str2, size_t _MaxCount);
	int _strnicoll_l(const char*_Str1, const char*_Str2, size_t _MaxCount, _locale_t _Locale);
	size_t strcspn(const char*_Str, const char*_Control);
	char*_strerror(const char*_ErrMsg);
	errno_t _strerror_s(char*_Buf, size_t _SizeInBytes, const char*_ErrMsg);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _strerror_s(char(&_Buffer)[_SizeFIXED], const char*_ErrorMessage)
		{
			return _strerror_s(_Buffer, _SizeFIXED, _ErrorMessage);
		}
	}
	char*strerror(int);
	errno_t strerror_s(char*_Buf, size_t _SizeInBytes, int _ErrNum);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t strerror_s(char(&_Buffer)[_SizeFIXED], int _ErrorMessage)
		{
			return strerror_s(_Buffer, _SizeFIXED, _ErrorMessage);
		}
	}
	errno_t _strlwr_s(char*_Str, size_t _Size);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _strlwr_s(char(&_String)[_SizeFIXED])
		{
			return _strlwr_s(_String, _SizeFIXED);
		}
	}
	char*_strlwr(char*_String);
	errno_t _strlwr_s_l(char*_Str, size_t _Size, _locale_t _Locale);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _strlwr_s_l(char(&_String)[_SizeFIXED], _locale_t _Locale)
		{
			return _strlwr_s_l(_String, _SizeFIXED, _Locale);
		}
	}
	char*_strlwr_l(char*_String, _locale_t _Locale);
	errno_t strncat_s(char*_Dst, rsize_t _DstSize, const char*_Src, rsize_t _MaxCount);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t strncat_s(char(&_Dest)[_SizeFIXED], const char*_Source, size_t _Count)
		{
			return strncat_s(_Dest, _SizeFIXED, _Source, _Count);
		}
	}
	char*strncat(char*_Dest, const char*_Source, size_t _Count);
	int strncmp(const char*_Str1, const char*_Str2, size_t _MaxCount);
	int _strnicmp(const char*_Str1, const char*_Str2, size_t _MaxCount);
	int _strnicmp_l(const char*_Str1, const char*_Str2, size_t _MaxCount, _locale_t _Locale);
	errno_t strncpy_s(char*_Dst, rsize_t _DstSize, const char*_Src, rsize_t _MaxCount);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t strncpy_s(char(&_Dest)[_SizeFIXED], const char*_Source, size_t _Count)
		{
			return strncpy_s(_Dest, _SizeFIXED, _Source, _Count);
		}
	}
	char*strncpy(char*_Dest, const char*_Source, size_t _Count);
	char*_strnset(char*_Str, int _Val, size_t _MaxCount);
	errno_t _strnset_s(char*_Str, size_t _Size, int _Val, size_t _MaxCount);
	const char*strpbrk(const char*_Str, const char*_Control);
	const char*strrchr(const char*_Str, int _Ch);
	char*_strrev(char*_Str);
	size_t strspn(const char*_Str, const char*_Control);
	const char*strstr(const char*_Str, const char*_SubStr);
	char*strtok(char*_Str, const char*_Delim);
	char*strtok_s(char*_Str, const char*_Delim, char**_Context);
	errno_t _strupr_s(char*_Str, size_t _Size);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _strupr_s(char(&_String)[_SizeFIXED])
		{
			return _strupr_s(_String, _SizeFIXED);
		}
	}
	char*_strupr(char*_String);
	errno_t _strupr_s_l(char*_Str, size_t _Size, _locale_t _Locale);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _strupr_s_l(char(&_String)[_SizeFIXED], _locale_t _Locale)
		{
			return _strupr_s_l(_String, _SizeFIXED, _Locale);
		}
	}
	char*_strupr_l(char*_String, _locale_t _Locale);
	size_t strxfrm(char*_Dst, const char*_Src, size_t _MaxCount);
	size_t _strxfrm_l(char*_Dst, const char*_Src, size_t _MaxCount, _locale_t _Locale);
	extern "C++"
	{
		inline char*strchr(char*_Str, int _Ch)
		{
			return (char*)strchr((const char*)_Str, _Ch);
		}
		inline char*strpbrk(char*_Str, const char*_Control)
		{
			return (char*)strpbrk((const char*)_Str, _Control);
		}
		inline char*strrchr(char*_Str, int _Ch)
		{
			return (char*)strrchr((const char*)_Str, _Ch);
		}
		inline char*strstr(char*_Str, const char*_SubStr)
		{
			return (char*)strstr((const char*)_Str, _SubStr);
		}
		inline void*memchr(void*_Pv, int _C, size_t _N)
		{
			return (void*)memchr((const void*)_Pv, _C, _N);
		}
	}
	wchar_t*_wcsdup(const wchar_t*_Str);
	errno_t wcscat_s(wchar_t*_Dst, rsize_t _DstSize, const wchar_t*_Src);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t wcscat_s(wchar_t(&_Dest)[_SizeFIXED], const wchar_t*_Source)
		{
			return wcscat_s(_Dest, _SizeFIXED, _Source);
		}
	}
	wchar_t*wcscat(wchar_t*_Dest, const wchar_t*_Source);
	const wchar_t*wcschr(const wchar_t*_Str, wchar_t _Ch);
	int wcscmp(const wchar_t*_Str1, const wchar_t*_Str2);
	errno_t wcscpy_s(wchar_t*_Dst, rsize_t _DstSize, const wchar_t*_Src);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t wcscpy_s(wchar_t(&_Dest)[_SizeFIXED], const wchar_t*_Source)
		{
			return wcscpy_s(_Dest, _SizeFIXED, _Source);
		}
	}
	wchar_t*wcscpy(wchar_t*_Dest, const wchar_t*_Source);
	size_t wcscspn(const wchar_t*_Str, const wchar_t*_Control);
	size_t wcslen(const wchar_t*_Str);
	size_t wcsnlen(const wchar_t*_Src, size_t _MaxCount);
	static size_t wcsnlen_s(const wchar_t*_Src, size_t _MaxCount)
	{
		return wcsnlen(_Src, _MaxCount);
	}
	errno_t wcsncat_s(wchar_t*_Dst, rsize_t _DstSize, const wchar_t*_Src, rsize_t _MaxCount);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t wcsncat_s(wchar_t(&_Dest)[_SizeFIXED], const wchar_t*_Source, size_t _Count)
		{
			return wcsncat_s(_Dest, _SizeFIXED, _Source, _Count);
		}
	}
	wchar_t*wcsncat(wchar_t*_Dest, const wchar_t*_Source, size_t _Count);
	int wcsncmp(const wchar_t*_Str1, const wchar_t*_Str2, size_t _MaxCount);
	errno_t wcsncpy_s(wchar_t*_Dst, rsize_t _DstSize, const wchar_t*_Src, rsize_t _MaxCount);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t wcsncpy_s(wchar_t(&_Dest)[_SizeFIXED], const wchar_t*_Source, size_t _Count)
		{
			return wcsncpy_s(_Dest, _SizeFIXED, _Source, _Count);
		}
	}
	wchar_t*wcsncpy(wchar_t*_Dest, const wchar_t*_Source, size_t _Count);
	const wchar_t*wcspbrk(const wchar_t*_Str, const wchar_t*_Control);
	const wchar_t*wcsrchr(const wchar_t*_Str, wchar_t _Ch);
	size_t wcsspn(const wchar_t*_Str, const wchar_t*_Control);
	const wchar_t*wcsstr(const wchar_t*_Str, const wchar_t*_SubStr);
	wchar_t*wcstok(wchar_t*_Str, const wchar_t*_Delim);
	wchar_t*wcstok_s(wchar_t*_Str, const wchar_t*_Delim, wchar_t**_Context);
	wchar_t*_wcserror(int _ErrNum);
	errno_t _wcserror_s(wchar_t*_Buf, size_t _SizeInWords, int _ErrNum);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wcserror_s(wchar_t(&_Buffer)[_SizeFIXED], int _Error)
		{
			return _wcserror_s(_Buffer, _SizeFIXED, _Error);
		}
	}
	wchar_t*__wcserror(const wchar_t*_Str);
	errno_t __wcserror_s(wchar_t*_Buffer, size_t _SizeInWords, const wchar_t*_ErrMsg);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t __wcserror_s(wchar_t(&_Buffer)[_SizeFIXED], const wchar_t*_ErrorMessage)
		{
			return __wcserror_s(_Buffer, _SizeFIXED, _ErrorMessage);
		}
	}
	int _wcsicmp(const wchar_t*_Str1, const wchar_t*_Str2);
	int _wcsicmp_l(const wchar_t*_Str1, const wchar_t*_Str2, _locale_t _Locale);
	int _wcsnicmp(const wchar_t*_Str1, const wchar_t*_Str2, size_t _MaxCount);
	int _wcsnicmp_l(const wchar_t*_Str1, const wchar_t*_Str2, size_t _MaxCount, _locale_t _Locale);
	wchar_t*_wcsnset(wchar_t*_Str, wchar_t _Val, size_t _MaxCount);
	errno_t _wcsnset_s(wchar_t*_Dst, size_t _DstSizeInWords, wchar_t _Val, size_t _MaxCount);
	wchar_t*_wcsrev(wchar_t*_Str);
	wchar_t*_wcsset(wchar_t*_Str, wchar_t _Val);
	errno_t _wcsset_s(wchar_t*_Str, size_t _SizeInWords, wchar_t _Val);
	errno_t _wcslwr_s(wchar_t*_Str, size_t _SizeInWords);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wcslwr_s(wchar_t(&_String)[_SizeFIXED])
		{
			return _wcslwr_s(_String, _SizeFIXED);
		}
	}
	wchar_t*_wcslwr(wchar_t*_String);
	errno_t _wcslwr_s_l(wchar_t*_Str, size_t _SizeInWords, _locale_t _Locale);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wcslwr_s_l(wchar_t(&_String)[_SizeFIXED], _locale_t _Locale)
		{
			return _wcslwr_s_l(_String, _SizeFIXED, _Locale);
		}
	}
	wchar_t*_wcslwr_l(wchar_t*_String, _locale_t _Locale);
	errno_t _wcsupr_s(wchar_t*_Str, size_t _Size);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wcsupr_s(wchar_t(&_String)[_SizeFIXED])
		{
			return _wcsupr_s(_String, _SizeFIXED);
		}
	}
	wchar_t*_wcsupr(wchar_t*_String);
	errno_t _wcsupr_s_l(wchar_t*_Str, size_t _Size, _locale_t _Locale);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wcsupr_s_l(wchar_t(&_String)[_SizeFIXED], _locale_t _Locale)
		{
			return _wcsupr_s_l(_String, _SizeFIXED, _Locale);
		}
	}
	wchar_t*_wcsupr_l(wchar_t*_String, _locale_t _Locale);
	size_t wcsxfrm(wchar_t*_Dst, const wchar_t*_Src, size_t _MaxCount);
	size_t _wcsxfrm_l(wchar_t*_Dst, const wchar_t*_Src, size_t _MaxCount, _locale_t _Locale);
	int wcscoll(const wchar_t*_Str1, const wchar_t*_Str2);
	int _wcscoll_l(const wchar_t*_Str1, const wchar_t*_Str2, _locale_t _Locale);
	int _wcsicoll(const wchar_t*_Str1, const wchar_t*_Str2);
	int _wcsicoll_l(const wchar_t*_Str1, const wchar_t*_Str2, _locale_t _Locale);
	int _wcsncoll(const wchar_t*_Str1, const wchar_t*_Str2, size_t _MaxCount);
	int _wcsncoll_l(const wchar_t*_Str1, const wchar_t*_Str2, size_t _MaxCount, _locale_t _Locale);
	int _wcsnicoll(const wchar_t*_Str1, const wchar_t*_Str2, size_t _MaxCount);
	int _wcsnicoll_l(const wchar_t*_Str1, const wchar_t*_Str2, size_t _MaxCount, _locale_t _Locale);
	extern "C++"
	{
		inline wchar_t*wcschr(wchar_t*_Str, wchar_t _Ch)
		{
			return ((wchar_t*)wcschr((const wchar_t*)_Str, _Ch));
		}
		inline wchar_t*wcspbrk(wchar_t*_Str, const wchar_t*_Control)
		{
			return ((wchar_t*)wcspbrk((const wchar_t*)_Str, _Control));
		}
		inline wchar_t*wcsrchr(wchar_t*_Str, wchar_t _Ch)
		{
			return ((wchar_t*)wcsrchr((const wchar_t*)_Str, _Ch));
		}
		inline wchar_t*wcsstr(wchar_t*_Str, const wchar_t*_SubStr)
		{
			return ((wchar_t*)wcsstr((const wchar_t*)_Str, _SubStr));
		}
	}
}
namespace std
{
	using::size_t;
	using::memchr;
	using::memcmp;
	using::memcpy;
	using::memmove;
	using::memset;
	using::strcat;
	using::strchr;
	using::strcmp;
	using::strcoll;
	using::strcpy;
	using::strcspn;
	using::strerror;
	using::strlen;
	using::strncat;
	using::strncmp;
	using::strncpy;
	using::strpbrk;
	using::strrchr;
	using::strspn;
	using::strstr;
	using::strtok;
	using::strxfrm;
}
extern "C"
{
}
namespace std
{
	using::ptrdiff_t;
	using::size_t;
}
namespace std
{
	enum _Uninitialized
	{
		_Noinit
	};
}
namespace std
{
}
typedef void(*terminate_function)();
typedef void(*terminate_handler)();
typedef void(*unexpected_function)();
typedef void(*unexpected_handler)();
struct _EXCEPTION_POINTERS;
typedef void(*_se_translator_function)(unsigned int, struct _EXCEPTION_POINTERS*);
void terminate(void);
void unexpected(void);
int _is_exception_typeof(const type_info&_Type, struct _EXCEPTION_POINTERS*_ExceptionPtr);
terminate_function set_terminate(terminate_function _NewPtFunc);
extern "C" terminate_function _get_terminate(void);
unexpected_function set_unexpected(unexpected_function _NewPtFunc);
extern "C" unexpected_function _get_unexpected(void);
_se_translator_function _set_se_translator(_se_translator_function _NewPtFunc);
bool __uncaught_exception();
extern "C"
{
	typedef struct _heapinfo
	{
		int*_pentry;
		size_t _size;
		int _useflag;
	}
	_HEAPINFO;
	extern unsigned int _amblksiz;
	int _resetstkoflw(void);
	unsigned long _set_malloc_crt_max_wait(unsigned long _NewValue);
	void*_expand(void*_Memory, size_t _NewSize);
	size_t _msize(void*_Memory);
	void*_alloca(size_t _Size);
	size_t _get_sbh_threshold(void);
	int _set_sbh_threshold(size_t _NewValue);
	errno_t _set_amblksiz(size_t _Value);
	errno_t _get_amblksiz(size_t*_Value);
	int _heapadd(void*_Memory, size_t _Size);
	int _heapchk(void);
	int _heapmin(void);
	int _heapset(unsigned int _Fill);
	int _heapwalk(_HEAPINFO*_EntryInfo);
	size_t _heapused(size_t*_Used, size_t*_Commit);
	intptr_t _get_heap_handle(void);
	typedef char __static_assert_t[(sizeof(unsigned int)<=8)];
	void*_MarkAllocaS(void*_Ptr, unsigned int _Marker)
	{
		if(_Ptr)
		{
			*((unsigned int*)_Ptr)=_Marker;
			_Ptr=(char*)_Ptr+8;
		}
		return _Ptr;
	}
	void _freea(void*_Memory)
	{
		unsigned int _Marker;
		if(_Memory)
		{
			_Memory=(char*)_Memory-8;
			_Marker=*(unsigned int*)_Memory;
			if(_Marker==0xDDDD)
			{
				free(_Memory);
			}
		}
	}
}
typedef const char*__exString;
extern "C" size_t strlen(const char*);
extern "C" errno_t strcpy_s(char*_Dst, size_t _DstSize, const char*_Src);
namespace std
{
	class exception
	{
	public:
		exception();
		exception(const char*const&);
		exception(const char*const&, int);
		exception(const exception&);
		exception&operator=(const exception&);
		virtual~exception();
		virtual const char*what()const;
	private:
		const char*_m_what;
		int _m_doFree;
	};
	using::set_terminate;
	using::terminate_handler;
	using::terminate;
	using::set_unexpected;
	using::unexpected_handler;
	using::unexpected;
	typedef void(*_Prhand)(const exception&);
	bool uncaught_exception();
	class bad_exception: public exception
	{
	public:
		bad_exception(const char*_Message="bad exception")throw(): exception(_Message)
		{
		}
		virtual~bad_exception()throw()
		{
		}
	};
	static const char*_bad_alloc_Message="bad allocation";
	class bad_alloc: public exception
	{
	public:
		bad_alloc(const char*_Message)throw(): exception(_Message)
		{
		}
		bad_alloc()throw(): exception(_bad_alloc_Message, 1)
		{
		}
		virtual~bad_alloc()throw()
		{
		}
	};
}
namespace std
{
	typedef void(*new_handler)();
	struct nothrow_t
	{
	};
	extern const nothrow_t nothrow;
	new_handler set_new_handler(new_handler)throw();
}
void operator delete(void*)throw();
void*operator new(size_t _Size)throw(...);
inline void*operator new(size_t, void*_Where)throw()
{
	return (_Where);
}
inline void operator delete(void*, void*)throw()
{
}
inline void*operator new[](size_t, void*_Where)throw()
{
	return (_Where);
}
inline void operator delete[](void*, void*)throw()
{
}
void operator delete[](void*)throw();
void*operator new[](size_t _Size)throw(...);
void*operator new(size_t _Size, const std::nothrow_t&)throw();
void*operator new[](size_t _Size, const std::nothrow_t&)throw();
void operator delete(void*, const std::nothrow_t&)throw();
void operator delete[](void*, const std::nothrow_t&)throw();
using std::new_handler;
extern "C"
{
	typedef unsigned long _fsize_t;
	struct _wfinddata32_t
	{
		unsigned attrib;
		__time32_t time_create;
		__time32_t time_access;
		__time32_t time_write;
		_fsize_t size;
		wchar_t name[260];
	};
	struct _wfinddata32i64_t
	{
		unsigned attrib;
		__time32_t time_create;
		__time32_t time_access;
		__time32_t time_write;
		__int64 size;
		wchar_t name[260];
	};
	struct _wfinddata64i32_t
	{
		unsigned attrib;
		__time64_t time_create;
		__time64_t time_access;
		__time64_t time_write;
		_fsize_t size;
		wchar_t name[260];
	};
	struct _wfinddata64_t
	{
		unsigned attrib;
		__time64_t time_create;
		__time64_t time_access;
		__time64_t time_write;
		__int64 size;
		wchar_t name[260];
	};
	const unsigned short*__pctype_func(void);
	extern const unsigned short*_pctype;
	extern const unsigned short _wctype[];
	const wctype_t*__pwctype_func(void);
	extern const wctype_t*_pwctype;
	int iswalpha(wint_t _C);
	int _iswalpha_l(wint_t _C, _locale_t _Locale);
	int iswupper(wint_t _C);
	int _iswupper_l(wint_t _C, _locale_t _Locale);
	int iswlower(wint_t _C);
	int _iswlower_l(wint_t _C, _locale_t _Locale);
	int iswdigit(wint_t _C);
	int _iswdigit_l(wint_t _C, _locale_t _Locale);
	int iswxdigit(wint_t _C);
	int _iswxdigit_l(wint_t _C, _locale_t _Locale);
	int iswspace(wint_t _C);
	int _iswspace_l(wint_t _C, _locale_t _Locale);
	int iswpunct(wint_t _C);
	int _iswpunct_l(wint_t _C, _locale_t _Locale);
	int iswalnum(wint_t _C);
	int _iswalnum_l(wint_t _C, _locale_t _Locale);
	int iswprint(wint_t _C);
	int _iswprint_l(wint_t _C, _locale_t _Locale);
	int iswgraph(wint_t _C);
	int _iswgraph_l(wint_t _C, _locale_t _Locale);
	int iswcntrl(wint_t _C);
	int _iswcntrl_l(wint_t _C, _locale_t _Locale);
	int iswascii(wint_t _C);
	int isleadbyte(int _C);
	int _isleadbyte_l(int _C, _locale_t _Locale);
	wint_t towupper(wint_t _C);
	wint_t _towupper_l(wint_t _C, _locale_t _Locale);
	wint_t towlower(wint_t _C);
	wint_t _towlower_l(wint_t _C, _locale_t _Locale);
	int iswctype(wint_t _C, wctype_t _Type);
	int _iswctype_l(wint_t _C, wctype_t _Type, _locale_t _Locale);
	int __iswcsymf(wint_t _C);
	int _iswcsymf_l(wint_t _C, _locale_t _Locale);
	int __iswcsym(wint_t _C);
	int _iswcsym_l(wint_t _C, _locale_t _Locale);
	int is_wctype(wint_t _C, wctype_t _Type);
	wchar_t*_wgetcwd(wchar_t*_DstBuf, int _SizeInWords);
	wchar_t*_wgetdcwd(int _Drive, wchar_t*_DstBuf, int _SizeInWords);
	wchar_t*_wgetdcwd_nolock(int _Drive, wchar_t*_DstBuf, int _SizeInWords);
	int _wchdir(const wchar_t*_Path);
	int _wmkdir(const wchar_t*_Path);
	int _wrmdir(const wchar_t*_Path);
	int _waccess(const wchar_t*_Filename, int _AccessMode);
	errno_t _waccess_s(const wchar_t*_Filename, int _AccessMode);
	int _wchmod(const wchar_t*_Filename, int _Mode);
	int _wcreat(const wchar_t*_Filename, int _PermissionMode);
	intptr_t _wfindfirst32(const wchar_t*_Filename, struct _wfinddata32_t*_FindData);
	int _wfindnext32(intptr_t _FindHandle, struct _wfinddata32_t*_FindData);
	int _wunlink(const wchar_t*_Filename);
	int _wrename(const wchar_t*_NewFilename, const wchar_t*_OldFilename);
	errno_t _wmktemp_s(wchar_t*_TemplateName, size_t _SizeInWords);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wmktemp_s(wchar_t(&_TemplateName)[_SizeFIXED])
		{
			return _wmktemp_s(_TemplateName, _SizeFIXED);
		}
	}
	wchar_t*_wmktemp(wchar_t*_TemplateName);
	intptr_t _wfindfirst32i64(const wchar_t*_Filename, struct _wfinddata32i64_t*_FindData);
	intptr_t _wfindfirst64i32(const wchar_t*_Filename, struct _wfinddata64i32_t*_FindData);
	intptr_t _wfindfirst64(const wchar_t*_Filename, struct _wfinddata64_t*_FindData);
	int _wfindnext32i64(intptr_t _FindHandle, struct _wfinddata32i64_t*_FindData);
	int _wfindnext64i32(intptr_t _FindHandle, struct _wfinddata64i32_t*_FindData);
	int _wfindnext64(intptr_t _FindHandle, struct _wfinddata64_t*_FindData);
	errno_t _wsopen_s(int*_FileHandle, const wchar_t*_Filename, int _OpenFlag, int _ShareFlag, int _PermissionFlag);
	extern "C++" int _wopen(const wchar_t*_Filename, int _OpenFlag, int _PermissionMode=0);
	extern "C++" int _wsopen(const wchar_t*_Filename, int _OpenFlag, int _ShareFlag, int _PermissionMode=0);
	wchar_t*_wsetlocale(int _Category, const wchar_t*_Locale);
	intptr_t _wexecl(const wchar_t*_Filename, const wchar_t*_ArgList, ...);
	intptr_t _wexecle(const wchar_t*_Filename, const wchar_t*_ArgList, ...);
	intptr_t _wexeclp(const wchar_t*_Filename, const wchar_t*_ArgList, ...);
	intptr_t _wexeclpe(const wchar_t*_Filename, const wchar_t*_ArgList, ...);
	intptr_t _wexecv(const wchar_t*_Filename, const wchar_t*const*_ArgList);
	intptr_t _wexecve(const wchar_t*_Filename, const wchar_t*const*_ArgList, const wchar_t*const*_Env);
	intptr_t _wexecvp(const wchar_t*_Filename, const wchar_t*const*_ArgList);
	intptr_t _wexecvpe(const wchar_t*_Filename, const wchar_t*const*_ArgList, const wchar_t*const*_Env);
	intptr_t _wspawnl(int _Mode, const wchar_t*_Filename, const wchar_t*_ArgList, ...);
	intptr_t _wspawnle(int _Mode, const wchar_t*_Filename, const wchar_t*_ArgList, ...);
	intptr_t _wspawnlp(int _Mode, const wchar_t*_Filename, const wchar_t*_ArgList, ...);
	intptr_t _wspawnlpe(int _Mode, const wchar_t*_Filename, const wchar_t*_ArgList, ...);
	intptr_t _wspawnv(int _Mode, const wchar_t*_Filename, const wchar_t*const*_ArgList);
	intptr_t _wspawnve(int _Mode, const wchar_t*_Filename, const wchar_t*const*_ArgList, const wchar_t*const*_Env);
	intptr_t _wspawnvp(int _Mode, const wchar_t*_Filename, const wchar_t*const*_ArgList);
	intptr_t _wspawnvpe(int _Mode, const wchar_t*_Filename, const wchar_t*const*_ArgList, const wchar_t*const*_Env);
	typedef unsigned short _ino_t;
	typedef unsigned int _dev_t;
	typedef long _off_t;
	struct _stat32
	{
		_dev_t st_dev;
		_ino_t st_ino;
		unsigned short st_mode;
		short st_nlink;
		short st_uid;
		short st_gid;
		_dev_t st_rdev;
		_off_t st_size;
		__time32_t st_atime;
		__time32_t st_mtime;
		__time32_t st_ctime;
	};
	struct _stat32i64
	{
		_dev_t st_dev;
		_ino_t st_ino;
		unsigned short st_mode;
		short st_nlink;
		short st_uid;
		short st_gid;
		_dev_t st_rdev;
		__int64 st_size;
		__time32_t st_atime;
		__time32_t st_mtime;
		__time32_t st_ctime;
	};
	struct _stat64i32
	{
		_dev_t st_dev;
		_ino_t st_ino;
		unsigned short st_mode;
		short st_nlink;
		short st_uid;
		short st_gid;
		_dev_t st_rdev;
		_off_t st_size;
		__time64_t st_atime;
		__time64_t st_mtime;
		__time64_t st_ctime;
	};
	struct _stat64
	{
		_dev_t st_dev;
		_ino_t st_ino;
		unsigned short st_mode;
		short st_nlink;
		short st_uid;
		short st_gid;
		_dev_t st_rdev;
		__int64 st_size;
		__time64_t st_atime;
		__time64_t st_mtime;
		__time64_t st_ctime;
	};
	int _wstat32(const wchar_t*_Name, struct _stat32*_Stat);
	int _wstat32i64(const wchar_t*_Name, struct _stat32i64*_Stat);
	int _wstat64i32(const wchar_t*_Name, struct _stat64i32*_Stat);
	int _wstat64(const wchar_t*_Name, struct _stat64*_Stat);
	errno_t _cgetws_s(wchar_t*_Buffer, size_t _SizeInWords, size_t*_SizeRead);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _cgetws_s(wchar_t(&_Buffer)[_SizeFIXED], size_t*_Size)
		{
			return _cgetws_s(_Buffer, _SizeFIXED, _Size);
		}
	}
	wchar_t*_cgetws(wchar_t*_Buffer);
	wint_t _getwch(void);
	wint_t _getwche(void);
	wint_t _putwch(wchar_t _WCh);
	wint_t _ungetwch(wint_t _WCh);
	int _cputws(const wchar_t*_String);
	int _cwprintf(const wchar_t*_Format, ...);
	int _cwprintf_s(const wchar_t*_Format, ...);
	int _cwscanf(const wchar_t*_Format, ...);
	int _cwscanf_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _cwscanf_s(const wchar_t*_Format, ...);
	int _cwscanf_s_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _vcwprintf(const wchar_t*_Format, va_list _ArgList);
	int _vcwprintf_s(const wchar_t*_Format, va_list _ArgList);
	int _cwprintf_p(const wchar_t*_Format, ...);
	int _vcwprintf_p(const wchar_t*_Format, va_list _ArgList);
	int _cwprintf_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _cwprintf_s_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _vcwprintf_l(const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _vcwprintf_s_l(const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	int _cwprintf_p_l(const wchar_t*_Format, _locale_t _Locale, ...);
	int _vcwprintf_p_l(const wchar_t*_Format, _locale_t _Locale, va_list _ArgList);
	wint_t _putwch_nolock(wchar_t _WCh);
	wint_t _getwch_nolock(void);
	wint_t _getwche_nolock(void);
	wint_t _ungetwch_nolock(wint_t _WCh);
	struct tm
	{
		int tm_sec;
		int tm_min;
		int tm_hour;
		int tm_mday;
		int tm_mon;
		int tm_year;
		int tm_wday;
		int tm_yday;
		int tm_isdst;
	};
	wchar_t*_wasctime(const struct tm*_Tm);
	errno_t _wasctime_s(wchar_t*_Buf, size_t _SizeInWords, const struct tm*_Tm);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wasctime_s(wchar_t(&_Buffer)[_SizeFIXED], const struct tm*_Time)
		{
			return _wasctime_s(_Buffer, _SizeFIXED, _Time);
		}
	}
	wchar_t*_wctime32(const __time32_t*_Time);
	errno_t _wctime32_s(wchar_t*_Buf, size_t _SizeInWords, const __time32_t*_Time);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wctime32_s(wchar_t(&_Buffer)[_SizeFIXED], const __time32_t*_Time)
		{
			return _wctime32_s(_Buffer, _SizeFIXED, _Time);
		}
	}
	size_t wcsftime(wchar_t*_Buf, size_t _SizeInWords, const wchar_t*_Format, const struct tm*_Tm);
	size_t _wcsftime_l(wchar_t*_Buf, size_t _SizeInWords, const wchar_t*_Format, const struct tm*_Tm, _locale_t _Locale);
	errno_t _wstrdate_s(wchar_t*_Buf, size_t _SizeInWords);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wstrdate_s(wchar_t(&_Buffer)[_SizeFIXED])
		{
			return _wstrdate_s(_Buffer, _SizeFIXED);
		}
	}
	wchar_t*_wstrdate(wchar_t*_Buffer);
	errno_t _wstrtime_s(wchar_t*_Buf, size_t _SizeInWords);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wstrtime_s(wchar_t(&_Buffer)[_SizeFIXED])
		{
			return _wstrtime_s(_Buffer, _SizeFIXED);
		}
	}
	wchar_t*_wstrtime(wchar_t*_Buffer);
	wchar_t*_wctime64(const __time64_t*_Time);
	errno_t _wctime64_s(wchar_t*_Buf, size_t _SizeInWords, const __time64_t*_Time);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t _wctime64_s(wchar_t(&_Buffer)[_SizeFIXED], const __time64_t*_Time)
		{
			return _wctime64_s(_Buffer, _SizeFIXED, _Time);
		}
	}
	static wchar_t*_wctime(const time_t*_Time)
	{
		return _wctime64(_Time);
	}
	static errno_t _wctime_s(wchar_t*_Buffer, size_t _SizeInWords, const time_t*_Time)
	{
		return _wctime64_s(_Buffer, _SizeInWords, _Time);
	}
	typedef int mbstate_t;
	typedef wchar_t _Wint_t;
	wint_t btowc(int);
	size_t mbrlen(const char*_Ch, size_t _SizeInBytes, mbstate_t*_State);
	size_t mbrtowc(wchar_t*_DstCh, const char*_SrcCh, size_t _SizeInBytes, mbstate_t*_State);
	errno_t mbsrtowcs_s(size_t*_Retval, wchar_t*_Dst, size_t _SizeInWords, const char**_PSrc, size_t _N, mbstate_t*_State);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t mbsrtowcs_s(size_t*_Retval, wchar_t(&_Dest)[_SizeFIXED], const char**_PSource, size_t _Count, mbstate_t*_State)
		{
			return mbsrtowcs_s(_Retval, _Dest, _SizeFIXED, _PSource, _Count, _State);
		}
	}
	size_t mbsrtowcs(wchar_t*_Dest, const char**_PSrc, size_t _Count, mbstate_t*_State);
	errno_t wcrtomb_s(size_t*_Retval, char*_Dst, size_t _SizeInBytes, wchar_t _Ch, mbstate_t*_State);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t wcrtomb_s(size_t*_Retval, char(&_Dest)[_SizeFIXED], wchar_t _Source, mbstate_t*_State)
		{
			return wcrtomb_s(_Retval, _Dest, _SizeFIXED, _Source, _State);
		}
	}
	size_t wcrtomb(char*_Dest, wchar_t _Source, mbstate_t*_State);
	errno_t wcsrtombs_s(size_t*_Retval, char*_Dst, size_t _SizeInBytes, const wchar_t**_Src, size_t _Size, mbstate_t*_State);
	extern "C++"
	{
		template<size_t _SizeFIXED>
		inline errno_t wcsrtombs_s(size_t*_Retval, char(&_Dest)[_SizeFIXED], const wchar_t**_PSrc, size_t _Count, mbstate_t*_State)
		{
			return wcsrtombs_s(_Retval, _Dest, _SizeFIXED, _PSrc, _Count, _State);
		}
	}
	size_t wcsrtombs(char*_Dest, const wchar_t**_PSource, size_t _Count, mbstate_t*_State);
	int wctob(wint_t _WCh);
	void*memmove(void*_Dst, const void*_Src, size_t _MaxCount);
	void*memcpy(void*_Dst, const void*_Src, size_t _MaxCount);
	errno_t memcpy_s(void*_Dst, rsize_t _DstSize, const void*_Src, rsize_t _MaxCount);
	errno_t memmove_s(void*_Dst, rsize_t _DstSize, const void*_Src, rsize_t _MaxCount);
	int fwide(FILE*_F, int _M)
	{
		(void)_F;
		return (_M);
	}
	int mbsinit(const mbstate_t*_P)
	{
		return (_P==0||*_P==0);
	}
	const wchar_t*wmemchr(const wchar_t*_S, wchar_t _C, size_t _N)
	{
		for(;
			0<_N;
			++_S, --_N)if(*_S==_C)return (const wchar_t*)(_S);
		return (0);
	}
	int wmemcmp(const wchar_t*_S1, const wchar_t*_S2, size_t _N)
	{
		for(;
			0<_N;
			++_S1, ++_S2, --_N)if(*_S1!=*_S2)return (*_S1<*_S2?-1: +1);
		return (0);
	}
	wchar_t*wmemcpy(wchar_t*_S1, const wchar_t*_S2, size_t _N)
	{
		return (wchar_t*)memcpy(_S1, _S2, _N*sizeof(wchar_t));
	}
	errno_t wmemcpy_s(wchar_t*_S1, rsize_t _N1, const wchar_t*_S2, rsize_t _N)
	{
		return memcpy_s(_S1, _N1*sizeof(wchar_t), _S2, _N*sizeof(wchar_t));
	}
	wchar_t*wmemmove(wchar_t*_S1, const wchar_t*_S2, size_t _N)
	{
		return (wchar_t*)memmove(_S1, _S2, _N*sizeof(wchar_t));
	}
	errno_t wmemmove_s(wchar_t*_S1, rsize_t _N1, const wchar_t*_S2, rsize_t _N)
	{
		return memmove_s(_S1, _N1*sizeof(wchar_t), _S2, _N*sizeof(wchar_t));
	}
	wchar_t*wmemset(wchar_t*_S, wchar_t _C, size_t _N)
	{
		wchar_t*_Su=_S;
		for(;
			0<_N;
			++_Su, --_N)
		{
			*_Su=_C;
		}
		return (_S);
	}
	extern "C++"
	{
		inline wchar_t*wmemchr(wchar_t*_S, wchar_t _C, size_t _N)
		{
			return (wchar_t*)wmemchr((const wchar_t*)_S, _C, _N);
		}
	}
}
typedef mbstate_t _Mbstatet;
namespace std
{
	using::mbstate_t;
	using::size_t;
	using::tm;
	using::wint_t;
	using::btowc;
	using::fgetwc;
	using::fgetws;
	using::fputwc;
	using::fputws;
	using::fwide;
	using::fwprintf;
	using::fwscanf;
	using::getwc;
	using::getwchar;
	using::mbrlen;
	using::mbrtowc;
	using::mbsrtowcs;
	using::mbsinit;
	using::putwc;
	using::putwchar;
	using::swprintf;
	using::swscanf;
	using::ungetwc;
	using::vfwprintf;
	using::vswprintf;
	using::vwprintf;
	using::wcrtomb;
	using::wprintf;
	using::wscanf;
	using::wcsrtombs;
	using::wcstol;
	using::wcscat;
	using::wcschr;
	using::wcscmp;
	using::wcscoll;
	using::wcscpy;
	using::wcscspn;
	using::wcslen;
	using::wcsncat;
	using::wcsncmp;
	using::wcsncpy;
	using::wcspbrk;
	using::wcsrchr;
	using::wcsspn;
	using::wcstod;
	using::wcstoul;
	using::wcsstr;
	using::wcstok;
	using::wcsxfrm;
	using::wctob;
	using::wmemchr;
	using::wmemcmp;
	using::wmemcpy;
	using::wmemmove;
	using::wmemset;
	using::wcsftime;
}
extern "C"
{
	typedef void*_HFILE;
	typedef int(*_CRT_REPORT_HOOK)(int, char*, int*);
	typedef int(*_CRT_REPORT_HOOKW)(int, wchar_t*, int*);
	typedef int(*_CRT_ALLOC_HOOK)(int, void*, size_t, int, long, const unsigned char*, int);
	typedef void(*_CRT_DUMP_CLIENT)(void*, size_t);
	struct _CrtMemBlockHeader;
	typedef struct _CrtMemState
	{
		struct _CrtMemBlockHeader*pBlockHeader;
		size_t lCounts[5];
		size_t lSizes[5];
		size_t lHighWaterCount;
		size_t lTotalCount;
	}
	_CrtMemState;
	extern long _crtAssertBusy;
	_CRT_REPORT_HOOK _CrtGetReportHook(void);
	_CRT_REPORT_HOOK _CrtSetReportHook(_CRT_REPORT_HOOK _PFnNewHook);
	int _CrtSetReportHook2(int _Mode, _CRT_REPORT_HOOK _PFnNewHook);
	int _CrtSetReportHookW2(int _Mode, _CRT_REPORT_HOOKW _PFnNewHook);
	int _CrtSetReportMode(int _ReportType, int _ReportMode);
	_HFILE _CrtSetReportFile(int _ReportType, _HFILE _ReportFile);
	int _CrtDbgReport(int _ReportType, const char*_Filename, int _Linenumber, const char*_ModuleName, const char*_Format, ...);
	size_t _CrtSetDebugFillThreshold(size_t _NewDebugFillThreshold);
	int _CrtDbgReportW(int _ReportType, const wchar_t*_Filename, int _LineNumber, const wchar_t*_ModuleName, const wchar_t*_Format, ...);
	extern long _crtBreakAlloc;
	long _CrtSetBreakAlloc(long _BreakAlloc);
	void*_malloc_dbg(size_t _Size, int _BlockType, const char*_Filename, int _LineNumber);
	void*_calloc_dbg(size_t _NumOfElements, size_t _SizeOfElements, int _BlockType, const char*_Filename, int _LineNumber);
	void*_realloc_dbg(void*_Memory, size_t _NewSize, int _BlockType, const char*_Filename, int _LineNumber);
	void*_recalloc_dbg(void*_Memory, size_t _NumOfElements, size_t _SizeOfElements, int _BlockType, const char*_Filename, int _LineNumber);
	void*_expand_dbg(void*_Memory, size_t _NewSize, int _BlockType, const char*_Filename, int _LineNumber);
	void _free_dbg(void*_Memory, int _BlockType);
	size_t _msize_dbg(void*_Memory, int _BlockType);
	void*_aligned_malloc_dbg(size_t _Size, size_t _Alignment, const char*_Filename, int _LineNumber);
	void*_aligned_realloc_dbg(void*_Memory, size_t _Size, size_t _Alignment, const char*_Filename, int _LineNumber);
	void*_aligned_recalloc_dbg(void*_Memory, size_t _NumOfElements, size_t _SizeOfElements, size_t _Alignment, const char*_Filename, int _LineNumber);
	void*_aligned_offset_malloc_dbg(size_t _Size, size_t _Alignment, size_t _Offset, const char*_Filename, int _LineNumber);
	void*_aligned_offset_realloc_dbg(void*_Memory, size_t _Size, size_t _Alignment, size_t _Offset, const char*_Filename, int _LineNumber);
	void*_aligned_offset_recalloc_dbg(void*_Memory, size_t _NumOfElements, size_t _SizeOfElements, size_t _Alignment, size_t _Offset, const char*_Filename, int _LineNumber);
	void _aligned_free_dbg(void*_Memory);
	char*_strdup_dbg(const char*_Str, int _BlockType, const char*_Filename, int _LineNumber);
	wchar_t*_wcsdup_dbg(const wchar_t*_Str, int _BlockType, const char*_Filename, int _LineNumber);
	char*_tempnam_dbg(const char*_DirName, const char*_FilePrefix, int _BlockType, const char*_Filename, int _LineNumber);
	wchar_t*_wtempnam_dbg(const wchar_t*_DirName, const wchar_t*_FilePrefix, int _BlockType, const char*_Filename, int _LineNumber);
	char*_fullpath_dbg(char*_FullPath, const char*_Path, size_t _SizeInBytes, int _BlockType, const char*_Filename, int _LineNumber);
	wchar_t*_wfullpath_dbg(wchar_t*_FullPath, const wchar_t*_Path, size_t _SizeInWords, int _BlockType, const char*_Filename, int _LineNumber);
	char*_getcwd_dbg(char*_DstBuf, int _SizeInBytes, int _BlockType, const char*_Filename, int _LineNumber);
	wchar_t*_wgetcwd_dbg(wchar_t*_DstBuf, int _SizeInWords, int _BlockType, const char*_Filename, int _LineNumber);
	char*_getdcwd_dbg(int _Drive, char*_DstBuf, int _SizeInBytes, int _BlockType, const char*_Filename, int _LineNumber);
	wchar_t*_wgetdcwd_dbg(int _Drive, wchar_t*_DstBuf, int _SizeInWords, int _BlockType, const char*_Filename, int _LineNumber);
	char*_getdcwd_lk_dbg(int _Drive, char*_DstBuf, int _SizeInBytes, int _BlockType, const char*_Filename, int _LineNumber);
	wchar_t*_wgetdcwd_lk_dbg(int _Drive, wchar_t*_DstBuf, int _SizeInWords, int _BlockType, const char*_Filename, int _LineNumber);
	errno_t _dupenv_s_dbg(char**_PBuffer, size_t*_PBufferSizeInBytes, const char*_VarName, int _BlockType, const char*_Filename, int _LineNumber);
	errno_t _wdupenv_s_dbg(wchar_t**_PBuffer, size_t*_PBufferSizeInWords, const wchar_t*_VarName, int _BlockType, const char*_Filename, int _LineNumber);
	_CRT_ALLOC_HOOK _CrtGetAllocHook(void);
	_CRT_ALLOC_HOOK _CrtSetAllocHook(_CRT_ALLOC_HOOK _PfnNewHook);
	extern int _crtDbgFlag;
	int _CrtCheckMemory(void);
	int _CrtSetDbgFlag(int _NewFlag);
	void _CrtDoForAllClientObjects(void(*_PFn)(void*, void*), void*_Context);
	int _CrtIsValidPointer(const void*_Ptr, unsigned int _Bytes, int _ReadWrite);
	int _CrtIsValidHeapPointer(const void*_HeapPtr);
	int _CrtIsMemoryBlock(const void*_Memory, unsigned int _Bytes, long*_RequestNumber, char**_Filename, int*_LineNumber);
	int _CrtReportBlockType(const void*_Memory);
	_CRT_DUMP_CLIENT _CrtGetDumpClient(void);
	_CRT_DUMP_CLIENT _CrtSetDumpClient(_CRT_DUMP_CLIENT _PFnNewDump);
	void _CrtMemCheckpoint(_CrtMemState*_State);
	int _CrtMemDifference(_CrtMemState*_State, const _CrtMemState*_OldState, const _CrtMemState*_NewState);
	void _CrtMemDumpAllObjectsSince(const _CrtMemState*_State);
	void _CrtMemDumpStatistics(const _CrtMemState*_State);
	int _CrtDumpMemoryLeaks(void);
	int _CrtSetCheckCount(int _CheckCount);
	int _CrtGetCheckCount(void);
}
extern "C++"
{
	void*operator new[](size_t _Size);
	void*operator new(size_t _Size, int, const char*, int);
	void*operator new[](size_t _Size, int, const char*, int);
	void operator delete[](void*);
	inline void operator delete(void*_P, int, const char*, int)
	{
		::operator delete(_P);
	}
	inline void operator delete[](void*_P, int, const char*, int)
	{
		::operator delete[](_P);
	}
}
namespace std
{
	typedef long streamoff;
	typedef int streamsize;
	extern fpos_t _Fpz;
	extern const streamoff _BADOFF;
	template<class _Statetype>
	class fpos
	{
		typedef fpos<_Statetype>_Myt;
	public:
		fpos(streamoff _Off=0): _Myoff(_Off), _Fpos(0), _Mystate(_Stz)
		{
		}
		fpos(_Statetype _State, fpos_t _Fileposition): _Myoff(0), _Fpos(_Fileposition), _Mystate(_State)
		{
		}
		_Statetype state()const
		{
			return (_Mystate);
		}
		void state(_Statetype _State)
		{
			_Mystate=_State;
		}
		fpos_t seekpos()const
		{
			return (_Fpos);
		}
		operator streamoff()const
		{
			return (_Myoff+((long)(_Fpos)));
		}
		streamoff operator-(const _Myt&_Right)const
		{
			return ((streamoff)*this-(streamoff)_Right);
		}
		_Myt&operator+=(streamoff _Off)
		{
			_Myoff+=_Off;
			return (*this);
		}
		_Myt&operator-=(streamoff _Off)
		{
			_Myoff-=_Off;
			return (*this);
		}
		_Myt operator+(streamoff _Off)const
		{
			_Myt _Tmp=*this;
			return (_Tmp+=_Off);
		}
		_Myt operator-(streamoff _Off)const
		{
			_Myt _Tmp=*this;
			return (_Tmp-=_Off);
		}
		bool operator==(const _Myt&_Right)const
		{
			return ((streamoff)*this==(streamoff)_Right);
		}
		bool operator!=(const _Myt&_Right)const
		{
			return (!(*this==_Right));
		}
	private:
		static const _Statetype _Stz;
		streamoff _Myoff;
		fpos_t _Fpos;
		_Statetype _Mystate;
	};
	template<class _Statetype>
	const _Statetype fpos<_Statetype>::_Stz=_Statetype();
	typedef fpos<_Mbstatet>streampos;
	typedef streampos wstreampos;
	struct _Unsecure_char_traits_tag
	{
	};
	struct _Secure_char_traits_tag
	{
	};
	struct _Char_traits_base
	{
		typedef _Unsecure_char_traits_tag _Secure_char_traits;
	};
	template<class _Elem>
	struct char_traits: public _Char_traits_base
	{
		typedef _Elem char_type;
		typedef long int_type;
		typedef streampos pos_type;
		typedef streamoff off_type;
		typedef _Mbstatet state_type;
		static void assign(_Elem&_Left, const _Elem&_Right)
		{
			_Left=_Right;
		}
		static bool eq(const _Elem&_Left, const _Elem&_Right)
		{
			return (_Left==_Right);
		}
		static bool lt(const _Elem&_Left, const _Elem&_Right)
		{
			return (_Left<_Right);
		}
		static int compare(const _Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			for(;
				0<_Count;
				--_Count, ++_First1, ++_First2)if(!eq(*_First1, *_First2))return (lt(*_First1, *_First2)?-1: +1);
			return (0);
		}
		static size_t length(const _Elem*_First)
		{
			size_t _Count;
			for(_Count=0;
				!eq(*_First, _Elem());
				++_First)++_Count;
			return (_Count);
		}
		static _Elem*copy(_Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			return _Copy_s(_First1, _Count, _First2, _Count);
		}
		static _Elem*_Copy_s(_Elem*_First1, size_t _Dest_size, const _Elem*_First2, size_t _Count)
		{

			{
				if(!(_Dest_size>=_Count))
				{
					(void)((!!((("_Dest_size >= _Count", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\iosfwd", 212, 0, L"(\"_Dest_size >= _Count\", 0)"))||(__debugbreak(), 0));
					::_invalid_parameter(L"_Dest_size >= _Count", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\iosfwd", 212, 0);
					return (0);
				}
			};
			_Elem*_Next=_First1;
			for(;
				0<_Count;
				--_Count, ++_Next, ++_First2)assign(*_Next, *_First2);
			return (_First1);
		}
		static const _Elem*find(const _Elem*_First, size_t _Count, const _Elem&_Ch)
		{
			for(;
				0<_Count;
				--_Count, ++_First)if(eq(*_First, _Ch))return (_First);
			return (0);
		}
		static _Elem*move(_Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			return _Move_s(_First1, _Count, _First2, _Count);
		}
		static _Elem*_Move_s(_Elem*_First1, size_t _Dest_size, const _Elem*_First2, size_t _Count)
		{

			{
				if(!(_Dest_size>=_Count))
				{
					(void)((!!((("_Dest_size >= _Count", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\iosfwd", 242, 0, L"(\"_Dest_size >= _Count\", 0)"))||(__debugbreak(), 0));
					::_invalid_parameter(L"_Dest_size >= _Count", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\iosfwd", 242, 0);
					return (0);
				}
			};
			_Elem*_Next=_First1;
			if(_First2<_Next&&_Next<_First2+_Count)for(_Next+=_Count, _First2+=_Count;
			0<_Count;
			--_Count)assign(*--_Next, *--_First2);
			else for(;
			0<_Count;
			--_Count, ++_Next, ++_First2)assign(*_Next, *_First2);
			return (_First1);
		}
		static _Elem*assign(_Elem*_First, size_t _Count, _Elem _Ch)
		{
			_Elem*_Next=_First;
			for(;
				0<_Count;
				--_Count, ++_Next)assign(*_Next, _Ch);
			return (_First);
		}
		static _Elem to_char_type(const int_type&_Meta)
		{
			return ((_Elem)_Meta);
		}
		static int_type to_int_type(const _Elem&_Ch)
		{
			return ((int_type)_Ch);
		}
		static bool eq_int_type(const int_type&_Left, const int_type&_Right)
		{
			return (_Left==_Right);
		}
		static int_type eof()
		{
			return ((int_type)(-1));
		}
		static int_type not_eof(const int_type&_Meta)
		{
			return (_Meta!=eof()?(int_type)_Meta: (int_type)!eof());
		}
	};
	template<>
	struct char_traits<wchar_t>: public _Char_traits_base
	{
		typedef wchar_t _Elem;
		typedef _Elem char_type;
		typedef wint_t int_type;
		typedef streampos pos_type;
		typedef streamoff off_type;
		typedef _Mbstatet state_type;
		static void assign(_Elem&_Left, const _Elem&_Right)
		{
			_Left=_Right;
		}
		static bool eq(const _Elem&_Left, const _Elem&_Right)
		{
			return (_Left==_Right);
		}
		static bool lt(const _Elem&_Left, const _Elem&_Right)
		{
			return (_Left<_Right);
		}
		static int compare(const _Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			return (::wmemcmp(_First1, _First2, _Count));
		}
		static size_t length(const _Elem*_First)
		{
			return (::wcslen(_First));
		}
		static _Elem*copy(_Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			return _Copy_s(_First1, _Count, _First2, _Count);
		}
		static _Elem*_Copy_s(_Elem*_First1, size_t _Size_in_words, const _Elem*_First2, size_t _Count)
		{
			::wmemcpy_s((_First1), (_Size_in_words), (_First2), (_Count));
			return _First1;
		}
		static const _Elem*find(const _Elem*_First, size_t _Count, const _Elem&_Ch)
		{
			return ((const _Elem*)::wmemchr(_First, _Ch, _Count));
		}
		static _Elem*move(_Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			return _Move_s(_First1, _Count, _First2, _Count);
		}
		static _Elem*_Move_s(_Elem*_First1, size_t _Size_in_words, const _Elem*_First2, size_t _Count)
		{
			::wmemmove_s((_First1), (_Size_in_words), (_First2), (_Count));
			return (_Elem*)_First1;
		}
		static _Elem*assign(_Elem*_First, size_t _Count, _Elem _Ch)
		{
			return ((_Elem*)::wmemset(_First, _Ch, _Count));
		}
		static _Elem to_char_type(const int_type&_Meta)
		{
			return (_Meta);
		}
		static int_type to_int_type(const _Elem&_Ch)
		{
			return (_Ch);
		}
		static bool eq_int_type(const int_type&_Left, const int_type&_Right)
		{
			return (_Left==_Right);
		}
		static int_type eof()
		{
			return ((wint_t)(0xFFFF));
		}
		static int_type not_eof(const int_type&_Meta)
		{
			return (_Meta!=eof()?_Meta: !eof());
		}
	};
	template<>
	struct char_traits<char>: public _Char_traits_base
	{
		typedef char _Elem;
		typedef _Elem char_type;
		typedef int int_type;
		typedef streampos pos_type;
		typedef streamoff off_type;
		typedef _Mbstatet state_type;
		static void assign(_Elem&_Left, const _Elem&_Right)
		{
			_Left=_Right;
		}
		static bool eq(const _Elem&_Left, const _Elem&_Right)
		{
			return (_Left==_Right);
		}
		static bool lt(const _Elem&_Left, const _Elem&_Right)
		{
			return (_Left<_Right);
		}
		static int compare(const _Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			return (::memcmp(_First1, _First2, _Count));
		}
		static size_t length(const _Elem*_First)
		{
			return (::strlen(_First));
		}
		static _Elem*copy(_Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			return _Copy_s(_First1, _Count, _First2, _Count);
		}
		static _Elem*_Copy_s(_Elem*_First1, size_t _Size_in_bytes, const _Elem*_First2, size_t _Count)
		{
			::memcpy_s((_First1), (_Size_in_bytes), (_First2), (_Count));
			return _First1;
		}
		static const _Elem*find(const _Elem*_First, size_t _Count, const _Elem&_Ch)
		{
			return ((const _Elem*)::memchr(_First, _Ch, _Count));
		}
		static _Elem*move(_Elem*_First1, const _Elem*_First2, size_t _Count)
		{
			return _Move_s(_First1, _Count, _First2, _Count);
		}
		static _Elem*_Move_s(_Elem*_First1, size_t _Size_in_bytes, const _Elem*_First2, size_t _Count)
		{
			::memmove_s((_First1), (_Size_in_bytes), (_First2), (_Count));
			return _First1;
		}
		static _Elem*assign(_Elem*_First, size_t _Count, _Elem _Ch)
		{
			return ((_Elem*)::memset(_First, _Ch, _Count));
		}
		static _Elem to_char_type(const int_type&_Meta)
		{
			return ((_Elem)_Meta);
		}
		static int_type to_int_type(const _Elem&_Ch)
		{
			return ((unsigned char)_Ch);
		}
		static bool eq_int_type(const int_type&_Left, const int_type&_Right)
		{
			return (_Left==_Right);
		}
		static int_type eof()
		{
			return ((-1));
		}
		static int_type not_eof(const int_type&_Meta)
		{
			return (_Meta!=eof()?_Meta: !eof());
		}
	};
	template<class _Traits>
	class _Inherits_from_char_traits_base
	{
		typedef char _True;
		class _False
		{
			char _Dummy[2];
		};
		static _True _Inherits(_Char_traits_base);
		static _False _Inherits(...);
		static _Traits _Make_traits();
	public:
		enum
		{
			_Exists=sizeof(_Inherits(_Make_traits()))==sizeof(_True)
		};
	};
	template<class _Traits, bool _Inherits_from_char_traits_base>
	class _Char_traits_category_helper
	{
	public:
		typedef _Unsecure_char_traits_tag _Secure_char_traits;
	};
	template<class _Elem>
	class _Char_traits_category_helper<char_traits<_Elem>, true>
	{
	public:
		typedef _Secure_char_traits_tag _Secure_char_traits;
	};
	template<class _Traits>
	class _Char_traits_category_helper<_Traits, true>
	{
	public:
		typedef typename _Traits::_Secure_char_traits _Secure_char_traits;
	};
	template<class _Traits>
	class _Char_traits_category
	{
	public:
		typedef typename _Char_traits_category_helper<_Traits, _Inherits_from_char_traits_base<_Traits>::_Exists>::_Secure_char_traits _Secure_char_traits;
	};
	template<class _Traits>
	inline typename _Char_traits_category<_Traits>::_Secure_char_traits _Char_traits_cat()
	{
		typename _Char_traits_category<_Traits>::_Secure_char_traits _Secure;
		return (_Secure);
	}
	namespace _Traits_helper
	{
		template<class _Traits>
		inline typename _Traits::char_type*copy_s(typename _Traits::char_type*_First1, size_t _Size, const typename _Traits::char_type*_First2, size_t _Count)
		{
			return copy_s<_Traits>(_First1, _Size, _First2, _Count, _Char_traits_cat<_Traits>());
		}
		template<class _Traits>
		inline typename _Traits::char_type*copy_s(typename _Traits::char_type*_First1, size_t _Size, const typename _Traits::char_type*_First2, size_t _Count, _Secure_char_traits_tag)
		{
			return _Traits::_Copy_s(_First1, _Size, _First2, _Count);
		}
		template<class _Traits>
		inline typename _Traits::char_type*copy_s(typename _Traits::char_type*_First1, size_t _Size, const typename _Traits::char_type*_First2, size_t _Count, _Unsecure_char_traits_tag)
		{
			return _Traits::copy(_First1, _First2, _Count);
		}
		template<class _Traits>
		inline typename _Traits::char_type*move_s(typename _Traits::char_type*_First1, size_t _Size, const typename _Traits::char_type*_First2, size_t _Count)
		{
			return move_s<_Traits>(_First1, _Size, _First2, _Count, _Char_traits_cat<_Traits>());
		}
		template<class _Traits>
		inline typename _Traits::char_type*move_s(typename _Traits::char_type*_First1, size_t _Size, const typename _Traits::char_type*_First2, size_t _Count, _Secure_char_traits_tag)
		{
			return _Traits::_Move_s(_First1, _Size, _First2, _Count);
		}
		template<class _Traits>
		inline typename _Traits::char_type*move_s(typename _Traits::char_type*_First1, size_t _Size, const typename _Traits::char_type*_First2, size_t _Count, _Unsecure_char_traits_tag)
		{
			return _Traits::move(_First1, _First2, _Count);
		}
	}
	template<class _Ty>
	class allocator;
	class ios_base;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_ios;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class istreambuf_iterator;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class ostreambuf_iterator;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_streambuf;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_istream;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_ostream;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_iostream;
	template<class _Elem, class _Traits=char_traits<_Elem>, class _Alloc=allocator<_Elem> >
	class basic_stringbuf;
	template<class _Elem, class _Traits=char_traits<_Elem>, class _Alloc=allocator<_Elem> >
	class basic_istringstream;
	template<class _Elem, class _Traits=char_traits<_Elem>, class _Alloc=allocator<_Elem> >
	class basic_ostringstream;
	template<class _Elem, class _Traits=char_traits<_Elem>, class _Alloc=allocator<_Elem> >
	class basic_stringstream;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_filebuf;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_ifstream;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_ofstream;
	template<class _Elem, class _Traits=char_traits<_Elem> >
	class basic_fstream;
	typedef basic_ios<char, char_traits<char> >ios;
	typedef basic_streambuf<char, char_traits<char> >streambuf;
	typedef basic_istream<char, char_traits<char> >istream;
	typedef basic_ostream<char, char_traits<char> >ostream;
	typedef basic_iostream<char, char_traits<char> >iostream;
	typedef basic_stringbuf<char, char_traits<char>, allocator<char> >stringbuf;
	typedef basic_istringstream<char, char_traits<char>, allocator<char> >istringstream;
	typedef basic_ostringstream<char, char_traits<char>, allocator<char> >ostringstream;
	typedef basic_stringstream<char, char_traits<char>, allocator<char> >stringstream;
	typedef basic_filebuf<char, char_traits<char> >filebuf;
	typedef basic_ifstream<char, char_traits<char> >ifstream;
	typedef basic_ofstream<char, char_traits<char> >ofstream;
	typedef basic_fstream<char, char_traits<char> >fstream;
	typedef basic_ios<wchar_t, char_traits<wchar_t> >wios;
	typedef basic_streambuf<wchar_t, char_traits<wchar_t> >wstreambuf;
	typedef basic_istream<wchar_t, char_traits<wchar_t> >wistream;
	typedef basic_ostream<wchar_t, char_traits<wchar_t> >wostream;
	typedef basic_iostream<wchar_t, char_traits<wchar_t> >wiostream;
	typedef basic_stringbuf<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >wstringbuf;
	typedef basic_istringstream<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >wistringstream;
	typedef basic_ostringstream<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >wostringstream;
	typedef basic_stringstream<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >wstringstream;
	typedef basic_filebuf<wchar_t, char_traits<wchar_t> >wfilebuf;
	typedef basic_ifstream<wchar_t, char_traits<wchar_t> >wifstream;
	typedef basic_ofstream<wchar_t, char_traits<wchar_t> >wofstream;
	typedef basic_fstream<wchar_t, char_traits<wchar_t> >wfstream;
}
namespace std
{
	template<class _Ty>
	inline void swap(_Ty&_Left, _Ty&_Right)
	{
		_Ty _Tmp=_Left;
		_Left=_Right, _Right=_Tmp;
	}
	template<class _Ty1, class _Ty2>
	struct pair
	{
		typedef pair<_Ty1, _Ty2>_Myt;
		typedef _Ty1 first_type;
		typedef _Ty2 second_type;
		pair(): first(_Ty1()), second(_Ty2())
		{
		}
		pair(const _Ty1&_Val1, const _Ty2&_Val2): first(_Val1), second(_Val2)
		{
		}
		template<class _Other1, class _Other2>
		pair(const pair<_Other1, _Other2>&_Right): first(_Right.first), second(_Right.second)
		{
		}
		void swap(_Myt&_Right)
		{
			std::swap(first, _Right.first);
			std::swap(second, _Right.second);
		}
		_Ty1 first;
		_Ty2 second;
	};
	template<class _Ty1, class _Ty2>
	inline bool operator==(const pair<_Ty1, _Ty2>&_Left, const pair<_Ty1, _Ty2>&_Right)
	{
		return (_Left.first==_Right.first&&_Left.second==_Right.second);
	}
	template<class _Ty1, class _Ty2>
	inline bool operator!=(const pair<_Ty1, _Ty2>&_Left, const pair<_Ty1, _Ty2>&_Right)
	{
		return (!(_Left==_Right));
	}
	template<class _Ty1, class _Ty2>
	inline bool operator<(const pair<_Ty1, _Ty2>&_Left, const pair<_Ty1, _Ty2>&_Right)
	{
		return (_Left.first<_Right.first||!(_Right.first<_Left.first)&&_Left.second<_Right.second);
	}
	template<class _Ty1, class _Ty2>
	inline bool operator>(const pair<_Ty1, _Ty2>&_Left, const pair<_Ty1, _Ty2>&_Right)
	{
		return (_Right<_Left);
	}
	template<class _Ty1, class _Ty2>
	inline bool operator<=(const pair<_Ty1, _Ty2>&_Left, const pair<_Ty1, _Ty2>&_Right)
	{
		return (!(_Right<_Left));
	}
	template<class _Ty1, class _Ty2>
	inline bool operator>=(const pair<_Ty1, _Ty2>&_Left, const pair<_Ty1, _Ty2>&_Right)
	{
		return (!(_Left<_Right));
	}
	template<class _Ty1, class _Ty2>
	inline pair<_Ty1, _Ty2>make_pair(_Ty1 _Val1, _Ty2 _Val2)
	{
		return (pair<_Ty1, _Ty2>(_Val1, _Val2));
	}
	template<class _Ty1, class _Ty2>
	inline void swap(pair<_Ty1, _Ty2>&_Left, pair<_Ty1, _Ty2>&_Right)
	{
		_Left.swap(_Right);
	}
	namespace rel_ops
	{
		template<class _Ty>
		inline bool operator!=(const _Ty&_Left, const _Ty&_Right)
		{
			return (!(_Left==_Right));
		}
		template<class _Ty>
		inline bool operator>(const _Ty&_Left, const _Ty&_Right)
		{
			return (_Right<_Left);
		}
		template<class _Ty>
		inline bool operator<=(const _Ty&_Left, const _Ty&_Right)
		{
			return (!(_Right<_Left));
		}
		template<class _Ty>
		inline bool operator>=(const _Ty&_Left, const _Ty&_Right)
		{
			return (!(_Left<_Right));
		}
	}
}
namespace std
{
	struct _Unchecked_iterator_tag
	{
	};
	struct _Range_checked_iterator_tag
	{
	};
	struct _Unchanged_checked_iterator_base_type_tag
	{
	};
	struct _Different_checked_iterator_base_type_tag
	{
	};
	struct _Undefined_inner_type_tag
	{
	};
	struct _Undefined_move_tag
	{
	};
	struct _Swap_move_tag
	{
	};
	void _Debug_message(const char*, const char*);
	void _Debug_message(const wchar_t*, const wchar_t*, unsigned int line);
	class _Iterator_base;
	class _Container_base
	{
	public:
		friend class _Iterator_base;
		typedef _Undefined_move_tag _Move_category;
		_Container_base(): _Myfirstiter(0)
		{
		}
		_Container_base(const _Container_base&): _Myfirstiter(0)
		{
		}
		_Container_base&operator=(const _Container_base&)
		{
			return (*this);
		}
		~_Container_base()
		{
			_Orphan_all();
		}
		void _Orphan_all()const;
		void _Swap_all(_Container_base&)const;
		_Iterator_base*_Myfirstiter;
	};
	class _Iterator_base
	{
	public:
		typedef _Unchecked_iterator_tag _Checked_iterator_category;
		typedef _Unchanged_checked_iterator_base_type_tag _Checked_iterator_base_type;
		typedef _Undefined_inner_type_tag _Inner_type;
		friend class _Container_base;
		_Iterator_base(): _Mycont(0), _Mynextiter(0)
		{
		}
		_Iterator_base(const _Iterator_base&_Right): _Mycont(0), _Mynextiter(0)
		{
			*this=_Right;
		}
		_Iterator_base&operator=(const _Iterator_base&_Right)
		{
			if(_Mycont!=_Right._Mycont)
			{
				_Lockit _Lock(3);
				_Orphan_me();
				_Adopt(_Right._Mycont);
			}
			return (*this);
		}
		~_Iterator_base()
		{
			_Lockit _Lock(3);
			_Orphan_me();
		}
		void _Adopt(const _Container_base*_Parent)
		{
			if(_Mycont!=_Parent)
			{
				_Lockit _Lock(3);
				_Orphan_me();
				if(_Parent!=0&&_Parent->_Myfirstiter!=((_Iterator_base*)-3))
				{
					_Mynextiter=_Parent->_Myfirstiter;
					((_Container_base*)_Parent)->_Myfirstiter=this;
				}
				_Mycont=_Parent;
			}
		}
		void _Orphan_me()
		{
			if(_Mycont!=0&&_Mycont->_Myfirstiter!=((_Iterator_base*)-3))
			{
				_Iterator_base**_Pnext=(_Iterator_base**)&_Mycont->_Myfirstiter;
				while(*_Pnext!=0&&*_Pnext!=this)_Pnext=&(*_Pnext)->_Mynextiter;
				if(*_Pnext==0)_Debug_message(L"ITERATOR LIST CORRUPTED!", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 178);
				*_Pnext=_Mynextiter;
				_Mycont=0;
			}
		}
		const _Container_base*_Mycont;
		_Iterator_base*_Mynextiter;
	};
	typedef _Iterator_base _Iterator_base_secure;
	inline void _Container_base::_Orphan_all()const
	{
		_Lockit _Lock(3);
		if(_Myfirstiter!=((_Iterator_base*)-3))
		{
			for(_Iterator_base**_Pnext=(_Iterator_base**)&_Myfirstiter;
				*_Pnext!=0;
				*_Pnext=(*_Pnext)->_Mynextiter)(*_Pnext)->_Mycont=0;
			*(_Iterator_base**)&_Myfirstiter=0;
		}
	}
	inline void _Container_base::_Swap_all(_Container_base&_Right)const
	{
		_Lockit _Lock(3);
		_Iterator_base*_Pnext;
		_Iterator_base*_Temp=(_Iterator_base*)_Myfirstiter;
		*(_Iterator_base**)&_Myfirstiter=(_Iterator_base*)_Right._Myfirstiter;
		*(_Iterator_base**)&_Right._Myfirstiter=_Temp;
		if(_Myfirstiter!=((_Iterator_base*)-3))
		{
			for(_Pnext=(_Iterator_base*)_Myfirstiter;
				_Pnext!=0;
				_Pnext=_Pnext->_Mynextiter)_Pnext->_Mycont=this;
		}
		if(_Right._Myfirstiter!=((_Iterator_base*)-3))
		{
			for(_Pnext=(_Iterator_base*)_Right._Myfirstiter;
				_Pnext!=0;
				_Pnext=_Pnext->_Mynextiter)_Pnext->_Mycont=&_Right;
		}
	}
	template<class _Iter>
	inline const _Container_base*_Debug_get_cont(const _Iter&)
	{
		return (0);
	}
	inline const _Container_base*_Debug_get_cont(const _Iterator_base&_Where)
	{
		return (_Where._Mycont);
	}
	template<class _Ty1, class _Ty2>
	inline bool _Debug_lt(const _Ty1&_Left, const _Ty2&_Right, const wchar_t*_Where, unsigned int _Line)
	{
		if(!(_Left<_Right))return (false);
		else if(_Right<_Left)_Debug_message(L"invalid operator<", _Where, _Line);
		return (true);
	}
	template<class _Ty1, class _Ty2>
	inline bool _Debug_lt(const _Ty1&_Left, _Ty2&_Right, const wchar_t*_Where, unsigned int _Line)
	{
		if(!(_Left<_Right))return (false);
		else if(_Right<_Left)_Debug_message(L"invalid operator<", _Where, _Line);
		return (true);
	}
	template<class _Ty1, class _Ty2>
	inline bool _Debug_lt(_Ty1&_Left, const _Ty2&_Right, const wchar_t*_Where, unsigned int _Line)
	{
		if(!(_Left<_Right))return (false);
		else if(_Right<_Left)_Debug_message(L"invalid operator<", _Where, _Line);
		return (true);
	}
	template<class _Ty1, class _Ty2>
	inline bool _Debug_lt(_Ty1&_Left, _Ty2&_Right, const wchar_t*_Where, unsigned int _Line)
	{
		if(!(_Left<_Right))return (false);
		else if(_Right<_Left)_Debug_message(L"invalid operator<", _Where, _Line);
		return (true);
	}
	template<class _Pr, class _Ty1, class _Ty2>
	inline bool _Debug_lt_pred(_Pr _Pred, const _Ty1&_Left, const _Ty2&_Right, const wchar_t*_Where, unsigned int _Line)
	{
		if(!_Pred(_Left, _Right))return (false);
		else if(_Pred(_Right, _Left))_Debug_message(L"invalid operator<", _Where, _Line);
		return (true);
	}
	template<class _Pr, class _Ty1, class _Ty2>
	inline bool _Debug_lt_pred(_Pr _Pred, const _Ty1&_Left, _Ty2&_Right, const wchar_t*_Where, unsigned int _Line)
	{
		if(!_Pred(_Left, _Right))return (false);
		else if(_Pred(_Right, _Left))_Debug_message(L"invalid operator<", _Where, _Line);
		return (true);
	}
	template<class _Pr, class _Ty1, class _Ty2>
	inline bool _Debug_lt_pred(_Pr _Pred, _Ty1&_Left, const _Ty2&_Right, const wchar_t*_Where, unsigned int _Line)
	{
		if(!_Pred(_Left, _Right))return (false);
		else if(_Pred(_Right, _Left))_Debug_message(L"invalid operator<", _Where, _Line);
		return (true);
	}
	template<class _Pr, class _Ty1, class _Ty2>
	inline bool _Debug_lt_pred(_Pr _Pred, _Ty1&_Left, _Ty2&_Right, const wchar_t*_Where, unsigned int _Line)
	{
		if(!_Pred(_Left, _Right))return (false);
		else if(_Pred(_Right, _Left))_Debug_message(L"invalid operator<", _Where, _Line);
		return (true);
	}
	struct input_iterator_tag
	{
	};
	struct output_iterator_tag
	{
	};
	struct forward_iterator_tag: public input_iterator_tag
	{
	};
	struct bidirectional_iterator_tag: public forward_iterator_tag
	{
	};
	struct random_access_iterator_tag: public bidirectional_iterator_tag
	{
	};
	struct _Int_iterator_tag
	{
	};
	struct _Float_iterator_tag
	{
	};
	struct _Nonscalar_ptr_iterator_tag
	{
	};
	struct _Scalar_ptr_iterator_tag
	{
	};
	template<class _Category, class _Ty, class _Diff=ptrdiff_t, class _Pointer=_Ty*, class _Reference=_Ty&>
	struct iterator: public _Iterator_base
	{
		typedef _Category iterator_category;
		typedef _Ty value_type;
		typedef _Diff difference_type;
		typedef _Diff distance_type;
		typedef _Pointer pointer;
		typedef _Reference reference;
	};
	template<class _Category, class _Ty, class _Diff=ptrdiff_t, class _Pointer=_Ty*, class _Reference=_Ty&, class _Base_class=_Iterator_base>
	struct _Iterator_with_base: public _Base_class
	{
		typedef _Category iterator_category;
		typedef _Ty value_type;
		typedef _Diff difference_type;
		typedef _Diff distance_type;
		typedef _Pointer pointer;
		typedef _Reference reference;
	};
	template<class _Ty, class _Diff, class _Pointer, class _Reference>
	struct _Bidit: public _Iterator_base
	{
		typedef bidirectional_iterator_tag iterator_category;
		typedef _Ty value_type;
		typedef _Diff difference_type;
		typedef _Diff distance_type;
		typedef _Pointer pointer;
		typedef _Reference reference;
	};
	template<class _Ty, class _Diff, class _Pointer, class _Reference>
	struct _Ranit: public _Iterator_base
	{
		typedef random_access_iterator_tag iterator_category;
		typedef _Ty value_type;
		typedef _Diff difference_type;
		typedef _Diff distance_type;
		typedef _Pointer pointer;
		typedef _Reference reference;
	};
	template<class _Ty, class _Diff, class _Pointer, class _Reference, class _Base_class>
	struct _Ranit_base: public _Base_class
	{
		typedef random_access_iterator_tag iterator_category;
		typedef _Ty value_type;
		typedef _Diff difference_type;
		typedef _Diff distance_type;
		typedef _Pointer pointer;
		typedef _Reference reference;
	};
	struct _Outit: public iterator<output_iterator_tag, void, void, void, void>
	{
	};
	template<class _Base_class>
	struct _Outit_with_base: public _Iterator_with_base<output_iterator_tag, void, void, void, void, _Base_class>
	{
	};
	template<class _Iter>
	struct iterator_traits
	{
		typedef typename _Iter::iterator_category iterator_category;
		typedef typename _Iter::value_type value_type;
		typedef typename _Iter::difference_type difference_type;
		typedef difference_type distance_type;
		typedef typename _Iter::pointer pointer;
		typedef typename _Iter::reference reference;
	};
	template<class _Ty>
	struct iterator_traits<_Ty*>
	{
		typedef random_access_iterator_tag iterator_category;
		typedef _Ty value_type;
		typedef ptrdiff_t difference_type;
		typedef ptrdiff_t distance_type;
		typedef _Ty*pointer;
		typedef _Ty&reference;
	};
	template<class _Ty>
	struct iterator_traits<const _Ty*>
	{
		typedef random_access_iterator_tag iterator_category;
		typedef _Ty value_type;
		typedef ptrdiff_t difference_type;
		typedef ptrdiff_t distance_type;
		typedef const _Ty*pointer;
		typedef const _Ty&reference;
	};
	template<>
	struct iterator_traits<_Bool>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef _Bool value_type;
		typedef _Bool difference_type;
		typedef _Bool distance_type;
		typedef _Bool*pointer;
		typedef _Bool&reference;
	};
	template<>
	struct iterator_traits<char>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef char value_type;
		typedef char difference_type;
		typedef char distance_type;
		typedef char*pointer;
		typedef char&reference;
	};
	template<>
	struct iterator_traits<signed char>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef signed char value_type;
		typedef signed char difference_type;
		typedef signed char distance_type;
		typedef signed char*pointer;
		typedef signed char&reference;
	};
	template<>
	struct iterator_traits<unsigned char>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef unsigned char value_type;
		typedef unsigned char difference_type;
		typedef unsigned char distance_type;
		typedef unsigned char*pointer;
		typedef unsigned char&reference;
	};
	template<>
	struct iterator_traits<wchar_t>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef wchar_t value_type;
		typedef wchar_t difference_type;
		typedef wchar_t distance_type;
		typedef wchar_t*pointer;
		typedef wchar_t&reference;
	};
	template<>
	struct iterator_traits<short>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef short value_type;
		typedef short difference_type;
		typedef short distance_type;
		typedef short*pointer;
		typedef short&reference;
	};
	template<>
	struct iterator_traits<unsigned short>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef unsigned short value_type;
		typedef unsigned short difference_type;
		typedef unsigned short distance_type;
		typedef unsigned short*pointer;
		typedef unsigned short&reference;
	};
	template<>
	struct iterator_traits<int>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef int value_type;
		typedef int difference_type;
		typedef int distance_type;
		typedef int*pointer;
		typedef int&reference;
	};
	template<>
	struct iterator_traits<unsigned int>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef unsigned int value_type;
		typedef unsigned int difference_type;
		typedef unsigned int distance_type;
		typedef unsigned int*pointer;
		typedef unsigned int&reference;
	};
	template<>
	struct iterator_traits<long>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef long value_type;
		typedef long difference_type;
		typedef long distance_type;
		typedef long*pointer;
		typedef long&reference;
	};
	template<>
	struct iterator_traits<unsigned long>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef unsigned long value_type;
		typedef unsigned long difference_type;
		typedef unsigned long distance_type;
		typedef unsigned long*pointer;
		typedef unsigned long&reference;
	};
	template<>
	struct iterator_traits<double>
	{
		typedef _Float_iterator_tag iterator_category;
		typedef double value_type;
		typedef double difference_type;
		typedef double distance_type;
		typedef double*pointer;
		typedef double&reference;
	};
	template<>
	struct iterator_traits<float>
	{
		typedef _Float_iterator_tag iterator_category;
		typedef float value_type;
		typedef float difference_type;
		typedef float distance_type;
		typedef float*pointer;
		typedef float&reference;
	};
	template<>
	struct iterator_traits<__int64>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef __int64 value_type;
		typedef __int64 difference_type;
		typedef __int64 distance_type;
		typedef __int64*pointer;
		typedef __int64&reference;
	};
	template<>
	struct iterator_traits<unsigned __int64>
	{
		typedef _Int_iterator_tag iterator_category;
		typedef unsigned __int64 value_type;
		typedef unsigned __int64 difference_type;
		typedef unsigned __int64 distance_type;
		typedef unsigned __int64*pointer;
		typedef unsigned __int64&reference;
	};
	template<class _Iter>
	inline typename iterator_traits<_Iter>::iterator_category _Iter_cat(const _Iter&)
	{
		typename iterator_traits<_Iter>::iterator_category _Cat;
		return (_Cat);
	}
	template<class _Cat1, class _Cat2>
	class _Iter_random_helper
	{
	public:
		typedef forward_iterator_tag _Iter_random_cat;
	};
	template<>
	class _Iter_random_helper<random_access_iterator_tag, random_access_iterator_tag>
	{
	public:
		typedef random_access_iterator_tag _Iter_random_cat;
	};
	template<class _Cat1, class _Cat2, class _Cat3>
	class _Iter_random_helper3
	{
	public:
		typedef forward_iterator_tag _Iter_random_cat;
	};
	template<>
	class _Iter_random_helper3<random_access_iterator_tag, random_access_iterator_tag, random_access_iterator_tag>
	{
	public:
		typedef random_access_iterator_tag _Iter_random_cat;
	};
	template<class _Iter1, class _Iter2>
	inline typename _Iter_random_helper<typename iterator_traits<_Iter1>::iterator_category, typename iterator_traits<_Iter2>::iterator_category>::_Iter_random_cat _Iter_random(const _Iter1&, const _Iter2&)
	{
		typename _Iter_random_helper<iterator_traits<_Iter1>::iterator_category, iterator_traits<_Iter2>::iterator_category>::_Iter_random_cat _Cat;
		return (_Cat);
	}
	template<class _Iter1, class _Iter2, class _Iter3>
	inline typename _Iter_random_helper3<typename iterator_traits<_Iter1>::iterator_category, typename iterator_traits<_Iter2>::iterator_category, typename iterator_traits<_Iter3>::iterator_category>::_Iter_random_cat _Iter_random(const _Iter1&, const _Iter2&, const _Iter3&)
	{
		typename _Iter_random_helper3<iterator_traits<_Iter1>::iterator_category, iterator_traits<_Iter2>::iterator_category, iterator_traits<_Iter3>::iterator_category>::_Iter_random_cat _Cat;
		return (_Cat);
	}
	template<bool _Cond, class _Ty1, class _Ty2>
	class _If
	{
	public:
		typedef _Ty2 _Result;
	};
	template<class _Ty1, class _Ty2>
	class _If<true, _Ty1, _Ty2>
	{
	public:
		typedef _Ty1 _Result;
	};
	template<bool _Secure_validation>
	class _Secure_validation_helper
	{
	public:
		typedef _Unchecked_iterator_tag _Checked_iterator_category;
	};
	template<>
	class _Secure_validation_helper<true>
	{
	public:
		typedef _Range_checked_iterator_tag _Checked_iterator_category;
	};
	template<class _Iter, bool _Inherits_from_iterator_base>
	class _Checked_iterator_category_helper
	{
	public:
		typedef _Unchecked_iterator_tag _Checked_cat;
	};
	template<class _Iter>
	class _Checked_iterator_category_helper<_Iter, true>
	{
	public:
		typedef typename _Iter::_Checked_iterator_category _Checked_cat;
	};
	template<class _Iter>
	class _Checked_iterator_category
	{
	public:
		typedef typename _Checked_iterator_category_helper<_Iter, 0>::_Checked_cat _Checked_cat;
	};
	template<class _Iter>
	inline typename _Checked_iterator_category<_Iter>::_Checked_cat _Checked_cat(const _Iter&)
	{
		typename _Checked_iterator_category<_Iter>::_Checked_cat _Cat;
		return (_Cat);
	}
	template<class _Iter, bool _Inherits_from_iterator_base>
	class _Checked_iterator_base_helper2
	{
	public:
		typedef _Unchanged_checked_iterator_base_type_tag _Checked_iterator_base_type;
	};
	template<class _Iter>
	class _Checked_iterator_base_helper2<_Iter, true>
	{
	public:
		typedef typename _Iter::_Checked_iterator_base_type _Checked_iterator_base_type;
	};
	template<class _Iter, class _Base_type>
	class _Checked_iterator_base_helper1
	{
	public:
		typedef _Different_checked_iterator_base_type_tag _Base_type_tag;
		typedef _Base_type _Checked_iterator_base_type;
	};
	template<class _Iter>
	class _Checked_iterator_base_helper1<_Iter, _Unchanged_checked_iterator_base_type_tag>
	{
	public:
		typedef _Unchanged_checked_iterator_base_type_tag _Base_type_tag;
		typedef _Iter _Checked_iterator_base_type;
	};
	template<class _Iter>
	class _Checked_iterator_base_helper
	{
	public:
		typedef _Checked_iterator_base_helper2<_Iter, 0>_Base_helper2;
		typedef _Checked_iterator_base_helper1<_Iter, typename _Base_helper2::_Checked_iterator_base_type>_Base_helper1;
		typedef typename _Base_helper1::_Base_type_tag _Checked_iterator_base_type_tag;
		typedef typename _Base_helper1::_Checked_iterator_base_type _Checked_iterator_base_type;
	};
	template<class _Iter, class _Base_tag>
	inline typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type _Checked_base(const _Iter&_It, _Base_tag)
	{
		return _It._Checked_iterator_base();
	}
	template<class _Iter>
	inline typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type _Checked_base(const _Iter&_It, _Unchanged_checked_iterator_base_type_tag)
	{
		return _It;
	}
	template<class _Iter, class _Base_tag>
	inline typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type _Checked_base(_Iter&_It, _Base_tag)
	{
		return _It._Checked_iterator_base();
	}
	template<class _Iter>
	inline typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type _Checked_base(_Iter&_It, _Unchanged_checked_iterator_base_type_tag)
	{
		return _It;
	}
	template<class _Iter>
	inline typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type _Checked_base(const _Iter&_It)
	{
		typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type_tag _Base_tag;
		return _Checked_base(_It, _Base_tag);
	}
	template<class _Iter>
	inline typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type _Checked_base(_Iter&_It)
	{
		typename _Checked_iterator_base_helper<_Iter>::_Checked_iterator_base_type_tag _Base_tag;
		return _Checked_base(_It, _Base_tag);
	}
	template<class _DstIter, class _BaseIter>
	inline void _Checked_assign_from_base(_DstIter&_Dest, const _BaseIter&_Src)
	{
		_Dest._Checked_iterator_assign_from_base(_Src);
	}
	template<class _Iter>
	inline void _Checked_assign_from_base(_Iter&_Dest, const _Iter&_Src)
	{
		_Dest=_Src;
	}
	template<class _Value>
	class _Move_operation_category
	{
	public:
		typedef _Undefined_move_tag _Move_cat;
	};
	template<class _Iter>
	inline typename _Move_operation_category<typename iterator_traits<_Iter>::value_type>::_Move_cat _Move_cat(const _Iter&)
	{
		typename _Move_operation_category<typename iterator_traits<_Iter>::value_type>::_Move_cat _Cat;
		return (_Cat);
	}
	template<class _T1, class _T2, class _Checked_Cat1, class _Checked_Cat2>
	struct _Ptr_cat_with_checked_cat_helper
	{
		typedef _Nonscalar_ptr_iterator_tag _Ptr_cat;
	};
	template<class _T1, class _T2>
	struct _Ptr_cat_helper
	{
		typedef typename _Ptr_cat_with_checked_cat_helper<_T1, _T2, typename _Checked_iterator_category<_T1>::_Checked_cat, typename _Checked_iterator_category<_T2>::_Checked_cat>::_Ptr_cat _Ptr_cat;
	};
	template<class _T1, class _T2, class _Checked_Cat1>
	struct _Ptr_cat_with_checked_cat_helper<_T1, _T2, _Checked_Cat1, _Range_checked_iterator_tag>
	{
		typedef typename _Ptr_cat_helper<_T1, typename _T2::_Inner_type>::_Ptr_cat _Ptr_cat;
	};
	template<class _T1, class _T2, class _Checked_Cat2>
	struct _Ptr_cat_with_checked_cat_helper<_T1, _T2, _Range_checked_iterator_tag, _Checked_Cat2>
	{
		typedef typename _Ptr_cat_helper<typename _T1::_Inner_type, _T2>::_Ptr_cat _Ptr_cat;
	};
	template<class _T1, class _T2>
	struct _Ptr_cat_with_checked_cat_helper<_T1, _T2, _Range_checked_iterator_tag, _Range_checked_iterator_tag>
	{
		typedef typename _Ptr_cat_helper<typename _T1::_Inner_type, typename _T2::_Inner_type>::_Ptr_cat _Ptr_cat;
	};
	template<class _T1>
	struct _Ptr_cat_helper<_T1, _Undefined_inner_type_tag>
	{
		typedef _Nonscalar_ptr_iterator_tag _Ptr_cat;
	};
	template<class _T2>
	struct _Ptr_cat_helper<_Undefined_inner_type_tag, _T2>
	{
		typedef _Nonscalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<_Undefined_inner_type_tag, _Undefined_inner_type_tag>
	{
		typedef _Nonscalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<_Bool*, _Bool*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const _Bool*, _Bool*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<char*, char*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const char*, char*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<signed char*, signed char*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const signed char*, signed char*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<unsigned char*, unsigned char*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const unsigned char*, unsigned char*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<wchar_t*, wchar_t*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const wchar_t*, wchar_t*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<short*, short*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const short*, short*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<unsigned short*, unsigned short*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const unsigned short*, unsigned short*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<int*, int*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const int*, int*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<unsigned int*, unsigned int*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const unsigned int*, unsigned int*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<long*, long*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const long*, long*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<unsigned long*, unsigned long*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const unsigned long*, unsigned long*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<float*, float*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const float*, float*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<double*, double*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const double*, double*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<long double*, long double*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const long double*, long double*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<__int64*, __int64*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const __int64*, __int64*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<unsigned __int64*, unsigned __int64*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<>
	struct _Ptr_cat_helper<const unsigned __int64*, unsigned __int64*>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<class _Ty>
	struct _Ptr_cat_helper<_Ty**, _Ty**>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<class _Ty>
	struct _Ptr_cat_helper<_Ty**, const _Ty**>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<class _Ty>
	struct _Ptr_cat_helper<_Ty*const*, _Ty**>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<class _Ty>
	struct _Ptr_cat_helper<_Ty*const*, const _Ty**>
	{
		typedef _Scalar_ptr_iterator_tag _Ptr_cat;
	};
	template<class _T1, class _T2>
	inline typename _Ptr_cat_helper<_T1, _T2>::_Ptr_cat _Ptr_cat(_T1&, _T2&)
	{
		typename _Ptr_cat_helper<_T1, _T2>::_Ptr_cat _Cat;
		return (_Cat);
	}
	template<class _InIt>
	inline void _Debug_pointer(_InIt&, const wchar_t*, unsigned int)
	{
	}
	template<class _Ty>
	inline void _Debug_pointer(const _Ty*_First, const wchar_t*_File, unsigned int _Line)
	{
		if(_First==0)_Debug_message(L"invalid null pointer", _File, _Line);
	}
	template<class _Ty>
	inline void _Debug_pointer(_Ty*_First, const wchar_t*_File, unsigned int _Line)
	{
		if(_First==0)_Debug_message(L"invalid null pointer", _File, _Line);
	}
	template<class _InIt>
	inline void _Debug_range2(_InIt, _InIt, const wchar_t*, unsigned int, input_iterator_tag)
	{
	}
	template<class _RanIt>
	inline void _Debug_range2(_RanIt _First, _RanIt _Last, const wchar_t*_File, unsigned int _Line, random_access_iterator_tag)
	{
		if(_First!=_Last)
		{
			_Debug_pointer(_First, _File, _Line);
			_Debug_pointer(_Last, _File, _Line);
			if(_Last<_First)_Debug_message(L"invalid iterator range", _File, _Line);
		}
	}
	template<class _InIt>
	inline void _Debug_range(_InIt _First, _InIt _Last, const wchar_t*_File, unsigned int _Line)
	{
		_Debug_range2(_First, _Last, _File, _Line, _Iter_cat(_First));
	}
	template<class _InIt>
	inline void _Debug_order2(_InIt _First, _InIt _Last, const wchar_t*_File, unsigned int _Line, input_iterator_tag)
	{
	}
	template<class _FwdIt>
	inline void _Debug_order2(_FwdIt _First, _FwdIt _Last, const wchar_t*_File, unsigned int _Line, forward_iterator_tag)
	{
		if(_First!=_Last)for(_FwdIt _Next=_First;
		++_Next!=_Last;
		++_First)if(_Debug_lt(*_Next, *_First, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 1384))_Debug_message(L"sequence not ordered", _File, _Line);
	}
	template<class _InIt>
	inline void _Debug_order(_InIt _First, _InIt _Last, const wchar_t*_File, unsigned int _Line)
	{
		_Debug_range(_First, _Last, _File, _Line);
		_Debug_order2(_First, _Last, _File, _Line, _Iter_cat(_First));
	}
	template<class _InIt, class _Pr>
	inline void _Debug_order2(_InIt _First, _InIt _Last, _Pr _Pred, const wchar_t*_File, unsigned int _Line, input_iterator_tag)
	{
	}
	template<class _FwdIt, class _Pr>
	inline void _Debug_order2(_FwdIt _First, _FwdIt _Last, _Pr _Pred, const wchar_t*_File, unsigned int _Line, forward_iterator_tag)
	{
		if(_First!=_Last)for(_FwdIt _Next=_First;
		++_Next!=_Last;
		++_First)if(_Debug_lt_pred(_Pred, *_Next, *_First, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 1411))_Debug_message(L"sequence not ordered", _File, _Line);
	}
	template<class _InIt, class _Pr>
	inline void _Debug_order(_InIt _First, _InIt _Last, _Pr _Pred, const wchar_t*_File, unsigned int _Line)
	{
		_Debug_range(_First, _Last, _File, _Line);
		_Debug_pointer(_Pred, _File, _Line);
		_Debug_order2(_First, _Last, _Pred, _File, _Line, _Iter_cat(_First));
	}
	template<class _InIt>
	inline void _Debug_order_single2(_InIt _First, _InIt _Last, bool _IsFirstIteration, const wchar_t*_File, unsigned int _Line, input_iterator_tag)
	{
	}
	template<class _FwdIt>
	inline void _Debug_order_single2(_FwdIt _First, _FwdIt _Last, bool _IsFirstIteration, const wchar_t*_File, unsigned int _Line, forward_iterator_tag)
	{
		if(_First!=_Last)
		{
			_FwdIt _Next=_First;
			if(++_Next!=_Last)if(_Debug_lt(*_Next, *_First, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 1440))_Debug_message(L"sequence not ordered", _File, _Line);
		}
	}
	template<class _InIt>
	inline void _Debug_order_single(_InIt _First, _InIt _Last, bool _IsFirstIteration, const wchar_t*_File, unsigned int _Line)
	{
		_Debug_order_single2(_First, _Last, _IsFirstIteration, _File, _Line, _Iter_cat(_First));
	}
	template<class _InIt, class _Pr>
	inline void _Debug_order_single2(_InIt _First, _InIt _Last, _Pr _Pred, bool _IsFirstIteration, const wchar_t*_File, unsigned int _Line, input_iterator_tag)
	{
	}
	template<class _FwdIt, class _Pr>
	inline void _Debug_order_single2(_FwdIt _First, _FwdIt _Last, _Pr _Pred, bool _IsFirstIteration, const wchar_t*_File, unsigned int _Line, forward_iterator_tag)
	{
		if(_First!=_Last)
		{
			_FwdIt _Next=_First;
			if(++_Next!=_Last)if(_Debug_lt_pred(_Pred, *_Next, *_First, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 1469))_Debug_message(L"sequence not ordered", _File, _Line);
		}
	}
	template<class _InIt, class _Pr>
	inline void _Debug_order_single(_InIt _First, _InIt _Last, _Pr _Pred, bool _IsFirstIteration, const wchar_t*_File, unsigned int _Line)
	{
		_Debug_order_single2(_First, _Last, _Pred, _IsFirstIteration, _File, _Line, _Iter_cat(_First));
	}
	template<class _Iter>
	inline typename iterator_traits<_Iter>::value_type*_Val_type(_Iter)
	{
		return (0);
	}
	template<class _InIt, class _Diff>
	inline void _Advance(_InIt&_Where, _Diff _Off, input_iterator_tag)
	{
		for(;
			0<_Off;
			--_Off)++_Where;
	}
	template<class _InIt, class _Diff>
	inline void advance(_InIt&_Where, _Diff _Off)
	{
		_Advance(_Where, _Off, _Iter_cat(_Where));
	}
	template<class _FI, class _Diff>
	inline void _Advance(_FI&_Where, _Diff _Off, forward_iterator_tag)
	{
		for(;
			0<_Off;
			--_Off)++_Where;
	}
	template<class _BI, class _Diff>
	inline void _Advance(_BI&_Where, _Diff _Off, bidirectional_iterator_tag)
	{
		for(;
			0<_Off;
			--_Off)++_Where;
		for(;
			_Off<0;
			++_Off)--_Where;
	}
	template<class _RI, class _Diff>
	inline void _Advance(_RI&_Where, _Diff _Off, random_access_iterator_tag)
	{
		_Where+=_Off;
	}
	template<class _Iter>
	inline typename iterator_traits<_Iter>::difference_type*_Dist_type(_Iter)
	{
		return (0);
	}
	template<class _InIt, class _Diff>
	inline void _Distance2(_InIt _First, _InIt _Last, _Diff&_Off, input_iterator_tag)
	{
		for(;
			_First!=_Last;
			++_First)++_Off;
	}
	template<class _FwdIt, class _Diff>
	inline void _Distance2(_FwdIt _First, _FwdIt _Last, _Diff&_Off, forward_iterator_tag)
	{
		for(;
			_First!=_Last;
			++_First)++_Off;
	}
	template<class _BidIt, class _Diff>
	inline void _Distance2(_BidIt _First, _BidIt _Last, _Diff&_Off, bidirectional_iterator_tag)
	{
		for(;
			_First!=_Last;
			++_First)++_Off;
	}
	template<class _RanIt, class _Diff>
	inline void _Distance2(_RanIt _First, _RanIt _Last, _Diff&_Off, random_access_iterator_tag)
	{
		if(_First!=_Last)
		{
			_Debug_pointer(_First, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 1642);
			_Debug_pointer(_Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 1643);
		}
		_Off+=_Last-_First;
	}
	template<class _InIt>
	inline typename iterator_traits<_InIt>::difference_type distance(_InIt _First, _InIt _Last)
	{
		typename iterator_traits<_InIt>::difference_type _Off=0;
		_Distance2(_First, _Last, _Off, _Iter_cat(_First));
		return (_Off);
	}
	template<class _InIt, class _Diff>
	inline void _Distance(_InIt _First, _InIt _Last, _Diff&_Off)
	{
		_Distance2(_First, _Last, _Off, _Iter_cat(_First));
	}
	template<class _RanIt>
	class reverse_iterator: public _Iterator_base_secure
	{
	public:
		typedef reverse_iterator<_RanIt>_Myt;
		typedef typename iterator_traits<_RanIt>::iterator_category iterator_category;
		typedef typename iterator_traits<_RanIt>::value_type value_type;
		typedef typename iterator_traits<_RanIt>::difference_type difference_type;
		typedef typename iterator_traits<_RanIt>::difference_type distance_type;
		typedef typename iterator_traits<_RanIt>::pointer pointer;
		typedef typename iterator_traits<_RanIt>::reference reference;
		typedef _RanIt iterator_type;
		reverse_iterator()
		{
		}
		explicit reverse_iterator(_RanIt _Right): current(_Right)
		{
		}
		template<class _Other>
		reverse_iterator(const reverse_iterator<_Other>&_Right): current(_Right.base())
		{
		}
		_RanIt base()const
		{
			return (current);
		}
		typedef typename::std::_Checked_iterator_category<_RanIt>::_Checked_cat _Checked_iterator_category;
		typedef reverse_iterator<typename _Checked_iterator_base_helper<_RanIt>::_Checked_iterator_base_type>_Checked_iterator_base_type;
		_Checked_iterator_base_type _Checked_iterator_base()const
		{
			typename _Checked_iterator_base_type _Base(::std::_Checked_base(current));
			return _Base;
		}
		void _Checked_iterator_assign_from_base(_Checked_iterator_base_type _Base)
		{
			::std::_Checked_assign_from_base(current, _Base.base());
		}
		reference operator*()const
		{
			_RanIt _Tmp=current;
			return (*--_Tmp);
		}
		pointer operator->()const
		{
			return (&**this);
		}
		_Myt&operator++()
		{
			--current;
			return (*this);
		}
		_Myt operator++(int)
		{
			_Myt _Tmp=*this;
			--current;
			return (_Tmp);
		}
		_Myt&operator--()
		{
			++current;
			return (*this);
		}
		_Myt operator--(int)
		{
			_Myt _Tmp=*this;
			++current;
			return (_Tmp);
		}
		template<class _Other>
		bool _Equal(const reverse_iterator<_Other>&_Right)const
		{
			return (current==_Right.base());
		}
		_Myt&operator+=(difference_type _Off)
		{
			current-=_Off;
			return (*this);
		}
		_Myt operator+(difference_type _Off)const
		{
			return (_Myt(current-_Off));
		}
		_Myt&operator-=(difference_type _Off)
		{
			current+=_Off;
			return (*this);
		}
		_Myt operator-(difference_type _Off)const
		{
			return (_Myt(current+_Off));
		}
		reference operator[](difference_type _Off)const
		{
			return (*(*this+_Off));
		}
		template<class _Other>
		bool _Less(const reverse_iterator<_Other>&_Right)const
		{
			return (_Right.base()<current);
		}
		template<class _Other>
		difference_type _Minus(const reverse_iterator<_Other>&_Right)const
		{
			return (_Right.base()-current);
		}
	protected: _RanIt current;
	};
	template<class _RanIt, class _Diff>
	inline reverse_iterator<_RanIt>operator+(_Diff _Off, const reverse_iterator<_RanIt>&_Right)
	{
		return (_Right+_Off);
	}
	template<class _RanIt1, class _RanIt2>
	inline typename reverse_iterator<_RanIt1>::difference_type operator-(const reverse_iterator<_RanIt1>&_Left, const reverse_iterator<_RanIt2>&_Right)
	{
		return (_Left._Minus(_Right));
	}
	template<class _RanIt1, class _RanIt2>
	inline bool operator==(const reverse_iterator<_RanIt1>&_Left, const reverse_iterator<_RanIt2>&_Right)
	{
		return (_Left._Equal(_Right));
	}
	template<class _RanIt1, class _RanIt2>
	inline bool operator!=(const reverse_iterator<_RanIt1>&_Left, const reverse_iterator<_RanIt2>&_Right)
	{
		return (!(_Left==_Right));
	}
	template<class _RanIt1, class _RanIt2>
	inline bool operator<(const reverse_iterator<_RanIt1>&_Left, const reverse_iterator<_RanIt2>&_Right)
	{
		return (_Left._Less(_Right));
	}
	template<class _RanIt1, class _RanIt2>
	inline bool operator>(const reverse_iterator<_RanIt1>&_Left, const reverse_iterator<_RanIt2>&_Right)
	{
		return (_Right<_Left);
	}
	template<class _RanIt1, class _RanIt2>
	inline bool operator<=(const reverse_iterator<_RanIt1>&_Left, const reverse_iterator<_RanIt2>&_Right)
	{
		return (!(_Right<_Left));
	}
	template<class _RanIt1, class _RanIt2>
	inline bool operator>=(const reverse_iterator<_RanIt1>&_Left, const reverse_iterator<_RanIt2>&_Right)
	{
		return (!(_Left<_Right));
	}
	template<class _BidIt, class _Ty, class _Reference=_Ty&, class _Pointer=_Ty*, class _Diff=ptrdiff_t>
	class reverse_bidirectional_iterator: public iterator<bidirectional_iterator_tag, _Ty, _Diff, _Pointer, _Reference>
	{
	public:
		typedef reverse_bidirectional_iterator<_BidIt, _Ty, _Reference, _Pointer, _Diff>_Myt;
		typedef _BidIt iterator_type;
		reverse_bidirectional_iterator()
		{
		}
		explicit reverse_bidirectional_iterator(_BidIt _Right): current(_Right)
		{
		}
		_BidIt base()const
		{
			return (current);
		}
		_Reference operator*()const
		{
			_BidIt _Tmp=current;
			return (*--_Tmp);
		}
		_Pointer operator->()const
		{
			_Reference _Tmp=**this;
			return (&_Tmp);
		}
		_Myt&operator++()
		{
			--current;
			return (*this);
		}
		_Myt operator++(int)
		{
			_Myt _Tmp=*this;
			--current;
			return (_Tmp);
		}
		_Myt&operator--()
		{
			++current;
			return (*this);
		}
		_Myt operator--(int)
		{
			_Myt _Tmp=*this;
			++current;
			return (_Tmp);
		}
		bool operator==(const _Myt&_Right)const
		{
			return (current==_Right.current);
		}
		bool operator!=(const _Myt&_Right)const
		{
			return (!(*this==_Right));
		}
	protected: _BidIt current;
	};
	template<class _BidIt, class _BidIt2=_BidIt>
	class _Revbidit: public iterator<typename iterator_traits<_BidIt>::iterator_category, typename iterator_traits<_BidIt>::value_type, typename iterator_traits<_BidIt>::difference_type, typename iterator_traits<_BidIt>::pointer, typename iterator_traits<_BidIt>::reference>
	{
	public:
		typedef _Revbidit<_BidIt, _BidIt2>_Myt;
		typedef typename iterator_traits<_BidIt>::difference_type _Diff;
		typedef typename iterator_traits<_BidIt>::pointer _Pointer;
		typedef typename iterator_traits<_BidIt>::reference _Reference;
		typedef _BidIt iterator_type;
		_Revbidit()
		{
		}
		explicit _Revbidit(_BidIt _Right): current(_Right)
		{
		}
		_Revbidit(const _Revbidit<_BidIt2>&_Other): current(_Other.base())
		{
		}
		_BidIt base()const
		{
			return (current);
		}
		_Reference operator*()const
		{
			_BidIt _Tmp=current;
			return (*--_Tmp);
		}
		_Pointer operator->()const
		{
			_Reference _Tmp=**this;
			return (&_Tmp);
		}
		_Myt&operator++()
		{
			--current;
			return (*this);
		}
		_Myt operator++(int)
		{
			_Myt _Tmp=*this;
			--current;
			return (_Tmp);
		}
		_Myt&operator--()
		{
			++current;
			return (*this);
		}
		_Myt operator--(int)
		{
			_Myt _Tmp=*this;
			++current;
			return (_Tmp);
		}
		bool operator==(const _Myt&_Right)const
		{
			return (current==_Right.current);
		}
		bool operator!=(const _Myt&_Right)const
		{
			return (!(*this==_Right));
		}
	protected: _BidIt current;
	};
	template<class _Elem, class _Traits>
	class istreambuf_iterator: public _Iterator_with_base<input_iterator_tag, _Elem, typename _Traits::off_type, _Elem*, _Elem&, _Iterator_base_secure>
	{
		typedef istreambuf_iterator<_Elem, _Traits>_Myt;
	public:
		typedef _Elem char_type;
		typedef _Traits traits_type;
		typedef basic_streambuf<_Elem, _Traits>streambuf_type;
		typedef basic_istream<_Elem, _Traits>istream_type;
		typedef typename traits_type::int_type int_type;
		istreambuf_iterator(streambuf_type*_Sb=0)throw(): _Strbuf(_Sb), _Got(_Sb==0)
		{
		}
		istreambuf_iterator(istream_type&_Istr)throw(): _Strbuf(_Istr.rdbuf()), _Got(_Istr.rdbuf()==0)
		{
		}
		_Elem operator*()const
		{
			if(!_Got)((_Myt*)this)->_Peek();
			if(_Strbuf==0)_Debug_message(L"istreambuf_iterator is not dereferencable", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2074);
			return (_Val);
		}
		_Myt&operator++()
		{
			if(_Strbuf==0)_Debug_message(L"istreambuf_iterator is not incrementable", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2085);
			_Inc();
			return (*this);
		}
		_Myt operator++(int)
		{
			if(!_Got)_Peek();
			_Myt _Tmp=*this;
			++*this;
			return (_Tmp);
		}
		bool equal(const _Myt&_Right)const
		{
			if(!_Got)((_Myt*)this)->_Peek();
			if(!_Right._Got)((_Myt*)&_Right)->_Peek();
			return (_Strbuf==0&&_Right._Strbuf==0||_Strbuf!=0&&_Right._Strbuf!=0);
		}
	private:
		void _Inc()
		{
			if(_Strbuf==0||traits_type::eq_int_type(traits_type::eof(), _Strbuf->sbumpc()))_Strbuf=0, _Got=true;
			else _Got=false;
		}
		_Elem _Peek()
		{
			int_type _Meta;
			if(_Strbuf==0||traits_type::eq_int_type(traits_type::eof(), _Meta=_Strbuf->sgetc()))_Strbuf=0;
			else _Val=traits_type::to_char_type(_Meta);
			_Got=true;
			return (_Val);
		}
		streambuf_type*_Strbuf;
		bool _Got;
		_Elem _Val;
	};
	template<class _Elem, class _Traits>
	inline bool operator==(const istreambuf_iterator<_Elem, _Traits>&_Left, const istreambuf_iterator<_Elem, _Traits>&_Right)
	{
		return (_Left.equal(_Right));
	}
	template<class _Elem, class _Traits>
	inline bool operator!=(const istreambuf_iterator<_Elem, _Traits>&_Left, const istreambuf_iterator<_Elem, _Traits>&_Right)
	{
		return (!(_Left==_Right));
	}
	template<class _Elem, class _Traits>
	class ostreambuf_iterator: public _Outit_with_base<_Iterator_base_secure>
	{
		typedef ostreambuf_iterator<_Elem, _Traits>_Myt;
	public:
		typedef _Elem char_type;
		typedef _Traits traits_type;
		typedef basic_streambuf<_Elem, _Traits>streambuf_type;
		typedef basic_ostream<_Elem, _Traits>ostream_type;
		typedef _Range_checked_iterator_tag _Checked_iterator_category;
		ostreambuf_iterator(streambuf_type*_Sb)throw(): _Failed(false), _Strbuf(_Sb)
		{
		}
		ostreambuf_iterator(ostream_type&_Ostr)throw(): _Failed(false), _Strbuf(_Ostr.rdbuf())
		{
		}
		_Myt&operator=(_Elem _Right)
		{
			if(_Strbuf==0||traits_type::eq_int_type(_Traits::eof(), _Strbuf->sputc(_Right)))_Failed=true;
			return (*this);
		}
		_Myt&operator*()
		{
			return (*this);
		}
		_Myt&operator++()
		{
			return (*this);
		}
		_Myt&operator++(int)
		{
			return (*this);
		}
		bool failed()const throw()
		{
			return (_Failed);
		}
	private:
		bool _Failed;
		streambuf_type*_Strbuf;
	};
	template<class _InIt, class _OutIt, class _InOutItCat>
	inline _OutIt _Copy_opt(_InIt _First, _InIt _Last, _OutIt _Dest, _InOutItCat, _Nonscalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2226);
		for(;
			_First!=_Last;
			++_Dest, ++_First)*_Dest=*_First;
		return (_Dest);
	}
	template<class _InIt, class _OutIt>
	inline _OutIt _Copy_opt(_InIt _First, _InIt _Last, _OutIt _Dest, random_access_iterator_tag, _Nonscalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{
		_OutIt _Result=_Dest+(_Last-_First);
		_Copy_opt(_First, _Last, ::std::_Checked_base(_Dest), forward_iterator_tag(), _Nonscalar_ptr_iterator_tag(), _Range_checked_iterator_tag());
		return _Result;
	}
	template<class _InIt, class _OutIt, class _InOutItCat>
	inline _OutIt _Copy_opt(_InIt _First, _InIt _Last, _OutIt _Dest, _InOutItCat, _Scalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2253);
		if(_First!=_Last)_Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2255);
		ptrdiff_t _Off=_Last-_First;
		_OutIt _Result=_Dest+_Off;
		if(_Off>0)::memmove_s((&*_Dest), (_Off*sizeof(*_First)), (&*_First), (_Off*sizeof(*_First)));
		return _Result;
	}
	template<class _InIt, class _OutIt, class _InOutItCat, class _Ty>
	inline _OutIt _Copy_opt(_InIt _First, _InIt _Last, _OutIt _Dest, _InOutItCat _First_dest_cat, _Ty _PtrCat, _Unchecked_iterator_tag)
	{
		return _Copy_opt(_First, _Last, _Dest, _First_dest_cat, _PtrCat, _Range_checked_iterator_tag());
	}
	template<class _InIt, class _OutIt>
	inline _OutIt copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{
		return (_Copy_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, _Iter_random(_First, _Dest), _Ptr_cat(_First, _Dest), ::std::_Checked_cat(_Dest)));
	}
	template<class _InIt, class _OutIt, class _InOutItCat, class _MoveCatTy>
	inline _OutIt _Move_opt(_InIt _First, _InIt _Last, _OutIt _Dest, _InOutItCat _First_dest_cat, _MoveCatTy, _Range_checked_iterator_tag)
	{
		return _Copy_opt(_First, _Last, _Dest, _First_dest_cat, _Ptr_cat(_First, _Dest), _Range_checked_iterator_tag());
	}
	template<class _InIt, class _OutIt, class _InOutItCat>
	inline _OutIt _Move_opt(_InIt _First, _InIt _Last, _OutIt _Dest, _InOutItCat, _Swap_move_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2300);
		for(;
			_First!=_Last;
			++_Dest, ++_First)::std::swap(*_Dest, *_First);
		return (_Dest);
	}
	template<class _InIt, class _OutIt, class _InOutItCat>
	inline _OutIt _Move_opt(_InIt _First, _InIt _Last, _OutIt _Dest, random_access_iterator_tag, _Swap_move_tag _Move_cat, _Range_checked_iterator_tag)
	{
		_OutIt _Result=_Dest+(_Last-_First);
		_Move_opt(_First, _Last, ::std::_Checked_base(_Dest), forward_iterator_tag(), _Move_cat, _Range_checked_iterator_tag());
		return _Result;
	}
	template<class _InIt, class _OutIt, class _InOutItCat, class _MoveCatTy>
	inline _OutIt _Move_opt(_InIt _First, _InIt _Last, _OutIt _Dest, _InOutItCat _First_dest_cat, _MoveCatTy _Move_cat, _Unchecked_iterator_tag)
	{
		return _Move_opt(_First, _Last, _Dest, _First_dest_cat, _Move_cat, _Range_checked_iterator_tag());
	}
	template<class _InIt, class _OutIt>
	inline _OutIt _Move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{
		return _Move_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, _Iter_random(_First, _Dest), _Move_cat(_Dest), ::std::_Checked_cat(_Dest));
	}
	template<class _BidIt1, class _BidIt2, class _InOutItCat>
	inline _BidIt2 _Copy_backward_opt(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest, _InOutItCat, _Nonscalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2345);
		while(_First!=_Last)*--_Dest=*--_Last;
		return (_Dest);
	}
	template<class _InIt, class _OutIt>
	inline _OutIt _Copy_backward_opt(_InIt _First, _InIt _Last, _OutIt _Dest, random_access_iterator_tag, _Nonscalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{
		_OutIt _Result=_Dest-(_Last-_First);
		_Copy_backward_opt(_First, _Last, ::std::_Checked_base(_Dest), forward_iterator_tag(), _Nonscalar_ptr_iterator_tag(), _Range_checked_iterator_tag());
		return _Result;
	}
	template<class _InIt, class _OutIt, class _InOutItCat>
	inline _OutIt _Copy_backward_opt(_InIt _First, _InIt _Last, _OutIt _Dest, _InOutItCat, _Scalar_ptr_iterator_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2372);
		if(_First!=_Last)_Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2374);
		ptrdiff_t _Off=_Last-_First;
		_OutIt _Result=_Dest-_Off;
		if(_Off>0)::memmove_s((&*_Result), (_Off*sizeof(*_First)), (&*_First), (_Off*sizeof(*_First)));
		return _Result;
	}
	template<class _BidIt1, class _BidIt2, class _InOutItCat, class _Ty>
	inline _BidIt2 _Copy_backward_opt(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest, _InOutItCat _First_dest_cat, _Ty _PtrCat, _Unchecked_iterator_tag)
	{
		return _Copy_backward_opt(_First, _Last, _Dest, _First_dest_cat, _PtrCat, _Range_checked_iterator_tag());
	}
	template<class _BidIt1, class _BidIt2>
	inline _BidIt2 copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{
		return _Copy_backward_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, _Iter_random(_First, _Dest), _Ptr_cat(_First, _Dest), ::std::_Checked_cat(_Dest));
	}
	template<class _BidIt1, class _BidIt2, class _InOutItCat, class _MoveCatTy>
	inline _BidIt2 _Move_backward_opt(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest, _InOutItCat _First_dest_cat, _MoveCatTy, _Range_checked_iterator_tag)
	{
		return _Copy_backward_opt(_First, _Last, _Dest, _First_dest_cat, _Ptr_cat(_First, _Dest), _Range_checked_iterator_tag());
	}
	template<class _BidIt1, class _BidIt2, class _InOutItCat>
	inline _BidIt2 _Move_backward_opt(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest, _InOutItCat, _Swap_move_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2422);
		while(_First!=_Last)::std::swap(*--_Dest, *--_Last);
		return (_Dest);
	}
	template<class _BidIt1, class _BidIt2>
	inline _BidIt2 _Move_backward_opt(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest, random_access_iterator_tag, _Swap_move_tag _Move_cat, _Range_checked_iterator_tag)
	{
		_BidIt2 _Result=_Dest-(_Last-_First);
		_Move_backward_opt(_First, _Last, ::std::_Checked_base(_Dest), forward_iterator_tag(), _Move_cat, _Range_checked_iterator_tag());
		return _Result;
	}
	template<class _BidIt1, class _BidIt2, class _InOutItCat, class _MoveCatTy>
	inline _BidIt2 _Move_backward_opt(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest, _InOutItCat _First_dest_cat, _MoveCatTy _Move_cat, _Unchecked_iterator_tag)
	{
		return _Move_backward_opt(_First, _Last, _Dest, _First_dest_cat, _Move_cat, _Range_checked_iterator_tag());
	}
	template<class _BidIt1, class _BidIt2>
	inline _BidIt2 _Move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{
		return _Move_backward_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, _Iter_random(_First, _Dest), _Move_cat(_Dest), ::std::_Checked_cat(_Dest));
	}
	template<class _InIt1, class _InIt2, class _InItCats>
	inline pair<_InIt1, _InIt2>_Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _InItCats, _Range_checked_iterator_tag)
	{
		_Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2471);
		if(_First1!=_Last1)_Debug_pointer(_First2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2473);
		for(;
			_First1!=_Last1&&*_First1==*_First2;
			)++_First1, ++_First2;
		return (pair<_InIt1, _InIt2>(_First1, _First2));
	}
	template<class _InIt1, class _InIt2>
	inline pair<_InIt1, _InIt2>_Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		_InIt2 _Last2=_First2+(_Last1-_First1);
		(_Last2);
		pair<_InIt1, typename::std::_Checked_iterator_base_helper<_InIt2>::_Checked_iterator_base_type>_Result=_Mismatch(_First1, _Last1, ::std::_Checked_base(_First2), forward_iterator_tag(), _Range_checked_iterator_tag());
		::std::_Checked_assign_from_base(_First2, _Result.second);
		return (pair<_InIt1, _InIt2>(_Result.first, _First2));
	}
	template<class _InIt1, class _InIt2, class _InItCats>
	inline pair<_InIt1, _InIt2>_Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _InItCats _InIt_cats, _Unchecked_iterator_tag)
	{
		return _Mismatch(_First1, _Last1, _First2, _InIt_cats, _Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2>
	inline pair<_InIt1, _InIt2>mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{
		pair<typename::std::_Checked_iterator_base_helper<_InIt1>::_Checked_iterator_base_type, _InIt2>_Result=_Mismatch(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, _Iter_random(_First1, _First2), ::std::_Checked_cat(_First2));
		::std::_Checked_assign_from_base(_First1, _Result.first);
		return (pair<_InIt1, _InIt2>(_First1, _Result.second));
	}
	template<class _InIt1, class _InIt2, class _Pr, class _InItCats>
	inline pair<_InIt1, _InIt2>_Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred, _InItCats, _Range_checked_iterator_tag)
	{
		_Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2537);
		if(_First1!=_Last1)_Debug_pointer(_First2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2539);
		_Debug_pointer(_Pred, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2540);
		for(;
			_First1!=_Last1&&_Pred(*_First1, *_First2);
			)++_First1, ++_First2;
		return (pair<_InIt1, _InIt2>(_First1, _First2));
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline pair<_InIt1, _InIt2>_Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred, random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		_InIt2 _Last2=_First2+(_Last1-_First1);
		(_Last2);
		pair<_InIt1, typename::std::_Checked_iterator_base_helper<_InIt2>::_Checked_iterator_base_type>_Result=_Mismatch(_First1, _Last1, ::std::_Checked_base(_First2), _Pred, forward_iterator_tag(), _Range_checked_iterator_tag());
		::std::_Checked_assign_from_base(_First2, _Result.second);
		return (pair<_InIt1, _InIt2>(_Result.first, _First2));
	}
	template<class _InIt1, class _InIt2, class _Pr, class _InItCats>
	inline pair<_InIt1, _InIt2>_Mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred, _InItCats _InIt_cats, _Unchecked_iterator_tag)
	{
		return _Mismatch(_First1, _Last1, _First2, _Pred, _InIt_cats, _Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline pair<_InIt1, _InIt2>mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{
		pair<typename::std::_Checked_iterator_base_helper<_InIt1>::_Checked_iterator_base_type, _InIt2>_Result=_Mismatch(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, _Pred, _Iter_random(_First1, _First2), ::std::_Checked_cat(_First2));
		::std::_Checked_assign_from_base(_First1, _Result.first);
		return (pair<_InIt1, _InIt2>(_First1, _Result.second));
	}
	template<class _InIt1, class _InIt2, class _InItCats>
	inline bool _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _InItCats, _Range_checked_iterator_tag)
	{
		return (_Mismatch(_First1, _Last1, _First2, forward_iterator_tag(), _Range_checked_iterator_tag()).first==_Last1);
	}
	inline bool _Equal(const char*_First1, const char*_Last1, const char*_First2, random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2610);
		if(_First1!=_Last1)_Debug_pointer(_First2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2612);
		return (::memcmp(_First1, _First2, _Last1-_First1)==0);
	}
	inline bool _Equal(const signed char*_First1, const signed char*_Last1, const signed char*_First2, random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2623);
		if(_First1!=_Last1)_Debug_pointer(_First2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2625);
		return (::memcmp(_First1, _First2, _Last1-_First1)==0);
	}
	inline bool _Equal(const unsigned char*_First1, const unsigned char*_Last1, const unsigned char*_First2, random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		_Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2636);
		if(_First1!=_Last1)_Debug_pointer(_First2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2638);
		return (::memcmp(_First1, _First2, _Last1-_First1)==0);
	}
	template<class _InIt1, class _InIt2>
	inline bool _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		_InIt2 _Last2=_First2+(_Last1-_First1);
		(_Last2);
		return _Equal(_First1, _Last1, ::std::_Checked_base(_First2), forward_iterator_tag(), _Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2, class _InItCats>
	inline bool _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _InItCats _InIt_cats, _Unchecked_iterator_tag)
	{
		return _Equal(_First1, _Last1, _First2, _InIt_cats, _Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2>
	inline bool equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{
		return _Equal(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, _Iter_random(_First1, _First2), ::std::_Checked_cat(_First2));
	}
	template<class _InIt1, class _InIt2, class _Pr, class _InItCats>
	inline bool _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred, _InItCats, _Range_checked_iterator_tag)
	{
		return (_Mismatch(_First1, _Last1, _First2, _Pred, forward_iterator_tag(), _Range_checked_iterator_tag()).first==_Last1);
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline bool _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred, random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		_InIt2 _Last2=_First2+(_Last1-_First1);
		(_Last2);
		return _Equal(_First1, _Last1, ::std::_Checked_base(_First2), _Pred, forward_iterator_tag(), _Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2, class _Pr, class _InItCats>
	inline bool _Equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred, _InItCats _InIt_cats, _Unchecked_iterator_tag)
	{
		return _Equal(_First1, _Last1, _First2, _Pred, _InIt_cats, _Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline bool equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{
		return _Equal(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, _Pred, _Iter_random(_First1, _First2), ::std::_Checked_cat(_First2));
	}
	template<class _FwdIt, class _Ty>
	inline void _Fill(_FwdIt _First, _FwdIt _Last, const _Ty&_Val)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2724);
		for(;
			_First!=_Last;
			++_First)*_First=_Val;
	}
	inline void _Fill(char*_First, char*_Last, int _Val)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2732);
		::memset(_First, _Val, _Last-_First);
	}
	inline void _Fill(signed char*_First, signed char*_Last, int _Val)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2739);
		::memset(_First, _Val, _Last-_First);
	}
	inline void _Fill(unsigned char*_First, unsigned char*_Last, int _Val)
	{
		_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2747);
		::memset(_First, _Val, _Last-_First);
	}
	template<class _FwdIt, class _Ty>
	inline void fill(_FwdIt _First, _FwdIt _Last, const _Ty&_Val)
	{
		_Fill(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Val);
	}
	template<class _OutIt, class _Diff, class _Ty>
	inline void _Fill_n(_OutIt _First, _Diff _Count, const _Ty&_Val, _Range_checked_iterator_tag)
	{
		for(;
			0<_Count;
			--_Count, ++_First)*_First=_Val;
	}
	inline void _Fill_n(char*_First, size_t _Count, int _Val, _Range_checked_iterator_tag)
	{
		if(0<_Count)_Debug_pointer(_First, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2774);
		::memset(_First, _Val, _Count);
	}
	inline void _Fill_n(signed char*_First, size_t _Count, int _Val, _Range_checked_iterator_tag)
	{
		if(0<_Count)_Debug_pointer(_First, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2786);
		::memset(_First, _Val, _Count);
	}
	inline void _Fill_n(unsigned char*_First, size_t _Count, int _Val, _Range_checked_iterator_tag)
	{
		if(0<_Count)_Debug_pointer(_First, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2798);
		::memset(_First, _Val, _Count);
	}
	template<class _OutIt, class _Diff, class _Ty, class _OutCat>
	inline void _Fill_n(_OutIt _First, _Diff _Count, const _Ty&_Val, _OutCat, _Range_checked_iterator_tag)
	{
		_Fill_n(_First, _Count, _Val, _Range_checked_iterator_tag());
	}
	template<class _OutIt, class _Diff, class _Ty>
	inline void _Fill_n(_OutIt _First, _Diff _Count, const _Ty&_Val, random_access_iterator_tag, _Range_checked_iterator_tag)
	{
		_OutIt _Last=_First+_Count;
		(_Last);
		_Fill_n(::std::_Checked_base(_First), _Count, _Val, _Range_checked_iterator_tag());
	}
	template<class _OutIt, class _Diff, class _Ty, class _OutCat>
	inline void _Fill_n(_OutIt _First, _Diff _Count, const _Ty&_Val, _OutCat _First_cat, _Unchecked_iterator_tag)
	{
		_Fill_n(_First, _Count, _Val, _First_cat, _Range_checked_iterator_tag());
	}
	template<class _OutIt, class _Diff, class _Ty>
	inline void fill_n(_OutIt _First, _Diff _Count, const _Ty&_Val)
	{
		_Fill_n(_First, _Count, _Val, _Iter_cat(_First), ::std::_Checked_cat(_First));
	}
	template<class _InIt1, class _InIt2>
	inline bool _Lexicographical_compare(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _InIt2 _Last2)
	{
		_Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2849);
		_Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2850);
		for(;
			_First1!=_Last1&&_First2!=_Last2;
			++_First1, ++_First2)if(_Debug_lt(*_First1, *_First2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2852))return (true);
			else if(*_First2<*_First1)return (false);
			return (_First1==_Last1&&_First2!=_Last2);
	}
	template<class _InIt1, class _InIt2>
	inline bool lexicographical_compare(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _InIt2 _Last2)
	{
		return _Lexicographical_compare(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), ::std::_Checked_base(_First2), ::std::_Checked_base(_Last2));
	}
	inline bool lexicographical_compare(const unsigned char*_First1, const unsigned char*_Last1, const unsigned char*_First2, const unsigned char*_Last2)
	{
		_Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2871);
		_Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2872);
		ptrdiff_t _Num1=_Last1-_First1;
		ptrdiff_t _Num2=_Last2-_First2;
		int _Ans=::memcmp(_First1, _First2, _Num1<_Num2?_Num1: _Num2);
		return (_Ans<0||_Ans==0&&_Num1<_Num2);
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline bool _Lexicographical_compare(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _InIt2 _Last2, _Pr _Pred)
	{
		_Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2900);
		_Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2901);
		_Debug_pointer(_Pred, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2902);
		for(;
			_First1!=_Last1&&_First2!=_Last2;
			++_First1, ++_First2)if(_Debug_lt_pred(_Pred, *_First1, *_First2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2904))return (true);
			else if(_Pred(*_First2, *_First1))return (false);
			return (_First1==_Last1&&_First2!=_Last2);
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline bool lexicographical_compare(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _InIt2 _Last2, _Pr _Pred)
	{
		return _Lexicographical_compare(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), ::std::_Checked_base(_First2), ::std::_Checked_base(_Last2), _Pred);
	}
	template<class _Ty>
	inline const _Ty&(max)(const _Ty&_Left, const _Ty&_Right)
	{
		return (_Debug_lt(_Left, _Right, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2938)?_Right: _Left);
	}
	template<class _Ty, class _Pr>
	inline const _Ty&(max)(const _Ty&_Left, const _Ty&_Right, _Pr _Pred)
	{
		return (_Debug_lt_pred(_Pred, _Left, _Right, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2946)?_Right: _Left);
	}
	template<class _Ty>
	inline const _Ty&(min)(const _Ty&_Left, const _Ty&_Right)
	{
		return (_Debug_lt(_Right, _Left, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2953)?_Right: _Left);
	}
	template<class _Ty, class _Pr>
	inline const _Ty&(min)(const _Ty&_Left, const _Ty&_Right, _Pr _Pred)
	{
		return (_Debug_lt_pred(_Pred, _Right, _Left, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xutility", 2961)?_Right: _Left);
	}
}
namespace stdext
{
	template<class _InIt, class _OutIt>
	inline _OutIt unchecked_copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{
		return (::std::_Copy_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, ::std::_Iter_random(_First, _Dest), ::std::_Ptr_cat(_First, _Dest), ::std::_Range_checked_iterator_tag()));
	}
	template<class _InIt, class _OutIt>
	inline _OutIt checked_copy(_InIt _First, _InIt _Last, _OutIt _Dest)
	{
		return (::std::_Copy_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, ::std::_Iter_random(_First, _Dest), ::std::_Ptr_cat(_First, _Dest), ::std::_Checked_cat(_Dest)));
	}
	template<class _BidIt1, class _BidIt2>
	inline _BidIt2 unchecked_copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{
		return (::std::_Copy_backward_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, ::std::_Iter_random(_First, _Dest), ::std::_Ptr_cat(_First, _Dest), ::std::_Range_checked_iterator_tag()));
	}
	template<class _BidIt1, class _BidIt2>
	inline _BidIt2 checked_copy_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{
		return (::std::_Copy_backward_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, ::std::_Iter_random(_First, _Dest), ::std::_Ptr_cat(_First, _Dest), ::std::_Checked_cat(_Dest)));
	}
	template<class _InIt, class _OutIt>
	inline _OutIt _Unchecked_move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{
		return (::std::_Move_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, ::std::_Iter_random(_First, _Dest), ::std::_Move_cat(_Dest), ::std::_Range_checked_iterator_tag()));
	}
	template<class _InIt, class _OutIt>
	inline _OutIt _Checked_move(_InIt _First, _InIt _Last, _OutIt _Dest)
	{
		return (::std::_Move_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, ::std::_Iter_random(_First, _Dest), ::std::_Move_cat(_Dest), ::std::_Checked_cat(_Dest)));
	}
	template<class _BidIt1, class _BidIt2>
	inline _BidIt2 _Unchecked_move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{
		return (::std::_Move_backward_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, ::std::_Iter_random(_First, _Dest), ::std::_Move_cat(_Dest), ::std::_Range_checked_iterator_tag()));
	}
	template<class _BidIt1, class _BidIt2>
	inline _BidIt2 _Checked_move_backward(_BidIt1 _First, _BidIt1 _Last, _BidIt2 _Dest)
	{
		return (::std::_Move_backward_opt(::std::_Checked_base(_First), ::std::_Checked_base(_Last), _Dest, ::std::_Iter_random(_First, _Dest), ::std::_Move_cat(_Dest), ::std::_Checked_cat(_Dest)));
	}
	template<class _OutIt, class _Diff, class _Ty>
	inline void unchecked_fill_n(_OutIt _First, _Diff _Count, const _Ty&_Val)
	{
		::std::_Fill_n(_First, _Count, _Val, ::std::_Iter_cat(_First), ::std::_Range_checked_iterator_tag());
	}
	template<class _OutIt, class _Diff, class _Ty>
	inline void checked_fill_n(_OutIt _First, _Diff _Count, const _Ty&_Val)
	{
		::std::_Fill_n(_First, _Count, _Val, ::std::_Iter_cat(_First), ::std::_Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2>
	inline::std::pair<_InIt1, _InIt2>unchecked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{
		::std::pair<typename::std::_Checked_iterator_base_helper<_InIt1>::_Checked_iterator_base_type, _InIt2>_Result=::std::_Mismatch(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, ::std::_Iter_random(_First1, _First2), ::std::_Range_checked_iterator_tag());
		::std::_Checked_assign_from_base(_First1, _Result.first);
		return (::std::pair<_InIt1, _InIt2>(_First1, _Result.second));
	}
	template<class _InIt1, class _InIt2>
	inline::std::pair<_InIt1, _InIt2>checked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{
		::std::pair<typename::std::_Checked_iterator_base_helper<_InIt1>::_Checked_iterator_base_type, _InIt2>_Result=::std::_Mismatch(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, ::std::_Iter_random(_First1, _First2), ::std::_Checked_cat(_First2));
		::std::_Checked_assign_from_base(_First1, _Result.first);
		return (::std::pair<_InIt1, _InIt2>(_First1, _Result.second));
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline::std::pair<_InIt1, _InIt2>unchecked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{
		::std::pair<typename::std::_Checked_iterator_base_helper<_InIt1>::_Checked_iterator_base_type, _InIt2>_Result=::std::_Mismatch(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, _Pred, ::std::_Iter_random(_First1, _First2), ::std::_Range_checked_iterator_tag());
		::std::_Checked_assign_from_base(_First1, _Result.first);
		return (::std::pair<_InIt1, _InIt2>(_First1, _Result.second));
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline::std::pair<_InIt1, _InIt2>checked_mismatch(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{
		::std::pair<typename::std::_Checked_iterator_base_helper<_InIt1>::_Checked_iterator_base_type, _InIt2>_Result=::std::_Mismatch(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, _Pred, ::std::_Iter_random(_First1, _First2), ::std::_Checked_cat(_First2));
		::std::_Checked_assign_from_base(_First1, _Result.first);
		return (::std::pair<_InIt1, _InIt2>(_First1, _Result.second));
	}
	template<class _InIt1, class _InIt2>
	inline bool unchecked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{
		return ::std::_Equal(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, ::std::_Iter_random(_First1, _First2), ::std::_Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2>
	inline bool checked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2)
	{
		return ::std::_Equal(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, ::std::_Iter_random(_First1, _First2), ::std::_Checked_cat(_First2));
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline bool unchecked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{
		return ::std::_Equal(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, _Pred, ::std::_Iter_random(_First1, _First2), ::std::_Range_checked_iterator_tag());
	}
	template<class _InIt1, class _InIt2, class _Pr>
	inline bool checked_equal(_InIt1 _First1, _InIt1 _Last1, _InIt2 _First2, _Pr _Pred)
	{
		return ::std::_Equal(::std::_Checked_base(_First1), ::std::_Checked_base(_Last1), _First2, _Pred, ::std::_Iter_random(_First1, _First2), ::std::_Checked_cat(_First2));
	}
}
extern "C" void _invalid_parameter(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t);
namespace std
{
	template<class _Ty>
	inline _Ty*_Allocate(size_t _Count, _Ty*)
	{
		if(_Count<=0)_Count=0;
		else if(((size_t)(-1)/_Count)<sizeof(_Ty))throw std::bad_alloc(0);
		return ((_Ty*)::operator new(_Count*sizeof(_Ty)));
	}
	template<class _T1, class _T2>
	inline void _Construct(_T1*_Ptr, const _T2&_Val)
	{
		void*_Vptr=_Ptr;
		::new(_Vptr)_T1(_Val);
	}
	template<class _Ty>
	inline void _Destroy(_Ty*_Ptr)
	{
		(_Ptr)->~_Ty();
	}
	template<>
	inline void _Destroy(char*)
	{
	}
	template<>
	inline void _Destroy(wchar_t*)
	{
	}
	template<class _Ty>
	struct _Allocator_base
	{
		typedef _Ty value_type;
	};
	template<class _Ty>
	struct _Allocator_base<const _Ty>
	{
		typedef _Ty value_type;
	};
	template<class _Ty>
	class allocator: public _Allocator_base<_Ty>
	{
	public:
		typedef _Allocator_base<_Ty>_Mybase;
		typedef typename _Mybase::value_type value_type;
		typedef value_type*pointer;
		typedef value_type&reference;
		typedef const value_type*const_pointer;
		typedef const value_type&const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		template<class _Other>
		struct rebind
		{
			typedef allocator<_Other>other;
		};
		pointer address(reference _Val)const
		{
			return (&_Val);
		}
		const_pointer address(const_reference _Val)const
		{
			return (&_Val);
		}
		allocator()throw()
		{
		}
		allocator(const allocator<_Ty>&)throw()
		{
		}
		template<class _Other>
		allocator(const allocator<_Other>&)throw()
		{
		}
		template<class _Other>
		allocator<_Ty>&operator=(const allocator<_Other>&)
		{
			return (*this);
		}
		void deallocate(pointer _Ptr, size_type)
		{
			::operator delete(_Ptr);
		}
		pointer allocate(size_type _Count)
		{
			return (_Allocate(_Count, (pointer)0));
		}
		pointer allocate(size_type _Count, const void*)
		{
			return (allocate(_Count));
		}
		void construct(pointer _Ptr, const _Ty&_Val)
		{
			_Construct(_Ptr, _Val);
		}
		void destroy(pointer _Ptr)
		{
			_Destroy(_Ptr);
		}
		size_t max_size()const throw()
		{
			size_t _Count=(size_t)(-1)/sizeof(_Ty);
			return (0<_Count?_Count: 1);
		}
	};
	template<class _Ty, class _Other>
	inline bool operator==(const allocator<_Ty>&, const allocator<_Other>&)throw()
	{
		return (true);
	}
	template<class _Ty, class _Other>
	inline bool operator!=(const allocator<_Ty>&, const allocator<_Other>&)throw()
	{
		return (false);
	}
	template<>
	class allocator<void>
	{
	public:
		typedef void _Ty;
		typedef _Ty*pointer;
		typedef const _Ty*const_pointer;
		typedef _Ty value_type;
		template<class _Other>
		struct rebind
		{
			typedef allocator<_Other>other;
		};
		allocator()throw()
		{
		}
		allocator(const allocator<_Ty>&)throw()
		{
		}
		template<class _Other>
		allocator(const allocator<_Other>&)throw()
		{
		}
		template<class _Other>
		allocator<_Ty>&operator=(const allocator<_Other>&)
		{
			return (*this);
		}
	};
	template<class _Ty, class _Alloc>
	inline void _Destroy_range(_Ty*_First, _Ty*_Last, _Alloc&_Al)
	{
		_Destroy_range(_First, _Last, _Al, _Ptr_cat(_First, _Last));
	}
	template<class _Ty, class _Alloc>
	inline void _Destroy_range(_Ty*_First, _Ty*_Last, _Alloc&_Al, _Nonscalar_ptr_iterator_tag)
	{
		for(;
			_First!=_Last;
			++_First)_Al.destroy(_First);
	}
	template<class _Ty, class _Alloc>
	inline void _Destroy_range(_Ty*_First, _Ty*_Last, _Alloc&_Al, _Scalar_ptr_iterator_tag)
	{
	}
}
namespace std
{
	template<class _Elem, class _Traits=char_traits<_Elem>, class _Ax=allocator<_Elem> >
	class basic_string;
	template<class _Elem, class _Traits, class _Alloc>
	class _String_const_iterator: public _Ranit_base<_Elem, typename _Alloc::difference_type, typename _Alloc::const_pointer, typename _Alloc::const_reference, _Iterator_base_secure>
	{
	public:
		typedef _String_const_iterator<_Elem, _Traits, _Alloc>_Myt;
		typedef basic_string<_Elem, _Traits, _Alloc>_Mystring;
		typedef random_access_iterator_tag iterator_category;
		typedef _Elem value_type;
		typedef typename _Alloc::difference_type difference_type;
		typedef typename _Alloc::const_pointer pointer;
		typedef typename _Alloc::const_reference reference;
		typedef _Range_checked_iterator_tag _Checked_iterator_category;
		_String_const_iterator()
		{
			_Myptr=0;
		}
		_String_const_iterator(pointer _Ptr, const _Container_base*_Pstring)
		{

			{
				if(!(_Pstring==0||_Ptr!=0&&((_Mystring*)_Pstring)->_Myptr()<=_Ptr&&_Ptr<=(((_Mystring*)_Pstring)->_Myptr()+((_Mystring*)_Pstring)->_Mysize)))
				{
					(void)((!!((("_Pstring == NULL || _Ptr != NULL && ((_Mystring *)_Pstring)->_Myptr() <= _Ptr && _Ptr <= (((_Mystring *)_Pstring)->_Myptr() + ((_Mystring *)_Pstring)->_Mysize)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 72, 0, L"(\"_Pstring == NULL || _Ptr != NULL && ((_Mystring *)_Pstring)->_Myptr() <= _Ptr && _Ptr <= (((_Mystring *)_Pstring)->_Myptr() + ((_Mystring *)_Pstring)->_Mysize)\", 0)"))||(__debugbreak(), 0));
					::_invalid_parameter(L"\"invalid argument\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 72, 0);
				};
			};
			this->_Adopt(_Pstring);
			_Myptr=_Ptr;
		}
		reference operator*()const
		{
			if(this->_Mycont==0||_Myptr==0||_Myptr<((_Mystring*)this->_Mycont)->_Myptr()||((_Mystring*)this->_Mycont)->_Myptr()+((_Mystring*)this->_Mycont)->_Mysize<=_Myptr)
			{
				_Debug_message(L"string iterator not dereferencable", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 111);

				{
					(void)((!!((("Standard C++ Libraries Out of Range", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 112, 0, L"(\"Standard C++ Libraries Out of Range\", 0)"))||(__debugbreak(), 0));
					::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 112, 0);
				};
			}
			return (*_Myptr);
		}
		pointer operator->()const
		{
			return (&**this);
		}
		_Myt&operator++()
		{
			if(this->_Mycont!=((const _Container_base*)-2))
			{

				{
					if(!(this->_Mycont!=0))
					{
						(void)((!!((("this->_Mycont != NULL", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 134, 0, L"(\"this->_Mycont != NULL\", 0)"))||(__debugbreak(), 0));
						::_invalid_parameter(L"\"invalid argument\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 134, 0);
					};
				};

				{
					if(!(_Myptr<(((_Mystring*)this->_Mycont)->_Myptr()+((_Mystring*)this->_Mycont)->_Mysize)))
					{
						(void)((!!((("_Myptr < (((_Mystring *)this->_Mycont)->_Myptr() + ((_Mystring *)this->_Mycont)->_Mysize)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 135, 0, L"(\"_Myptr < (((_Mystring *)this->_Mycont)->_Myptr() + ((_Mystring *)this->_Mycont)->_Mysize)\", 0)"))||(__debugbreak(), 0));
						::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 135, 0);
					};
				};
			}
			++_Myptr;
			return (*this);
		}
		_Myt operator++(int)
		{
			_Myt _Tmp=*this;
			++*this;
			return (_Tmp);
		}
		_Myt&operator--()
		{
			if(this->_Mycont!=((const _Container_base*)-2))
			{

				{
					if(!(this->_Mycont!=0))
					{
						(void)((!!((("this->_Mycont != NULL", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 152, 0, L"(\"this->_Mycont != NULL\", 0)"))||(__debugbreak(), 0));
						::_invalid_parameter(L"\"invalid argument\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 152, 0);
					};
				};

				{
					if(!(_Myptr>((_Mystring*)this->_Mycont)->_Myptr()))
					{
						(void)((!!((("_Myptr > ((_Mystring *)this->_Mycont)->_Myptr()", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 153, 0, L"(\"_Myptr > ((_Mystring *)this->_Mycont)->_Myptr()\", 0)"))||(__debugbreak(), 0));
						::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 153, 0);
					};
				};
			}
			--_Myptr;
			return (*this);
		}
		_Myt operator--(int)
		{
			_Myt _Tmp=*this;
			--*this;
			return (_Tmp);
		}
		_Myt&operator+=(difference_type _Off)
		{
			if(this->_Mycont!=((const _Container_base*)-2))
			{

				{
					if(!(this->_Mycont!=0))
					{
						(void)((!!((("this->_Mycont != NULL", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 170, 0, L"(\"this->_Mycont != NULL\", 0)"))||(__debugbreak(), 0));
						::_invalid_parameter(L"\"invalid argument\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 170, 0);
					};
				};

				{
					if(!(_Myptr+_Off<=(((_Mystring*)this->_Mycont)->_Myptr()+((_Mystring*)this->_Mycont)->_Mysize)&&_Myptr+_Off>=((_Mystring*)this->_Mycont)->_Myptr()))
					{
						(void)((!!((("_Myptr + _Off <= (((_Mystring *)this->_Mycont)->_Myptr() + ((_Mystring *)this->_Mycont)->_Mysize) && _Myptr + _Off >= ((_Mystring *)this->_Mycont)->_Myptr()", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 171, 0, L"(\"_Myptr + _Off <= (((_Mystring *)this->_Mycont)->_Myptr() + ((_Mystring *)this->_Mycont)->_Mysize) && _Myptr + _Off >= ((_Mystring *)this->_Mycont)->_Myptr()\", 0)"))||(__debugbreak(), 0));
						::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 171, 0);
					};
				};
			}
			_Myptr+=_Off;
			return (*this);
		}
		_Myt operator+(difference_type _Off)const
		{
			_Myt _Tmp=*this;
			return (_Tmp+=_Off);
		}
		_Myt&operator-=(difference_type _Off)
		{
			return (*this+=-_Off);
		}
		_Myt operator-(difference_type _Off)const
		{
			_Myt _Tmp=*this;
			return (_Tmp-=_Off);
		}
		difference_type operator-(const _Myt&_Right)const
		{
			_Compat(_Right);
			return (_Myptr-_Right._Myptr);
		}
		reference operator[](difference_type _Off)const
		{
			return (*(*this+_Off));
		}
		bool operator==(const _Myt&_Right)const
		{
			_Compat(_Right);
			return (_Myptr==_Right._Myptr);
		}
		bool operator!=(const _Myt&_Right)const
		{
			return (!(*this==_Right));
		}
		bool operator<(const _Myt&_Right)const
		{
			_Compat(_Right);
			return (_Myptr<_Right._Myptr);
		}
		bool operator>(const _Myt&_Right)const
		{
			return (_Right<*this);
		}
		bool operator<=(const _Myt&_Right)const
		{
			return (!(_Right<*this));
		}
		bool operator>=(const _Myt&_Right)const
		{
			return (!(*this<_Right));
		}
		void _Compat(const _Myt&_Right)const
		{
			if(this->_Mycont==0||this->_Mycont!=_Right._Mycont)
			{
				_Debug_message(L"string iterators incompatible", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 271);

				{
					(void)((!!((("Standard C++ Libraries Invalid Argument", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 272, 0, L"(\"Standard C++ Libraries Invalid Argument\", 0)"))||(__debugbreak(), 0));
					::_invalid_parameter(L"\"invalid argument\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 272, 0);
				};
			}
		}
		static void _Xlen()
		{
			_Mystring::_Xlen();
		}
		static void _Xran()
		{
			_Mystring::_Xran();
		}
		static void _Xinvarg()
		{
			_Mystring::_Xinvarg();
		}
		pointer _Myptr;
	};
	template<class _Elem, class _Traits, class _Alloc>
	inline _String_const_iterator<_Elem, _Traits, _Alloc>operator+(typename _String_const_iterator<_Elem, _Traits, _Alloc>::difference_type _Off, _String_const_iterator<_Elem, _Traits, _Alloc>_Next)
	{
		return (_Next+=_Off);
	}
	template<class _Elem, class _Traits, class _Alloc>
	class _String_iterator: public _String_const_iterator<_Elem, _Traits, _Alloc>
	{
	public:
		typedef _String_iterator<_Elem, _Traits, _Alloc>_Myt;
		typedef _String_const_iterator<_Elem, _Traits, _Alloc>_Mybase;
		typedef random_access_iterator_tag iterator_category;
		typedef _Elem value_type;
		typedef typename _Alloc::difference_type difference_type;
		typedef typename _Alloc::pointer pointer;
		typedef typename _Alloc::reference reference;
		_String_iterator()
		{
		}
		_String_iterator(pointer _Ptr, const _Container_base*_Pstring): _Mybase(_Ptr, _Pstring)
		{
		}
		reference operator*()const
		{
			return ((reference)**(_Mybase*)this);
		}
		pointer operator->()const
		{
			return (&**this);
		}
		_Myt&operator++()
		{
			++(*(_Mybase*)this);
			return (*this);
		}
		_Myt operator++(int)
		{
			_Myt _Tmp=*this;
			++*this;
			return (_Tmp);
		}
		_Myt&operator--()
		{
			--(*(_Mybase*)this);
			return (*this);
		}
		_Myt operator--(int)
		{
			_Myt _Tmp=*this;
			--*this;
			return (_Tmp);
		}
		_Myt&operator+=(difference_type _Off)
		{
			(*(_Mybase*)this)+=_Off;
			return (*this);
		}
		_Myt operator+(difference_type _Off)const
		{
			_Myt _Tmp=*this;
			return (_Tmp+=_Off);
		}
		_Myt&operator-=(difference_type _Off)
		{
			return (*this+=-_Off);
		}
		_Myt operator-(difference_type _Off)const
		{
			_Myt _Tmp=*this;
			return (_Tmp-=_Off);
		}
		difference_type operator-(const _Mybase&_Right)const
		{
			return ((_Mybase)*this-_Right);
		}
		reference operator[](difference_type _Off)const
		{
			return (*(*this+_Off));
		}
	};
	template<class _Elem, class _Traits, class _Alloc>
	inline _String_iterator<_Elem, _Traits, _Alloc>operator+(typename _String_iterator<_Elem, _Traits, _Alloc>::difference_type _Off, _String_iterator<_Elem, _Traits, _Alloc>_Next)
	{
		return (_Next+=_Off);
	}
	class _String_base: public _Container_base
	{
	public:
		static void _Xlen();
		static void _Xran();
		static void _Xinvarg();
	};
	template<class _Ty, class _Alloc>
	class _String_val: public _String_base
	{
	protected: typedef typename _Alloc::template rebind<_Ty>::other _Alty;
			   _String_val(_Alty _Al=_Alty()): _Alval(_Al)
			   {
			   }
			   _Alty _Alval;
	};
	template<class _Elem, class _Traits, class _Ax>
	class basic_string: public _String_val<_Elem, _Ax>
	{
	public:
		typedef basic_string<_Elem, _Traits, _Ax>_Myt;
		typedef _String_val<_Elem, _Ax>_Mybase;
		typedef typename _Mybase::_Alty _Alloc;
		typedef typename _Alloc::size_type size_type;
		typedef typename _Alloc::difference_type _Dift;
		typedef _Dift difference_type;
		typedef typename _Alloc::pointer _Tptr;
		typedef typename _Alloc::const_pointer _Ctptr;
		typedef _Tptr pointer;
		typedef _Ctptr const_pointer;
		typedef typename _Alloc::reference _Reft;
		typedef _Reft reference;
		typedef typename _Alloc::const_reference const_reference;
		typedef typename _Alloc::value_type value_type;
		typedef _String_iterator<_Elem, _Traits, _Alloc>iterator;
		typedef _String_const_iterator<_Elem, _Traits, _Alloc>const_iterator;
		friend class _String_const_iterator<_Elem, _Traits, _Alloc>;
		typedef std::reverse_iterator<iterator>reverse_iterator;
		typedef std::reverse_iterator<const_iterator>const_reverse_iterator;
		basic_string(): _Mybase()
		{
			_Tidy();
		}
		explicit basic_string(const _Alloc&_Al): _Mybase(_Al)
		{
			_Tidy();
		}
		basic_string(const _Myt&_Right): _Mybase(_Right._Alval)
		{
			_Tidy();
			assign(_Right, 0, npos);
		}
		basic_string(const _Myt&_Right, size_type _Roff, size_type _Count=npos): _Mybase()
		{
			_Tidy();
			assign(_Right, _Roff, _Count);
		}
		basic_string(const _Myt&_Right, size_type _Roff, size_type _Count, const _Alloc&_Al): _Mybase(_Al)
		{
			_Tidy();
			assign(_Right, _Roff, _Count);
		}
		basic_string(const _Elem*_Ptr, size_type _Count): _Mybase()
		{
			_Tidy();
			assign(_Ptr, _Count);
		}
		basic_string(const _Elem*_Ptr, size_type _Count, const _Alloc&_Al): _Mybase(_Al)
		{
			_Tidy();
			assign(_Ptr, _Count);
		}
		basic_string(const _Elem*_Ptr): _Mybase()
		{
			_Tidy();
			assign(_Ptr);
		}
		basic_string(const _Elem*_Ptr, const _Alloc&_Al): _Mybase(_Al)
		{
			_Tidy();
			assign(_Ptr);
		}
		basic_string(size_type _Count, _Elem _Ch): _Mybase()
		{
			_Tidy();
			assign(_Count, _Ch);
		}
		basic_string(size_type _Count, _Elem _Ch, const _Alloc&_Al): _Mybase(_Al)
		{
			_Tidy();
			assign(_Count, _Ch);
		}
		template<class _It>
		basic_string(_It _First, _It _Last): _Mybase()
		{
			_Tidy();
			_Construct(_First, _Last, _Iter_cat(_First));
		}
		template<class _It>
		basic_string(_It _First, _It _Last, const _Alloc&_Al): _Mybase(_Al)
		{
			_Tidy();
			_Construct(_First, _Last, _Iter_cat(_First));
		}
		template<class _It>
		void _Construct(_It _Count, _It _Ch, _Int_iterator_tag)
		{
			assign((size_type)_Count, (_Elem)_Ch);
		}
		template<class _It>
		void _Construct(_It _First, _It _Last, input_iterator_tag)
		{
			try
			{
				for(;
					_First!=_Last;
					++_First)append((size_type)1, (_Elem)*_First);
			}
			catch(...)
			{
				_Tidy(true);
				throw;
			}
		}
		template<class _It>
		void _Construct(_It _First, _It _Last, forward_iterator_tag)
		{
			if(this->_Myfirstiter!=((_Iterator_base*)-3))
			{
				_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 819);
			}
			size_type _Count=0;
			_Distance(_First, _Last, _Count);
			reserve(_Count);
			try
			{
				for(;
					_First!=_Last;
					++_First)append((size_type)1, (_Elem)*_First);
			}
			catch(...)
			{
				_Tidy(true);
				throw;
			}
		}
		basic_string(const_pointer _First, const_pointer _Last): _Mybase()
		{
			if(this->_Myfirstiter!=((_Iterator_base*)-3))
			{
				_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 842);
			}
			_Tidy();
			if(_First!=_Last)assign(&*_First, _Last-_First);
		}
		basic_string(const_iterator _First, const_iterator _Last): _Mybase()
		{
			if(this->_Myfirstiter!=((_Iterator_base*)-3))
			{
				_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 857);
			}
			_Tidy();
			if(_First!=_Last)assign(&*_First, _Last-_First);
		}
		~basic_string()
		{
			_Tidy(true);
		}
		typedef _Traits traits_type;
		typedef _Alloc allocator_type;
		static const size_type npos;
		_Myt&operator=(const _Myt&_Right)
		{
			return (assign(_Right));
		}
		_Myt&operator=(const _Elem*_Ptr)
		{
			return (assign(_Ptr));
		}
		_Myt&operator=(_Elem _Ch)
		{
			return (assign(1, _Ch));
		}
		_Myt&operator+=(const _Myt&_Right)
		{
			return (append(_Right));
		}
		_Myt&operator+=(const _Elem*_Ptr)
		{
			return (append(_Ptr));
		}
		_Myt&operator+=(_Elem _Ch)
		{
			return (append((size_type)1, _Ch));
		}
		_Myt&append(const _Myt&_Right)
		{
			return (append(_Right, 0, npos));
		}
		_Myt&append(const _Myt&_Right, size_type _Roff, size_type _Count)
		{
			if(_Right.size()<_Roff)_String_base::_Xran();
			size_type _Num=_Right.size()-_Roff;
			if(_Num<_Count)_Count=_Num;
			if(npos-_Mysize<=_Count||_Mysize+_Count<_Mysize)_String_base::_Xlen();
			if(0<_Count&&_Grow(_Num=_Mysize+_Count))
			{
				_Traits_helper::copy_s<_Traits>(_Myptr()+_Mysize, _Myres-_Mysize, _Right._Myptr()+_Roff, _Count);
				_Eos(_Num);
			}
			return (*this);
		}
		_Myt&append(const _Elem*_Ptr, size_type _Count)
		{
			if(_Inside(_Ptr))return (append(*this, _Ptr-_Myptr(), _Count));
			if(npos-_Mysize<=_Count||_Mysize+_Count<_Mysize)_String_base::_Xlen();
			size_type _Num;
			if(0<_Count&&_Grow(_Num=_Mysize+_Count))
			{
				_Traits_helper::copy_s<_Traits>(_Myptr()+_Mysize, _Myres-_Mysize, _Ptr, _Count);
				_Eos(_Num);
			}
			return (*this);
		}
		_Myt&append(const _Elem*_Ptr)
		{
			return (append(_Ptr, _Traits::length(_Ptr)));
		}
		_Myt&append(size_type _Count, _Elem _Ch)
		{
			if(npos-_Mysize<=_Count)_String_base::_Xlen();
			size_type _Num;
			if(0<_Count&&_Grow(_Num=_Mysize+_Count))
			{
				_Chassign(_Mysize, _Count, _Ch);
				_Eos(_Num);
			}
			return (*this);
		}
		template<class _It>
		_Myt&append(_It _First, _It _Last)
		{
			return (_Append(_First, _Last, _Iter_cat(_First)));
		}
		template<class _It>
		_Myt&_Append(_It _Count, _It _Ch, _Int_iterator_tag)
		{
			return (append((size_type)_Count, (_Elem)_Ch));
		}
		template<class _It>
		_Myt&_Append(_It _First, _It _Last, input_iterator_tag)
		{
			return (replace(end(), end(), _First, _Last));
		}
		_Myt&append(const_pointer _First, const_pointer _Last)
		{
			return (replace(end(), end(), _First, _Last));
		}
		_Myt&append(const_iterator _First, const_iterator _Last)
		{
			return (replace(end(), end(), _First, _Last));
		}
		_Myt&assign(const _Myt&_Right)
		{
			return (assign(_Right, 0, npos));
		}
		_Myt&assign(const _Myt&_Right, size_type _Roff, size_type _Count)
		{
			if(_Right.size()<_Roff)_String_base::_Xran();
			size_type _Num=_Right.size()-_Roff;
			if(_Count<_Num)_Num=_Count;
			if(this==&_Right)erase((size_type)(_Roff+_Num)), erase(0, _Roff);
			else if(_Grow(_Num))
			{
				_Traits_helper::copy_s<_Traits>(_Myptr(), _Myres, _Right._Myptr()+_Roff, _Num);
				_Eos(_Num);
			}
			return (*this);
		}
		_Myt&assign(const _Elem*_Ptr, size_type _Num)
		{
			if(_Inside(_Ptr))return (assign(*this, _Ptr-_Myptr(), _Num));
			if(_Grow(_Num))
			{
				_Traits_helper::copy_s<_Traits>(_Myptr(), _Myres, _Ptr, _Num);
				_Eos(_Num);
			}
			return (*this);
		}
		_Myt&assign(const _Elem*_Ptr)
		{
			return (assign(_Ptr, _Traits::length(_Ptr)));
		}
		_Myt&assign(size_type _Count, _Elem _Ch)
		{
			if(_Count==npos)_String_base::_Xlen();
			if(_Grow(_Count))
			{
				_Chassign(0, _Count, _Ch);
				_Eos(_Count);
			}
			return (*this);
		}
		template<class _It>
		_Myt&assign(_It _First, _It _Last)
		{
			return (_Assign(_First, _Last, _Iter_cat(_First)));
		}
		template<class _It>
		_Myt&_Assign(_It _Count, _It _Ch, _Int_iterator_tag)
		{
			return (assign((size_type)_Count, (_Elem)_Ch));
		}
		template<class _It>
		_Myt&_Assign(_It _First, _It _Last, input_iterator_tag)
		{
			return (replace(begin(), end(), _First, _Last));
		}
		_Myt&assign(const_pointer _First, const_pointer _Last)
		{
			return (replace(begin(), end(), _First, _Last));
		}
		_Myt&assign(const_iterator _First, const_iterator _Last)
		{
			return (replace(begin(), end(), _First, _Last));
		}
		_Myt&insert(size_type _Off, const _Myt&_Right)
		{
			return (insert(_Off, _Right, 0, npos));
		}
		_Myt&insert(size_type _Off, const _Myt&_Right, size_type _Roff, size_type _Count)
		{
			if(_Mysize<_Off||_Right.size()<_Roff)_String_base::_Xran();
			size_type _Num=_Right.size()-_Roff;
			if(_Num<_Count)_Count=_Num;
			if(npos-_Mysize<=_Count)_String_base::_Xlen();
			if(0<_Count&&_Grow(_Num=_Mysize+_Count))
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off, _Mysize-_Off);
				if(this==&_Right)_Traits_helper::move_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Myptr()+(_Off<_Roff?_Roff+_Count: _Roff), _Count);
				else _Traits_helper::copy_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Right._Myptr()+_Roff, _Count);
				_Eos(_Num);
			}
			return (*this);
		}
		_Myt&insert(size_type _Off, const _Elem*_Ptr, size_type _Count)
		{
			if(_Inside(_Ptr))return (insert(_Off, *this, _Ptr-_Myptr(), _Count));
			if(_Mysize<_Off)_String_base::_Xran();
			if(npos-_Mysize<=_Count)_String_base::_Xlen();
			size_type _Num;
			if(0<_Count&&_Grow(_Num=_Mysize+_Count))
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off, _Mysize-_Off);
				_Traits_helper::copy_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Ptr, _Count);
				_Eos(_Num);
			}
			return (*this);
		}
		_Myt&insert(size_type _Off, const _Elem*_Ptr)
		{
			return (insert(_Off, _Ptr, _Traits::length(_Ptr)));
		}
		_Myt&insert(size_type _Off, size_type _Count, _Elem _Ch)
		{
			if(_Mysize<_Off)_String_base::_Xran();
			if(npos-_Mysize<=_Count)_String_base::_Xlen();
			size_type _Num;
			if(0<_Count&&_Grow(_Num=_Mysize+_Count))
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off, _Mysize-_Off);
				_Chassign(_Off, _Count, _Ch);
				_Eos(_Num);
			}
			return (*this);
		}
		iterator insert(iterator _Where)
		{
			return (insert(_Where, _Elem()));
		}
		iterator insert(iterator _Where, _Elem _Ch)
		{
			size_type _Off=_Pdif(_Where, begin());
			insert(_Off, 1, _Ch);
			return (begin()+_Off);
		}
		void insert(iterator _Where, size_type _Count, _Elem _Ch)
		{
			size_type _Off=_Pdif(_Where, begin());
			insert(_Off, _Count, _Ch);
		}
		template<class _It>
		void insert(iterator _Where, _It _First, _It _Last)
		{
			_Insert(_Where, _First, _Last, _Iter_cat(_First));
		}
		template<class _It>
		void _Insert(iterator _Where, _It _Count, _It _Ch, _Int_iterator_tag)
		{
			insert(_Where, (size_type)_Count, (_Elem)_Ch);
		}
		template<class _It>
		void _Insert(iterator _Where, _It _First, _It _Last, input_iterator_tag)
		{
			replace(_Where, _Where, _First, _Last);
		}
		void insert(iterator _Where, const_pointer _First, const_pointer _Last)
		{
			replace(_Where, _Where, _First, _Last);
		}
		void insert(iterator _Where, const_iterator _First, const_iterator _Last)
		{
			replace(_Where, _Where, _First, _Last);
		}
		_Myt&erase(size_type _Off=0, size_type _Count=npos)
		{
			if(_Mysize<_Off)_String_base::_Xran();
			if(_Mysize-_Off<_Count)_Count=_Mysize-_Off;
			if(0<_Count)
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Myptr()+_Off+_Count, _Mysize-_Off-_Count);
				size_type _Newsize=_Mysize-_Count;
				_Eos(_Newsize);
			}
			return (*this);
		}
		iterator erase(iterator _Where)
		{
			size_type _Count=_Pdif(_Where, begin());
			erase(_Count, 1);
			return (iterator(_Myptr()+_Count, this));
		}
		iterator erase(iterator _First, iterator _Last)
		{
			size_type _Count=_Pdif(_First, begin());
			erase(_Count, _Pdif(_Last, _First));
			return (iterator(_Myptr()+_Count, this));
		}
		void clear()
		{
			erase(begin(), end());
		}
		_Myt&replace(size_type _Off, size_type _N0, const _Myt&_Right)
		{
			return (replace(_Off, _N0, _Right, 0, npos));
		}
		_Myt&replace(size_type _Off, size_type _N0, const _Myt&_Right, size_type _Roff, size_type _Count)
		{
			if(_Mysize<_Off||_Right.size()<_Roff)_String_base::_Xran();
			if(_Mysize-_Off<_N0)_N0=_Mysize-_Off;
			size_type _Num=_Right.size()-_Roff;
			if(_Num<_Count)_Count=_Num;
			if(npos-_Count<=_Mysize-_N0)_String_base::_Xlen();
			size_type _Nm=_Mysize-_N0-_Off;
			size_type _Newsize=_Mysize+_Count-_N0;
			if(_Mysize<_Newsize)_Grow(_Newsize);
			if(this!=&_Right)
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
				_Traits_helper::copy_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Right._Myptr()+_Roff, _Count);
			}
			else if(_Count<=_N0)
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Myptr()+_Roff, _Count);
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
			}
			else if(_Roff<=_Off)
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Myptr()+_Roff, _Count);
			}
			else if(_Off+_N0<=_Roff)
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Myptr()+(_Roff+_Count-_N0), _Count);
			}
			else
			{
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Myptr()+_Roff, _N0);
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
				_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_N0, _Myres-_Off-_N0, _Myptr()+_Roff+_Count, _Count-_N0);
			}
			_Eos(_Newsize);
			return (*this);
		}
		_Myt&replace(size_type _Off, size_type _N0, const _Elem*_Ptr, size_type _Count)
		{
			if(_Inside(_Ptr))return (replace(_Off, _N0, *this, _Ptr-_Myptr(), _Count));
			if(_Mysize<_Off)_String_base::_Xran();
			if(_Mysize-_Off<_N0)_N0=_Mysize-_Off;
			if(npos-_Count<=_Mysize-_N0)_String_base::_Xlen();
			size_type _Nm=_Mysize-_N0-_Off;
			if(_Count<_N0)_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
			size_type _Num;
			if((0<_Count||0<_N0)&&_Grow(_Num=_Mysize+_Count-_N0))
			{
				if(_N0<_Count)_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
				_Traits_helper::copy_s<_Traits>(_Myptr()+_Off, _Myres-_Off, _Ptr, _Count);
				_Eos(_Num);
			}
			return (*this);
		}
		_Myt&replace(size_type _Off, size_type _N0, const _Elem*_Ptr)
		{
			return (replace(_Off, _N0, _Ptr, _Traits::length(_Ptr)));
		}
		_Myt&replace(size_type _Off, size_type _N0, size_type _Count, _Elem _Ch)
		{
			if(_Mysize<_Off)_String_base::_Xran();
			if(_Mysize-_Off<_N0)_N0=_Mysize-_Off;
			if(npos-_Count<=_Mysize-_N0)_String_base::_Xlen();
			size_type _Nm=_Mysize-_N0-_Off;
			if(_Count<_N0)_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
			size_type _Num;
			if((0<_Count||0<_N0)&&_Grow(_Num=_Mysize+_Count-_N0))
			{
				if(_N0<_Count)_Traits_helper::move_s<_Traits>(_Myptr()+_Off+_Count, _Myres-_Off-_Count, _Myptr()+_Off+_N0, _Nm);
				_Chassign(_Off, _Count, _Ch);
				_Eos(_Num);
			}
			return (*this);
		}
		_Myt&replace(iterator _First, iterator _Last, const _Myt&_Right)
		{
			return (replace(_Pdif(_First, begin()), _Pdif(_Last, _First), _Right));
		}
		_Myt&replace(iterator _First, iterator _Last, const _Elem*_Ptr, size_type _Count)
		{
			return (replace(_Pdif(_First, begin()), _Pdif(_Last, _First), _Ptr, _Count));
		}
		_Myt&replace(iterator _First, iterator _Last, const _Elem*_Ptr)
		{
			return (replace(_Pdif(_First, begin()), _Pdif(_Last, _First), _Ptr));
		}
		_Myt&replace(iterator _First, iterator _Last, size_type _Count, _Elem _Ch)
		{
			return (replace(_Pdif(_First, begin()), _Pdif(_Last, _First), _Count, _Ch));
		}
		template<class _It>
		_Myt&replace(iterator _First, iterator _Last, _It _First2, _It _Last2)
		{
			return (_Replace(_First, _Last, _First2, _Last2, _Iter_cat(_First2)));
		}
		template<class _It>
		_Myt&_Replace(iterator _First, iterator _Last, _It _Count, _It _Ch, _Int_iterator_tag)
		{
			return (replace(_First, _Last, (size_type)_Count, (_Elem)_Ch));
		}
		template<class _It>
		_Myt&_Replace(iterator _First, iterator _Last, _It _First2, _It _Last2, input_iterator_tag)
		{
			_Myt _Right(_First2, _Last2);
			replace(_First, _Last, _Right);
			return (*this);
		}
		_Myt&replace(iterator _First, iterator _Last, const_pointer _First2, const_pointer _Last2)
		{
			if(_First2==_Last2)erase(_Pdif(_First, begin()), _Pdif(_Last, _First));
			else replace(_Pdif(_First, begin()), _Pdif(_Last, _First), &*_First2, _Last2-_First2);
			return (*this);
		}
		_Myt&replace(iterator _First, iterator _Last, const_iterator _First2, const_iterator _Last2)
		{
			if(_First2==_Last2)erase(_Pdif(_First, begin()), _Pdif(_Last, _First));
			else replace(_Pdif(_First, begin()), _Pdif(_Last, _First), &*_First2, _Last2-_First2);
			return (*this);
		}
		iterator begin()
		{
			return (iterator(_Myptr(), this));
		}
		const_iterator begin()const
		{
			return (const_iterator(_Myptr(), this));
		}
		iterator end()
		{
			return (iterator(_Myptr()+_Mysize, this));
		}
		const_iterator end()const
		{
			return (const_iterator(_Myptr()+_Mysize, this));
		}
		reverse_iterator rbegin()
		{
			return (reverse_iterator(end()));
		}
		const_reverse_iterator rbegin()const
		{
			return (const_reverse_iterator(end()));
		}
		reverse_iterator rend()
		{
			return (reverse_iterator(begin()));
		}
		const_reverse_iterator rend()const
		{
			return (const_reverse_iterator(begin()));
		}
		reference at(size_type _Off)
		{
			if(_Mysize<=_Off)_String_base::_Xran();
			return (_Myptr()[_Off]);
		}
		const_reference at(size_type _Off)const
		{
			if(_Mysize<=_Off)_String_base::_Xran();
			return (_Myptr()[_Off]);
		}
		reference operator[](size_type _Off)
		{
			if(this->_Myfirstiter!=((_Iterator_base*)-3))
			{
				if(_Mysize<_Off)
				{
					_Debug_message(L"string subscript out of range", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1498);

					{
						(void)((!!((("Standard C++ Libraries Out of Range", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1499, 0, L"(\"Standard C++ Libraries Out of Range\", 0)"))||(__debugbreak(), 0));
						::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1499, 0);
					};
				}
			}
			return (_Myptr()[_Off]);
		}
		const_reference operator[](size_type _Off)const
		{
			if(this->_Myfirstiter!=((_Iterator_base*)-3))
			{
				if(_Mysize<_Off)
				{
					_Debug_message(L"string subscript out of range", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1518);

					{
						(void)((!!((("Standard C++ Libraries Out of Range", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1519, 0, L"(\"Standard C++ Libraries Out of Range\", 0)"))||(__debugbreak(), 0));
						::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1519, 0);
					};
				}
			}
			return (_Myptr()[_Off]);
		}
		void push_back(_Elem _Ch)
		{
			insert(end(), _Ch);
		}
		const _Elem*c_str()const
		{
			return (_Myptr());
		}
		const _Elem*data()const
		{
			return (c_str());
		}
		size_type length()const
		{
			return (_Mysize);
		}
		size_type size()const
		{
			return (_Mysize);
		}
		size_type max_size()const
		{
			size_type _Num=_Mybase::_Alval.max_size();
			return (_Num<=1?1: _Num-1);
		}
		void resize(size_type _Newsize)
		{
			resize(_Newsize, _Elem());
		}
		void resize(size_type _Newsize, _Elem _Ch)
		{
			if(_Newsize<=_Mysize)erase(_Newsize);
			else append(_Newsize-_Mysize, _Ch);
		}
		size_type capacity()const
		{
			return (_Myres);
		}
		void reserve(size_type _Newcap=0)
		{
			if(_Mysize<=_Newcap&&_Myres!=_Newcap)
			{
				size_type _Size=_Mysize;
				if(_Grow(_Newcap, true))_Eos(_Size);
			}
		}
		bool empty()const
		{
			return (_Mysize==0);
		}
		size_type copy(_Elem*_Dest, size_type _Count, size_type _Off=0)const
		{
			return _Copy_s(_Dest, _Count, _Count, _Off);
		}
		size_type _Copy_s(_Elem*_Dest, size_type _Dest_size, size_type _Count, size_type _Off=0)const
		{
			_Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1652);
			if(_Mysize<_Off)_String_base::_Xran();
			if(_Mysize-_Off<_Count)_Count=_Mysize-_Off;
			_Traits_helper::copy_s<_Traits>(_Dest, _Dest_size, _Myptr()+_Off, _Count);
			return (_Count);
		}
		void swap(_Myt&_Right)
		{
			if(_Mybase::_Alval==_Right._Alval)
			{
				this->_Swap_all(_Right);
				_Bxty _Tbx=_Bx;
				_Bx=_Right._Bx, _Right._Bx=_Tbx;
				size_type _Tlen=_Mysize;
				_Mysize=_Right._Mysize, _Right._Mysize=_Tlen;
				size_type _Tres=_Myres;
				_Myres=_Right._Myres, _Right._Myres=_Tres;
			}
			else
			{
				_Myt _Tmp=*this;
				*this=_Right, _Right=_Tmp;
			}
		}
		size_type find(const _Myt&_Right, size_type _Off=0)const
		{
			return (find(_Right._Myptr(), _Off, _Right.size()));
		}
		size_type find(const _Elem*_Ptr, size_type _Off, size_type _Count)const
		{
			_Debug_pointer(_Ptr, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1693);
			if(_Count==0&&_Off<=_Mysize)return (_Off);
			size_type _Nm;
			if(_Off<_Mysize&&_Count<=(_Nm=_Mysize-_Off))
			{
				const _Elem*_Uptr, *_Vptr;
				for(_Nm-=_Count-1, _Vptr=_Myptr()+_Off;
					(_Uptr=_Traits::find(_Vptr, _Nm, *_Ptr))!=0;
					_Nm-=_Uptr-_Vptr+1, _Vptr=_Uptr+1)if(_Traits::compare(_Uptr, _Ptr, _Count)==0)return (_Uptr-_Myptr());
			}
			return (npos);
		}
		size_type find(const _Elem*_Ptr, size_type _Off=0)const
		{
			return (find(_Ptr, _Off, _Traits::length(_Ptr)));
		}
		size_type find(_Elem _Ch, size_type _Off=0)const
		{
			return (find((const _Elem*)&_Ch, _Off, 1));
		}
		size_type rfind(const _Myt&_Right, size_type _Off=npos)const
		{
			return (rfind(_Right._Myptr(), _Off, _Right.size()));
		}
		size_type rfind(const _Elem*_Ptr, size_type _Off, size_type _Count)const
		{
			_Debug_pointer(_Ptr, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1729);
			if(_Count==0)return (_Off<_Mysize?_Off: _Mysize);
			if(_Count<=_Mysize)
			{
				const _Elem*_Uptr=_Myptr()+(_Off<_Mysize-_Count?_Off: _Mysize-_Count);
				for(;
					;
					--_Uptr)if(_Traits::eq(*_Uptr, *_Ptr)&&_Traits::compare(_Uptr, _Ptr, _Count)==0)return (_Uptr-_Myptr());
					else if(_Uptr==_Myptr())break;
			}
			return (npos);
		}
		size_type rfind(const _Elem*_Ptr, size_type _Off=npos)const
		{
			return (rfind(_Ptr, _Off, _Traits::length(_Ptr)));
		}
		size_type rfind(_Elem _Ch, size_type _Off=npos)const
		{
			return (rfind((const _Elem*)&_Ch, _Off, 1));
		}
		size_type find_first_of(const _Myt&_Right, size_type _Off=0)const
		{
			return (find_first_of(_Right._Myptr(), _Off, _Right.size()));
		}
		size_type find_first_of(const _Elem*_Ptr, size_type _Off, size_type _Count)const
		{
			_Debug_pointer(_Ptr, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1766);
			if(0<_Count&&_Off<_Mysize)
			{
				const _Elem*const _Vptr=_Myptr()+_Mysize;
				for(const _Elem*_Uptr=_Myptr()+_Off;
					_Uptr<_Vptr;
					++_Uptr)if(_Traits::find(_Ptr, _Count, *_Uptr)!=0)return (_Uptr-_Myptr());
			}
			return (npos);
		}
		size_type find_first_of(const _Elem*_Ptr, size_type _Off=0)const
		{
			return (find_first_of(_Ptr, _Off, _Traits::length(_Ptr)));
		}
		size_type find_first_of(_Elem _Ch, size_type _Off=0)const
		{
			return (find((const _Elem*)&_Ch, _Off, 1));
		}
		size_type find_last_of(const _Myt&_Right, size_type _Off=npos)const
		{
			return (find_last_of(_Right._Myptr(), _Off, _Right.size()));
		}
		size_type find_last_of(const _Elem*_Ptr, size_type _Off, size_type _Count)const
		{
			_Debug_pointer(_Ptr, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1797);
			if(0<_Count&&0<_Mysize)for(const _Elem*_Uptr=_Myptr()+(_Off<_Mysize?_Off: _Mysize-1);
			;
			--_Uptr)if(_Traits::find(_Ptr, _Count, *_Uptr)!=0)return (_Uptr-_Myptr());
			else if(_Uptr==_Myptr())break;
			return (npos);
		}
		size_type find_last_of(const _Elem*_Ptr, size_type _Off=npos)const
		{
			return (find_last_of(_Ptr, _Off, _Traits::length(_Ptr)));
		}
		size_type find_last_of(_Elem _Ch, size_type _Off=npos)const
		{
			return (rfind((const _Elem*)&_Ch, _Off, 1));
		}
		size_type find_first_not_of(const _Myt&_Right, size_type _Off=0)const
		{
			return (find_first_not_of(_Right._Myptr(), _Off, _Right.size()));
		}
		size_type find_first_not_of(const _Elem*_Ptr, size_type _Off, size_type _Count)const
		{
			_Debug_pointer(_Ptr, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1830);
			if(_Off<_Mysize)
			{
				const _Elem*const _Vptr=_Myptr()+_Mysize;
				for(const _Elem*_Uptr=_Myptr()+_Off;
					_Uptr<_Vptr;
					++_Uptr)if(_Traits::find(_Ptr, _Count, *_Uptr)==0)return (_Uptr-_Myptr());
			}
			return (npos);
		}
		size_type find_first_not_of(const _Elem*_Ptr, size_type _Off=0)const
		{
			return (find_first_not_of(_Ptr, _Off, _Traits::length(_Ptr)));
		}
		size_type find_first_not_of(_Elem _Ch, size_type _Off=0)const
		{
			return (find_first_not_of((const _Elem*)&_Ch, _Off, 1));
		}
		size_type find_last_not_of(const _Myt&_Right, size_type _Off=npos)const
		{
			return (find_last_not_of(_Right._Myptr(), _Off, _Right.size()));
		}
		size_type find_last_not_of(const _Elem*_Ptr, size_type _Off, size_type _Count)const
		{
			_Debug_pointer(_Ptr, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1861);
			if(0<_Mysize)for(const _Elem*_Uptr=_Myptr()+(_Off<_Mysize?_Off: _Mysize-1);
			;
			--_Uptr)if(_Traits::find(_Ptr, _Count, *_Uptr)==0)return (_Uptr-_Myptr());
			else if(_Uptr==_Myptr())break;
			return (npos);
		}
		size_type find_last_not_of(const _Elem*_Ptr, size_type _Off=npos)const
		{
			return (find_last_not_of(_Ptr, _Off, _Traits::length(_Ptr)));
		}
		size_type find_last_not_of(_Elem _Ch, size_type _Off=npos)const
		{
			return (find_last_not_of((const _Elem*)&_Ch, _Off, 1));
		}
		_Myt substr(size_type _Off=0, size_type _Count=npos)const
		{
			return (_Myt(*this, _Off, _Count));
		}
		int compare(const _Myt&_Right)const
		{
			return (compare(0, _Mysize, _Right._Myptr(), _Right.size()));
		}
		int compare(size_type _Off, size_type _N0, const _Myt&_Right)const
		{
			return (compare(_Off, _N0, _Right, 0, npos));
		}
		int compare(size_type _Off, size_type _N0, const _Myt&_Right, size_type _Roff, size_type _Count)const
		{
			if(_Right.size()<_Roff)_String_base::_Xran();
			if(_Right._Mysize-_Roff<_Count)_Count=_Right._Mysize-_Roff;
			return (compare(_Off, _N0, _Right._Myptr()+_Roff, _Count));
		}
		int compare(const _Elem*_Ptr)const
		{
			return (compare(0, _Mysize, _Ptr, _Traits::length(_Ptr)));
		}
		int compare(size_type _Off, size_type _N0, const _Elem*_Ptr)const
		{
			return (compare(_Off, _N0, _Ptr, _Traits::length(_Ptr)));
		}
		int compare(size_type _Off, size_type _N0, const _Elem*_Ptr, size_type _Count)const
		{
			_Debug_pointer(_Ptr, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 1923);
			if(_Mysize<_Off)_String_base::_Xran();
			if(_Mysize-_Off<_N0)_N0=_Mysize-_Off;
			size_type _Ans=_Traits::compare(_Myptr()+_Off, _Ptr, _N0<_Count?_N0: _Count);
			return (_Ans!=0?(int)_Ans: _N0<_Count?-1: _N0==_Count?0: +1);
		}
		allocator_type get_allocator()const
		{
			return (_Mybase::_Alval);
		}
		enum
		{
			_BUF_SIZE=16/sizeof(_Elem)<1?1: 16/sizeof(_Elem)
		};
	protected: enum
			   {
				   _ALLOC_MASK=sizeof(_Elem)<=1?15: sizeof(_Elem)<=2?7: sizeof(_Elem)<=4?3: sizeof(_Elem)<=8?1: 0
			   };
			   void _Chassign(size_type _Off, size_type _Count, _Elem _Ch)
			   {
				   if(_Count==1)_Traits::assign(*(_Myptr()+_Off), _Ch);
				   else _Traits::assign(_Myptr()+_Off, _Count, _Ch);
			   }
			   void _Copy(size_type _Newsize, size_type _Oldlen)
			   {
				   size_type _Newres=_Newsize|_ALLOC_MASK;
				   if(max_size()<_Newres)_Newres=_Newsize;
				   else if(_Newres/3<_Myres/2&&_Myres<=max_size()-_Myres/2)_Newres=_Myres+_Myres/2;
				   _Elem*_Ptr=0;
				   try
				   {
					   _Ptr=_Mybase::_Alval.allocate(_Newres+1);
				   }
				   catch(...)
				   {
					   _Newres=_Newsize;
					   try
					   {
						   _Ptr=_Mybase::_Alval.allocate(_Newres+1);
					   }
					   catch(...)
					   {
						   _Tidy(true);
						   throw;
					   }
				   }
				   if(0<_Oldlen)_Traits_helper::copy_s<_Traits>(_Ptr, _Newres+1, _Myptr(), _Oldlen);
				   _Tidy(true);
				   _Bx._Ptr=_Ptr;
				   _Myres=_Newres;
				   _Eos(_Oldlen);
			   }
			   void _Eos(size_type _Newsize)
			   {
				   _Traits::assign(_Myptr()[_Mysize=_Newsize], _Elem());
			   }
			   bool _Grow(size_type _Newsize, bool _Trim=false)
			   {
				   if(max_size()<_Newsize)_String_base::_Xlen();
				   if(_Myres<_Newsize)_Copy(_Newsize, _Mysize);
				   else if(_Trim&&_Newsize<_BUF_SIZE)_Tidy(true, _Newsize<_Mysize?_Newsize: _Mysize);
				   else if(_Newsize==0)_Eos(0);
				   return (0<_Newsize);
			   }
			   bool _Inside(const _Elem*_Ptr)
			   {
				   _Debug_pointer(_Ptr, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xstring", 2013);
				   if(_Ptr<_Myptr()||_Myptr()+_Mysize<=_Ptr)return (false);
				   else return (true);
			   }
			   static size_type _Pdif(const_iterator _P2, const_iterator _P1)
			   {
				   return ((_P2)._Myptr==0?0: _P2-_P1);
			   }
			   void _Tidy(bool _Built=false, size_type _Newsize=0)
			   {
				   if(!_Built);
				   else if(_BUF_SIZE<=_Myres)
				   {
					   _Elem*_Ptr=_Bx._Ptr;
					   if(0<_Newsize)_Traits_helper::copy_s<_Traits>(_Bx._Buf, _BUF_SIZE, _Ptr, _Newsize);
					   _Mybase::_Alval.deallocate(_Ptr, _Myres+1);
				   }
				   _Myres=_BUF_SIZE-1;
				   _Eos(_Newsize);
			   }
			   union _Bxty
			   {
				   _Elem _Buf[_BUF_SIZE];
				   _Elem*_Ptr;
			   }
			   _Bx;
			   _Elem*_Myptr()
			   {
				   return (_BUF_SIZE<=_Myres?_Bx._Ptr: _Bx._Buf);
			   }
			   const _Elem*_Myptr()const
			   {
				   return (_BUF_SIZE<=_Myres?_Bx._Ptr: _Bx._Buf);
			   }
			   size_type _Mysize;
			   size_type _Myres;
	};
	template<class _Elem, class _Traits, class _Ax>
	class _Move_operation_category<basic_string<_Elem, _Traits, _Ax> >
	{
	public:
		typedef _Swap_move_tag _Move_cat;
	};
	template<class _Elem, class _Traits, class _Alloc>
	const typename basic_string<_Elem, _Traits, _Alloc>::size_type basic_string<_Elem, _Traits, _Alloc>::npos=(typename basic_string<_Elem, _Traits, _Alloc>::size_type)(-1);
	template<class _Elem, class _Traits, class _Alloc>
	inline void swap(basic_string<_Elem, _Traits, _Alloc>&_Left, basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		_Left.swap(_Right);
	}
	typedef basic_string<char, char_traits<char>, allocator<char> >string;
	typedef basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >wstring;
}
namespace std
{
	class logic_error: public std::exception
	{
	public:
		explicit logic_error(const string&_Message): _Str(_Message)
		{
		}
		virtual~logic_error()throw()
		{
		}
		virtual const char*what()const throw()
		{
			return (_Str.c_str());
		}
	private:
		string _Str;
	};
	class domain_error: public logic_error
	{
	public:
		explicit domain_error(const string&_Message): logic_error(_Message)
		{
		}
		virtual~domain_error()throw()
		{
		}
	};
	class invalid_argument: public logic_error
	{
	public:
		explicit invalid_argument(const string&_Message): logic_error(_Message)
		{
		}
		virtual~invalid_argument()throw()
		{
		}
	};
	class length_error: public logic_error
	{
	public:
		explicit length_error(const string&_Message): logic_error(_Message)
		{
		}
		virtual~length_error()throw()
		{
		}
	};
	class out_of_range: public logic_error
	{
	public:
		explicit out_of_range(const string&_Message): logic_error(_Message)
		{
		}
		virtual~out_of_range()throw()
		{
		}
	};
	class runtime_error: public std::exception
	{
	public:
		explicit runtime_error(const string&_Message): _Str(_Message)
		{
		}
		virtual~runtime_error()throw()
		{
		}
		virtual const char*what()const throw()
		{
			return (_Str.c_str());
		}
	private:
		string _Str;
	};
	class overflow_error: public runtime_error
	{
	public:
		explicit overflow_error(const string&_Message): runtime_error(_Message)
		{
		}
		virtual~overflow_error()throw()
		{
		}
	};
	class underflow_error: public runtime_error
	{
	public:
		explicit underflow_error(const string&_Message): runtime_error(_Message)
		{
		}
		virtual~underflow_error()throw()
		{
		}
	};
	class range_error: public runtime_error
	{
	public:
		explicit range_error(const string&_Message): runtime_error(_Message)
		{
		}
		virtual~range_error()throw()
		{
		}
	};
}
struct __type_info_node
{
	void*memPtr;
	__type_info_node*next;
};
extern __type_info_node __type_info_root_node;
class type_info
{
public:
	virtual~type_info();
	bool operator==(const type_info&rhs)const;
	bool operator!=(const type_info&rhs)const;
	int before(const type_info&rhs)const;
	const char*name(__type_info_node*__ptype_info_node=&__type_info_root_node)const;
	const char*raw_name()const;
private:
	void*_m_data;
	char _m_d_name[1];
	type_info(const type_info&rhs);
	type_info&operator=(const type_info&rhs);
	static const char*_Name_base(const type_info*, __type_info_node*__ptype_info_node);
	static void _Type_info_dtor(type_info*);
};
namespace std
{
	using::type_info;
}
namespace std
{
	class bad_cast: public exception
	{
	public:
		bad_cast(const char*_Message="bad cast");
		bad_cast(const bad_cast&);
		virtual~bad_cast();
	};
	class bad_typeid: public exception
	{
	public:
		bad_typeid(const char*_Message="bad typeid");
		bad_typeid(const bad_typeid&);
		virtual~bad_typeid();
	};
	class __non_rtti_object: public bad_typeid
	{
	public:
		__non_rtti_object(const char*_Message);
		__non_rtti_object(const __non_rtti_object&);
		virtual~__non_rtti_object();
	};
}
namespace std
{
	struct _DebugHeapTag_t
	{
		int _Type;
	};
}
void*operator new(size_t _Size, const std::_DebugHeapTag_t&, char*, int)throw(...);
void*operator new[](size_t _Size, const std::_DebugHeapTag_t&, char*, int)throw(...);
void operator delete(void*, const std::_DebugHeapTag_t&, char*, int)throw();
void operator delete[](void*, const std::_DebugHeapTag_t&, char*, int)throw();
namespace std
{
	const _DebugHeapTag_t&_DebugHeapTag_func();
	template<class _Ty>
	void _DebugHeapDelete(_Ty*_Ptr)
	{
		if(_Ptr!=0)
		{
			_Ptr->~_Ty();
			free(_Ptr);
		}
	}
	template<class _Ty>
	class _DebugHeapAllocator: public allocator<_Ty>
	{
	public:
		template<class _Other>
		struct rebind
		{
			typedef typename _DebugHeapAllocator<_Other>other;
		};
		typename allocator<_Ty>::pointer allocate(typename allocator<_Ty>::size_type _Count, const void*)
		{
			if(_Count<=0)_Count=0;
			else if(((size_t)(-1)/_Count)<sizeof(_Ty))throw std::bad_alloc(0);
			return ((_Ty*)new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xdebug", 86)char[_Count*sizeof(_Ty)]);
		}
		typename allocator<_Ty>::pointer allocate(typename allocator<_Ty>::size_type _Count)
		{
			if(_Count<=0)_Count=0;
			else if(((size_t)(-1)/_Count)<sizeof(_Ty))throw std::bad_alloc(0);
			return ((_Ty*)new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xdebug", 97)char[_Count*sizeof(_Ty)]);
		}
		void deallocate(typename allocator<_Ty>::pointer _Ptr, typename allocator<_Ty>::size_type)
		{
			std::_DebugHeapDelete((void*)_Ptr);
		}
	};
	template class _DebugHeapAllocator<char>
		;
	class _DebugHeapString: public basic_string<char, char_traits<char>, _DebugHeapAllocator<char> >
	{
	public:
		typedef _DebugHeapString _Myt;
		typedef basic_string<char, char_traits<char>, _DebugHeapAllocator<char> >_Mybase;
		typedef char _Elem;
		_DebugHeapString(): _Mybase()
		{
		}
		_DebugHeapString(const _Myt&_Right): _Mybase(_Right)
		{
		}
		_DebugHeapString(const _Elem*_Ptr): _Mybase(_Ptr)
		{
		}
		_DebugHeapString(const string&_Str): _Mybase(_Str.c_str())
		{
		}
		operator string()const
		{
			return (string(c_str()));
		}
	};
}
extern "C"
{
	int _isctype(int _C, int _Type);
	int _isctype_l(int _C, int _Type, _locale_t _Locale);
	int isalpha(int _C);
	int _isalpha_l(int _C, _locale_t _Locale);
	int isupper(int _C);
	int _isupper_l(int _C, _locale_t _Locale);
	int islower(int _C);
	int _islower_l(int _C, _locale_t _Locale);
	int isdigit(int _C);
	int _isdigit_l(int _C, _locale_t _Locale);
	int isxdigit(int _C);
	int _isxdigit_l(int _C, _locale_t _Locale);
	int isspace(int _C);
	int _isspace_l(int _C, _locale_t _Locale);
	int ispunct(int _C);
	int _ispunct_l(int _C, _locale_t _Locale);
	int isalnum(int _C);
	int _isalnum_l(int _C, _locale_t _Locale);
	int isprint(int _C);
	int _isprint_l(int _C, _locale_t _Locale);
	int isgraph(int _C);
	int _isgraph_l(int _C, _locale_t _Locale);
	int iscntrl(int _C);
	int _iscntrl_l(int _C, _locale_t _Locale);
	int toupper(int _C);
	int tolower(int _C);
	int _tolower(int _C);
	int _tolower_l(int _C, _locale_t _Locale);
	int _toupper(int _C);
	int _toupper_l(int _C, _locale_t _Locale);
	int __isascii(int _C);
	int __toascii(int _C);
	int __iscsymf(int _C);
	int __iscsym(int _C);
	int _chvalidator(int _Ch, int _Mask);
	int _chvalidator_l(_locale_t, int _Ch, int _Mask);
}
extern "C"
{
	struct lconv
	{
		char*decimal_point;
		char*thousands_sep;
		char*grouping;
		char*int_curr_symbol;
		char*currency_symbol;
		char*mon_decimal_point;
		char*mon_thousands_sep;
		char*mon_grouping;
		char*positive_sign;
		char*negative_sign;
		char int_frac_digits;
		char frac_digits;
		char p_cs_precedes;
		char p_sep_by_space;
		char n_cs_precedes;
		char n_sep_by_space;
		char p_sign_posn;
		char n_sign_posn;
	};
	int _configthreadlocale(int _Flag);
	char*setlocale(int _Category, const char*_Locale);
	struct lconv*localeconv(void);
	_locale_t _get_current_locale(void);
	_locale_t _create_locale(int _Category, const char*_Locale);
	void _free_locale(_locale_t _Locale);
	_locale_t __get_current_locale(void);
	_locale_t __create_locale(int _Category, const char*_Locale);
	void __free_locale(_locale_t _Locale);
}
extern "C"
{
	typedef struct _Collvec
	{
		unsigned long _Hand;
		unsigned int _Page;
	}
	_Collvec;
	typedef struct _Ctypevec
	{
		unsigned long _Hand;
		unsigned int _Page;
		const short*_Table;
		int _Delfl;
	}
	_Ctypevec;
	typedef struct _Cvtvec
	{
		unsigned long _Hand;
		unsigned int _Page;
	}
	_Cvtvec;
	_Collvec _Getcoll();
	_Ctypevec _Getctype();
	_Cvtvec _Getcvt();
	int _Getdateorder();
	int _Mbrtowc(wchar_t*, const char*, size_t, mbstate_t*, const _Cvtvec*);
	float _Stof(const char*, char**, long);
	double _Stod(const char*, char**, long);
	long double _Stold(const char*, char**, long);
	int _Strcoll(const char*, const char*, const char*, const char*, const _Collvec*);
	size_t _Strxfrm(char*_String1, char*_End1, const char*, const char*, const _Collvec*);
	int _Tolower(int, const _Ctypevec*);
	int _Toupper(int, const _Ctypevec*);
	int _Wcrtomb(char*, wchar_t, mbstate_t*, const _Cvtvec*);
	int _Wcscoll(const wchar_t*, const wchar_t*, const wchar_t*, const wchar_t*, const _Collvec*);
	size_t _Wcsxfrm(wchar_t*_String1, wchar_t*_End1, const wchar_t*, const wchar_t*, const _Collvec*);
	short _Getwctype(wchar_t, const _Ctypevec*);
	const wchar_t*_Getwctypes(const wchar_t*, const wchar_t*, short*, const _Ctypevec*);
	wchar_t _Towlower(wchar_t, const _Ctypevec*);
	wchar_t _Towupper(wchar_t, const _Ctypevec*);
}
extern "C"
{
	void*_Gettnames();
	char*_Getdays();
	char*_Getmonths();
	size_t _Strftime(char*, size_t _Maxsize, const char*, const struct tm*, void*);
}
extern "C"
{
	_locale_t _GetLocaleForCP(unsigned int);
}
namespace std
{
	class _Timevec
	{
	public:
		_Timevec(void*_Ptr=0): _Timeptr(_Ptr)
		{
		}
		_Timevec(const _Timevec&_Right)
		{
			*this=_Right;
		}
		~_Timevec()
		{
			free(_Timeptr);
		}
		_Timevec&operator=(const _Timevec&_Right)
		{
			_Timeptr=_Right._Timeptr;
			((_Timevec*)&_Right)->_Timeptr=0;
			return (*this);
		}
		void*_Getptr()const
		{
			return (_Timeptr);
		}
	private:
		void*_Timeptr;
	};
	class _Locinfo
	{
	public:
		typedef ::_Collvec _Collvec;
		typedef ::_Ctypevec _Ctypevec;
		typedef ::_Cvtvec _Cvtvec;
		typedef std::_Timevec _Timevec;
		static void _Locinfo_ctor(_Locinfo*, const char*);
		static void _Locinfo_ctor(_Locinfo*, int, const char*);
		static void _Locinfo_dtor(_Locinfo*);
		static _Locinfo&_Locinfo_Addcats(_Locinfo*, int, const char*);
		_Locinfo(const char*_Pch="C"): _Lock(0)
		{
			if(_Pch==0)throw runtime_error("bad locale name");
			_Locinfo_ctor(this, _Pch);
		}
		_Locinfo(int _I, const char*_Pch): _Lock(0)
		{
			if(_Pch==0)throw runtime_error("bad locale name");
			_Locinfo_ctor(this, _I, _Pch);
		}
		~_Locinfo()
		{
			_Locinfo_dtor(this);
		}
		_Locinfo&_Addcats(int _I, const char*_Pch)
		{
			if(_Pch==0)throw runtime_error("bad locale name");
			return _Locinfo_Addcats(this, _I, _Pch);
		}
		string _Getname()const
		{
			return (_Newlocname);
		}
		_Collvec _Getcoll()const
		{
			return (::_Getcoll());
		}
		_Ctypevec _Getctype()const
		{
			return (::_Getctype());
		}
		_Cvtvec _Getcvt()const
		{
			return (::_Getcvt());
		}
		const lconv*_Getlconv()const
		{
			return (localeconv());
		}
		_Timevec _Gettnames()const
		{
			return (_Timevec(::_Gettnames()));
		}
		const char*_Getdays()const
		{
			const char*_Ptr=::_Getdays();
			if(_Ptr!=0)
			{
				((_Locinfo*)this)->_Days=_Ptr;
				free((void*)_Ptr);
			}
			return (_Days.size()!=0?_Days.c_str(): ":Sun:Sunday:Mon:Monday:Tue:Tuesday:Wed:Wednesday" ":Thu:Thursday:Fri:Friday:Sat:Saturday");
		}
		const char*_Getmonths()const
		{
			const char*_Ptr=::_Getmonths();
			if(_Ptr!=0)
			{
				((_Locinfo*)this)->_Months=_Ptr;
				free((void*)_Ptr);
			}
			return (_Months.size()!=0?_Months.c_str(): ":Jan:January:Feb:February:Mar:March" ":Apr:April:May:May:Jun:June" ":Jul:July:Aug:August:Sep:September" ":Oct:October:Nov:November:Dec:December");
		}
		const char*_Getfalse()const
		{
			return ("false");
		}
		const char*_Gettrue()const
		{
			return ("true");
		}
		int _Getdateorder()const
		{
			return ::_Getdateorder();
		}
	private:
		_Lockit _Lock;
		string _Days;
		string _Months;
		string _Oldlocname;
		string _Newlocname;
	};
	template<class _Elem>
	inline int _LStrcoll(const _Elem*_First1, const _Elem*_Last1, const _Elem*_First2, const _Elem*_Last2, const _Locinfo::_Collvec*)
	{
		for(;
			_First1!=_Last1&&_First2!=_Last2;
			++_First1, ++_First2)if(*_First1<*_First2)return (-1);
			else if(*_First2<*_First1)return (+1);
			return (_First2!=_Last2?-1: _First1!=_Last1?+1: 0);
	}
	template<>
	inline int _LStrcoll(const char*_First1, const char*_Last1, const char*_First2, const char*_Last2, const _Locinfo::_Collvec*_Vector)
	{
		return (_Strcoll(_First1, _Last1, _First2, _Last2, _Vector));
	}
	template<>
	inline int _LStrcoll(const wchar_t*_First1, const wchar_t*_Last1, const wchar_t*_First2, const wchar_t*_Last2, const _Locinfo::_Collvec*_Vector)
	{
		return (_Wcscoll(_First1, _Last1, _First2, _Last2, _Vector));
	}
	template<class _Elem>
	inline size_t _LStrxfrm(_Elem*_First1, _Elem*_Last1, const _Elem*_First2, const _Elem*_Last2, const _Locinfo::_Collvec*)
	{
		size_t _Count=_Last2-_First2;
		if(_Count<=(size_t)(_Last1-_First1))::memcpy_s((_First1), ((_Last1-_First1)*sizeof(_Elem)), (_First2), (_Count*sizeof(_Elem)));
		return (_Count);
	}
	template<>
	inline size_t _LStrxfrm(char*_First1, char*_Last1, const char*_First2, const char*_Last2, const _Locinfo::_Collvec*_Vector)
	{
		return (_Strxfrm(_First1, _Last1, _First2, _Last2, _Vector));
	}
	template<>
	inline size_t _LStrxfrm(wchar_t*_First1, wchar_t*_Last1, const wchar_t*_First2, const wchar_t*_Last2, const _Locinfo::_Collvec*_Vector)
	{
		return (_Wcsxfrm(_First1, _Last1, _First2, _Last2, _Vector));
	}
}
namespace std
{
	template<class _Elem>
	class collate;
	template<class _Dummy>
	class _Locbase
	{
	public:
		static const int collate=((1<<(1))>>1);
		static const int ctype=((1<<(2))>>1);
		static const int monetary=((1<<(3))>>1);
		static const int numeric=((1<<(4))>>1);
		static const int time=((1<<(5))>>1);
		static const int messages=((1<<(6))>>1);
		static const int all=(((1<<(7))>>1)-1);
		static const int none=0;
	};
	template<class _Dummy>
	const int _Locbase<_Dummy>::collate;
	template<class _Dummy>
	const int _Locbase<_Dummy>::ctype;
	template<class _Dummy>
	const int _Locbase<_Dummy>::monetary;
	template<class _Dummy>
	const int _Locbase<_Dummy>::numeric;
	template<class _Dummy>
	const int _Locbase<_Dummy>::time;
	template<class _Dummy>
	const int _Locbase<_Dummy>::messages;
	template<class _Dummy>
	const int _Locbase<_Dummy>::all;
	template<class _Dummy>
	const int _Locbase<_Dummy>::none;
	class locale;
	template<class _Facet>
	const _Facet&use_facet(const locale&);
	class locale: public _Locbase<int>
	{
	public:
		typedef int category;
		class id
		{
		public:
			id(size_t _Val=0): _Id(_Val)
			{
			}
			operator size_t()
			{
				if(_Id==0)
				{

					{
						::std::_Lockit _Lock(0);
						if(_Id==0)_Id=++_Id_cnt;
					}
				}
				return (_Id);
			}
		private:
			id(const id&);
			id&operator=(const id&);
			size_t _Id;
			static int&_Id_cnt_func();
			static int _Id_cnt;
		};
		class _Locimp;
		class facet
		{
			friend class locale;
			friend class _Locimp;
		public:
			static size_t _Getcat(const facet** =0)
			{
				return ((size_t)(-1));
			}
			void _Incref()
			{

				{
					::std::_Lockit _Lock(0);
					if(_Refs<(size_t)(-1))++_Refs;
				}
			}
			facet*_Decref()
			{

				{
					::std::_Lockit _Lock(0);
					if(0<_Refs&&_Refs<(size_t)(-1))--_Refs;
					return (_Refs==0?this: 0);
				}
			}
			void _Register()
			{
				facet_Register(this);
			}
			void*operator new(size_t _Size)
			{
				return (operator new(_Size, std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 144));
			}
			void*operator new(size_t _Size, const std::_DebugHeapTag_t&_Tag, char*_File, int _Line)
			{
				return (::operator new(_Size, _Tag, _File, _Line));
			}
			void operator delete(void*_Ptr, const std::_DebugHeapTag_t&, char*, int)
			{
				operator delete(_Ptr);
			}
			void operator delete(void*_Ptr)
			{
				std::_DebugHeapDelete((facet*)_Ptr);
			}
			virtual~facet()
			{
			}
		protected: explicit facet(size_t _Initrefs=0): _Refs(_Initrefs)
				   {
				   }
		private:
			static void facet_Register(facet*);
			facet(const facet&);
			facet&operator=(const facet&);
			size_t _Refs;
		};
		class _Locimp: public facet
		{
		protected: ~_Locimp()
				   {
					   _Locimp_dtor(this);
				   }
		private:
			static void _Locimp_dtor(_Locimp*);
			static void _Locimp_ctor(_Locimp*, const _Locimp&);
			static void _Locimp_Addfac(_Locimp*, facet*, size_t);
			friend class locale;
			_Locimp(bool _Transparent=false): locale::facet(1), _Facetvec(0), _Facetcount(0), _Catmask(none), _Xparent(_Transparent), _Name("*")
			{
			}
			_Locimp(const _Locimp&_Right): locale::facet(1), _Facetvec(0), _Facetcount(_Right._Facetcount), _Catmask(_Right._Catmask), _Xparent(_Right._Xparent), _Name(_Right._Name)
			{
				_Locimp_ctor(this, _Right);
			}
			void _Addfac(facet*_Pfacet, size_t _Id)
			{
				_Locimp_Addfac(this, _Pfacet, _Id);
			}
			static _Locimp*_Makeloc(const _Locinfo&, category, _Locimp*, const locale*);
			static void _Makewloc(const _Locinfo&, category, _Locimp*, const locale*);
			static void _Makeushloc(const _Locinfo&, category, _Locimp*, const locale*);
			static void _Makexloc(const _Locinfo&, category, _Locimp*, const locale*);
			facet**_Facetvec;
			size_t _Facetcount;
			category _Catmask;
			bool _Xparent;
			_DebugHeapString _Name;
			static _Locimp*&_Clocptr_func();
			static _Locimp*_Clocptr;
		private:
			_Locimp&operator=(const _Locimp&);
		};
		locale&_Addfac(facet*_Fac, size_t _Id, size_t _Catmask)
		{
			if(1<this->_Ptr->_Refs)
			{
				this->_Ptr->_Decref();
				this->_Ptr=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 256)_Locimp(*this->_Ptr);
			}
			this->_Ptr->_Addfac(_Fac, _Id);
			if(_Catmask!=0)this->_Ptr->_Name="*";
			return (*this);
		}
		template<class _Elem, class _Traits, class _Alloc>
		bool operator()(const basic_string<_Elem, _Traits, _Alloc>&_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)const
		{
			const std::collate<_Elem>&_Coll_fac=std::use_facet<std::collate<_Elem> >(*this);
			return (_Coll_fac.compare(_Left.c_str(), _Left.c_str()+_Left.size(), _Right.c_str(), _Right.c_str()+_Right.size())<0);
		}
		template<class _Facet>
		locale combine(const locale&_Loc)const
		{
			_Facet*_Facptr;
			try
			{
				_Facptr=(_Facet*)&std::use_facet<_Facet>(_Loc);
			}
			catch(...)
			{
				throw runtime_error("locale::combine facet missing");
			}
			_Locimp*_Newimp=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 289)_Locimp(*_Ptr);
			_Newimp->_Addfac(_Facptr, _Facet::id);
			_Newimp->_Catmask=0;
			_Newimp->_Name="*";
			return (locale(_Newimp));
		}
		template<class _Facet>
		locale(const locale&_Loc, const _Facet*_Facptr): _Ptr(new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 298)_Locimp(*_Loc._Ptr))
		{
			if(_Facptr!=0)
			{
				_Ptr->_Addfac((_Facet*)_Facptr, _Facet::id);
				if(_Facet::_Getcat()!=(size_t)(-1))_Ptr->_Catmask=0, _Ptr->_Name="*";
			}
		}
		locale()throw(): _Ptr(_Init())
		{
			_Getgloballocale()->_Incref();
		}
		locale(_Uninitialized)
		{
		}
		locale(const locale&_Right)throw(): _Ptr(_Right._Ptr)
		{
			_Ptr->_Incref();
		}
		locale(const locale&_Loc, const locale&_Other, category _Cat): _Ptr(new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 327)_Locimp(*_Loc._Ptr))
		{
			try
			{

				{
					_Locinfo _Lobj(_Loc._Ptr->_Catmask, _Loc._Ptr->_Name.c_str());
					_Locimp::_Makeloc(_Lobj._Addcats(_Cat&_Other._Ptr->_Catmask, _Other._Ptr->_Name.c_str()), _Cat, _Ptr, &_Other);
				}
			}
			catch(...)
			{
				std::_DebugHeapDelete(_Ptr->_Decref());
				throw;
			}
		}
		explicit locale(const char*_Locname, category _Cat=all): _Ptr(new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 342)_Locimp)
		{
			try
			{
				_Init();

				{
					_Locinfo _Lobj(_Cat, _Locname);
					if(_Lobj._Getname().compare("*")==0)throw runtime_error("bad locale name");
					_Locimp::_Makeloc(_Lobj, _Cat, _Ptr, 0);
				}
			}
			catch(...)
			{
				std::_DebugHeapDelete(_Ptr->_Decref());
				throw;
			}
		}
		locale(const locale&_Loc, const char*_Locname, category _Cat): _Ptr(new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 359)_Locimp(*_Loc._Ptr))
		{
			try
			{

				{
					_Locinfo _Lobj(_Loc._Ptr->_Catmask, _Loc._Ptr->_Name.c_str());
					bool _Hadname=_Lobj._Getname().compare("*")!=0;
					_Lobj._Addcats(_Cat, _Locname);
					if(_Hadname&&_Lobj._Getname().compare("*")==0)throw runtime_error("bad locale name");
					_Locimp::_Makeloc(_Lobj, _Cat, _Ptr, 0);
				}
			}
			catch(...)
			{
				std::_DebugHeapDelete(_Ptr->_Decref());
				throw;
			}
		}
		~locale()throw()
		{
			if(_Ptr!=0)std::_DebugHeapDelete(_Ptr->_Decref());
		}
		locale&operator=(const locale&_Right)throw()
		{
			if(_Ptr!=_Right._Ptr)
			{
				std::_DebugHeapDelete(_Ptr->_Decref());
				_Ptr=_Right._Ptr;
				_Ptr->_Incref();
			}
			return (*this);
		}
		string name()const
		{
			return (_Ptr->_Name);
		}
		const facet*_Getfacet(size_t _Id)const
		{
			const facet*_Facptr=_Id<_Ptr->_Facetcount?_Ptr->_Facetvec[_Id]: 0;
			if(_Facptr!=0||!_Ptr->_Xparent)return (_Facptr);
			else
			{
				locale::_Locimp*_Ptr=_Getgloballocale();
				return (_Id<_Ptr->_Facetcount?_Ptr->_Facetvec[_Id]: 0);
			}
		}
		bool operator==(const locale&_Loc)const
		{
			return (_Ptr==_Loc._Ptr||name().compare("*")!=0&&name().compare(_Loc.name())==0);
		}
		bool operator!=(const locale&_Right)const
		{
			return (!(*this==_Right));
		}
		static const locale&classic();
		static locale global(const locale&);
		static locale empty();
	private:
		locale(_Locimp*_Ptrimp): _Ptr(_Ptrimp)
		{
		}
		static _Locimp*_Getgloballocale();
		static _Locimp*_Init();
		static void _Setgloballocale(void*);
		_Locimp*_Ptr;
	};
	template<class _Facet>
	struct _Facetptr
	{
		static const locale::facet*_Psave;
	};
	template<class _Facet>
	const locale::facet*_Facetptr<_Facet>::_Psave=0;
	template<class _Facet>
	inline locale _Addfac(locale _Loc, const _Facet*_Facptr)
	{
		return (_Loc._Addfac((_Facet*)_Facptr, _Facet::id, _Facet::_Getcat()));
	}
	template<class _Facet>
	inline const _Facet&use_facet(const locale&_Loc)
	{

		{
			::std::_Lockit _Lock(0);
			const locale::facet*_Psave=_Facetptr<_Facet>::_Psave;
			size_t _Id=_Facet::id;
			const locale::facet*_Pf=_Loc._Getfacet(_Id);
			if(_Pf!=0);
			else if(_Psave!=0)_Pf=_Psave;
			else if(_Facet::_Getcat(&_Psave)==(size_t)(-1))throw bad_cast();
			else
			{
				_Pf=_Psave;
				_Facetptr<_Facet>::_Psave=_Psave;
				locale::facet*_Pfmod=(_Facet*)_Psave;
				_Pfmod->_Incref();
				_Pfmod->_Register();
			}
			return ((const _Facet&)(*_Pf));
		}
	}
	template<class _Facet>
	inline const _Facet&use_facet(const locale&_Loc, const _Facet*, bool=false)
	{
		return use_facet<_Facet>(_Loc);
	}
	template<class _Elem, class _InIt>
	inline int _Getloctxt(_InIt&_First, _InIt&_Last, size_t _Numfields, const _Elem*_Ptr)
	{
		for(size_t _Off=0;
			_Ptr[_Off]!=(_Elem)0;
			++_Off)if(_Ptr[_Off]==_Ptr[0])++_Numfields;
		string _Str(_Numfields, '\0');
		int _Ans=-2;
		for(size_t _Column=1;
			;
			++_Column, ++_First, _Ans=-1)
		{
			bool _Prefix=false;
			size_t _Off=0;
			size_t _Field=0;
			for(;
				_Field<_Numfields;
				++_Field)
			{
				for(;
					_Ptr[_Off]!=(_Elem)0&&_Ptr[_Off]!=_Ptr[0];
					++_Off);
				if(_Str[_Field]!='\0')_Off+=_Str[_Field];
				else if(_Ptr[_Off+=_Column]==_Ptr[0]||_Ptr[_Off]==(_Elem)0)
				{
					_Str[_Field]=(char)(_Column<127?_Column: 127);
					_Ans=(int)_Field;
				}
				else if(_First==_Last||_Ptr[_Off]!=*_First)_Str[_Field]=(char)(_Column<127?_Column: 127);
				else _Prefix=true;
			}
			if(!_Prefix||_First==_Last)break;
		}
		return (_Ans);
	}
	template<class _Elem>
	inline char _Maklocbyte(_Elem _Char, const _Locinfo::_Cvtvec&)
	{
		return ((char)(unsigned char)_Char);
	}
	template<>
	inline char _Maklocbyte(wchar_t _Char, const _Locinfo::_Cvtvec&_Cvt)
	{
		char _Byte='\0';
		mbstate_t _Mbst1=
		{
			0
		};
		_Wcrtomb(&_Byte, _Char, &_Mbst1, &_Cvt);
		return (_Byte);
	}
	template<class _Elem>
	inline _Elem _Maklocchr(char _Byte, _Elem*, const _Locinfo::_Cvtvec&)
	{
		return ((_Elem)(unsigned char)_Byte);
	}
	template<>
	inline wchar_t _Maklocchr(char _Byte, wchar_t*, const _Locinfo::_Cvtvec&_Cvt)
	{
		wchar_t _Wc=L'\0';
		mbstate_t _Mbst1=
		{
			0
		};
		_Mbrtowc(&_Wc, &_Byte, 1, &_Mbst1, &_Cvt);
		return (_Wc);
	}
	template<class _Elem>
	inline _Elem*_Maklocstr(const char*_Ptr, _Elem*, const _Locinfo::_Cvtvec&)
	{
		size_t _Count=::strlen(_Ptr)+1;
		_Elem*_Ptrdest=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 609)_Elem[_Count];
		for(_Elem*_Ptrnext=_Ptrdest;
			0<_Count;
			--_Count, ++_Ptrnext, ++_Ptr)*_Ptrnext=(_Elem)(unsigned char)*_Ptr;
		return (_Ptrdest);
	}
	template<>
	inline wchar_t*_Maklocstr(const char*_Ptr, wchar_t*, const _Locinfo::_Cvtvec&_Cvt)
	{
		size_t _Count, _Count1;
		size_t _Wchars;
		const char*_Ptr1;
		int _Bytes;
		wchar_t _Wc;
		mbstate_t _Mbst1=
		{
			0
		};
		_Count1=::strlen(_Ptr)+1;
		for(_Count=_Count1, _Wchars=0, _Ptr1=_Ptr;
			0<_Count;
			_Count-=_Bytes, _Ptr1+=_Bytes, ++_Wchars)if((_Bytes=_Mbrtowc(&_Wc, _Ptr1, _Count, &_Mbst1, &_Cvt))<=0)break;
		++_Wchars;
		wchar_t*_Ptrdest=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 638)wchar_t[_Wchars];
		wchar_t*_Ptrnext=_Ptrdest;
		mbstate_t _Mbst2=
		{
			0
		};
		for(;
			0<_Wchars;
			_Count-=_Bytes, _Ptr+=_Bytes, --_Wchars, ++_Ptrnext)if((_Bytes=_Mbrtowc(_Ptrnext, _Ptr, _Count1, &_Mbst2, &_Cvt))<=0)break;
		*_Ptrnext=L'\0';
		return (_Ptrdest);
	}
	class codecvt_base: public locale::facet
	{
	public:
		enum
		{
			ok, partial, error, noconv
		};
		typedef int result;
		codecvt_base(size_t _Refs=0): locale::facet(_Refs)
		{
		}
		bool always_noconv()const throw()
		{
			return (do_always_noconv());
		}
		int max_length()const throw()
		{
			return (do_max_length());
		}
		int encoding()const throw()
		{
			return (do_encoding());
		}
		~codecvt_base()
		{
		}
	protected: virtual bool do_always_noconv()const throw()
			   {
				   return (true);
			   }
			   virtual int do_max_length()const throw()
			   {
				   return (1);
			   }
			   virtual int do_encoding()const throw()
			   {
				   return (1);
			   }
	};
	template<class _Elem, class _Byte, class _Statype>
	class codecvt: public codecvt_base
	{
	public:
		typedef _Elem intern_type;
		typedef _Byte extern_type;
		typedef _Statype state_type;
		result in(_Statype&_State, const _Byte*_First1, const _Byte*_Last1, const _Byte*&_Mid1, _Elem*_First2, _Elem*_Last2, _Elem*&_Mid2)const
		{
			return (do_in(_State, _First1, _Last1, _Mid1, _First2, _Last2, _Mid2));
		}
		result out(_Statype&_State, const _Elem*_First1, const _Elem*_Last1, const _Elem*&_Mid1, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
		{
			return (do_out(_State, _First1, _Last1, _Mid1, _First2, _Last2, _Mid2));
		}
		result unshift(_Statype&_State, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
		{
			return (do_unshift(_State, _First2, _Last2, _Mid2));
		}
		int length(const _Statype&_State, const _Byte*_First1, const _Byte*_Last1, size_t _Count)const
		{
			return (do_length(_State, _First1, _Last1, _Count));
		}
		static locale::id id;
		explicit codecvt(size_t _Refs=0): codecvt_base(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
		}
		codecvt(const _Locinfo&_Lobj, size_t _Refs=0): codecvt_base(_Refs)
		{
			_Init(_Lobj);
		}
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 764)codecvt<_Elem, _Byte, _Statype>;
			return (2);
		}
	protected: virtual~codecvt()
			   {
			   }
	protected: void _Init(const _Locinfo&)
			   {
			   }
			   virtual result do_in(_Statype&, const _Byte*_First1, const _Byte*, const _Byte*&_Mid1, _Elem*_First2, _Elem*, _Elem*&_Mid2)const
			   {
				   _Mid1=_First1, _Mid2=_First2;
				   return (noconv);
			   }
			   virtual result do_out(_Statype&, const _Elem*_First1, const _Elem*, const _Elem*&_Mid1, _Byte*_First2, _Byte*, _Byte*&_Mid2)const
			   {
				   _Mid1=_First1, _Mid2=_First2;
				   return (noconv);
			   }
			   virtual result do_unshift(_Statype&, _Byte*_First2, _Byte*, _Byte*&_Mid2)const
			   {
				   _Mid2=_First2;
				   return (noconv);
			   }
			   virtual int do_length(const _Statype&, const _Byte*_First1, const _Byte*_Last1, size_t _Count)const
			   {
				   return ((int)(_Count<(size_t)(_Last1-_First1)?_Count: _Last1-_First1));
			   }
	};
	template<class _Elem, class _Byte, class _Statype>
	locale::id codecvt<_Elem, _Byte, _Statype>::id;
	template<>
	class codecvt<wchar_t, char, _Mbstatet>: public codecvt_base
	{
	public:
		typedef wchar_t _Elem;
		typedef char _Byte;
		typedef _Mbstatet _Statype;
		typedef _Elem intern_type;
		typedef _Byte extern_type;
		typedef _Statype state_type;
		result in(_Statype&_State, const _Byte*_First1, const _Byte*_Last1, const _Byte*&_Mid1, _Elem*_First2, _Elem*_Last2, _Elem*&_Mid2)const
		{
			return (do_in(_State, _First1, _Last1, _Mid1, _First2, _Last2, _Mid2));
		}
		result out(_Statype&_State, const _Elem*_First1, const _Elem*_Last1, const _Elem*&_Mid1, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
		{
			return (do_out(_State, _First1, _Last1, _Mid1, _First2, _Last2, _Mid2));
		}
		result unshift(_Statype&_State, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
		{
			return (do_unshift(_State, _First2, _Last2, _Mid2));
		}
		int length(const _Statype&_State, const _Byte*_First1, const _Byte*_Last1, size_t _Count)const
		{
			return (do_length(_State, _First1, _Last1, _Count));
		}
		static locale::id&_Id_func();
		static locale::id id;
		explicit codecvt(size_t _Refs=0): codecvt_base(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
		}
		codecvt(const _Locinfo&_Lobj, size_t _Refs=0): codecvt_base(_Refs)
		{
			_Init(_Lobj);
		}
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 880)codecvt<_Elem, _Byte, _Statype>;
			return (2);
		}
	protected: virtual~codecvt()
			   {
			   }
	protected: void _Init(const _Locinfo&_Lobj)
			   {
				   _Cvt=_Lobj._Getcvt();
			   }
			   virtual result do_in(_Statype&_State, const _Byte*_First1, const _Byte*_Last1, const _Byte*&_Mid1, _Elem*_First2, _Elem*_Last2, _Elem*&_Mid2)const
			   {
				   _Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 899);
				   _Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 900);
				   _Mid1=_First1, _Mid2=_First2;
				   result _Ans=_Mid1==_Last1?ok: partial;
				   int _Bytes;
				   while(_Mid1!=_Last1&&_Mid2!=_Last2)switch(_Bytes=_Mbrtowc(_Mid2, _Mid1, _Last1-_Mid1, &_State, &_Cvt))
				   {
	case-2: _Mid1=_Last1;
		return (_Ans);
	case-1: return (error);
	case 0: if(*_Mid2==(_Elem)0)_Bytes=(int)::strlen(_Mid1)+1;
	default: if(_Bytes==-3)_Bytes=0;
		_Mid1+=_Bytes;
		++_Mid2;
		_Ans=ok;
				   }
				   return (_Ans);
			   }
			   virtual result do_out(_Statype&_State, const _Elem*_First1, const _Elem*_Last1, const _Elem*&_Mid1, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
			   {
				   _Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 935);
				   _Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 936);
				   _Mid1=_First1, _Mid2=_First2;
				   result _Ans=_Mid1==_Last1?ok: partial;
				   int _Bytes;
				   while(_Mid1!=_Last1&&_Mid2!=_Last2)if(5<=_Last2-_Mid2)if((_Bytes=_Wcrtomb(_Mid2, *_Mid1, &_State, &_Cvt))<0)return (error);
				   else++_Mid1, _Mid2+=_Bytes, _Ans=ok;
				   else
				   {
					   _Byte _Buf[5];
					   _Statype _Stsave=_State;
					   if((_Bytes=_Wcrtomb(_Buf, *_Mid1, &_State, &_Cvt))<0)return (error);
					   else if(_Last2-_Mid2<_Bytes)
					   {
						   _State=_Stsave;
						   return (_Ans);
					   }
					   else
					   {
						   ::memcpy_s((_Mid2), (_Last2-_Mid2), (_Buf), (_Bytes));
						   ++_Mid1, _Mid2+=_Bytes, _Ans=ok;
					   }
				   }
				   return (_Ans);
			   }
			   virtual result do_unshift(_Statype&_State, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
			   {
				   _Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 973);
				   _Mid2=_First2;
				   result _Ans=ok;
				   int _Bytes;
				   _Byte _Buf[5];
				   _Statype _Stsave=_State;
				   if((_Bytes=_Wcrtomb(_Buf, L'\0', &_State, &_Cvt))<=0)_Ans=error;
				   else if(_Last2-_Mid2<--_Bytes)
				   {
					   _State=_Stsave;
					   _Ans=partial;
				   }
				   else if(0<_Bytes)
				   {
					   ::memcpy_s((_Mid2), (_Last2-_Mid2), (_Buf), (_Bytes));
					   _Mid2+=_Bytes;
				   }
				   return (_Ans);
			   }
			   virtual int do_length(const _Statype&_State, const _Byte*_First1, const _Byte*_Last1, size_t _Count)const
			   {
				   _Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 998);
				   int _Wchars;
				   const _Byte*_Mid1;
				   _Statype _Mystate=_State;
				   for(_Wchars=0, _Mid1=_First1;
					   (size_t)_Wchars<_Count&&_Mid1!=_Last1;
					   )
				   {
					   int _Bytes;
					   _Elem _Ch;
					   switch(_Bytes=_Mbrtowc(&_Ch, _Mid1, _Last1-_Mid1, &_Mystate, &_Cvt))
					   {
					   case-2: return (_Wchars);
					   case-1: return (_Wchars);
					   case 0: if(_Ch==(_Elem)0)_Bytes=(int)::strlen(_Mid1)+1;
					   default: if(_Bytes==-3)_Bytes=0;
						   _Mid1+=_Bytes;
						   ++_Wchars;
					   }
				   }
				   return (_Wchars);
			   }
			   virtual bool do_always_noconv()const throw()
			   {
				   return (false);
			   }
			   virtual int do_max_length()const throw()
			   {
				   return (5);
			   }
			   virtual int do_encoding()const throw()
			   {
				   return (0);
			   }
	private:
		_Locinfo::_Cvtvec _Cvt;
	};
	template<>
	class codecvt<unsigned short, char, _Mbstatet>: public codecvt_base
	{
	public:
		typedef unsigned short _Elem;
		typedef char _Byte;
		typedef _Mbstatet _Statype;
		typedef _Elem intern_type;
		typedef _Byte extern_type;
		typedef _Statype state_type;
		result in(_Statype&_State, const _Byte*_First1, const _Byte*_Last1, const _Byte*&_Mid1, _Elem*_First2, _Elem*_Last2, _Elem*&_Mid2)const
		{
			return (do_in(_State, _First1, _Last1, _Mid1, _First2, _Last2, _Mid2));
		}
		result out(_Statype&_State, const _Elem*_First1, const _Elem*_Last1, const _Elem*&_Mid1, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
		{
			return (do_out(_State, _First1, _Last1, _Mid1, _First2, _Last2, _Mid2));
		}
		result unshift(_Statype&_State, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
		{
			return (do_unshift(_State, _First2, _Last2, _Mid2));
		}
		int length(const _Statype&_State, const _Byte*_First1, const _Byte*_Last1, size_t _Count)const
		{
			return (do_length(_State, _First1, _Last1, _Count));
		}
		static locale::id&_Id_func();
		static locale::id id;
		explicit codecvt(size_t _Refs=0): codecvt_base(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
		}
		codecvt(const _Locinfo&_Lobj, size_t _Refs=0): codecvt_base(_Refs)
		{
			_Init(_Lobj);
		}
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1118)codecvt<_Elem, _Byte, _Statype>;
			return (2);
		}
	protected: virtual~codecvt()
			   {
			   }
	protected: codecvt(const char*_Locname, size_t _Refs=0): codecvt_base(_Refs)
			   {

				   {
					   _Locinfo _Lobj(_Locname);
					   _Init(_Lobj);
				   }
			   }
			   void _Init(const _Locinfo&_Lobj)
			   {
				   _Cvt=_Lobj._Getcvt();
			   }
			   virtual result do_in(_Statype&_State, const _Byte*_First1, const _Byte*_Last1, const _Byte*&_Mid1, _Elem*_First2, _Elem*_Last2, _Elem*&_Mid2)const
			   {
				   _Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1145);
				   _Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1146);
				   _Mid1=_First1, _Mid2=_First2;
				   result _Ans=_Mid1==_Last1?ok: partial;
				   int _Bytes;
				   while(_Mid1!=_Last1&&_Mid2!=_Last2)switch(_Bytes=_Mbrtowc((wchar_t*)_Mid2, _Mid1, _Last1-_Mid1, &_State, &_Cvt))
				   {
	case-2: _Mid1=_Last1;
		return (_Ans);
	case-1: return (error);
	case 0: if(*_Mid2==(_Elem)0)_Bytes=(int)::strlen(_Mid1)+1;
	default: if(_Bytes==-3)_Bytes=0;
		_Mid1+=_Bytes;
		++_Mid2;
		_Ans=ok;
				   }
				   return (_Ans);
			   }
			   virtual result do_out(_Statype&_State, const _Elem*_First1, const _Elem*_Last1, const _Elem*&_Mid1, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
			   {
				   _Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1181);
				   _Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1182);
				   _Mid1=_First1, _Mid2=_First2;
				   result _Ans=_Mid1==_Last1?ok: partial;
				   int _Bytes;
				   while(_Mid1!=_Last1&&_Mid2!=_Last2)if(5<=_Last2-_Mid2)if((_Bytes=_Wcrtomb(_Mid2, *_Mid1, &_State, &_Cvt))<0)return (error);
				   else++_Mid1, _Mid2+=_Bytes, _Ans=ok;
				   else
				   {
					   _Byte _Buf[5];
					   _Statype _Stsave=_State;
					   if((_Bytes=_Wcrtomb(_Buf, *_Mid1, &_State, &_Cvt))<0)return (error);
					   else if(_Last2-_Mid2<_Bytes)
					   {
						   _State=_Stsave;
						   return (_Ans);
					   }
					   else
					   {
						   ::memcpy_s((_Mid2), (_Last2-_Mid2), (_Buf), (_Bytes));
						   ++_Mid1, _Mid2+=_Bytes, _Ans=ok;
					   }
				   }
				   return (_Ans);
			   }
			   virtual result do_unshift(_Statype&_State, _Byte*_First2, _Byte*_Last2, _Byte*&_Mid2)const
			   {
				   _Debug_range(_First2, _Last2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1219);
				   _Mid2=_First2;
				   result _Ans=ok;
				   int _Bytes;
				   _Byte _Buf[5];
				   _Statype _Stsave=_State;
				   if((_Bytes=_Wcrtomb(_Buf, L'\0', &_State, &_Cvt))<=0)_Ans=error;
				   else if(_Last2-_Mid2<--_Bytes)
				   {
					   _State=_Stsave;
					   _Ans=partial;
				   }
				   else if(0<_Bytes)
				   {
					   ::memcpy_s((_Mid2), (_Last2-_Mid2), (_Buf), (_Bytes));
					   _Mid2+=_Bytes;
				   }
				   return (_Ans);
			   }
			   virtual int do_length(const _Statype&_State, const _Byte*_First1, const _Byte*_Last1, size_t _Count)const
			   {
				   _Debug_range(_First1, _Last1, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1244);
				   int _Wchars;
				   const _Byte*_Mid1;
				   _Statype _Mystate=_State;
				   for(_Wchars=0, _Mid1=_First1;
					   (size_t)_Wchars<_Count&&_Mid1!=_Last1;
					   )
				   {
					   int _Bytes;
					   _Elem _Ch;
					   switch(_Bytes=_Mbrtowc((wchar_t*)&_Ch, _Mid1, _Last1-_Mid1, &_Mystate, &_Cvt))
					   {
					   case-2: return (_Wchars);
					   case-1: return (_Wchars);
					   case 0: if(_Ch==(_Elem)0)_Bytes=(int)::strlen(_Mid1)+1;
					   default: if(_Bytes==-3)_Bytes=0;
						   _Mid1+=_Bytes;
						   ++_Wchars;
					   }
				   }
				   return (_Wchars);
			   }
			   virtual bool do_always_noconv()const throw()
			   {
				   return (false);
			   }
			   virtual int do_max_length()const throw()
			   {
				   return (5);
			   }
			   virtual int do_encoding()const throw()
			   {
				   return (0);
			   }
	private:
		_Locinfo::_Cvtvec _Cvt;
	};
	template<class _Elem, class _Byte, class _Statype>
	class codecvt_byname: public codecvt<_Elem, _Byte, _Statype>
	{
	public:
		explicit codecvt_byname(const char*_Locname, size_t _Refs=0): codecvt<_Elem, _Byte, _Statype>(_Locname, _Refs)
		{
		}
	protected: virtual~codecvt_byname()
			   {
			   }
	};
	struct ctype_base: public locale::facet
	{
		enum
		{
			alnum=0x4|0x2|0x1|0x100, alpha=0x2|0x1|0x100, cntrl=0x20, digit=0x4, graph=0x4|0x2|0x10|0x1|0x100, lower=0x2, print=0x4|0x2|0x10|0x40|0x1|0x100|0x80, punct=0x10, space=0x8|0x40|0x000, upper=0x1, xdigit=0x80
		};
		typedef short mask;
		ctype_base(size_t _Refs=0): locale::facet(_Refs)
		{
		}
		~ctype_base()
		{
		}
	protected: static void _Xran()
			   {
				   throw out_of_range("out_of_range in ctype<T>");
			   }
	};
	template<class _Elem>
	class ctype: public ctype_base
	{
	public:
		typedef _Elem char_type;
		bool is(mask _Maskval, _Elem _Ch)const
		{
			return (do_is(_Maskval, _Ch));
		}
		const _Elem*is(const _Elem*_First, const _Elem*_Last, mask*_Dest)const
		{
			return (do_is(_First, _Last, _Dest));
		}
		const _Elem*scan_is(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
		{
			return (do_scan_is(_Maskval, _First, _Last));
		}
		const _Elem*scan_not(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
		{
			return (do_scan_not(_Maskval, _First, _Last));
		}
		_Elem tolower(_Elem _Ch)const
		{
			return (do_tolower(_Ch));
		}
		const _Elem*tolower(_Elem*_First, const _Elem*_Last)const
		{
			return (do_tolower(_First, _Last));
		}
		_Elem toupper(_Elem _Ch)const
		{
			return (do_toupper(_Ch));
		}
		const _Elem*toupper(_Elem*_First, const _Elem*_Last)const
		{
			return (do_toupper(_First, _Last));
		}
		_Elem widen(char _Byte)const
		{
			return (do_widen(_Byte));
		}
		const char*widen(const char*_First, const char*_Last, _Elem*_Dest)const
		{
			return (do_widen(_First, _Last, _Dest));
		}
		const char*_Widen_s(const char*_First, const char*_Last, _Elem*_Dest, size_t _Dest_size)const
		{
			return (_Do_widen_s(_First, _Last, _Dest, _Dest_size));
		}
		char narrow(_Elem _Ch, char _Dflt='\0')const
		{
			return (do_narrow(_Ch, _Dflt));
		}
		const _Elem*narrow(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest)const
		{
			return (do_narrow(_First, _Last, _Dflt, _Dest));
		}
		const _Elem*_Narrow_s(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest, size_t _Dest_size)const
		{
			return (_Do_narrow_s(_First, _Last, _Dflt, _Dest, _Dest_size));
		}
		static locale::id id;
		explicit ctype(size_t _Refs=0): ctype_base(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
		}
		ctype(const _Locinfo&_Lobj, size_t _Refs=0): ctype_base(_Refs)
		{
			_Init(_Lobj);
		}
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1460)ctype<_Elem>;
			return (2);
		}
	protected: virtual~ctype()
			   {
				   if(_Ctype._Delfl)free((void*)_Ctype._Table);
			   }
	protected: void _Init(const _Locinfo&_Lobj)
			   {
				   _Ctype=_Lobj._Getctype();
			   }
			   virtual bool do_is(mask _Maskval, _Elem _Ch)const
			   {
				   return ((_Ctype._Table[(unsigned char)narrow(_Ch)]&_Maskval)!=0);
			   }
			   virtual const _Elem*do_is(const _Elem*_First, const _Elem*_Last, mask*_Dest)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1486);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1487);
				   for(;
					   _First!=_Last;
					   ++_First, ++_Dest)*_Dest=_Ctype._Table[(unsigned char)narrow(*_First)];
				   return (_First);
			   }
			   virtual const _Elem*do_scan_is(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1496);
				   for(;
					   _First!=_Last&&!is(_Maskval, *_First);
					   ++_First);
				   return (_First);
			   }
			   virtual const _Elem*do_scan_not(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1505);
				   for(;
					   _First!=_Last&&is(_Maskval, *_First);
					   ++_First);
				   return (_First);
			   }
			   virtual _Elem do_tolower(_Elem _Ch)const
			   {
				   unsigned char _Byte=(unsigned char)narrow(_Ch, '\0');
				   if(_Byte=='\0')return (_Ch);
				   else return (widen((char)_Tolower(_Byte, &_Ctype)));
			   }
			   virtual const _Elem*do_tolower(_Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range((const _Elem*)_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1522);
				   for(;
					   _First!=_Last;
					   ++_First)
				   {
					   unsigned char _Byte=(unsigned char)narrow(*_First, '\0');
					   if(_Byte!='\0')*_First=(widen((char)_Tolower(_Byte, &_Ctype)));
				   }
				   return ((const _Elem*)_First);
			   }
			   virtual _Elem do_toupper(_Elem _Ch)const
			   {
				   unsigned char _Byte=(unsigned char)narrow(_Ch, '\0');
				   if(_Byte=='\0')return (_Ch);
				   else return (widen((char)_Toupper(_Byte, &_Ctype)));
			   }
			   virtual const _Elem*do_toupper(_Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range((const _Elem*)_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1543);
				   for(;
					   _First!=_Last;
					   ++_First)
				   {
					   unsigned char _Byte=(unsigned char)narrow(*_First, '\0');
					   if(_Byte!='\0')*_First=(widen((char)_Toupper(_Byte, &_Ctype)));
				   }
				   return ((const _Elem*)_First);
			   }
			   virtual _Elem do_widen(char _Byte)const
			   {
				   return (_Maklocchr(_Byte, (_Elem*)0, _Cvt));
			   }
			   virtual const char*do_widen(const char*_First, const char*_Last, _Elem*_Dest)const
			   {
				   return _Do_widen_s(_First, _Last, _Dest, _Last-_First);
			   }
			   virtual const char*_Do_widen_s(const char*_First, const char*_Last, _Elem*_Dest, size_t _Dest_size)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1569);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1570);

				   {
					   if(!(_Dest_size>=(size_t)(_Last-_First)))
					   {
						   (void)((!!((("_Dest_size >= (size_t)(_Last - _First)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1571, 0, L"(\"_Dest_size >= (size_t)(_Last - _First)\", 0)"))||(__debugbreak(), 0));
						   ::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1571, 0);
					   }
				   };
				   for(;
					   _First!=_Last;
					   ++_First, ++_Dest)*_Dest=_Maklocchr(*_First, (_Elem*)0, _Cvt);
				   return (_First);
			   }
			   char _Donarrow(_Elem _Ch, char _Dflt)const
			   {
				   char _Byte;
				   if(_Ch==(_Elem)0)return ('\0');
				   else if((_Byte=_Maklocbyte((_Elem)_Ch, _Cvt))=='\0')return (_Dflt);
				   else return (_Byte);
			   }
			   virtual char do_narrow(_Elem _Ch, char _Dflt)const
			   {
				   return (_Donarrow(_Ch, _Dflt));
			   }
			   virtual const _Elem*do_narrow(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest)const
			   {
				   return _Do_narrow_s(_First, _Last, _Dflt, _Dest, _Last-_First);
			   }
			   virtual const _Elem*_Do_narrow_s(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest, size_t _Dest_size)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1606);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1607);

				   {
					   if(!(_Dest_size>=(size_t)(_Last-_First)))
					   {
						   (void)((!!((("_Dest_size >= (size_t)(_Last - _First)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1608, 0, L"(\"_Dest_size >= (size_t)(_Last - _First)\", 0)"))||(__debugbreak(), 0));
						   ::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1608, 0);
					   }
				   };
				   for(;
					   _First!=_Last;
					   ++_First, ++_Dest)*_Dest=_Donarrow(*_First, _Dflt);
				   return (_First);
			   }
	private:
		_Locinfo::_Ctypevec _Ctype;
		_Locinfo::_Cvtvec _Cvt;
	};
	template<class _Elem>
	locale::id ctype<_Elem>::id;
	template<>
	class ctype<char>: public ctype_base
	{
		typedef ctype<char>_Myt;
	public:
		typedef char _Elem;
		typedef _Elem char_type;
		bool is(mask _Maskval, _Elem _Ch)const
		{
			return ((_Ctype._Table[(unsigned char)_Ch]&_Maskval)!=0);
		}
		const _Elem*is(const _Elem*_First, const _Elem*_Last, mask*_Dest)const
		{
			_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1641);
			_Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1642);
			for(;
				_First!=_Last;
				++_First, ++_Dest)*_Dest=_Ctype._Table[(unsigned char)*_First];
			return (_First);
		}
		const _Elem*scan_is(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
		{
			_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1651);
			for(;
				_First!=_Last&&!is(_Maskval, *_First);
				++_First);
			return (_First);
		}
		const _Elem*scan_not(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
		{
			_Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1660);
			for(;
				_First!=_Last&&is(_Maskval, *_First);
				++_First);
			return (_First);
		}
		_Elem tolower(_Elem _Ch)const
		{
			return (do_tolower(_Ch));
		}
		const _Elem*tolower(_Elem*_First, const _Elem*_Last)const
		{
			return (do_tolower(_First, _Last));
		}
		_Elem toupper(_Elem _Ch)const
		{
			return (do_toupper(_Ch));
		}
		const _Elem*toupper(_Elem*_First, const _Elem*_Last)const
		{
			return (do_toupper(_First, _Last));
		}
		_Elem widen(char _Byte)const
		{
			return (do_widen(_Byte));
		}
		const _Elem*widen(const char*_First, const char*_Last, _Elem*_Dest)const
		{
			return (do_widen(_First, _Last, _Dest));
		}
		const _Elem*_Widen_s(const char*_First, const char*_Last, _Elem*_Dest, size_t _Dest_size)const
		{
			return (_Do_widen_s(_First, _Last, _Dest, _Dest_size));
		}
		_Elem narrow(_Elem _Ch, char _Dflt='\0')const
		{
			return (do_narrow(_Ch, _Dflt));
		}
		const _Elem*narrow(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest)const
		{
			return (do_narrow(_First, _Last, _Dflt, _Dest));
		}
		const _Elem*_Narrow_s(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest, size_t _Dest_size)const
		{
			return (_Do_narrow_s(_First, _Last, _Dflt, _Dest, _Dest_size));
		}
		static locale::id&_Id_func();
		static locale::id id;
		explicit ctype(const mask*_Table=0, bool _Deletetable=false, size_t _Refs=0): ctype_base(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
			if(_Table!=0)
			{
				_Tidy();
				_Ctype._Table=_Table;
				_Ctype._Delfl=_Deletetable?-1: 0;
			}
		}
		ctype(const _Locinfo&_Lobj, size_t _Refs=0): ctype_base(_Refs)
		{
			_Init(_Lobj);
		}
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1763)ctype<_Elem>;
			return (2);
		}
		static const size_t table_size=1<<8;
	protected: virtual~ctype()
			   {
				   _Tidy();
			   }
	protected: void _Init(const _Locinfo&_Lobj)
			   {
				   _Ctype=_Lobj._Getctype();
			   }
			   void _Tidy()
			   {
				   if(0<_Ctype._Delfl)free((void*)_Ctype._Table);
				   else if(_Ctype._Delfl<0)delete[](void*)_Ctype._Table;
			   }
			   virtual _Elem do_tolower(_Elem _Ch)const
			   {
				   return ((_Elem)_Tolower((unsigned char)_Ch, &_Ctype));
			   }
			   virtual const _Elem*do_tolower(_Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range((const _Elem*)_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1797);
				   for(;
					   _First!=_Last;
					   ++_First)*_First=(_Elem)_Tolower((unsigned char)*_First, &_Ctype);
				   return ((const _Elem*)_First);
			   }
			   virtual _Elem do_toupper(_Elem _Ch)const
			   {
				   return ((_Elem)_Toupper((unsigned char)_Ch, &_Ctype));
			   }
			   virtual const _Elem*do_toupper(_Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range((const _Elem*)_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1811);
				   for(;
					   _First!=_Last;
					   ++_First)*_First=(_Elem)_Toupper((unsigned char)*_First, &_Ctype);
				   return ((const _Elem*)_First);
			   }
			   virtual _Elem do_widen(char _Byte)const
			   {
				   return (_Byte);
			   }
			   virtual const _Elem*do_widen(const char*_First, const char*_Last, _Elem*_Dest)const
			   {
				   return _Do_widen_s(_First, _Last, _Dest, _Last-_First);
			   }
			   virtual const _Elem*_Do_widen_s(const char*_First, const char*_Last, _Elem*_Dest, size_t _Dest_size)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1833);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1834);

				   {
					   if(!(_Dest_size>=(size_t)(_Last-_First)))
					   {
						   (void)((!!((("_Dest_size >= (size_t)(_Last - _First)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1835, 0, L"(\"_Dest_size >= (size_t)(_Last - _First)\", 0)"))||(__debugbreak(), 0));
						   ::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1835, 0);
					   }
				   };
				   ::memcpy_s((_Dest), (_Dest_size), (_First), (_Last-_First));
				   return (_Last);
			   }
			   virtual _Elem do_narrow(_Elem _Ch, char)const
			   {
				   return (_Ch);
			   }
			   virtual const _Elem*do_narrow(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest)const
			   {
				   return _Do_narrow_s(_First, _Last, _Dflt, _Dest, _Last-_First);
			   }
			   virtual const _Elem*_Do_narrow_s(const _Elem*_First, const _Elem*_Last, char, char*_Dest, size_t _Dest_size)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1858);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1859);

				   {
					   if(!(_Dest_size>=(size_t)(_Last-_First)))
					   {
						   (void)((!!((("_Dest_size >= (size_t)(_Last - _First)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1860, 0, L"(\"_Dest_size >= (size_t)(_Last - _First)\", 0)"))||(__debugbreak(), 0));
						   ::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 1860, 0);
					   }
				   };
				   ::memcpy_s((_Dest), (_Dest_size), (_First), (_Last-_First));
				   return (_Last);
			   }
			   const mask*table()const throw()
			   {
				   return (_Ctype._Table);
			   }
			   static const mask*classic_table()throw()
			   {
				   const _Myt&_Ctype_fac=use_facet<_Myt>(locale::classic());
				   return (_Ctype_fac.table());
			   }
	private:
		_Locinfo::_Ctypevec _Ctype;
	};
	template<>
	class ctype<wchar_t>: public ctype_base
	{
		typedef ctype<wchar_t>_Myt;
	public:
		typedef wchar_t _Elem;
		typedef _Elem char_type;
		bool is(mask _Maskval, _Elem _Ch)const
		{
			return (do_is(_Maskval, _Ch));
		}
		const _Elem*is(const _Elem*_First, const _Elem*_Last, mask*_Dest)const
		{
			return (do_is(_First, _Last, _Dest));
		}
		const _Elem*scan_is(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
		{
			return (do_scan_is(_Maskval, _First, _Last));
		}
		const _Elem*scan_not(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
		{
			return (do_scan_not(_Maskval, _First, _Last));
		}
		_Elem tolower(_Elem _Ch)const
		{
			return (do_tolower(_Ch));
		}
		const _Elem*tolower(_Elem*_First, const _Elem*_Last)const
		{
			return (do_tolower(_First, _Last));
		}
		_Elem toupper(_Elem _Ch)const
		{
			return (do_toupper(_Ch));
		}
		const _Elem*toupper(_Elem*_First, const _Elem*_Last)const
		{
			return (do_toupper(_First, _Last));
		}
		_Elem widen(char _Byte)const
		{
			return (do_widen(_Byte));
		}
		const char*widen(const char*_First, const char*_Last, _Elem*_Dest)const
		{
			return (do_widen(_First, _Last, _Dest));
		}
		const char*_Widen_s(const char*_First, const char*_Last, _Elem*_Dest, size_t _Dest_size)const
		{
			return (_Do_widen_s(_First, _Last, _Dest, _Dest_size));
		}
		char narrow(_Elem _Ch, char _Dflt='\0')const
		{
			return (do_narrow(_Ch, _Dflt));
		}
		const _Elem*narrow(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest)const
		{
			return (do_narrow(_First, _Last, _Dflt, _Dest));
		}
		const _Elem*_Narrow_s(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest, size_t _Dest_size)const
		{
			return (_Do_narrow_s(_First, _Last, _Dflt, _Dest, _Dest_size));
		}
		static locale::id&_Id_func();
		static locale::id id;
		explicit ctype(size_t _Refs=0): ctype_base(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
		}
		ctype(const _Locinfo&_Lobj, size_t _Refs=0): ctype_base(_Refs)
		{
			_Init(_Lobj);
		}
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2002)ctype<_Elem>;
			return (2);
		}
	protected: virtual~ctype()
			   {
				   if(_Ctype._Delfl)free((void*)_Ctype._Table);
			   }
	protected: void _Init(const _Locinfo&_Lobj)
			   {
				   _Ctype=_Lobj._Getctype();
				   _Cvt=_Lobj._Getcvt();
			   }
			   virtual bool do_is(mask _Maskval, _Elem _Ch)const
			   {
				   return ((::_Getwctype(_Ch, &_Ctype)&_Maskval)!=0);
			   }
			   virtual const wchar_t*do_is(const _Elem*_First, const _Elem*_Last, mask*_Dest)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2028);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2029);
				   return (::_Getwctypes(_First, _Last, _Dest, &_Ctype));
			   }
			   virtual const _Elem*do_scan_is(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2036);
				   for(;
					   _First!=_Last&&!is(_Maskval, *_First);
					   ++_First);
				   return (_First);
			   }
			   virtual const _Elem*do_scan_not(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2045);
				   for(;
					   _First!=_Last&&is(_Maskval, *_First);
					   ++_First);
				   return (_First);
			   }
			   virtual _Elem do_tolower(_Elem _Ch)const
			   {
				   return (_Towlower(_Ch, &_Ctype));
			   }
			   virtual const _Elem*do_tolower(_Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range((const _Elem*)_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2059);
				   for(;
					   _First!=_Last;
					   ++_First)*_First=_Towlower(*_First, &_Ctype);
				   return ((const _Elem*)_First);
			   }
			   virtual _Elem do_toupper(_Elem _Ch)const
			   {
				   return (_Towupper(_Ch, &_Ctype));
			   }
			   virtual const _Elem*do_toupper(_Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range((const _Elem*)_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2073);
				   for(;
					   _First!=_Last;
					   ++_First)*_First=_Towupper(*_First, &_Ctype);
				   return ((const _Elem*)_First);
			   }
			   _Elem _Dowiden(char _Byte)const
			   {
				   mbstate_t _Mbst=
				   {
					   0
				   };
				   wchar_t _Wc;
				   return (_Mbrtowc(&_Wc, &_Byte, 1, &_Mbst, &_Cvt)<0?(wchar_t)(wint_t)(0xFFFF): _Wc);
			   }
			   virtual _Elem do_widen(char _Byte)const
			   {
				   return (_Dowiden(_Byte));
			   }
			   virtual const char*do_widen(const char*_First, const char*_Last, _Elem*_Dest)const
			   {
				   return _Do_widen_s(_First, _Last, _Dest, _Last-_First);
			   }
			   virtual const char*_Do_widen_s(const char*_First, const char*_Last, _Elem*_Dest, size_t _Dest_size)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2103);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2104);

				   {
					   if(!(_Dest_size>=(size_t)(_Last-_First)))
					   {
						   (void)((!!((("_Dest_size >= (size_t)(_Last - _First)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2105, 0, L"(\"_Dest_size >= (size_t)(_Last - _First)\", 0)"))||(__debugbreak(), 0));
						   ::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2105, 0);
					   }
				   };
				   for(;
					   _First!=_Last;
					   ++_First, ++_Dest)*_Dest=_Dowiden(*_First);
				   return (_First);
			   }
			   char _Donarrow(_Elem _Ch, char _Dflt)const
			   {
				   char _Buf[5];
				   mbstate_t _Mbst=
				   {
					   0
				   };
				   return (_Wcrtomb(_Buf, _Ch, &_Mbst, &_Cvt)!=1?_Dflt: _Buf[0]);
			   }
			   virtual char do_narrow(_Elem _Ch, char _Dflt)const
			   {
				   return (_Donarrow(_Ch, _Dflt));
			   }
			   virtual const _Elem*do_narrow(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest)const
			   {
				   return _Do_narrow_s(_First, _Last, _Dflt, _Dest, _Last-_First);
			   }
			   virtual const _Elem*_Do_narrow_s(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest, size_t _Dest_size)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2138);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2139);

				   {
					   if(!(_Dest_size>=(size_t)(_Last-_First)))
					   {
						   (void)((!!((("_Dest_size >= (size_t)(_Last - _First)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2140, 0, L"(\"_Dest_size >= (size_t)(_Last - _First)\", 0)"))||(__debugbreak(), 0));
						   ::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2140, 0);
					   }
				   };
				   for(;
					   _First!=_Last;
					   ++_First, ++_Dest)*_Dest=_Donarrow(*_First, _Dflt);
				   return (_First);
			   }
	private:
		_Locinfo::_Ctypevec _Ctype;
		_Locinfo::_Cvtvec _Cvt;
	};
	template<>
	class ctype<unsigned short>: public ctype_base
	{
		typedef ctype<unsigned short>_Myt;
	public:
		typedef unsigned short _Elem;
		typedef _Elem char_type;
		bool is(mask _Maskval, _Elem _Ch)const
		{
			return (do_is(_Maskval, _Ch));
		}
		const _Elem*is(const _Elem*_First, const _Elem*_Last, mask*_Dest)const
		{
			return (do_is(_First, _Last, _Dest));
		}
		const _Elem*scan_is(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
		{
			return (do_scan_is(_Maskval, _First, _Last));
		}
		const _Elem*scan_not(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
		{
			return (do_scan_not(_Maskval, _First, _Last));
		}
		_Elem tolower(_Elem _Ch)const
		{
			return (do_tolower(_Ch));
		}
		const _Elem*tolower(_Elem*_First, const _Elem*_Last)const
		{
			return (do_tolower(_First, _Last));
		}
		_Elem toupper(_Elem _Ch)const
		{
			return (do_toupper(_Ch));
		}
		const _Elem*toupper(_Elem*_First, const _Elem*_Last)const
		{
			return (do_toupper(_First, _Last));
		}
		_Elem widen(char _Byte)const
		{
			return (do_widen(_Byte));
		}
		const char*widen(const char*_First, const char*_Last, _Elem*_Dest)const
		{
			return (do_widen(_First, _Last, _Dest));
		}
		const char*_Widen_s(const char*_First, const char*_Last, _Elem*_Dest, size_t _Dest_size)const
		{
			return (_Do_widen_s(_First, _Last, _Dest, _Dest_size));
		}
		char narrow(_Elem _Ch, char _Dflt='\0')const
		{
			return (do_narrow(_Ch, _Dflt));
		}
		const _Elem*narrow(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest)const
		{
			return (do_narrow(_First, _Last, _Dflt, _Dest));
		}
		const _Elem*_Narrow_s(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest, size_t _Dest_size)const
		{
			return (_Do_narrow_s(_First, _Last, _Dflt, _Dest, _Dest_size));
		}
		static locale::id&_Id_func();
		static locale::id id;
		explicit ctype(size_t _Refs=0): ctype_base(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
		}
		ctype(const _Locinfo&_Lobj, size_t _Refs=0): ctype_base(_Refs)
		{
			_Init(_Lobj);
		}
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2274)ctype<_Elem>;
			return (2);
		}
	protected: virtual~ctype()
			   {
				   if(_Ctype._Delfl)free((void*)_Ctype._Table);
			   }
	protected: ctype(const char*_Locname, size_t _Refs=0): ctype_base(_Refs)
			   {

				   {
					   _Locinfo _Lobj(_Locname);
					   _Init(_Lobj);
				   }
			   }
			   void _Init(const _Locinfo&_Lobj)
			   {
				   _Ctype=_Lobj._Getctype();
				   _Cvt=_Lobj._Getcvt();
			   }
			   virtual bool do_is(mask _Maskval, _Elem _Ch)const
			   {
				   return ((::_Getwctype(_Ch, &_Ctype)&_Maskval)!=0);
			   }
			   virtual const _Elem*do_is(const _Elem*_First, const _Elem*_Last, mask*_Dest)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2308);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2309);
				   return ((const _Elem*)::_Getwctypes((const wchar_t*)_First, (const wchar_t*)_Last, _Dest, &_Ctype));
			   }
			   virtual const _Elem*do_scan_is(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2317);
				   for(;
					   _First!=_Last&&!is(_Maskval, *_First);
					   ++_First);
				   return (_First);
			   }
			   virtual const _Elem*do_scan_not(mask _Maskval, const _Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2326);
				   for(;
					   _First!=_Last&&is(_Maskval, *_First);
					   ++_First);
				   return (_First);
			   }
			   virtual _Elem do_tolower(_Elem _Ch)const
			   {
				   return (_Towlower(_Ch, &_Ctype));
			   }
			   virtual const _Elem*do_tolower(_Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range((const _Elem*)_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2340);
				   for(;
					   _First!=_Last;
					   ++_First)*_First=_Towlower(*_First, &_Ctype);
				   return ((const _Elem*)_First);
			   }
			   virtual _Elem do_toupper(_Elem _Ch)const
			   {
				   return (_Towupper(_Ch, &_Ctype));
			   }
			   virtual const _Elem*do_toupper(_Elem*_First, const _Elem*_Last)const
			   {
				   _Debug_range((const _Elem*)_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2354);
				   for(;
					   _First!=_Last;
					   ++_First)*_First=_Towupper(*_First, &_Ctype);
				   return ((const _Elem*)_First);
			   }
			   _Elem _Dowiden(char _Byte)const
			   {
				   mbstate_t _Mbst=
				   {
					   0
				   };
				   unsigned short _Wc;
				   return (_Mbrtowc((wchar_t*)&_Wc, &_Byte, 1, &_Mbst, &_Cvt)<0?(unsigned short)(wint_t)(0xFFFF): _Wc);
			   }
			   virtual _Elem do_widen(char _Byte)const
			   {
				   return (_Dowiden(_Byte));
			   }
			   virtual const char*do_widen(const char*_First, const char*_Last, _Elem*_Dest)const
			   {
				   return _Do_widen_s(_First, _Last, _Dest, _Last-_First);
			   }
			   virtual const char*_Do_widen_s(const char*_First, const char*_Last, _Elem*_Dest, size_t _Dest_size)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2384);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2385);

				   {
					   if(!(_Dest_size>=(size_t)(_Last-_First)))
					   {
						   (void)((!!((("_Dest_size >= (size_t)(_Last - _First)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2386, 0, L"(\"_Dest_size >= (size_t)(_Last - _First)\", 0)"))||(__debugbreak(), 0));
						   ::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2386, 0);
					   }
				   };
				   for(;
					   _First!=_Last;
					   ++_First, ++_Dest)*_Dest=_Dowiden(*_First);
				   return (_First);
			   }
			   char _Donarrow(_Elem _Ch, char _Dflt)const
			   {
				   char _Buf[5];
				   mbstate_t _Mbst=
				   {
					   0
				   };
				   return (_Wcrtomb(_Buf, _Ch, &_Mbst, &_Cvt)!=1?_Dflt: _Buf[0]);
			   }
			   virtual char do_narrow(_Elem _Ch, char _Dflt)const
			   {
				   return (_Donarrow(_Ch, _Dflt));
			   }
			   virtual const _Elem*do_narrow(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest)const
			   {
				   return _Do_narrow_s(_First, _Last, _Dflt, _Dest, _Last-_First);
			   }
			   virtual const _Elem*_Do_narrow_s(const _Elem*_First, const _Elem*_Last, char _Dflt, char*_Dest, size_t _Dest_size)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2418);
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2419);

				   {
					   if(!(_Dest_size>=(size_t)(_Last-_First)))
					   {
						   (void)((!!((("_Dest_size >= (size_t)(_Last - _First)", 0))))||(1!=_CrtDbgReportW(2, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2420, 0, L"(\"_Dest_size >= (size_t)(_Last - _First)\", 0)"))||(__debugbreak(), 0));
						   ::_invalid_parameter(L"\"out of range\"", L"<function-sig>", L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocale", 2420, 0);
					   }
				   };
				   for(;
					   _First!=_Last;
					   ++_First, ++_Dest)*_Dest=_Donarrow(*_First, _Dflt);
				   return (_First);
			   }
	private:
		_Locinfo::_Ctypevec _Ctype;
		_Locinfo::_Cvtvec _Cvt;
	};
	template<class _Elem>
	class ctype_byname: public ctype<_Elem>
	{
	public:
		explicit ctype_byname(const char*_Locname, size_t _Refs=0): ctype<_Elem>(_Locname, _Refs)
		{
		}
	protected: virtual~ctype_byname()
			   {
			   }
	};
	template<>
	class ctype_byname<char>: public ctype<char>
	{
	public:
		explicit ctype_byname(const char*_Locname, size_t _Refs=0): ctype<char>(_Locname, _Refs)
		{
		}
	protected: virtual~ctype_byname()
			   {
			   }
	};
}
namespace std
{
	template<class _Dummy>
	class _Iosb
	{
	public:
		enum _Dummy_enum
		{
			_Dummy_enum_val=1
		};
		enum _Fmtflags
		{
			_Fmtmask=0xffff, _Fmtzero=0
		};
		static const _Fmtflags skipws=(_Fmtflags)0x0001;
		static const _Fmtflags unitbuf=(_Fmtflags)0x0002;
		static const _Fmtflags uppercase=(_Fmtflags)0x0004;
		static const _Fmtflags showbase=(_Fmtflags)0x0008;
		static const _Fmtflags showpoint=(_Fmtflags)0x0010;
		static const _Fmtflags showpos=(_Fmtflags)0x0020;
		static const _Fmtflags left=(_Fmtflags)0x0040;
		static const _Fmtflags right=(_Fmtflags)0x0080;
		static const _Fmtflags internal=(_Fmtflags)0x0100;
		static const _Fmtflags dec=(_Fmtflags)0x0200;
		static const _Fmtflags oct=(_Fmtflags)0x0400;
		static const _Fmtflags hex=(_Fmtflags)0x0800;
		static const _Fmtflags scientific=(_Fmtflags)0x1000;
		static const _Fmtflags fixed=(_Fmtflags)0x2000;
		static const _Fmtflags boolalpha=(_Fmtflags)0x4000;
		static const _Fmtflags _Stdio=(_Fmtflags)0x8000;
		static const _Fmtflags adjustfield=(_Fmtflags)(0x0040|0x0080|0x0100);
		static const _Fmtflags basefield=(_Fmtflags)(0x0200|0x0400|0x0800);
		static const _Fmtflags floatfield=(_Fmtflags)(0x1000|0x2000);
		enum _Iostate
		{
			_Statmask=0x17
		};
		static const _Iostate goodbit=(_Iostate)0x0;
		static const _Iostate eofbit=(_Iostate)0x1;
		static const _Iostate failbit=(_Iostate)0x2;
		static const _Iostate badbit=(_Iostate)0x4;
		static const _Iostate _Hardfail=(_Iostate)0x10;
		enum _Openmode
		{
			_Openmask=0xff
		};
		static const _Openmode in=(_Openmode)0x01;
		static const _Openmode out=(_Openmode)0x02;
		static const _Openmode ate=(_Openmode)0x04;
		static const _Openmode app=(_Openmode)0x08;
		static const _Openmode trunc=(_Openmode)0x10;
		static const _Openmode _Nocreate=(_Openmode)0x40;
		static const _Openmode _Noreplace=(_Openmode)0x80;
		static const _Openmode binary=(_Openmode)0x20;
		enum _Seekdir
		{
			_Seekmask=0x3
		};
		static const _Seekdir beg=(_Seekdir)0;
		static const _Seekdir cur=(_Seekdir)1;
		static const _Seekdir end=(_Seekdir)2;
		enum
		{
			_Openprot=0x40
		};
	};
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::skipws;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::unitbuf;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::uppercase;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::showbase;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::showpoint;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::showpos;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::left;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::right;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::internal;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::dec;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::oct;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::hex;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::scientific;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::fixed;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::boolalpha;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::_Stdio;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::adjustfield;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::basefield;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Fmtflags _Iosb<_Dummy>::floatfield;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Iostate _Iosb<_Dummy>::goodbit;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Iostate _Iosb<_Dummy>::eofbit;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Iostate _Iosb<_Dummy>::failbit;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Iostate _Iosb<_Dummy>::badbit;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Iostate _Iosb<_Dummy>::_Hardfail;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Openmode _Iosb<_Dummy>::in;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Openmode _Iosb<_Dummy>::out;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Openmode _Iosb<_Dummy>::ate;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Openmode _Iosb<_Dummy>::app;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Openmode _Iosb<_Dummy>::trunc;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Openmode _Iosb<_Dummy>::_Nocreate;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Openmode _Iosb<_Dummy>::_Noreplace;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Openmode _Iosb<_Dummy>::binary;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Seekdir _Iosb<_Dummy>::beg;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Seekdir _Iosb<_Dummy>::cur;
	template<class _Dummy>
	const typename _Iosb<_Dummy>::_Seekdir _Iosb<_Dummy>::end;
	class ios_base: public _Iosb<int>
	{
	public:
		typedef int fmtflags;
		typedef int iostate;
		typedef int openmode;
		typedef int seekdir;
		typedef std::streamoff streamoff;
		typedef std::streampos streampos;
		enum event
		{
			erase_event, imbue_event, copyfmt_event
		};
		typedef void(*event_callback)(event, ios_base&, int);
		typedef unsigned int io_state, open_mode, seek_dir;
		class failure: public runtime_error
		{
		public:
			explicit failure(const string&_Message): runtime_error(_Message)
			{
			}
			virtual~failure()throw()
			{
			}
		};
		class Init
		{
		public:
			Init()
			{
				_Init_ctor(this);
			}
			~Init()
			{
				_Init_dtor(this);
			}
		private:
			static void _Init_ctor(Init*);
			static void _Init_dtor(Init*);
			static int&_Init_cnt_func();
			static int _Init_cnt;
		};
		ios_base&operator=(const ios_base&_Right)
		{
			if(this!=&_Right)
			{
				_Mystate=_Right._Mystate;
				copyfmt(_Right);
			}
			return (*this);
		}
		operator void*()const
		{
			return (fail()?0: (void*)this);
		}
		bool operator!()const
		{
			return (fail());
		}
		void clear(iostate _State, bool _Reraise)
		{
			_Mystate=(iostate)(_State&_Statmask);
			if((_Mystate&_Except)==0);
			else if(_Reraise)throw;
			else if(_Mystate&_Except&badbit)throw failure("ios_base::badbit set");
			else if(_Mystate&_Except&failbit)throw failure("ios_base::failbit set");
			else throw failure("ios_base::eofbit set");
		}
		void clear(iostate _State=goodbit)
		{
			clear(_State, false);
		}
		void clear(io_state _State)
		{
			clear((iostate)_State);
		}
		iostate rdstate()const
		{
			return (_Mystate);
		}
		void setstate(iostate _State, bool _Exreraise)
		{
			if(_State!=goodbit)clear((iostate)((int)rdstate()|(int)_State), _Exreraise);
		}
		void setstate(iostate _State)
		{
			if(_State!=goodbit)clear((iostate)((int)rdstate()|(int)_State), false);
		}
		void setstate(io_state _State)
		{
			setstate((iostate)_State);
		}
		bool good()const
		{
			return (rdstate()==goodbit);
		}
		bool eof()const
		{
			return ((int)rdstate()&(int)eofbit);
		}
		bool fail()const
		{
			return (((int)rdstate()&((int)badbit|(int)failbit))!=0);
		}
		bool bad()const
		{
			return (((int)rdstate()&(int)badbit)!=0);
		}
		iostate exceptions()const
		{
			return (_Except);
		}
		void exceptions(iostate _Newexcept)
		{
			_Except=(iostate)((int)_Newexcept&(int)_Statmask);
			clear(_Mystate);
		}
		void exceptions(io_state _State)
		{
			exceptions((iostate)_State);
		}
		fmtflags flags()const
		{
			return (_Fmtfl);
		}
		fmtflags flags(fmtflags _Newfmtflags)
		{
			fmtflags _Oldfmtflags=_Fmtfl;
			_Fmtfl=(fmtflags)((int)_Newfmtflags&(int)_Fmtmask);
			return (_Oldfmtflags);
		}
		fmtflags setf(fmtflags _Newfmtflags)
		{
			ios_base::fmtflags _Oldfmtflags=_Fmtfl;
			_Fmtfl=(fmtflags)((int)_Fmtfl|(int)_Newfmtflags&(int)_Fmtmask);
			return (_Oldfmtflags);
		}
		fmtflags setf(fmtflags _Newfmtflags, fmtflags _Mask)
		{
			ios_base::fmtflags _Oldfmtflags=_Fmtfl;
			_Fmtfl=(fmtflags)(((int)_Fmtfl&(int)~_Mask)|((int)_Newfmtflags&(int)_Mask&(int)_Fmtmask));
			return (_Oldfmtflags);
		}
		void unsetf(fmtflags _Mask)
		{
			_Fmtfl=(fmtflags)((int)_Fmtfl&(int)~_Mask);
		}
		streamsize precision()const
		{
			return (_Prec);
		}
		streamsize precision(streamsize _Newprecision)
		{
			streamsize _Oldprecision=_Prec;
			_Prec=_Newprecision;
			return (_Oldprecision);
		}
		streamsize width()const
		{
			return (_Wide);
		}
		streamsize width(streamsize _Newwidth)
		{
			streamsize _Oldwidth=_Wide;
			_Wide=_Newwidth;
			return (_Oldwidth);
		}
		locale getloc()const
		{
			return (*_Ploc);
		}
		locale imbue(const locale&_Loc)
		{
			locale _Oldlocale=*_Ploc;
			*_Ploc=_Loc;
			_Callfns(imbue_event);
			return (_Oldlocale);
		}
		static int xalloc()
		{

			{
				::std::_Lockit _Lock(2);
				return (_Index++);
			}
		}
		long&iword(int _Idx)
		{
			return (_Findarr(_Idx)._Lo);
		}
		void*&pword(int _Idx)
		{
			return (_Findarr(_Idx)._Vp);
		}
		void register_callback(event_callback _Pfn, int _Idx)
		{
			_Calls=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xiosbase", 449)_Fnarray(_Idx, _Pfn, _Calls);
		}
		ios_base&copyfmt(const ios_base&_Other)
		{
			if(this!=&_Other)
			{
				_Tidy();
				*_Ploc=*_Other._Ploc;
				_Fmtfl=_Other._Fmtfl;
				_Prec=_Other._Prec;
				_Wide=_Other._Wide;
				_Iosarray*_Ptr=_Other._Arr;
				for(_Arr=0;
					_Ptr!=0;
					_Ptr=_Ptr->_Next)if(_Ptr->_Lo!=0||_Ptr->_Vp!=0)
				{
					iword(_Ptr->_Index)=_Ptr->_Lo;
					pword(_Ptr->_Index)=_Ptr->_Vp;
				}
				for(_Fnarray*_Q=_Other._Calls;
					_Q!=0;
					_Q=_Q->_Next)register_callback(_Q->_Pfn, _Q->_Index);
				_Callfns(copyfmt_event);
				exceptions(_Other._Except);
			}
			return (*this);
		}
		virtual~ios_base()
		{
			_Ios_base_dtor(this);
		}
		static bool sync_with_stdio(bool _Newsync=true)
		{

			{
				::std::_Lockit _Lock(2);
				const bool _Oldsync=_Sync;
				_Sync=_Newsync;
				return (_Oldsync);
			}
		}
		static void _Addstd(ios_base*);
		size_t _Stdstr;
	protected: ios_base()
			   {
			   }
			   void _Init()
			   {
				   _Ploc=0;
				   _Except=goodbit;
				   _Fmtfl=skipws|dec;
				   _Prec=6;
				   _Wide=0;
				   _Arr=0;
				   _Calls=0;
				   clear(goodbit);
				   _Ploc=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xiosbase", 512)locale;
			   }
	private:
		struct _Iosarray
		{
		public:
			_Iosarray(int _Idx, _Iosarray*_Link): _Next(_Link), _Index(_Idx), _Lo(0), _Vp(0)
			{
			}
			_Iosarray*_Next;
			int _Index;
			long _Lo;
			void*_Vp;
		};
		struct _Fnarray
		{
			_Fnarray(int _Idx, event_callback _Pnew, _Fnarray*_Link): _Next(_Link), _Index(_Idx), _Pfn(_Pnew)
			{
			}
			_Fnarray*_Next;
			int _Index;
			event_callback _Pfn;
		};
		void _Callfns(event _Ev)
		{
			for(_Fnarray*_Ptr=_Calls;
				_Ptr!=0;
				_Ptr=_Ptr->_Next)(*_Ptr->_Pfn)(_Ev, *this, _Ptr->_Index);
		}
		_Iosarray&_Findarr(int _Idx)
		{
			_Iosarray*_Ptr, *_Q;
			for(_Ptr=_Arr, _Q=0;
				_Ptr!=0;
				_Ptr=_Ptr->_Next)if(_Ptr->_Index==_Idx)return (*_Ptr);
				else if(_Q==0&&_Ptr->_Lo==0&&_Ptr->_Vp==0)_Q=_Ptr;
				if(_Q!=0)
				{
					_Q->_Index=_Idx;
					return (*_Q);
				}
				_Arr=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xiosbase", 566)_Iosarray(_Idx, _Arr);
				return (*_Arr);
		}
		void _Tidy()
		{
			_Callfns(erase_event);
			_Iosarray*_Q1, *_Q2;
			for(_Q1=_Arr;
				_Q1!=0;
				_Q1=_Q2)_Q2=_Q1->_Next, std::_DebugHeapDelete(_Q1);
			_Arr=0;
			_Fnarray*_Q3, *_Q4;
			for(_Q3=_Calls;
				_Q3!=0;
				_Q3=_Q4)_Q4=_Q3->_Next, std::_DebugHeapDelete(_Q3);
			_Calls=0;
		}
		static void _Ios_base_dtor(ios_base*);
		iostate _Mystate;
		iostate _Except;
		fmtflags _Fmtfl;
		streamsize _Prec;
		streamsize _Wide;
		_Iosarray*_Arr;
		_Fnarray*_Calls;
		locale*_Ploc;
		static int _Index;
		static bool _Sync;
		static int&_Index_func();
		static bool&_Sync_func();
	};
}
namespace std
{
	template<class _Elem, class _Traits>
	class basic_streambuf
	{
		basic_streambuf(const basic_streambuf<_Elem, _Traits>&);
		basic_streambuf<_Elem, _Traits>&operator=(const basic_streambuf<_Elem, _Traits>&);
	protected: basic_streambuf(): _Plocale(new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\streambuf", 28)(locale))
			   {
				   _Init();
			   }
			   basic_streambuf(_Uninitialized)
			   {
			   }
	public:
		typedef basic_streambuf<_Elem, _Traits>_Myt;
		typedef _Elem char_type;
		typedef _Traits traits_type;
		virtual~basic_streambuf()
		{
			std::_DebugHeapDelete(_Plocale);
		}
		typedef typename _Traits::int_type int_type;
		typedef typename _Traits::pos_type pos_type;
		typedef typename _Traits::off_type off_type;
		pos_type pubseekoff(off_type _Off, ios_base::seekdir _Way, ios_base::openmode _Mode=ios_base::in|ios_base::out)
		{
			return (seekoff(_Off, _Way, _Mode));
		}
		pos_type pubseekoff(off_type _Off, ios_base::seek_dir _Way, ios_base::open_mode _Mode)
		{
			return (pubseekoff(_Off, (ios_base::seekdir)_Way, (ios_base::openmode)_Mode));
		}
		pos_type pubseekpos(pos_type _Pos, ios_base::openmode _Mode=ios_base::in|ios_base::out)
		{
			return (seekpos(_Pos, _Mode));
		}
		pos_type pubseekpos(pos_type _Pos, ios_base::open_mode _Mode)
		{
			return (seekpos(_Pos, (ios_base::openmode)_Mode));
		}
		_Myt*pubsetbuf(_Elem*_Buffer, streamsize _Count)
		{
			return (setbuf(_Buffer, _Count));
		}
		locale pubimbue(const locale&_Newlocale)
		{
			locale _Oldlocale=*_Plocale;
			imbue(_Newlocale);
			*_Plocale=_Newlocale;
			return (_Oldlocale);
		}
		locale getloc()const
		{
			return (*_Plocale);
		}
		streamsize in_avail()
		{
			streamsize _Res=_Gnavail();
			return (0<_Res?_Res: showmanyc());
		}
		int pubsync()
		{
			return (sync());
		}
		int_type sbumpc()
		{
			return (0<_Gnavail()?_Traits::to_int_type(*_Gninc()): uflow());
		}
		int_type sgetc()
		{
			return (0<_Gnavail()?_Traits::to_int_type(*gptr()): underflow());
		}
		streamsize _Sgetn_s(_Elem*_Ptr, size_t _Ptr_size, streamsize _Count)
		{
			return _Xsgetn_s(_Ptr, _Ptr_size, _Count);
		}
		streamsize sgetn(_Elem*_Ptr, streamsize _Count)
		{
			return xsgetn(_Ptr, _Count);
		}
		int_type snextc()
		{
			return (1<_Gnavail()?_Traits::to_int_type(*_Gnpreinc()): _Traits::eq_int_type(_Traits::eof(), sbumpc())?_Traits::eof(): sgetc());
		}
		int_type sputbackc(_Elem _Ch)
		{
			return (gptr()!=0&&eback()<gptr()&&_Traits::eq(_Ch, gptr()[-1])?_Traits::to_int_type(*_Gndec()): pbackfail(_Traits::to_int_type(_Ch)));
		}
		void stossc()
		{
			if(0<_Gnavail())_Gninc();
			else uflow();
		}
		int_type sungetc()
		{
			return (gptr()!=0&&eback()<gptr()?_Traits::to_int_type(*_Gndec()): pbackfail());
		}
		int_type sputc(_Elem _Ch)
		{
			return (0<_Pnavail()?_Traits::to_int_type(*_Pninc()=_Ch): overflow(_Traits::to_int_type(_Ch)));
		}
		streamsize sputn(const _Elem*_Ptr, streamsize _Count)
		{
			return (xsputn(_Ptr, _Count));
		}
		void _Lock()
		{
			_Mylock._Lock();
		}
		void _Unlock()
		{
			_Mylock._Unlock();
		}
	protected: _Elem*eback()const
			   {
				   return (*_IGfirst);
			   }
			   _Elem*gptr()const
			   {
				   return (*_IGnext);
			   }
			   _Elem*pbase()const
			   {
				   return (*_IPfirst);
			   }
			   _Elem*pptr()const
			   {
				   return (*_IPnext);
			   }
			   _Elem*egptr()const
			   {
				   return (*_IGnext+*_IGcount);
			   }
			   void gbump(int _Off)
			   {
				   *_IGcount-=_Off;
				   *_IGnext+=_Off;
			   }
			   void setg(_Elem*_First, _Elem*_Next, _Elem*_Last)
			   {
				   *_IGfirst=_First;
				   *_IGnext=_Next;
				   *_IGcount=(int)(_Last-_Next);
			   }
			   _Elem*epptr()const
			   {
				   return (*_IPnext+*_IPcount);
			   }
			   _Elem*_Gndec()
			   {
				   ++*_IGcount;
				   return (--*_IGnext);
			   }
			   _Elem*_Gninc()
			   {
				   --*_IGcount;
				   return ((*_IGnext)++);
			   }
			   _Elem*_Gnpreinc()
			   {
				   --*_IGcount;
				   return (++(*_IGnext));
			   }
			   streamsize _Gnavail()const
			   {
				   return (*_IGnext!=0?*_IGcount: 0);
			   }
			   void pbump(int _Off)
			   {
				   *_IPcount-=_Off;
				   *_IPnext+=_Off;
			   }
			   void setp(_Elem*_First, _Elem*_Last)
			   {
				   *_IPfirst=_First;
				   *_IPnext=_First;
				   *_IPcount=(int)(_Last-_First);
			   }
			   void setp(_Elem*_First, _Elem*_Next, _Elem*_Last)
			   {
				   *_IPfirst=_First;
				   *_IPnext=_Next;
				   *_IPcount=(int)(_Last-_Next);
			   }
			   _Elem*_Pninc()
			   {
				   --*_IPcount;
				   return ((*_IPnext)++);
			   }
			   streamsize _Pnavail()const
			   {
				   return (*_IPnext!=0?*_IPcount: 0);
			   }
			   void _Init()
			   {
				   _IGfirst=&_Gfirst, _IPfirst=&_Pfirst;
				   _IGnext=&_Gnext, _IPnext=&_Pnext;
				   _IGcount=&_Gcount, _IPcount=&_Pcount;
				   setp(0, 0), setg(0, 0, 0);
			   }
			   void _Init(_Elem**_Gf, _Elem**_Gn, int*_Gc, _Elem**_Pf, _Elem**_Pn, int*_Pc)
			   {
				   _IGfirst=_Gf, _IPfirst=_Pf;
				   _IGnext=_Gn, _IPnext=_Pn;
				   _IGcount=_Gc, _IPcount=_Pc;
			   }
			   virtual int_type overflow(int_type=_Traits::eof())
			   {
				   return (_Traits::eof());
			   }
			   virtual int_type pbackfail(int_type=_Traits::eof())
			   {
				   return (_Traits::eof());
			   }
			   virtual streamsize showmanyc()
			   {
				   return (0);
			   }
			   virtual int_type underflow()
			   {
				   return (_Traits::eof());
			   }
			   virtual int_type uflow()
			   {
				   return (_Traits::eq_int_type(_Traits::eof(), underflow())?_Traits::eof(): _Traits::to_int_type(*_Gninc()));
			   }
			   virtual streamsize xsgetn(_Elem*_Ptr, streamsize _Count)
			   {
				   return _Xsgetn_s(_Ptr, (size_t)-1, _Count);
			   }
			   virtual streamsize _Xsgetn_s(_Elem*_Ptr, size_t _Ptr_size, streamsize _Count)
			   {
				   int_type _Meta;
				   streamsize _Stream_size, _Size, _Copied;
				   for(_Copied=0;
					   0<_Count;
					   )if(0<(_Stream_size=_Gnavail()))
				   {
					   _Size=_Stream_size;
					   if(_Count<_Size)_Size=_Count;
					   _Traits_helper::copy_s<_Traits>(_Ptr, _Ptr_size, gptr(), _Size);
					   _Ptr+=_Size;
					   _Copied+=_Size;
					   _Count-=_Size;
					   gbump((int)_Size);
				   }
					   else if(_Traits::eq_int_type(_Traits::eof(), _Meta=uflow()))break;
					   else
					   {
						   *_Ptr++=_Traits::to_char_type(_Meta);
						   ++_Copied;
						   --_Count;
					   }
					   return (_Copied);
			   }
			   virtual streamsize xsputn(const _Elem*_Ptr, streamsize _Count)
			   {
				   streamsize _Stream_size, _Size, _Copied;
				   for(_Copied=0;
					   0<_Count;
					   )if(0<(_Stream_size=_Pnavail()))
				   {
					   _Size=_Stream_size;
					   if(_Count<_Size)_Size=_Count;
					   _Traits_helper::copy_s<_Traits>(pptr(), _Stream_size, _Ptr, _Size);
					   _Ptr+=_Size;
					   _Copied+=_Size;
					   _Count-=_Size;
					   pbump((int)_Size);
				   }
					   else if(_Traits::eq_int_type(_Traits::eof(), overflow(_Traits::to_int_type(*_Ptr))))break;
					   else
					   {
						   ++_Ptr;
						   ++_Copied;
						   --_Count;
					   }
					   return (_Copied);
			   }
			   virtual pos_type seekoff(off_type, ios_base::seekdir, ios_base::openmode=ios_base::in|ios_base::out)
			   {
				   return (streampos(_BADOFF));
			   }
			   virtual pos_type seekpos(pos_type, ios_base::openmode=ios_base::in|ios_base::out)
			   {
				   return (streampos(_BADOFF));
			   }
			   virtual _Myt*setbuf(_Elem*, streamsize)
			   {
				   return (this);
			   }
			   virtual int sync()
			   {
				   return (0);
			   }
			   virtual void imbue(const locale&)
			   {
			   }
	private:
		_Mutex _Mylock;
		_Elem*_Gfirst;
		_Elem*_Pfirst;
		_Elem**_IGfirst;
		_Elem**_IPfirst;
		_Elem*_Gnext;
		_Elem*_Pnext;
		_Elem**_IGnext;
		_Elem**_IPnext;
		int _Gcount;
		int _Pcount;
		int*_IGcount;
		int*_IPcount;
		locale*_Plocale;
	};
}
extern "C"
{
	extern long _Stolx(const char*, char**, int, int*);
	extern unsigned long _Stoulx(const char*, char**, int, int*);
	extern float _Stofx(const char*, char**, long, int*);
	extern double _Stodx(const char*, char**, long, int*);
	extern long double _Stoldx(const char*, char**, long, int*);
}
namespace std
{
	template<class _Elem>
	class numpunct: public locale::facet
	{
	public:
		typedef basic_string<_Elem, char_traits<_Elem>, allocator<_Elem> >string_type;
		typedef _Elem char_type;
		static locale::id id;
		_Elem decimal_point()const
		{
			return (do_decimal_point());
		}
		_Elem thousands_sep()const
		{
			return (do_thousands_sep());
		}
		string grouping()const
		{
			return (do_grouping());
		}
		string_type falsename()const
		{
			return (do_falsename());
		}
		string_type truename()const
		{
			return (do_truename());
		}
		explicit numpunct(size_t _Refs=0): locale::facet(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
				if(_Kseparator==0)_Kseparator=_Maklocchr(',', (_Elem*)0, _Lobj._Getcvt());
			}
		}
		numpunct(const _Locinfo&_Lobj, size_t _Refs=0): locale::facet(_Refs)
		{
			_Init(_Lobj);
		}
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 103)numpunct<_Elem>;
			return (4);
		}
	protected: virtual~numpunct()
			   {
				   _Tidy();
			   }
	protected: numpunct(const char*_Locname, size_t _Refs=0): locale::facet(_Refs)
			   {

				   {
					   _Locinfo _Lobj(_Locname);
					   _Init(_Lobj);
				   }
			   }
			   void _Init(const _Locinfo&_Lobj)
			   {
				   const lconv*_Ptr=_Lobj._Getlconv();
				   _Grouping=0;
				   _Falsename=0;
				   _Truename=0;
				   try
				   {
					   _Grouping=_Maklocstr(_Ptr->grouping, (char*)0, _Lobj._Getcvt());
					   _Falsename=_Maklocstr(_Lobj._Getfalse(), (_Elem*)0, _Lobj._Getcvt());
					   _Truename=_Maklocstr(_Lobj._Gettrue(), (_Elem*)0, _Lobj._Getcvt());
				   }
				   catch(...)
				   {
					   _Tidy();
					   throw;
				   }
				   _Dp=_Maklocchr(_Ptr->decimal_point[0], (_Elem*)0, _Lobj._Getcvt());
				   _Kseparator=_Maklocchr(_Ptr->thousands_sep[0], (_Elem*)0, _Lobj._Getcvt());
			   }
			   virtual _Elem do_decimal_point()const
			   {
				   return (_Dp);
			   }
			   virtual _Elem do_thousands_sep()const
			   {
				   return (_Kseparator);
			   }
			   virtual string do_grouping()const
			   {
				   return (string(_Grouping));
			   }
			   virtual string_type do_falsename()const
			   {
				   return (string_type(_Falsename));
			   }
			   virtual string_type do_truename()const
			   {
				   return (string_type(_Truename));
			   }
	private:
		void _Tidy()
		{
			std::_DebugHeapDelete((void*)(void*)_Grouping);
			std::_DebugHeapDelete((void*)(void*)_Falsename);
			std::_DebugHeapDelete((void*)(void*)_Truename);
		}
		const char*_Grouping;
		_Elem _Dp;
		_Elem _Kseparator;
		const _Elem*_Falsename;
		const _Elem*_Truename;
	};
	typedef numpunct<char>_Npc;
	typedef numpunct<wchar_t>_Npwc;
	template<class _Elem>
	class numpunct_byname: public numpunct<_Elem>
	{
	public:
		explicit numpunct_byname(const char*_Locname, size_t _Refs=0): numpunct<_Elem>(_Locname, _Refs)
		{
		}
	protected: virtual~numpunct_byname()
			   {
			   }
	};
	template<class _Elem>
	locale::id numpunct<_Elem>::id;
	template<class _Elem, class _InIt=istreambuf_iterator<_Elem, char_traits<_Elem> > >
	class num_get: public locale::facet
	{
	public:
		typedef numpunct<_Elem>_Mypunct;
		typedef basic_string<_Elem, char_traits<_Elem>, allocator<_Elem> >_Mystr;
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 222)num_get<_Elem, _InIt>;
			return (4);
		}
		static locale::id id;
	protected: virtual~num_get()
			   {
			   }
	protected: void _Init(const _Locinfo&_Lobj)
			   {
				   _Cvt=_Lobj._Getcvt();
			   }
			   _Locinfo::_Cvtvec _Cvt;
	public:
		explicit num_get(size_t _Refs=0): locale::facet(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
		}
		num_get(const _Locinfo&_Lobj, size_t _Refs=0): locale::facet(_Refs)
		{
			_Init(_Lobj);
		}
		typedef _Elem char_type;
		typedef _InIt iter_type;
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, _Bool&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, unsigned short&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, unsigned int&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, long&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, unsigned long&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, __int64&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, unsigned __int64&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, float&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, double&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, long double&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
		_InIt get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, void*&_Val)const
		{
			return (do_get(_First, _Last, _Iosbase, _State, _Val));
		}
	protected: virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, _Bool&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 343);
				   int _Ans=-1;
				   if(_Iosbase.flags()&ios_base::boolalpha)
				   {
					   typedef typename _Mystr::size_type _Mystrsize;
					   const _Mypunct&_Punct_fac=use_facet<_Mypunct>(_Iosbase.getloc());
					   _Mystr _Str((_Mystrsize)1, (char_type)0);
					   _Str+=_Punct_fac.falsename();
					   _Str+=(char_type)0;
					   _Str+=_Punct_fac.truename();
					   _Ans=_Getloctxt(_First, _Last, (size_t)2, _Str.c_str());
				   }
				   else
				   {
					   char _Ac[32], *_Ep;
					   int _Errno=0;
					   const unsigned long _Ulo=::_Stoulx(_Ac, &_Ep, _Getifld(_Ac, _First, _Last, _Iosbase.flags(), _Iosbase.getloc()), &_Errno);
					   if(_Ep!=_Ac&&_Errno==0&&_Ulo<=1)_Ans=_Ulo;
				   }
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ans<0)_State|=ios_base::failbit;
				   else _Val=_Ans!=0;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, unsigned short&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 380);
				   char _Ac[32], *_Ep;
				   int _Errno=0;
				   int _Base=_Getifld(_Ac, _First, _Last, _Iosbase.flags(), _Iosbase.getloc());
				   char*_Ptr=_Ac[0]=='-'?_Ac+1: _Ac;
				   const unsigned long _Ans=::_Stoulx(_Ptr, &_Ep, _Base, &_Errno);
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ptr||_Errno!=0||0xffff<_Ans)_State|=ios_base::failbit;
				   else _Val=(unsigned short)(_Ac[0]=='-'?0-_Ans: _Ans);
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, unsigned int&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 403);
				   char _Ac[32], *_Ep;
				   int _Errno=0;
				   int _Base=_Getifld(_Ac, _First, _Last, _Iosbase.flags(), _Iosbase.getloc());
				   char*_Ptr=_Ac[0]=='-'?_Ac+1: _Ac;
				   const unsigned long _Ans=::_Stoulx(_Ptr, &_Ep, _Base, &_Errno);
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ptr||_Errno!=0||0xffffffff<_Ans)_State|=ios_base::failbit;
				   else _Val=_Ac[0]=='-'?0-_Ans: _Ans;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, long&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 425);
				   char _Ac[32], *_Ep;
				   int _Errno=0;
				   const long _Ans=::_Stolx(_Ac, &_Ep, _Getifld(_Ac, _First, _Last, _Iosbase.flags(), _Iosbase.getloc()), &_Errno);
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ac||_Errno!=0)_State|=ios_base::failbit;
				   else _Val=_Ans;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, unsigned long&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 445);
				   char _Ac[32], *_Ep;
				   int _Errno=0;
				   const unsigned long _Ans=::_Stoulx(_Ac, &_Ep, _Getifld(_Ac, _First, _Last, _Iosbase.flags(), _Iosbase.getloc()), &_Errno);
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ac||_Errno!=0)_State|=ios_base::failbit;
				   else _Val=_Ans;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, __int64&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 466);
				   char _Ac[32], *_Ep;
				   int _Errno=0;
				   const __int64 _Ans=::_strtoi64(_Ac, &_Ep, _Getifld(_Ac, _First, _Last, _Iosbase.flags(), _Iosbase.getloc()));
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ac||_Errno!=0)_State|=ios_base::failbit;
				   else _Val=_Ans;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, unsigned __int64&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 486);
				   char _Ac[32], *_Ep;
				   int _Errno=0;
				   const unsigned __int64 _Ans=::_strtoui64(_Ac, &_Ep, _Getifld(_Ac, _First, _Last, _Iosbase.flags(), _Iosbase.getloc()));
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ac||_Errno!=0)_State|=ios_base::failbit;
				   else _Val=_Ans;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, float&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 507);
				   char _Ac[8+36+16], *_Ep;
				   int _Errno=0;
				   const float _Ans=::_Stofx(_Ac, &_Ep, _Getffld(_Ac, _First, _Last, _Iosbase.getloc()), &_Errno);
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ac||_Errno!=0)_State|=ios_base::failbit;
				   else _Val=_Ans;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, double&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 527);
				   char _Ac[8+36+16], *_Ep;
				   int _Errno=0;
				   const double _Ans=::_Stodx(_Ac, &_Ep, _Getffld(_Ac, _First, _Last, _Iosbase.getloc()), &_Errno);
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ac||_Errno!=0)_State|=ios_base::failbit;
				   else _Val=_Ans;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, long double&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 547);
				   char _Ac[8+36+16], *_Ep;
				   int _Errno=0;
				   const long double _Ans=::_Stoldx(_Ac, &_Ep, _Getffld(_Ac, _First, _Last, _Iosbase.getloc()), &_Errno);
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ac||_Errno!=0)_State|=ios_base::failbit;
				   else _Val=_Ans;
				   return (_First);
			   }
			   virtual _InIt do_get(_InIt _First, _InIt _Last, ios_base&_Iosbase, ios_base::iostate&_State, void*&_Val)const
			   {
				   _Debug_range(_First, _Last, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 567);
				   char _Ac[32], *_Ep;
				   int _Errno=0;
				   int _Base=_Getifld(_Ac, _First, _Last, ios_base::hex, _Iosbase.getloc());
				   const unsigned __int64 _Ans=(sizeof(void*)==sizeof(unsigned long))?(unsigned __int64)::_Stoulx(_Ac, &_Ep, _Base, &_Errno): ::_strtoui64(_Ac, &_Ep, _Base);
				   if(_First==_Last)_State|=ios_base::eofbit;
				   if(_Ep==_Ac||_Errno!=0)_State|=ios_base::failbit;
				   else _Val=(void*)(uintptr_t)_Ans;
				   return (_First);
			   }
	private:
		int _Getifld(char*_Ac, _InIt&_First, _InIt&_Last, ios_base::fmtflags _Basefield, const locale&_Loc)const
		{
			const _Mypunct&_Punct_fac=use_facet<_Mypunct>(_Loc);
			const string _Grouping=_Punct_fac.grouping();
			const _Elem _Kseparator=_Punct_fac.thousands_sep();
			const _Elem _E0=_Maklocchr('0', (_Elem*)0, _Cvt);
			char*_Ptr=_Ac;
			if(_First==_Last);
			else if(*_First==_Maklocchr('+', (_Elem*)0, _Cvt))*_Ptr++='+', ++_First;
			else if(*_First==_Maklocchr('-', (_Elem*)0, _Cvt))*_Ptr++='-', ++_First;
			_Basefield&=ios_base::basefield;
			int _Base=_Basefield==ios_base::oct?8: _Basefield==ios_base::hex?16: _Basefield==ios_base::_Fmtzero?0: 10;
			bool _Seendigit=false;
			bool _Nonzero=false;
			if(_First!=_Last&&*_First==_E0)
			{
				_Seendigit=true, ++_First;
				if(_First!=_Last&&(*_First==_Maklocchr('x', (_Elem*)0, _Cvt)||*_First==_Maklocchr('X', (_Elem*)0, _Cvt))&&(_Base==0||_Base==16))_Base=16, _Seendigit=false, ++_First;
				else if(_Base==0)_Base=8;
			}
			int _Dlen=_Base==0||_Base==10?10: _Base==8?8: 16+6;
			string _Groups((size_t)1, (char)_Seendigit);
			size_t _Group=0;
			for(char*const _Pe=&_Ac[32-1];
				_First!=_Last;
				++_First)if(::memchr("0123456789abcdefABCDEF", *_Ptr=_Maklocbyte((_Elem)*_First, _Cvt), _Dlen)!=0)
			{
				if((_Nonzero||*_Ptr!='0')&&_Ptr<_Pe)++_Ptr, _Nonzero=true;
				_Seendigit=true;
				if(_Groups[_Group]!=127)++_Groups[_Group];
			}
				else if(_Groups[_Group]=='\0'||_Kseparator==(_Elem)0||*_First!=_Kseparator)break;
				else
				{
					_Groups.append((string::size_type)1, '\0');
					++_Group;
				}
				if(_Group==0);
				else if('\0'<_Groups[_Group])++_Group;
				else _Seendigit=false;
				for(const char*_Pg=_Grouping.c_str();
					_Seendigit&&0<_Group;
					)if(*_Pg==127)break;
					else if(0<--_Group&&*_Pg!=_Groups[_Group]||0==_Group&&*_Pg<_Groups[_Group])_Seendigit=false;
					else if('\0'<_Pg[1])++_Pg;
					if(_Seendigit&&!_Nonzero)*_Ptr++='0';
					else if(!_Seendigit)_Ptr=_Ac;
					*_Ptr='\0';
					return (_Base);
		}
		int _Getffld(char*_Ac, _InIt&_First, _InIt&_Last, const locale&_Loc)const
		{
			const _Mypunct&_Punct_fac=use_facet<_Mypunct>(_Loc);
			const string _Grouping=_Punct_fac.grouping();
			const _Elem _E0=_Maklocchr('0', (_Elem*)0, _Cvt);
			char*_Ptr=_Ac;
			bool _Bad=false;
			if(_First==_Last);
			else if(*_First==_Maklocchr('+', (_Elem*)0, _Cvt))*_Ptr++='+', ++_First;
			else if(*_First==_Maklocchr('-', (_Elem*)0, _Cvt))*_Ptr++='-', ++_First;
			bool _Seendigit=false;
			int _Significant=0;
			int _Pten=0;
			if(*_Grouping.c_str()==127||*_Grouping.c_str()<='\0')for(;
			_First!=_Last&&_E0<=*_First&&*_First<=_E0+9;
			_Seendigit=true, ++_First)if(36<=_Significant)++_Pten;
			else if(*_First==_E0&&_Significant==0);
			else
			{
				*_Ptr++=(char)((*_First-_E0)+'0');
				++_Significant;
			}
			else
			{
				const _Elem _Kseparator=_Punct_fac.thousands_sep();
				string _Groups((size_t)1, '\0');
				size_t _Group=0;
				for(;
					_First!=_Last;
					++_First)if(_E0<=*_First&&*_First<=_E0+9)
				{
					_Seendigit=true;
					if(36<=_Significant)++_Pten;
					else if(*_First==_E0&&_Significant==0);
					else
					{
						*_Ptr++=(char)((*_First-_E0)+'0');
						++_Significant;
					}
					if(_Groups[_Group]!=127)++_Groups[_Group];
				}
					else if(_Groups[_Group]=='\0'||_Kseparator==(_Elem)0||*_First!=_Kseparator)break;
					else
					{
						_Groups.append((size_t)1, '\0');
						++_Group;
					}
					if(_Group==0);
					else if('\0'<_Groups[_Group])++_Group;
					else _Bad=true;
					for(const char*_Pg=_Grouping.c_str();
						!_Bad&&0<_Group;
						)if(*_Pg==127)break;
						else if(0<--_Group&&*_Pg!=_Groups[_Group]||0==_Group&&*_Pg<_Groups[_Group])_Bad=true;
						else if('\0'<_Pg[1])++_Pg;
			}
			if(_Seendigit&&_Significant==0)*_Ptr++='0';
			if(_First!=_Last&&*_First==_Punct_fac.decimal_point())*_Ptr++=localeconv()->decimal_point[0], ++_First;
			if(_Significant==0)
			{
				for(;
					_First!=_Last&&*_First==_E0;
					_Seendigit=true, ++_First)--_Pten;
				if(_Pten<0)*_Ptr++='0', ++_Pten;
			}
			for(;
				_First!=_Last&&_E0<=*_First&&*_First<=_E0+9;
				_Seendigit=true, ++_First)if(_Significant<36)
			{
				*_Ptr++=(char)((*_First-_E0)+'0');
				++_Significant;
			}
			if(_Seendigit&&_First!=_Last&&(*_First==_Maklocchr('e', (_Elem*)0, _Cvt)||*_First==_Maklocchr('E', (_Elem*)0, _Cvt)))
			{
				*_Ptr++='e', ++_First;
				_Seendigit=false, _Significant=0;
				if(_First==_Last);
				else if(*_First==_Maklocchr('+', (_Elem*)0, _Cvt))*_Ptr++='+', ++_First;
				else if(*_First==_Maklocchr('-', (_Elem*)0, _Cvt))*_Ptr++='-', ++_First;
				for(;
					_First!=_Last&&*_First==_E0;
					)_Seendigit=true, ++_First;
				if(_Seendigit)*_Ptr++='0';
				for(;
					_First!=_Last&&_E0<=*_First&&*_First<=_E0+9;
					_Seendigit=true, ++_First)if(_Significant<8)
				{
					*_Ptr++=(char)((*_First-_E0)+'0');
					++_Significant;
				}
			}
			if(_Bad||!_Seendigit)_Ptr=_Ac;
			*_Ptr='\0';
			return (_Pten);
		}
	};
	template<class _Elem, class _InIt>
	locale::id num_get<_Elem, _InIt>::id;
	template<class _Elem, class _OutIt=ostreambuf_iterator<_Elem, char_traits<_Elem> > >
	class num_put: public locale::facet
	{
	public:
		typedef numpunct<_Elem>_Mypunct;
		typedef basic_string<_Elem, char_traits<_Elem>, allocator<_Elem> >_Mystr;
		static size_t _Getcat(const locale::facet**_Ppf=0)
		{
			if(_Ppf!=0&&*_Ppf==0)*_Ppf=new(std::_DebugHeapTag_func(), "z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 840)num_put<_Elem, _OutIt>;
			return (4);
		}
		static locale::id id;
	protected: virtual~num_put()
			   {
			   }
	protected: void _Init(const _Locinfo&_Lobj)
			   {
				   _Cvt=_Lobj._Getcvt();
			   }
			   _Locinfo::_Cvtvec _Cvt;
	public:
		explicit num_put(size_t _Refs=0): locale::facet(_Refs)
		{

			{
				_Locinfo _Lobj;
				_Init(_Lobj);
			}
		}
		num_put(const _Locinfo&_Lobj, size_t _Refs=0): locale::facet(_Refs)
		{
			_Init(_Lobj);
		}
		typedef _Elem char_type;
		typedef _OutIt iter_type;
		_OutIt put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, _Bool _Val)const
		{
			return (do_put(_Dest, _Iosbase, _Fill, _Val));
		}
		_OutIt put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, long _Val)const
		{
			return (do_put(_Dest, _Iosbase, _Fill, _Val));
		}
		_OutIt put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, unsigned long _Val)const
		{
			return (do_put(_Dest, _Iosbase, _Fill, _Val));
		}
		_OutIt put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, __int64 _Val)const
		{
			return (do_put(_Dest, _Iosbase, _Fill, _Val));
		}
		_OutIt put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, unsigned __int64 _Val)const
		{
			return (do_put(_Dest, _Iosbase, _Fill, _Val));
		}
		_OutIt put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, double _Val)const
		{
			return (do_put(_Dest, _Iosbase, _Fill, _Val));
		}
		_OutIt put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, long double _Val)const
		{
			return (do_put(_Dest, _Iosbase, _Fill, _Val));
		}
		_OutIt put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, const void*_Val)const
		{
			return (do_put(_Dest, _Iosbase, _Fill, _Val));
		}
	protected: virtual _OutIt do_put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, _Bool _Val)const
			   {
				   _Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 931);
				   if(!(_Iosbase.flags()&ios_base::boolalpha))return (do_put(_Dest, _Iosbase, _Fill, (long)_Val));
				   else
				   {
					   const _Mypunct&_Punct_fac=use_facet<_Mypunct>(_Iosbase.getloc());
					   _Mystr _Str;
					   if(_Val)_Str.assign(_Punct_fac.truename());
					   else _Str.assign(_Punct_fac.falsename());
					   size_t _Fillcount=_Iosbase.width()<=0||(size_t)_Iosbase.width()<=_Str.size()?0: (size_t)_Iosbase.width()-_Str.size();
					   if((_Iosbase.flags()&ios_base::adjustfield)!=ios_base::left)
					   {
						   _Dest=_Rep(_Dest, _Fill, _Fillcount);
						   _Fillcount=0;
					   }
					   _Dest=_Put(_Dest, _Str.c_str(), _Str.size());
					   _Iosbase.width(0);
					   return (_Rep(_Dest, _Fill, _Fillcount));
				   }
			   }
			   virtual _OutIt do_put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, long _Val)const
			   {
				   const size_t _Buf_size=2*32;
				   char _Buf[_Buf_size], _Fmt[6];
				   return (_Iput(_Dest, _Iosbase, _Fill, _Buf, ::sprintf_s(_Buf, _Buf_size, _Ifmt(_Fmt, "ld", _Iosbase.flags()), _Val)));
			   }
			   virtual _OutIt do_put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, unsigned long _Val)const
			   {
				   const size_t _Buf_size=2*32;
				   char _Buf[_Buf_size], _Fmt[6];
				   return (_Iput(_Dest, _Iosbase, _Fill, _Buf, ::sprintf_s(_Buf, _Buf_size, _Ifmt(_Fmt, "lu", _Iosbase.flags()), _Val)));
			   }
			   virtual _OutIt do_put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, __int64 _Val)const
			   {
				   const size_t _Buf_size=2*32;
				   char _Buf[_Buf_size], _Fmt[8];
				   return (_Iput(_Dest, _Iosbase, _Fill, _Buf, ::sprintf_s(_Buf, _Buf_size, _Ifmt(_Fmt, "Ld", _Iosbase.flags()), _Val)));
			   }
			   virtual _OutIt do_put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, unsigned __int64 _Val)const
			   {
				   const size_t _Buf_size=2*32;
				   char _Buf[_Buf_size], _Fmt[8];
				   return (_Iput(_Dest, _Iosbase, _Fill, _Buf, ::sprintf_s(_Buf, _Buf_size, _Ifmt(_Fmt, "Lu", _Iosbase.flags()), _Val)));
			   }
			   virtual _OutIt do_put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, double _Val)const
			   {
				   const size_t _Buf_size=8+36+64;
				   char _Buf[_Buf_size], _Fmt[8];
				   streamsize _Precision=_Iosbase.precision()<=0&&!(_Iosbase.flags()&ios_base::fixed)?6: _Iosbase.precision();
				   int _Significance=36<_Precision?36: (int)_Precision;
				   _Precision-=_Significance;
				   size_t _Beforepoint=0;
				   size_t _Afterpoint=0;
				   if((_Iosbase.flags()&ios_base::floatfield)==ios_base::fixed)
				   {
					   bool _Signed=_Val<0;
					   if(_Signed)_Val=-_Val;
					   for(;
						   1e35<=_Val&&_Beforepoint<5000;
						   _Beforepoint+=10)_Val/=1e10;
					   if(0<_Val)for(;
					   10<=_Precision&&_Val<=1e-35&&_Afterpoint<5000;
					   _Afterpoint+=10)
					   {
						   _Val*=1e10;
						   _Precision-=10;
					   }
					   if(_Signed)_Val=-_Val;
				   }
				   return (_Fput(_Dest, _Iosbase, _Fill, _Buf, _Beforepoint, _Afterpoint, _Precision, ::sprintf_s(_Buf, _Buf_size, _Ffmt(_Fmt, 0, _Iosbase.flags()), _Significance, _Val)));
			   }
			   virtual _OutIt do_put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, long double _Val)const
			   {
				   const size_t _Buf_size=8+36+64;
				   char _Buf[_Buf_size], _Fmt[8];
				   streamsize _Precision=_Iosbase.precision()<=0&&!(_Iosbase.flags()&ios_base::fixed)?6: _Iosbase.precision();
				   int _Significance=36<_Precision?36: (int)_Precision;
				   _Precision-=_Significance;
				   size_t _Beforepoint=0;
				   size_t _Afterpoint=0;
				   if((_Iosbase.flags()&ios_base::floatfield)==ios_base::fixed)
				   {
					   bool _Signed=_Val<0;
					   if(_Signed)_Val=-_Val;
					   for(;
						   1e35<=_Val&&_Beforepoint<5000;
						   _Beforepoint+=10)_Val/=1e10;
					   if(0<_Val)for(;
					   10<=_Precision&&_Val<=1e-35&&_Afterpoint<5000;
					   _Afterpoint+=10)
					   {
						   _Val*=1e10;
						   _Precision-=10;
					   }
					   if(_Signed)_Val=-_Val;
				   }
				   return (_Fput(_Dest, _Iosbase, _Fill, _Buf, _Beforepoint, _Afterpoint, _Precision, ::sprintf_s(_Buf, _Buf_size, _Ffmt(_Fmt, 'L', _Iosbase.flags()), _Significance, _Val)));
			   }
			   virtual _OutIt do_put(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, const void*_Val)const
			   {
				   const size_t _Buf_size=2*32;
				   char _Buf[_Buf_size];
				   return (_Iput(_Dest, _Iosbase, _Fill, _Buf, ::sprintf_s(_Buf, _Buf_size, "%p", _Val)));
			   }
	private:
		char*_Ffmt(char*_Fmt, char _Spec, ios_base::fmtflags _Flags)const
		{
			char*_Ptr=_Fmt;
			*_Ptr++='%';
			if(_Flags&ios_base::showpos)*_Ptr++='+';
			if(_Flags&ios_base::showpoint)*_Ptr++='#';
			*_Ptr++='.';
			*_Ptr++='*';
			if(_Spec!='\0')*_Ptr++=_Spec;
			ios_base::fmtflags _Ffl=_Flags&ios_base::floatfield;
			*_Ptr++=_Ffl==ios_base::fixed?'f': _Ffl==ios_base::scientific?'e': 'g';
			*_Ptr='\0';
			return (_Fmt);
		}
		_OutIt _Fput(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, const char*_Buf, size_t _Beforepoint, size_t _Afterpoint, size_t _Trailing, size_t _Count)const
		{
			_Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 1119);
			const _Mypunct&_Punct_fac=use_facet<_Mypunct>(_Iosbase.getloc());
			const string _Grouping=_Punct_fac.grouping();
			const _Elem _Kseparator=_Punct_fac.thousands_sep();
			string _Groupstring;
			const _Elem _E0=_Maklocchr('0', (_Elem*)0, _Cvt);
			size_t _Prefix=_Buf[0]=='+'||_Buf[0]=='-'?1: 0;
			char _Enders[3];
			_Enders[0]=::localeconv()->decimal_point[0];
			_Enders[1]='e';
			_Enders[2]='\0';
			const char*_Eptr=(const char*)::memchr(_Buf, 'e', _Count);
			const char*_Pointptr=(const char*)::memchr(_Buf, _Enders[0], _Count);
			if(_Pointptr==0)_Trailing=0;
			if(*_Grouping.c_str()!=127&&'\0'<*_Grouping.c_str())
			{
				_Groupstring.append(_Buf, _Count);
				if(_Eptr==0)_Groupstring.append(_Trailing, '0');
				else
				{
					if(_Pointptr==0)
					{
						_Groupstring.append(_Beforepoint, '0');
						_Beforepoint=0;
					}
					_Groupstring.insert(_Eptr-_Buf, _Trailing, '0');
				}
				_Trailing=0;
				if(_Pointptr==0)_Groupstring.append(_Beforepoint, '0');
				else
				{
					_Groupstring.insert(_Pointptr-_Buf+1, _Afterpoint, '0');
					_Groupstring.insert(_Pointptr-_Buf, _Beforepoint, '0');
					_Afterpoint=0;
				}
				_Beforepoint=0;
				const char*_Pg=_Grouping.c_str();
				size_t _Off=::strcspn(&_Groupstring[0], &_Enders[0]);
				while(*_Pg!=127&&'\0'<*_Pg&&(size_t)*_Pg<_Off-_Prefix)
				{
					_Groupstring.insert(_Off-=*_Pg, (size_t)1, '\0');
					if('\0'<_Pg[1])++_Pg;
				}
				_Buf=&_Groupstring[0];
				_Trailing=0;
				_Count=_Groupstring.size();
			}
			size_t _Fillcount=_Beforepoint+_Afterpoint+_Trailing+_Count;
			_Fillcount=_Iosbase.width()<=0||(size_t)_Iosbase.width()<=_Fillcount?0: (size_t)_Iosbase.width()-_Fillcount;
			ios_base::fmtflags _Adjustfield=_Iosbase.flags()&ios_base::adjustfield;
			if(_Adjustfield!=ios_base::left&&_Adjustfield!=ios_base::internal)
			{
				_Dest=_Rep(_Dest, _Fill, _Fillcount);
				_Fillcount=0;
			}
			else if(_Adjustfield==ios_base::internal)
			{
				if(0<_Prefix)
				{
					_Dest=_Putc(_Dest, _Buf, 1);
					++_Buf, --_Count;
				}
				_Dest=_Rep(_Dest, _Fill, _Fillcount);
				_Fillcount=0;
			}
			_Pointptr=(const char*)::memchr(_Buf, _Enders[0], _Count);
			if(_Pointptr!=0)
			{
				size_t _Fracoffset=_Pointptr-_Buf+1;
				_Dest=_Putgrouped(_Dest, _Buf, _Fracoffset-1, _Kseparator);
				_Dest=_Rep(_Dest, _E0, _Beforepoint);
				_Dest=_Rep(_Dest, _Punct_fac.decimal_point(), 1);
				_Dest=_Rep(_Dest, _E0, _Afterpoint);
				_Buf+=_Fracoffset, _Count-=_Fracoffset;
			}
			_Eptr=(const char*)::memchr(_Buf, 'e', _Count);
			if(_Eptr!=0)
			{
				size_t _Expoffset=_Eptr-_Buf+1;
				_Dest=_Putgrouped(_Dest, _Buf, _Expoffset-1, _Kseparator);
				_Dest=_Rep(_Dest, _E0, _Trailing), _Trailing=0;
				_Dest=_Putc(_Dest, _Iosbase.flags()&ios_base::uppercase?"E": "e", 1);
				_Buf+=_Expoffset, _Count-=_Expoffset;
			}
			_Dest=_Putgrouped(_Dest, _Buf, _Count, _Kseparator);
			_Dest=_Rep(_Dest, _E0, _Trailing);
			_Iosbase.width(0);
			return (_Rep(_Dest, _Fill, _Fillcount));
		}
		char*_Ifmt(char*_Fmt, const char*_Spec, ios_base::fmtflags _Flags)const
		{
			char*_Ptr=_Fmt;
			*_Ptr++='%';
			if(_Flags&ios_base::showpos)*_Ptr++='+';
			if(_Flags&ios_base::showbase)*_Ptr++='#';
			if(_Spec[0]!='L')*_Ptr++=_Spec[0];
			else
			{
				*_Ptr++='I';
				*_Ptr++='6';
				*_Ptr++='4';
			}
			ios_base::fmtflags _Basefield=_Flags&ios_base::basefield;
			*_Ptr++=_Basefield==ios_base::oct?'o': _Basefield!=ios_base::hex?_Spec[1]: _Flags&ios_base::uppercase?'X': 'x';
			*_Ptr='\0';
			return (_Fmt);
		}
		_OutIt _Iput(_OutIt _Dest, ios_base&_Iosbase, _Elem _Fill, char*_Buf, size_t _Count)const
		{
			_Debug_pointer(_Dest, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\xlocnum", 1264);
			const _Mypunct&_Punct_fac=use_facet<_Mypunct>(_Iosbase.getloc());
			const string _Grouping=_Punct_fac.grouping();
			const size_t _Prefix=*_Buf=='+'||*_Buf=='-'?1: *_Buf=='0'&&(_Buf[1]=='x'||_Buf[1]=='X')?2: 0;
			if(*_Grouping.c_str()!=127&&'\0'<*_Grouping.c_str())
			{
				const char*_Pg=_Grouping.c_str();
				size_t _Off=_Count;
				while(*_Pg!=127&&'\0'<*_Pg&&(size_t)*_Pg<_Off-_Prefix)
				{
					_Off-=*_Pg;
					::memmove_s((&_Buf[_Off+1]), (_Count+1-_Off), (&_Buf[_Off]), (_Count+1-_Off));
					_Buf[_Off]='\0', ++_Count;
					if('\0'<_Pg[1])++_Pg;
				}
			}
			size_t _Fillcount=_Iosbase.width()<=0||(size_t)_Iosbase.width()<=_Count?0: (size_t)_Iosbase.width()-_Count;
			ios_base::fmtflags _Adjustfield=_Iosbase.flags()&ios_base::adjustfield;
			if(_Adjustfield!=ios_base::left&&_Adjustfield!=ios_base::internal)
			{
				_Dest=_Rep(_Dest, _Fill, _Fillcount);
				_Fillcount=0;
			}
			else if(_Adjustfield==ios_base::internal)
			{
				_Dest=_Putc(_Dest, _Buf, _Prefix);
				_Buf+=_Prefix, _Count-=_Prefix;
				_Dest=_Rep(_Dest, _Fill, _Fillcount), _Fillcount=0;
			}
			_Dest=_Putgrouped(_Dest, _Buf, _Count, _Punct_fac.thousands_sep());
			_Iosbase.width(0);
			return (_Rep(_Dest, _Fill, _Fillcount));
		}
		_OutIt _Put(_OutIt _Dest, const _Elem*_Ptr, size_t _Count)const
		{
			for(;
				0<_Count;
				--_Count, ++_Dest, ++_Ptr)*_Dest=*_Ptr;
			return (_Dest);
		}
		_OutIt _Putc(_OutIt _Dest, const char*_Ptr, size_t _Count)const
		{
			for(;
				0<_Count;
				--_Count, ++_Dest, ++_Ptr)*_Dest=_Maklocchr(*_Ptr, (_Elem*)0, _Cvt);
			return (_Dest);
		}
		_OutIt _Putgrouped(_OutIt _Dest, const char*_Ptr, size_t _Count, _Elem _Kseparator)const
		{
			for(;
				;
				++_Ptr, --_Count)
			{
				const char*_Pend=(const char*)::memchr(_Ptr, '\0', _Count);
				size_t _Groupsize=_Pend!=0?_Pend-_Ptr: _Count;
				_Dest=_Putc(_Dest, _Ptr, _Groupsize);
				_Ptr+=_Groupsize, _Count-=_Groupsize;
				if(_Count==0)break;
				if(_Kseparator!=(_Elem)0)_Dest=_Rep(_Dest, _Kseparator, 1);
			}
			return (_Dest);
		}
		_OutIt _Rep(_OutIt _Dest, _Elem _Ch, size_t _Count)const
		{
			for(;
				0<_Count;
				--_Count, ++_Dest)*_Dest=_Ch;
			return (_Dest);
		}
	};
	template<class _Elem, class _OutIt>
	locale::id num_put<_Elem, _OutIt>::id;
}
namespace std
{
	template<class _Elem, class _Traits>
	class basic_ios: public ios_base
	{
	public:
		typedef basic_ios<_Elem, _Traits>_Myt;
		typedef basic_ostream<_Elem, _Traits>_Myos;
		typedef basic_streambuf<_Elem, _Traits>_Mysb;
		typedef ctype<_Elem>_Ctype;
		typedef _Elem char_type;
		typedef _Traits traits_type;
		typedef typename _Traits::int_type int_type;
		typedef typename _Traits::pos_type pos_type;
		typedef typename _Traits::off_type off_type;
		explicit basic_ios(_Mysb*_Strbuf)
		{
			init(_Strbuf);
		}
		virtual~basic_ios()
		{
		}
		void clear(iostate _State=goodbit, bool _Reraise=false)
		{
			ios_base::clear((iostate)(_Mystrbuf==0?(int)_State|(int)badbit: (int)_State), _Reraise);
		}
		void clear(io_state _State)
		{
			clear((iostate)_State);
		}
		void setstate(iostate _State, bool _Reraise=false)
		{
			if(_State!=goodbit)clear((iostate)((int)rdstate()|(int)_State), _Reraise);
		}
		void setstate(io_state _State)
		{
			setstate((iostate)_State);
		}
		_Myt&copyfmt(const _Myt&_Right)
		{
			_Tiestr=_Right.tie();
			_Fillch=_Right.fill();
			ios_base::copyfmt(_Right);
			return (*this);
		}
		_Myos*tie()const
		{
			return (_Tiestr);
		}
		_Myos*tie(_Myos*_Newtie)
		{
			_Myos*_Oldtie=_Tiestr;
			_Tiestr=_Newtie;
			return (_Oldtie);
		}
		_Mysb*rdbuf()const
		{
			return (_Mystrbuf);
		}
		_Mysb*rdbuf(_Mysb*_Strbuf)
		{
			_Mysb*_Oldstrbuf=_Mystrbuf;
			_Mystrbuf=_Strbuf;
			clear();
			return (_Oldstrbuf);
		}
		locale imbue(const locale&_Loc)
		{
			locale _Oldlocale=ios_base::imbue(_Loc);
			if(rdbuf()!=0)rdbuf()->pubimbue(_Loc);
			return (_Oldlocale);
		}
		_Elem fill()const
		{
			return (_Fillch);
		}
		_Elem fill(_Elem _Newfill)
		{
			_Elem _Oldfill=_Fillch;
			_Fillch=_Newfill;
			return (_Oldfill);
		}
		char narrow(_Elem _Ch, char _Dflt='\0')const
		{
			const _Ctype&_Ctype_fac=use_facet<_Ctype>(getloc());
			return (_Ctype_fac.narrow(_Ch, _Dflt));
		}
		_Elem widen(char _Byte)const
		{
			const _Ctype&_Ctype_fac=use_facet<_Ctype>(getloc());
			return (_Ctype_fac.widen(_Byte));
		}
	protected: void init(_Mysb*_Strbuf=0, bool _Isstd=false)
			   {
				   _Init();
				   _Mystrbuf=_Strbuf;
				   _Tiestr=0;
				   _Fillch=widen(' ');
				   if(_Mystrbuf==0)setstate(badbit);
				   if(_Isstd)_Addstd(this);
				   else _Stdstr=0;
			   }
			   basic_ios()
			   {
			   }
	private:
		basic_ios(const _Myt&);
		_Myt&operator=(const _Myt&);
		_Mysb*_Mystrbuf;
		_Myos*_Tiestr;
		_Elem _Fillch;
	};
	inline ios_base&boolalpha(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::boolalpha);
		return (_Iosbase);
	}
	inline ios_base&dec(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::dec, ios_base::basefield);
		return (_Iosbase);
	}
	inline ios_base&fixed(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::fixed, ios_base::floatfield);
		return (_Iosbase);
	}
	inline ios_base&hex(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::hex, ios_base::basefield);
		return (_Iosbase);
	}
	inline ios_base&internal(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::internal, ios_base::adjustfield);
		return (_Iosbase);
	}
	inline ios_base&left(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::left, ios_base::adjustfield);
		return (_Iosbase);
	}
	inline ios_base&noboolalpha(ios_base&_Iosbase)
	{
		_Iosbase.unsetf(ios_base::boolalpha);
		return (_Iosbase);
	}
	inline ios_base&noshowbase(ios_base&_Iosbase)
	{
		_Iosbase.unsetf(ios_base::showbase);
		return (_Iosbase);
	}
	inline ios_base&noshowpoint(ios_base&_Iosbase)
	{
		_Iosbase.unsetf(ios_base::showpoint);
		return (_Iosbase);
	}
	inline ios_base&noshowpos(ios_base&_Iosbase)
	{
		_Iosbase.unsetf(ios_base::showpos);
		return (_Iosbase);
	}
	inline ios_base&noskipws(ios_base&_Iosbase)
	{
		_Iosbase.unsetf(ios_base::skipws);
		return (_Iosbase);
	}
	inline ios_base&nounitbuf(ios_base&_Iosbase)
	{
		_Iosbase.unsetf(ios_base::unitbuf);
		return (_Iosbase);
	}
	inline ios_base&nouppercase(ios_base&_Iosbase)
	{
		_Iosbase.unsetf(ios_base::uppercase);
		return (_Iosbase);
	}
	inline ios_base&oct(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::oct, ios_base::basefield);
		return (_Iosbase);
	}
	inline ios_base&right(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::right, ios_base::adjustfield);
		return (_Iosbase);
	}
	inline ios_base&scientific(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::scientific, ios_base::floatfield);
		return (_Iosbase);
	}
	inline ios_base&showbase(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::showbase);
		return (_Iosbase);
	}
	inline ios_base&showpoint(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::showpoint);
		return (_Iosbase);
	}
	inline ios_base&showpos(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::showpos);
		return (_Iosbase);
	}
	inline ios_base&skipws(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::skipws);
		return (_Iosbase);
	}
	inline ios_base&unitbuf(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::unitbuf);
		return (_Iosbase);
	}
	inline ios_base&uppercase(ios_base&_Iosbase)
	{
		_Iosbase.setf(ios_base::uppercase);
		return (_Iosbase);
	}
}
namespace std
{
	template<class _Elem, class _Traits>
	class basic_ostream: virtual public basic_ios<_Elem, _Traits>
	{
	public:
		typedef basic_ostream<_Elem, _Traits>_Myt;
		typedef basic_ios<_Elem, _Traits>_Myios;
		typedef basic_streambuf<_Elem, _Traits>_Mysb;
		typedef ostreambuf_iterator<_Elem, _Traits>_Iter;
		typedef num_put<_Elem, _Iter>_Nput;
		explicit basic_ostream(basic_streambuf<_Elem, _Traits>*_Strbuf, bool _Isstd=false)
		{
			_Myios::init(_Strbuf, _Isstd);
		}
		basic_ostream(_Uninitialized)
		{
			ios_base::_Addstd(this);
		}
		virtual~basic_ostream()
		{
		}
		typedef typename _Traits::int_type int_type;
		typedef typename _Traits::pos_type pos_type;
		typedef typename _Traits::off_type off_type;
		class _Sentry_base
		{
		public:
			_Sentry_base(_Myt&_Ostr): _Myostr(_Ostr)
			{
				if(_Myostr.rdbuf()!=0)_Myostr.rdbuf()->_Lock();
			}
			~_Sentry_base()
			{
				if(_Myostr.rdbuf()!=0)_Myostr.rdbuf()->_Unlock();
			}
			_Myt&_Myostr;
		};
		class sentry: public _Sentry_base
		{
		public:
			explicit sentry(_Myt&_Ostr): _Sentry_base(_Ostr)
			{
				if(_Ostr.good()&&_Ostr.tie()!=0)_Ostr.tie()->flush();
				_Ok=_Ostr.good();
			}
			~sentry()
			{
				if(!std::uncaught_exception())this->_Myostr._Osfx();
			}
			operator bool()const
			{
				return (_Ok);
			}
		private:
			sentry(const sentry&);
			sentry&operator=(const sentry&);
			bool _Ok;
		};
		bool opfx()
		{
			if(ios_base::good()&&_Myios::tie()!=0)_Myios::tie()->flush();
			return (ios_base::good());
		}
		void osfx()
		{
			_Osfx();
		}
		void _Osfx()
		{
			try
			{
				if(ios_base::flags()&ios_base::unitbuf)flush();
			}
			catch(...)
			{
			}
		}
		_Myt&operator<<(_Myt&(*_Pfn)(_Myt&))
		{
			_Debug_pointer(_Pfn, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\ostream", 170);
			return ((*_Pfn)(*this));
		}
		_Myt&operator<<(_Myios&(*_Pfn)(_Myios&))
		{
			_Debug_pointer(_Pfn, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\ostream", 176);
			(*_Pfn)(*(_Myios*)this);
			return (*this);
		}
		_Myt&operator<<(ios_base&(*_Pfn)(ios_base&))
		{
			_Debug_pointer(_Pfn, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\ostream", 183);
			(*_Pfn)(*(ios_base*)this);
			return (*this);
		}
		_Myt&operator<<(_Bool _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(short _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				ios_base::fmtflags _Bfl=ios_base::flags()&ios_base::basefield;
				long _Tmp=(_Bfl==ios_base::oct||_Bfl==ios_base::hex)?(long)(unsigned short)_Val: (long)_Val;
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Tmp).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(unsigned short _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), (unsigned long)_Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(int _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				ios_base::fmtflags _Bfl=ios_base::flags()&ios_base::basefield;
				long _Tmp=(_Bfl==ios_base::oct||_Bfl==ios_base::hex)?(long)(unsigned int)_Val: (long)_Val;
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Tmp).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(unsigned int _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), (unsigned long)_Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(long _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(unsigned long _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), (unsigned long)_Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(__int64 _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(unsigned __int64 _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(float _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), (double)_Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(double _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(long double _Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(const void*_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nput&_Nput_fac=use_facet<_Nput>(ios_base::getloc());
				try
				{
					if(_Nput_fac.put(_Iter(_Myios::rdbuf()), *this, _Myios::fill(), _Val).failed())_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator<<(_Mysb*_Strbuf)
		{
			ios_base::iostate _State=ios_base::goodbit;
			bool _Copied=false;
			const sentry _Ok(*this);
			if(_Ok&&_Strbuf!=0)for(int_type _Meta=_Traits::eof();
			;
			_Copied=true)
			{
				try
				{
					_Meta=_Traits::eq_int_type(_Traits::eof(), _Meta)?_Strbuf->sgetc(): _Strbuf->snextc();
				}
				catch(...)
				{
					_Myios::setstate(ios_base::failbit);
					throw;
				}
				if(_Traits::eq_int_type(_Traits::eof(), _Meta))break;
				try
				{
					if(_Traits::eq_int_type(_Traits::eof(), _Myios::rdbuf()->sputc(_Traits::to_char_type(_Meta))))
					{
						_State|=ios_base::badbit;
						break;
					}
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			ios_base::width(0);
			_Myios::setstate(_Strbuf==0?ios_base::badbit: !_Copied?_State|ios_base::failbit: _State);
			return (*this);
		}
		_Myt&put(_Elem _Ch)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(!_Ok)_State|=ios_base::badbit;
			else
			{
				try
				{
					if(_Traits::eq_int_type(_Traits::eof(), _Myios::rdbuf()->sputc(_Ch)))_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&write(const _Elem*_Str, streamsize _Count)
		{
			_Debug_pointer(_Str, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\ostream", 528);
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(!_Ok)_State|=ios_base::badbit;
			else
			{
				try
				{
					if(_Myios::rdbuf()->sputn(_Str, _Count)!=_Count)_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&flush()
		{
			ios_base::iostate _State=ios_base::goodbit;
			if(!ios_base::fail()&&_Myios::rdbuf()->pubsync()==-1)_State|=ios_base::badbit;
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&seekp(pos_type _Pos)
		{
			if(!ios_base::fail()&&(off_type)_Myios::rdbuf()->pubseekpos(_Pos, ios_base::out)==_BADOFF)_Myios::setstate(ios_base::failbit);
			return (*this);
		}
		_Myt&seekp(off_type _Off, ios_base::seekdir _Way)
		{
			if(!ios_base::fail()&&(off_type)_Myios::rdbuf()->pubseekoff(_Off, _Way, ios_base::out)==_BADOFF)_Myios::setstate(ios_base::failbit);
			return (*this);
		}
		pos_type tellp()
		{
			if(!ios_base::fail())return (_Myios::rdbuf()->pubseekoff(0, ios_base::cur, ios_base::out));
			else return (pos_type(_BADOFF));
		}
	};
	template<class _Elem, class _Traits>
	inline basic_ostream<_Elem, _Traits>&operator<<(basic_ostream<_Elem, _Traits>&_Ostr, const char*_Val)
	{
		ios_base::iostate _State=ios_base::goodbit;
		streamsize _Count=(streamsize)::strlen(_Val);
		streamsize _Pad=_Ostr.width()<=0||_Ostr.width()<=_Count?0: _Ostr.width()-_Count;
		const typename basic_ostream<_Elem, _Traits>::sentry _Ok(_Ostr);
		if(!_Ok)_State|=ios_base::badbit;
		else
		{
			try
			{
				const ctype<_Elem>&_Ctype_fac=use_facet<ctype<_Elem> >(_Ostr.getloc());
				if((_Ostr.flags()&ios_base::adjustfield)!=ios_base::left)for(;
				0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))
				{
					_State|=ios_base::badbit;
					break;
				}
				for(;
					_State==ios_base::goodbit&&0<_Count;
					--_Count, ++_Val)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ctype_fac.widen(*_Val))))_State|=ios_base::badbit;
				if(_State==ios_base::goodbit)for(;
				0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))
				{
					_State|=ios_base::badbit;
					break;
				}
				_Ostr.width(0);
			}
			catch(...)
			{
				(_Ostr).setstate(ios_base::badbit, true);
			}
		}
		_Ostr.setstate(_State);
		return (_Ostr);
	}
	template<class _Elem, class _Traits>
	inline basic_ostream<_Elem, _Traits>&operator<<(basic_ostream<_Elem, _Traits>&_Ostr, char _Ch)
	{
		ios_base::iostate _State=ios_base::goodbit;
		const typename basic_ostream<_Elem, _Traits>::sentry _Ok(_Ostr);
		if(_Ok)
		{
			const ctype<_Elem>&_Ctype_fac=use_facet<ctype<_Elem> >(_Ostr.getloc());
			streamsize _Pad=_Ostr.width()<=1?0: _Ostr.width()-1;
			try
			{
				if((_Ostr.flags()&ios_base::adjustfield)!=ios_base::left)for(;
				_State==ios_base::goodbit&&0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))_State|=ios_base::badbit;
				if(_State==ios_base::goodbit&&_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ctype_fac.widen(_Ch))))_State|=ios_base::badbit;
				for(;
					_State==ios_base::goodbit&&0<_Pad;
					--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))_State|=ios_base::badbit;
			}
			catch(...)
			{
				(_Ostr).setstate(ios_base::badbit, true);
			}
		}
		_Ostr.width(0);
		_Ostr.setstate(_State);
		return (_Ostr);
	}
	template<class _Traits>
	inline basic_ostream<char, _Traits>&operator<<(basic_ostream<char, _Traits>&_Ostr, const char*_Val)
	{
		typedef char _Elem;
		typedef basic_ostream<_Elem, _Traits>_Myos;
		ios_base::iostate _State=ios_base::goodbit;
		streamsize _Count=(streamsize)_Traits::length(_Val);
		streamsize _Pad=_Ostr.width()<=0||_Ostr.width()<=_Count?0: _Ostr.width()-_Count;
		const typename _Myos::sentry _Ok(_Ostr);
		if(!_Ok)_State|=ios_base::badbit;
		else
		{
			try
			{
				if((_Ostr.flags()&ios_base::adjustfield)!=ios_base::left)for(;
				0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))
				{
					_State|=ios_base::badbit;
					break;
				}
				if(_State==ios_base::goodbit&&_Ostr.rdbuf()->sputn(_Val, _Count)!=_Count)_State|=ios_base::badbit;
				if(_State==ios_base::goodbit)for(;
				0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))
				{
					_State|=ios_base::badbit;
					break;
				}
				_Ostr.width(0);
			}
			catch(...)
			{
				(_Ostr).setstate(ios_base::badbit, true);
			}
		}
		_Ostr.setstate(_State);
		return (_Ostr);
	}
	template<class _Traits>
	inline basic_ostream<char, _Traits>&operator<<(basic_ostream<char, _Traits>&_Ostr, char _Ch)
	{
		typedef char _Elem;
		typedef basic_ostream<_Elem, _Traits>_Myos;
		ios_base::iostate _State=ios_base::goodbit;
		const typename _Myos::sentry _Ok(_Ostr);
		if(_Ok)
		{
			streamsize _Pad=_Ostr.width()<=1?0: _Ostr.width()-1;
			try
			{
				if((_Ostr.flags()&ios_base::adjustfield)!=ios_base::left)for(;
				_State==ios_base::goodbit&&0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))_State|=ios_base::badbit;
				if(_State==ios_base::goodbit&&_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ch)))_State|=ios_base::badbit;
				for(;
					_State==ios_base::goodbit&&0<_Pad;
					--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))_State|=ios_base::badbit;
			}
			catch(...)
			{
				(_Ostr).setstate(ios_base::badbit, true);
			}
		}
		_Ostr.width(0);
		_Ostr.setstate(_State);
		return (_Ostr);
	}
	template<class _Elem, class _Traits>
	inline basic_ostream<_Elem, _Traits>&operator<<(basic_ostream<_Elem, _Traits>&_Ostr, const _Elem*_Val)
	{
		typedef basic_ostream<_Elem, _Traits>_Myos;
		ios_base::iostate _State=ios_base::goodbit;
		streamsize _Count=(streamsize)_Traits::length(_Val);
		streamsize _Pad=_Ostr.width()<=0||_Ostr.width()<=_Count?0: _Ostr.width()-_Count;
		const typename _Myos::sentry _Ok(_Ostr);
		if(!_Ok)_State|=ios_base::badbit;
		else
		{
			try
			{
				if((_Ostr.flags()&ios_base::adjustfield)!=ios_base::left)for(;
				0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))
				{
					_State|=ios_base::badbit;
					break;
				}
				if(_State==ios_base::goodbit&&_Ostr.rdbuf()->sputn(_Val, _Count)!=_Count)_State|=ios_base::badbit;
				if(_State==ios_base::goodbit)for(;
				0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))
				{
					_State|=ios_base::badbit;
					break;
				}
				_Ostr.width(0);
			}
			catch(...)
			{
				(_Ostr).setstate(ios_base::badbit, true);
			}
		}
		_Ostr.setstate(_State);
		return (_Ostr);
	}
	template<class _Elem, class _Traits>
	inline basic_ostream<_Elem, _Traits>&operator<<(basic_ostream<_Elem, _Traits>&_Ostr, _Elem _Ch)
	{
		typedef basic_ostream<_Elem, _Traits>_Myos;
		ios_base::iostate _State=ios_base::goodbit;
		const typename _Myos::sentry _Ok(_Ostr);
		if(_Ok)
		{
			streamsize _Pad=_Ostr.width()<=1?0: _Ostr.width()-1;
			try
			{
				if((_Ostr.flags()&ios_base::adjustfield)!=ios_base::left)for(;
				_State==ios_base::goodbit&&0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))_State|=ios_base::badbit;
				if(_State==ios_base::goodbit&&_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ch)))_State|=ios_base::badbit;
				for(;
					_State==ios_base::goodbit&&0<_Pad;
					--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))_State|=ios_base::badbit;
			}
			catch(...)
			{
				(_Ostr).setstate(ios_base::badbit, true);
			}
		}
		_Ostr.width(0);
		_Ostr.setstate(_State);
		return (_Ostr);
	}
	template<class _Traits>
	inline basic_ostream<char, _Traits>&operator<<(basic_ostream<char, _Traits>&_Ostr, const signed char*_Val)
	{
		return (_Ostr<<(const char*)_Val);
	}
	template<class _Traits>
	inline basic_ostream<char, _Traits>&operator<<(basic_ostream<char, _Traits>&_Ostr, signed char _Ch)
	{
		return (_Ostr<<(char)_Ch);
	}
	template<class _Traits>
	inline basic_ostream<char, _Traits>&operator<<(basic_ostream<char, _Traits>&_Ostr, const unsigned char*_Val)
	{
		return (_Ostr<<(const char*)_Val);
	}
	template<class _Traits>
	inline basic_ostream<char, _Traits>&operator<<(basic_ostream<char, _Traits>&_Ostr, unsigned char _Ch)
	{
		return (_Ostr<<(char)_Ch);
	}
	template<class _Elem, class _Traits>
	inline basic_ostream<_Elem, _Traits>&endl(basic_ostream<_Elem, _Traits>&_Ostr)
	{
		_Ostr.put(_Ostr.widen('\n'));
		_Ostr.flush();
		return (_Ostr);
	}
	template<class _Elem, class _Traits>
	inline basic_ostream<_Elem, _Traits>&ends(basic_ostream<_Elem, _Traits>&_Ostr)
	{
		_Ostr.put(_Elem());
		return (_Ostr);
	}
	template<class _Elem, class _Traits>
	inline basic_ostream<_Elem, _Traits>&flush(basic_ostream<_Elem, _Traits>&_Ostr)
	{
		_Ostr.flush();
		return (_Ostr);
	}
	inline basic_ostream<char, char_traits<char> >&endl(basic_ostream<char, char_traits<char> >&_Ostr)
	{
		_Ostr.put('\n');
		_Ostr.flush();
		return (_Ostr);
	}
	inline basic_ostream<wchar_t, char_traits<wchar_t> >&endl(basic_ostream<wchar_t, char_traits<wchar_t> >&_Ostr)
	{
		_Ostr.put('\n');
		_Ostr.flush();
		return (_Ostr);
	}
	inline basic_ostream<char, char_traits<char> >&ends(basic_ostream<char, char_traits<char> >&_Ostr)
	{
		_Ostr.put('\0');
		return (_Ostr);
	}
	inline basic_ostream<wchar_t, char_traits<wchar_t> >&ends(basic_ostream<wchar_t, char_traits<wchar_t> >&_Ostr)
	{
		_Ostr.put('\0');
		return (_Ostr);
	}
	inline basic_ostream<char, char_traits<char> >&flush(basic_ostream<char, char_traits<char> >&_Ostr)
	{
		_Ostr.flush();
		return (_Ostr);
	}
	inline basic_ostream<wchar_t, char_traits<wchar_t> >&flush(basic_ostream<wchar_t, char_traits<wchar_t> >&_Ostr)
	{
		_Ostr.flush();
		return (_Ostr);
	}
}
namespace std
{
	template<class _Elem, class _Traits>
	class basic_istream: virtual public basic_ios<_Elem, _Traits>
	{
	public:
		typedef basic_istream<_Elem, _Traits>_Myt;
		typedef basic_ios<_Elem, _Traits>_Myios;
		typedef basic_streambuf<_Elem, _Traits>_Mysb;
		typedef istreambuf_iterator<_Elem, _Traits>_Iter;
		typedef ctype<_Elem>_Ctype;
		typedef num_get<_Elem, _Iter>_Nget;
		explicit basic_istream(_Mysb*_Strbuf, bool _Isstd=false): _Chcount(0)
		{
			_Myios::init(_Strbuf, _Isstd);
		}
		basic_istream(_Uninitialized)
		{
			ios_base::_Addstd(this);
		}
		virtual~basic_istream()
		{
		}
		typedef typename _Traits::int_type int_type;
		typedef typename _Traits::pos_type pos_type;
		typedef typename _Traits::off_type off_type;
		class _Sentry_base
		{
		public:
			_Sentry_base(_Myt&_Istr): _Myistr(_Istr)
			{
				if(_Myistr.rdbuf()!=0)_Myistr.rdbuf()->_Lock();
			}
			~_Sentry_base()
			{
				if(_Myistr.rdbuf()!=0)_Myistr.rdbuf()->_Unlock();
			}
			_Myt&_Myistr;
		};
		class sentry: public _Sentry_base
		{
		public:
			explicit sentry(_Myt&_Istr, bool _Noskip=false): _Sentry_base(_Istr)
			{
				_Ok=this->_Myistr._Ipfx(_Noskip);
			}
			operator bool()const
			{
				return (_Ok);
			}
		private:
			sentry(const sentry&);
			sentry&operator=(const sentry&);
			bool _Ok;
		};
		bool _Ipfx(bool _Noskip=false)
		{
			if(ios_base::good())
			{
				if(_Myios::tie()!=0)_Myios::tie()->flush();
				if(!_Noskip&&ios_base::flags()&ios_base::skipws)
				{
					const _Ctype&_Ctype_fac=use_facet<_Ctype>(ios_base::getloc());
					try
					{
						int_type _Meta=_Myios::rdbuf()->sgetc();
						for(;
							;
							_Meta=_Myios::rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
						{
							_Myios::setstate(ios_base::eofbit);
							break;
						}
							else if(!_Ctype_fac.is(_Ctype::space, _Traits::to_char_type(_Meta)))break;
					}
					catch(...)
					{
						_Myios::setstate(ios_base::badbit, true);
					}
				}
				if(ios_base::good())return (true);
			}
			_Myios::setstate(ios_base::failbit);
			return (false);
		}
		bool ipfx(bool _Noskip=false)
		{
			return _Ipfx(_Noskip);
		}
		void isfx()
		{
		}
		_Myt&operator>>(_Myt&(*_Pfn)(_Myt&))
		{
			_Debug_pointer(_Pfn, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\istream", 157);
			return ((*_Pfn)(*this));
		}
		_Myt&operator>>(_Myios&(*_Pfn)(_Myios&))
		{
			_Debug_pointer(_Pfn, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\istream", 163);
			(*_Pfn)(*(_Myios*)this);
			return (*this);
		}
		_Myt&operator>>(ios_base&(*_Pfn)(ios_base&))
		{
			_Debug_pointer(_Pfn, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\istream", 170);
			(*_Pfn)(*(ios_base*)this);
			return (*this);
		}
		_Myt&operator>>(_Bool&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(short&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				long _Tmp=0;
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Tmp);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
				if(_State&ios_base::failbit||_Tmp<(-32768)||32767<_Tmp)_State|=ios_base::failbit;
				else _Val=(short)_Tmp;
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(unsigned short&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(int&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				long _Tmp=0;
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Tmp);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
				if(_State&ios_base::failbit||_Tmp<(-2147483647-1)||2147483647<_Tmp)_State|=ios_base::failbit;
				else _Val=_Tmp;
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(unsigned int&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(long&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(unsigned long&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, (unsigned long)_Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(__int64&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(unsigned __int64&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(float&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(double&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(long double&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(void*&_Val)
		{
			ios_base::iostate _State=ios_base::goodbit;
			const sentry _Ok(*this);
			if(_Ok)
			{
				const _Nget&_Nget_fac=use_facet<_Nget>(ios_base::getloc());
				try
				{
					_Nget_fac.get(_Iter(_Myios::rdbuf()), _Iter(0), *this, _State, _Val);
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&operator>>(_Mysb*_Strbuf)
		{
			ios_base::iostate _State=ios_base::goodbit;
			bool _Copied=false;
			const sentry _Ok(*this);
			if(_Ok&&_Strbuf!=0)
			{
				try
				{
					int_type _Meta=_Myios::rdbuf()->sgetc();
					for(;
						;
						_Meta=_Myios::rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
					{
						_State|=ios_base::eofbit;
						break;
					}
						else
						{
							try
							{
								if(_Traits::eq_int_type(_Traits::eof(), _Strbuf->sputc(_Traits::to_char_type(_Meta))))break;
							}
							catch(...)
							{
								break;
							}
							_Copied=true;
					}
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(!_Copied?_State|ios_base::failbit: _State);
			return (*this);
		}
		int_type get()
		{
			int_type _Meta=0;
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			if(!_Ok)_Meta=_Traits::eof();
			else
			{
				try
				{
					_Meta=_Myios::rdbuf()->sbumpc();
					if(_Traits::eq_int_type(_Traits::eof(), _Meta))_State|=ios_base::eofbit|ios_base::failbit;
					else++_Chcount;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (_Meta);
		}
		_Myt&get(_Elem*_Str, streamsize _Count)
		{
			return (get(_Str, _Count, _Myios::widen('\n')));
		}
		_Myt&get(_Elem*_Str, streamsize _Count, _Elem _Delim)
		{
			_Debug_pointer(_Str, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\istream", 509);
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			if(_Ok&&0<_Count)
			{
				try
				{
					int_type _Meta=_Myios::rdbuf()->sgetc();
					for(;
						0<--_Count;
						_Meta=_Myios::rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
					{
						_State|=ios_base::eofbit;
						break;
					}
						else if(_Traits::to_char_type(_Meta)==_Delim)break;
						else
						{
							*_Str++=_Traits::to_char_type(_Meta);
							++_Chcount;
						}
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_Chcount==0?_State|ios_base::failbit: _State);
			*_Str=_Elem();
			return (*this);
		}
		_Myt&get(_Elem&_Ch)
		{
			int_type _Meta=get();
			if(!_Traits::eq_int_type(_Traits::eof(), _Meta))_Ch=_Traits::to_char_type(_Meta);
			return (*this);
		}
		_Myt&get(_Mysb&_Strbuf)
		{
			return (get(_Strbuf, _Myios::widen('\n')));
		}
		_Myt&get(_Mysb&_Strbuf, _Elem _Delim)
		{
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			if(_Ok)
			{
				try
				{
					int_type _Meta=_Myios::rdbuf()->sgetc();
					for(;
						;
						_Meta=_Myios::rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
					{
						_State|=ios_base::eofbit;
						break;
					}
						else
						{
							try
							{
								_Elem _Ch=_Traits::to_char_type(_Meta);
								if(_Ch==_Delim||_Traits::eq_int_type(_Traits::eof(), _Strbuf.sputc(_Ch)))break;
							}
							catch(...)
							{
								break;
							}
							++_Chcount;
					}
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			if(_Chcount==0)_State|=ios_base::failbit;
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&getline(_Elem*_Str, streamsize _Count)
		{
			return (getline(_Str, _Count, _Myios::widen('\n')));
		}
		_Myt&getline(_Elem*_Str, streamsize _Count, _Elem _Delim)
		{
			_Debug_pointer(_Str, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\istream", 601);
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			if(_Ok&&0<_Count)
			{
				int_type _Metadelim=_Traits::to_int_type(_Delim);
				try
				{
					int_type _Meta=_Myios::rdbuf()->sgetc();
					for(;
						;
						_Meta=_Myios::rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
					{
						_State|=ios_base::eofbit;
						break;
					}
						else if(_Meta==_Metadelim)
						{
							++_Chcount;
							_Myios::rdbuf()->sbumpc();
							break;
					}
						else if(--_Count<=0)
						{
							_State|=ios_base::failbit;
							break;
						}
						else
						{
							++_Chcount;
							*_Str++=_Traits::to_char_type(_Meta);
						}
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			*_Str=_Elem();
			_Myios::setstate(_Chcount==0?_State|ios_base::failbit: _State);
			return (*this);
		}
		_Myt&ignore(streamsize _Count=1, int_type _Metadelim=_Traits::eof())
		{
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			if(_Ok&&0<_Count)
			{
				try
				{
					for(;
						;
						)
					{
						int_type _Meta;
						if(_Count!=2147483647&&--_Count<0)break;
						else if(_Traits::eq_int_type(_Traits::eof(), _Meta=_Myios::rdbuf()->sbumpc()))
						{
							_State|=ios_base::eofbit;
							break;
						}
						else
						{
							++_Chcount;
							if(_Meta==_Metadelim)break;
						}
					}
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&_Read_s(_Elem*_Str, size_t _Str_size, streamsize _Count)
		{
			_Debug_pointer(_Str, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\istream", 680);
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			if(_Ok)
			{
				try
				{
					const streamsize _Num=_Myios::rdbuf()->_Sgetn_s(_Str, _Str_size, _Count);
					_Chcount+=_Num;
					if(_Num!=_Count)_State|=ios_base::eofbit|ios_base::failbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&read(_Elem*_Str, streamsize _Count)
		{
			return _Read_s(_Str, (size_t)-1, _Count);
		}
		streamsize _Readsome_s(_Elem*_Str, size_t _Str_size, streamsize _Count)
		{
			_Debug_pointer(_Str, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\istream", 706);
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			streamsize _Num;
			if(!_Ok)_State|=ios_base::failbit;
			else if((_Num=_Myios::rdbuf()->in_avail())<0)_State|=ios_base::eofbit;
			else if(0<_Num)_Read_s(_Str, _Str_size, _Num<_Count?_Num: _Count);
			_Myios::setstate(_State);
			return (gcount());
		}
		streamsize readsome(_Elem*_Str, streamsize _Count)
		{
			return _Readsome_s(_Str, (size_t)-1, _Count);
		}
		int_type peek()
		{
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			int_type _Meta=0;
			const sentry _Ok(*this, true);
			if(!_Ok)_Meta=_Traits::eof();
			else
			{
				try
				{
					if(_Traits::eq_int_type(_Traits::eof(), _Meta=_Myios::rdbuf()->sgetc()))_State|=ios_base::eofbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (_Meta);
		}
		_Myt&putback(_Elem _Ch)
		{
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			if(_Ok)
			{
				try
				{
					if(_Traits::eq_int_type(_Traits::eof(), _Myios::rdbuf()->sputbackc(_Ch)))_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		_Myt&unget()
		{
			ios_base::iostate _State=ios_base::goodbit;
			_Chcount=0;
			const sentry _Ok(*this, true);
			if(_Ok)
			{
				try
				{
					if(_Traits::eq_int_type(_Traits::eof(), _Myios::rdbuf()->sungetc()))_State|=ios_base::badbit;
				}
				catch(...)
				{
					_Myios::setstate(ios_base::badbit, true);
				}
			}
			_Myios::setstate(_State);
			return (*this);
		}
		streamsize gcount()const
		{
			return (_Chcount);
		}
		int sync()
		{
			ios_base::iostate _State=ios_base::goodbit;
			int _Ans;
			if(_Myios::rdbuf()==0)_Ans=-1;
			else if(_Myios::rdbuf()->pubsync()==-1)
			{
				_State|=ios_base::badbit;
				_Ans=-1;
			}
			else _Ans=0;
			_Myios::setstate(_State);
			return (_Ans);
		}
		_Myt&seekg(pos_type _Pos)
		{
			if(!ios_base::fail()&&(off_type)_Myios::rdbuf()->pubseekpos(_Pos, ios_base::in)==_BADOFF)_Myios::setstate(ios_base::failbit);
			return (*this);
		}
		_Myt&seekg(off_type _Off, ios_base::seekdir _Way)
		{
			if(!ios_base::fail()&&(off_type)_Myios::rdbuf()->pubseekoff(_Off, _Way, ios_base::in)==_BADOFF)_Myios::setstate(ios_base::failbit);
			return (*this);
		}
		pos_type tellg()
		{
			if(!ios_base::fail())return (_Myios::rdbuf()->pubseekoff(0, ios_base::cur, ios_base::in));
			else return (pos_type(_BADOFF));
		}
	private:
		streamsize _Chcount;
	};
	template<class _Elem, class _Traits>
	class basic_iostream: public basic_istream<_Elem, _Traits>, public basic_ostream<_Elem, _Traits>
	{
	public:
		typedef _Elem char_type;
		typedef _Traits traits_type;
		typedef typename _Traits::int_type int_type;
		typedef typename _Traits::pos_type pos_type;
		typedef typename _Traits::off_type off_type;
		explicit basic_iostream(basic_streambuf<_Elem, _Traits>*_Strbuf): basic_istream<_Elem, _Traits>(_Strbuf, false), basic_ostream<_Elem, _Traits>(_Strbuf)
		{
		}
		virtual~basic_iostream()
		{
		}
	};
	template<class _Elem, class _Traits>
	inline basic_istream<_Elem, _Traits>&operator>>(basic_istream<_Elem, _Traits>&_Istr, _Elem*_Str)
	{
		_Debug_pointer(_Str, L"z:\\work\\cpparch\\cppparse\\msvc-8.0\\INCLUDE\\istream", 933);
		typedef basic_istream<_Elem, _Traits>_Myis;
		typedef ctype<_Elem>_Ctype;
		ios_base::iostate _State=ios_base::goodbit;
		_Elem*_Str0=_Str;
		const typename _Myis::sentry _Ok(_Istr);
		if(_Ok)
		{
			const _Ctype&_Ctype_fac=use_facet<_Ctype>(_Istr.getloc());
			try
			{
				streamsize _Count=0<_Istr.width()?_Istr.width(): 2147483647;
				typename _Myis::int_type _Meta=_Istr.rdbuf()->sgetc();
				_Elem _Ch;
				for(;
					0<--_Count;
					_Meta=_Istr.rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
				{
					_State|=ios_base::eofbit;
					break;
				}
					else if(_Ctype_fac.is(_Ctype::space, _Ch=_Traits::to_char_type(_Meta))||_Ch==_Elem())break;
					else*_Str++=_Traits::to_char_type(_Meta);
			}
			catch(...)
			{
				(_Istr).setstate(ios_base::badbit, true);
			}
		}
		*_Str=_Elem();
		_Istr.width(0);
		_Istr.setstate(_Str==_Str0?_State|ios_base::failbit: _State);
		return (_Istr);
	}
	template<class _Elem, class _Traits>
	inline basic_istream<_Elem, _Traits>&operator>>(basic_istream<_Elem, _Traits>&_Istr, _Elem&_Ch)
	{
		typedef basic_istream<_Elem, _Traits>_Myis;
		typename _Myis::int_type _Meta;
		ios_base::iostate _State=ios_base::goodbit;
		const typename _Myis::sentry _Ok(_Istr);
		if(_Ok)
		{
			try
			{
				_Meta=_Istr.rdbuf()->sbumpc();
				if(_Traits::eq_int_type(_Traits::eof(), _Meta))_State|=ios_base::eofbit|ios_base::failbit;
				else _Ch=_Traits::to_char_type(_Meta);
			}
			catch(...)
			{
				(_Istr).setstate(ios_base::badbit, true);
			}
		}
		_Istr.setstate(_State);
		return (_Istr);
	}
	template<class _Traits>
	inline basic_istream<char, _Traits>&operator>>(basic_istream<char, _Traits>&_Istr, signed char*_Str)
	{
		return (_Istr>>(char*)_Str);
	}
	template<class _Traits>
	inline basic_istream<char, _Traits>&operator>>(basic_istream<char, _Traits>&_Istr, signed char&_Ch)
	{
		return (_Istr>>(char&)_Ch);
	}
	template<class _Traits>
	inline basic_istream<char, _Traits>&operator>>(basic_istream<char, _Traits>&_Istr, unsigned char*_Str)
	{
		return (_Istr>>(char*)_Str);
	}
	template<class _Traits>
	inline basic_istream<char, _Traits>&operator>>(basic_istream<char, _Traits>&_Istr, unsigned char&_Ch)
	{
		return (_Istr>>(char&)_Ch);
	}
	template<class _Elem, class _Traits>
	inline basic_istream<_Elem, _Traits>&ws(basic_istream<_Elem, _Traits>&_Istr)
	{
		typedef basic_istream<_Elem, _Traits>_Myis;
		typedef ctype<_Elem>_Ctype;
		if(!_Istr.eof())
		{
			ios_base::iostate _State=ios_base::goodbit;
			const typename _Myis::sentry _Ok(_Istr, true);
			if(_Ok)
			{
				const _Ctype&_Ctype_fac=use_facet<_Ctype>(_Istr.getloc());
				try
				{
					for(typename _Traits::int_type _Meta=_Istr.rdbuf()->sgetc();
						;
						_Meta=_Istr.rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
					{
						_State|=ios_base::eofbit;
						break;
					}
						else if(!_Ctype_fac.is(_Ctype::space, _Traits::to_char_type(_Meta)))break;
				}
				catch(...)
				{
					(_Istr).setstate(ios_base::badbit, true);
				}
			}
			_Istr.setstate(_State);
		}
		return (_Istr);
	}
	inline basic_istream<char, char_traits<char> >&ws(basic_istream<char, char_traits<char> >&_Istr)
	{
		typedef char _Elem;
		typedef char_traits<_Elem>_Traits;
		if(!_Istr.eof())
		{
			ios_base::iostate _State=ios_base::goodbit;
			const basic_istream<_Elem, _Traits>::sentry _Ok(_Istr, true);
			if(_Ok)
			{
				const ctype<_Elem>&_Ctype_fac=use_facet<ctype<_Elem> >(_Istr.getloc());
				try
				{
					for(_Traits::int_type _Meta=_Istr.rdbuf()->sgetc();
						;
						_Meta=_Istr.rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
					{
						_State|=ios_base::eofbit;
						break;
					}
						else if(!_Ctype_fac.is(ctype<_Elem>::space, _Traits::to_char_type(_Meta)))break;
				}
				catch(...)
				{
					(_Istr).setstate(ios_base::badbit, true);
				}
			}
			_Istr.setstate(_State);
		}
		return (_Istr);
	}
	inline basic_istream<wchar_t, char_traits<wchar_t> >&ws(basic_istream<wchar_t, char_traits<wchar_t> >&_Istr)
	{
		typedef wchar_t _Elem;
		typedef char_traits<_Elem>_Traits;
		if(!_Istr.eof())
		{
			ios_base::iostate _State=ios_base::goodbit;
			const basic_istream<_Elem, _Traits>::sentry _Ok(_Istr, true);
			if(_Ok)
			{
				const ctype<_Elem>&_Ctype_fac=use_facet<ctype<_Elem> >(_Istr.getloc());
				try
				{
					for(_Traits::int_type _Meta=_Istr.rdbuf()->sgetc();
						;
						_Meta=_Istr.rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
					{
						_State|=ios_base::eofbit;
						break;
					}
						else if(!_Ctype_fac.is(ctype<_Elem>::space, _Traits::to_char_type(_Meta)))break;
				}
				catch(...)
				{
					(_Istr).setstate(ios_base::badbit, true);
				}
			}
			_Istr.setstate(_State);
		}
		return (_Istr);
	}
}
namespace std
{
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_string<_Elem, _Traits, _Alloc>operator+(const basic_string<_Elem, _Traits, _Alloc>&_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (basic_string<_Elem, _Traits, _Alloc>(_Left)+=_Right);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_string<_Elem, _Traits, _Alloc>operator+(const _Elem*_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (basic_string<_Elem, _Traits, _Alloc>(_Left)+=_Right);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_string<_Elem, _Traits, _Alloc>operator+(const _Elem _Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (basic_string<_Elem, _Traits, _Alloc>(1, _Left)+=_Right);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_string<_Elem, _Traits, _Alloc>operator+(const basic_string<_Elem, _Traits, _Alloc>&_Left, const _Elem*_Right)
	{
		return (basic_string<_Elem, _Traits, _Alloc>(_Left)+=_Right);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_string<_Elem, _Traits, _Alloc>operator+(const basic_string<_Elem, _Traits, _Alloc>&_Left, const _Elem _Right)
	{
		return (basic_string<_Elem, _Traits, _Alloc>(_Left)+=_Right);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator==(const basic_string<_Elem, _Traits, _Alloc>&_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (_Left.compare(_Right)==0);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator==(const _Elem*_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (_Right.compare(_Left)==0);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator==(const basic_string<_Elem, _Traits, _Alloc>&_Left, const _Elem*_Right)
	{
		return (_Left.compare(_Right)==0);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator!=(const basic_string<_Elem, _Traits, _Alloc>&_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (!(_Left==_Right));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator!=(const _Elem*_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (!(_Left==_Right));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator!=(const basic_string<_Elem, _Traits, _Alloc>&_Left, const _Elem*_Right)
	{
		return (!(_Left==_Right));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator<(const basic_string<_Elem, _Traits, _Alloc>&_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (_Left.compare(_Right)<0);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator<(const _Elem*_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (_Right.compare(_Left)>0);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator<(const basic_string<_Elem, _Traits, _Alloc>&_Left, const _Elem*_Right)
	{
		return (_Left.compare(_Right)<0);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator>(const basic_string<_Elem, _Traits, _Alloc>&_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (_Right<_Left);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator>(const _Elem*_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (_Right<_Left);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator>(const basic_string<_Elem, _Traits, _Alloc>&_Left, const _Elem*_Right)
	{
		return (_Right<_Left);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator<=(const basic_string<_Elem, _Traits, _Alloc>&_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (!(_Right<_Left));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator<=(const _Elem*_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (!(_Right<_Left));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator<=(const basic_string<_Elem, _Traits, _Alloc>&_Left, const _Elem*_Right)
	{
		return (!(_Right<_Left));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator>=(const basic_string<_Elem, _Traits, _Alloc>&_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (!(_Left<_Right));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator>=(const _Elem*_Left, const basic_string<_Elem, _Traits, _Alloc>&_Right)
	{
		return (!(_Left<_Right));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline bool operator>=(const basic_string<_Elem, _Traits, _Alloc>&_Left, const _Elem*_Right)
	{
		return (!(_Left<_Right));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>&operator>>(basic_istream<_Elem, _Traits>&_Istr, basic_string<_Elem, _Traits, _Alloc>&_Str)
	{
		typedef ctype<_Elem>_Ctype;
		typedef basic_istream<_Elem, _Traits>_Myis;
		typedef basic_string<_Elem, _Traits, _Alloc>_Mystr;
		typedef typename _Mystr::size_type _Mysizt;
		ios_base::iostate _State=ios_base::goodbit;
		bool _Changed=false;
		const typename _Myis::sentry _Ok(_Istr);
		if(_Ok)
		{
			const _Ctype&_Ctype_fac=use_facet<_Ctype>(_Istr.getloc());
			_Str.erase();
			try
			{
				_Mysizt _Size=0<_Istr.width()&&(_Mysizt)_Istr.width()<_Str.max_size()?(_Mysizt)_Istr.width(): _Str.max_size();
				typename _Traits::int_type _Meta=_Istr.rdbuf()->sgetc();
				for(;
					0<_Size;
					--_Size, _Meta=_Istr.rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
				{
					_State|=ios_base::eofbit;
					break;
				}
					else if(_Ctype_fac.is(_Ctype::space, _Traits::to_char_type(_Meta)))break;
					else
					{
						_Str.append(1, _Traits::to_char_type(_Meta));
						_Changed=true;
					}
			}
			catch(...)
			{
				(_Istr).setstate(ios_base::badbit, true);
			}
		}
		_Istr.width(0);
		if(!_Changed)_State|=ios_base::failbit;
		_Istr.setstate(_State);
		return (_Istr);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>&getline(basic_istream<_Elem, _Traits>&_Istr, basic_string<_Elem, _Traits, _Alloc>&_Str, const _Elem _Delim)
	{
		typedef basic_istream<_Elem, _Traits>_Myis;
		ios_base::iostate _State=ios_base::goodbit;
		bool _Changed=false;
		const typename _Myis::sentry _Ok(_Istr, true);
		if(_Ok)
		{
			try
			{
				_Str.erase();
				const typename _Traits::int_type _Metadelim=_Traits::to_int_type(_Delim);
				typename _Traits::int_type _Meta=_Istr.rdbuf()->sgetc();
				for(;
					;
					_Meta=_Istr.rdbuf()->snextc())if(_Traits::eq_int_type(_Traits::eof(), _Meta))
				{
					_State|=ios_base::eofbit;
					break;
				}
					else if(_Traits::eq_int_type(_Meta, _Metadelim))
					{
						_Changed=true;
						_Istr.rdbuf()->sbumpc();
						break;
				}
					else if(_Str.max_size()<=_Str.size())
					{
						_State|=ios_base::failbit;
						break;
					}
					else
					{
						_Str+=_Traits::to_char_type(_Meta);
						_Changed=true;
					}
			}
			catch(...)
			{
				(_Istr).setstate(ios_base::badbit, true);
			}
		}
		if(!_Changed)_State|=ios_base::failbit;
		_Istr.setstate(_State);
		return (_Istr);
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_istream<_Elem, _Traits>&getline(basic_istream<_Elem, _Traits>&_Istr, basic_string<_Elem, _Traits, _Alloc>&_Str)
	{
		return (getline(_Istr, _Str, _Istr.widen('\n')));
	}
	template<class _Elem, class _Traits, class _Alloc>
	inline basic_ostream<_Elem, _Traits>&operator<<(basic_ostream<_Elem, _Traits>&_Ostr, const basic_string<_Elem, _Traits, _Alloc>&_Str)
	{
		typedef basic_ostream<_Elem, _Traits>_Myos;
		typedef basic_string<_Elem, _Traits, _Alloc>_Mystr;
		typedef typename _Mystr::size_type _Mysizt;
		ios_base::iostate _State=ios_base::goodbit;
		_Mysizt _Size=_Str.size();
		_Mysizt _Pad=_Ostr.width()<=0||(_Mysizt)_Ostr.width()<=_Size?0: (_Mysizt)_Ostr.width()-_Size;
		const typename _Myos::sentry _Ok(_Ostr);
		if(!_Ok)_State|=ios_base::badbit;
		else
		{
			try
			{
				if((_Ostr.flags()&ios_base::adjustfield)!=ios_base::left)for(;
				0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))
				{
					_State|=ios_base::badbit;
					break;
				}
				if(_State==ios_base::goodbit)for(_Mysizt _Count=0;
				_Count<_Size;
				++_Count)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Str[_Count])))
				{
					_State|=ios_base::badbit;
					break;
				}
				if(_State==ios_base::goodbit)for(;
				0<_Pad;
				--_Pad)if(_Traits::eq_int_type(_Traits::eof(), _Ostr.rdbuf()->sputc(_Ostr.fill())))
				{
					_State|=ios_base::badbit;
					break;
				}
				_Ostr.width(0);
			}
			catch(...)
			{
				(_Ostr).setstate(ios_base::badbit, true);
			}
		}
		_Ostr.setstate(_State);
		return (_Ostr);
	}
}
namespace std
{
	template<class _Elem, class _Traits, class _Alloc>
	class basic_stringbuf: public basic_streambuf<_Elem, _Traits>
	{
	public:
		typedef _Alloc allocator_type;
		typedef basic_streambuf<_Elem, _Traits>_Mysb;
		typedef basic_string<_Elem, _Traits, _Alloc>_Mystr;
		explicit basic_stringbuf(ios_base::openmode _Mode=ios_base::in|ios_base::out)
		{
			_Init(0, 0, _Getstate(_Mode));
		}
		explicit basic_stringbuf(const _Mystr&_Str, ios_base::openmode _Mode=ios_base::in|ios_base::out)
		{
			_Init(_Str.c_str(), _Str.size(), _Getstate(_Mode));
		}
		virtual~basic_stringbuf()
		{
			_Tidy();
		}
		enum
		{
			_Allocated=1, _Constant=2, _Noread=4, _Append=8, _Atend=16
		};
		typedef int _Strstate;
		typedef typename _Traits::int_type int_type;
		typedef typename _Traits::pos_type pos_type;
		typedef typename _Traits::off_type off_type;
		_Mystr str()const
		{
			if(!(_Mystate&_Constant)&&_Mysb::pptr()!=0)
			{
				_Mystr _Str(_Mysb::pbase(), (_Seekhigh<_Mysb::pptr()?_Mysb::pptr(): _Seekhigh)-_Mysb::pbase());
				return (_Str);
			}
			else if(!(_Mystate&_Noread)&&_Mysb::gptr()!=0)
			{
				_Mystr _Str(_Mysb::eback(), _Mysb::egptr()-_Mysb::eback());
				return (_Str);
			}
			else
			{
				_Mystr _Nul;
				return (_Nul);
			}
		}
		void str(const _Mystr&_Newstr)
		{
			_Tidy();
			_Init(_Newstr.c_str(), _Newstr.size(), _Mystate);
		}
	protected:
		virtual int_type overflow(int_type _Meta=_Traits::eof())
			   {
				   if(_Mystate&_Append&&_Mysb::pptr()!=0&&_Mysb::pptr()<_Seekhigh)_Mysb::setp(_Mysb::pbase(), _Seekhigh, _Mysb::epptr());
				   if(_Traits::eq_int_type(_Traits::eof(), _Meta))return (_Traits::not_eof(_Meta));
				   else if(_Mysb::pptr()!=0&&_Mysb::pptr()<_Mysb::epptr())
				   {
					   *_Mysb::_Pninc()=_Traits::to_char_type(_Meta);
					   return (_Meta);
				   }
				   else if(_Mystate&_Constant)return (_Traits::eof());
				   else
				   {
					   size_t _Oldsize=_Mysb::pptr()==0?0: _Mysb::epptr()-_Mysb::eback();
					   size_t _Newsize=_Oldsize;
					   size_t _Inc=_Newsize/2<_MINSIZE?_MINSIZE: _Newsize/2;
					   _Elem*_Ptr=0;
					   while(0<_Inc&&2147483647-_Inc<_Newsize)_Inc/=2;
					   if(0<_Inc)
					   {
						   _Newsize+=_Inc;
						   _Ptr=_Al.allocate(_Newsize);
					   }
					   if(0<_Oldsize)_Traits_helper::copy_s<_Traits>(_Ptr, _Newsize, _Mysb::eback(), _Oldsize);
					   if(_Mystate&_Allocated)_Al.deallocate(_Mysb::eback(), _Oldsize);
					   _Mystate|=_Allocated;
					   if(_Oldsize==0)
					   {
						   _Seekhigh=_Ptr;
						   _Mysb::setp(_Ptr, _Ptr+_Newsize);
						   if(_Mystate&_Noread)_Mysb::setg(_Ptr, 0, _Ptr);
						   else _Mysb::setg(_Ptr, _Ptr, _Ptr+1);
					   }
					   else
					   {
						   _Seekhigh=_Seekhigh-_Mysb::eback()+_Ptr;
						   _Mysb::setp(_Mysb::pbase()-_Mysb::eback()+_Ptr, _Mysb::pptr()-_Mysb::eback()+_Ptr, _Ptr+_Newsize);
						   if(_Mystate&_Noread)_Mysb::setg(_Ptr, 0, _Ptr);
						   else _Mysb::setg(_Ptr, _Mysb::gptr()-_Mysb::eback()+_Ptr, _Mysb::pptr()+1);
					   }
					   *_Mysb::_Pninc()=_Traits::to_char_type(_Meta);
					   return (_Meta);
				   }
			   }
			   virtual int_type pbackfail(int_type _Meta=_Traits::eof())
			   {
				   if(_Mysb::gptr()==0||_Mysb::gptr()<=_Mysb::eback()||!_Traits::eq_int_type(_Traits::eof(), _Meta)&&!_Traits::eq(_Traits::to_char_type(_Meta), _Mysb::gptr()[-1])&&_Mystate&_Constant)return (_Traits::eof());
				   else
				   {
					   _Mysb::gbump(-1);
					   if(!_Traits::eq_int_type(_Traits::eof(), _Meta))*_Mysb::gptr()=_Traits::to_char_type(_Meta);
					   return (_Traits::not_eof(_Meta));
				   }
			   }
			   virtual int_type underflow()
			   {
				   if(_Mysb::gptr()==0)return (_Traits::eof());
				   else if(_Mysb::gptr()<_Mysb::egptr())return (_Traits::to_int_type(*_Mysb::gptr()));
				   else if(_Mystate&_Noread||_Mysb::pptr()==0||_Mysb::pptr()<=_Mysb::gptr()&&_Seekhigh<=_Mysb::gptr())return (_Traits::eof());
				   else
				   {
					   if(_Seekhigh<_Mysb::pptr())_Seekhigh=_Mysb::pptr();
					   _Mysb::setg(_Mysb::eback(), _Mysb::gptr(), _Seekhigh);
					   return (_Traits::to_int_type(*_Mysb::gptr()));
				   }
			   }
			   virtual pos_type seekoff(off_type _Off, ios_base::seekdir _Way, ios_base::openmode _Which=ios_base::in|ios_base::out)
			   {
				   if(_Mysb::pptr()!=0&&_Seekhigh<_Mysb::pptr())_Seekhigh=_Mysb::pptr();
				   if(_Which&ios_base::in&&_Mysb::gptr()!=0)
				   {
					   if(_Way==ios_base::end)_Off+=(off_type)(_Seekhigh-_Mysb::eback());
					   else if(_Way==ios_base::cur&&(_Which&ios_base::out)==0)_Off+=(off_type)(_Mysb::gptr()-_Mysb::eback());
					   else if(_Way!=ios_base::beg)_Off=_BADOFF;
					   if(0<=_Off&&_Off<=_Seekhigh-_Mysb::eback())
					   {
						   _Mysb::gbump((int)(_Mysb::eback()-_Mysb::gptr()+_Off));
						   if(_Which&ios_base::out&&_Mysb::pptr()!=0)_Mysb::setp(_Mysb::pbase(), _Mysb::gptr(), _Mysb::epptr());
					   }
					   else _Off=_BADOFF;
				   }
				   else if(_Which&ios_base::out&&_Mysb::pptr()!=0)
				   {
					   if(_Way==ios_base::end)_Off+=(off_type)(_Seekhigh-_Mysb::eback());
					   else if(_Way==ios_base::cur)_Off+=(off_type)(_Mysb::pptr()-_Mysb::eback());
					   else if(_Way!=ios_base::beg)_Off=_BADOFF;
					   if(0<=_Off&&_Off<=_Seekhigh-_Mysb::eback())_Mysb::pbump((int)(_Mysb::eback()-_Mysb::pptr()+_Off));
					   else _Off=_BADOFF;
				   }
				   else _Off=_BADOFF;
				   return (pos_type(_Off));
			   }
			   virtual pos_type seekpos(pos_type _Ptr, ios_base::openmode _Mode=ios_base::in|ios_base::out)
			   {
				   streamoff _Off=(streamoff)_Ptr;
				   if(_Mysb::pptr()!=0&&_Seekhigh<_Mysb::pptr())_Seekhigh=_Mysb::pptr();
				   if(_Off==_BADOFF);
				   else if(_Mode&ios_base::in&&_Mysb::gptr()!=0)
				   {
					   if(0<=_Off&&_Off<=_Seekhigh-_Mysb::eback())
					   {
						   _Mysb::gbump((int)(_Mysb::eback()-_Mysb::gptr()+_Off));
						   if(_Mode&ios_base::out&&_Mysb::pptr()!=0)_Mysb::setp(_Mysb::pbase(), _Mysb::gptr(), _Mysb::epptr());
					   }
					   else _Off=_BADOFF;
				   }
				   else if(_Mode&ios_base::out&&_Mysb::pptr()!=0)
				   {
					   if(0<=_Off&&_Off<=_Seekhigh-_Mysb::eback())_Mysb::pbump((int)(_Mysb::eback()-_Mysb::pptr()+_Off));
					   else _Off=_BADOFF;
				   }
				   else _Off=_BADOFF;
				   return (streampos(_Off));
			   }
			   void _Init(const _Elem*_Ptr, size_t _Count, _Strstate _State)
			   {
				   _Seekhigh=0;
				   _Mystate=_State;
				   if(_Count!=0&&(_Mystate&(_Noread|_Constant))!=(_Noread|_Constant))
				   {
					   _Elem*_Pnew=_Al.allocate(_Count);
					   _Traits_helper::copy_s<_Traits>(_Pnew, _Count, _Ptr, _Count);
					   _Seekhigh=_Pnew+_Count;
					   if(!(_Mystate&_Noread))_Mysb::setg(_Pnew, _Pnew, _Pnew+_Count);
					   if(!(_Mystate&_Constant))
					   {
						   _Mysb::setp(_Pnew, (_Mystate&_Atend)?_Pnew+_Count: _Pnew, _Pnew+_Count);
						   if(_Mysb::gptr()==0)_Mysb::setg(_Pnew, 0, _Pnew);
					   }
					   _Mystate|=_Allocated;
				   }
			   }
			   void _Tidy()
			   {
				   if(_Mystate&_Allocated)_Al.deallocate(_Mysb::eback(), (_Mysb::pptr()!=0?_Mysb::epptr(): _Mysb::egptr())-_Mysb::eback());
				   _Mysb::setg(0, 0, 0);
				   _Mysb::setp(0, 0);
				   _Seekhigh=0;
				   _Mystate&=~_Allocated;
			   }
	private:
		enum
		{
			_MINSIZE=32
		};
		_Strstate _Getstate(ios_base::openmode _Mode)
		{
			_Strstate _State=(_Strstate)0;
			if(!(_Mode&ios_base::in))_State|=_Noread;
			if(!(_Mode&ios_base::out))_State|=_Constant;
			if(_Mode&ios_base::app)_State|=_Append;
			if(_Mode&ios_base::ate)_State|=_Atend;
			return (_State);
		}
		_Elem*_Seekhigh;
		_Strstate _Mystate;
		allocator_type _Al;
	};
}
