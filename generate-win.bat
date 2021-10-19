@echo off

:: Available generators:
:: clean             Remove all binaries and generated files
:: codelite          Generate CodeLite project files
:: gmake             Generate GNU makefiles for POSIX, MinGW, and Cygwin
:: gmake2            Generate GNU makefiles for POSIX, MinGW, and Cygwin
:: vs2005            Generate Visual Studio 2005 project files
:: vs2008            Generate Visual Studio 2008 project files
:: vs2010            Generate Visual Studio 2010 project files
:: vs2012            Generate Visual Studio 2012 project files
:: vs2013            Generate Visual Studio 2013 project files
:: vs2015            Generate Visual Studio 2015 project files
:: vs2017            Generate Visual Studio 2017 project files
:: vs2019            Generate Visual Studio 2019 project files
:: xcode4            Generate Apple Xcode 4 project files

:: Replace the generator here
set _generator=vs2019
set _projectname=TechnicalSketcher

:: ===================================================================
echo Generating project '%_projectname%'

%~dp0premake5\windows\premake5.exe %_generator% --file=%~dp0premake5.lua --projectname=%_projectname% && start %_projectname%.sln
Timeout 5
