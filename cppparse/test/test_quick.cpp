// iostream standard header for Microsoft
extern"C"
{
}
extern"C"
{
	typedef unsigned int uintptr_t;
	typedef char*va_list;
}
extern"C"
{
	typedef unsigned int size_t;
	typedef size_t rsize_t;
	typedef int intptr_t;
	typedef int ptrdiff_t;
	typedef unsigned short wchar_t;
	typedef unsigned short wint_t;
	typedef unsigned short wctype_t;
	typedef int errcode;
	typedef int errno_t;
	typedef long __time32_t;
	typedef __time32_t time_t;
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

namespace GNamespace
{
	typedef int NType;
	NType nVar;
}

typedef GNamespace::NType GType;

GType gVar = GNamespace::nVar;

GType gFunc(GType param)
{
	typedef GType FType;
	FType lVar;
	if(FType lVar = gVar)
	{
	}
}

class GClass
{
	typedef GType CType;
	GClass(CType param) : mVar(param)
	{
	}
	CType mFunc(CType param)
	{
		typedef CType FType;
		FType fVar = param;
		if(FType cVar = mVar)
		{
			typedef FType LType;
			LType lVar;
		}

		class FClass
		{
			FClass(FType param) : mVar(param)
			{
			}
			FType mFunc(FType param)
			{
				typedef CType FType;
				FType fVar = param;
				if(FType cVar = mVar)
				{
					typedef FType LType;
					LType lVar;
				}
			}
			FType mVar;
		} fVar2;
	}
	CType mVar;

	class MClass
	{
		MClass(CType param) : mVar(param)
		{
		}
		CType mFunc(CType param)
		{
			typedef CType FType;
			FType fVar = param;
			if(FType cVar = mVar)
			{
				typedef FType LType;
				LType lVar;
			}
		}
		CType mVar;
	} mVar2;
} gVar2;
