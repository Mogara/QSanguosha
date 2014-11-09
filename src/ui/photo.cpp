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

#include "photo.h"
#include "clientplayer.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "client.h"
#include "rolecombobox.h"
#include "skinbank.h"
#include "graphicspixmaphoveritem.h"

#include <QPainter>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMessageBox>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QMenu>
#include <QFile>

#include "pixmapanimation.h"

using namespace QSanProtocol;

// skins that remain to be extracted:
// equips
// mark
// emotions
// hp
// seatNumber
// death logo
// kingdom mask and kingdom icon (decouple from player)
// make layers (drawing order) configurable

Photo::Photo() : PlayerCardContainer() {
    _m_mainFrame = NULL;
    m_player = NULL;
    _m_focusFrame = NULL;
    _m_onlineStatusItem = NULL;
    _m_layout = &G_PHOTO_LAYOUT;
    _m_frameType = S_FRAME_NO_FRAME;
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setTransform(QTransform::fromTranslate(-G_PHOTO_LAYOUT.m_normalWidth / 2, -G_PHOTO_LAYOUT.m_normalHeight / 2), true);
    _m_skillNameItem = new QGraphicsPixmapItem(_m_groupMain);

    emotion_item = new Sprite(_m_groupMain);

    _createControls();
}

Photo::~Photo(){
    if (emotion_item){
        delete emotion_item;
        emotion_item = NULL;
    }
}

