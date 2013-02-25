# Linux Makefile for QSanguosha
# Author: pansz at github
#
# you can use the following to compile the first time:
#
#	make -f linux.mk
#
# Next time you could just use 'make'
#
# This makefile only works for GNU Linux and don't try it on other platforms.
#

PREFIX:=/usr/local
OLDPWD:=$(PWD)
BUILD:=$(OLDPWD)_build
DEBUG_BUILD:=$(OLDPWD)_debugbuild

all: sanguosha.qm

$(BUILD)/libfmodex.so:
	mkdir -p $(BUILD)
	@echo "NOTE: if you had installed fmodex please copy the .so file from /usr/local/lib/ to $(BUILD)/libfmodex.so"
	@ls $@

$(DEBUG_BUILD)/libfmodex.so:
	mkdir -p $(BUILD)
	@echo "NOTE: if you had installed fmodex please copy the .so file from /usr/local/lib/ to $(DEBUG_BUILD)/libfmodex.so"
	@ls $@

swig/sanguosha_wrap.cxx: swig/sanguosha.i
	cd swig && swig -c++ -lua sanguosha.i

$(DEBUG_BUILD)/Makefile: $(OLDPWD)/QSanguosha.pro
	cd $(DEBUG_BUILD) && qmake $(OLDPWD)/QSanguosha.pro "CONFIG+=declarative_debug"

$(BUILD)/Makefile: $(OLDPWD)/QSanguosha.pro
	cd $(BUILD) && qmake $(OLDPWD)/QSanguosha.pro "CONFIG+=release"

$(BUILD)/swig/sanguosha_wrap.cxx: swig/sanguosha_wrap.cxx
	mkdir -p $(BUILD)/swig
	cp $(PWD)/swig/sanguosha_wrap.cxx $(BUILD)/swig/sanguosha_wrap.cxx

$(BUILD)/QSanguosha: $(BUILD)/libfmodex.so $(BUILD)/swig/sanguosha_wrap.cxx $(BUILD)/Makefile
	@echo "PWD is: $(OLDPWD)"
	@ln -sf linux.mk Makefile
	cd $(BUILD) && $(MAKE)
	@rm -f QSanguosha
	@ln -sf $(BUILD)/QSanguosha QSanguosha

sanguosha.qm: $(BUILD)/QSanguosha sanguosha.ts
	lupdate QSanguosha.pro
	lrelease QSanguosha.pro
	@echo "Well, compile done. Now you can run make install with root "

install: $(BUILD)/QSanguosha
	mkdir -p $(PREFIX)/games
	mkdir -p $(PREFIX)/share/QSanguosha
	rm -rf $(PREFIX)/share/QSanguosha/*
	install -s $(BUILD)/QSanguosha $(PREFIX)/games/QSanguosha
	cp -r acknowledgement $(PREFIX)/share/QSanguosha/.
	cp -r audio $(PREFIX)/share/QSanguosha/.
	cp -r backdrop $(PREFIX)/share/QSanguosha/.
	cp -r diy $(PREFIX)/share/QSanguosha/.
	cp -r etc $(PREFIX)/share/QSanguosha/.
	cp -r font $(PREFIX)/share/QSanguosha/.
	cp -r image $(PREFIX)/share/QSanguosha/.
	cp -r lang $(PREFIX)/share/QSanguosha/.
	cp -r lua $(PREFIX)/share/QSanguosha/.
	cp -r scenarios $(PREFIX)/share/QSanguosha/.
	cp -r skins $(PREFIX)/share/QSanguosha/.
	cp gpl-3.0.txt $(PREFIX)/share/QSanguosha/COPYING
	cp sanguosha.qm $(PREFIX)/share/QSanguosha/.
	cp sanguosha.qss $(PREFIX)/share/QSanguosha/.

clean:
	-cd $(BUILD) && $(MAKE) clean

distclean:
	-cd $(BUILD) && $(MAKE) distclean
	rm -f QSanguosha Makefile swig/sanguosha_wrap.cxx sanguosha.qm

.PHONY: $(BUILD)/QSanguosha
