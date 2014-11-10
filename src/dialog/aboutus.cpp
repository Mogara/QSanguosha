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

#include "aboutus.h"
#include "engine.h"
#include "stylehelper.h"

#include <QListWidget>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QPushButton>

AboutUsDialog::AboutUsDialog(QWidget *parent)
    : FlatDialog(parent)
{
    setWindowTitle(tr("About Us"));
    lua_State *L = Sanguosha->getLuaState();
    int width = GetValueFromLuaState(L, "about_us", "width").toInt();
    int height = GetValueFromLuaState(L, "about_us", "height").toInt();
    resize(width, height);

    setStyleSheet("QToolTip{ border: none; background: white; }");

    list = new QListWidget;
    list->setMaximumWidth(150);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, &QPushButton::clicked, this, &AboutUsDialog::reject);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(list);
    vLayout->addWidget(closeButton);

    content_box = new QTextBrowser;
    content_box->setOpenExternalLinks(true);
    content_box->setProperty("description", true);

    QHBoxLayout *hLayout = new QHBoxLayout;

    hLayout->addWidget(content_box);
    hLayout->addLayout(vLayout);

    layout->addLayout(hLayout);

    QStringList developers = GetValueFromLuaState(L, "about_us", "developers").toStringList();

    foreach(QString name, developers) {
        QListWidgetItem *item = new QListWidgetItem(Sanguosha->translate(name), list);
        item->setData(Qt::UserRole, name);
    }

    connect(list, &QListWidget::currentRowChanged, this, &AboutUsDialog::loadContent);

    if (!developers.isEmpty())
        loadContent(0);

    const QString style = StyleHelper::styleSheetOfScrollBar();
    list->verticalScrollBar()->setStyleSheet(style);
    content_box->verticalScrollBar()->setStyleSheet(style);
}

void AboutUsDialog::loadContent(int row) {
    QString name = list->item(row)->data(Qt::UserRole).toString();
    lua_State *L = Sanguosha->getLuaState();
    QString page = GetValueFromLuaState(L, "about_us", name.toLatin1().constData()).toString();
    content_box->setHtml(page);
}
