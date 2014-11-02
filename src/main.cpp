/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#if defined(WIN32) && !defined(GPP) && !defined(QT_NO_DEBUG) && !defined(WINRT)
#include <vld/vld.h>
#endif

#include <QCoreApplication>
#include <QApplication>
#include <QTranslator>
#include <QDateTime>
#include <QMessageBox>
#include <QtQuick/QQuickView>

#include "server.h"
#include "settings.h"
#include "engine.h"
#include "audio.h"

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

    if (noGui)
        new QCoreApplication(argc, argv);
    else
        new QApplication(argc, argv);

#ifdef USE_BREAKPAD
    ExceptionHandler eh(L"./dmp", NULL, callback, NULL, ExceptionHandler::HANDLER_ALL);
#endif

#if defined(Q_OS_MAC) && defined(QT_NO_DEBUG)
    QDir::setCurrent(qApp->applicationDirPath());
#endif

#ifdef Q_OS_LINUX
    QDir dir(QString("lua"));
    if (!dir.exists() || !(dir.exists(QString("config.lua")))) {
#ifndef Q_OS_ANDROID
        QDir::setCurrent(qApp->applicationFilePath().replace("games", "share"));
#else
        bool found = false;
        QDir storageDir("/storage");
        QStringList sdcards = storageDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const QString &sdcard, sdcards) {
            QDir root(QString("/storage/%1/Android/data/org.qsanguosha").arg(sdcard));
            if (root.exists("lua/config.lua")) {
                QDir::setCurrent(root.absolutePath());
                found = true;
                break;
            }
        }
        if (!found) {
            QDir root("/sdcard/Android/data/org.qsanguosha");
            QDir::setCurrent(root.absolutePath());
        }
#endif
    }
#endif

    // initialize random seed for later use
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    QTranslator translator;
    translator.load("sanguosha.qm");
    qApp->installTranslator(&translator);

    new Settings;
    Config.init();

    QQuickView mainView;
    mainView.setSource(QUrl::fromLocalFile("ui-script/main.qml"));
    mainView.show();

    QTranslator qt_translator;
    qt_translator.load("qt_zh_CN.qm");
    qApp->installTranslator(&qt_translator);

    new Engine;
    Sanguosha->setParent(&mainView);

    if (qApp->arguments().contains("-server")) {
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if (server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    }

#ifdef AUDIO_SUPPORT
    Audio::init();
#else
    if (!noGui)
        QMessageBox::warning(NULL, QMessageBox::tr("Warning"), QMessageBox::tr("Audio support is disabled when compiled"));
#endif

    foreach(QString arg, qApp->arguments()) {
        if (arg.startsWith("-connect:")) {
            arg.remove("-connect:");
            Config.HostAddress = arg;
            Config.setValue("HostAddress", arg);

            //main_window.startConnection();
            break;
        }
    }

    return qApp->exec();
}
