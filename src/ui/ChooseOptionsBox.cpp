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

#include "ChooseOptionsBox.h"
#include "engine.h"
#include "roomscene.h"
#include "button.h"

ChooseOptionsBox::ChooseOptionsBox()
    : optionsNumber(0), progressBar(NULL)
{
}

void ChooseOptionsBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //====================
    //||================||
    //|| Please Choose: ||
    //||    _______     ||
    //||   |   1   |    ||
    //||    -------     ||
    //||    _______     ||
    //||   |   2   |    ||
    //||    -------     ||
    //||    _______     ||
    //||   |   3   |    ||
    //||    -------     ||
    //====================

    title = QString("%1 %2").arg((Sanguosha->translate(skillName)).arg(tr("Please choose:"));
    GraphicsBox::paint(painter, option, widget);
}

QRectF ChooseOptionsBox::boundingRect() const
{
    const int width = default_button_width + left_blank_width * 2;

    int height = top_blank_width + optionsNumber * default_button_height + (optionsNumber - 1) * interval + bottom_blank_width;

    if (ServerInfo.OperationTimeout != 0)
        height += 12;

    return QRectF(0, 0, width, height);
}

void ChooseOptionsBox::chooseOption(const QStringList &options)
{
    //repaint background
    this->options = options;
    optionsNumber = options.length();
    update();

    foreach (QString option, options) {
        QString title = QString("%1:%2").arg(skillName).arg(option);
        QString tranlated = Sanguosha->translate(title);
        if (tranlated == title)
            tranlated = Sanguosha->translate(option);
        Button *button = new Button(tranlated, QSizeF(default_button_width, default_button_height), true);
        button->setFlag(QGraphicsItem::ItemIsFocusable);
        button->setObjectName(option);
        buttons << button;
        button->setParentItem(this);

        QString original_tooltip = QString(":%1").arg(title);
        QString tooltip = Sanguosha->translate(original_tooltip);
        if (tooltip == original_tooltip) {
            original_tooltip = QString(":%1").arg(option);
            tooltip = Sanguosha->translate(original_tooltip);
        }
        connect(button, SIGNAL(clicked()), this, SLOT(reply()));
        if (tooltip != original_tooltip)
            button->setToolTip(QString("<font color=%1>%2</font>")
                               .arg(Config.SkillDescriptionInToolTipColor.name())
                               .arg(tooltip));
    }

    setPos(RoomSceneInstance->tableCenterPos() - QPointF(boundingRect().width() / 2, boundingRect().height() / 2));
    show();

    for (int i = 0; i < buttons.length(); ++i) {
        Button *button = buttons.at(i);

        QPointF pos;
        pos.setX(left_blank_width);
        pos.setY(top_blank_width + default_button_height * i + (i - 1) * interval + default_button_height / 2);

        button->setPos(pos);
    }

    if (ServerInfo.OperationTimeout != 0) {
        if (!progressBar) {
            progressBar = new QSanCommandProgressBar();
            progressBar->setMaximumWidth(boundingRect().width() - 16);
            progressBar->setMaximumHeight(12);
            progressBar->setTimerEnabled(true);
            progress_bar_item = new QGraphicsProxyWidget(this);
            progress_bar_item->setWidget(progressBar);
            progress_bar_item->setPos(boundingRect().center().x() - progress_bar_item->boundingRect().width() / 2, boundingRect().height() - 20);
            connect(progressBar, SIGNAL(timedOut()), this, SLOT(reply()));
        }
        progressBar->setCountdown(QSanProtocol::S_COMMAND_MULTIPLE_CHOICE);
        progressBar->show();
    }
}

void ChooseOptionsBox::reply()
{
    QString choice = sender()->objectName();
    if (choice.isEmpty())
        choice = options.first();
    ClientInstance->onPlayerMakeChoice(choice);
    clear();
}

void ChooseOptionsBox::clear()
{
    if (progressBar != NULL){
        progressBar->hide();
        progressBar->deleteLater();
        progressBar = NULL;
    }

    foreach(Button *button, buttons)
        button->deleteLater();

    buttons.clear();

    update();

    hide();
}
