@echo off

mkdir modules\BatteryEngine 2>NUL
cd modules\BatteryEngine

git pull origin master

call update-allegro5-binaries.bat

Pause
