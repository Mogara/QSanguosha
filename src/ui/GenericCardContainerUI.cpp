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

#include "GenericCardContainerUI.h"
#include "engine.h"
#include "standard.h"
#include "clientplayer.h"
#include "roomscene.h"
#include "GraphicsPixmapHoverItem.h"

#include <QPropertyAnimation>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsColorizeEffect>
#include <QPushButton>
#include <QTextDocument>
#include <QMenu>

using namespace QSanProtocol;

QList<CardItem *> GenericCardContainer::cloneCardItems(QList<int> card_ids) {
    return _createCards(card_ids);
}

QList<CardItem *> GenericCardContainer::_createCards(QList<int> card_ids) {
    QList<CardItem *> result;
    foreach(int card_id, card_ids) {
        CardItem *item = _createCard(card_id);
        result.append(item);
    }
    return result;
}

CardItem *GenericCardContainer::_createCard(int card_id) {
    const Card *card = Sanguosha->getCard(card_id);
    CardItem *item = new CardItem(card);
    item->setOpacity(0.0);
    item->setParentItem(this);
    return item;
}

void GenericCardContainer::_destroyCard() {
    CardItem *card = (CardItem *)sender();
    card->setVisible(false);
    card->deleteLater();
}

bool GenericCardContainer::_horizontalPosLessThan(const CardItem *card1, const CardItem *card2) {
    return (card1->x() < card2->x());
}

void GenericCardContainer::_disperseCards(QList<CardItem *> &cards, QRectF fillRegion,
    Qt::Alignment align, bool useHomePos, bool keepOrder) {
    int numCards = cards.size();
    if (numCards == 0) return;
    if (!keepOrder) qSort(cards.begin(), cards.end(), GenericCardContainer::_horizontalPosLessThan);
    double maxWidth = fillRegion.width();
    int cardWidth = G_COMMON_LAYOUT.m_cardNormalWidth;
    double step = qMin((double)cardWidth, (maxWidth - cardWidth) / (numCards - 1));
    align &= Qt::AlignHorizontal_Mask;
    for (int i = 0; i < numCards; i++) {
        CardItem *card = cards[i];
        double newX = 0;
        if (align == Qt::AlignHCenter)
            newX = fillRegion.center().x() + step * (i - (numCards - 1) / 2.0);
        else if (align == Qt::AlignLeft)
            newX = fillRegion.left() + step * i + card->boundingRect().width() / 2.0;
        else if (align == Qt::AlignRight)
            newX = fillRegion.right() + step * (i - numCards) + card->boundingRect().width() / 2.0;
        else
            continue;
        QPointF newPos = QPointF(newX, fillRegion.center().y());
        if (useHomePos)
            card->setHomePos(newPos);
        else
            card->setPos(newPos);
        card->setZValue(_m_highestZ++);
    }
}

void GenericCardContainer::onAnimationFinished() {
}

void GenericCardContainer::_doUpdate() {
    update();
}

void GenericCardContainer::_playMoveCardsAnimation(QList<CardItem *> &cards, bool destroyCards) {
    QParallelAnimationGroup *animation = new QParallelAnimationGroup(this);
    foreach(CardItem *card_item, cards) {
        if (destroyCards)
            connect(card_item, SIGNAL(movement_animation_finished()), this, SLOT(_destroyCard()));
        animation->addAnimation(card_item->getGoBackAnimation(true));
    }

    connect(animation, SIGNAL(finished()), this, SLOT(_doUpdate()));
    connect(animation, SIGNAL(finished()), this, SLOT(onAnimationFinished()));
    animation->start();
}

void GenericCardContainer::addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo) {
    foreach(CardItem *card_item, card_items) {
        card_item->setPos(mapFromScene(card_item->scenePos()));
        card_item->setParentItem(this);
    }
    bool destroy = _addCardItems(card_items, moveInfo);
    _playMoveCardsAnimation(card_items, destroy);
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QString &key) {
    _paintPixmap(item, rect, _getPixmap(key));
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect,
    const QString &key, QGraphicsItem *parent) {
    _paintPixmap(item, rect, _getPixmap(key), parent);
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QPixmap &pixmap) {
    _paintPixmap(item, rect, pixmap, _m_groupMain);
}

QPixmap PlayerCardContainer::_getPixmap(const QString &key, const QString &sArg) {
    Q_ASSERT(key.contains("%1"));
    if (key.contains("%2")) {
        QString rKey = key.arg(getResourceKeyName()).arg(sArg);

        if (G_ROOM_SKIN.isImageKeyDefined(rKey))
            return G_ROOM_SKIN.getPixmap(rKey); // first try "%1key%2 = ...", %1 = "photo", %2 = sArg

        rKey = key.arg(getResourceKeyName());
        return G_ROOM_SKIN.getPixmap(rKey, sArg); // then try "%1key = ..."
    }
    else {
        return G_ROOM_SKIN.getPixmap(key, sArg); // finally, try "key = ..."
    }
}

