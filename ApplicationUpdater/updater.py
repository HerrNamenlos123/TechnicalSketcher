
import requests
import json
import zipfile
import os
import shutil

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
    print("New update is available, downloading...")
    try:
        assets = j["assets"]
        for asset in assets:
            if (asset["name"] == automaticUpdaterRelease):
                url = asset["browser_download_url"]

                download_file(url)
                print("Successfully downloaded update as " + automaticUpdaterRelease)
                    
    except:
        print("Failed to download update")

def unzipUpdate():
    print("Unzipping...")
    try:
        with zipfile.ZipFile(automaticUpdaterRelease, 'r') as zip_ref:
            zip_ref.extractall("update")
        
        print("Successfully unzipped update")
                    
    except:
        print("Failed to unzip update")

def updateApplication():
    print("Updating application now...")
    try:
        # Make the outdated folder empty
        if (os.path.exists("outdated")):
            # First, delete all .exe files to prevent deleting 
            # resource files while an exe is running
            for file in os.listdir("outdated"):
                if (file.endswith(".exe")):
                    os.remove(os.path.join("outdated", file))
            # Next, delete the entire folder
            shutil.rmtree("outdated")

        # Only move files to outdated if latest even exists
        if (os.path.exists("latest")):
            # Outdated folder is now gone
            os.mkdir("outdated")

            # Now move all files to outdated
            for file in os.listdir("latest"):
                shutil.move(os.path.join("latest", file), os.path.join("outdated", file))
        else:
            os.mkdir("latest")

        try:
            # Now move the new files to latest
            for file in os.listdir("update/app"):
                shutil.move(os.path.join("update/app", file), os.path.join("latest", file))

            # Update was successful, now delete application files
            shutil.rmtree("update/app")
            print("Successfully updated the main application")
            print("Updating updater and uninstaller now")

            # Check if updates are there
            if (os.path.exists("update/updater.exe") and os.path.exists("update/uninstall.exe")):
                # Now apply them
                shutil.move("uninstall.exe", "uninstall_outdated.exe")
                shutil.move("update/uninstall.exe", "uninstall.exe")
                shutil.move("updater.exe", "updater_outdated.exe")
                shutil.move("update/updater.exe", "updater.exe")

                # And delete all files
                shutil.rmtree("update")
            else:
                print("Can't update updater and uninstaller, files were not included")
                shutil.rmtree("update")

        except:
            print("Failed applying update, trying to revert...")

            if (os.path.exists("outdated")):
                if (not os.path.exists("latest")):
                    os.mkdir("latest")

                # Move all files to latest
                for file in os.listdir("outdated"):
                    shutil.move(os.path.join("outdated", file), os.path.join("latest", file))
                print("Application restored, restoring updater and uninstaller")

                # Restore updater and uninstaller
                if (os.path.exists("updater_outdated.exe") and os.path.exists("uninstall_outdated.exe")):
                    shutil.move("updater_outdated.exe", "updater.exe")
                    shutil.move("uninstall_outdated.exe", "uninstall.exe")

                print("Successfully restored all parts")
            else:
                print("Failed to revert all parts")
        
                    
    except:
        print("Failed to apply the update")


def deleteFiles():

    # Make the outdated folder empty
    if (os.path.exists("outdated")):
        print("Trying to delete outdated folder...")
        try:
            # First, delete all .exe files to prevent deleting 
            # resource files while an exe is running
            for file in os.listdir("outdated"):
                if (file.endswith(".exe")):
                    os.remove(os.path.join("outdated", file))
            # Next, delete the entire folder
            shutil.rmtree("outdated")

            print("Outdated folder was deleted")
        except:
            print("Failed to delete outdated folder, program is probably still running")

    if os.path.exists(automaticUpdaterRelease):
        print("Trying to delete old zip file...")
        try:
            os.remove(automaticUpdaterRelease)
        except:
            print("Failed to delete old zip file")
        print("Deleted old zip file")







versionInstalled = ""
try:
    with open("latest/version", "r") as versionFile:
        versionInstalled = versionFile.readlines()[0]
except:
    print("Failed to read installed version")


try:
    release = getLatestGithubRelease("HerrNamenlos123", "TechnicalSketcher")
    if (release.ok):

        j = json.loads(release.content)
        latestVersion = j["tag_name"]

        if (latestVersion != versionInstalled): # Latest version is different from the installed one, update now
            downloadUpdate(j)
            unzipUpdate()
            updateApplication()
            deleteFiles()
            print("Done")

    else:
        print("Failed to access Github Releases")
except:
    print("Failed to parse Github response")
