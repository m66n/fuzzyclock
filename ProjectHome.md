**Latest revision available via [SVN](http://code.google.com/p/fuzzyclock/source/checkout).**

![http://fuzzyclock.googlecode.com/svn/images/FuzzyClock.png](http://fuzzyclock.googlecode.com/svn/images/FuzzyClock.png)

## Overview ##
A blatant rip-off of the [KDE source code](http://techbase.kde.org/Getting_Started) with tips picked up from [here](http://homepage1.nifty.com/kazubon/tclocklight/index.html). This code injects a DLL (_FuzzyHook.dll_) into the `WndProc` of `TrayClockWClass` to replace the standard numeric text with fuzzy text. The code also places a clock icon (taken from Crystal SVG; thanks [Everaldo](http://www.everaldo.com/)!) in the system tray. Right-click on the icon to exit the program.

## Requirements ##
  * [Microsoft Visual C++ 2005 SP1 Redistributable Package (x86)](http://www.microsoft.com/downloads/details.aspx?FamilyID=200b2fd9-ae1a-4a14-984d-389c36f85647&displaylang=en) (for 32-bit platforms)
  * [Microsoft Visual C++ 2008 SP1 Redistributable Package (x64)](http://www.microsoft.com/downloads/details.aspx?familyid=BA9257CA-337F-4B40-8C14-157CFDFFEE4E&displaylang=en) (for 64-bit platforms)

If a configuration error dialog is displayed when launching FuzzyClock, install the corresponding redistributable package.

## Installation ##
  1. Extract _FuzzyClock.zip_ to a folder of your choosing (i.e. _x:\Program Files\FuzzyClock_).
  1. The default language is English. Additional languages can be found within _Localizations.zip_.  Drag and drop a localization file onto the icon representing _FuzzyClock.exe_ and that localization will be used.  The localization file is stored in _%APPDATA%\FuzzyClock_ as _FuzzyClock.xml_.
    * In Windows 2000, XP, and Server 2003, _%APPDATA%_ is located at _x:\Documents and Settings\`<`username`>`\Application Data_
    * In Windows Vista, _%APPDATA%_ is located at _x:\Users\`<`username`>`\AppData\Roaming_
  1. To start FuzzyClock with Windows, create a shortcut for _FuzzyClock.exe_ and place it in _%STARTMENU%\Programs\Startup_.
    * In Windows 2000, XP, and Server 2003, _%STARTMENU%_ is located at _x:\Documents and Settings\`<`username`>`\Start Menu_
    * In Windows Vista, _%STARTMENU%_ is located at _x:\Users\`<`username`>`\AppData\Roaming\Microsoft\Windows\Start Menu_

## Use ##
  * Right-click (or double-click) on FuzzyClock tray icon to exit program.
  * Click on FuzzyClock tray icon to toggle between fuzzy and precise times.

## Credits ##
Almost the entirety of the translations is from [KDE source code](http://i18n.kde.org/). A few words were grabbed from [Microsoft Terminology Translations](http://msdn.microsoft.com/en-us/goglobal/bb688105.aspx).

## Details ##
Built using Visual Studio 2005 SP1 (x86 build) and Visual Studio 2008 SP1 (x64 build).  Tested on Windows XP and Windows Server 2003.  According to [Softpedia](http://www.softpedia.com/get/Desktop-Enhancements/Clocks-Time-Management/FuzyClock.shtml), this should work on Win2K and Vista, too.