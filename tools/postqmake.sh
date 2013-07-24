#!/bin/sh

# this script is used to generate some files 

qmakedir=$(dirname "$1")

# generate sanguosha.qm

if [ ! -e sanguosha.qm ]; then
	echo "Qt message file does not exist, generate it"
	"$qmakedir/lrelease" sanguosha.ts
	echo "Done"
fi 

# decompress font
TTF_FILE="font/font.ttf"
ZIP_FILE="font/font.7z"
if [ ! -e "$TTF_FILE" ]; then
	echo "Font file does not exists, should extract it from a 7z file"
	if which 7z ; then
		7z x "$ZIP_FILE"
		mv FONT.TTF "$TTF_FILE"
	else
		echo "7z command does not exist, please decompress manually"
	fi
fi
