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

#include "dashboard.h"
#include "engine.h"
#include "settings.h"
#include "client.h"
#include "standard.h"
#include "roomscene.h"
#include "heroskincontainer.h"
#include "graphicspixmaphoveritem.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPixmapCache>
#include <QParallelAnimationGroup>

using namespace QSanProtocol;

Dashboard::Dashboard(QGraphicsItem *widget)
    : // At this stage, we cannot decide the dashboard size yet, the whole
      // point in creating them here is to allow PlayerCardContainer to
      // anchor all controls and widgets to the correct frame.
      //
      // Note that 20 is just a random plug-in so that we can proceed with
      // control creation, the actual width is updated when setWidth() is
      // called by its graphics parent.
      width(G_DASHBOARD_LAYOUT.m_leftWidth + G_DASHBOARD_LAYOUT.m_rightWidth + 20),
      leftFrame(NULL), middleFrame(NULL), rightFrame(NULL),
      rightFrameBase(NULL), rightFrameBg(NULL), magatamasBase(NULL),
      headGeneralFrame(NULL), deputyGeneralFrame(NULL),
      buttonWidget(widget), selected(NULL), layout(&G_DASHBOARD_LAYOUT),
      leftHiddenMark(NULL), rightHiddenMark(NULL),
      headIcon(NULL), deputyIcon(NULL),
      pendingCard(NULL), viewAsSkill(NULL), filter(NULL),
      m_changeHeadHeroSkinButton(NULL), m_changeDeputyHeroSkinButton(NULL),
      m_headHeroSkinContainer(NULL), m_deputyHeroSkinContainer(NULL)

{
    Q_ASSERT(buttonWidget);
    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        _m_equipSkillBtns[i] = NULL;
        _m_isEquipsAnimOn[i] = false;
    }

    _m_layout = layout;
    m_player = Self;
    leftDisableShowLock = NULL;
    rightDisableShowLock = NULL;

    _createLeft();
    _createMiddle();
    _createRight();

    // only do this after you create all frames.
    _createControls();
    _createExtraButtons();

    connect(_m_avatarIcon, SIGNAL(hover_enter()), this, SLOT(onAvatarHoverEnter()));
    connect(_m_avatarIcon, SIGNAL(hover_leave()), this, SLOT(onAvatarHoverLeave()));
    connect(_m_avatarIcon, SIGNAL(skin_changing_start()),
        this, SLOT(onSkinChangingStart()));
    connect(_m_avatarIcon, SIGNAL(skin_changing_finished()),
        this, SLOT(onSkinChangingFinished()));

    connect(_m_smallAvatarIcon, SIGNAL(hover_enter()), this, SLOT(onAvatarHoverEnter()));
    connect(_m_smallAvatarIcon, SIGNAL(hover_leave()), this, SLOT(onAvatarHoverLeave()));
    connect(_m_smallAvatarIcon, SIGNAL(skin_changing_start()),
        this, SLOT(onSkinChangingStart()));
    connect(_m_smallAvatarIcon, SIGNAL(skin_changing_finished()),
        this, SLOT(onSkinChangingFinished()));

    _m_sort_menu = new QMenu(RoomSceneInstance->mainWindow());
}

void Dashboard::refresh() {
    PlayerCardContainer::refresh();
    if (!m_player || !m_player->getGeneral() || !m_player->isAlive()) {
        _m_shadow_layer1->setBrush(Qt::NoBrush);
        _m_shadow_layer2->setBrush(Qt::NoBrush);
        leftHiddenMark->setVisible(false);
        rightHiddenMark->setVisible(false);
    } else if (m_player) {
        _m_shadow_layer1->setBrush(m_player->hasShownGeneral1() ? Qt::transparent : G_DASHBOARD_LAYOUT.m_generalShadowColor);
        _m_shadow_layer2->setBrush(m_player->hasShownGeneral2() ? Qt::transparent : G_DASHBOARD_LAYOUT.m_generalShadowColor);
        leftHiddenMark->setVisible(m_player->isHidden(true));
        rightHiddenMark->setVisible(m_player->isHidden(false));
    }
}

void Dashboard::repaintAll() {
    PlayerCardContainer::repaintAll();
    if (NULL != m_changeHeadHeroSkinButton) {
        m_changeHeadHeroSkinButton->setPos(layout->m_changeHeadHeroSkinButtonPos);
    }
    if (NULL != m_changeDeputyHeroSkinButton) {
        m_changeDeputyHeroSkinButton->setPos(layout->m_changeDeputyHeroSkinButtonPos);
    }
}

bool Dashboard::isAvatarUnderMouse() {
    return _m_avatarArea->isUnderMouse() || _m_secondaryAvatarArea->isUnderMouse();
}

void Dashboard::hideControlButtons() {
    m_trustButton->hide();
    m_btnReverseSelection->hide();
    m_btnSortHandcard->hide();
    m_btnNoNullification->hide();

}

void Dashboard::showControlButtons() {
    m_trustButton->show();
    m_btnReverseSelection->show();
    m_btnSortHandcard->show();
    m_btnNoNullification->show();
}

void Dashboard::showProgressBar(QSanProtocol::Countdown countdown) {
    _m_progressBar->setCountdown(countdown);
    connect(_m_progressBar, SIGNAL(timedOut()), this, SIGNAL(progressBarTimedOut()));
    _m_progressBar->show();
}

QGraphicsItem *Dashboard::getMouseClickReceiver() {
    return _m_avatarIcon;
}

QGraphicsItem *Dashboard::getMouseClickReceiver2() {
    return _m_smallAvatarIcon;
}

void Dashboard::_createLeft() {
    QRect rect = QRect(0, 0, G_DASHBOARD_LAYOUT.m_leftWidth, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(leftFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_LEFTFRAME), this);
    leftFrame->setZValue(-1000); // nobody should be under me.
    _createEquipBorderAnimations();
}

int Dashboard::getButtonWidgetWidth() const{
    Q_ASSERT(buttonWidget);
    return buttonWidget->boundingRect().width();
}

void Dashboard::_createMiddle() {
    // this is just a random rect. see constructor for more details
    QRect rect = QRect(0, 0, 1, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(middleFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_MIDDLEFRAME), this);
    middleFrame->setZValue(-1000); // nobody should be under me.
    buttonWidget->setParentItem(middleFrame);

    trusting_item = new QGraphicsRectItem(this);
    trusting_text = new QGraphicsSimpleTextItem(tr("Trusting ..."), this);
    trusting_text->setPos(this->boundingRect().width() / 2, 50);

    QBrush trusting_brush(G_DASHBOARD_LAYOUT.m_trustEffectColor);
    trusting_item->setBrush(trusting_brush);
    trusting_item->setOpacity(0.36);
    trusting_item->setZValue(1002.0);

    trusting_text->setFont(Config.BigFont);
    trusting_text->setBrush(Qt::white);
    trusting_text->setZValue(1002.1);

    trusting_item->hide();
    trusting_text->hide();
}

void Dashboard::_adjustComponentZValues() {
    PlayerCardContainer::_adjustComponentZValues();
    // make sure right frame is on top because we have a lot of stuffs
    // attached to it, such as the rolecomboBox, which should not be under
    // middle frame
    _layUnder(rightFrame);
    _layUnder(leftFrame);
    _layUnder(middleFrame);
    _layBetween(buttonWidget, middleFrame, _m_roleComboBox);
    _layUnder(rightHiddenMark);
    _layUnder(leftHiddenMark);
    _layUnder(_m_secondaryAvatarArea);
    _layUnder(_m_avatarArea);
    _layUnder(_m_shadow_layer2);
    _layUnder(_m_shadow_layer1);
    _layUnder(_m_faceTurnedIcon2);
    _layUnder(_m_faceTurnedIcon);
    _layUnder(_m_smallAvatarIcon);
    _layUnder(_m_avatarIcon);
    _layUnder(rightFrameBg);
    _layUnder(magatamasBase);
    _layUnder(rightFrameBase);
    //the following 2 items must be on top
    headGeneralFrame->setZValue(1000);
    deputyGeneralFrame->setZValue(1000);
    headIcon->setZValue(2000);
    deputyIcon->setZValue(2000);
}

