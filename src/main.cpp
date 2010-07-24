#include <QtGui/QApplication>

#include <QTranslator>
#include <QDir>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setStyle("plastique");

    QString dir_name = QDir::current().dirName();
    if(dir_name == "release" || dir_name == "debug")
        QDir::setCurrent("..");

    QTranslator translator;
    translator.load("sanguosha.qm");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
