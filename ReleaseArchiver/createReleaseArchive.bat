@echo off

set archive=AutomaticUpdaterRelease.zip

if exist %archive% del %archive%

cd ../x64/Release && call "../../ReleaseArchiver/7z.exe" a -tzip ../../ReleaseArchiver/%archive% *.dll *.exe