int Dashboard::getWidth() {
    return this->width;
}

void Dashboard::_createRight() {
    QRect rect = QRect(width - G_DASHBOARD_LAYOUT.m_rightWidth, 0,
        G_DASHBOARD_LAYOUT.m_rightWidth,
        G_DASHBOARD_LAYOUT.m_normalHeight);
    QPixmap pix = QPixmap(1, 1);
    pix.fill(QColor(0, 0, 0, 0));
    _paintPixmap(rightFrame, rect, pix, _m_groupMain);
    _paintPixmap(rightFrameBase, QRect(0, 0, rect.width(), rect.height()),
        _getPixmap(QSanRoomSkin::S_SKIN_KEY_RIGHTBASE), rightFrame);
    _paintPixmap(rightFrameBg, QRect(0, 0, rect.width(), rect.height()),
        _getPixmap(QSanRoomSkin::S_SKIN_KEY_RIGHTFRAME), rightFrame);
    _paintPixmap(magatamasBase,
        QRect(rect.width() - G_DASHBOARD_LAYOUT.m_magatamasBaseWidth,
        0, rect.width(), rect.height()),
        _getPixmap(QSanRoomSkin::S_SKIN_KEY_MAGATAMAS_BASE), rightFrame);
    _paintPixmap(headGeneralFrame, G_DASHBOARD_LAYOUT.m_avatarArea,
        _getPixmap(QSanRoomSkin::S_SKIN_KEY_AVATAR_FRAME), rightFrame);
    _paintPixmap(deputyGeneralFrame, G_DASHBOARD_LAYOUT.m_secondaryAvatarArea,
        _getPixmap(QSanRoomSkin::S_SKIN_KEY_AVATAR_FRAME), rightFrame);
    rightFrame->setZValue(-1000); // nobody should be under me.

    QRect avatar1 = G_DASHBOARD_LAYOUT.m_avatarArea;
    rightSkillDock = new QSanInvokeSkillDock(rightFrame);
    rightSkillDock->setPos(avatar1.left(), avatar1.bottom() -
        G_DASHBOARD_LAYOUT.m_skillButtonsSize[0].height() / 1.3);
    rightSkillDock->setWidth(avatar1.width());

    QRect avatar2 = G_DASHBOARD_LAYOUT.m_secondaryAvatarArea;
    leftSkillDock = new QSanInvokeSkillDock(rightFrame);
    leftSkillDock->setPos(avatar2.left(), avatar2.bottom() -
        G_DASHBOARD_LAYOUT.m_skillButtonsSize[0].height() / 1.3);
    leftSkillDock->setWidth(avatar2.width());

    _m_shadow_layer1 = new QGraphicsRectItem(rightFrame);
    _m_shadow_layer1->setRect(G_DASHBOARD_LAYOUT.m_avatarArea);
    _m_shadow_layer2 = new QGraphicsRectItem(rightFrame);
    _m_shadow_layer2->setRect(G_DASHBOARD_LAYOUT.m_secondaryAvatarArea);

    _paintPixmap(_m_faceTurnedIcon2, _m_layout->m_secondaryAvatarArea, QSanRoomSkin::S_SKIN_KEY_FACETURNEDMASK,
        rightFrame);

    _paintPixmap(leftHiddenMark, G_DASHBOARD_LAYOUT.m_hiddenMarkRegion1, _getPixmap(QSanRoomSkin::S_SKIN_KEY_HIDDEN_MARK), rightFrame);
    _paintPixmap(rightHiddenMark, G_DASHBOARD_LAYOUT.m_hiddenMarkRegion2, _getPixmap(QSanRoomSkin::S_SKIN_KEY_HIDDEN_MARK), rightFrame);

    connect(ClientInstance, SIGNAL(head_preshowed()), this,
        SLOT(onHeadSkillPreshowed()));
    connect(ClientInstance, SIGNAL(deputy_preshowed()), this,
        SLOT(onDeputySkillPreshowed()));

    _paintPixmap(headIcon, G_DASHBOARD_LAYOUT.m_headIconRegion, _getPixmap(QSanRoomSkin::S_SKIN_KEY_HEAD_ICON), rightFrame);
    _paintPixmap(deputyIcon, G_DASHBOARD_LAYOUT.m_deputyIconRegion, _getPixmap(QSanRoomSkin::S_SKIN_KEY_DEPUTY_ICON), rightFrame);

    m_changeHeadHeroSkinButton = new QSanButton("heroskin",
        "change", rightFrame);
    m_changeHeadHeroSkinButton->hide();
    connect(m_changeHeadHeroSkinButton, SIGNAL(clicked()), this, SLOT(showHeroSkinList()));
    connect(m_changeHeadHeroSkinButton, SIGNAL(clicked_outside()), this, SLOT(heroSkinButtonMouseOutsideClicked()));

    m_changeDeputyHeroSkinButton = new QSanButton("heroskin",
        "change", rightFrame);
    m_changeDeputyHeroSkinButton->hide();
    connect(m_changeDeputyHeroSkinButton, SIGNAL(clicked()), this, SLOT(showHeroSkinList()));
    connect(m_changeDeputyHeroSkinButton, SIGNAL(clicked_outside()), this, SLOT(heroSkinButtonMouseOutsideClicked()));
}

void Dashboard::_updateFrames() {
    // Here is where we adjust all frames to actual width
    QRect rect = QRect(G_DASHBOARD_LAYOUT.m_leftWidth, 0,
        width - G_DASHBOARD_LAYOUT.m_rightWidth - G_DASHBOARD_LAYOUT.m_leftWidth, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(middleFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_MIDDLEFRAME), this);
    QRect rect2 = QRect(0, 0, width, G_DASHBOARD_LAYOUT.m_normalHeight);
    trusting_item->setRect(rect2);
    trusting_item->setPos(0, 0);
    trusting_text->setPos((rect2.width() - Config.BigFont.pixelSize() * 4.5) / 2,
        (rect2.height() - Config.BigFont.pixelSize()) / 2);
    rightFrame->setX(width - G_DASHBOARD_LAYOUT.m_rightWidth);
    Q_ASSERT(buttonWidget);
    buttonWidget->setX(rect.width() - getButtonWidgetWidth());
    buttonWidget->setY(0);
}

void Dashboard::setTrust(bool trust) {
    trusting_item->setVisible(trust);
    trusting_text->setVisible(trust);
}

void Dashboard::killPlayer() {
    trusting_item->hide();
    trusting_text->hide();
    _m_roleComboBox->fix(m_player->getRole() == "careerist" ? "careerist" : m_player->getKingdom());
    _m_roleComboBox->setEnabled(false);
    _updateDeathIcon();
    _m_saveMeIcon->hide();
    if (_m_votesItem) _m_votesItem->hide();
    if (_m_distanceItem) _m_distanceItem->hide();

    _m_deathIcon->show();
}

void Dashboard::revivePlayer() {
    _m_votesGot = 0;
    this->setGraphicsEffect(NULL);
    Q_ASSERT(_m_deathIcon);
    _m_deathIcon->hide();
    refresh();
}

