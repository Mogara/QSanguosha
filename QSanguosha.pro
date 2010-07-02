# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = QSanguosha
QT += phonon \
    opengl \
    script \
    network
TEMPLATE = app
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/gamescene.cpp \
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
    src/cardclass.cpp \
    src/carditem.cpp \
    src/room.cpp \
    src/generaloverview.cpp \
    src/player.cpp
HEADERS += src/gamescene.h \
    src/mainwindow.h \
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
    src/cardclass.h \
    src/carditem.h \
    src/room.h \
    src/generaloverview.h \
    src/player.h
FORMS += src/mainwindow.ui \
    src/connectiondialog.ui \
    src/generaloverview.ui
OTHER_FILES += scripts/generals.js \
    scripts/cards.js \
    scripts/init.js
RESOURCES += resource/sanguosha.qrc
RC_FILE += resource/icon.rc
TRANSLATIONS += sanguosha.ts