QPixmap PlayerCardContainer::_getPixmap(const QString &key) {
    if (key.contains("%1") && G_ROOM_SKIN.isImageKeyDefined(key.arg(getResourceKeyName())))
        return G_ROOM_SKIN.getPixmap(key.arg(getResourceKeyName()));
    else return G_ROOM_SKIN.getPixmap(key);

}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect,
    const QPixmap &pixmap, QGraphicsItem *parent) {
    if (item == NULL) {
        item = new QGraphicsPixmapItem(parent);
        item->setTransformationMode(Qt::SmoothTransformation);
    }
    item->setPos(rect.x(), rect.y());
    if (pixmap.size() == rect.size())
        item->setPixmap(pixmap);
    else
        item->setPixmap(pixmap.scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    item->setParentItem(parent);
}

void PlayerCardContainer::_clearPixmap(QGraphicsPixmapItem *pixmap) {
    QPixmap dummy;
    if (pixmap == NULL) return;
    pixmap->setPixmap(dummy);
    pixmap->hide();
}

void PlayerCardContainer::hideProgressBar() {
    _m_progressBar->hide();
}

void PlayerCardContainer::showProgressBar(Countdown countdown) {
    _m_progressBar->setCountdown(countdown);
    _m_progressBar->show();
}

QPixmap PlayerCardContainer::getHeadAvatarIcon(const QString &generalName) {
    const int avatarSize = m_player->getGeneral2() ? _m_layout->m_primaryAvatarSize : _m_layout->m_avatarSize;
    return G_ROOM_SKIN.getGeneralPixmap(generalName,
                                        (QSanRoomSkin::GeneralIconSize)avatarSize,
                                        m_player->getHeadSkinId());
}

QPixmap PlayerCardContainer::getDeputyAvatarIcon(const QString &generalName)
{
    const int avatarSize = _m_layout->m_smallAvatarSize;
    return G_ROOM_SKIN.getGeneralPixmap(generalName,
                                        (QSanRoomSkin::GeneralIconSize)avatarSize,
                                        m_player->getDeputySkinId());
}

void PlayerCardContainer::updateAvatar() {
    if (_m_avatarIcon == NULL) {
        _m_avatarIcon = new GraphicsPixmapHoverItem(this, _getAvatarParent());
        _m_avatarIcon->setTransformationMode(Qt::SmoothTransformation);
    }

    const General *general = NULL;
    if (m_player) {
        general = m_player->getAvatarGeneral();
        IQSanComponentSkin::QSanShadowTextFont font = _m_layout->m_screenNameFont;
        if (m_player->screenName() == tr("Moxuanyanyun"))
            font.m_color = Qt::red;

        font.paintText(_m_screenNameItem, _m_layout->m_screenNameArea,
                       Qt::AlignCenter, m_player->screenName());
    } else {
        _m_layout->m_screenNameFont.paintText(_m_screenNameItem, _m_layout->m_screenNameArea,
                                              Qt::AlignCenter, QString());
    }

    QGraphicsPixmapItem *avatarIconTmp = _m_avatarIcon;
    if (general != NULL) {
        _m_avatarArea->setToolTip(m_player->getHeadSkillDescription());
        QString name = general->objectName();
        QPixmap avatarIcon = getHeadAvatarIcon(name);
        QRect area = _m_layout->m_avatarArea;
        _paintPixmap(avatarIconTmp, area, avatarIcon, _getAvatarParent());
        // this is just avatar general, perhaps game has not started yet.
        if (m_player->getGeneral() != NULL) {
            QString kingdom = m_player->getKingdom();
            _paintPixmap(_m_kingdomColorMaskIcon, _m_layout->m_kingdomMaskArea,
                G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK, kingdom), this->_getAvatarParent());
            _paintPixmap(_m_handCardBg, _m_layout->m_handCardArea,
                _getPixmap(QSanRoomSkin::S_SKIN_KEY_HANDCARDNUM, kingdom), this->_getAvatarParent());
            QString name = Sanguosha->translate("&" + general->objectName());
            if (name.startsWith("&"))
                name = Sanguosha->translate(general->objectName());
            _m_layout->m_avatarNameFont.paintText(_m_avatarNameItem,
                _m_layout->m_avatarNameArea,
                Qt::AlignLeft | Qt::AlignJustify, name);
        } else {
            _paintPixmap(_m_handCardBg, _m_layout->m_handCardArea,
                _getPixmap(QSanRoomSkin::S_SKIN_KEY_HANDCARDNUM, QSanRoomSkin::S_SKIN_KEY_DEFAULT_SECOND),
                _getAvatarParent());
        }
    } else {
        _paintPixmap(avatarIconTmp, _m_layout->m_avatarArea,
            QSanRoomSkin::S_SKIN_KEY_BLANK_GENERAL, _getAvatarParent());
        _clearPixmap(_m_kingdomColorMaskIcon);
        _clearPixmap(_m_kingdomIcon);
        _paintPixmap(_m_handCardBg, _m_layout->m_handCardArea,
            _getPixmap(QSanRoomSkin::S_SKIN_KEY_HANDCARDNUM, QSanRoomSkin::S_SKIN_KEY_DEFAULT_SECOND),
            _getAvatarParent());
        _m_avatarArea->setToolTip(QString());
    }
    _m_avatarIcon->show();
    _adjustComponentZValues();
}

QPixmap PlayerCardContainer::paintByMask(QPixmap &source) {
    QPixmap tmp = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_GENERAL_CIRCLE_MASK, QString::number(_m_layout->m_circleImageSize));
    if (tmp.height() <= 1 && tmp.width() <= 1) return source;
    QPainter p(&tmp);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.drawPixmap(0, 0, _m_layout->m_secondaryAvatarArea.width(), _m_layout->m_secondaryAvatarArea.height(), source);
    return tmp;
}

bool PlayerCardContainer::canBeSelected()
{
    QGraphicsItem *item1 = getMouseClickReceiver();
    QGraphicsItem *item2 = getMouseClickReceiver2();
    return (item1 != NULL || item2 != NULL) && isEnabled() && (flags() & QGraphicsItem::ItemIsSelectable);
}

const ClientPlayer *PlayerCardContainer::getPlayer() const {
    if (m_player) return m_player;
    return NULL;
}