void Dashboard::setDeathColor() {
    QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect();
    effect->setColor(_m_layout->m_deathEffectColor);
    effect->setStrength(1.0);
    this->setGraphicsEffect(effect);
    refresh();
}

bool Dashboard::_addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo) {
    Player::Place place = moveInfo.to_place;
    if (place == Player::PlaceSpecial) {
        foreach(CardItem *card, card_items)
            card->setHomeOpacity(0.0);
        QPointF center = mapFromItem(_getAvatarParent(), layout->m_secondaryAvatarArea.center());
        QRectF rect = QRectF(0, 0, layout->m_disperseWidth, 0);
        rect.moveCenter(center);
        _disperseCards(card_items, rect, Qt::AlignCenter, true, false);
        return true;
    }

    if (place == Player::PlaceEquip)
        addEquips(card_items);
    else if (place == Player::PlaceDelayedTrick)
        addDelayedTricks(card_items);
    else if (place == Player::PlaceHand)
        addHandCards(card_items);

    adjustCards(true);
    return false;
}

void Dashboard::addHandCards(QList<CardItem *> &card_items) {
    foreach(CardItem *card_item, card_items)
        _addHandCard(card_item);
    updateHandcardNum();
}

void Dashboard::_addHandCard(CardItem *card_item, bool prepend, const QString &footnote) {
    //card item in dashboard should never be disabled
    if (!card_item->isEnabled())
        card_item->setEnabled(true);

    if (ClientInstance->getStatus() == Client::Playing && card_item->getCard()) {
        const bool frozen = !card_item->getCard()->isAvailable(Self);
        card_item->setFrozen(frozen, false);
        if (!frozen && Config.EnableSuperDrag)
            card_item->setFlag(ItemIsMovable);
    } else {
        card_item->setFrozen(true, false);
    }

    card_item->setHomeOpacity(1.0);
    card_item->setRotation(0.0);
    card_item->setFlag(ItemIsFocusable);
    card_item->setZValue(0.1);
    if (!footnote.isEmpty()) {
        card_item->setFootnote(footnote);
        card_item->showFootnote();
    }
    if (prepend)
        m_handCards.prepend(card_item);
    else
        m_handCards.append(card_item);

    connect(card_item, SIGNAL(clicked()), this, SLOT(onCardItemClicked()));
    connect(card_item, SIGNAL(double_clicked()), this, SLOT(onCardItemDoubleClicked()));
    connect(card_item, SIGNAL(thrown()), this, SLOT(onCardItemThrown()));
    connect(card_item, SIGNAL(enter_hover()), this, SLOT(bringSenderToTop()));
    connect(card_item, SIGNAL(leave_hover()), this, SLOT(resetSenderZValue()));

    card_item->setOuterGlowEffectEnabled(true);

    //Make sure that the card is a handcard.
    if (Self->getHandcards().contains(card_item->getCard())
            && card_item->getCard()->isTransferable()) {
        card_item->setTransferable(true);
        if (card_item->getTransferButton() && !_transferButtons.contains(card_item->getTransferButton()))
            _transferButtons << card_item->getTransferButton();
    }
}

void Dashboard::_createRoleComboBox() {
    _m_roleComboBox = new RoleComboBox(rightFrame, true);
}

void Dashboard::selectCard(const QString &pattern, bool forward, bool multiple) {
    if (!multiple && selected && selected->isSelected())
        selected->clickItem();

    // find all cards that match the card type
    QList<CardItem *> matches;
    foreach(CardItem *card_item, m_handCards) {
        if (!card_item->isFrozen() && (pattern == "." || card_item->getCard()->match(pattern)))
            matches << card_item;
    }

    if (matches.isEmpty()) {
        if (!multiple || !selected) {
            unselectAll();
            return;
        }
    }

    int index = matches.indexOf(selected), n = matches.length();
    index = (index + (forward ? 1 : n - 1)) % n;

    CardItem *to_select = matches[index];
    if (!to_select->isSelected())
        to_select->clickItem();
    else if (to_select->isSelected() && (!multiple || (multiple && to_select != selected)))
        to_select->clickItem();
    selected = to_select;

    adjustCards();
}

void Dashboard::selectEquip(int position) {
    int i = position - 1;
    if (_m_equipCards[i] && _m_equipCards[i]->isMarkable()) {
        _m_equipCards[i]->mark(!_m_equipCards[i]->isMarked());
        update();
    }
}

void Dashboard::selectOnlyCard(bool need_only) {
    if (selected && selected->isSelected())
        selected->clickItem();

    int count = 0;

    QList<CardItem *> items;
    foreach(CardItem *card_item, m_handCards) {
        if (!card_item->isFrozen()) {
            items << card_item;
            count++;
            if (need_only && count > 1) {
                unselectAll();
                return;
            }
        }
    }

    QList<int> equip_pos;
    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (_m_equipCards[i] && _m_equipCards[i]->isMarkable()) {
            equip_pos << i;
            count++;
            if (need_only && count > 1) return;
        }
    }
    if (count == 0) return;
    if (!items.isEmpty()) {
        CardItem *item = items.first();
        item->clickItem();
        selected = item;
        adjustCards();
    }
    else if (!equip_pos.isEmpty()) {
        int pos = equip_pos.first();
        _m_equipCards[pos]->mark(!_m_equipCards[pos]->isMarked());
        update();
    }
}

const Card *Dashboard::getSelected() const{
    if (viewAsSkill)
        return pendingCard;
    else if (selected)
        return selected->getCard();
    else
        return NULL;
}

void Dashboard::selectCard(CardItem *item, bool isSelected) {
    bool oldState = item->isSelected();
    if (oldState == isSelected) return;
    m_mutex.lock();

    item->setSelected(isSelected);
    QPointF oldPos = item->homePos();
    QPointF newPos = oldPos;
    newPos.setY(newPos.y() + (isSelected ? 1 : -1) * S_PENDING_OFFSET_Y);
    item->setHomePos(newPos);
    selected = item;

    m_mutex.unlock();
}

void Dashboard::unselectAll(const CardItem *except) {
    selected = NULL;

    foreach(CardItem *card_item, m_handCards) {
        if (card_item != except)
            selectCard(card_item, false);
    }

    adjustCards(true);
    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (_m_equipCards[i] && _m_equipCards[i] != except)
            _m_equipCards[i]->mark(false);
    }
    if (viewAsSkill) {
        pendings.clear();
        updatePending();
    }
}

QRectF Dashboard::boundingRect() const{
    return QRectF(0, 0, width, _m_layout->m_normalHeight);
}

void Dashboard::setWidth(int width) {
    prepareGeometryChange();
    adjustCards(true);
    this->width = width;
    _updateFrames();
    _updateDeathIcon();
}

QSanSkillButton *Dashboard::addSkillButton(const QString &skillName, const bool &head) {
    // if it's a equip skill, add it to equip bar
    _mutexEquipAnim.lock();

    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (!_m_equipCards[i]) continue;
        const EquipCard *equip = qobject_cast<const EquipCard *>(_m_equipCards[i]->getCard()->getRealCard());
        Q_ASSERT(equip);
        // @todo: we must fix this in the server side - add a skill to the card itself instead
        // of getting it from the engine.
        const Skill *skill = Sanguosha->getSkill(equip);
        if (skill == NULL) continue;
        if (skill->objectName() == skillName) {
            // If there is already a button there, then we haven't removed the last skill before attaching
            // a new one. The server must have sent the requests out of order. So crash.
            Q_ASSERT(_m_equipSkillBtns[i] == NULL);
            _m_equipSkillBtns[i] = new QSanInvokeSkillButton(this);
            _m_equipSkillBtns[i]->setSkill(skill);
            _m_equipSkillBtns[i]->setVisible(false);
            connect(_m_equipSkillBtns[i], SIGNAL(clicked()), this, SLOT(_onEquipSelectChanged()));
            connect(_m_equipSkillBtns[i], SIGNAL(enable_changed()), this, SLOT(_onEquipSelectChanged()));
            QSanSkillButton *btn = _m_equipSkillBtns[i];
            _mutexEquipAnim.unlock();
            return btn;
        }
    }
    _mutexEquipAnim.unlock();
