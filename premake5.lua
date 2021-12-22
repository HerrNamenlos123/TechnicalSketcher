
-- Retrieve the project name from 'generate-win.bat'
newoption { trigger = "projectname", description = "Name of the generated project" }
local projectName = _OPTIONS["projectname"]
if projectName == nil then print("The project name was not specified! --projectname=YourApplication") os.exit(1) end
if projectName == "BatteryEngine" then print("The project cannot be named 'BatteryEngine'!") os.exit(1) end




-- Main Solution
workspace (projectName)
    configurations { "Debug", "Release", "Deploy" }

    platforms { "x64" }
    defaultplatform "x64"
    startproject (projectName)


-- The BatteryEngine Subproject
include "modules/BatteryEngine"


-- Actual application project
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




    -- Configuration filters are active up to the next filter statement
    -- Indentation is purely visual
    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG", "NDEPLOY", "ALLEGRO_STATICLINK" }
        kind "ConsoleApp"
        runtime "Debug"
        symbols "On"
        optimize "Off"
        targetdir (_SCRIPT_DIR .. "/bin/debug")
        linkoptions { '/NODEFAULTLIB:libcmt.lib', '/NODEFAULTLIB:msvcrt.lib', '/NODEFAULTLIB:msvcrtd.lib' }

    filter "configurations:Release"
        defines { "NDEBUG", "NDEPLOY", "ALLEGRO_STATICLINK" }
        kind "WindowedApp"
        runtime "Release"
        symbols "Off"
        optimize "On"
        targetdir (_SCRIPT_DIR .. "/bin/release")
        linkoptions { '/NODEFAULTLIB:libcmtd.lib', '/NODEFAULTLIB:msvcrt.lib', '/NODEFAULTLIB:msvcrtd.lib' }

    filter "configurations:Deploy"
        defines { "NDEBUG", "DEPLOY", "ALLEGRO_STATICLINK" }
        kind "WindowedApp"
        runtime "Release"
        symbols "Off"
        optimize "On"
        targetdir (_SCRIPT_DIR .. "/bin/deploy")
        debugargs { "noupdate" }    -- Prevent automatic update when launching deploy configuration from VS
        linkoptions { '/NODEFAULTLIB:libcmtd.lib', '/NODEFAULTLIB:msvcrt.lib', '/NODEFAULTLIB:msvcrtd.lib' }

    filter {}


    -- Include directories for the compiler
    includedirs { "include" }
    

    -- Source files (all files in the project view)
    files "include/**"
    files "src/**"
    files "installer/**"
    files "version.txt"
    files "resource/resource.rc" -- Embed resource files

    --linkoptions { "/IGNORE:4099" }  -- Ignore warning that no .pdb file is found for debugging
    
 

    -- Load the BatteryEngine dependency
    dependson("BatteryEngine");
    includedirs { BATTERY_INCLUDE_DIRS }
    libdirs { BATTERY_LINK_DIRS }
    links { BATTERY_LINKS }



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
