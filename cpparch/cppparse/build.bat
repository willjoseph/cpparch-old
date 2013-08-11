
@echo %INCLUDE%
call "%ProgramFiles%\Microsoft Visual Studio 8\VC\vcvarsall.bat"
msbuild %~dp0cppparse.sln /t:Rebuild /p:Configuration=Debug /p:"VCBuildAdditionalOptions= /useenv"