#ifndef QT_NO_DEBUG
    const Skill *skill = Sanguosha->getSkill(skillName);
    Q_ASSERT(skill && !skill->inherits("WeaponSkill") && !skill->inherits("ArmorSkill"));
#endif
    if (rightSkillDock->getSkillButtonByName(skillName)) {
        //_m_button_recycle.append(_m_rightSkillDock->getSkillButtonByName(skillName));
        return NULL;
    }
    else if (leftSkillDock->getSkillButtonByName(skillName)) {
        //_m_button_recycle.append(_m_leftSkillDock->getSkillButtonByName(skillName));
        return NULL;
    }
    QSanInvokeSkillDock *dock = NULL;
    // The directions 'left' and 'right' here are opposite to the actual convention,
    // for we have swapped the positions of avatars. Names of the two properties should
    // be corrected later.
    if (Self->ownSkill(skillName) || Self->getAcquiredSkills().contains(skillName))
        dock = Self->inHeadSkills(skillName) ? rightSkillDock : leftSkillDock;
    else
        dock = head ? rightSkillDock : leftSkillDock;
    return dock->addSkillButtonByName(skillName);
}

QSanSkillButton *Dashboard::removeSkillButton(const QString &skillName) {
    QSanSkillButton *btn = NULL;
    _mutexEquipAnim.lock();
    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (!_m_equipSkillBtns[i]) continue;
        const Skill *skill = _m_equipSkillBtns[i]->getSkill();
        Q_ASSERT(skill != NULL);
        if (skill->objectName() == skillName) {
            btn = _m_equipSkillBtns[i];
            _m_equipSkillBtns[i] = NULL;
            continue;
        }
    }
    _mutexEquipAnim.unlock();
    if (btn == NULL) {
        QSanInvokeSkillDock *dock = rightSkillDock;
        QSanSkillButton *temp = rightSkillDock->getSkillButtonByName(skillName);
        if (temp == NULL) {
            temp = leftSkillDock->getSkillButtonByName(skillName);
            dock = leftSkillDock;
        }
        btn = dock->removeSkillButtonByName(skillName);
    }
    return btn;
}

void Dashboard::highlightEquip(QString skillName, bool highlight) {
    int i = 0;
    for (i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (!_m_equipCards[i]) continue;
        if (_m_equipCards[i]->getCard()->objectName() == skillName)
            break;
    }
    if (i != 5)
        _setEquipBorderAnimation(i, highlight);
}

void Dashboard::setPlayer(ClientPlayer *player) {
    PlayerCardContainer::setPlayer(player);
    connect(player, SIGNAL(head_state_changed()), this, SLOT(onHeadStateChanged()));
    connect(player, SIGNAL(deputy_state_changed()), this, SLOT(onDeputyStateChanged()));
}

void Dashboard::_createExtraButtons() {
    m_trustButton = new QSanButton("handcard", "trust", this, true);
    m_trustButton->setStyle(QSanButton::S_STYLE_TOGGLE);
    m_btnReverseSelection = new QSanButton("handcard", "reverse-selection", this);
    m_btnSortHandcard = new QSanButton("handcard", "sort", this);
    m_btnNoNullification = new QSanButton("handcard", "nullification", this, true);
    m_btnNoNullification->setStyle(QSanButton::S_STYLE_TOGGLE);
    m_trustButton->setPos(G_DASHBOARD_LAYOUT.m_rswidth, -m_trustButton->boundingRect().height());
    m_btnReverseSelection->setPos(m_trustButton->boundingRect().width() + G_DASHBOARD_LAYOUT.m_rswidth, -m_trustButton->boundingRect().height());
    m_btnSortHandcard->setPos(m_trustButton->boundingRect().width() + m_btnReverseSelection->boundingRect().width() + G_DASHBOARD_LAYOUT.m_rswidth,
        -m_trustButton->boundingRect().height());
    m_btnNoNullification->setPos(m_btnReverseSelection->boundingRect().width() + m_btnReverseSelection->boundingRect().width() + m_btnSortHandcard->boundingRect().width() + G_DASHBOARD_LAYOUT.m_rswidth,
        -m_trustButton->boundingRect().height());

    m_trustButton->hide();
    m_btnReverseSelection->hide();
    m_btnSortHandcard->hide();
    m_btnNoNullification->hide();
    connect(m_trustButton, SIGNAL(clicked()), RoomSceneInstance, SLOT(trust()));
    connect(Self, SIGNAL(state_changed()), this, SLOT(updateTrustButton()));
    connect(m_btnReverseSelection, SIGNAL(clicked()), this, SLOT(reverseSelection()));
    connect(m_btnSortHandcard, SIGNAL(clicked()), this, SLOT(sortCards()));
    connect(m_btnNoNullification, SIGNAL(clicked()), this, SLOT(cancelNullification()));
}

void Dashboard::showSeat() {
    const QRect region = G_DASHBOARD_LAYOUT.m_seatIconRegion;
    PixmapAnimation *pma = PixmapAnimation::GetPixmapAnimation(rightFrame, "seat");
    if (pma) {
        pma->setTransform(QTransform::fromTranslate(-pma->boundingRect().width() / 2, -pma->boundingRect().height() / 2));
        pma->setPos(region.x() + region.width() / 2, region.y() + region.height() / 2);
    }
    _paintPixmap(_m_seatItem, region,
        _getPixmap(QSanRoomSkin::S_SKIN_KEY_SEAT_NUMBER, QString::number(m_player->getSeat())),
        rightFrame);
    //save the seat number for later use
    m_player->setProperty("UI_Seat", m_player->getSeat());
    _m_seatItem->setZValue(1.1);
}

void Dashboard::clearPendings() {
    selected = NULL;
    foreach (CardItem *item, m_handCards)
        selectCard(item, false);
    pendings.clear();
}

QList<TransferButton *> Dashboard::getTransferButtons() const
{
    return _transferButtons;
}

void Dashboard::skillButtonActivated() {
    QSanSkillButton *button = qobject_cast<QSanSkillButton *>(sender());
    QList<QSanInvokeSkillButton *> buttons = rightSkillDock->getAllSkillButtons()
        + leftSkillDock->getAllSkillButtons();
    foreach(QSanSkillButton *btn, buttons) {
        if (button == btn) continue;

        if (btn->getViewAsSkill() != NULL && btn->isDown())
            btn->setState(QSanButton::S_STATE_UP);
    }

    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (button == _m_equipSkillBtns[i]) continue;

        if (_m_equipSkillBtns[i] != NULL)
            _m_equipSkillBtns[i]->setEnabled(false);
    }
}

void Dashboard::skillButtonDeactivated() {
    QList<QSanInvokeSkillButton *> buttons = rightSkillDock->getAllSkillButtons()
        + leftSkillDock->getAllSkillButtons();
    foreach(QSanSkillButton *btn, buttons) {
        if (btn->getViewAsSkill() != NULL && btn->isDown())
            btn->setState(QSanButton::S_STATE_UP);
    }

    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (_m_equipSkillBtns[i] != NULL) {
            _m_equipSkillBtns[i]->setEnabled(true);
            if (_m_equipSkillBtns[i]->isDown())
                _m_equipSkillBtns[i]->click();
        }
    }

    foreach (TransferButton *button, _transferButtons) {
        if (button->isDown())
            button->setState(QSanButton::S_STATE_UP);
    }
}

