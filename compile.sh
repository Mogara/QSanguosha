#!/bin/sh

# This is a Shell script that ease our compilation work in Linux
# You should has Qt, swig, and plib installed before running this script

if ! which qmake ; then
	echo "Qt is not installed or its tools are not in the PATH"
	exit 1
fi

if ! which swig ; then
	echo "Swig is not installed!"
	exit 1
fi

if ! which 7z ; then
	echo "You need 7z to unzip some files"
	exit 1
fi


echo "Decompressing ttf file"
cd font
7z x font.7z
if [ -f "FONT.TTF" ]; then
	mv FONT.TTF font.ttf # to lowercase
fi
cd $OLDPWD

# first, generate Lua binding C++ code
echo "Generate lua binding code"
sh swig/swig.sh

# second, we create the Makefile from project file
echo "Generating Makefile using project file"
qmake QSanguosha.pro

# then do make
echo "Compile it"
if ! make ; then
	echo "Compiling error"
	exit 1
fi

# Compile the translation file
lrelease QSanguosha.pro

echo "Well, everything is OK. You can run it with ./QSanguosha"
