#!/bin/sh

QMAKE="$1"
SOURCE="$2"
if [ -z "$SOURCE" ]; then
  SOURCE='QSanguosha'
fi

BUILD="$3"
if [ -z "$BUILD"]; then
  BUILD='QSanguosha-build'
fi

RELEASE='QSanguosha-release'


if [ -z "$QMAKE" ]; then
    QMAKE=$(which qmake)
    if [ -z "$QMAKE" ]; then
        echo "qmake is not installed at PATH, quiting..."
        exit 1
    fi
fi

qtversion=$($QMAKE -v | sed -n 's/.*Qt version \([0-9.]\).*/\1/p')
qtdir=$(dirname $QMAKE)
echo "Using Qt's drectory: $qtdir"

mkdir -p $RELEASE

mkdir -p $RELEASE/imageformats
cp $qtdir/../plugins/imageformats/qjpeg.dll $RELEASE/imageformats

if [ "$qtversion" == '5' ]; then
    mkdir -p $RELEASE/platforms
    cp $qtdir/../plugins/platforms/qwindows.dll $RELEASE/platforms
    cp $qtdir/../plugins/platforms/qminimal.dll $RELEASE/platforms

    dlls=$(cat <<EOF
        icudt51
        icuin51
        icuuc51
        libgcc_s_dw2-1
        libstdc++-6
        libwinpthread-1
        Qt5Core
        Qt5Gui
        Qt5Network
        Qt5Sql
        Qt5Widgets
EOF)
else
    dlls=$(cat <<EOF
        libgcc_s_dw2-1
        mingwm10
        QtCore4
        QtDeclarative4
        QtGui4
        QtNetwork4
        QtSql4
        QtXml4
        QtXmlPatterns4
EOF)
fi

for dll in $dlls
do
    cp $qtdir/$dll.dll $RELEASE
done

cplist=$(cat <<EOF
audio
backdrop
diy
etc
extension-doc
image
lang
lua
scenarios
fmodex.dll
libluasqlite3.dll
qt_zh_CN.qm
sanguosha.qss
gpl-3.0.txt
7zr.exe
EOF)

for file in $cplist
do
  cp -R $SOURCE/$file $RELEASE
done

mkdir -p $RELEASE/font
cp $SOURCE/font/font.ttf $RELEASE/font

if [ ! -f "$SOURCE/sanguosha.qm" ]; then
    pushd $PWD
    echo "No sanguosha.qm found, publish it"
    cd $SOURCE
    $qtdir/lrelease sanguosha.ts
    popd
fi
cp $SOURCE/sanguosha.qm $RELEASE

cp $BUILD/release/QSanguosha.exe $RELEASE