void Dashboard::selectAll() {
    retractPileCards("wooden_ox");
    selectCards(".");
}

void Dashboard::selectCards(const QString &pattern) {
    if (viewAsSkill) {
        unselectAll();
        foreach(CardItem *card_item, m_handCards) {
            if (!Sanguosha->matchExpPattern(pattern, Self, card_item->getCard())) continue;
            selectCard(card_item, true);
            pendings << card_item;
        }
        updatePending();
    }
    adjustCards(true);
}

void Dashboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void Dashboard::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
    PlayerCardContainer::mouseReleaseEvent(mouseEvent);

    CardItem *to_select = NULL;
    int i;
    for (i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (_m_equipRegions[i]->isUnderMouse()) {
            to_select = _m_equipCards[i];
            break;
        }
    }
    if (!to_select) return;
    if (_m_equipSkillBtns[i] != NULL && _m_equipSkillBtns[i]->isEnabled())
        _m_equipSkillBtns[i]->click();
    else if (to_select->isMarkable()) {
        // According to the game rule, you cannot select a weapon as a card when
        // you are invoking the skill of that equip. So something must be wrong.
        // Crash.
        Q_ASSERT(_m_equipSkillBtns[i] == NULL || !_m_equipSkillBtns[i]->isDown());
        to_select->mark(!to_select->isMarked());
        update();
    }
}

void Dashboard::_onEquipSelectChanged() {
    QSanSkillButton *btn = qobject_cast<QSanSkillButton *>(sender());
    if (btn) {
        for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
            if (_m_equipSkillBtns[i] == btn) {
                _setEquipBorderAnimation(i, btn->isDown());
                break;
            }
        }
    } else {
        CardItem *equip = qobject_cast<CardItem *>(sender());
        // Do not remove this assertion. If equip is NULL here, some other
        // sources that could select equip has not been considered and must
        // be implemented.
        Q_ASSERT(equip);
        for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
            if (_m_equipCards[i] == equip) {
                _setEquipBorderAnimation(i, equip->isMarked());
                break;
            }
        }
    }
}

void Dashboard::_createEquipBorderAnimations() {
    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        _m_equipBorders[i] = new PixmapAnimation();
        _m_equipBorders[i]->setParentItem(_getEquipParent());
        _m_equipBorders[i]->setPath("image/system/emotion/equipborder/");
        if (!_m_equipBorders[i]->valid()) {
            delete _m_equipBorders[i];
            _m_equipBorders[i] = NULL;
            continue;
        }
        _m_equipBorders[i]->setPos(layout->m_equipBorderPos + layout->m_equipSelectedOffset + layout->m_equipAreas[i].topLeft());
        _m_equipBorders[i]->hide();
    }
}

void Dashboard::_setEquipBorderAnimation(int index, bool turnOn) {
    _mutexEquipAnim.lock();
    if (_m_isEquipsAnimOn[index] == turnOn) {
        _mutexEquipAnim.unlock();
        return;
    }

    QPoint newPos;
    if (turnOn)
        newPos = layout->m_equipSelectedOffset + layout->m_equipAreas[index].topLeft();
    else
        newPos = layout->m_equipAreas[index].topLeft();

    _m_equipAnim[index]->stop();
    _m_equipAnim[index]->clear();
    QPropertyAnimation *anim = new QPropertyAnimation(_m_equipRegions[index], "pos");
    anim->setEndValue(newPos);
    anim->setDuration(200);
    _m_equipAnim[index]->addAnimation(anim);
    anim = new QPropertyAnimation(_m_equipRegions[index], "opacity");
    anim->setEndValue(255);
    anim->setDuration(200);
    _m_equipAnim[index]->addAnimation(anim);
    _m_equipAnim[index]->start();

    Q_ASSERT(_m_equipBorders[index]);
    if (turnOn) {
        _m_equipBorders[index]->show();
        _m_equipBorders[index]->start();
    }
    else {
        _m_equipBorders[index]->hide();
        _m_equipBorders[index]->stop();
    }

    _m_isEquipsAnimOn[index] = turnOn;
    _mutexEquipAnim.unlock();
}

void Dashboard::_initializeRemovedEffect()
{
    _removedEffect = new QPropertyAnimation(this, "opacity", this);
    _removedEffect->setDuration(2000);
    _removedEffect->setEasingCurve(QEasingCurve::OutInBounce);
    _removedEffect->setEndValue(0.6);
    _removedEffect->setStartValue(1.0);
}

void Dashboard::showHeroSkinListHelper(const General *general,
                                       HeroSkinContainer * &heroSkinContainer)
{
    if (NULL == general) {
        return;
    }

    QString generalName = general->objectName();
    if (NULL == heroSkinContainer) {
        heroSkinContainer = RoomSceneInstance->findHeroSkinContainer(generalName);

        if (NULL == heroSkinContainer) {
            heroSkinContainer = new HeroSkinContainer(generalName, general->getKingdom());

            RoomSceneInstance->addHeroSkinContainer(heroSkinContainer);
            RoomSceneInstance->addItem(heroSkinContainer);

            heroSkinContainer->setPos(getHeroSkinContainerPosition());
            RoomSceneInstance->bringToFront(heroSkinContainer);
        }
    }

    if (!heroSkinContainer->isVisible()) {
        heroSkinContainer->show();
    }

    heroSkinContainer->bringToTopMost();
}

QPointF Dashboard::getHeroSkinContainerPosition() const
{
    QRectF avatarParentRect = rightFrame->sceneBoundingRect();
    QRectF heroSkinContainerRect = (m_headHeroSkinContainer != NULL)
        ? m_headHeroSkinContainer->boundingRect()
        : m_deputyHeroSkinContainer->boundingRect();;
    return QPointF(avatarParentRect.left() - heroSkinContainerRect.width() - 120,
        avatarParentRect.bottom() - heroSkinContainerRect.height() - 5);
}

void Dashboard::adjustCards(bool playAnimation) {
    _adjustCards();
    foreach(CardItem *card, m_handCards)
        card->goBack(playAnimation);
}

void Dashboard::_adjustCards() {
    int maxCards = Config.MaxCards;

    int n = m_handCards.length();
    if (n == 0) return;

    if (maxCards >= n)
        maxCards = n;
    else if (maxCards <= (n - 1) / 2 + 1)
        maxCards = (n - 1) / 2 + 1;
    QList<CardItem *> row;
    QSanRoomSkin::DashboardLayout *layout = (QSanRoomSkin::DashboardLayout *)_m_layout;
    int leftWidth = layout->m_leftWidth;
    int cardHeight = G_COMMON_LAYOUT.m_cardNormalHeight;
    int middleWidth = width - layout->m_leftWidth - layout->m_rightWidth - this->getButtonWidgetWidth();
    QRect rowRect = QRect(leftWidth, layout->m_normalHeight - cardHeight - 3, middleWidth, cardHeight);
    for (int i = 0; i < maxCards; i++)
        row.push_back(m_handCards[i]);

    _m_highestZ = n;
    _disperseCards(row, rowRect, Qt::AlignLeft, true, true);

    row.clear();
    rowRect.translate(0, 1.5 * S_PENDING_OFFSET_Y);
    for (int i = maxCards; i < n; i++)
        row.push_back(m_handCards[i]);

    _m_highestZ = 0;
    _disperseCards(row, rowRect, Qt::AlignLeft, true, true);

    for (int i = 0; i < n; i++) {
        CardItem *card = m_handCards[i];
        if (card->isSelected()) {
            QPointF newPos = card->homePos();
            newPos.setY(newPos.y() + S_PENDING_OFFSET_Y);
            card->setHomePos(newPos);
        }
    }
}

