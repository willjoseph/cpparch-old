
@echo %INCLUDE%
call "%ProgramFiles%\Microsoft Visual Studio 8\VC\vcvarsall.bat"
cd %~dp0..\..\detours-2.1\bin
withdll /d:%~dp0..\bin\debug\interposed.dll %~dp0build.bat
