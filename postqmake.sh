#!/bin/sh

# this script is used to generate some files 

qmakedir=$(dirname "$1")

# generate sanguosha.qm

if [ ! -e sanguosha.qm ]; then
	"$qmakedir/lrelease" sanguosha.ts
fi 

# decompress font
TTF_FILE="font/font.ttf"
ZIP_FILE="font/font.7z"
if [ ! -e "$TTF_FILE" ]; then
	if which 7z ; then
		7z x "$ZIP_FILE"
		mv FONT.TTF "$TTF_FILE"
	fi
fi
