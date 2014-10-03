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

#include "choosetriggerorderbox.h"
#include "engine.h"
#include "button.h"
#include "skinbank.h"
#include "client.h"
#include "clientplayer.h"
#include "timedprogressbar.h"

#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QGraphicsProxyWidget>

static qreal initialOpacity = 0.8;
static int optionButtonHeight = 40;
static QSize generalButtonSize;

static int getSkinId(const ClientPlayer *player, const QString &generalName)
{
    if (player->getGeneralName() == generalName)
        return player->getHeadSkinId();
    else
        return player->getDeputySkinId();

    return 0;
}

static int getSkinId(const QString &playerName, const QString &generalName)
{
    const ClientPlayer *player = ClientInstance->getPlayer(playerName);
    return getSkinId(player, generalName);
}

TriggerOptionButton::TriggerOptionButton(QGraphicsObject *parent, const QString &player, const QString &skill, const int width)
    : QGraphicsObject(parent), skillName(skill), playerName(player), width(width)
{
    const Skill *sk = Sanguosha->getSkill(skill);
    if (sk)
        setToolTip(sk->getDescription());
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemIsFocusable);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    setAcceptsHoverEvents(true);
#else
    setAcceptHoverEvents(true);
#endif

    setOpacity(initialOpacity);
}

QString TriggerOptionButton::getGeneralNameBySkill() const
{
    QString generalName;
    const ClientPlayer *player = ClientInstance->getPlayer(playerName);
    if (skillName == "GameRule_AskForArraySummon") {
        foreach(const Skill *skill, player->getVisibleSkillList()) {
            if (!skill->inherits("BattleArraySkill")) continue;
            if (player->inHeadSkills(skill))
                generalName = player->getGeneralName();
            else
                generalName = player->getGeneral2Name();
        }
    } else {
        if (player->inHeadSkills(skillName))
            generalName = player->getGeneralName();
        else
            generalName = player->getGeneral2Name();
    }
    return generalName;
}

void TriggerOptionButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    painter->save();
    painter->setBrush(Qt::black);
    painter->setPen(Sanguosha->getKingdomColor(Self->getGeneral()->getKingdom()));
    QRectF rect = boundingRect();
    painter->drawRoundedRect(rect, 5, 5);
    painter->restore();

    const QString generalName = getGeneralNameBySkill();

    QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY, getSkinId(playerName, generalName));
    pixmap = pixmap.scaledToHeight(optionButtonHeight, Qt::SmoothTransformation);
    QRect pixmapRect(QPoint(0, (rect.height() - pixmap.height()) / 2), pixmap.size());
    painter->setBrush(pixmap);
    painter->drawRoundedRect(pixmapRect, 5, 5);

    QRect textArea(optionButtonHeight, 0, width - optionButtonHeight,
                   optionButtonHeight);
    G_COMMON_LAYOUT.optionButtonText.paintText(painter, textArea,
                                               Qt::AlignCenter,
                                               Sanguosha->translate(skillName));
}

QRectF TriggerOptionButton::boundingRect() const {
    return QRectF(0, 0, width, optionButtonHeight);
}

void TriggerOptionButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void TriggerOptionButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
    emit clicked();
}

void TriggerOptionButton::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setEndValue(1.0);
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void TriggerOptionButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setEndValue(initialOpacity);
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

GeneralButton::GeneralButton(QGraphicsObject *parent, const QString &general, const bool isHead)
    : QGraphicsObject(parent), generalName(general), isHead(isHead)
{
    setToolTip(Sanguosha->getGeneral(general)->getSkillDescription(true));
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlag(ItemIsFocusable);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    setAcceptsHoverEvents(true);
#else
    setAcceptHoverEvents(true);
#endif

    setOpacity(initialOpacity);
}

void GeneralButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::HighQualityAntialiasing);
    QPixmap generalImage = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE, getSkinId(Self, generalName));
    generalImage = generalImage.scaled(generalButtonSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    painter->setBrush(generalImage);
    painter->drawRoundedRect(boundingRect(), 5, 5, Qt::RelativeSize);

    const General *general = Sanguosha->getGeneral(generalName);
    Q_ASSERT(general);

    QPixmap nameBg = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK, general->getKingdom());
    painter->drawPixmap(0, 5, nameBg);

    if (Self->getGeneral() == general || Self->getGeneral2() == general) {
        QString key = (Self->getGeneral() == general) ? QSanRoomSkin::S_SKIN_KEY_HEAD_ICON : QSanRoomSkin::S_SKIN_KEY_DEPUTY_ICON;
        QPixmap positionIcon = G_ROOM_SKIN.getPixmap(key);
        painter->drawPixmap(G_COMMON_LAYOUT.generalButtonPositionIconRegion, positionIcon);
    }

    QString name = Sanguosha->translate("&" + general->objectName());
    if (name.startsWith("&"))
        name = Sanguosha->translate(general->objectName());
    G_DASHBOARD_LAYOUT.m_avatarNameFont.paintText(painter,
    G_COMMON_LAYOUT.generalButtonNameRegion,
    Qt::AlignLeft | Qt::AlignVCenter | Qt::AlignJustify, name);
}

QRectF GeneralButton::boundingRect() const
{
    return QRectF(QPoint(0, 0), generalButtonSize);
}

void GeneralButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void GeneralButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
    emit clicked();
}

void GeneralButton::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setEndValue(1.0);
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void GeneralButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
    QPropertyAnimation *animation = new QPropertyAnimation(this, "opacity");
    animation->setEndValue(initialOpacity);
    animation->setDuration(100);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

