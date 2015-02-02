TEMPLATE = app

QT += qml quick

SOURCES += main.cpp

RESOURCES += qml.qrc

INCLUDEPATH += Cardirector/include

win32-msvc2013:LIBS += -L"$$_PRO_FILE_PWD_/Cardirector/lib/win32-msvc2013"

CONFIG(debug, debug|release): LIBS += -lCardirectord
else:LIBS += -lCardirector

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
