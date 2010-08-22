//////////////////////////////////////////////////////////////////////////////
//
//  Detours Test Program (traceapi.cpp of traceapi.dll)
//
//  Microsoft Research Detours Package, Version 2.1.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
#define _WIN32_WINNT        0x0400
#if 0
#define WIN32
#endif
#define NT

#define DBG_TRACE   0

#if _MSC_VER >= 1300
#include <winsock2.h>
#endif
#include <windows.h>
#include <stdio.h>
#include "detours.h"
#include "detoured.h"
#include "syelog.h"
#include "commandline.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

#if (_MSC_VER < 1299)
#define LONG_PTR    LONG
#define ULONG_PTR   ULONG
#define PLONG_PTR   PLONG
#define PULONG_PTR  PULONG
#define INT_PTR     INT
#define UINT_PTR    UINT
#define PINT_PTR    PINT
#define PUINT_PTR   PUINT
#define DWORD_PTR   DWORD
#define PDWORD_PTR  PDWORD
#endif

//////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4127)   // Many of our asserts are constants.

#define ASSERT_ALWAYS(x)   \
    do {                                                        \
    if (!(x)) {                                                 \
            AssertMessage(#x, __FILE__, __LINE__);              \
            DebugBreak();                                       \
    }                                                           \
    } while (0)

#ifndef NDEBUG
#define ASSERT(x)           ASSERT_ALWAYS(x)
#else
#define ASSERT(x)
#endif

#define UNUSED(c)    (c) = (c)
#define ARRAYOF(x)      (sizeof(x)/sizeof(x[0]))

//////////////////////////////////////////////////////////////////////////////
static HMODULE s_hInst = NULL;
static WCHAR s_wzDllPath[MAX_PATH];

char gOutputPath[1024];
char gUuid[40];


BOOL ProcessEnumerate();
BOOL InstanceEnumerate(HINSTANCE hInst);

VOID _PrintEnter(const CHAR *psz, ...);
VOID _PrintExit(const CHAR *psz, ...);
VOID _Print(const CHAR *psz, ...);
VOID _VPrint(PCSTR msg, va_list args, PCHAR pszBuf, LONG cbBuf);

VOID AssertMessage(CONST PCHAR pszMsg, CONST PCHAR pszFile, ULONG nLine);

//////////////////////////////////////////////////////////////////////////////
//
// Trampolines
//
extern "C" {
    //  Trampolines for SYELOG library.
    //
    HANDLE (WINAPI *Real_CreateFileW)(LPCWSTR a0, DWORD a1, DWORD a2,
                                      LPSECURITY_ATTRIBUTES a3, DWORD a4, DWORD a5,
                                      HANDLE a6)
		= CreateFileW;
    BOOL (WINAPI *Real_WriteFile)(HANDLE hFile,
                                  LPCVOID lpBuffer,
                                  DWORD nNumberOfBytesToWrite,
                                  LPDWORD lpNumberOfBytesWritten,
                                  LPOVERLAPPED lpOverlapped)
		= WriteFile;
    BOOL (WINAPI *Real_FlushFileBuffers)(HANDLE hFile)
		= FlushFileBuffers;
    BOOL (WINAPI *Real_CloseHandle)(HANDLE hObject)
		= CloseHandle;
    BOOL (WINAPI *Real_WaitNamedPipeW)(LPCWSTR lpNamedPipeName, DWORD nTimeOut)
		= WaitNamedPipeW;
    BOOL (WINAPI *Real_SetNamedPipeHandleState)(HANDLE hNamedPipe,
                                                LPDWORD lpMode,
                                                LPDWORD lpMaxCollectionCount,
                                                LPDWORD lpCollectDataTimeout)
		= SetNamedPipeHandleState;
    DWORD (WINAPI *Real_GetCurrentProcessId)(VOID)
		= GetCurrentProcessId;
    VOID (WINAPI *Real_GetSystemTimeAsFileTime)(LPFILETIME lpSystemTimeAsFileTime)
		= GetSystemTimeAsFileTime;

    VOID ( WINAPI * Real_InitializeCriticalSection)(LPCRITICAL_SECTION lpSection)
        = InitializeCriticalSection;
    VOID ( WINAPI * Real_EnterCriticalSection)(LPCRITICAL_SECTION lpSection)
        = EnterCriticalSection;
    VOID ( WINAPI * Real_LeaveCriticalSection)(LPCRITICAL_SECTION lpSection)
        = LeaveCriticalSection;
}

///////////////////////////////////////////////////////////////// Trampolines.
//


BOOL (__stdcall * Real_CreateProcessA)(LPCSTR a0,
                                       LPSTR a1,
                                       LPSECURITY_ATTRIBUTES a2,
                                       LPSECURITY_ATTRIBUTES a3,
                                       BOOL a4,
                                       DWORD a5,
                                       LPVOID a6,
                                       LPCSTR a7,
                                       LPSTARTUPINFOA a8,
                                       LPPROCESS_INFORMATION a9)
    = CreateProcessA;

BOOL (__stdcall * Real_CreateProcessW)(LPCWSTR a0,
                                       LPWSTR a1,
                                       LPSECURITY_ATTRIBUTES a2,
                                       LPSECURITY_ATTRIBUTES a3,
                                       BOOL a4,
                                       DWORD a5,
                                       LPVOID a6,
                                       LPCWSTR a7,
                                       LPSTARTUPINFOW a8,
                                       LPPROCESS_INFORMATION a9)
    = CreateProcessW;

template<typename Char>
inline bool ReportCompilerArguments(const Char* compiler, const Char* path, const Char* commandline)
{
	const Char* filename = findFilename(path);
	if(string_equal(filename, compiler))
	{
#if 1
		Report("COMPILING", commandline);
#else
		std::cout << "COMPILING: " << commandline << std::endl;
#endif

		char tmp[1024];

		strcpy(tmp, gOutputPath);
		strcat(tmp, gUuid);
		dir_create(tmp);
		strcat(tmp, "\\");

		std::stringstream ss;
		ss << boost::uuids::random_generator()();
		strcat(tmp, ss.str().c_str());

		Report("OUT", tmp);

		std::ofstream out(tmp);

		out << path << std::endl;

		const Char* arguments = commandline;
		// skip the first argument
		{
			arguments = SkipWhitespace(arguments);
			char buf[1024];
			char* p = buf;
			arguments = ParseArgument(arguments, p);
		}


		char prefix[1024];
		if(GetEnvironmentVariableA("CL", prefix, ARRAY_COUNT(prefix)) != 0)
		{
			ParseCompilerCommandLine(prefix, out);
		}

		ParseCompilerCommandLine(arguments, out);

		// get the INCLUDE variable at the point of invocation of the compiler, to ensure we have the correct value
		char include[1024];
		if(GetEnvironmentVariableA("INCLUDE", include, ARRAY_COUNT(include)) != 0)
		{
			const char* value = include;
			for(const char* p = include; *p != '\0'; ++p)
			{
				if(*p == ';')
				{
					if(p != value)
					{
						out << "/I ";
						out.write(value, std::streamsize(p - value));
						out << std::endl;
					}
					value = p + 1;
				}
			}
		}

		char postfix[1024];
		if(GetEnvironmentVariableA("_CL_", postfix, ARRAY_COUNT(postfix)) != 0)
		{
			ParseCompilerCommandLine(postfix, out);
		}

		return true;
	}
	return false;
}

inline bool ReportCompilerArgumentsA(const char* path, const char* commandline)
{
	if(path != 0)
	{
		return ReportCompilerArguments("cl.exe", path, commandline == 0 ? path : commandline);
	}
	return false;
}

inline bool ReportCompilerArgumentsW(const wchar_t* path, const wchar_t* commandline)
{
	if(path != 0)
	{
		return ReportCompilerArguments(L"cl.exe", path, commandline == 0 ? path : commandline);
	}
	return false;
}

BOOL __stdcall Mine_CreateProcessA(LPCSTR a0,
                                   LPSTR a1,
                                   LPSECURITY_ATTRIBUTES a2,
                                   LPSECURITY_ATTRIBUTES a3,
                                   BOOL a4,
                                   DWORD a5,
                                   LPVOID a6,
                                   LPCSTR a7,
                                   LPSTARTUPINFOA a8,
                                   LPPROCESS_INFORMATION a9)
{
    _PrintEnter("CreateProcessA(%hs,%hs,%p,%p,%p,%p,%p,%hs,%p,%p)\n", a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);

    BOOL rv = 0;
    __try {
#if 1
		char detoured[1024];
		GetModuleFileNameA(Detoured(), detoured, 1024);

		char dll[1024];
		GetModuleFileNameA(s_hInst, dll, 1024);

#if 0
		Syelog(SYELOG_SEVERITY_INFORMATION, "detoured: %s\n", detoured);
		Syelog(SYELOG_SEVERITY_INFORMATION, "dll: %s\n", dll);
#endif

		ReportCompilerArgumentsA(a0, a1);
		
		rv = DetourCreateProcessWithDllA(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, detoured, dll, Real_CreateProcessA);
#else
        rv = Real_CreateProcessA(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
#endif
    } __finally {
        _PrintExit("CreateProcessA(,,,,,,,,,) -> %x\n", rv);
    };
    return rv;
}

BOOL __stdcall Mine_CreateProcessW(LPCWSTR a0,
                                   LPWSTR a1,
                                   LPSECURITY_ATTRIBUTES a2,
                                   LPSECURITY_ATTRIBUTES a3,
                                   BOOL a4,
                                   DWORD a5,
                                   LPVOID a6,
                                   LPCWSTR a7,
                                   LPSTARTUPINFOW a8,
                                   LPPROCESS_INFORMATION a9)
{
    _PrintEnter("CreateProcessW(%ls,%ls,%p,%p,%p,%p,%p,%ls,%p,%p)\n", a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);

    BOOL rv = 0;
    __try {
#if 1
		char detoured[1024];
		GetModuleFileNameA(Detoured(), detoured, 1024);

		char dll[1024];
		GetModuleFileNameA(s_hInst, dll, 1024);

#if 0
		Syelog(SYELOG_SEVERITY_INFORMATION, "detoured: %s\n", detoured);
		Syelog(SYELOG_SEVERITY_INFORMATION, "dll: %s\n", dll);
#endif

		ReportCompilerArgumentsW(a0, a1);

		rv = DetourCreateProcessWithDllW(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, detoured, dll, Real_CreateProcessW);
#else
        rv = Real_CreateProcessW(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
#endif
    } __finally {
        _PrintExit("CreateProcessW(,,,,,,,,,) -> %x\n", rv);
    };
    return rv;
}

////////////////////////////////////////////////////////////// AttachDetours.
//
static PCHAR DetRealName(PCHAR psz)
{
    PCHAR pszBeg = psz;
    // Move to end of name.
    while (*psz) {
        psz++;
    }
    // Move back through A-Za-z0-9 names.
    while (psz > pszBeg &&
           ((psz[-1] >= 'A' && psz[-1] <= 'Z') ||
            (psz[-1] >= 'a' && psz[-1] <= 'z') ||
            (psz[-1] >= '0' && psz[-1] <= '9'))) {
        psz--;
    }
    return psz;
}

static VOID Dump(PBYTE pbBytes, LONG nBytes, PBYTE pbTarget)
{
    CHAR szBuffer[256];
    PCHAR pszBuffer = szBuffer;

    for (LONG n = 0; n < nBytes; n += 12) {
#ifdef _CRT_INSECURE_DEPRECATE
        pszBuffer += sprintf_s(pszBuffer, sizeof(szBuffer), "  %p: ", pbBytes + n);
#else
        pszBuffer += sprintf(pszBuffer, "  %p: ", pbBytes + n);
#endif
        for (LONG m = n; m < n + 12; m++) {
            if (m >= nBytes) {
#ifdef _CRT_INSECURE_DEPRECATE
                pszBuffer += sprintf_s(pszBuffer, sizeof(szBuffer), "   ");
#else
                pszBuffer += sprintf(pszBuffer, "   ");
#endif
            }
            else {
#ifdef _CRT_INSECURE_DEPRECATE
                pszBuffer += sprintf_s(pszBuffer, sizeof(szBuffer), "%02x ", pbBytes[m]);
#else
                pszBuffer += sprintf(pszBuffer, "%02x ", pbBytes[m]);
#endif
            }
        }
        if (n == 0) {
#ifdef _CRT_INSECURE_DEPRECATE
            pszBuffer += sprintf_s(pszBuffer, sizeof(szBuffer), "[%p]", pbTarget);
#else
            pszBuffer += sprintf(pszBuffer, "[%p]", pbTarget);
#endif
        }
#ifdef _CRT_INSECURE_DEPRECATE
        pszBuffer += sprintf_s(pszBuffer, sizeof(szBuffer), "\n");
#else
        pszBuffer += sprintf(pszBuffer, "\n");
#endif
    }

    Syelog(SYELOG_SEVERITY_INFORMATION, "%s", szBuffer);
}

static VOID Decode(PBYTE pbCode, LONG nInst)
{
    PBYTE pbSrc = pbCode;
    PBYTE pbEnd;
    PBYTE pbTarget;
    for (LONG n = 0; n < nInst; n++) {
        pbTarget = NULL;
        pbEnd = (PBYTE)DetourCopyInstruction(NULL, (PVOID)pbSrc, (PVOID*)&pbTarget);
        Dump(pbSrc, (int)(pbEnd - pbSrc), pbTarget);
        pbSrc = pbEnd;

        if (pbTarget != NULL) {
            break;
        }
    }
}

VOID DetAttach(PVOID *ppvReal, PVOID pvMine, PCHAR psz)
{
    LONG l = DetourAttach(ppvReal, pvMine);
    if (l != 0) {
        Syelog(SYELOG_SEVERITY_NOTICE,
               "Attach failed: `%s': error %d\n", DetRealName(psz), l);

        Decode((PBYTE)*ppvReal, 3);
    }
}

VOID DetDetach(PVOID *ppvReal, PVOID pvMine, PCHAR psz)
{
    LONG l = DetourDetach(ppvReal, pvMine);
    if (l != 0) {
#if 0
        Syelog(SYELOG_SEVERITY_NOTICE,
               "Detach failed: `%s': error %d\n", DetRealName(psz), l);
#else
        (void)psz;
#endif
    }
}

#define ATTACH(x,y)   DetAttach(x,y,#x)
#define DETACH(x,y)   DetDetach(x,y,#x)

LONG AttachDetours(VOID)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // For this many APIs, we'll ignore one or two can't be detoured.
    DetourSetIgnoreTooSmall(TRUE);

    ATTACH(&(PVOID&)Real_CreateProcessA, Mine_CreateProcessA);
    ATTACH(&(PVOID&)Real_CreateProcessW, Mine_CreateProcessW);

    if (DetourTransactionCommit() != 0) {
        PVOID *ppbFailedPointer = NULL;
        LONG error = DetourTransactionCommitEx(&ppbFailedPointer);

        printf("traceapi.dll: Attach transaction failed to commit. Error %d (%p/%p)",
               error, ppbFailedPointer, *ppbFailedPointer);
        return error;
    }
    return 0;
}

LONG DetachDetours(VOID)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // For this many APIs, we'll ignore one or two can't be detoured.
    DetourSetIgnoreTooSmall(TRUE);

    DETACH(&(PVOID&)Real_CreateProcessA, Mine_CreateProcessA);
    DETACH(&(PVOID&)Real_CreateProcessW, Mine_CreateProcessW);

    if (DetourTransactionCommit() != 0) {
        PVOID *ppbFailedPointer = NULL;
        LONG error = DetourTransactionCommitEx(&ppbFailedPointer);

        printf("traceapi.dll: Detach transaction failed to commit. Error %d (%p/%p)",
               error, ppbFailedPointer, *ppbFailedPointer);
        return error;
    }
    return 0;
}

//
////////////////////////////////////////////////////////////// Logging System.
//
static BOOL s_bLog = FALSE;
static LONG s_nTlsIndent = -1;
static LONG s_nTlsThread = -1;
static LONG s_nThreadCnt = 0;

VOID _PrintEnter(const CHAR *psz, ...)
{
    DWORD dwErr = GetLastError();

    LONG nIndent = 0;
    LONG nThread = 0;
    if (s_nTlsIndent >= 0) {
        nIndent = (LONG)(LONG_PTR)TlsGetValue(s_nTlsIndent);
        TlsSetValue(s_nTlsIndent, (PVOID)(LONG_PTR)(nIndent + 1));
    }
    if (s_nTlsThread >= 0) {
        nThread = (LONG)(LONG_PTR)TlsGetValue(s_nTlsThread);
    }

    if (s_bLog && psz) {
        CHAR szBuf[1024];
        PCHAR pszBuf = szBuf;
        LONG nLen = (nIndent > 0) ? (nIndent < 35 ? nIndent * 2 : 70) : 0;
        *pszBuf++ = (CHAR)('0' + ((nThread / 100) % 10));
        *pszBuf++ = (CHAR)('0' + ((nThread / 10) % 10));
        *pszBuf++ = (CHAR)('0' + ((nThread / 1) % 10));
        *pszBuf++ = ' ';
        while (nLen-- > 0) {
            *pszBuf++ = ' ';
        }

        va_list  args;
        va_start(args, psz);

        while ((*pszBuf++ = *psz++) != 0) {
            // Copy characters.
        }
        SyelogV(SYELOG_SEVERITY_INFORMATION, szBuf, args);

        va_end(args);
    }
    SetLastError(dwErr);
}

VOID _PrintExit(const CHAR *psz, ...)
{
    DWORD dwErr = GetLastError();

    LONG nIndent = 0;
    LONG nThread = 0;
    if (s_nTlsIndent >= 0) {
        nIndent = (LONG)(LONG_PTR)TlsGetValue(s_nTlsIndent) - 1;
        ASSERT(nIndent >= 0);
        TlsSetValue(s_nTlsIndent, (PVOID)(LONG_PTR)nIndent);
    }
    if (s_nTlsThread >= 0) {
        nThread = (LONG)(LONG_PTR)TlsGetValue(s_nTlsThread);
    }

    if (s_bLog && psz) {
        CHAR szBuf[1024];
        PCHAR pszBuf = szBuf;
        LONG nLen = (nIndent > 0) ? (nIndent < 35 ? nIndent * 2 : 70) : 0;
        *pszBuf++ = (CHAR)('0' + ((nThread / 100) % 10));
        *pszBuf++ = (CHAR)('0' + ((nThread / 10) % 10));
        *pszBuf++ = (CHAR)('0' + ((nThread / 1) % 10));
        *pszBuf++ = ' ';
        while (nLen-- > 0) {
            *pszBuf++ = ' ';
        }

        va_list  args;
        va_start(args, psz);

        while ((*pszBuf++ = *psz++) != 0) {
            // Copy characters.
        }
        SyelogV(SYELOG_SEVERITY_INFORMATION, szBuf, args);

        va_end(args);
    }
    SetLastError(dwErr);
}

VOID _Print(const CHAR *psz, ...)
{
    DWORD dwErr = GetLastError();

    LONG nIndent = 0;
    LONG nThread = 0;
    if (s_nTlsIndent >= 0) {
        nIndent = (LONG)(LONG_PTR)TlsGetValue(s_nTlsIndent);
    }
    if (s_nTlsThread >= 0) {
        nThread = (LONG)(LONG_PTR)TlsGetValue(s_nTlsThread);
    }

    if (s_bLog && psz) {
        CHAR szBuf[1024];
        PCHAR pszBuf = szBuf;
        LONG nLen = (nIndent > 0) ? (nIndent < 35 ? nIndent * 2 : 70) : 0;
        *pszBuf++ = (CHAR)('0' + ((nThread / 100) % 10));
        *pszBuf++ = (CHAR)('0' + ((nThread / 10) % 10));
        *pszBuf++ = (CHAR)('0' + ((nThread / 1) % 10));
        *pszBuf++ = ' ';
        while (nLen-- > 0) {
            *pszBuf++ = ' ';
        }

        va_list  args;
        va_start(args, psz);

        while ((*pszBuf++ = *psz++) != 0) {
            // Copy characters.
        }
        SyelogV(SYELOG_SEVERITY_INFORMATION, szBuf, args);

        va_end(args);
    }
    SetLastError(dwErr);
}

VOID AssertMessage(CONST PCHAR pszMsg, CONST PCHAR pszFile, ULONG nLine)
{
    Syelog(SYELOG_SEVERITY_FATAL,
           "ASSERT(%s) failed in %s, line %d.\n", pszMsg, pszFile, nLine);
}

__declspec(dllexport) VOID NullExport()
{
}

//////////////////////////////////////////////////////////////////////////////
//
PIMAGE_NT_HEADERS NtHeadersForInstance(HINSTANCE hInst)
{
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hInst;
    __try {
        if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            SetLastError(ERROR_BAD_EXE_FORMAT);
            return NULL;
        }

        PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader +
                                                          pDosHeader->e_lfanew);
        if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
            SetLastError(ERROR_INVALID_EXE_SIGNATURE);
            return NULL;
        }
        if (pNtHeader->FileHeader.SizeOfOptionalHeader == 0) {
            SetLastError(ERROR_EXE_MARKED_INVALID);
            return NULL;
        }
        return pNtHeader;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
    SetLastError(ERROR_EXE_MARKED_INVALID);

    return NULL;
}

