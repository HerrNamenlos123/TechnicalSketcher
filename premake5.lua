
-- Retrieve the project name
newoption { trigger = "projectname", description = "Name of the generated project" }
local projectName = _OPTIONS["projectname"]
if projectName == nil then print("The project name was not specified! --projectname=YourApplication") end

-- Check if an environment variable exists, otherwise abort the program
function CheckEnvVar (variable, productName)
    if (os.getenv(variable) == nil) then
        print("Environment variable " .. variable .. " not found! Make sure the " .. productName .. " is installed correctly!")
        projectName = nil
    end
end

-- Here check if the Festo Robotino API 2 is installed, otherwise abort
CheckEnvVar("BATTERY_ENGINE_INCLUDE_DIRECTORY", "BatteryEngine")
CheckEnvVar("BATTERY_ENGINE_DEBUG_LINK_FILES", "BatteryEngine")
CheckEnvVar("BATTERY_ENGINE_RELEASE_LINK_FILES", "BatteryEngine")
CheckEnvVar("BATTERY_ENGINE_DEPLOY_LINK_FILES", "BatteryEngine")
CheckEnvVar("BATTERY_ENGINE_DEBUG_LINK_DIRS", "BatteryEngine")
CheckEnvVar("BATTERY_ENGINE_RELEASE_LINK_DIRS", "BatteryEngine")
CheckEnvVar("BATTERY_ENGINE_DEPLOY_LINK_DIRS", "BatteryEngine")

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
        libdirs { "$(BATTERY_ENGINE_DEBUG_LINK_DIRS)" }
        links { "$(BATTERY_ENGINE_DEBUG_LINK_FILES)" }
        targetdir (_SCRIPT_DIR .. "/bin/debug")

    filter "configurations:Release"
        defines { "NDEBUG", "NDEPLOY", "ALLEGRO_STATICLINK" }
        kind "WindowedApp"
        runtime "Release"
        symbols "Off"
        optimize "On"
        libdirs { "$(BATTERY_ENGINE_RELEASE_LINK_DIRS)" }
        links { "$(BATTERY_ENGINE_RELEASE_LINK_FILES)" }
        targetdir (_SCRIPT_DIR .. "/bin/release")

    filter "configurations:Deploy"
        defines { "NDEBUG", "DEPLOY", "ALLEGRO_STATICLINK" }
        kind "WindowedApp"
        runtime "Release"
        symbols "Off"
        optimize "On"
        libdirs { "$(BATTERY_ENGINE_DEPLOY_LINK_DIRS)" }
        links { "$(BATTERY_ENGINE_DEPLOY_LINK_FILES)" }
        targetdir (_SCRIPT_DIR .. "/bin/deploy")

    filter {}
    
    includedirs { _SCRIPT_DIR .. "/include", "$(BATTERY_ENGINE_INCLUDE_DIRECTORY)" }
    files { _SCRIPT_DIR .. "/include/**", _SCRIPT_DIR .. "/src/**" }

    linkoptions { "/IGNORE:4099" }  -- Ignore warning that no .pdb file is found for debugging

    -- Embed resource files
    files "resource/resource.rc"



    -- Build the Installer
    filter "configurations:Deploy"

        local infile = "installer/TechnicalSketcher.wxs"
        local outfile = "bin/deploy/TechnicalSketcher-Installer-x64.msi"
        local zipfile = "bin\\deploy\\AutomaticUpdaterRelease.zip"

        postbuildcommands { 
            "echo Building Installer...",   -- Build the installer
            "del /F ../" .. outfile .. " 2>nul",
            "\"%WIX%bin\\candle.exe\" ../" .. infile .. " -o obj/",
            "\"%WIX%bin\\light.exe\" obj/*.wixobj -o ../" .. outfile .. " -sice:ICE91 -spdb",
            "echo TechnicalSketcher.vcxproj -^> " .. _SCRIPT_DIR .. "/" .. outfile,
            "echo Installer has been built successfully",
            "echo .",
            "echo Bundling package for automatic updates...",       -- Copy all files to be compressed
            "del /F ..\\" .. zipfile .. " 2>nul",
            "rmdir /S /Q ..\\installer\\compress 2>nul",
            "xcopy /y ..\\version ..\\installer\\compress\\",
            "xcopy /y ..\\bin\\deploy\\TechnicalSketcher.exe ..\\installer\\compress\\",
            "powershell.exe Compress-Archive ../installer/compress/** ../" .. zipfile,   -- Compress the archive
            "echo TechnicalSketcher.vcxproj -^> " .. _SCRIPT_DIR .. "/" .. zipfile,
            "echo Updater package was generated successfully"
        }

    filter {}
