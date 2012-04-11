#!/bin/sh

# This is a Shell script that ease our compilation work in Linux
# You should has Qt, swig, and plib installed before running this script

# first, generate Lua binding C++ code
cd swig
swig -c++ -lua sanguosha.i
cd ..

# second, we create the Makefile from project file
echo "Generating Makefile using project file"
qmake QSanguosha.pro

# then do make
echo "Compile it"
make

# Compile the translation file
lrelease QSanguosha.pro

echo "Well, everything is OK. You can run it with ./QSanguosha"