int Dashboard::getMiddleWidth() {
    return width - G_DASHBOARD_LAYOUT.m_leftWidth - G_DASHBOARD_LAYOUT.m_rightWidth;
}

QList<CardItem *> Dashboard::cloneCardItems(QList<int> card_ids) {
    QList<CardItem *> result;
    CardItem *card_item;
    CardItem *new_card;

    foreach(int card_id, card_ids) {
        new_card = _createCard(card_id);

        card_item = CardItem::FindItem(m_handCards, card_id);
        if (card_item) {
            new_card->setPos(card_item->pos());
            new_card->setHomePos(card_item->homePos());
        }

        result.append(new_card);
    }
    return result;
}

QList<CardItem *> Dashboard::removeHandCards(const QList<int> &card_ids) {
    QList<CardItem *> result;
    CardItem *card_item;
    foreach(int card_id, card_ids) {
        card_item = CardItem::FindItem(m_handCards, card_id);
        if (card_item == selected) selected = NULL;
        Q_ASSERT(card_item);
        if (card_item) {
            m_handCards.removeOne(card_item);
            card_item->hideFrame();
            card_item->disconnect(this);
            card_item->setOuterGlowEffectEnabled(false);
            if (card_item->getCard()->isTransferable()) {
                card_item->setTransferable(false);
                _transferButtons.removeOne(card_item->getTransferButton());
            }
            // frozen is a fake disabled state only used in dashboard.
            // Disable the item when removing.
            if (card_item->isFrozen())
                card_item->setEnabled(false);
            result.append(card_item);
        }
    }
    updateHandcardNum();
    return result;
}

QList<CardItem *> Dashboard::removeCardItems(const QList<int> &card_ids, Player::Place place) {
    CardItem *card_item = NULL;
    QList<CardItem *> result;
    if (place == Player::PlaceHand)
        result = removeHandCards(card_ids);
    else if (place == Player::PlaceEquip)
        result = removeEquips(card_ids);
    else if (place == Player::PlaceDelayedTrick)
        result = removeDelayedTricks(card_ids);
    else if (place == Player::PlaceSpecial) {
        foreach(int card_id, card_ids) {
            card_item = _createCard(card_id);
            card_item->setOpacity(0.0);
            result.push_back(card_item);
        }
    } else {
        Q_ASSERT(false);
    }

    Q_ASSERT(result.size() == card_ids.size());
    if (place == Player::PlaceHand)
        adjustCards();
    else if (result.size() > 1 || place == Player::PlaceSpecial) {
        QRect rect(0, 0, layout->m_disperseWidth, 0);
        QPointF center(0, 0);
        if (place == Player::PlaceEquip || place == Player::PlaceDelayedTrick) {
            for (int i = 0; i < result.size(); i++)
                center += result[i]->pos();
            center = 1.0 / result.length() * center;
        }
        else if (place == Player::PlaceSpecial)
            center = mapFromItem(_getAvatarParent(), layout->m_secondaryAvatarArea.center());
        else
            Q_ASSERT(false);
        rect.moveCenter(center.toPoint());
        _disperseCards(result, rect, Qt::AlignCenter, false, false);
    }
    update();
    return result;
}

static bool CompareByNumber(const CardItem *a, const CardItem *b)  {
    return Card::CompareByNumber(a->getCard(), b->getCard());
}

static bool CompareBySuit(const CardItem *a, const CardItem *b)  {
    return Card::CompareBySuit(a->getCard(), b->getCard());
}

static bool CompareByType(const CardItem *a, const CardItem *b)  {
    return Card::CompareByType(a->getCard(), b->getCard());
}

void Dashboard::sortCards() {
    if (m_handCards.length() == 0) return;

    QMenu *menu = _m_sort_menu;
    menu->clear();
    menu->setTitle(tr("Sort handcards"));

    QAction *action1 = menu->addAction(tr("Sort by type"));
    action1->setData((int)ByType);

    QAction *action2 = menu->addAction(tr("Sort by suit"));
    action2->setData((int)BySuit);

    QAction *action3 = menu->addAction(tr("Sort by number"));
    action3->setData((int)ByNumber);

    connect(action1, SIGNAL(triggered()), this, SLOT(beginSorting()));
    connect(action2, SIGNAL(triggered()), this, SLOT(beginSorting()));
    connect(action3, SIGNAL(triggered()), this, SLOT(beginSorting()));

    QPointF posf = QCursor::pos();
    menu->popup(QPoint(posf.x(), posf.y()));
}

void Dashboard::beginSorting() {
    QAction *action = qobject_cast<QAction *>(sender());
    SortType type = ByType;
    if (action)
        type = (SortType)(action->data().toInt());

    switch (type) {
    case ByType: qSort(m_handCards.begin(), m_handCards.end(), CompareByType); break;
    case BySuit: qSort(m_handCards.begin(), m_handCards.end(), CompareBySuit); break;
    case ByNumber: qSort(m_handCards.begin(), m_handCards.end(), CompareByNumber); break;
    default: Q_ASSERT(false);
    }

    adjustCards();
}

void Dashboard::reverseSelection() {
    if (!viewAsSkill) return;

    QList<CardItem *> selected_items;
    foreach(CardItem *item, m_handCards){
        if (item->isSelected()) {
            item->clickItem();
            selected_items << item;
        }
    }
    foreach(CardItem *item, m_handCards)
        if (!item->isFrozen() && !selected_items.contains(item))
            item->clickItem();
    adjustCards();
}


void Dashboard::cancelNullification() {
    ClientInstance->m_noNullificationThisTime = !ClientInstance->m_noNullificationThisTime;
    if (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE
        && Sanguosha->getCurrentCardUsePattern() == "nullification"
        && RoomSceneInstance->isCancelButtonEnabled()) {
        RoomSceneInstance->doCancelButton();
    }
}

void Dashboard::controlNullificationButton() {
    if (ClientInstance->getReplayer()) return;
    m_btnNoNullification->setState(QSanButton::S_STATE_UP);
}

void Dashboard::disableAllCards() {
    m_mutexEnableCards.lock();
    foreach(CardItem *card_item, m_handCards)
        card_item->setFrozen(true, false);
    m_mutexEnableCards.unlock();
}

void Dashboard::enableCards() {
    m_mutexEnableCards.lock();
    expandPileCards("wooden_ox");
    foreach(CardItem *card_item, m_handCards) {
        const bool frozen = !card_item->getCard()->isAvailable(Self);
        card_item->setFrozen(frozen, false);
        if (!frozen && Config.EnableSuperDrag)
            card_item->setFlag(ItemIsMovable);
    }
    m_mutexEnableCards.unlock();
}

void Dashboard::enableAllCards() {
    m_mutexEnableCards.lock();
    foreach(CardItem *card_item, m_handCards) {
        card_item->setFrozen(false, false);
        if (Config.EnableSuperDrag)
            card_item->setFlag(ItemIsMovable);
    }
    m_mutexEnableCards.unlock();
}

