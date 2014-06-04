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

#include "AboutUs.h"
#include "engine.h"

#include <QListWidget>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>

AboutUsDialog::AboutUsDialog(QWidget *parent)
: QDialog(parent)
{
    setWindowTitle(tr("About Us"));
    resize(800, 600);

    setStyleSheet("QToolTip{ border: 0px solid; background: white; }");

    list = new QListWidget;
    list->setMaximumWidth(150);

    content_box = new QTextBrowser;
    content_box->setOpenExternalLinks(true);
    content_box->setProperty("description", true);

    QHBoxLayout *layout = new QHBoxLayout;

    layout->addWidget(content_box);
    layout->addWidget(list);

    setLayout(layout);

    QStringList developers = GetConfigFromLuaState(Sanguosha->getLuaState(), "developers").toStringList();
    developers.prepend(tr("QSanguosha-Hegemony-V2"));

    foreach(QString name, developers) {
        QListWidgetItem *item = new QListWidgetItem(name, list);
        item->setData(Qt::UserRole, name);
    }

    connect(list, SIGNAL(currentRowChanged(int)), this, SLOT(loadContent(int)));

    if (!developers.isEmpty())
        loadContent(0);
}

void AboutUsDialog::loadContent(int row) {
    QString name = list->item(row)->data(Qt::UserRole).toString();
    QString filename = QString("developers/%1.htm").arg(name);
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString content = stream.readAll();
        content_box->setHtml(content);
    }
}