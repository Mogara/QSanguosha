# -------------------------------------------------
# Project created by QtCreator 2010-06-13T04:26:52
# -------------------------------------------------
TARGET = QSanguosha
QT += network
!winrt:QT += declarative
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app
CONFIG += audio

android:DEFINES += "\"getlocaledecpoint()='.'\""

CONFIG += lua

SOURCES += \
    src/main.cpp \
    src/client/aux-skills.cpp \
    src/client/client.cpp \
    src/client/clientplayer.cpp \
    src/client/clientstruct.cpp \
    src/core/banpair.cpp \
    src/core/card.cpp \
    src/core/engine.cpp \
    src/core/general.cpp \
    src/core/lua-wrapper.cpp \
    src/core/player.cpp \
    src/core/protocol.cpp \
    src/core/record-analysis.cpp \
    src/core/RoomState.cpp \
    src/core/settings.cpp \
    src/core/skill.cpp \
    src/core/structs.cpp \
    src/core/util.cpp \
    src/core/WrappedCard.cpp \
    src/core/version.cpp \
    src/core/json.cpp \
    src/dialog/AboutUs.cpp \
    src/dialog/cardbutton.cpp \
    src/dialog/cardeditor.cpp \
    src/dialog/cardoverview.cpp \
    src/dialog/configdialog.cpp \
    src/dialog/connectiondialog.cpp \
    src/dialog/customassigndialog.cpp \
    src/dialog/distanceviewdialog.cpp \
    src/dialog/FreeChooseDialog.cpp \
    src/dialog/generaloverview.cpp \
    src/dialog/mainwindow.cpp \
    src/dialog/playercarddialog.cpp \
    src/dialog/rule-summary.cpp \
    src/dialog/UpdateChecker.cpp \
    src/dialog/FlatDialog.cpp \
    src/dialog/UdpDetectorDialog.cpp \
    src/dialog/AvatarModel.cpp \
    src/dialog/GeneralModel.cpp \
    src/package/exppattern.cpp \
    src/package/formation.cpp \
    src/package/jiange-defense.cpp \
    src/package/momentum.cpp \
    src/package/standard.cpp \
    src/package/standard-basics.cpp \
    src/package/standard-equips.cpp \
    src/package/standard-tricks.cpp \
    src/package/standard-qun-generals.cpp \
    src/package/standard-shu-generals.cpp \
    src/package/standard-wu-generals.cpp \
    src/package/standard-wei-generals.cpp \
    src/package/standard-package.cpp \
    src/package/strategic-advantage.cpp \
    src/package/package.cpp \
    src/scenario/miniscenarios.cpp \
    src/scenario/scenario.cpp \
    src/scenario/scenerule.cpp \
    src/scenario/jiange-defense-scenario.cpp \
    src/server/ai.cpp \
    src/server/gamerule.cpp \
    src/server/generalselector.cpp \
    src/server/room.cpp \
    src/server/roomthread.cpp \
    src/server/server.cpp \
    src/server/serverplayer.cpp \
    src/ui/button.cpp \
    src/ui/cardcontainer.cpp \
    src/ui/carditem.cpp \
    src/ui/chatwidget.cpp \
    src/ui/choosegeneralbox.cpp \
    src/ui/clientlogbox.cpp \
    src/ui/dashboard.cpp \
    src/ui/GenericCardContainerUI.cpp \
    src/ui/indicatoritem.cpp \
    src/ui/magatamasItem.cpp \
    src/ui/photo.cpp \
    src/ui/pixmapanimation.cpp \
    src/ui/qsanbutton.cpp \
    src/ui/QSanSelectableItem.cpp \
    src/ui/rolecombobox.cpp \
    src/ui/roomscene.cpp \
    src/ui/SkinBank.cpp \
    src/ui/sprite.cpp \
    src/ui/startscene.cpp \
    src/ui/TablePile.cpp \
    src/ui/TimedProgressBar.cpp \
    src/ui/uiUtils.cpp \
    src/ui/window.cpp \
    src/ui/ChooseOptionsBox.cpp \
    src/ui/ChooseTriggerOrderBox.cpp \
    src/ui/GraphicsBox.cpp \
    src/ui/GuanxingBox.cpp \
    src/ui/Title.cpp \
    src/ui/BubbleChatBox.cpp \
    src/ui/StyleHelper.cpp \
    src/ui/PlayerCardBox.cpp \
    src/ui/GraphicsPixmapHoverItem.cpp \
    src/ui/HeroSkinContainer.cpp \
    src/ui/SkinItem.cpp \
    src/util/detector.cpp \
    src/util/nativesocket.cpp \
    src/util/recorder.cpp \
    swig/sanguosha_wrap.cxx

