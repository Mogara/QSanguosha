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

#include "rule-summary.h"
#include "engine.h"
#include "stylehelper.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QPushButton>

RuleSummary::RuleSummary(QWidget *parent)
    : FlatDialog(parent)
{
    setWindowTitle(tr("Rule Summary"));
    resize(853, 600);

    list = new QListWidget;
    list->setMinimumWidth(90);
    list->setMaximumWidth(100);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(list);
    vLayout->addWidget(closeButton);

    content_box = new QTextEdit;
    content_box->setReadOnly(true);
    content_box->setProperty("description", true);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(content_box);
    hLayout->addLayout(vLayout);

    layout->addLayout(hLayout);

    QStringList names = Sanguosha->getModScenarioNames();
    for(int i = 0; i < names.size(); ++ i) {
        QString fileName = QString("rule/%1.html").arg(names.at(i));
        if (!QFile::exists(fileName))
            names.removeAt(i);
    }

    names << "hegemony" << "rule1-card" << "rule2-wording" << "rule3-extras";
    foreach(QString name, names) {
        QString text = Sanguosha->translate(name);
        QListWidgetItem *item = new QListWidgetItem(text, list);
        item->setData(Qt::UserRole, name);
    }

    connect(list, SIGNAL(currentRowChanged(int)), this, SLOT(loadContent(int)));

    if (!names.isEmpty())
        loadContent(0);

    const QString style = StyleHelper::styleSheetOfScrollBar();
    list->verticalScrollBar()->setStyleSheet(style);
    content_box->verticalScrollBar()->setStyleSheet(style);
}

void RuleSummary::loadContent(int row) {
    QString name = list->item(row)->data(Qt::UserRole).toString();
    QString filename = QString("rule/%1.html").arg(name);
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString content = stream.readAll();
        content_box->setHtml(content);
    }
}
