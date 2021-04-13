
import os
import shutil
import win32com.client
import signal
import time
import sys

def getpid(process_name):
    wmi=win32com.client.GetObject('winmgmts:')
    return [item.split()[1] for item in os.popen('tasklist').read().splitlines()[4:] if process_name in item.split()]

def terminateExecutable(name):
    wmi=win32com.client.GetObject('winmgmts:')
    found = 0
    for p in wmi.InstancesOf('win32_process'):
        if p.Name == name:
            found = 1
            process_id = getpid(name)
            try:
                if len(process_id) > 1:
                    for x in process_id:
                        pid = int(x)
                        os.kill(pid, signal.SIGTERM)
                        print(name + " terminated")

                else:
                    pid = int("".join(getpid(name)))
                    os.kill(pid, signal.SIGTERM)
                    print(name + " terminated")
            except:
                print("Failed to terminate, permission denied")
                
    if not found:
        print("Process not found")

def deleteExecutable(name):
    print("Deleting executable: " + name)
    try:
        os.remove(name)
        print("Success")
        return 1
    except:
        try:
            print("Terminating process...")
            terminateExecutable(os.path.basename(name))
            time.sleep(1)
            os.remove(name)
            print("Done")
            return 1
        except:
            print("Failed to delete executable")
            return 0

def deleteAppdata():
    datapath = os.getenv('APPDATA')
    try:
        shutil.rmtree(os.path.join(datapath, "TechnicalSketcher"))
    except:
        return

def deleteFolder(folder):
    if os.path.exists(folder):
        print("Deleting folder: " + folder)
        try:
            for file in os.listdir(folder):
                try:
                    if os.path.isfile(os.path.join(folder, file)):
                        if file[-4:] == ".exe":
                            deleteExecutable(os.path.join(folder, file))
                        else:
                            print("Removing file: " + os.path.join(folder, file))
                            os.remove(os.path.join(folder, file))
                            print("Success")
                    else:
                        deleteFolder(os.path.join(folder, file))
                except:
                    print("Failed to remove file in folder")
                    return
            shutil.rmtree(folder)
            print("Successfully removed folder: " + folder)
        except:
            print("Failed to remove folder")

def deleteLocalAppdata():
    folder = os.path.join(os.getenv('LOCALAPPDATA'), "TechnicalSketcher")
    if os.path.exists(folder):
        for file in os.listdir(folder):
            try:
                if os.path.isfile(os.path.join(folder, file)):
                    if file != "uninstall.exe":
                        if file[-4:] == ".exe":
                            deleteExecutable(os.path.join(folder, file))
                        else:
                            os.remove(os.path.join(folder, file))
                else:
                    deleteFolder(os.path.join(folder, file))
            except:
                pass

def testModule(module):
    if module not in sys.modules:
        print("Module '" + module + "' was not found!")
        sys.exit(-1)



def main():
    # First, set the working directory
    os.chdir(os.path.dirname(sys.executable))
    
    # If the first command line argument is "test", only perform tests and then exit
    if (len(sys.argv) >= 2):
        if sys.argv[1] == "test":
    
            # Perform test of all modules
    
            testModule("os")
            testModule("shutil")
            testModule("win32com.client")
            testModule("signal")
            testModule("time")
            testModule("sys")
    
            sys.exit(0)
    
    try:
        # Delete all files from the local appdata location
        deleteAppdata()
        deleteLocalAppdata()
    except:
        print("Failed to delete files")


if __name__ == "__main__":
    main()