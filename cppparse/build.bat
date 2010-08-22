
@echo %INCLUDE%
call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat"
msbuild %~dp0cppparse.sln /t:test:rebuild /property:Configuration=Debug /p:"VCBuildAdditionalOptions= /useenv"