BOOL InstanceEnumerate(HINSTANCE hInst)
{
    WCHAR wzDllName[MAX_PATH];

    PIMAGE_NT_HEADERS pinh = NtHeadersForInstance(hInst);
    if (pinh && GetModuleFileNameW(hInst, wzDllName, ARRAYOF(wzDllName))) {
        Syelog(SYELOG_SEVERITY_INFORMATION, "### %p: %ls\n", hInst, wzDllName);
        return TRUE;
    }
    return FALSE;
}

BOOL ProcessEnumerate()
{
    Syelog(SYELOG_SEVERITY_INFORMATION,
           "######################################################### Binaries\n");

    PBYTE pbNext;
    for (PBYTE pbRegion = (PBYTE)0x10000;; pbRegion = pbNext) {
        MEMORY_BASIC_INFORMATION mbi;
        ZeroMemory(&mbi, sizeof(mbi));

        if (VirtualQuery((PVOID)pbRegion, &mbi, sizeof(mbi)) <= 0) {
            break;
        }
        pbNext = (PBYTE)mbi.BaseAddress + mbi.RegionSize;

        // Skip free regions, reserver regions, and guard pages.
        //
        if (mbi.State == MEM_FREE || mbi.State == MEM_RESERVE) {
            continue;
        }
        if (mbi.Protect & PAGE_GUARD || mbi.Protect & PAGE_NOCACHE) {
            continue;
        }
        if (mbi.Protect == PAGE_NOACCESS) {
            continue;
        }

        // Skip over regions from the same allocation...
        {
            MEMORY_BASIC_INFORMATION mbiStep;

            while (VirtualQuery((PVOID)pbNext, &mbiStep, sizeof(mbiStep)) > 0) {
                if ((PBYTE)mbiStep.AllocationBase != pbRegion) {
                    break;
                }
                pbNext = (PBYTE)mbiStep.BaseAddress + mbiStep.RegionSize;
                mbi.Protect |= mbiStep.Protect;
            }
        }

        WCHAR wzDllName[MAX_PATH];
        PIMAGE_NT_HEADERS pinh = NtHeadersForInstance((HINSTANCE)pbRegion);

        if (pinh &&
            GetModuleFileNameW((HINSTANCE)pbRegion,wzDllName,ARRAYOF(wzDllName))) {

            Syelog(SYELOG_SEVERITY_INFORMATION,
                   "### %p..%p: %ls\n", pbRegion, pbNext, wzDllName);
        }
        else {
            Syelog(SYELOG_SEVERITY_INFORMATION,
                   "### %p..%p: State=%04x, Protect=%08x\n",
                   pbRegion, pbNext, mbi.State, mbi.Protect);
        }
    }
    Syelog(SYELOG_SEVERITY_INFORMATION, "###\n");

    LPVOID lpvEnv = GetEnvironmentStrings();
    Syelog(SYELOG_SEVERITY_INFORMATION, "### Env= %08x [%08x %08x]\n",
           lpvEnv, ((PVOID*)lpvEnv)[0], ((PVOID*)lpvEnv)[1]);

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////
//
// DLL module information
//
BOOL ThreadAttach(HMODULE hDll)
{
    (void)hDll;

    if (s_nTlsIndent >= 0) {
        TlsSetValue(s_nTlsIndent, (PVOID)0);
    }
    if (s_nTlsThread >= 0) {
        LONG nThread = InterlockedIncrement(&s_nThreadCnt);
        TlsSetValue(s_nTlsThread, (PVOID)(LONG_PTR)nThread);
    }
    return TRUE;
}

BOOL ThreadDetach(HMODULE hDll)
{
    (void)hDll;

    if (s_nTlsIndent >= 0) {
        TlsSetValue(s_nTlsIndent, (PVOID)0);
    }
    if (s_nTlsThread >= 0) {
        TlsSetValue(s_nTlsThread, (PVOID)0);
    }
    return TRUE;
}

BOOL ProcessAttach(HMODULE hDll)
{
    s_bLog = FALSE;
    s_nTlsIndent = TlsAlloc();
    s_nTlsThread = TlsAlloc();
    ThreadAttach(hDll);

    WCHAR wzExeName[MAX_PATH];

    s_hInst = hDll;
    GetModuleFileNameW(hDll, s_wzDllPath, ARRAYOF(s_wzDllPath));
    GetModuleFileNameW(NULL, wzExeName, ARRAYOF(wzExeName));

    SyelogOpen("traceapi", SYELOG_FACILITY_APPLICATION);
#if 0
    ProcessEnumerate();
#endif

	if(GetEnvironmentVariableA("INTERPOSE_OUT", gOutputPath, sizeof(gOutputPath)) != 0)
	{
		Syelog(SYELOG_SEVERITY_INFORMATION, "OUT: %s\n", gOutputPath);
	}

	if(GetEnvironmentVariableA("INTERPOSE_UUID", gUuid, sizeof(gUuid)) != 0)
	{
		Syelog(SYELOG_SEVERITY_INFORMATION, "UUID: %s\n", gUuid);
	}

    LONG error = AttachDetours();
    if (error != NO_ERROR) {
        Syelog(SYELOG_SEVERITY_FATAL, "### Error attaching detours: %d\n", error);
    }

    s_bLog = TRUE;
    return TRUE;
}

BOOL ProcessDetach(HMODULE hDll)
{
    ThreadDetach(hDll);
    s_bLog = FALSE;

    LONG error = DetachDetours();
    if (error != NO_ERROR) {
        Syelog(SYELOG_SEVERITY_FATAL, "### Error detaching detours: %d\n", error);
    }

    Syelog(SYELOG_SEVERITY_NOTICE, "### Closing.\n");
    SyelogClose(FALSE);

    if (s_nTlsIndent >= 0) {
        TlsFree(s_nTlsIndent);
    }
    if (s_nTlsThread >= 0) {
        TlsFree(s_nTlsThread);
    }
    return TRUE;
}

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, PVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;
    BOOL ret;

    switch (dwReason) {
      case DLL_PROCESS_ATTACH:
        OutputDebugString(L"traceapi.dll: DllMain DLL_PROCESS_ATTACH\n");
#if 1
        printf("traceapi.dll: Starting.\n");
#endif
        fflush(stdout);
        Sleep(50);
        Sleep(50);
        DetourRestoreAfterWith();
        return ProcessAttach(hModule);
      case DLL_PROCESS_DETACH:
        ret = ProcessDetach(hModule);
        OutputDebugString(L"traceapi.dll: DllMain DLL_PROCESS_DETACH\n");
        return ret;
      case DLL_THREAD_ATTACH:
        OutputDebugString(L"traceapi.dll: DllMain DLL_THREAD_ATTACH\n");
        return ThreadAttach(hModule);
      case DLL_THREAD_DETACH:
        OutputDebugString(L"traceapi.dll: DllMain DLL_THREAD_DETACH\n");
        return ThreadDetach(hModule);
    }
    return TRUE;
}
//
///////////////////////////////////////////////////////////////// End of File.