void PlayerCardContainer::updateSmallAvatar() {
    if (_m_smallAvatarIcon == NULL) {
        _m_smallAvatarIcon = new GraphicsPixmapHoverItem(this, _getAvatarParent());
        _m_smallAvatarIcon->setTransformationMode(Qt::SmoothTransformation);
    }

    const General *general = NULL;
    if (m_player) general = m_player->getGeneral2();

    QGraphicsPixmapItem *smallAvatarIconTmp = _m_smallAvatarIcon;
    if (general != NULL) {
        _m_secondaryAvatarArea->setToolTip(m_player->getDeputySkillDescription());
        QString name = general->objectName();
        QPixmap avatarIcon = getHeadAvatarIcon(name);
        QRect area = _m_layout->m_secondaryAvatarArea;
        _paintPixmap(smallAvatarIconTmp, area, avatarIcon, _getAvatarParent());
        QString kingdom = m_player->getKingdom();
        _paintPixmap(_m_kingdomColorMaskIcon2, _m_layout->m_kingdomMaskArea2,
            G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK, kingdom), this->_getAvatarParent());
        QString show_name = Sanguosha->translate("&" + name);
        if (show_name.startsWith("&"))
            show_name = Sanguosha->translate(name);
        _m_layout->m_smallAvatarNameFont.paintText(_m_secondaryAvatarNameItem,
            _m_layout->m_secondaryAvatarNameArea,
            Qt::AlignLeft | Qt::AlignJustify, show_name);
    } else {
        _paintPixmap(smallAvatarIconTmp, _m_layout->m_secondaryAvatarArea,
                     QSanRoomSkin::S_SKIN_KEY_BLANK_GENERAL, _getAvatarParent());
        _clearPixmap(_m_kingdomColorMaskIcon2);
        _clearPixmap(_m_kingdomIcon);
        _m_secondaryAvatarArea->setToolTip(QString());
    }
    _m_smallAvatarIcon->show();
    _adjustComponentZValues();
}

void PlayerCardContainer::updatePhase() {
    if (!m_player || !m_player->isAlive())
        _clearPixmap(_m_phaseIcon);
    else if (m_player->getPhase() != Player::NotActive) {
        if (m_player->getPhase() == Player::PhaseNone)
            return;
        int index = static_cast<int>(m_player->getPhase());
        QRect phaseArea = _m_layout->m_phaseArea.getTranslatedRect(_getPhaseParent()->boundingRect().toRect());
        _paintPixmap(_m_phaseIcon, phaseArea,
            _getPixmap(QSanRoomSkin::S_SKIN_KEY_PHASE, QString::number(index)),
            _getPhaseParent());
        _m_phaseIcon->show();
    }
    else {
        if (_m_progressBar) _m_progressBar->hide();
        if (_m_phaseIcon) _m_phaseIcon->hide();
    }
}

void PlayerCardContainer::updateHp() {
    Q_ASSERT(_m_hpBox && _m_saveMeIcon && m_player);
    _m_hpBox->setHp(m_player->getHp());
    _m_hpBox->setMaxHp(m_player->getMaxHp());
    _m_hpBox->update();
    if (m_player->getHp() > 0 || m_player->getMaxHp() == 0)
        _m_saveMeIcon->setVisible(false);
}

void PlayerCardContainer::updatePile(const QString &pile_name) {
    ClientPlayer *player = (ClientPlayer *)sender();
    if (!player)
        player = m_player;
    if (!player) return;
    QString treasure_name;
    if (player->getTreasure()) treasure_name = player->getTreasure()->objectName();

    const QList<int> &pile = player->getPile(pile_name);
    if (pile.size() == 0) {
        if (_m_privatePiles.contains(pile_name)) {
            delete _m_privatePiles[pile_name];
            _m_privatePiles[pile_name] = NULL;
            _m_privatePiles.remove(pile_name);
        }
    } else {
        // retrieve menu and create a new pile if necessary
        QPushButton *button;
        if (!_m_privatePiles.contains(pile_name)) {
            button = new QPushButton;
            button->setObjectName(pile_name);
            if (treasure_name == pile_name)
                button->setProperty("treasure", "true");
            else
                button->setProperty("private_pile", "true");
            QGraphicsProxyWidget *button_widget = new QGraphicsProxyWidget(_getPileParent());
            button_widget->setObjectName(pile_name);
            button_widget->setWidget(button);
            _m_privatePiles[pile_name] = button_widget;
        } else {
            button = (QPushButton *)(_m_privatePiles[pile_name]->widget());
        }
        QString text = Sanguosha->translate(pile_name);
        if (pile.length() > 0)
             text.append(QString("(%1)").arg(pile.length()));
        button->setText(text);
        disconnect(button, SIGNAL(clicked()), this, SLOT(showPile()));
        connect(button, SIGNAL(clicked()), this, SLOT(showPile()));
    }

    QPoint start = _m_layout->m_privatePileStartPos;
    QPoint step = _m_layout->m_privatePileStep;
    QSize size = _m_layout->m_privatePileButtonSize;
    QList<QGraphicsProxyWidget *> widgets_t, widgets_p, widgets = _m_privatePiles.values();
    foreach (QGraphicsProxyWidget *widget, widgets) {
        if (widget->objectName() == treasure_name)
            widgets_t << widget;
        else
            widgets_p << widget;
    }
    widgets = widgets_t + widgets_p;
    for (int i = 0; i < widgets.length(); i++) {
        QGraphicsProxyWidget *widget = widgets[i];
        widget->setPos(start + i * step);
        widget->resize(size);
    }
}

void PlayerCardContainer::showPile() {
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        const ClientPlayer *player = getPlayer();
        if (!player) return;
        QList<int> card_ids = player->getPile(button->objectName());
        if (card_ids.isEmpty() || card_ids.contains(-1)) return;
        RoomSceneInstance->doGongxin(card_ids, false, QList<int>());
    }
}

void PlayerCardContainer::updateDrankState() {
    // we have two avatar areas
    if (m_player->getMark("drank") > 0) {
        _m_avatarArea->setBrush(G_PHOTO_LAYOUT.m_drankMaskColor);
        _m_secondaryAvatarArea->setBrush(G_PHOTO_LAYOUT.m_drankMaskColor);
    }
    else {
        _m_avatarArea->setBrush(Qt::NoBrush);
        _m_secondaryAvatarArea->setBrush(Qt::NoBrush);
    }
}

void PlayerCardContainer::updateHandcardNum() {
    int num = 0;
    if (m_player && m_player->getGeneral()) num = m_player->getHandcardNum();
    Q_ASSERT(num >= 0);
    _m_layout->m_handCardFont.paintText(_m_handCardNumText, _m_layout->m_handCardArea,
        Qt::AlignCenter, QString::number(num));
    _m_handCardNumText->setVisible(true);
}

