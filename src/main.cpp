/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#if defined(WIN32) && !defined(GPP) && !defined(QT_NO_DEBUG)
#include <vld/vld.h>
#endif

#include <QFile>
#include <QCoreApplication>
#include <QApplication>
#include <QTranslator>
#include <QDateTime>
#include <QSplashScreen>

#include "server.h"
#include "settings.h"
#include "engine.h"
#include "mainwindow.h"
#include "audio.h"
#include "StyleHelper.h"

#ifndef WINDOWS
#include <QDir>
#endif

#ifdef USE_BREAKPAD
#include <client/windows/handler/exception_handler.h>
#include <QProcess>

using namespace google_breakpad;

static bool callback(const wchar_t *, const wchar_t *id, void *, EXCEPTION_POINTERS *, MDRawAssertionInfo *, bool succeeded) {
    if (succeeded && QFile::exists("QSanSMTPClient.exe")){
        char ID[16000];
        memset(ID, 0, sizeof(ID));
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif
        wcstombs(ID, id, wcslen(id));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
        QProcess *process = new QProcess(qApp);
        QStringList args;
        args << QString(ID) + ".dmp";
        process->start("QSanSMTPClient", args);
    }
    return succeeded;
}
#endif

int main(int argc, char *argv[]) {
    bool noGui = argc > 1 && strcmp(argv[1], "-server") == 0;
    bool noSplash = false;
#ifdef Q_OS_MAC
    noSplash = true;
#endif

    if (noGui)
        new QCoreApplication(argc, argv);
    else
        new QApplication(argc, argv);

    QPixmap raraLogo("image/system/developers/logo.png");
    QSplashScreen splash(raraLogo);
    const int alignment = Qt::AlignBottom | Qt::AlignHCenter;
    if (!noGui || !noSplash) {
        splash.show();
        qApp->processEvents();
    }

#ifdef USE_BREAKPAD
    if (!noGui || !noSplash) {
        splash.showMessage(QSplashScreen::tr("Loading BreakPad..."), alignment, Qt::cyan);
        qApp->processEvents();
    }

    ExceptionHandler eh(L"./dmp", NULL, callback, NULL, ExceptionHandler::HANDLER_ALL);
#endif



#ifdef Q_OS_MAC
#ifdef QT_NO_DEBUG
    if (!noGui || !noSplash) {
        splash.showMessage(QSplashScreen::tr("Setting game path..."), alignment, Qt::cyan);
        qApp->processEvents();
    }
    QDir::setCurrent(qApp->applicationDirPath());
#endif
#endif

#ifdef Q_OS_LINUX
    if (!noGui || !noSplash) {
        splash.showMessage(QSplashScreen::tr("Checking game path..."), alignment, Qt::cyan);
        qApp->processEvents();
    }

    QDir dir(QString("lua"));
    if (dir.exists() && (dir.exists(QString("config.lua")))) {
        // things look good and use current dir
    } else {
        if (!noGui || !noSplash) {
            splash.showMessage(QSplashScreen::tr("Setting game path..."), alignment, Qt::cyan);
            qApp->processEvents();
        }
#ifndef Q_OS_ANDROID
        QDir::setCurrent(qApp->applicationFilePath().replace("games", "share"));
#else
        QDir storageDir("/storage");
        QStringList sdcards = storageDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const QString &sdcard, sdcards) {
            QDir root(QString("/storage/%1/Android/data/org.qsgsrara.qsanguosha").arg(sdcard));
            if (root.exists("lua/config.lua")) {
                QDir::setCurrent(root.absolutePath());
                break;
            }
        }
#endif
    }
#endif

    // initialize random seed for later use
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    // load the main translation file first for we need to translate messages of splash.
    QTranslator translator;
    translator.load("sanguosha.qm");
    qApp->installTranslator(&translator);

    if (!noGui || !noSplash) {
        splash.showMessage(QSplashScreen::tr("Loading translation..."), alignment, Qt::cyan);
        qApp->processEvents();
    }

    QTranslator qt_translator;
    qt_translator.load("qt_zh_CN.qm");
    qApp->installTranslator(&qt_translator);

    if (!noGui || !noSplash) {
        splash.showMessage(QSplashScreen::tr("Initializing game engine..."), alignment, Qt::cyan);
        qApp->processEvents();
    }
    new Settings;
    Sanguosha = new Engine;

    if (!noGui || !noSplash) {
        splash.showMessage(QSplashScreen::tr("Loading user's configurations..."), alignment, Qt::cyan);
        qApp->processEvents();
    }
    Config.init();
    qApp->setFont(Config.AppFont);

    if (qApp->arguments().contains("-server")) {
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if (server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    }

    if (!noSplash) {
        splash.showMessage(QSplashScreen::tr("Loading style sheet..."), alignment, Qt::cyan);
        qApp->processEvents();
    }

    QFile file("style-sheet/sanguosha.qss");
    QString styleSheet;
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        styleSheet = stream.readAll();
    }
    qApp->setStyleSheet(styleSheet + StyleHelper::styleSheetOfTooltip());

#ifdef AUDIO_SUPPORT
    if (!noSplash) {
        splash.showMessage(QSplashScreen::tr("Initializing audio module..."), alignment, Qt::cyan);
        qApp->processEvents();
    }

    Audio::init();
#else
    QMessageBox::warning(this, QMessageBox::tr("Warning"), QMessageBox::tr("Audio support is disabled when compiled"));
#endif

    if (!noSplash) {
        splash.showMessage(QSplashScreen::tr("Loading main window..."), alignment, Qt::cyan);
        qApp->processEvents();
    }

    MainWindow main_window;

    Sanguosha->setParent(&main_window);
    main_window.show();
    if (!noSplash)
        splash.finish(&main_window);

    foreach(QString arg, qApp->arguments()) {
        if (arg.startsWith("-connect:")) {
            arg.remove("-connect:");
            Config.HostAddress = arg;
            Config.setValue("HostAddress", arg);

            main_window.startConnection();
            break;
        }
    }

    return qApp->exec();
}