void Dashboard::startPending(const ViewAsSkill *skill) {
    m_mutexEnableCards.lock();
    viewAsSkill = skill;
    pendings.clear();
    unselectAll();

    bool expand = (skill && skill->isResponseOrUse());
    if (!expand && skill && skill->inherits("ResponseSkill")) {
        const ResponseSkill *resp_skill = qobject_cast<const ResponseSkill *>(skill);
        if (resp_skill && (resp_skill->getRequest() == Card::MethodResponse || resp_skill->getRequest() == Card::MethodUse))
            expand = true;
    }
    if (expand)
        expandPileCards("wooden_ox");
    else {
        retractPileCards("wooden_ox");
        if (skill && !skill->getExpandPile().isEmpty()) {
            foreach (QString pile_name, skill->getExpandPile().split(","))
                expandPileCards(pile_name);
        }
    }

    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        if (_m_equipCards[i] != NULL)
            connect(_m_equipCards[i], SIGNAL(mark_changed()), this, SLOT(onMarkChanged()));
    }

    updatePending();
    m_mutexEnableCards.unlock();
}

void Dashboard::stopPending() {
    m_mutexEnableCards.lock();

    if (viewAsSkill && !viewAsSkill->getExpandPile().isEmpty()) {
        foreach (QString pile_name, viewAsSkill->getExpandPile().split(","))
            retractPileCards(pile_name);
    }

    viewAsSkill = NULL;
    pendingCard = NULL;
    retractPileCards("wooden_ox");
    emit card_selected(NULL);

    foreach(CardItem *item, m_handCards)
        item->setFrozen(true, false);

    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        CardItem *equip = _m_equipCards[i];
        if (equip != NULL) {
            equip->mark(false);
            equip->setMarkable(false);
            _m_equipRegions[i]->setOpacity(1.0);
            equip->setEnabled(false);
            disconnect(equip, SIGNAL(mark_changed()));
        }
    }
    pendings.clear();
    adjustCards(true);
    m_mutexEnableCards.unlock();
}

void Dashboard::expandPileCards(const QString &pile_name) {
    if (_m_pile_expanded.contains(pile_name)) return;
    _m_pile_expanded << pile_name;
    QString new_name = pile_name;
    QList<int> pile;
    if (new_name.startsWith("%")) {
        new_name = new_name.mid(1);
        foreach (const Player *p, Self->getAliveSiblings())
           pile += p->getPile(new_name);
    } else {
        pile = Self->getPile(new_name);
    }
    if (pile.isEmpty()) return;
    QList<CardItem *> card_items = _createCards(pile);
    foreach (CardItem *card_item, card_items) {
        card_item->setPos(mapFromScene(card_item->scenePos()));
        card_item->setParentItem(this);
    }
    foreach (CardItem *card_item, card_items)
        _addHandCard(card_item, true, Sanguosha->translate(new_name));
    adjustCards();
    _playMoveCardsAnimation(card_items, false);
    update();
}

void Dashboard::retractPileCards(const QString &pile_name) {
    if (!_m_pile_expanded.contains(pile_name)) return;
    _m_pile_expanded.removeOne(pile_name);
    QString new_name = pile_name;
    QList<int> pile;
    if (new_name.startsWith("%")) {
        new_name = new_name.mid(1);
        foreach (const Player *p, Self->getAliveSiblings())
            pile += p->getPile(new_name);
    } else {
        pile = Self->getPile(new_name);
    }
    if (pile.isEmpty()) return;
    CardItem *card_item;
    foreach (int card_id, pile) {
        card_item = CardItem::FindItem(m_handCards, card_id);
        if (card_item == selected) selected = NULL;
        Q_ASSERT(card_item);
        if (card_item) {
            m_handCards.removeOne(card_item);
            card_item->disconnect(this);
            delete card_item;
            card_item = NULL;
        }
    }
    adjustCards();
    update();
}

void Dashboard::onCardItemClicked() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (!card_item) return;

    if (viewAsSkill) {
        if (card_item->isSelected()) {
            selectCard(card_item, false);
            pendings.removeOne(card_item);
            if (viewAsSkill->inherits("TransferSkill"))
                RoomSceneInstance->doCancelButton();
        } else {
            if (viewAsSkill->inherits("OneCardViewAsSkill"))
                unselectAll();
            selectCard(card_item, true);
            pendings << card_item;
        }

        updatePending();
    } else {
        if (card_item->isSelected()) {
            unselectAll();
            emit card_selected(NULL);
        } else {
            unselectAll();
            selectCard(card_item, true);
            selected = card_item;

            emit card_selected(selected->getCard());
        }
    }
}

void Dashboard::updatePending() {
    if (!viewAsSkill) return;
    QList<const Card *> cards;
    foreach(CardItem *item, pendings)
        cards.append(item->getCard());

    QList<const Card *> pended;
    if (!viewAsSkill->inherits("OneCardViewAsSkill"))
        pended = cards;
    foreach(CardItem *item, m_handCards) {
        if (!item->isSelected() || pendings.isEmpty()) {
            const bool frozen = !viewAsSkill->viewFilter(pended, item->getCard());
            item->setFrozen(frozen, false);
            if (!frozen && Config.EnableSuperDrag)
                item->setFlag(ItemIsMovable);
        }
    }

    for (int i = 0; i < S_EQUIP_AREA_LENGTH; i++) {
        CardItem *equip = _m_equipCards[i];
        if (equip && !equip->isMarked())
            equip->setMarkable(viewAsSkill->viewFilter(pended, equip->getCard()));
        if (equip) {
            if (!equip->isMarkable() && (!_m_equipSkillBtns[i] || !_m_equipSkillBtns[i]->isEnabled()))
                _m_equipRegions[i]->setOpacity(0.7);
            else
                _m_equipRegions[i]->setOpacity(1.0);
        }
    }

    const Card *new_pending_card = viewAsSkill->viewAs(cards);
    if (pendingCard != new_pending_card) {
        if (pendingCard && !pendingCard->parent() && pendingCard->isVirtualCard()) {
            delete pendingCard;
            pendingCard = NULL;
        }
        pendingCard = new_pending_card;
        emit card_selected(pendingCard);
    }
}

void Dashboard::onCardItemDoubleClicked() {
    if (!Config.EnableDoubleClick) return;
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        if (!viewAsSkill) selected = card_item;
        emit card_to_use();
    }
}

void Dashboard::onCardItemThrown() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        if (!viewAsSkill) selected = card_item;
        emit card_to_use();
    }
}

void Dashboard::onMarkChanged() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());

    Q_ASSERT(card_item->isEquipped());

    if (card_item) {
        if (card_item->isMarked()) {
            if (!pendings.contains(card_item)) {
                if (viewAsSkill && viewAsSkill->inherits("OneCardViewAsSkill"))
                    unselectAll(card_item);
                pendings.append(card_item);
            }
        }
        else
            pendings.removeOne(card_item);

        updatePending();
    }
}

void Dashboard::onHeadStateChanged() {
    if (m_player && RoomSceneInstance->game_started && !m_player->hasShownGeneral1())
        _m_shadow_layer1->setBrush(G_DASHBOARD_LAYOUT.m_generalShadowColor);
    else
        _m_shadow_layer1->setBrush(Qt::NoBrush);
    onHeadSkillPreshowed();
}

void Dashboard::onDeputyStateChanged() {
    if (m_player && RoomSceneInstance->game_started && !m_player->hasShownGeneral2())
        _m_shadow_layer2->setBrush(G_DASHBOARD_LAYOUT.m_generalShadowColor);
    else
        _m_shadow_layer2->setBrush(Qt::NoBrush);
    onDeputySkillPreshowed();
}

void Dashboard::onHeadSkillPreshowed() {
    if (m_player && RoomSceneInstance->game_started && !m_player->hasShownGeneral1())
        leftHiddenMark->setVisible(m_player->isHidden(true));
    else
        leftHiddenMark->setVisible(false);
}

void Dashboard::onDeputySkillPreshowed() {
    if (m_player && RoomSceneInstance->game_started && !m_player->hasShownGeneral2())
        rightHiddenMark->setVisible(m_player->isHidden(false));
    else
        rightHiddenMark->setVisible(false);
}