ChooseTriggerOrderBox::ChooseTriggerOrderBox()
    : optional(true), cancel(new Button(tr("cancel"), 0.6, true)), progressBar(NULL)
{
    cancel->hide();
    cancel->setParentItem(this);
    cancel->setObjectName("cancel");
    connect(cancel, SIGNAL(clicked()), this, SLOT(reply()));

    generalButtonSize = G_ROOM_SKIN.getGeneralPixmap("caocao", QSanRoomSkin::S_GENERAL_ICON_SIZE_LARGE).size() * 0.6;
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

void ChooseTriggerOrderBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    title = Sanguosha->translate(reason);
    GraphicsBox::paint(painter, option, widget);
}

QRectF ChooseTriggerOrderBox::boundingRect() const
{
    const int generalNum = getGeneralNum();
    int width = generalButtonSize.width() + left_blank_width * 2;
    if (generalNum == 2)
        width += generalButtonSize.width() + interval;

    int height = top_blank_width
            + (options.size() - generalNum) * optionButtonHeight
            + (options.size() - generalNum - 1) * interval
            + bottom_blank_width;

    if (ServerInfo.OperationTimeout != 0)
        height += 12;

    if (generalNum > 0)
        height += generalButtonSize.height() + interval;

    if (optional)
        height += cancel->boundingRect().height() + interval;

    return QRectF(0, 0, width, height);
}

void ChooseTriggerOrderBox::chooseOption(const QString &reason, const QStringList &options, const bool optional)
{
    this->reason = reason;
    this->options = options;
    this->optional = optional;
    prepareGeometryChange();

    const int generalCount = getGeneralNum();

    int width = generalButtonSize.width();
    int generalHeight = 0;

    switch (generalCount) {
    case 2: {
        GeneralButton *head = new GeneralButton(this, Self->getGeneral()->objectName(), true);
        head->setObjectName(QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShowHead"));
        generalButtons << head;

        const int generalTop = top_blank_width
                + (options.size() - generalCount) * optionButtonHeight
                + (options.size() - generalCount) * interval;
        head->setPos(left_blank_width, generalTop);

        GeneralButton *deputy = new GeneralButton(this, Self->getGeneral2()->objectName(), true);
        deputy->setObjectName(QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShowDeputy"));
        generalButtons << deputy;
        deputy->setPos(head->pos().x() + head->boundingRect().width() + interval,
                       generalTop);

        width = deputy->pos().x() - head->pos().x() + deputy->boundingRect().width();
        generalHeight = head->boundingRect().height();
        break;
    }
    case 1: {
        const bool isHead = options.contains(QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShowHead"));
        const QString general = isHead ? Self->getGeneralName() : Self->getGeneral2Name();
        GeneralButton *generalButton = new GeneralButton(this, general, true);
        QString objectName = QString("%1:%2").arg(Self->objectName()).arg("GameRule_AskForGeneralShow");
        objectName.append(isHead ? "Head" : "Deputy");
        generalButton->setObjectName(objectName);
        generalButtons << generalButton;
        const int generalTop = top_blank_width
                + (options.size() - generalCount) * optionButtonHeight
                + (options.size() - generalCount) * interval;
        generalButton->setPos(left_blank_width, generalTop);

        generalHeight = generalButton->boundingRect().height();
        break;
    }
    default:
        break;
    }

    foreach (const QString &option, options) {
        QStringList pair = option.split(":");
        if (pair.last().startsWith("GameRule_AskForGeneralShow"))
            continue;

        TriggerOptionButton *button = new TriggerOptionButton(this, pair.first(), pair.last(), width);
        button->setObjectName(option);
        optionButtons << button;
    }

    moveToCenter();
    show();

    int y = top_blank_width;
    foreach (TriggerOptionButton *button, optionButtons) {
        QPointF pos;
        pos.setX(left_blank_width);
        pos.setY(y);

        button->setPos(pos);
        connect(button, SIGNAL(clicked()), this, SLOT(reply()));
        y += button->boundingRect().height() + interval;
    }

    foreach (GeneralButton *button, generalButtons) {
        connect(button, SIGNAL(clicked()), this, SLOT(reply()));
    }

    if (optional) {
        cancel->setPos((boundingRect().width() - cancel->boundingRect().width()) / 2,
                       y + generalHeight + interval);
        cancel->show();
    }


    if (ServerInfo.OperationTimeout != 0) {
        if (!progressBar) {
            progressBar = new QSanCommandProgressBar;
            progressBar->setMaximumWidth(boundingRect().width() - 16);
            progressBar->setMaximumHeight(12);
            progressBar->setTimerEnabled(true);
            progress_bar_item = new QGraphicsProxyWidget(this);
            progress_bar_item->setWidget(progressBar);
            progress_bar_item->setPos(boundingRect().center().x() - progress_bar_item->boundingRect().width() / 2, boundingRect().height() - 20);
            connect(progressBar, SIGNAL(timedOut()), this, SLOT(reply()));
        }
        progressBar->setCountdown(QSanProtocol::S_COMMAND_TRIGGER_ORDER);
        progressBar->show();
    }
}

void ChooseTriggerOrderBox::clear()
{
    if (progressBar != NULL){
        progressBar->hide();
        progressBar->deleteLater();
        progressBar = NULL;
    }

    foreach(TriggerOptionButton *button, optionButtons)
        button->deleteLater();

    foreach(GeneralButton *button, generalButtons)
        button->deleteLater();

    optionButtons.clear();
    generalButtons.clear();

    cancel->hide();

    disappear();
}

void ChooseTriggerOrderBox::reply()
{
    QString choice = sender()->objectName();
    if (choice.isEmpty()) {
        if (optional)
            choice = "cancel";
        else
            choice = options.first();
    }
    ClientInstance->onPlayerChooseTriggerOrder(choice);
    clear();
}