void PlayerCardContainer::updateMarks() {
    if (!_m_markItem) return;
    QRect parentRect = _getMarkParent()->boundingRect().toRect();
    QSize markSize = _m_markItem->boundingRect().size().toSize();
    QRect newRect = _m_layout->m_markTextArea.getTranslatedRect(parentRect, markSize);
    if (_m_layout == &G_PHOTO_LAYOUT)
        _m_markItem->setPos(newRect.topLeft());
    else
        _m_markItem->setPos(newRect.left(), newRect.top() + newRect.height() / 2);
}

void PlayerCardContainer::_updateEquips() {
    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        CardItem *equip = _m_equipCards[i];
        if (equip == NULL) continue;
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard()->getRealCard());
        QPixmap pixmap = _getEquipPixmap(equip_card);
        _m_equipLabel[i]->setPixmap(pixmap);
        _m_equipRegions[i]->setPos(_m_layout->m_equipAreas[i].topLeft());
    }
}

void PlayerCardContainer::refresh() {
    if (!m_player || !m_player->getGeneral() || !m_player->isAlive()) {

        _m_faceTurnedIcon->setVisible(false);
        if (_m_faceTurnedIcon2)
            _m_faceTurnedIcon2->setVisible(false);
        if (_m_chainIcon)
            _m_chainIcon->setVisible(false);
        if (_m_duanchangMask)
            _m_duanchangMask->setVisible(false);
        if (_m_duanchangMask2)
            _m_duanchangMask2->setVisible(false);
        _m_actionIcon->setVisible(false);
        _m_saveMeIcon->setVisible(false);
        leftDisableShowLock->setVisible(false);
        rightDisableShowLock->setVisible(false);
    } else if (m_player) {
        if (_m_faceTurnedIcon)
            _m_faceTurnedIcon->setVisible(!m_player->faceUp());
        if (_m_faceTurnedIcon2)
            _m_faceTurnedIcon2->setVisible(!m_player->faceUp());
        if (_m_chainIcon)
            _m_chainIcon->setVisible(m_player->isChained());
        if (_m_duanchangMask)
            _m_duanchangMask->setVisible(m_player->isDuanchang(true));
        if (_m_duanchangMask2)
            _m_duanchangMask2->setVisible(m_player->isDuanchang(false));
        if (_m_actionIcon)
            _m_actionIcon->setVisible(m_player->hasFlag("actioned"));
        if (_m_deathIcon)
            _m_deathIcon->setVisible(m_player->isDead());
        if (leftDisableShowLock)
            leftDisableShowLock->setVisible(!m_player->hasShownGeneral1() && !m_player->disableShow(true).isEmpty());
        if (rightDisableShowLock)
            rightDisableShowLock->setVisible(m_player->getGeneral2() && !m_player->hasShownGeneral2() && !m_player->disableShow(false).isEmpty());
    }
    updateHandcardNum();
    _adjustComponentZValues();
}

void PlayerCardContainer::repaintAll() {
    _m_avatarArea->setRect(_m_layout->m_avatarArea);
    _m_secondaryAvatarArea->setRect(_m_layout->m_secondaryAvatarArea);

    stopHeroSkinChangingAnimation();

    updateAvatar();
    updateSmallAvatar();
    updatePhase();
    updateMarks();
    _updateProgressBar();
    _updateDeathIcon();
    _updateEquips();
    updateDelayedTricks();
    //we have two avatar areas now...
    _paintPixmap(_m_faceTurnedIcon, _m_layout->m_avatarArea, QSanRoomSkin::S_SKIN_KEY_FACETURNEDMASK,
        _getAvatarParent());
    //paint faceTurnedIcon in secondaryAvatarArea only if inheriting Dashboard
    _paintPixmap(_m_chainIcon, _m_layout->m_chainedIconRegion, QSanRoomSkin::S_SKIN_KEY_CHAIN,
        _getAvatarParent());
    _paintPixmap(_m_duanchangMask, _m_layout->m_duanchangMaskRegion, QSanRoomSkin::S_SKIN_KEY_DUANCHANG,
        _getAvatarParent());
    _paintPixmap(_m_duanchangMask2, _m_layout->m_duanchangMaskRegion2, QSanRoomSkin::S_SKIN_KEY_DUANCHANG,
        _getAvatarParent());
    _paintPixmap(_m_saveMeIcon, _m_layout->m_saveMeIconRegion, QSanRoomSkin::S_SKIN_KEY_SAVE_ME_ICON,
        _getAvatarParent());
    _paintPixmap(_m_actionIcon, _m_layout->m_actionedIconRegion, QSanRoomSkin::S_SKIN_KEY_ACTIONED_ICON,
        _getAvatarParent());
    if (_m_seatItem != NULL)
        _paintPixmap(_m_seatItem, _m_layout->m_seatIconRegion,
            _getPixmap(QSanRoomSkin::S_SKIN_KEY_SEAT_NUMBER, QString::number(m_player->property("UI_Seat").toInt())),
            _getAvatarParent());
    if (_m_roleComboBox != NULL)
        _m_roleComboBox->setPos(_m_layout->m_roleComboBoxPos);

    _m_hpBox->setIconSize(_m_layout->m_magatamaSize);
    _m_hpBox->setOrientation(_m_layout->m_magatamasHorizontal ? Qt::Horizontal : Qt::Vertical);
    _m_hpBox->setBackgroundVisible(_m_layout->m_magatamasBgVisible);
    _m_hpBox->setAnchorEnable(true);
    _m_hpBox->setAnchor(_m_layout->m_magatamasAnchor, _m_layout->m_magatamasAlign);
    _m_hpBox->setImageArea(_m_layout->m_magatamaImageArea);
    _m_hpBox->update();

    QPixmap lock = _getPixmap(QSanRoomSkin::S_SKIN_KEY_DISABLE_SHOW_LOCK);
    _paintPixmap(leftDisableShowLock, _m_layout->leftDisableShowLockArea,
                 lock, _getAvatarParent());
    _paintPixmap(rightDisableShowLock, _m_layout->rightDisableShowLockArea,
                 lock, _getAvatarParent());

    _adjustComponentZValues();
    _initializeRemovedEffect();
    refresh();
}

