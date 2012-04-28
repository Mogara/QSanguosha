#!/bin/sh

# This is a Shell script that ease our compilation work in Linux
# You should has Qt, swig, and plib installed before running this script

which -s qmake
if [[ $? != 0 ]]; then
	echo "Qt is not installed or its tools are not in the PATH"
	exit 1
fi

which -s swig 
if [[ $? != 0 ]]; then
	echo "Swig is not installed!"
	exit 1
fi

# first, generate Lua binding C++ code
echo "Generate lua binding code"
sh swig/swig.sh

# second, we create the Makefile from project file
echo "Generating Makefile using project file"
qmake QSanguosha.pro

# then do make
echo "Compile it"
make

# Compile the translation file
lrelease QSanguosha.pro

echo "Well, everything is OK. You can run it with ./QSanguosha"
