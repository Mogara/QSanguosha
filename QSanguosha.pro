# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = QSanguosha
QT += network sql declarative
TEMPLATE = app
CONFIG += warn_on audio qaxcontainer

# If you want to enable joystick support, please uncomment the following line:
# CONFIG += joystick
# However, joystick is not supported under Mac OS X temporarily

SOURCES += src/main.cpp \
	src/client/aux-skills.cpp \
	src/client/client.cpp \
	src/client/clientplayer.cpp \
	src/client/clientstruct.cpp \
	src/core/card.cpp \
	src/core/engine.cpp \
	src/core/general.cpp \
	src/core/lua-wrapper.cpp \
	src/core/player.cpp \
	src/core/settings.cpp \
	src/core/skill.cpp \
	src/dialog/cardeditor.cpp \
	src/dialog/cardoverview.cpp \
	src/dialog/choosegeneraldialog.cpp \
	src/dialog/configdialog.cpp \
	src/dialog/connectiondialog.cpp \
	src/dialog/distanceviewdialog.cpp \
	src/dialog/generaloverview.cpp \
	src/dialog/generalselector.cpp \
	src/dialog/mainwindow.cpp \
	src/dialog/packagingeditor.cpp \
	src/dialog/playercarddialog.cpp \
	src/dialog/roleassigndialog.cpp \
	src/dialog/scenario-overview.cpp \
        src/dialog/halldialog.cpp \
        src/package/exppattern.cpp \
	src/package/firepackage.cpp \
	src/package/god.cpp \
	src/package/joypackage.cpp \
	src/package/maneuvering.cpp \
	src/package/mountainpackage.cpp \
	src/package/nostalgia.cpp \
	src/package/sp-package.cpp \
	src/package/standard-cards.cpp \
	src/package/standard-generals.cpp \
	src/package/standard-skillcards.cpp \
	src/package/standard.cpp \
	src/package/thicket.cpp \
	src/package/wind.cpp \
	src/package/wisdompackage.cpp \
	src/package/yitianpackage.cpp \
	src/package/yjcm-package.cpp \
        src/package/bgm-package.cpp \
	src/scenario/boss-mode-scenario.cpp \
	src/scenario/couple-scenario.cpp \
	src/scenario/fancheng-scenario.cpp \
	src/scenario/guandu-scenario.cpp \
	src/scenario/scenario.cpp \
	src/scenario/scenerule.cpp \
	src/scenario/zombie-mode-scenario.cpp \
	src/server/ai.cpp \
	src/server/contestdb.cpp \
	src/server/gamerule.cpp \
	src/server/room.cpp \
	src/server/roomthread.cpp \
	src/server/roomthread1v1.cpp \
	src/server/roomthread3v3.cpp \
	src/server/server.cpp \
	src/server/serverplayer.cpp \
	src/ui/button.cpp \
	src/ui/cardcontainer.cpp \
	src/ui/carditem.cpp \
	src/ui/clientlogbox.cpp \
	src/ui/dashboard.cpp \
	src/ui/indicatoritem.cpp \
	src/ui/photo.cpp \
	src/ui/pixmap.cpp \
	src/ui/rolecombobox.cpp \
	src/ui/roomscene.cpp \
	src/ui/startscene.cpp \
	src/ui/window.cpp \
	src/util/detector.cpp \
	src/util/nativesocket.cpp \
	src/util/recorder.cpp \
	swig/sanguosha_wrap.cxx \
    src/ui/pixmapanimation.cpp \
    src/ui/sprite.cpp \
    src/core/banpair.cpp \
    src/ui/chatwidget.cpp \
    src/scenario/miniscenarios.cpp \
    src/dialog/customassigndialog.cpp

