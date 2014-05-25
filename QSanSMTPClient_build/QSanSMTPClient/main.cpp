/********************************************************************
 * The SMTP uplorder for QSanguosha-Hegemony is based on
 * Tőkés Attila's project SmtpClient-for-Qt.
 *
 * The SmtpClient for Qt is small library writen for Qt 4
 * (or later) (C++ version) that allows application to send complex
 * emails (plain text, html, attachments, inline files,
 * etc.) using the Simple Mail Transfer Protocol (SMTP).
 *
 * We create an application with Qt GUI module to make it easier to
 * use. The Program is just used in QSanguosha-Hegemony for dump file
 * collection at present. You can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * QSanguosha-Hegemony Team
 * Last Updated By Yanguam Siliagim
 * March 2 2014
 * *****************************************************************/

#include "mainwindow.h"

#include <QApplication>
#include <QTranslator>
#include <QDir>

int main(int argc, char *argv[])
{
    if (argc < 2) return 9527;
    QApplication a(argc, argv);

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");
    translator.load("QSanSMTPClient.qm");

    a.installTranslator(&qt_translator);
    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    qApp->alert(&w);
    return w.askForUploading();
}
