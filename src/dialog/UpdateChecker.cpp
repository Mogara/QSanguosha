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

#include "UpdateChecker.h"
#include "engine.h"
#include "version.h"

#include <QLabel>
#include <QTextEdit>
#include <QFormLayout>
#include <QFile>

UpdateChecker::UpdateChecker()
{
    state_label = new QLabel(this);
    address_label = new QLabel(this);
    page = new QTextEdit(this);
    page->setObjectName("whatsnew");
    page->setReadOnly(true);

    QFormLayout *layout = new QFormLayout;
    layout->addRow(state_label);
    layout->addRow(tr("Download Address"), address_label);
    layout->addRow(tr("What's New"), page);

    setLayout(layout);
}

void UpdateChecker::fill(UpdateInfoStruct info)
{
    QString state;
    bool lastest = false;
    if (info.version_number > Sanguosha->getVersionNumber()) {
        QString postfix = " : " + info.version_number;
        if (info.is_patch)
            state = tr("New Patch Available") + postfix;
        else
            state = tr("New Client Available") + postfix;
    }
    else {
        state = tr("Lastest Version Already");
        lastest = true;
    }
    state_label->setText(state);

    if (lastest)
        address_label->setText(tr("Lastest Version Already"));
    else {
        address_label->setOpenExternalLinks(true);
        address_label->setText(QString("<a href='%1' style = \"color:#0072c1; \">%1</a> <br/>").arg(info.address));
    }

    QFile file("info.html");
    if (!lastest && file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        QString content = stream.readAll();
        page->setHtml(content);
    }
    else
        page->setText(tr("Lastest Version Already"));
}

void UpdateChecker::clear()
{
    state_label->clear();
    address_label->clear();
    page->clear();
}