void PlayerCardContainer::_createRoleComboBox() {
    _m_roleComboBox = new RoleComboBox(_getRoleComboBoxParent());
}

void PlayerCardContainer::setPlayer(ClientPlayer *player) {
    this->m_player = player;
    if (player) {
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));
        connect(player, SIGNAL(general2_changed()), this, SLOT(updateSmallAvatar()));
        connect(player, SIGNAL(kingdom_changed(QString)), this, SLOT(updateAvatar()));
        connect(player, SIGNAL(state_changed()), this, SLOT(refresh()));
        connect(player, SIGNAL(phase_changed()), this, SLOT(updatePhase()));
        connect(player, SIGNAL(drank_changed()), this, SLOT(updateDrankState()));
        connect(player, SIGNAL(action_taken()), this, SLOT(refresh()));
        connect(player, SIGNAL(duanchang_invoked()), this, SLOT(refresh()));
        connect(player, SIGNAL(pile_changed(QString)), this, SLOT(updatePile(QString)));
        connect(player, SIGNAL(kingdom_changed(QString)), _m_roleComboBox, SLOT(fix(QString)));
        connect(player, SIGNAL(hp_changed()), this, SLOT(updateHp()));
        connect(player, SIGNAL(disable_show_changed()), this, SLOT(refresh()));
        connect(player, SIGNAL(removedChanged()), this, SLOT(onRemovedChanged()));

        QTextDocument *textDoc = m_player->getMarkDoc();
        Q_ASSERT(_m_markItem);
        _m_markItem->setDocument(textDoc);
        connect(textDoc, SIGNAL(contentsChanged()), this, SLOT(updateMarks()));
        connect(player, SIGNAL(headSkinIdChanged(QString)),
                _m_avatarIcon, SLOT(startChangeHeroSkinAnimation(const QString &)));
        connect(player, SIGNAL(deputySkinIdChanged(QString)),
                _m_smallAvatarIcon, SLOT(startChangeHeroSkinAnimation(const QString &)));
    }
    updateAvatar();
    refresh();
}

QList<CardItem *> PlayerCardContainer::removeDelayedTricks(const QList<int> &cardIds) {
    QList<CardItem *> result;
    foreach(int card_id, cardIds) {
        CardItem *item = CardItem::FindItem(_m_judgeCards, card_id);
        Q_ASSERT(item != NULL);
        int index = _m_judgeCards.indexOf(item);
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * index);
        item->setOpacity(0.0);
        item->setPos(start.center());
        _m_judgeCards.removeAt(index);
        delete _m_judgeIcons.takeAt(index);
        result.append(item);
    }
    updateDelayedTricks();
    return result;
}

void PlayerCardContainer::updateDelayedTricks() {
    for (int i = 0; i < _m_judgeIcons.size(); i++) {
        QGraphicsPixmapItem *item = _m_judgeIcons[i];
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * i);
        item->setPos(start.topLeft());
    }
}

void PlayerCardContainer::addDelayedTricks(QList<CardItem *> &tricks) {
    foreach(CardItem *trick, tricks) {
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(_getDelayedTrickParent());
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * _m_judgeCards.size());
        _paintPixmap(item, start, G_ROOM_SKIN.getCardJudgeIconPixmap(trick->getCard()->objectName()));
        trick->setHomeOpacity(0.0);
        trick->setHomePos(start.center());
        const Card *card = trick->getCard();
        const Card *realCard = Sanguosha->getEngineCard(card->getEffectiveId());
        QString toolTip = QString("<font color=%1><b>%2 [</b><img src='image/system/log/%3.png' height=12/><b>%4]</b></font><br />%5")
            .arg(Config.SkillDescriptionInToolTipColor.name())
            .arg(Sanguosha->translate(realCard->objectName()))
            .arg(realCard->getSuitString())
            .arg(realCard->getNumberString())
            .arg(card->getDescription());
        item->setToolTip(toolTip);
        _m_judgeCards.append(trick);
        _m_judgeIcons.append(item);
    }
}

QPixmap PlayerCardContainer::_getEquipPixmap(const EquipCard *equip) {
    const Card *realCard = Sanguosha->getEngineCard(equip->getEffectiveId());
    QPixmap equipIcon(_m_layout->m_equipAreas[0].size());
    equipIcon.fill(Qt::transparent);
    QPainter painter(&equipIcon);
    // icon / background
    QRect imageArea = _m_layout->m_equipImageArea;
    if (equip->isKindOf("Horse"))
        imageArea = _m_layout->m_horseImageArea;
    painter.drawPixmap(imageArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_EQUIP_ICON, equip->objectName()));
    // equip suit
    QRect suitArea = _m_layout->m_equipSuitArea;
    if (equip->isKindOf("Horse"))
        suitArea = _m_layout->m_horseSuitArea;
    painter.drawPixmap(suitArea, G_ROOM_SKIN.getCardSuitPixmap(realCard->getSuit()));
    // equip point
    QRect pointArea = _m_layout->m_equipPointArea;
    if (equip->isKindOf("Horse"))
        pointArea = _m_layout->m_horsePointArea;
    _m_layout->m_equipPointFont.paintText(&painter,
                                          pointArea,
                                          Qt::AlignLeft | Qt::AlignVCenter,
                                          realCard->getNumberString());
    return equipIcon;
}

void PlayerCardContainer::setFloatingArea(QRect rect) {
    _m_floatingAreaRect = rect;
    QPixmap dummy(rect.size());
    dummy.fill(Qt::transparent);
    _m_floatingArea->setPixmap(dummy);
    _m_floatingArea->setPos(rect.topLeft());
    if (_getPhaseParent() == _m_floatingArea) updatePhase();
    if (_getMarkParent() == _m_floatingArea) updateMarks();
    if (_getProgressBarParent() == _m_floatingArea) _updateProgressBar();
}

