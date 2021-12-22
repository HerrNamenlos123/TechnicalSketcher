# TechnicalSketcher

<div>Icons made by <a href="http://www.freepik.com/" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div> 


Download the latest version here (Download the .msi file): 

https://github.com/HerrNamenlos123/TechnicalSketcher/releases/latest

## Building from source

 - Clone and build the [BatteryEngine](https://github.com/HerrNamenlos123/BatteryEngine)
 - Clone this repository: `git clone --recursive https://github.com/HerrNamenlos123/TechnicalSketcher.git`
 - Execute `generate-win.bat`
 - Build the project in `Debug` or `Release` configuration

### Building the installer

 - Enable the [.NET Framework 3.5](https://docs.microsoft.com/en-us/dotnet/framework/install/dotnet-35-windows), needed for WiX (use the control panel)
 - Install the [WiX Toolchain](https://wixtoolset.org/releases/) (direct link -> [WiX stable 3.11.2](https://github.com/wixtoolset/wix3/releases/download/wix3112rtm/wix311.exe))
 - Build the project in `Deploy` configuration
