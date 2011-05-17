#include <QtGui/QApplication>

#include <QTranslator>
#include <QDir>

#include "mainwindow.h"
#include "settings.h"
#include "banpairdialog.h"

#ifdef AUDIO_SUPPORT

#include "irrKlang.h"

irrklang::ISoundEngine *SoundEngine;

#endif

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

#ifdef AUDIO_SUPPORT

    SoundEngine = irrklang::createIrrKlangDevice();
    if(SoundEngine)
        SoundEngine->setSoundVolume(Config.Volume);

#endif

    Sanguosha = new Engine;

    BanPair::loadBanPairs();
    MainWindow *main_window = new MainWindow;

    Sanguosha->setParent(main_window);
    main_window->show();    

    return a.exec();
}
