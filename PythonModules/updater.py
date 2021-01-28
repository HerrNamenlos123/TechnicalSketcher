
import requests
import json
import zipfile
import os
import shutil
import time
import sys
import zc.lockfile

automaticUpdaterRelease = "AutomaticUpdaterRelease.zip"

def getLatestGithubRelease(user, repository):
    return requests.get("https://api.github.com/repos/" + user + "/" + repository + "/releases/latest")

def download_file(url):
    local_filename = url.split('/')[-1]
    # NOTE the stream=True parameter below
    with requests.get(url, stream=True) as r:
        r.raise_for_status()
        with open(local_filename, 'wb') as f:
            for chunk in r.iter_content(chunk_size=8192): 
                # If you have chunk encoded response uncomment if
                # and set chunk_size parameter to None.
                #if chunk: 
                f.write(chunk)
    return local_filename

def downloadUpdate(j):
    print("Downloading " + automaticUpdaterRelease + "...")
    try:
        assets = j["assets"]
        for asset in assets:
            if (asset["name"] == automaticUpdaterRelease):
                url = asset["browser_download_url"]

                download_file(url)
                print("Successfully downloaded " + automaticUpdaterRelease + "\n")   
                return 1
    except:
        print("Failed to download update\n")
        return 0

def unzipUpdate():
    print("Unzipping update...")
    try:
        with zipfile.ZipFile(automaticUpdaterRelease, 'r') as zip_ref:
            zip_ref.extractall("update")
        
        print("Successfully unzipped update\n")
        return 1
                    
    except:
        print("Failed to unzip update\n")
        return 0

def updateApplication():
    print("Applying update...")
    outdatedSet = 0
    
    try:
        # Make the outdated folder empty
        if (os.path.exists("outdated")):
            print("\tDeleting outdated folder...")
            try:
                # First, delete all .exe files to prevent deleting 
                # resource files while an exe is running
                for file in os.listdir("outdated"):
                    if (file.endswith(".exe")):
                        os.remove(os.path.join("outdated", file))
                # Next, delete the entire folder
                shutil.rmtree("outdated")
                print("\tDone")
            except:
                print("\tDeletion failed")
                raise
        
        # Move currently latest file to outdated
        if (os.path.exists("latest")):
            print("\tMoving latest files to outdated...")
            try:
                os.mkdir("outdated")
                # Now move all files to outdated
                for file in os.listdir("latest"):
                    shutil.move(os.path.join("latest", file), os.path.join("outdated", file))
                print("\tDone migrating to outdated")
            except:
                print("\tFailed migrating old files")
                raise
        else:
            print("\t'latest' folder does not exist, creating...")
            try:
                os.mkdir("latest")
                print("\tDone")
            except:
                print("\tFailed to create latest directory")
                raise

        outdatedSet = 1
        # Now all files are outdated, if an error
        # occurs now, they need to be recovered

        # Now move the new files to latest
        if (os.path.exists("update/latest")):
            print("\tCopying new files to latest...")
            try:
                for file in os.listdir("update/latest"):
                    shutil.move(os.path.join("update/latest", file), os.path.join("latest", file))
                print("\tSuccessfully replaced application files")
            except:
                print("\tFailed copying files")
                raise
        
        # Main update done, update installers now
        print("\tApplication update successful, updating installers now")

        # Check if updates are there
        if (os.path.exists("update/updater.exe") and os.path.exists("update/uninstall.exe")):
            # Now apply them
            print("\tReplacing updater and uninstaller...")
            try:
                if os.path.exists("uninstall.exe"):
                    shutil.move("uninstall.exe", "uninstall_outdated.exe")
                shutil.move("update/uninstall.exe", "uninstall.exe")
                if os.path.exists("updater.exe"):
                    shutil.move("updater.exe", "updater_outdated.exe")
                shutil.move("update/updater.exe", "updater.exe")
                print("\tSuccessfully replaced installers")
            except:
                try:

                    shutil.move("updater_outdated.exe", "updater.exe")
                    shutil.move("uninstall_outdated.exe", "uninstall.exe")
                except:
                    pass
                print("\tFailed to replace installers")
                raise
        else:
            print("\tCan't find new installer files, ignoring them")
        print("Update successful\n")
        return 1
                    
    except:
        print("\nFailed applying the update\n")

        if outdatedSet:
            print("Trying to recover...")
            # Make latest folder empty and copy all outdated files to latest
            try:
                if os.path.exists("latest"):
                    shutil.rmtree("latest")
                    time.sleep(0.1)
                os.mkdir("latest")
                time.sleep(0.1)
                for file in os.listdir("outdated"):
                    shutil.move(os.path.join("outdated", file), os.path.join("latest", file))
                shutil.rmtree("outdated")
                print("Successfully recovered\n")
            except:
                print("Failed to recover\n")

        return 0


