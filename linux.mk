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

OLDPWD:=$(PWD)
BUILD:=$(OLDPWD)_build

all: sanguosha.qm

$(BUILD)/libfmodex.so:
	mkdir -p $(BUILD)
	@echo "NOTE: if you had installed fmodex please copy the .so file from /usr/local/lib/ to $(BUILD)/libfmodex.so"
	@ls $@

swig/sanguosha_wrap.cxx: swig/sanguosha.i
	cd swig && swig -c++ -lua sanguosha.i

$(BUILD)/Makefile: $(OLDPWD)/QSanguosha.pro
	cd $(BUILD) && qmake $(OLDPWD)/QSanguosha.pro

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
	lrelease QSanguosha.pro
	@echo "Well, everything is OK. You can run it with ./QSanguosha"

clean:
	-cd $(BUILD) && $(MAKE) clean

distclean:
	-cd $(BUILD) && $(MAKE) distclean
	rm -f QSanguosha Makefile swig/sanguosha_wrap.cxx sanguosha.qm

.PHONY: $(BUILD)/QSanguosha swig/sanguosha_wrap.cxx