HEADERS += \
    src/client/aux-skills.h \
    src/client/client.h \
    src/client/clientplayer.h \
    src/client/clientstruct.h \
    src/core/audio.h \
    src/core/banpair.h \
    src/core/card.h \
    src/core/compiler-specific.h \
    src/core/engine.h \
    src/core/general.h \
    src/core/lua-wrapper.h \
    src/core/namespace.h \
    src/core/player.h \
    src/core/protocol.h \
    src/core/record-analysis.h \
    src/core/RoomState.h \
    src/core/settings.h \
    src/core/skill.h \
    src/core/structs.h \
    src/core/util.h \
    src/core/WrappedCard.h \
    src/core/version.h \
    src/core/json.h \
    src/dialog/AboutUs.h \
    src/dialog/cardbutton.h \
    src/dialog/cardeditor.h \
    src/dialog/cardoverview.h \
    src/dialog/configdialog.h \
    src/dialog/connectiondialog.h \
    src/dialog/customassigndialog.h \
    src/dialog/distanceviewdialog.h \
    src/dialog/FreeChooseDialog.h \
    src/dialog/generaloverview.h \
    src/dialog/mainwindow.h \
    src/dialog/playercarddialog.h \
    src/dialog/rule-summary.h \
    src/dialog/UpdateChecker.h \
    src/dialog/FlatDialog.h \
    src/dialog/UdpDetectorDialog.h \
    src/dialog/AvatarModel.h \
    src/dialog/GeneralModel.h \
    src/package/exppattern.h \
    src/package/formation.h \
    src/package/jiange-defense.h \
    src/package/momentum.h \
    src/package/package.h \
    src/package/standard.h \
    src/package/standard-basics.h \
    src/package/standard-equips.h \
    src/package/standard-tricks.h \
    src/package/standard-qun-generals.h \
    src/package/standard-shu-generals.h \
    src/package/standard-wu-generals.h \
    src/package/standard-wei-generals.h \
    src/package/strategic-advantage.h \
    src/package/standard-package.h \
    src/scenario/miniscenarios.h \
    src/scenario/scenario.h \
    src/scenario/scenerule.h \
    src/scenario/jiange-defense-scenario.h \
    src/server/ai.h \
    src/server/gamerule.h \
    src/server/generalselector.h \
    src/server/room.h \
    src/server/roomthread.h \
    src/server/server.h \
    src/server/serverplayer.h \
    src/ui/button.h \
    src/ui/cardcontainer.h \
    src/ui/carditem.h \
    src/ui/chatwidget.h \
    src/ui/choosegeneralbox.h \
    src/ui/clientlogbox.h \
    src/ui/dashboard.h \
    src/ui/GenericCardContainerUI.h \
    src/ui/indicatoritem.h \
    src/ui/magatamasItem.h \
    src/ui/photo.h \
    src/ui/pixmapanimation.h \
    src/ui/qsanbutton.h \
    src/ui/QSanSelectableItem.h \
    src/ui/rolecombobox.h \
    src/ui/roomscene.h \
    src/ui/SkinBank.h \
    src/ui/sprite.h \
    src/ui/startscene.h \
    src/ui/TablePile.h \
    src/ui/TimedProgressBar.h \
    src/ui/uiUtils.h \
    src/ui/window.h \
    src/ui/ChooseOptionsBox.h \
    src/ui/ChooseTriggerOrderBox.h \
    src/ui/GraphicsBox.h \
    src/ui/GuanxingBox.h \
    src/ui/Title.h \
    src/ui/BubbleChatBox.h \
    src/ui/StyleHelper.h \
    src/ui/PlayerCardBox.h \
    src/ui/GraphicsPixmapHoverItem.h \
    src/ui/HeroSkinContainer.h \
    src/ui/SkinItem.h \
    src/util/detector.h \
    src/util/nativesocket.h \
    src/util/recorder.h \
    src/util/socket.h

