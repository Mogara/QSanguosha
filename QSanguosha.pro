TEMPLATE = app

QT += qml quick

SOURCES += main.cpp

RESOURCES += qml.qrc

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
}

CONFIG(debug, debug|release): LIBS += -lCardirectord
else:LIBS += -lCardirector

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = $$PWD

# Default rules for deployment.
include(deployment.pri)
