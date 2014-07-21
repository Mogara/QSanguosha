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

#include "ChooseTriggerOrderBox.h"
#include "engine.h"
#include "roomscene.h"
#include "protocol.h"
#include "button.h"
#include "SkinBank.h"

#include <QApplication>

TriggerOptionButton::TriggerOptionButton(QGraphicsObject *parent, const QString &general, const QString &skill, const int width)
    : QGraphicsObject(parent), skillName(skill), generalName(general), width(width)
{
}

void TriggerOptionButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->save();
    painter->setBrush(Sanguosha->getKingdomColor(Self->getGeneral()->getKingdom()));
    painter->setPen(Qt::yellow);
    QRectF rect = boundingRect();
    painter->drawRoundedRect(rect, 50, 10, Qt::RelativeSize);
    painter->restore();

    QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY);
    QRect pixmapRect(QPoint(4, (rect.height() - pixmap.height()) / 2), pixmap.size());
    painter->setBrush(QBrush(pixmap));
    painter->drawRoundedRect(pixmapRect, 20, 20, Qt::RelativeSize);

    QRect textArea(QPoint(pixmap.width() + 4, 0), rect.bottomRight().toPoint());
    G_COMMON_LAYOUT.optionButtonText.paintText(painter, textArea, Qt::AlignCenter, skillName);
}

QRectF TriggerOptionButton::boundingRect() const {
    return QRectF(0, 0, width, width / 2);
}

GeneralButton::GeneralButton(QGraphicsObject *parent, const QString &general, const bool isHead)
    : QGraphicsObject(parent), generalName(general), isHead(isHead)
{
}

void GeneralButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    QPixmap generalImg = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::S_GENERAL_ICON_SIZE_DASHBOARD_PRIMARY);
    painter->setBrush(generalImg);
    painter->drawRoundedRect(boundingRect(), 5, 10, Qt::RelativeSize);

    const General *general = Sanguosha->getGeneral(generalName);
    Q_ASSERT(general);

    QPixmap nameBg = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK, general->getKingdom());
    painter->drawPixmap(0, 5, nameBg);

    if (Self->getGeneral() == general || Self->getGeneral2() == general) {
        QString key = (Self->getGeneral() == general) ? QSanRoomSkin::S_SKIN_KEY_HEAD_ICON : QSanRoomSkin::S_SKIN_KEY_DEPUTY_ICON;
        QPixmap positionIcon = G_ROOM_SKIN.getPixmap(key, QSanRoomSkin::S_SKIN_KEY_DASHBOARD);
        painter->drawPixmap(2, 5, positionIcon);
    }

    QString name = Sanguosha->translate("&" + general->objectName());
    if (name.startsWith("&"))
        name = Sanguosha->translate(general->objectName());
    G_DASHBOARD_LAYOUT.m_avatarNameFont.paintText(painter,
    QRect(10, 5, nameBg.width() - 10, nameBg.height()),
    Qt::AlignLeft | Qt::AlignJustify, name);
}

QRectF GeneralButton::boundingRect() const
{
    static QSize size = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::S_GENERAL_ICON_SIZE_DASHBOARD_PRIMARY).size();
    return QRectF(QPoint(0, 0), size);
}

ChooseTriggerOrderBox::ChooseTriggerOrderBox()
    : optional(true), progressBar(NULL)
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
}