void Dashboard::updateTrustButton() {
    if (!ClientInstance->getReplayer()) {
        bool trusting = Self->getState() == "trust";
        m_trustButton->update();
        setTrust(trusting);
    }
}

void Dashboard::bringSenderToTop()
{
    CardItem *item = qobject_cast<CardItem *>(sender());

    Q_ASSERT(item);
    item->setData(CARDITEM_Z_DATA_KEY, item->zValue());
    item->setZValue(1000);
}

void Dashboard::resetSenderZValue()
{
    CardItem *item = qobject_cast<CardItem *>(sender());

    Q_ASSERT(item);
    const int z = item->data(CARDITEM_Z_DATA_KEY).toInt();
    item->setZValue(z);
}

void Dashboard::showHeroSkinList()
{
    if (NULL != m_player) {
        if (sender() == m_changeHeadHeroSkinButton) {
            showHeroSkinListHelper(m_player->getGeneral(),
                                   m_headHeroSkinContainer);
        } else {
            showHeroSkinListHelper(m_player->getGeneral2(),
                m_deputyHeroSkinContainer);
        }
    }
}

void Dashboard::heroSkinButtonMouseOutsideClicked()
{
    if (NULL != m_player) {
        QSanButton *heroSKinBtn = NULL;
        if (sender() == m_changeHeadHeroSkinButton) {
            heroSKinBtn = m_changeHeadHeroSkinButton;
        } else {
            heroSKinBtn = m_changeDeputyHeroSkinButton;
        }

        QGraphicsItem *parent = heroSKinBtn->parentItem();
        if (NULL != parent && !parent->isUnderMouse()) {
            heroSKinBtn->hide();

            /*if (Self == m_player && NULL != _m_screenNameItem && _m_screenNameItem->isVisible()) {
                _m_screenNameItem->hide();
            }*/
        }
    }
}

void Dashboard::onAvatarHoverEnter() {
    if (NULL != m_player) {
        QObject *senderObj = sender();

        const General *general = NULL;
        GraphicsPixmapHoverItem *avatarItem = NULL;
        QSanButton *heroSKinBtn = NULL;

        if (senderObj == _m_avatarIcon) {
            general = m_player->getGeneral();
            avatarItem = _m_avatarIcon;
            heroSKinBtn = m_changeHeadHeroSkinButton;

            m_changeDeputyHeroSkinButton->hide();
        } else if (senderObj == _m_smallAvatarIcon) {
            general = m_player->getGeneral2();
            avatarItem = _m_smallAvatarIcon;
            heroSKinBtn = m_changeDeputyHeroSkinButton;

            m_changeHeadHeroSkinButton->hide();
        }

        if (NULL != general && HeroSkinContainer::hasSkin(general->objectName())
            && avatarItem->isSkinChangingFinished()) {
                heroSKinBtn->show();
        }
    }
}

void Dashboard::onAvatarHoverLeave()
{
    if (NULL != m_player) {
        QObject *senderObj = sender();

        QSanButton *heroSKinBtn = NULL;

        if (senderObj == _m_avatarIcon)
            heroSKinBtn = m_changeHeadHeroSkinButton;
        else if (senderObj == _m_smallAvatarIcon)
            heroSKinBtn = m_changeDeputyHeroSkinButton;

        if ((NULL != heroSKinBtn) && (!heroSKinBtn->isMouseInside())) {
            heroSKinBtn->hide();
            //doAvatarHoverLeave();
        }
    }
}

void Dashboard::onSkinChangingStart()
{
    QSanButton *heroSKinBtn = NULL;
    QString generalName;

    if (sender() == _m_avatarIcon) {
        heroSKinBtn = m_changeHeadHeroSkinButton;
        generalName = m_player->getGeneralName();
    } else {
        heroSKinBtn = m_changeDeputyHeroSkinButton;
        generalName = m_player->getGeneral2Name();
    }

    heroSKinBtn->hide();
}

void Dashboard::onSkinChangingFinished()
{
    GraphicsPixmapHoverItem *avatarItem = NULL;
    QSanButton *heroSKinBtn = NULL;
    QSanInvokeSkillDock *skillDock = NULL;
    QString generalName;

    if (sender() == _m_avatarIcon) {
        avatarItem = _m_avatarIcon;
        heroSKinBtn = m_changeHeadHeroSkinButton;
        generalName = m_player->getGeneralName();
        skillDock = leftSkillDock;
    } else {
        avatarItem = _m_smallAvatarIcon;
        heroSKinBtn = m_changeDeputyHeroSkinButton;
        generalName = m_player->getGeneral2Name();
        skillDock = rightSkillDock;
    }

    if (avatarItem->isUnderMouse() && !skillDock->isUnderMouse())
        heroSKinBtn->show();
}

const ViewAsSkill *Dashboard::currentSkill() const{
    return viewAsSkill;
}

const Card *Dashboard::getPendingCard() const{
    return pendingCard;
}

void Dashboard::updateAvatar()
{
    if (_m_avatarIcon == NULL) {
        _m_avatarIcon = new GraphicsPixmapHoverItem(this, _getAvatarParent());
        _m_avatarIcon->setTransformationMode(Qt::SmoothTransformation);
    }

    const General *general = NULL;
    if (m_player) {
        general = m_player->getAvatarGeneral();
        //@@todo:design the style of screen name for dashboard
        /*
        _m_layout->m_screenNameFont.paintText(_m_screenNameItem,
        _m_layout->m_screenNameArea,
        Qt::AlignCenter,
        m_player->screenName());
        */
    } /*
        _m_layout->m_screenNameFont.paintText(_m_screenNameItem,
        _m_layout->m_screenNameArea,
        Qt::AlignCenter,
        QString());
        */

    QGraphicsPixmapItem *avatarIconTmp = _m_avatarIcon;
    if (general != NULL) {
        _m_avatarArea->setToolTip(m_player->getHeadSkillDescription());
        QString name = general->objectName();
        QPixmap avatarIcon = getHeadAvatarIcon(name);
        QRect area = _m_layout->m_avatarArea;
        area = QRect(area.left() + 2, area.top() + 1, area.width() - 2, area.height() - 3);
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

void Dashboard::updateSmallAvatar()
{
    if (_m_smallAvatarIcon == NULL) {
        _m_smallAvatarIcon = new GraphicsPixmapHoverItem(this, _getAvatarParent());
        _m_smallAvatarIcon->setTransformationMode(Qt::SmoothTransformation);
    }

    const General *general = NULL;
    if (m_player)
        general = m_player->getGeneral2();

    QGraphicsPixmapItem *smallAvatarIconTmp = _m_smallAvatarIcon;
    if (general != NULL) {
        _m_secondaryAvatarArea->setToolTip(m_player->getDeputySkillDescription());
        QString name = general->objectName();
        QPixmap avatarIcon = getHeadAvatarIcon(name);
        QRect area = _m_layout->m_secondaryAvatarArea;
        area = QRect(area.left() + 2, area.top() + 1, area.width() - 2, area.height() - 3);
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
    }
    else if (m_player->getAvatarGeneral() && m_player->getAvatarGeneral()->getKingdom() != "god") {
        QPixmap avatarIcon = getHeadAvatarIcon("deputy-" + m_player->getAvatarGeneral()->getKingdom());
        QRect area = _m_layout->m_secondaryAvatarArea;
        area = QRect(area.left() + 2, area.top() + 1, area.width() - 2, area.height() - 3);
        _paintPixmap(smallAvatarIconTmp, area, avatarIcon, _getAvatarParent());
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
