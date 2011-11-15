#include <QtGui/QApplication>

#include <QCoreApplication>
#include <QTranslator>
#include <QDir>
#include <cstring>
#include <QDateTime>

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

    if(argc > 1 && strcmp(argv[1], "-server") == 0)
        new QCoreApplication(argc, argv);
    else
        new QApplication(argc, argv);

    // initialize random seed for later use
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");
    translator.load("sanguosha.qm");

    qApp->installTranslator(&qt_translator);
    qApp->installTranslator(&translator);

    Config.init();
    Sanguosha = new Engine;
    BanPair::loadBanPairs();


    QFile file("sanguosha.qss");
    if(file.open(QIODevice::ReadOnly)){
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
    }

    if(qApp->arguments().contains("-server")){
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if(server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    }

#ifdef AUDIO_SUPPORT

#ifdef  Q_OS_WIN32
    SoundEngine = irrklang::createIrrKlangDevice();
    if(SoundEngine)
        SoundEngine->setSoundVolume(Config.Volume);
#else
    SoundEngine = new Phonon::MediaObject(qApp);
    SoundOutput = new Phonon::AudioOutput(Phonon::GameCategory, qApp);
    Phonon::createPath(SoundEngine, SoundOutput);
#endif

#endif

    MainWindow *main_window = new MainWindow;

    Sanguosha->setParent(main_window);
    main_window->show();

    return qApp->exec();
}
