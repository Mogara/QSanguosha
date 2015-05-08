TEMPLATE = app

QT += qml quick

SOURCES += src/main.cpp

RESOURCES += image.qrc

INCLUDEPATH += Cardirector/include
LIBS += -L"$$_PRO_FILE_PWD_/Cardirector/lib"

defineTest(copy) {
    file = $$1
    path = $$2
    !exists($$file): return(false)
    system("$$QMAKE_COPY $$system_path($$file) $$system_path($$path)")

    return(true)
}

win32 {
    CONFIG(debug, debug|release): copy(Cardirector/bin/Cardirectord.dll, $$_PRO_FILE_PWD_/)
    else:copy(Cardirector/bin/Cardirector.dll, $$_PRO_FILE_PWD_/)

    RC_FILE = icons/icon.rc

    QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
}

macx {
    ICON = icons/icon.icns
}

android {
    QT += androidextras
    ANDROID_EXTRA_LIBS += $$PWD/Cardirector/lib/libCardirector.so
}

LIBS += -l$$qtLibraryTarget(Cardirector)

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = $$PWD

QML_FILES = src/Client/ClientSettings.qml \
            src/Gui/McdSplash.qml \
            src/Gui/Splash.qml \
            src/Gui/StartScene.qml \
            src/main.qml

# Create the resource file
GENERATED_RESOURCE_FILE = qml.qrc

INCLUDED_RESOURCE_FILES = $$QML_FILES

RESOURCE_CONTENT = \
    "<RCC>" \
    "    <qresource prefix=\"/\">"

for(resourcefile, INCLUDED_RESOURCE_FILES) {
    RESOURCE_CONTENT += "        <file>$$resourcefile</file>"
}

RESOURCE_CONTENT += \
    "    </qresource>" \
    "</RCC>"

write_file($$GENERATED_RESOURCE_FILE, RESOURCE_CONTENT)|error("Aborting.")

RESOURCES += $$GENERATED_RESOURCE_FILE

lupdate_only {
    SOURCES += $$QML_FILES
}

TRANSLATIONS += translations/zh_CN.ts

OTHER_FILES += android/AndroidManifest.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

# Default rules for deployment.
include(deployment.pri)
