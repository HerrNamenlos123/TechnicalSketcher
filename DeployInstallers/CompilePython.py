
import sys
import os
import subprocess

requiredPythonVersion = "3.7.4"
requiredPyInstallerVersion = "3.4"

def checkPythonVersion():
    version = str(sys.version_info[0]) + "." + str(sys.version_info[1]) + "." + str(sys.version_info[2])
    if (version != requiredPythonVersion):
        print("Can't compile python files: The wrong python version is installed! Installed version is " + version
        + ", but version " + requiredPythonVersion + " is required due to antivirus concerns.")
        sys.exit(-1)

def checkPyInstallerVersion():
    version = subprocess.run(["pyinstaller", "--version"], capture_output=True, shell=True, 
                            universal_newlines = False).stdout.splitlines()[0].decode()

    if (version != requiredPyInstallerVersion):
        print("Can't compile python files: The wrong PyInstaller version is installed! Installed version is " + version
        + ", but version " + requiredPyInstallerVersion + " is required due to antivirus concerns.")
        sys.exit(-1)

def compilePythonFile(file):
    print("Compiling file " + file)
    process = subprocess.Popen(["pyinstaller", "--onefile", file], shell=True, 
                                universal_newlines = False, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
    output = process.communicate()[0].decode()
    returnCode = process.returncode

    if (returnCode != 0):
        print("Failed to compile python files\n")
        print("PyInstaller output: " + output)
        print("PyInstaller returncode: " + str(returnCode))
        print("Failed to compile python files")
        sys.exit(-1)

    print("Success")

def testExecutable(file):
    print("Performing test on " + file + "...")
    process = subprocess.Popen(["cd", "dist", "&&", file, "test"], shell=True, 
                                universal_newlines = False, stderr=subprocess.STDOUT, stdout=subprocess.PIPE)
    output = process.communicate()[0].decode()
    returnCode = process.returncode

    if (returnCode != 0):
        print("Test failed, application did not exist or a module was not imported correctly!")
        print("Application output: " + output)
        sys.exit(-1)

    print("Test successful")




# Check if the correct versions are installed
checkPythonVersion()
checkPyInstallerVersion()

# Now the files can be compiled
compilePythonFile("../PythonModules/updater.py")
compilePythonFile("../PythonModules/uninstall.py")

# Check if executables work correctly
testExecutable("updater.exe")
testExecutable("uninstall.exe")

# Everything successful
print("Successfully compiled python files")