void PlayerCardContainer::addEquips(QList<CardItem *> &equips) {
    foreach(CardItem *equip, equips) {
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard()->getRealCard());
        int index = (int)(equip_card->location());
        Q_ASSERT(_m_equipCards[index] == NULL);
        _m_equipCards[index] = equip;
        connect(equip, SIGNAL(mark_changed()), this, SLOT(_onEquipSelectChanged()));
        equip->setHomeOpacity(0.0);
        equip->setHomePos(_m_layout->m_equipAreas[index].center());
        _m_equipRegions[index]->setToolTip(equip_card->getDescription());
        QPixmap pixmap = _getEquipPixmap(equip_card);
        _m_equipLabel[index]->setPixmap(pixmap);

        _mutexEquipAnim.lock();
        _m_equipRegions[index]->setPos(_m_layout->m_equipAreas[index].topLeft()
            + QPoint(_m_layout->m_equipAreas[index].width() / 2, 0));
        _m_equipRegions[index]->setOpacity(0);
        _m_equipAnim[index]->stop();
        _m_equipAnim[index]->clear();
        QPropertyAnimation *anim = new QPropertyAnimation(_m_equipRegions[index], "pos", this);
        anim->setEndValue(_m_layout->m_equipAreas[index].topLeft());
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        anim = new QPropertyAnimation(_m_equipRegions[index], "opacity", this);
        anim->setEndValue(255);
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        _m_equipAnim[index]->start();
        _mutexEquipAnim.unlock();
    }
}

QList<CardItem *> PlayerCardContainer::removeEquips(const QList<int> &cardIds) {
    QList<CardItem *> result;
    foreach(int card_id, cardIds) {
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(Sanguosha->getEngineCard(card_id));
        int index = (int)(equip_card->location());
        Q_ASSERT(_m_equipCards[index] != NULL);
        CardItem *equip = _m_equipCards[index];
        equip->setHomeOpacity(0.0);
        equip->setPos(_m_layout->m_equipAreas[index].center());
        result.append(equip);
        _m_equipCards[index] = NULL;
        _mutexEquipAnim.lock();
        _m_equipAnim[index]->stop();
        _m_equipAnim[index]->clear();
        QPropertyAnimation *anim = new QPropertyAnimation(_m_equipRegions[index], "pos", this);
        anim->setEndValue(_m_layout->m_equipAreas[index].topLeft()
            + QPoint(_m_layout->m_equipAreas[index].width() / 2, 0));
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        anim = new QPropertyAnimation(_m_equipRegions[index], "opacity", this);
        anim->setEndValue(0);
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        _m_equipAnim[index]->start();
        _mutexEquipAnim.unlock();
    }
    return result;
}

void PlayerCardContainer::updateAvatarTooltip() {
    if (m_player) {
        _m_avatarArea->setToolTip(m_player->getHeadSkillDescription());
        _m_secondaryAvatarArea->setToolTip(m_player->getDeputySkillDescription());
    }
}

PlayerCardContainer::PlayerCardContainer() {
    _m_layout = NULL;
    _m_avatarArea = _m_secondaryAvatarArea = NULL;
    _m_avatarNameItem = _m_secondaryAvatarNameItem = NULL;
    _m_avatarIcon = _m_smallAvatarIcon = NULL;
    _m_circleItem = NULL;
    _m_screenNameItem = NULL;
    _m_chainIcon = NULL;
    _m_duanchangMask = _m_duanchangMask2 = NULL;
    _m_faceTurnedIcon = _m_faceTurnedIcon2 = NULL;
    _m_handCardBg = _m_handCardNumText = NULL;
    _m_kingdomColorMaskIcon = _m_kingdomColorMaskIcon2 = _m_deathIcon = NULL;
    _m_actionIcon = NULL;
    _m_kingdomIcon = NULL;
    _m_saveMeIcon = NULL;
    _m_phaseIcon = NULL;
    _m_markItem = NULL;
    _m_roleComboBox = NULL;
    m_player = NULL;
    _m_selectedFrame = _m_selectedFrame2 = NULL;

    leftDisableShowLock = rightDisableShowLock = NULL;

    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        _m_equipCards[i] = NULL;
        _m_equipRegions[i] = NULL;
        _m_equipAnim[i] = NULL;
        _m_equipLabel[i] = NULL;
    }
    _m_extraSkillBg = NULL;
    _m_extraSkillText = NULL;

    _m_floatingArea = NULL;
    _m_votesGot = 0;
    _m_maxVotes = 1;
    _m_votesItem = NULL;
    _m_distanceItem = NULL;
    _m_seatItem = NULL;
    _m_groupMain = new QGraphicsPixmapItem(this);
    _m_groupMain->setFlag(ItemHasNoContents);
    _m_groupMain->setPos(0, 0);
    _m_groupDeath = new QGraphicsPixmapItem(this);
    _m_groupDeath->setFlag(ItemHasNoContents);
    _m_groupDeath->setPos(0, 0);
    _allZAdjusted = false;
}

void PlayerCardContainer::hideAvatars() {
    if (_m_avatarIcon) _m_avatarIcon->hide();
    if (_m_smallAvatarIcon) _m_smallAvatarIcon->hide();
}

void PlayerCardContainer::_layUnder(QGraphicsItem *item) {
    _lastZ--;
    //Q_ASSERT((unsigned long)item != 0xcdcdcdcd);
    if (item)
        item->setZValue(_lastZ--);
    else
        _allZAdjusted = false;
}

bool PlayerCardContainer::_startLaying() {
    if (_allZAdjusted) return false;
    _allZAdjusted = true;
    _lastZ = -1;
    return true;
}

void PlayerCardContainer::_layBetween(QGraphicsItem *middle, QGraphicsItem *item1, QGraphicsItem *item2) {
    if (middle && item1 && item2)
        middle->setZValue((item1->zValue() + item2->zValue()) / 2.0);
    else
        _allZAdjusted = false;
}

