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

# copy irrKlang files and update the dynamic library cache
echo "Copying irrKlang library files to /usr/lib"
cp lib/libIrrKlang.so /usr/lib
cp lib/ikpMP3.so /usr/lib

echo "Updating cache"
ldconfig

echo "Well, everything is OK. You can run it with ./QSanguosha"
