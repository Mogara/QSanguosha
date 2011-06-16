#include <QtGui/QApplication>

#include <QTranslator>
#include <QDir>

#include "mainwindow.h"
#include "settings.h"
#include "banpairdialog.h"
#include "server.h"

#ifdef AUDIO_SUPPORT
#ifdef  Q_OS_WIN32
    #include "irrKlang.h"
    irrklang::ISoundEngine *SoundEngine;
#else
    #include <phonon/MediaObject>
    #include <phonon/AudioOutput>
    Phonon::MediaObject *SoundEngine;
    Phonon::AudioOutput *SoundOutput;
#endif
#endif

int main(int argc, char *argv[])
{
    QString dir_name = QDir::current().dirName();
    if(dir_name == "release" || dir_name == "debug")
        QDir::setCurrent("..");

    QApplication a(argc, argv);

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");
    translator.load("sanguosha.qm");

    a.installTranslator(&qt_translator);
    a.installTranslator(&translator);

    Config.init();
    Sanguosha = new Engine;
    BanPair::loadBanPairs();

    if(a.arguments().contains("-server")){
        Server *server = new Server(&a);
        server->listen();
        server->daemonize();

        return a.exec();
    }

#ifdef AUDIO_SUPPORT

#ifdef  Q_OS_WIN32
    SoundEngine = irrklang::createIrrKlangDevice();
    if(SoundEngine)
        SoundEngine->setSoundVolume(Config.Volume);
#else
    SoundEngine = new Phonon::MediaObject(&a);
    SoundOutput = new Phonon::AudioOutput(Phonon::GameCategory, &a);
    Phonon::createPath(SoundEngine, SoundOutput);
#endif

#endif

    MainWindow *main_window = new MainWindow;

    Sanguosha->setParent(main_window);
    main_window->show();

    return a.exec();
}