FORMS += \
    src/dialog/cardoverview.ui \
    src/dialog/configdialog.ui \
    src/dialog/connectiondialog.ui \
    src/dialog/generaloverview.ui \
    src/dialog/mainwindow.ui

INCLUDEPATH += include
INCLUDEPATH += src/client
INCLUDEPATH += src/core
INCLUDEPATH += src/dialog
INCLUDEPATH += src/package
INCLUDEPATH += src/scenario
INCLUDEPATH += src/server
INCLUDEPATH += src/ui
INCLUDEPATH += src/util

lessThan(QT_MAJOR_VERSION, 5){
    SOURCES += src/jsoncpp/src/json_writer.cpp \
        src/jsoncpp/src/json_valueiterator.inl \
        src/jsoncpp/src/json_value.cpp \
        src/jsoncpp/src/json_reader.cpp \
        src/jsoncpp/src/json_internalmap.inl \
        src/jsoncpp/src/json_internalarray.inl

    HEADERS += src/jsoncpp/src/json_tool.h \
        src/jsoncpp/src/json_batchallocator.h \
        src/jsoncpp/include/json/writer.h \
        src/jsoncpp/include/json/value.h \
        src/jsoncpp/include/json/reader.h \
        src/jsoncpp/include/json/json.h \
        src/jsoncpp/include/json/forwards.h \
        src/jsoncpp/include/json/features.h \
        src/jsoncpp/include/json/config.h \
        src/jsoncpp/include/json/autolink.h \
        src/jsoncpp/include/json/assertions.h

    INCLUDEPATH += src/jsoncpp/include
}

win32{
    RC_FILE += resource/icon.rc
}

macx{
    ICON = resource/icon/sgs.icns
}

LIBS += -L.
win32-msvc*{
    DEFINES += _CRT_SECURE_NO_WARNINGS
    !contains(QMAKE_HOST.arch, x86_64) {
        DEFINES += WIN32
        LIBS += -L"$$_PRO_FILE_PWD_/lib/win/x86"
    } else {
        DEFINES += WIN64
        LIBS += -L"$$_PRO_FILE_PWD_/lib/win/x64"
    }
    CONFIG(debug, debug|release) {
        INCLUDEPATH += include/vld
    } else {
        QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
        DEFINES += USE_BREAKPAD

        SOURCES += src/breakpad/client/windows/crash_generation/client_info.cc \
            src/breakpad/client/windows/crash_generation/crash_generation_client.cc \
            src/breakpad/client/windows/crash_generation/crash_generation_server.cc \
            src/breakpad/client/windows/crash_generation/minidump_generator.cc \
            src/breakpad/client/windows/handler/exception_handler.cc \
            src/breakpad/common/windows/guid_string.cc

        HEADERS += src/breakpad/client/windows/crash_generation/client_info.h \
            src/breakpad/client/windows/crash_generation/crash_generation_client.h \
            src/breakpad/client/windows/crash_generation/crash_generation_server.h \
            src/breakpad/client/windows/crash_generation/minidump_generator.h \
            src/breakpad/client/windows/handler/exception_handler.h \
            src/breakpad/common/windows/guid_string.h

        INCLUDEPATH += src/breakpad
        INCLUDEPATH += src/breakpad/client/windows
    }
}
win32-g++{
    DEFINES += WIN32
    LIBS += -L"$$_PRO_FILE_PWD_/lib/win/MinGW"
    DEFINES += GPP
}
winrt{
    DEFINES += WINRT
    LIBS += -L"$$_PRO_FILE_PWD_/lib/winrt"
}
macx{
    DEFINES += MAC
    LIBS += -L"$$_PRO_FILE_PWD_/lib/mac/lib"
}
ios{
    DEFINES += IOS
    CONFIG(iphonesimulator){
        LIBS += -L"$$_PRO_FILE_PWD_/lib/ios/simulator/lib"
    }
    else {
        LIBS += -L"$$_PRO_FILE_PWD_/lib/ios/device/lib"
    }
}
linux{
    android{
        DEFINES += ANDROID
        ANDROID_LIBPATH = $$_PRO_FILE_PWD_/lib/android/$$ANDROID_ARCHITECTURE/lib
        LIBS += -L"$$ANDROID_LIBPATH"
    }
    else {
        DEFINES += LINUX
        !contains(QMAKE_HOST.arch, x86_64) {
            LIBS += -L"$$_PRO_FILE_PWD_/lib/linux/x86"
        }
        else {
            LIBS += -L"$$_PRO_FILE_PWD_/lib/linux/x64"
        }
    }
}

