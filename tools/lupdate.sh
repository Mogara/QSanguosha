#!/bin/sh

# This script is used to update sanguosha.ts under Mac

sed -e '/axcontainer/d' QSanguosha.pro > TempQSanguosha.pro
lupdate TempQSanguosha.pro -no-obsolete
rm TempQSanguosha.pro