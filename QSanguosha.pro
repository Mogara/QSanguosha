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
    win32 {
        system("copy $$system_path($$file) $$system_path($$path)")
    }
    else {
        system("cp $$file $$path")
    }

    return(true)
}

win32 {
    CONFIG(debug, debug|release): copy(Cardirector/bin/Cardirectord.dll, $$_PRO_FILE_PWD_/)
    else:copy(Cardirector/bin/Cardirector.dll, $$_PRO_FILE_PWD_/)

    RC_FILE = icons/icon.rc
}

macx {
    ICON = icons/icon.icns
}

CONFIG(debug, debug|release): LIBS += -lCardirectord
else:LIBS += -lCardirector

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = $$PWD

QML_FILES = src/Client/ClientSettings.qml \
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

# Default rules for deployment.
include(deployment.pri)
