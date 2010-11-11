#include <QtGui/QApplication>

#include <QTranslator>
#include <QDir>

#include "mainwindow.h"
#include "ircdetector.h"
#include "settings.h"
#include "audiere.h"
#include "banpairdialog.h"

audiere::AudioDevicePtr Device = NULL;

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

    Config.init();

    QWidget *widget;
    if(a.arguments().contains("-detect")){
        Sanguosha = new Engine;
        widget = new IrcDetectorDialog;
    }else{
#ifdef Q_OS_WIN32
        Device = audiere::OpenDevice("winmm");
#else
        Device = audiere::OpenDevice();
#endif
        Sanguosha = new Engine;

        BanPair::loadBanPairs();
        widget = new MainWindow;
    }

    Sanguosha->setParent(widget);
    widget->show();

    return a.exec();
}