void Photo::refresh() {
    PlayerCardContainer::refresh();
    if (!m_player) return;
    QString state_str = m_player->getState();
    if (!state_str.isEmpty() && state_str != "online") {
        QRect rect = G_PHOTO_LAYOUT.m_onlineStatusArea;
        QImage image(rect.size(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.fillRect(QRect(0, 0, rect.width(), rect.height()), G_PHOTO_LAYOUT.m_onlineStatusBgColor);
        G_PHOTO_LAYOUT.m_onlineStatusFont.paintText(&painter, QRect(QPoint(0, 0), rect.size()),
            Qt::AlignCenter,
            Sanguosha->translate(state_str));
        QPixmap pixmap = QPixmap::fromImage(image);
        _paintPixmap(_m_onlineStatusItem, rect, pixmap, _m_groupMain);
        _layBetween(_m_onlineStatusItem, _m_mainFrame, _m_chainIcon);
        if (!_m_onlineStatusItem->isVisible()) _m_onlineStatusItem->show();
    }
    else if (_m_onlineStatusItem != NULL && state_str == "online")
        _m_onlineStatusItem->hide();

}



QRectF Photo::boundingRect() const{
    return QRect(0, 0, G_PHOTO_LAYOUT.m_normalWidth, G_PHOTO_LAYOUT.m_normalHeight);
}

void Photo::repaintAll() {
    resetTransform();
    setTransform(QTransform::fromTranslate(-G_PHOTO_LAYOUT.m_normalWidth / 2, -G_PHOTO_LAYOUT.m_normalHeight / 2), true);
    _paintPixmap(_m_mainFrame, G_PHOTO_LAYOUT.m_mainFrameArea, QSanRoomSkin::S_SKIN_KEY_MAINFRAME);
    setFrame(_m_frameType);
    hideSkillName(); // @todo: currently we don't adjust skillName's position for simplicity,
    // consider repainting it instead of hiding it in the future.
    PlayerCardContainer::repaintAll();
    refresh();
}

void Photo::_adjustComponentZValues() {
    PlayerCardContainer::_adjustComponentZValues();
    _layBetween(_m_mainFrame, _m_faceTurnedIcon, _m_equipRegions[3]);
    _layBetween(emotion_item, _m_secondaryAvatarNameItem, _m_roleComboBox);
    _layBetween(_m_skillNameItem, _m_secondaryAvatarNameItem, _m_roleComboBox);
    _m_progressBarItem->setZValue(_m_groupMain->zValue() + 1);
}

void Photo::setEmotion(const QString &emotion, bool permanent) {
    if (emotion == ".") {
        hideEmotion();
        return;
    }

    QString path = QString("image/system/emotion/%1.png").arg(emotion);
    if (QFile::exists(path)) {
        QPixmap pixmap = QPixmap(path);
        emotion_item->setPixmap(pixmap);
        emotion_item->setPos((G_PHOTO_LAYOUT.m_normalWidth - pixmap.width()) / 2,
            (G_PHOTO_LAYOUT.m_normalHeight - pixmap.height()) / 2);
        _layBetween(emotion_item, _m_chainIcon, _m_roleComboBox);

        QPropertyAnimation *appear = new QPropertyAnimation(emotion_item, "opacity");
        appear->setStartValue(0.0);
        if (permanent) {
            appear->setEndValue(1.0);
            appear->setDuration(500);
        }
        else {
            appear->setKeyValueAt(0.25, 1.0);
            appear->setKeyValueAt(0.75, 1.0);
            appear->setEndValue(0.0);
            appear->setDuration(2000);
        }
        appear->start(QAbstractAnimation::DeleteWhenStopped);
    }
    else {
        PixmapAnimation::GetPixmapAnimation(this, emotion);
    }
}

void Photo::tremble() {
    QPropertyAnimation *vibrate = new QPropertyAnimation(this, "x");
    static qreal offset = 20;

    vibrate->setKeyValueAt(0.5, x() - offset);
    vibrate->setEndValue(x());

    vibrate->setEasingCurve(QEasingCurve::OutInBounce);

    vibrate->start(QAbstractAnimation::DeleteWhenStopped);
}

void Photo::showSkillName(const QString &skill_name) {
    G_PHOTO_LAYOUT.m_skillNameFont.paintText(_m_skillNameItem,
        G_PHOTO_LAYOUT.m_skillNameArea,
        Qt::AlignLeft,
        Sanguosha->translate(skill_name));
    _m_skillNameItem->show();
    QTimer::singleShot(1000, this, SLOT(hideSkillName()));
}

void Photo::hideSkillName() {
    _m_skillNameItem->hide();
}

void Photo::hideEmotion() {
    QPropertyAnimation *disappear = new QPropertyAnimation(emotion_item, "opacity");
    disappear->setStartValue(1.0);
    disappear->setEndValue(0.0);
    disappear->setDuration(500);
    disappear->start(QAbstractAnimation::DeleteWhenStopped);
}

const ClientPlayer *Photo::getPlayer() const{
    return m_player;
}

void Photo::speak(const QString &) {
    //@@todo: complete it
}

void Photo::updateSmallAvatar() {
    updateAvatar();
    if (_m_smallAvatarIcon == NULL) {
        _m_smallAvatarIcon = new GraphicsPixmapHoverItem(this, _getAvatarParent());
        _m_smallAvatarIcon->setTransformationMode(Qt::SmoothTransformation);
    }

    const General *general = NULL;
    if (m_player) general = m_player->getGeneral2();

    if (general != NULL) {
        QPixmap smallAvatarIcon = G_ROOM_SKIN.getGeneralPixmap(general->objectName(),
                                                               QSanRoomSkin::GeneralIconSize(_m_layout->m_smallAvatarSize),
                                                               m_player->getDeputySkinId());
        smallAvatarIcon = paintByMask(smallAvatarIcon);
        QGraphicsPixmapItem *smallAvatarTmp = _m_smallAvatarIcon;
        _paintPixmap(smallAvatarTmp, _m_layout->m_secondaryAvatarArea,
                     smallAvatarIcon, _getAvatarParent());
        _paintPixmap(_m_circleItem, _m_layout->m_circleArea,
            QString(QSanRoomSkin::S_SKIN_KEY_GENERAL_CIRCLE_IMAGE).arg(_m_layout->m_circleImageSize),
            _getAvatarParent());
        _m_secondaryAvatarArea->setToolTip(m_player->getDeputySkillDescription());
        QString name = Sanguosha->translate("&" + general->objectName());
        if (name.startsWith("&"))
            name = Sanguosha->translate(general->objectName());
        _m_layout->m_smallAvatarNameFont.paintText(_m_secondaryAvatarNameItem,
            _m_layout->m_secondaryAvatarNameArea,
            Qt::AlignLeft | Qt::AlignJustify, name);
        _m_smallAvatarIcon->show();
    } else {
        _clearPixmap(_m_smallAvatarIcon);
        _clearPixmap(_m_circleItem);
        _m_layout->m_smallAvatarNameFont.paintText(_m_secondaryAvatarNameItem,
            _m_layout->m_secondaryAvatarNameArea,
            Qt::AlignLeft | Qt::AlignJustify, QString());
        _m_secondaryAvatarArea->setToolTip(QString());
    }
    _adjustComponentZValues();
}

QList<CardItem *> Photo::removeCardItems(const QList<int> &card_ids, Player::Place place) {
    QList<CardItem *> result;
    if (place == Player::PlaceHand || place == Player::PlaceSpecial) {
        result = _createCards(card_ids);
        updateHandcardNum();
    } else if (place == Player::PlaceEquip) {
        result = removeEquips(card_ids);
    } else if (place == Player::PlaceDelayedTrick) {
        result = removeDelayedTricks(card_ids);
    }

    // if it is just one card from equip or judge area, we'd like to keep them
    // to start from the equip/trick icon.
    if (result.size() > 1 || (place != Player::PlaceEquip && place != Player::PlaceDelayedTrick))
        _disperseCards(result, G_PHOTO_LAYOUT.m_cardMoveRegion, Qt::AlignCenter, false, false);

    update();
    return result;
}

bool Photo::_addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo) {
    _disperseCards(card_items, G_PHOTO_LAYOUT.m_cardMoveRegion, Qt::AlignCenter, true, false);
    double homeOpacity = 0.0;
    bool destroy = true;

    Player::Place place = moveInfo.to_place;

    foreach(CardItem *card_item, card_items)
        card_item->setHomeOpacity(homeOpacity);
    if (place == Player::PlaceEquip) {
        addEquips(card_items);
        destroy = false;
    }
    else if (place == Player::PlaceDelayedTrick) {
        addDelayedTricks(card_items);
        destroy = false;
    }
    else if (place == Player::PlaceHand) {
        updateHandcardNum();
    }
    return destroy;
}

void Photo::setFrame(FrameType type) {
    _m_frameType = type;
    if (type == S_FRAME_NO_FRAME) {
        if (_m_focusFrame) {
            if (_m_saveMeIcon && _m_saveMeIcon->isVisible())
                setFrame(S_FRAME_SOS);
            else if (m_player->getPhase() != Player::NotActive)
                setFrame(S_FRAME_PLAYING);
            else
                _m_focusFrame->hide();
        }
    }
    else {
        _paintPixmap(_m_focusFrame, G_PHOTO_LAYOUT.m_focusFrameArea,
            _getPixmap(QSanRoomSkin::S_SKIN_KEY_FOCUS_FRAME, QString::number(type)),
            _m_groupMain);
        _layBetween(_m_focusFrame, _m_avatarArea, _m_mainFrame);
        _m_focusFrame->show();
    }
    update();
}

void Photo::updatePhase() {
    PlayerCardContainer::updatePhase();
    if (m_player->getPhase() != Player::NotActive)
        setFrame(S_FRAME_PLAYING);
    else
        setFrame(S_FRAME_NO_FRAME);
}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

QPropertyAnimation *Photo::initializeBlurEffect(GraphicsPixmapHoverItem *icon)
{
    QGraphicsBlurEffect *effect = new QGraphicsBlurEffect;
    effect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
    effect->setBlurRadius(0);
    icon->setGraphicsEffect(effect);

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "blurRadius");
    animation->setEasingCurve(QEasingCurve::OutInBounce);
    animation->setDuration(2000);
    animation->setStartValue(0);
    animation->setEndValue(5);
    return animation;
}

void Photo::_initializeRemovedEffect()
{
    _blurEffect = new QParallelAnimationGroup(this);
    _blurEffect->addAnimation(initializeBlurEffect(_m_avatarIcon));
    _blurEffect->addAnimation(initializeBlurEffect(_m_smallAvatarIcon));
}

QGraphicsItem *Photo::getMouseClickReceiver() {
    return this;
}

