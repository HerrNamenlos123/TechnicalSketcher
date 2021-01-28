
import zipfile
import os
import sys
import shutil

zipFile = "../Deploy/AutomaticUpdaterRelease.zip"
packageFiles =  [["../version", "version"], 
                ["../DeployInstallers/dist/updater.exe", "updater.exe"],
                ["../DeployInstallers/dist/uninstall.exe", "uninstall.exe"],
                ["../bin/Release/TechnicalSketcher.exe", "latest/TechnicalSketcher.exe"],
                ["../icon.ico", "latest/icon.ico"]
]

def deleteDeployed():
    # Make folder empty
    for i in range(10):
        if os.path.exists("../Deploy"):
            try:
                shutil.rmtree("../Deploy")
            except:
                pass
    
    if os.path.exists("../Deploy"):
        shutil.rmtree("../Deploy")
        
    os.mkdir("../Deploy")

def readFile(filename):
    try:
        file = open(filename)
        content = file.read()
        file.close()
        return content
    except:
        print("Failed to read file: '" + filename + "'")
        sys.exit(-1)

def packageZipFile():
    zipObj = zipfile.ZipFile(zipFile, 'w', compression=zipfile.ZIP_DEFLATED)
    
    for file in packageFiles:
        zipObj.write(file[0], file[1])

    zipObj.close()


def addDeployToHistory(version):
    # Check if folder already exists
    if (os.path.exists("../History/" + version)):
        print("The currently packaged version is already in the history, overwriting...")
        shutil.rmtree("../History/" + version)

    # Now copy latest files
    shutil.copytree("../Deploy", "../History/" + version)







# Get deployed version
version = readFile("../version")
if (version[0] != "v"):
    print("Failed to read version, the version must start with 'v'")
    sys.exit(-1)
print("Packaging for version '" + version + "'")

# First delete old deploy files
deleteDeployed()

# Next copy the final installer msi to deploy
shutil.copy("../Installer-x64/Release/TechnicalSketcher-Installer-x64.msi", "../Deploy/TechnicalSketcher-Installer-x64.msi")

# Collect all necessary files and package into zip file
packageZipFile()

# Finally, save all deployed files to the history
addDeployToHistory(version)

print("Successfully exported all installation files")

print("===================================================================================================================================================================")
print("    ____  _     __                             __                              __  __            ____                 __           __     ______          __   ___ ")
print("   / __ \(_)___/ /  __  ______  __  __   _____/ /_  ____ _____  ____ ____     / /_/ /_  ___     / __ \_________  ____/ /_  _______/ /_   / ____/___  ____/ /__/__ \\")
print("  / / / / / __  /  / / / / __ \/ / / /  / ___/ __ \/ __ `/ __ \/ __ `/ _ \   / __/ __ \/ _ \   / /_/ / ___/ __ \/ __  / / / / ___/ __/  / /   / __ \/ __  / _ \/ _/")
print(" / /_/ / / /_/ /  / /_/ / /_/ / /_/ /  / /__/ / / / /_/ / / / / /_/ /  __/  / /_/ / / /  __/  / ____/ /  / /_/ / /_/ / /_/ / /__/ /_   / /___/ /_/ / /_/ /  __/_/  ")
print("/_____/_/\__,_/   \__, /\____/\__,_/   \___/_/ /_/\__,_/_/ /_/\__, /\___/   \__/_/ /_/\___/  /_/   /_/   \____/\__,_/\__,_/\___/\__/   \____/\____/\__,_/\___(_)   ")
print("                 /____/                                      /____/                                                                                                ")
print("===================================================================================================================================================================")