int ChooseTriggerOrderBox::getGeneralNum() const
{
    if (options.isEmpty())
        return 0;

    int count = 0;
    if (options.contains(QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShowHead")))
        ++ count;
    if (options.contains(QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShowDeputy")))
        ++ count;

    return count;
}

void ChooseTriggerOrderBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();
    painter->setBrush(QBrush(G_COMMON_LAYOUT.m_chooseGeneralBoxBackgroundColor));
    QRectF rect = boundingRect();
    const int x = rect.x();
    const int y = rect.y();
    const int w = rect.width();
    const int h = rect.height();
    painter->drawRect(QRect(x, y, w, h));
    painter->drawRect(QRect(x, y, w, top_dark_bar));
    G_COMMON_LAYOUT.m_chooseGeneralBoxTitleFont.paintText(painter, QRect(x, y, w, top_dark_bar), Qt::AlignCenter | Qt::AlignJustify, Sanguosha->translate(reason));
    painter->restore();
    painter->setPen(G_COMMON_LAYOUT.m_chooseGeneralBoxBorderColor);
    painter->drawRect(QRect(x + 1, y + 1, w - 2, h - 2));
}

QRectF ChooseTriggerOrderBox::boundingRect() const
{
    const int generalNum = getGeneralNum();
    static const QSize generalSize = G_ROOM_SKIN.getGeneralPixmap(Self->getAvatarGeneral()->objectName(),
                                                           QSanRoomSkin::S_GENERAL_ICON_SIZE_DASHBOARD_PRIMARY).size();
    int width = generalSize.width() + left_blank_width * 2;
    if (generalNum == 2)
        width += generalSize.width() + interval;

    int height = top_blank_width
            + (options.size() - generalNum) * default_button_height
            + (options.size() - generalNum - 1) * interval
            + bottom_blank_width;

    if (ServerInfo.OperationTimeout != 0)
        height += 12;

    if (generalNum > 0)
        height += generalSize.height() + interval;

    if (optional)
        height += cancel->boundingRect().height() + interval;

    return QRectF(0, 0, width, height);
}

void ChooseTriggerOrderBox::chooseOption(const QString &reason, const QStringList &options, const bool optional)
{
    this->reason = reason;
    this->options = options;
    this->optional = optional;
    update();

    const int generalCount = getGeneralNum();
    const int generalTop = top_blank_width
            + (options.size() - generalCount) * default_button_height
            + (options.size() - generalCount) * interval;

    switch (generalCount) {
    case 2: {
        GeneralButton *head = new GeneralButton(this, Self->getGeneral()->objectName(), true);
        head->setObjectName(QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShowHead"));
        generalButtons << head;
        head->setPos(left_blank_width, generalTop);

        GeneralButton *deputy = new GeneralButton(this, Self->getGeneral()->objectName(), true);
        deputy->setObjectName(QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShowDeputy"));
        generalButtons << deputy;
        deputy->setPos(head->boundingRect().right() + interval, generalTop);
        break;
    }
    case 1: {
        const QString general = options.contains(QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShowHead")) ? Self->getGeneralName() :
                                                                                                                                    Self->getGeneral2Name();
        GeneralButton *generalButton = new GeneralButton(this, general, true);
        generalButtons << generalButton;
        generalButton->setPos(left_blank_width, generalTop);
        break;
    }
    default:
        break;
    }

    foreach (const QString &option, options) {

    }

    /*int z = 100;
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

        QString original_tooltip = QString(":%1").arg(title);
        QString tooltip = Sanguosha->translate(original_tooltip);
        if (tooltip == original_tooltip) {
            original_tooltip = QString(":%1").arg(option);
            tooltip = Sanguosha->translate(original_tooltip);
        }
        connect(button, SIGNAL(clicked()), this, SLOT(reply()));
        connect(button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerMakeChoice()));
        button->setZValue(--z);
        if (tooltip != original_tooltip) {
            //button->setToolTip(QString("<font color=%1>%2</font>").arg(Config.SkillDescriptionInToolTipColor.name()).arg(tooltip));
            ToolTipBox *tip_box = new ToolTipBox(tooltip);
            tip_box->hide();
            tip_box->setParentItem(button);
            tip_box->setZValue(10000);
            connect(button, SIGNAL(hover_entered()), tip_box, SLOT(showToolTip()));
            connect(button, SIGNAL(hover_left()), tip_box, SLOT(hideToolTip()));
        }
    }*/

    setPos(RoomSceneInstance->tableCenterPos() - QPointF(boundingRect().width() / 2, boundingRect().height() / 2));
    show();

    for (int i = 0; i < optionButtons.length(); ++i) {
        TriggerOptionButton *button = optionButtons.at(i);

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

void ChooseTriggerOrderBox::reply()
{
    if (progressBar != NULL){
        progressBar->hide();
        progressBar->deleteLater();
    }
    clear();
}

void ChooseTriggerOrderBox::clear()
{
    foreach(TriggerOptionButton *button, optionButtons)
        button->deleteLater();

    optionButtons.clear();

    hide();
}
