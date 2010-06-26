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
    src/servingthread.cpp \
    src/connectiondialog.cpp
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
    src/servingthread.h \
    src/connectiondialog.h
FORMS += src/mainwindow.ui \
    src/connectiondialog.ui
OTHER_FILES += 
RESOURCES += resource/sanguosha.qrc
RC_FILE += resource/icon.rc
TRANSLATIONS += sanguosha.ts