def deleteOutdatedFiles():

    # Make the outdated folder empty
    if (os.path.exists("outdated")):
        print("Deleting outdated folder...")
        try:
            # First, delete all .exe files to prevent deleting
            # resource files while an exe is running
            for file in os.listdir("outdated"):
                if (file.endswith(".exe")):
                    os.remove(os.path.join("outdated", file))
            # Next, delete the entire folder
            shutil.rmtree("outdated")
            print("Done")
        except:
            print("Deletion failed")
            sys.exit(-1)

    # Delete outdated updater
    if os.path.exists("updater_outdated.exe"):
        print("Deleting outdated updater...")
        try:
            os.remove("updater_outdated.exe")
            print("Done")
        except:
            print("Failed, not significant")

    # Delete outdated uninstaller
    if os.path.exists("uninstall_outdated.exe"):
        print("Deleting outdated uninstaller...")
        try:
            os.remove("uninstall_outdated.exe")
            print("Done")
        except:
            print("Failed, not significant")


def deleteFiles():
    print("Deleting no longer needed files and folders...")

    # Deleting update folder
    if os.path.exists("update"):
        try:
            print("Deleting update temp folder...")
            shutil.rmtree("update")
            print("Done")
        except:
            print("Failed, not significant")

    # Deleting zip file
    if os.path.exists(automaticUpdaterRelease):
        print("Deleting old zip file...")
        try:
            os.remove(automaticUpdaterRelease)
            print("Done")
        except:
            print("Failed, not significant")

def testModule(module):
    if module not in sys.modules:
        print("Module '" + module + "' was not found!")
        sys.exit(-1)





# If the first command line argument is "test", only perform tests and then exit
if (len(sys.argv) >= 2):
    if sys.argv[1] == "test":

        # Perform test of all modules

        testModule("requests")
        testModule("json")
        testModule("zipfile")
        testModule("os")
        testModule("shutil")
        testModule("time")
        testModule("sys")
        testModule("zc.lockfile")

        sys.exit(0)

# Make sure program is only running once
try:
    lock = zc.lockfile.LockFile('lock')
except:
    print("Can't check for updates: Another process is still running...")
    sys.exit(-1)

deleteOutdatedFiles()

# Find installed version
versionInstalled = ""
try:
    with open("version", "r") as versionFile:
        versionInstalled = versionFile.readlines()[0]
except:
    print("Failed to read installed version")


try:
    release = getLatestGithubRelease("HerrNamenlos123", "TechnicalSketcher")
    if (release.ok):

        j = json.loads(release.content)
        latestVersion = j["tag_name"]

        if (latestVersion != versionInstalled): # Latest version is different from the installed one, update now
            try:
                if (versionInstalled != ""):
                    print("Updating from " + versionInstalled + " to " + latestVersion)
                else:
                    print("Updating to " + latestVersion)
                print("")

                if not downloadUpdate(j):
                    raise ""
                
                if not unzipUpdate():
                    raise ""
                
                if not updateApplication():
                    raise ""
                
                deleteFiles()
                if (versionInstalled != ""):
                    print("\nSuccessfully updated from " + versionInstalled + " to " + latestVersion)
                else:
                    print("\nSuccessfully updated to " + latestVersion)
                print("")
                try:
                    file = open("version","w")
                    file.write(latestVersion) 
                    file.close() 
                except:
                    print("Failed to save installed version string, application may be updated again")
            except:
                print("Update failed, cleaning files...")
                deleteFiles()
                sys.exit(-1)
        else:
            print("Up to date: Version " + versionInstalled)

    else:
        print("Failed to access Github release")
        sys.exit(-1)
except:
    print("Failed to parse Github response")
    sys.exit(-1)
