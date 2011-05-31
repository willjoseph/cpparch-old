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
	if(argc < 2)
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
	if(argc > 2)
	{
		id << argv[2];
	}
	else
	{
		id << boost::uuids::random_generator()();
	}

	SetEnvironmentVariableA("INTERPOSE_UUID", id.str().c_str());


	char root[1024];
	char* p = root;
	p = string_write(p, argv[1]);
	p = string_write(p, id.str().c_str());
	p = string_write(p, "\\");
	*p = '\0';

	if(!file_exists(root))
	{

#define CMD L"..\\..\\cppparse\\build.bat"

		//////////////////////////////////////////////////////////////////////////
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

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
	}

	for(Dir dir(root); dir.good(); dir.next())
	{
		const char* name = dir.name();
		if(*name != '.')
		{
			*string_write(p, name) = '\0';
			std::cout << root << std::endl;
			{
				STARTUPINFO si;
				PROCESS_INFORMATION pi;

				ZeroMemory(&si, sizeof(si));
				ZeroMemory(&pi, sizeof(pi));
				si.cb = sizeof(si);

#define CPPPARSE_DIR L"..\\..\\cppparse\\"
#define CPPPARSE CPPPARSE_DIR L"cppparse.exe"

				wchar_t commandline[1024];
				wcscpy(commandline, CPPPARSE);
				wcscat(commandline, L" ");
				wchar_t* p = commandline + wcslen(commandline);
				mbstowcs(p, root, (commandline + sizeof(commandline)) - p);

				SetLastError(0);
				if (!CreateProcess(CPPPARSE, commandline,
					NULL, NULL, TRUE, 0, NULL, CPPPARSE_DIR,
					&si, &pi))
				{
					printf("interpose.exe: CreateProcess failed: %d\n", GetLastError());
					ExitProcess(9007);
				}
				WaitForSingleObject(pi.hProcess, INFINITE);
			}
		}
	}


    return 0;
}
//
///////////////////////////////////////////////////////////////// End of File.
