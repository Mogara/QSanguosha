#include <QtGui/QApplication>

#include <QTranslator>
#include <QDir>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString dir_name = QDir::current().dirName();
    if(dir_name == "release" || dir_name == "debug")
        QDir::setCurrent("..");

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");
    translator.load("sanguosha.qm");

    a.installTranslator(&qt_translator);
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
