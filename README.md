Open Source Sanguosha
==========

| Homepage:      | http://qsanguosha.org                        |
|----------------|-----------------------------------------------|
| API reference: | http://gaodayihao.github.com/QSanguosha/api   |
| Documentation: | https://github.com/gaodayihao/QSanguosha/wiki (Chinese) |

Third party evaluations of development process
---------
[![Code Climate](https://codeclimate.com/github/QSanguosha-Rara/QSanguosha-For-Hegemony.png)](https://codeclimate.com/github/QSanguosha-Rara/QSanguosha-For-Hegemony) 

[![Issue Stats](http://issuestats.com/github/QSanguosha/QSanguosha-For-Hegemony/badge/pr?style=flat)](http://issuestats.com/github/QSanguosha/QSanguosha-For-Hegemony)

[![Issue Stats](http://issuestats.com/github/QSanguosha/QSanguosha-For-Hegemony/badge/issue?style=flat)](http://issuestats.com/github/QSanguosha/QSanguosha-For-Hegemony)

Lisense
------------
###Code
This game is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3.0
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

See the LICENSE file for more details.

###Material
Our Materials are under the terms of the Creative Commons
Attribution-NonCommercial-NoDerivatives 4.0 International (CC
BY-NC-ND 4.0)

**You are free to:**

Share — copy and redistribute the material in any medium or format

**Under the following terms:**

Attribution — You must give appropriate credit, provide a link to
the license, and indicate if changes were made. You may do so in
any reasonable manner, but not in any way that suggests the licensor
endorses you or your use.

NonCommercial — You may not use the material for commercial purposes.

NoDerivatives — If you remix, transform, or build upon the material,
you may not distribute the modified material.

See the CC BY-NC-ND 4.0 file for more details.

Introduction
----------

Sanguosha is both a popular board game and online game,
this project try to clone the Sanguosha online version.
The whole project is written in C++,
using Qt's graphics view framework as the game engine.
I've tried many other open source game engines,
such as SDL, HGE, Clanlib and others,
but many of them lack some important features.
Although Qt is an application framework instead of a game engine,
its graphics view framework is suitable for my game developing. By Moligaloo

Features
----------

1. Framework
    * Open source with Qt graphics view framework
    * Use FMOD as sound engine
    * Use Freetype in Font Rendering
    * Use Lua as AI and extension script

2. Operation experience
    * Full package (include all yoka extension package)
    * Keyboard shortcut
    * Double-click to use cards
    * Cards sorting (by card type and card suit)
    * Multilayer display when cards are more than an upperlimit

3. Extensible
    * Some MODs are available based on this game
    * Lua Packages are supported in this game

HOW TO BUILD
=========
**Tips: "~" stands for the folder where the repo is in.**

VS2013(Windows)
--------

1. Download the following packages:
(1) QT libraries for Windows (Visual Studio 2013, 5.3.0) http://download.qt-project.org/official_releases/qt/5.3/5.3.0/q

3. Open Qsanguosha.sln right under ~/builds/vs2013, change the Configuration to Release Qt5|Win32.

4. Right click project "QSanguosha" in your Solution Explorer, select "Properties", go to "Debugging" tab, set "Working Directory" to "$(ProjectDir)..\..\" (do not enter the quote marks). Then select "OK".

4.1. [optional] Right click "sanguosha.ts" in the folder "Translaton Files" in project "QSanguosha", select "lrelease".

5. You are now able to build the solution. When compilation succeeded, the QSanguosha.exe is in ~/Bin folder. You should move this file to ~ folder.

6. Copy 6 files from Qt libraries to ~, they are listed below:
   Qt5Core.dll
   Qt5Gui.dll
   Qt5Network.dll
   Qt5Qml.dll
   Qt5Quick.dll
   Qt5Widgets.dll

   Copy 2 files from VS redist to ~, they are listed below:
   msvcp120.dll
   msvcr120.dll

7. Double-click the QSanguosha.exe and have fun!

Mac OS X
----------

1. Install XCode from App Store and enable the command tool

2. Download and install the libraries and Qt Creator from http://download.qt-project.org/official_releases/qt/5.3/5.3.0/qt-opensource-mac-x64-clang-5.3.0.dmg

3. Download the source code and install swig from http://sourceforge.net/projects/swig/files/swig/ and make sure the version of swig is 3.0.1 or later.
   Tips: unzip the source code tarball and open a terminal, type:
   ./configure --without-pcre
   make
   sudo make install

4. Open a terminal here, type:
   cd swig
   swig -c++ -lua sanguosha.i

5. Open QSanguosha.pro with Qt Creator, configure the project and make sure the project is compiled with clang. Change the configuration to Release.

6. You are now able to build the solution. When compilation succeeded, the QSanguosha.app folder is in ~/../Build-QSanguosha-**/ folder.
   I highly recommend you move this folder to ~.

7. This step is the most important and difficult one. Please pay a lot of attention to read this step.
   Open a terminal here, type:
   otool -L QSanguosha.app/Contents/MacOS/QSanguosha

You'll see something like this:
   QSanguosha.app/Contents/MacOS/QSanguosha:
   ./libfmodex.dylib (compatibility version 1.0.0, current version 1.0.0)
   libfreetype.1.dylib (compatibility version 1.0.0, current version 1.0.0)
   (QtDir)/5.3/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtCore.framework/Versions/5/QtCore (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtGui.framework/Versions/5/QtGui (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtQml.framework/Versions/5/QtQml (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtQuick.framework/Versions/5/QtQuick (compatibility version 5.3.0, current version 5.3.0)
   /System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL (compatibility version 1.0.0, current version 1.0.0)
   /System/Library/Frameworks/AGL.framework/Versions/A/AGL (compatibility version 1.0.0, current version 1.0.0)
   /usr/lib/libstdc++.6.dylib (compatibility version 7.0.0, current version 56.0.0)
   /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 169.3.0)

Pay attention to the 2 lines contains libfmodex.dylib or libfreetype.1.dylib.
using install_name_tool to change the path of the 2 library files to their absolute paths, like this:
   install_name_tool -change ./libfmodex.dylib ~/lib/mac/lib/libfmodex.dylib QSanguosha.app/Contents/MacOS/QSanguosha
   install_name_tool -change libfreetype.1.dylib ~/lib/mac/lib/libfreetype.dylib  QSanguosha.app/Contents/MacOS/QSanguosha

Type:
   otool -L QSanguosha.app/Contents/MacOS/QSanguosha

if you see all the directories is absolute path, it should succeed. (~ stands for the project dir)
   QSanguosha.app/Contents/MacOS/QSanguosha:
   ~/lib/mac/lib/libfmodex.dylib (compatibility version 1.0.0, current version 1.0.0)
   ~/lib/mac/lib/libfreetype.dylib (compatibility version 1.0.0, current version 1.0.0)
   (QtDir)/5.3/clang_64/lib/QtDeclarative.framework/Versions/5/QtDeclarative (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtXmlPatterns.framework/Versions/5/QtXmlPatterns (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtCore.framework/Versions/5/QtCore (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtGui.framework/Versions/5/QtGui (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtSql.framework/Versions/5/QtSql (compatibility version 5.3.0, current version 5.3.0)
   (QtDir)/5.3/clang_64/lib/QtScript.framework/Versions/5/QtScript (compatibility version 5.3.0, current version 5.3.0)
   /System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL (compatibility version 1.0.0, current version 1.0.0)
   /System/Library/Frameworks/AGL.framework/Versions/A/AGL (compatibility version 1.0.0, current version 1.0.0)
   /usr/lib/libstdc++.6.dylib (compatibility version 7.0.0, current version 56.0.0)
   /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 169.3.0)

Type:
   macdeployqt QSanguosha.app/

Some times the macdeployqt is not in the system directories, in this case you should go to the Qt Install Dir and find this file.

Type:
   otool -L QSanguosha.app/Contents/MacOS/QSanguosha

if you see all the non-system libraries is in @executable_path, it should succeed.
   QSanguosha.app/Contents/MacOS/QSanguosha:
   @executable_path/../Frameworks/libfmodex.dylib (compatibility version 1.0.0, current version 1.0.0)
   @executable_path/../Frameworks/libfreetype.dylib (compatibility version 1.0.0, current version 1.0.0)
   @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork (compatibility version 5.3.0, current version 5.3.0)
   @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore (compatibility version 5.3.0, current version 5.3.0)
   @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets (compatibility version 5.3.0, current version 5.3.0)
   @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui (compatibility version 5.3.0, current version 5.3.0)
   @executable_path/../Frameworks/QtQml.framework/Versions/5/QtQml (compatibility version 5.3.0, current version 5.3.0)
   @executable_path/../Frameworks/QtQuick.framework/Versions/5/QtQuick (compatibility version 5.3.0, current version 5.3.0)
   /System/Library/Frameworks/OpenGL.framework/Versions/A/OpenGL (compatibility version 1.0.0, current version 1.0.0)
   /System/Library/Frameworks/AGL.framework/Versions/A/AGL (compatibility version 1.0.0, current version 1.0.0)
   /usr/lib/libstdc++.6.dylib (compatibility version 7.0.0, current version 56.0.0)
   /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 169.3.0)

8.copy the following folders and files to QSanguosha.app/Contents/MacOS/, they are listed below:
   ai-selector/
   audio/
   developers/
   diy/
   font/
   image/
   lang/
   lua/
   rule/
   skins/
   ui-script/
   qt_zh_CN.qm
   sanguosha.qss

8.1 [optional] select Tools/External/QtLinguist/lrelease, copy ~/Builds/vs2013/sanguosha.qm to QSanguosha.app/Contents/MacOS.

9. Double-click QSanguosha.app folder and have fun!

Linux or MinGW(Windows)
----------

1. Download and install the libraries and Qt Creator from Qt offical website or software sources, make sure the version of Qt libraries is 5.3.x.
   (I strongly recommand you download the libraries and Qt Creator from software sources in Linux, because the compliation causes a lot of time)
   (If you are using MinGW, make sure the version of MinGW is compatible with Qt Libraries)

2. Install SWIG
(Linux)Download and install the swig from http://sourceforge.net/projects/swig/files/swig/ or software sources, make sure the version of swig is 3.0.1 or later.

(MinGW)Download the swigwin (swig for Windows, 3.0.2) http://sourceforge.net/projects/swig/files/swigwin/
   Create a ~/tools/swig folder under your source directory. Unzip swigwin and copy all unzipped files to ~/tools/swig.

3. Open a terminal here, type:
(Linux)
   cd swig
   swig -c++ -lua sanguosha.i
   cd ../lib/linux
   (x86)cd x86 (or) (x64)cd x64
   sudo cp libfmodex*.so /usr/lib (root password required)
   sudo ldconfig

(MinGW)
   cd swig
   ..\tools\swig\swig.exe -c++ -lua sanguosha.i

4. Open QSanguosha.pro, configure the project and make sure the project is compiled with g++. Change the configuration to Release.

4.1 [optional] select Tools/External/QtLinguist/lrelease, copy ~/Builds/vs2013/sanguosha.qm to ~.

5. You are now able to build the solution. When compilation succeeded, the (MinGW)QSanguosha.exe or (Linux)QSanguosha is in ~/../Build-QSanguosha-**/ folder. You should move this file to ~ folder.

6. (MinGW only)Copy 8 files from Qt libraries to ~, they are listed below:
   Qt5Core.dll
   Qt5Gui.dll
   Qt5Network.dll
   Qt5Qml.dll
   Qt5Quick.dll
   Qt5Widgets.dll

   (MinGW only)Copy 3 files from MinGW bin folder to ~, they are listed below:
   libgcc_s_dw2-1.dll
   libstdc++-6.dll
   libwinpthread-1.dll


