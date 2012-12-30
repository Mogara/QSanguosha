#!/bin/sh

if [ -d swig ]; then
	cd swig
fi

swig -c++ -lua sanguosha.i
