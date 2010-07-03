#include <QtGui/QApplication>

#include <QTranslator>
#include <QDir>
#include <QTextCodec>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString dir_name = QDir::current().dirName();
    if(dir_name == "release" || dir_name == "debug")
        QDir::setCurrent("..");

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB18030"));

    QTranslator translator;
    translator.load("sanguosha.qm");
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
