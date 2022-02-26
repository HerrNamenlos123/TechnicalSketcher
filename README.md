# TechnicalSketcher

<div>Icons made by <a href="http://www.freepik.com/" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a></div> 


Download the latest version here (Download the .msi file): 

https://github.com/HerrNamenlos123/TechnicalSketcher/releases/latest

Explain why the msi is blocked by windows smartscreen

<a href="https://hits.seeyoufarm.com"><img src="https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2FHerrNamenlos123%2FTechnicalSketcher&count_bg=%232188FF&title_bg=%23555555&icon=github.svg&icon_color=%23E7E7E7&title=visitors+today%2Ftotal%3A&edge_flat=false"/></a>
<a href="https://hits.seeyoufarm.com"><img src="https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2FHerrNamenlos123%2FTechnicalSketcher%2Freleases%2Flatest%2Fdownload%2FTechnicalSketcher-Installer-x64.msi&count_bg=%232188FF&title_bg=%23555555&icon=github.svg&icon_color=%23E7E7E7&title=downloads+today%2Ftotal%3A&edge_flat=false"/></a>

## Building from source

 - Clone and build the [BatteryEngine](https://github.com/HerrNamenlos123/BatteryEngine)
 - Clone this repository: `git clone --recursive https://github.com/HerrNamenlos123/TechnicalSketcher.git`
 - Execute `generate-win.bat`
 - Build the project in `Debug` or `Release` configuration

### Building the installer

 - Enable the [.NET Framework 3.5](https://docs.microsoft.com/en-us/dotnet/framework/install/dotnet-35-windows), needed for WiX (use the control panel)
 - Install the [WiX Toolchain](https://wixtoolset.org/releases/) (direct link -> [WiX stable 3.11.2](https://github.com/wixtoolset/wix3/releases/download/wix3112rtm/wix311.exe))
 - Build the project in `Deploy` configuration
