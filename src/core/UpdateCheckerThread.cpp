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

#include "UpdateCheckerThread.h"
#include "mainwindow.h"

#include <QNetworkReply>
#include <QRegExp>
#include <QEventLoop>
#include <QTextCodec>
#include <QFile>

UpdateCheckerThread::UpdateCheckerThread()
{
}

void UpdateCheckerThread::run() {
#if defined(WIN32) && (defined(VS2010) || defined(VS2012) || defined(VS2013))
    QNetworkAccessManager *mgr = new QNetworkAccessManager;
    //temp url for test
    QString URL = "http://ver.qsanguosha.org/test/UpdateInfo";
    QString URL2 = "http://ver.qsanguosha.org/test/whatsnew.html";
    QEventLoop loop;
    QNetworkReply *reply = mgr->get(QNetworkRequest(QUrl(URL)));
    QNetworkReply *reply2 = mgr->get(QNetworkRequest(QUrl(URL2)));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));

    //@to-do: terminated() is removed from QThread
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    connect(this, SIGNAL(terminated()), reply, SLOT(deleteLater()));
    connect(this, SIGNAL(terminated()), reply2, SLOT(deleteLater()));
    connect(this, SIGNAL(terminated()), mgr, SLOT(deleteLater()));
#else
    connect(this, SIGNAL(finished()), reply, SLOT(deleteLater()));
    connect(this, SIGNAL(finished()), reply2, SLOT(deleteLater()));
    connect(this, SIGNAL(finished()), mgr, SLOT(deleteLater()));
#endif

    loop.exec();

    bool is_comment = false;

    while (!reply->atEnd()) {
        QString line = reply->readLine();
        line.replace('\n', "");

        //simple comment support
        if (line.startsWith("//")) continue;
        if (!is_comment && line.startsWith("/*"))
            is_comment = true;
        if (is_comment) {
            line.trimmed();
            if (line.contains("*/"))
                //I wanna use QString::endsWith here, but the mothod always returns false.
                //@@todo:Refine It Later
                is_comment = false;
            continue;
        }

        QStringList texts = line.split("|", QString::SkipEmptyParts);

        Q_ASSERT(texts.size() == 2);

        QString key = texts.at(0);
        QString value = texts.at(1);
        emit storeKeyAndValue(key, value);
    }
    QString FILE_NAME = "info.html";
    QFile file(FILE_NAME);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        qDebug() << "Cannot open the file: " << FILE_NAME;
        return;
    }
    QByteArray codeContent = reply2->readAll();
    file.write(codeContent);
    file.close();
#endif
}
