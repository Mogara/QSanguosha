TEMPLATE = app

QT += qml quick

SOURCES += main.cpp

RESOURCES += qml.qrc

INCLUDEPATH += Cardirector/include

defineTest(copy) {
    file = $$1
    path = $$2
    win32 {
        system("copy $$replace(file, /, \\) $$replace(path, /, \\)")
    }
    else {
        system("cp $$file $$path")
    }

    return(true)
}

win32-msvc2013 {
    LIBS += -L"$$_PRO_FILE_PWD_/Cardirector/lib/win32-msvc2013"
    CONFIG(debug, debug|release): copy(Cardirector/bin/win32-msvc2013/Cardirectord.dll, $$_PRO_FILE_PWD_/)
    else:copy(Cardirector/bin/win32-msvc2013/Cardirector.dll, $$_PRO_FILE_PWD_/)
}

CONFIG(debug, debug|release): LIBS += -lCardirectord
else:LIBS += -lCardirector

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = $$PWD

# Default rules for deployment.
include(deployment.pri)