void PlayerCardContainer::_adjustComponentZValues() {
    // all components use negative zvalues to ensure that no other generated
    // cards can be under us.

    // layout
    if (!_startLaying()) return;

    _layUnder(_m_floatingArea);
    _layUnder(_m_distanceItem);
    _layUnder(_m_votesItem);
    foreach (QGraphicsItem *pile, _m_privatePiles)
        _layUnder(pile);
    foreach(QGraphicsItem *judge, _m_judgeIcons)
        _layUnder(judge);
    _layUnder(_m_markItem);
    _layUnder(_m_progressBarItem);
    _layUnder(_m_roleComboBox);
    _layUnder(_m_saveMeIcon);
    _layUnder(_m_secondaryAvatarNameItem);
    _layUnder(_m_avatarNameItem);
    _layUnder(_m_kingdomColorMaskIcon);
    _layUnder(_m_kingdomColorMaskIcon2);
    _layUnder(leftDisableShowLock);
    _layUnder(rightDisableShowLock);
    //it's meaningless to judge which icon should be on top
    _layUnder(_m_chainIcon);
    _layUnder(_m_hpBox);
    _layUnder(_m_handCardNumText);
    _layUnder(_m_handCardBg);
    _layUnder(_m_actionIcon);
    _layUnder(_m_phaseIcon);
    _layUnder(_m_kingdomIcon);
    _layUnder(_m_screenNameItem);
    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++)
        _layUnder(_m_equipRegions[i]);
    _layUnder(_m_selectedFrame2);
    _layUnder(_m_selectedFrame);
    _layUnder(_m_extraSkillText);
    _layUnder(_m_extraSkillBg);
    _layUnder(_m_faceTurnedIcon2);
    _layUnder(_m_faceTurnedIcon);
    _layUnder(_m_duanchangMask2);
    _layUnder(_m_duanchangMask);
    _layUnder(_m_secondaryAvatarArea);
    _layUnder(_m_avatarArea);
    _layUnder(_m_circleItem);
    _layUnder(_m_smallAvatarIcon);
    _layUnder(_m_avatarIcon);
}

void PlayerCardContainer::updateKingdom(const QString &kingdom) {
    _m_roleComboBox->fix(kingdom);
}

void PlayerCardContainer::_updateProgressBar() {
    QGraphicsItem *parent = _getProgressBarParent();
    if (parent == NULL) return;
    _m_progressBar->setOrientation(_m_layout->m_isProgressBarHorizontal ? Qt::Horizontal : Qt::Vertical);
    QRectF newRect = _m_layout->m_progressBarArea.getTranslatedRect(parent->boundingRect().toRect());
    _m_progressBar->setFixedHeight(newRect.height());
    _m_progressBar->setFixedWidth(newRect.width());
    _m_progressBarItem->setParentItem(parent);
    _m_progressBarItem->setPos(newRect.left(), newRect.top());
}

void PlayerCardContainer::_createControls() {
    _m_floatingArea = new QGraphicsPixmapItem(_m_groupMain);

    _m_screenNameItem = new QGraphicsPixmapItem(_getAvatarParent());

    _m_avatarArea = new QGraphicsRectItem(_m_layout->m_avatarArea, _getAvatarParent());
    _m_avatarArea->setPen(Qt::NoPen);
    _m_avatarNameItem = new QGraphicsPixmapItem(_getAvatarParent());

    _m_secondaryAvatarArea = new QGraphicsRectItem(_m_layout->m_secondaryAvatarArea, _getAvatarParent());
    _m_secondaryAvatarArea->setPen(Qt::NoPen);
    _m_secondaryAvatarNameItem = new QGraphicsPixmapItem(_getAvatarParent());

    _m_extraSkillText = new QGraphicsPixmapItem(_getAvatarParent());
    _m_extraSkillText->hide();

    _m_handCardNumText = new QGraphicsPixmapItem(_getAvatarParent());

    _m_hpBox = new MagatamasBoxItem(_getAvatarParent());

    // Now set up progress bar
    _m_progressBar = new QSanCommandProgressBar;
    _m_progressBar->setAutoHide(true);
    _m_progressBar->hide();
    _m_progressBarItem = new QGraphicsProxyWidget(_getProgressBarParent());
    _m_progressBarItem->setWidget(_m_progressBar);
    _updateProgressBar();

    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        _m_equipLabel[i] = new QLabel;
        _m_equipLabel[i]->setStyleSheet("QLabel { background-color: transparent; }");
        _m_equipLabel[i]->setPixmap(QPixmap(_m_layout->m_equipAreas[i].size()));
        _m_equipRegions[i] = new QGraphicsProxyWidget();
        _m_equipRegions[i]->setWidget(_m_equipLabel[i]);
        _m_equipRegions[i]->setPos(_m_layout->m_equipAreas[i].topLeft());
        _m_equipRegions[i]->setParentItem(_getEquipParent());
        _m_equipRegions[i]->setOpacity(0.0);
        _m_equipAnim[i] = new QParallelAnimationGroup(this);
    }

    _m_markItem = new QGraphicsTextItem(_getMarkParent());
    _m_markItem->setDefaultTextColor(Qt::white);

    _createRoleComboBox();
    repaintAll();
}

void PlayerCardContainer::_updateDeathIcon() {
    if (!m_player || !m_player->isDead()) return;
    QRect deathArea = _m_layout->m_deathIconRegion.getTranslatedRect(_getDeathIconParent()->boundingRect().toRect());
    _paintPixmap(_m_deathIcon, deathArea,
        QPixmap(m_player->getDeathPixmapPath()), _getDeathIconParent());
    _m_deathIcon->setZValue(30000.0);
}

