//////////////////////////////////////////////////////////////////////////////
//
//  Test DetourCreateProcessWithDll function (interpose.cpp).
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
// NOTE: an injected dll will fail to hook CreateProcess when run under the debugger!
int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("usage: interpose <output-dir>\n");
		return 1;
	}
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


	if(argc > 1)
	{
		SetEnvironmentVariableA("INTERPOSE_OUT", argv[1]);
	}

	std::stringstream id;
	id << boost::uuids::random_generator()();
	SetEnvironmentVariableA("INTERPOSE_UUID", id.str().c_str());


#define CMD L"..\\..\\cppparse\\build.bat"

	//////////////////////////////////////////////////////////////////////////
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	PCHAR pszFileExe = NULL;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	DWORD dwFlags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED;
	SetLastError(0);
    if (!DetourCreateProcessWithDll(CMD, CMD,
                                    NULL, NULL, TRUE, dwFlags, NULL, NULL,
									&si, &pi, "detoured.dll", "interposed.dll", NULL))
	{
        printf("interpose.exe: DetourCreateProcessWithDll failed: %d\n", GetLastError());
        ExitProcess(9007);
    }


    ResumeThread(pi.hThread);

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD dwResult = 0;
    if (!GetExitCodeProcess(pi.hProcess, &dwResult)) {
        printf("interpose.exe: GetExitCodeProcess failed: %d\n", GetLastError());
        return 9008;
    }

	if(dwResult != 0)
	{
		printf("interpose.exe: The process completed with non-zero exit code: 0x%08x\n", dwResult);
		return 1;
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
