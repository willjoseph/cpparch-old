//////////////////////////////////////////////////////////////////////////////
//
//  Test DetourCreateProcessWithDll function (withdll.cpp).
//
//  Microsoft Research Detours Package, Version 2.1.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//
#include <stdio.h>
#include <windows.h>
#include <detours.h>
#include <boost/config.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

#include "../interposed/commandline.h"

//////////////////////////////////////////////////////////////////////// main.
//
int main(int argc, char **argv)
{
    //////////////////////////////////////////////////////////////////////////
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    PCHAR pszFileExe = NULL;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

#if 0
	ParseCommandLine(L"blah");
	ParseCommandLine(L"\"bl ah\"");
	ParseCommandLine(L"b\"l a\"h");
	ParseCommandLine(L"bl\\\"ah");
	ParseCommandLine(L"\"bl\\\"ah\"");
	ParseCommandLine(L"\"blah\\\\\"");
	ParseCommandLine(L"\"bl\\\\\\\"ah\"");
	ParseCommandLine(L"bl\\\\\\ah");
	ParseCompilerCommandLine(L"\"bl\\\\\\ah\"");
	{
		std::ofstream out("C:\\dev\\cpparch\\out\\test.txt");
		ParseCompilerCommandLine(L"ax @C:\\dev\\cpparch\\cppparse\\Debug\\RSP0000012968430520.bin ay", out);
	}
#endif

    DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;

	if(argc > 1)
	{
		SetEnvironmentVariableA("INTERPOSE_OUT", argv[1]);
	}

	std::stringstream id;
	id << boost::uuids::random_generator()();
	SetEnvironmentVariableA("INTERPOSE_UUID", id.str().c_str());

//#define CMD L"c:\\windows\\system32\\cmd.exe"
#define CMD L"C:\\dev\\cpparch\\cppparse\\build.bat"
    SetLastError(0);
    if (!DetourCreateProcessWithDll(CMD, CMD,
                                    NULL, NULL, TRUE, dwFlags, NULL, NULL,
                                    &si, &pi, "detoured.dll", "interposed.dll", NULL)) {
        printf("withdll.exe: DetourCreateProcessWithDll failed: %d\n", GetLastError());
        ExitProcess(9007);
    }

    ResumeThread(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD dwResult = 0;
    if (!GetExitCodeProcess(pi.hProcess, &dwResult)) {
        printf("withdll.exe: GetExitCodeProcess failed: %d\n", GetLastError());
        return 9008;
    }

	char tmp[1024];
	char* p = tmp;
	p = string_write(p, argv[1]);
	p = string_write(p, id.str().c_str());
	p = string_write(p, "\\");
	*p = '\0';

	for(Dir dir(tmp); dir.good(); dir.next())
	{
		const char* name = dir.name();
		if(*name != '.')
		{
			*string_write(p, name) = '\0';
			std::cout << tmp << std::endl;
		}
	}


    return dwResult;
}
//
///////////////////////////////////////////////////////////////// End of File.
