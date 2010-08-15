# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = QSanguosha
QT += phonon \
    network
TEMPLATE = app
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/button.cpp \
    src/settings.cpp \
    src/startscene.cpp \
    src/roomscene.cpp \
    src/photo.cpp \
    src/dashboard.cpp \
    src/card.cpp \
    src/pixmap.cpp \
    src/general.cpp \
    src/server.cpp \
    src/engine.cpp \
    src/connectiondialog.cpp \
    src/client.cpp \
    src/carditem.cpp \
    src/room.cpp \
    src/generaloverview.cpp \
    src/player.cpp \
    src/skill.cpp \
    src/optionbutton.cpp \
    src/cardoverview.cpp \
    src/serverplayer.cpp \
    src/clientplayer.cpp \
    src/standard-cards.cpp \
    src/standard-generals.cpp \
    src/standard.cpp \
    src/standard-skillcards.cpp \
    src/cardpattern.cpp \
    src/gamerule.cpp \
    src/nullificationdialog.cpp \
    src/playercarddialog.cpp \
    src/magatamawidget.cpp
HEADERS += src/mainwindow.h \
    src/button.h \
    src/settings.h \
    src/startscene.h \
    src/roomscene.h \
    src/photo.h \
    src/dashboard.h \
    src/card.h \
    src/pixmap.h \
    src/general.h \
    src/server.h \
    src/engine.h \
    src/connectiondialog.h \
    src/client.h \
    src/carditem.h \
    src/room.h \
    src/generaloverview.h \
    src/player.h \
    src/skill.h \
    src/optionbutton.h \
    src/cardoverview.h \
    src/serverplayer.h \
    src/clientplayer.h \
    src/standard.h \
    src/package.h \
    src/cardpattern.h \
    src/gamerule.h \
    src/nullificationdialog.h \
    src/playercarddialog.h \
    src/magatamawidget.h
FORMS += src/mainwindow.ui \
    src/connectiondialog.ui \
    src/generaloverview.ui \
    src/cardoverview.ui
RESOURCES += resource/images/sanguosha.qrc
RC_FILE += resource/icon.rc
TRANSLATIONS += sanguosha.ts
