# fuzzyclock

![alt FuzzyClock in action](https://raw.githubusercontent.com/m66n/m66n.github.io/master/img/fuzzyclock/FuzzyClock.png)

## Overview

A blatant rip-off of the [KDE source code](http://techbase.kde.org/Getting_Started) with tips picked up from [here](http://homepage1.nifty.com/kazubon/tclocklight/index.html). This code injects a DLL (*FuzzyHook.dll*) into the `WndProc` of `TrayClockWClass` to replace the standard numeric text with fuzzy text. The code also places a clock icon (taken from Crystal SVG; thanks [Everaldo](http://www.everaldo.com/)!) in the system tray. Right-click on the icon to exit the program. 

## Requirements

* [Microsoft Visual C++ 2005 SP1 Redistributable Package (x86)](http://www.microsoft.com/downloads/details.aspx?FamilyID=200b2fd9-ae1a-4a14-984d-389c36f85647&displaylang=en) (for 32-bit platforms)
* [Microsoft Visual C++ 2008 SP1 Redistributable Package (x64)](http://www.microsoft.com/downloads/details.aspx?familyid=BA9257CA-337F-4B40-8C14-157CFDFFEE4E&displaylang=en) (for 64-bit platforms)

If a configuration error dialog is displayed when launching FuzzyClock, install the corresponding redistributable package.

## Installation

1. Extract *FuzzyClock.zip* to a folder of your choosing (i.e. *c:\\Program Files\\FuzzyClock*).
2. The default language is English. Additional languages can be found within *Localizations.zip*. Drag and drop a localization file onto the icon representing *FuzzyClock.exe* and that localization will be used. The localization file is stored in *%APPDATA%\\FuzzyClock* as *FuzzyClock.xml*.
  * In Windows 2000, XP, and Server 2003, *%APPDATA%* is located at *c:\\Documents and Settings\\\<username\>\\Application Data*
  * In Windows Vista, *%APPDATA%* is located at *c:\\Users\\\<username\>\\AppData\\Roaming*
3. To start FuzzyClock with Windows, create a shortcut for *FuzzyClock.exe* and place it in *%STARTMENU%\\Programs\\Startup*.
  * In Windows 2000, XP, and Server 2003, *%STARTMENU%* is located at *c:\\Documents and Settings\\\<username\>\\Start Menu*
  * In Windows Vista, *%STARTMENU%* is located at *c:\\Users\\\<username\>\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu*
  
## Use

* Right-click (or double-click) on FuzzyClock tray icon to exit program.
* Click on FuzzyClock tray icon to toggle between fuzzy and precise times.

## Credits

Almost the entirety of the translations is from [KDE source code](http://i18n.kde.org/). A few words were grabbed from [Microsoft Terminology Translations](http://msdn.microsoft.com/en-us/goglobal/bb688105.aspx).

## Details

Built using Visual Studio 2005 SP1 (x86 build) and Visual Studio 2008 SP1 (x64 build). Tested on Windows XP and Windows Server 2003. According to [Softpedia](http://www.softpedia.com/get/Desktop-Enhancements/Clocks-Time-Management/FuzyClock.shtml), this should work on Win2K and Vista, too.

Copyright (c) Michael Chapman. All rights reserved.
