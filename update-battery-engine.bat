@echo off

cd "%~dp0"
echo Pulling from github.com ...
git submodule update --remote --merge
echo Done

Pause