HEADERS += src/client/aux-skills.h \
	src/client/client.h \
	src/client/clientplayer.h \
	src/client/clientstruct.h \
	src/core/card.h \
	src/core/engine.h \
	src/core/general.h \
	src/core/lua-wrapper.h \
	src/core/player.h \
	src/core/settings.h \
	src/core/skill.h \
	src/dialog/cardeditor.h \
	src/dialog/cardoverview.h \
	src/dialog/choosegeneraldialog.h \
	src/dialog/configdialog.h \
	src/dialog/connectiondialog.h \
	src/dialog/distanceviewdialog.h \
	src/dialog/generaloverview.h \
	src/dialog/generalselector.h \
	src/dialog/halldialog.h \
	src/dialog/mainwindow.h \
	src/dialog/packagingeditor.h \
	src/dialog/playercarddialog.h \
	src/dialog/roleassigndialog.h \ 
	src/dialog/scenario-overview.h \
        src/package/exppattern.h \
	src/package/firepackage.h \
	src/package/god.h \
	src/package/joypackage.h \
	src/package/maneuvering.h \
	src/package/mountainpackage.h \
	src/package/nostalgia.h \
	src/package/package.h \
	src/package/sp-package.h \
	src/package/standard-equips.h \
	src/package/standard-skillcards.h \
	src/package/standard.h \
	src/package/thicket.h \
	src/package/wind.h \
	src/package/wisdompackage.h \
	src/package/yitianpackage.h \
	src/package/yjcm-package.h \
        src/package/bgm-package.h \
	src/scenario/boss-mode-scenario.h \
	src/scenario/couple-scenario.h \
	src/scenario/fancheng-scenario.h \
	src/scenario/guandu-scenario.h \
	src/scenario/scenario.h \
	src/scenario/scenerule.h \
	src/scenario/zombie-mode-scenario.h \
	src/server/ai.h \
	src/server/contestdb.h \
	src/server/gamerule.h \
	src/server/room.h \
	src/server/roomthread.h \
	src/server/roomthread1v1.h \
	src/server/roomthread3v3.h \
	src/server/server.h \
	src/server/serverplayer.h \
	src/server/structs.h \
	src/ui/button.h \
	src/ui/cardcontainer.h \
	src/ui/carditem.h \
	src/ui/clientlogbox.h \
	src/ui/dashboard.h \
	src/ui/indicatoritem.h \
	src/ui/photo.h \
	src/ui/pixmap.h \
	src/ui/rolecombobox.h \
	src/ui/roomscene.h \
	src/ui/startscene.h \
	src/ui/window.h \
	src/util/detector.h \
	src/util/nativesocket.h \
	src/util/recorder.h \
	src/util/socket.h \
    src/ui/pixmapanimation.h \
    src/ui/sprite.h \
    src/core/banpair.h \
    src/ui/chatwidget.h \
    src/scenario/miniscenarios.h \
    src/dialog/customassigndialog.h \
	src/core/audio.h
	
FORMS += src/dialog/cardoverview.ui \
	src/dialog/configdialog.ui \
	src/dialog/connectiondialog.ui \
	src/dialog/generaloverview.ui \
	src/dialog/mainwindow.ui 
	
INCLUDEPATH += include/lua
INCLUDEPATH += include
INCLUDEPATH += src/client
INCLUDEPATH += src/core
INCLUDEPATH += src/dialog
INCLUDEPATH += src/package
INCLUDEPATH += src/scenario
INCLUDEPATH += src/server
INCLUDEPATH += src/ui
INCLUDEPATH += src/util

win32{
    RC_FILE += resource/icon.rc
    LIBS += -L. -llua -lm
}

unix:!macx {
    LIBS += -lm -llua
}

macx {
    LIBS += -L. -lm -llua5.1
}

CONFIG(audio){
    DEFINES += AUDIO_SUPPORT
    INCLUDEPATH += include/fmod
    LIBS += -lfmodex
    SOURCES += src/core/audio.cpp
}

CONFIG(joystick){
    DEFINES += JOYSTICK_SUPPORT
    HEADERS += src/ui/joystick.h
    SOURCES += src/ui/joystick.cpp
    win32: LIBS += -lplibjs -lplibul -lwinmm
    unix: LIBS += -lplibjs -lplibul
}

TRANSLATIONS += sanguosha.ts

OTHER_FILES += \
    sanguosha.qss \
    scenarios/acknowledgement/main.qml

