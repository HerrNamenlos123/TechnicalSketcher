@echo off

:: Copy the zip file to deploy folder
cmd /c copy ..\ZipRelease\AutomaticUpdaterRelease.zip ..\Deploy\AutomaticUpdaterRelease.zip >NUL

:: Get current version
set /p folder=<..\version

:: Now everything is deployed, backup now
cmd /c rd /S /Q ..\OldVersions\%folder% >NUL
cmd /c mkdir ..\OldVersions\%folder% >NUL
cmd /c copy ..\Deploy\AutomaticUpdaterRelease.zip ..\OldVersions\%folder%\AutomaticUpdaterRelease.zip >NUL
cmd /c copy ..\Deploy\TechnicalSketcher-Installer-x64.msi ..\OldVersions\%folder%\TechnicalSketcher-Installer-x64.msi >NUL