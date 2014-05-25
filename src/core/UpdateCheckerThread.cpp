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
#include "UpdateCheckThread.h"
#include "mainwindow.h"

#include <QNetworkReply>
#include <QRegExp>
#include <QEventLoop>

UpdateCheckerThread::UpdateCheckerThread()
{
}

void UpdateCheckerThread::run() {
    QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
    const QString URL = "https://raw.githubusercontent.com/QSanguosha-Rara/QSanguosha-For-Hegemony/Qt-4.8/info/UpdateInfo";
    QEventLoop loop;
    QNetworkReply *reply = mgr->get(QNetworkRequest(QUrl(URL)));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(terminate()));

    loop.exec();

    while (!reply->atEnd()) {
        QString line = reply->readLine();
        QStringList texts = line.split("|");

        Q_ASSERT(texts.size() == 2);

        const QString key = texts.at(0);
        const QString value = texts.at(1);
        emit storeKeyAndValue(key, value);
    }
    terminate();
}