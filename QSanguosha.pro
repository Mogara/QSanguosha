# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = QSanguosha
QT += phonon \
    opengl \
    script
TEMPLATE = app
SOURCES += src/main.cpp \
    src/mainwindow.cpp \
    src/gamescene.cpp \
    src/button.cpp \
    src/settings.cpp \
    src/startscene.cpp \
    src/roomscene.cpp \
    src/photo.cpp \
    src/bottom.cpp \
    src/card.cpp \
    src/pixmap.cpp \
    src/general.cpp
HEADERS += src/gamescene.h \
    src/mainwindow.h \
    src/button.h \
    src/settings.h \
    src/startscene.h \
    src/roomscene.h \
    src/photo.h \
    src/bottom.h \
    src/card.h \
    src/pixmap.h \
    src/general.h
FORMS += src/mainwindow.ui
OTHER_FILES += 
RESOURCES += resource/sanguosha.qrc
RC_FILE += resource/icon.rc
TRANSLATIONS += sanguosha.ts