CONFIG(audio){
    DEFINES += AUDIO_SUPPORT
    INCLUDEPATH += include/fmod
    CONFIG(debug, debug|release): LIBS += -lfmodexL
    else:LIBS += -lfmodex
    SOURCES += src/core/audio.cpp

    android{
        CONFIG(debug, debug|release):ANDROID_EXTRA_LIBS += $$ANDROID_LIBPATH/libfmodexL.so
        else:ANDROID_EXTRA_LIBS += $$ANDROID_LIBPATH/libfmodex.so
    }
}

CONFIG(lua){
    SOURCES += \
        src/lua/lzio.c \
        src/lua/lvm.c \
        src/lua/lundump.c \
        src/lua/ltm.c \
        src/lua/ltablib.c \
        src/lua/ltable.c \
        src/lua/lstrlib.c \
        src/lua/lstring.c \
        src/lua/lstate.c \
        src/lua/lparser.c \
        src/lua/loslib.c \
        src/lua/lopcodes.c \
        src/lua/lobject.c \
        src/lua/loadlib.c \
        src/lua/lmem.c \
        src/lua/lmathlib.c \
        src/lua/llex.c \
        src/lua/liolib.c \
        src/lua/linit.c \
        src/lua/lgc.c \
        src/lua/lfunc.c \
        src/lua/ldump.c \
        src/lua/ldo.c \
        src/lua/ldebug.c \
        src/lua/ldblib.c \
        src/lua/lctype.c \
        src/lua/lcorolib.c \
        src/lua/lcode.c \
        src/lua/lbitlib.c \
        src/lua/lbaselib.c \
        src/lua/lauxlib.c \
        src/lua/lapi.c
    HEADERS += \
        src/lua/lzio.h \
        src/lua/lvm.h \
        src/lua/lundump.h \
        src/lua/lualib.h \
        src/lua/luaconf.h \
        src/lua/lua.hpp \
        src/lua/lua.h \
        src/lua/ltm.h \
        src/lua/ltable.h \
        src/lua/lstring.h \
        src/lua/lstate.h \
        src/lua/lparser.h \
        src/lua/lopcodes.h \
        src/lua/lobject.h \
        src/lua/lmem.h \
        src/lua/llimits.h \
        src/lua/llex.h \
        src/lua/lgc.h \
        src/lua/lfunc.h \
        src/lua/ldo.h \
        src/lua/ldebug.h \
        src/lua/lctype.h \
        src/lua/lcode.h \
        src/lua/lauxlib.h \
        src/lua/lapi.h
    INCLUDEPATH += src/lua
}

CONFIG(opengl){
    QT += opengl
    DEFINES += USING_OPENGL
}

TRANSLATIONS += builds/sanguosha.ts

!exists($$PWD/sanguosha.qm) {
    system("lrelease builds/sanguosha.ts -qm $$PWD/sanguosha.qm")
}

OTHER_FILES += \
    sanguosha.qss \
    ui-script/animation.qml \
    resource/android/AndroidManifest.xml \
    builds/sanguosha.ts

LIBS += -lfreetype

INCLUDEPATH += $$PWD/include/freetype
DEPENDPATH += $$PWD/include/freetype

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/resource/android