void PlayerCardContainer::killPlayer() {
    _m_roleComboBox->fix(m_player->getRole() == "careerist" ? "careerist" : m_player->getKingdom());
    _m_roleComboBox->setEnabled(false);
    _updateDeathIcon();
    _m_saveMeIcon->hide();
    if (_m_votesItem) _m_votesItem->hide();
    if (_m_distanceItem) _m_distanceItem->hide();
    QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect();
    effect->setColor(_m_layout->m_deathEffectColor);
    effect->setStrength(1.0);
    _m_groupMain->setGraphicsEffect(effect);
    refresh();
    _m_deathIcon->show();
}

void PlayerCardContainer::revivePlayer() {
    _m_votesGot = 0;
    _m_groupMain->setGraphicsEffect(NULL);
    Q_ASSERT(_m_deathIcon);
    _m_deathIcon->hide();
    refresh();
}

void PlayerCardContainer::mousePressEvent(QGraphicsSceneMouseEvent *) {
}

void PlayerCardContainer::updateVotes(bool need_select, bool display_1) {
    if ((need_select && !isSelected()) || _m_votesGot < 1 || (!display_1 && _m_votesGot == 1))
        _clearPixmap(_m_votesItem);
    else {
        _paintPixmap(_m_votesItem, _m_layout->m_votesIconRegion,
            _getPixmap(QSanRoomSkin::S_SKIN_KEY_VOTES_NUMBER, QString::number(_m_votesGot)),
            _getAvatarParent());
        _m_votesItem->setZValue(1);
        _m_votesItem->show();
    }
}

void PlayerCardContainer::updateReformState() {
    _m_votesGot--;
    updateVotes(false, true);
}

void PlayerCardContainer::showDistance() {
    int dis = Self->distanceTo(m_player);
    if (dis > 0) {
        _paintPixmap(_m_distanceItem, _m_layout->m_votesIconRegion,
            _getPixmap(QSanRoomSkin::S_SKIN_KEY_VOTES_NUMBER, QString::number(dis)),
            _getAvatarParent());
        _m_distanceItem->setZValue(1.1);
        if (!Self->inMyAttackRange(m_player)) {
            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect();
            effect->setColor(_m_layout->m_deathEffectColor);
            effect->setStrength(1.0);
            _m_distanceItem->setGraphicsEffect(effect);
        } else {
            _m_distanceItem->setGraphicsEffect(NULL);
        }
    } else {
        delete _m_distanceItem;
        _m_distanceItem = NULL;
    }
    if (!_m_distanceItem)
        return;
    if (!_m_distanceItem->isVisible())
        _m_distanceItem->show();
}

void PlayerCardContainer::hideDistance() {
    if (_m_distanceItem && _m_distanceItem->isVisible())
        _m_distanceItem->hide();
}

void PlayerCardContainer::onRemovedChanged()
{
    QAbstractAnimation::Direction direction = m_player->isRemoved() ? QAbstractAnimation::Forward
                                                                    : QAbstractAnimation::Backward;

    _getPlayerRemovedEffect()->setDirection(direction);
    _getPlayerRemovedEffect()->start();
}

void PlayerCardContainer::showSeat() {
    _paintPixmap(_m_seatItem, _m_layout->m_seatIconRegion,
        _getPixmap(QSanRoomSkin::S_SKIN_KEY_SEAT_NUMBER, QString::number(m_player->getSeat())),
        _getAvatarParent());
    //save the seat number for later use
    m_player->setProperty("UI_Seat", m_player->getSeat());
    _m_seatItem->setZValue(1.1);
}

bool PlayerCardContainer::_isSelected(QGraphicsItem *item) const {
    return item != NULL && item->isUnderMouse() && isEnabled() &&
        (flags() & QGraphicsItem::ItemIsSelectable);
}

void PlayerCardContainer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsItem *item1 = getMouseClickReceiver();
    QGraphicsItem *item2 = getMouseClickReceiver2();
    if (_isSelected(item1) || _isSelected(item2)) {
        if (event->button() == Qt::RightButton)
            setSelected(false);
        else if (event->button() == Qt::LeftButton) {
            _m_votesGot++;
            setSelected(_m_votesGot <= _m_maxVotes);
            if (_m_votesGot > 1) emit selected_changed();
        }
        updateVotes();
    }
}

void PlayerCardContainer::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) {
    if (Config.EnableDoubleClick)
        RoomSceneInstance->doOkButton();
}

QVariant PlayerCardContainer::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemSelectedHasChanged) {
        if (!value.toBool()) {
            _m_votesGot = 0;
            _clearPixmap(_m_selectedFrame);
            _m_selectedFrame->hide();
            if (getMouseClickReceiver2()) {
                _clearPixmap(_m_selectedFrame2);
                _m_selectedFrame2->hide();
            }
        } else {
            _paintPixmap(_m_selectedFrame, _m_layout->m_focusFrameArea,
                _getPixmap(QSanRoomSkin::S_SKIN_KEY_SELECTED_FRAME),
                _getFocusFrameParent());
            _m_selectedFrame->show();
            if (getMouseClickReceiver2()) {
                _paintPixmap(_m_selectedFrame2, _m_layout->m_focusFrameArea2,
                    _getPixmap(QSanRoomSkin::S_SKIN_KEY_SELECTED_FRAME),
                    _getFocusFrameParent());
                _m_selectedFrame2->show();
            }
        }
        updateVotes();
        emit selected_changed();
    } else if (change == ItemEnabledHasChanged) {
        _m_votesGot = 0;
        emit enable_changed();
    }

    return QGraphicsObject::itemChange(change, value);
}

void PlayerCardContainer::_onEquipSelectChanged() {
}

void PlayerCardContainer::stopHeroSkinChangingAnimation()
{
    if ((NULL != _m_avatarIcon) && !_m_avatarIcon->isSkinChangingFinished()) {
        _m_avatarIcon->stopChangeHeroSkinAnimation();
    }
    if ((NULL != _m_smallAvatarIcon) && !_m_smallAvatarIcon->isSkinChangingFinished()) {
        _m_smallAvatarIcon->stopChangeHeroSkinAnimation();
    }
}

