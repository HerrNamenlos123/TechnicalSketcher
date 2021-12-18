
-- Retrieve the project name
newoption { trigger = "projectname", description = "Name of the generated project" }
local projectName = _OPTIONS["projectname"]
if projectName == nil then print("The project name was not specified! --projectname=YourApplication") end

-- Main Solution
workspace (projectName)
    configurations { "Debug", "Release", "Deploy" }

    platforms { "x64" }
    defaultplatform "x64"
    startproject (projectName)

-- Actual project
project (projectName)
    language "C++"
	cppdialect "C++17"
	staticruntime "on"
    location "build"
    targetname (projectName)

    system "Windows"
    entrypoint "mainCRTStartup"
    architecture "x86_64"

    pchheader "pch.h"
    pchsource "src/pch.cpp"

    -- Configuration filters, filters are active up to the next filter statement
    -- Indentation is purely visual
    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG", "NDEPLOY", "ALLEGRO_STATICLINK" }
        kind "ConsoleApp"
        runtime "Debug"
        symbols "On"
        optimize "Off"
        libdirs { _SCRIPT_DIR .. "/modules/BatteryEngine/bin" }
        links { "BatteryEngine" }
        targetdir (_SCRIPT_DIR .. "/bin/debug")

    filter "configurations:Release"
        defines { "NDEBUG", "NDEPLOY", "ALLEGRO_STATICLINK" }
        kind "WindowedApp"
        runtime "Release"
        symbols "Off"
        optimize "On"
        libdirs { _SCRIPT_DIR .. "/modules/BatteryEngine/bin" }
        links { "BatteryEngine" }
        targetdir (_SCRIPT_DIR .. "/bin/release")

    filter "configurations:Deploy"
        defines { "NDEBUG", "DEPLOY", "ALLEGRO_STATICLINK" }
        kind "WindowedApp"
        runtime "Release"
        symbols "Off"
        optimize "On"
        libdirs { _SCRIPT_DIR .. "/modules/BatteryEngine/bin" }
        links { "BatteryEngine" }
        targetdir (_SCRIPT_DIR .. "/bin/deploy")
        debugargs { "noupdate" }    -- Prevent automatic update when launching deploy configuration from VS

    filter {}
    
    includedirs { _SCRIPT_DIR .. "/include", _SCRIPT_DIR .. "/modules/BatteryEngine/include" }
    files { _SCRIPT_DIR .. "/include/**", _SCRIPT_DIR .. "/src/**", _SCRIPT_DIR .. "/installer/**" }
    files { _SCRIPT_DIR .. "/version.txt" }

    linkoptions { "/IGNORE:4099" }  -- Ignore warning that no .pdb file is found for debugging

    -- Embed resource files
    files "resource/resource.rc"



    -- Build the Installer
    filter "configurations:Deploy"

        local infile = "installer/TechnicalSketcher.wxs"
        local outfile = "bin/deploy/TechnicalSketcher-Installer-x64.msi"
        local zipfile = "bin\\deploy\\AutomaticUpdaterRelease.zip"

        postbuildcommands { 
            "echo Building Installer...",   
            "del /F ../" .. outfile .. " 2>nul",

            -- Build the installer
            "\"%WIX%bin\\candle.exe\" ../" .. infile .. " -o obj/",
            "if %errorlevel% neq 0 exit /b %errorlevel%",

            "\"%WIX%bin\\light.exe\" obj/*.wixobj -o ../" .. outfile .. " -sice:ICE91 -spdb",
            "if %errorlevel% neq 0 exit /b %errorlevel%",

            "echo TechnicalSketcher.vcxproj -^> " .. _SCRIPT_DIR .. "/" .. outfile,
            "echo Installer has been built successfully",
            "echo .",

            -- Copy all files to be compressed
            "echo Bundling package for automatic updates...",       
            "del /F ..\\" .. zipfile .. " 2>nul",
            "rmdir /S /Q ..\\build\\updatearchive 2>nul",
            --"xcopy /y ..\\version ..\\installer\\compress\\",
            --"if %errorlevel% neq 0 exit /b %errorlevel%",

            -- Main Application Executable
            "xcopy /y ..\\bin\\deploy\\TechnicalSketcher.exe ..\\build\\updatearchive\\latest\\",
            "if %errorlevel% neq 0 exit /b %errorlevel%",

            -- Icon
            --"xcopy /y ..\\resource\\TechnicalSketcher.ico ..\\build\\updatearchive\\latest\\",
            --"if %errorlevel% neq 0 exit /b %errorlevel%",

            -- Compress the archive
            "powershell.exe Compress-Archive ../build/updatearchive/** $(ProjectDir)..\\" .. zipfile,   
            "if %errorlevel% neq 0 exit /b %errorlevel%",
            "echo TechnicalSketcher.vcxproj -^> $(ProjectDir)..\\" .. zipfile,
            "echo Updater package was generated successfully",
            
            -- Load version tag
            "set /p version=<..\\version.txt",
            "set version=%version:\"=%",

            -- Create version file, so you always see the version next to the .msi (for deploying)
            "del ..\\bin\\deploy\\v* 2>nul",
            "type nul > ..\\bin\\deploy\\%version%",

            -- Everything done now
            "echo .",
            "echo .",
            "echo The application was successfully packaged: Version %version%"
        }

    filter {}

-- Import the BatteryEngine as a project
include "modules/BatteryEngine"
