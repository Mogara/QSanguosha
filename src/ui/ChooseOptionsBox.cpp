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
#include "protocol.h"
#include "button.h"
#include "SkinBank.h"

#include <QApplication>

ChooseOptionsBox::ChooseOptionsBox()
    : options_number(0), skill_name(QString())
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
    progress_bar = NULL;
}

void ChooseOptionsBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    //====================
    //||================||
    //||   请选择一项   ||
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
    painter->save();
    painter->setBrush(QBrush(G_COMMON_LAYOUT.m_chooseGeneralBoxBackgroundColor));
    QRectF rect = boundingRect();
    const int x = rect.x();
    const int y = rect.y();
    const int w = rect.width();
    const int h = rect.height();
    painter->drawRect(QRect(x, y, w, h));
    painter->drawRect(QRect(x, y, w, top_dark_bar));
    QString title = QString();
    if (skill_name != "TriggerOrder" && skill_name != "TurnStartShowGeneral") {
        title.append(" ");
        title.append(tr("Please choose:"));
    }
    title.prepend(Sanguosha->translate(skill_name));
    G_COMMON_LAYOUT.m_chooseGeneralBoxTitleFont.paintText(painter, QRect(x, y, w, top_dark_bar), Qt::AlignCenter, title);
    painter->restore();
    painter->setPen(G_COMMON_LAYOUT.m_chooseGeneralBoxBorderColor);
    painter->drawRect(QRect(x + 1, y + 1, w - 2, h - 2));
}

QRectF ChooseOptionsBox::boundingRect() const
{
    const int width = default_button_width + left_blank_width * 2;

    const int height = top_blank_width + options_number * default_button_height + (options_number - 1) * card_bottom_to_split_line + bottom_blank_width;

    return QRectF(0, 0, width, height);
}

void ChooseOptionsBox::chooseGeneral(QStringList options)
{
    //重新绘制背景
    options_number = options.length();
    update();

    buttons.clear();
    foreach (QString option, options) {
        QString title = QString("%1:%2").arg(skill_name).arg(option);
        QString tranlated = Sanguosha->translate(title);
        if (tranlated == title)
            tranlated = Sanguosha->translate(option);
        Button *button = new Button(tranlated, QSizeF(default_button_width, default_button_height), true);
        button->setFlag(QGraphicsItem::ItemIsFocusable);
        button->setObjectName(option);
        buttons << button;
        button->setParentItem(this);
        connect(button, SIGNAL(clicked()), this, SLOT(reply()));
        connect(button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerMakeChoice()));
    }

    setPos(RoomSceneInstance->tableCenterPos() - QPointF(boundingRect().width() / 2, boundingRect().height() / 2));
    show();

    for (int i = 0; i < buttons.length(); ++i) {
        Button *button = buttons.at(i);

        QPointF pos;
        pos.setX(left_blank_width);
        pos.setY(top_blank_width + default_button_height * i + (i - 1) * card_bottom_to_split_line + default_button_height / 2);

        button->setPos(pos);
    }

    if (ServerInfo.OperationTimeout != 0) {
        if (!progress_bar) {
            progress_bar = new QSanCommandProgressBar();
            progress_bar->setMinimumWidth(200);
            progress_bar->setMaximumHeight(12);
            progress_bar->setTimerEnabled(true);
            progress_bar_item = new QGraphicsProxyWidget(this);
            progress_bar_item->setWidget(progress_bar);
            progress_bar_item->setPos(boundingRect().center().x() - progress_bar_item->boundingRect().width() / 2, boundingRect().height() - 30);
            connect(progress_bar, SIGNAL(timedOut()), this, SLOT(reply()));
        }
        progress_bar->setCountdown(QSanProtocol::S_COMMAND_CHOOSE_GENERAL);
        progress_bar->show();
    }
}

void ChooseOptionsBox::reply() {
    if (progress_bar != NULL){
        progress_bar->hide();
        progress_bar->deleteLater();
    }
    clear();
}

void ChooseOptionsBox::clear() {
    foreach(Button *button, buttons)
        button->deleteLater();

    buttons.clear();

    update();

    hide();
}
