#!/bin/sh

# This script is used to create Mac OS X bundle as well as DMG file

if [ $1 == "-help" -o $1 == "-h" ]; then
    echo "Usage: $0 [-dmg]"
    echo "Create a QSanguosha application bundle in build directory"
    echo "If you provide the parameter '-dmg' is provided, then this script will also create a disk image"
    exit 0
fi

SRC_DIR="$HOME/Projects/QSanguosha"
BUILD_DIR="$HOME/Projects/QSanguosha-build-desktop"

cd $BUILD_DIR

if [ ! -d "QSanguosha.app" ]; then
    echo "QSanguosha.app does not exist in build dir: $BUILD_DIR"
    exit 1
fi

# use here document to store items that should be copied
items=`cat<<EOF
acknowledgement 
audio
backdrop
diy
etc
extension-doc
font 
gpl-3.0.txt 
image 
lang 
lua 
qt_zh_CN.qm 
sanguosha.lua 
sanguosha.qm 
sanguosha.qss 
scenarios
EOF`

# first, we copy all resource files into MacOS folder:
for item in $items 
do
    echo "Copying $item ..."
    cp -R "$SRC_DIR/$item" "$BUILD_DIR/QSanguosha.app/Contents/MacOS"
done

# remove extra font.7z file
echo "Removing font.7z"
rm $BUILD_DIR/QSanguosha.app/Contents/MacOS/font/font.7z

echo "Call macdeployqt"
macdeployqt QSanguosha.app

echo "Call install_name_tool"
install_name_tool  -change ./libfmodex.dylib @executable_path/../Frameworks/libfmodex.dylib QSanguosha.app/Contents/MacOS/QSanguosha

if [ $1 == "-dmg" ]; then
    echo "Create disk image file"
    hdiutil create -srcfolder QSanguosha.app -format UDBZ -volname "太阳神三国杀" $OLDPWD/QSanguosha.dmg
fi

echo "Finished"



