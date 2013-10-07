#include "dashboard.h"
#include "engine.h"
#include "settings.h"
#include "client.h"
#include "standard.h"
#include "playercarddialog.h"
#include "roomscene.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPixmapCache>
#include <QParallelAnimationGroup>

using namespace QSanProtocol;

Dashboard::Dashboard(QGraphicsItem *widget)
    : button_widget(widget), selected(NULL), view_as_skill(NULL), filter(NULL)
{
    Q_ASSERT(button_widget);
    _dlayout = &G_DASHBOARD_LAYOUT;
    _m_layout = _dlayout;
    m_player = Self;
    _m_leftFrame = _m_rightFrame = _m_middleFrame = NULL;
    _m_rightFrameBg = NULL;
    animations = new EffectAnimation();
    pending_card = NULL;
    for (int i = 0; i < 4; i++) {
        _m_equipSkillBtns[i] = NULL;
        _m_isEquipsAnimOn[i] = false;
    }
    // At this stage, we cannot decide the dashboard size yet, the whole
    // point in creating them here is to allow PlayerCardContainer to
    // anchor all controls and widgets to the correct frame.
    //
    // Note that 20 is just a random plug-in so that we can proceed with
    // control creation, the actual width is updated when setWidth() is
    // called by its graphics parent.
    //
    _m_width = G_DASHBOARD_LAYOUT.m_leftWidth + G_DASHBOARD_LAYOUT.m_rightWidth + 20;

    _createLeft();
    _createMiddle();
    _createRight();

    // only do this after you create all frames.
    _createControls();
    _createExtraButtons();

    _m_sort_menu = new QMenu(RoomSceneInstance->mainWindow());
}

bool Dashboard::isAvatarUnderMouse() {
    return _m_avatarArea->isUnderMouse();
}

void Dashboard::hideControlButtons() {
    m_btnReverseSelection->hide();
    m_btnSortHandcard->hide();
}

void Dashboard::showControlButtons() {
    m_btnReverseSelection->show();
    m_btnSortHandcard->show();
}

void Dashboard::showProgressBar(QSanProtocol::Countdown countdown) {
    _m_progressBar->setCountdown(countdown);
    connect(_m_progressBar, SIGNAL(timedOut()), this, SIGNAL(progressBarTimedOut()));
    _m_progressBar->show();
}

QGraphicsItem *Dashboard::getMouseClickReceiver() {
    return _m_avatarIcon;
}

void Dashboard::_createLeft() {
    QRect rect = QRect(0, 0, G_DASHBOARD_LAYOUT.m_leftWidth, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(_m_leftFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_LEFTFRAME), this);
    _m_leftFrame->setZValue(-1000); // nobody should be under me.
    _createEquipBorderAnimations();
}

int Dashboard::getButtonWidgetWidth() const{
    Q_ASSERT(button_widget);
    return button_widget->boundingRect().width();
}

void Dashboard::_createMiddle() {
    // this is just a random rect. see constructor for more details
    QRect rect = QRect(0, 0, 1, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(_m_middleFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_MIDDLEFRAME), this);
    _m_middleFrame->setZValue(-1000); // nobody should be under me.
    button_widget->setParentItem(_m_middleFrame);

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
    _layUnder(_m_rightFrame);
    _layUnder(_m_leftFrame);
    _layUnder(_m_middleFrame);
    _layBetween(button_widget, _m_middleFrame, _m_roleComboBox);
    _layBetween(_m_rightFrameBg, _m_faceTurnedIcon, _m_equipRegions[3]);
}

int Dashboard::width() {
    return this->_m_width;
}

void Dashboard::_createRight() {
    QRect rect = QRect(_m_width - G_DASHBOARD_LAYOUT.m_rightWidth, 0,
                       G_DASHBOARD_LAYOUT.m_rightWidth,
                       G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(_m_rightFrame, rect, QPixmap(1, 1), _m_groupMain);
    _paintPixmap(_m_rightFrameBg, QRect(0, 0, rect.width(), rect.height()),
                 _getPixmap(QSanRoomSkin::S_SKIN_KEY_RIGHTFRAME), _m_rightFrame);
    _m_rightFrame->setZValue(-1000); // nobody should be under me.

    _m_skillDock = new QSanInvokeSkillDock(_m_rightFrame);
    QRect avatar = G_DASHBOARD_LAYOUT.m_avatarArea;
    _m_skillDock->setPos(avatar.left(), avatar.bottom() +
                         G_DASHBOARD_LAYOUT.m_skillButtonsSize[0].height());
    _m_skillDock->setWidth(avatar.width());
}

void Dashboard::_updateFrames() {
    // Here is where we adjust all frames to actual width
    QRect rect = QRect(G_DASHBOARD_LAYOUT.m_leftWidth, 0,
        this->width() - G_DASHBOARD_LAYOUT.m_rightWidth - G_DASHBOARD_LAYOUT.m_leftWidth, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(_m_middleFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_MIDDLEFRAME), this);
    QRect rect2 = QRect(0, 0, this->width(), G_DASHBOARD_LAYOUT.m_normalHeight);
    trusting_item->setRect(rect2);
    trusting_item->setPos(0, 0);
    trusting_text->setPos((rect2.width() - Config.BigFont.pixelSize() * 4.5) / 2,
                          (rect2.height() - Config.BigFont.pixelSize()) / 2);
    _m_rightFrame->setX(_m_width - G_DASHBOARD_LAYOUT.m_rightWidth);
    Q_ASSERT(button_widget);
    button_widget->setX(rect.width() - getButtonWidgetWidth());
    button_widget->setY(0);
}

void Dashboard::setTrust(bool trust) {
    trusting_item->setVisible(trust);
    trusting_text->setVisible(trust);
}

void Dashboard::killPlayer() {
    trusting_item->hide();
    trusting_text->hide();
    _m_roleComboBox->fix(m_player->getRole());
    _m_roleComboBox->setEnabled(false);
    _updateDeathIcon();
    _m_saveMeIcon->hide();
    if (_m_votesItem) _m_votesItem->hide();
    if (_m_distanceItem) _m_distanceItem->hide();

    _m_deathIcon->show();
    if (ServerInfo.GameMode == "04_1v3" && !Self->isLord()) {
        _m_votesGot = 6;
        updateVotes(false);
    }
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
        foreach (CardItem *card, card_items)
            card->setHomeOpacity(0.0);
        QPointF center = mapFromItem(_getAvatarParent(), _dlayout->m_avatarArea.center());
        QRectF rect = QRectF(0, 0, _dlayout->m_disperseWidth, 0);
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
    foreach (CardItem *card_item, card_items)
        _addHandCard(card_item);
    updateHandcardNum();
}

void Dashboard::_addHandCard(CardItem *card_item) {
    if (ClientInstance->getStatus() == Client::Playing)
        card_item->setEnabled(card_item->getCard()->isAvailable(Self));
    else
        card_item->setEnabled(false);

    card_item->setHomeOpacity(1.0);
    card_item->setRotation(0.0);
    card_item->setFlags(ItemIsFocusable);
    card_item->setZValue(0.1);
    m_handCards << card_item;

    connect(card_item, SIGNAL(clicked()), this, SLOT(onCardItemClicked()));
    connect(card_item, SIGNAL(double_clicked()), this, SLOT(onCardItemDoubleClicked()));
    connect(card_item, SIGNAL(thrown()), this, SLOT(onCardItemThrown()));
    connect(card_item, SIGNAL(enter_hover()), this, SLOT(onCardItemHover()));
    connect(card_item, SIGNAL(leave_hover()), this, SLOT(onCardItemLeaveHover()));
}

void Dashboard::selectCard(const QString &pattern, bool forward, bool multiple) {
    if (!multiple && selected && selected->isSelected())
        selected->clickItem();

    // find all cards that match the card type
    QList<CardItem *> matches;
    foreach (CardItem *card_item, m_handCards) {
        if (card_item->isEnabled() && (pattern == "." || card_item->getCard()->match(pattern)))
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
    foreach (CardItem *card_item, m_handCards) {
        if (card_item->isEnabled()) {
            items << card_item;
            count++;
            if (need_only && count > 1) {
                unselectAll();
                return;
            }
        }
    }

    QList<int> equip_pos;
    for (int i = 0; i < 4; i++) {
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
    } else if (!equip_pos.isEmpty()) {
        int pos = equip_pos.first();
        _m_equipCards[pos]->mark(!_m_equipCards[pos]->isMarked());
        update();
    }
}

const Card *Dashboard::getSelected() const{
    if (view_as_skill)
        return pending_card;
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

    foreach (CardItem *card_item, m_handCards) {
        if (card_item != except)
            selectCard(card_item, false);
    }

    adjustCards(true);
    for (int i = 0; i < 4; i++) {
        if (_m_equipCards[i] && _m_equipCards[i] != except)
            _m_equipCards[i]->mark(false);
    }
    if (view_as_skill) {
        pendings.clear();
        updatePending();
    }
}

QRectF Dashboard::boundingRect() const{
    return QRectF(0, 0, _m_width, _m_layout->m_normalHeight);
}

void Dashboard::setWidth(int width) {
    prepareGeometryChange();
    adjustCards(true);
    this->_m_width = width;
    _updateFrames();
    _updateDeathIcon();
}

QSanSkillButton *Dashboard::addSkillButton(const QString &skillName) {
    // if it's a equip skill, add it to equip bar
    _mutexEquipAnim.lock();

    for (int i = 0; i < 4; i++) {
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
            _m_equipSkillBtns[i] = new QSanInvokeSkillButton();
            _m_equipSkillBtns[i]->setSkill(skill);
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
    if (_m_skillDock->getSkillButtonByName(skillName) != NULL) {
        _m_button_recycle.append(_m_skillDock->getSkillButtonByName(skillName));
        return NULL;
    }
    return _m_skillDock->addSkillButtonByName(skillName);
}

QSanSkillButton *Dashboard::removeSkillButton(const QString &skillName) {
    QSanSkillButton *btn = NULL;
    _mutexEquipAnim.lock();
    for (int i = 0; i < 4; i++) {
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
        QSanSkillButton *temp = _m_skillDock->getSkillButtonByName(skillName);
        if (_m_button_recycle.contains(temp))
            _m_button_recycle.removeOne(temp);
        else
            btn = _m_skillDock->removeSkillButtonByName(skillName);
    }
    return btn;
}

void Dashboard::highlightEquip(QString skillName, bool highlight) {
    int i = 0;
    for (i = 0; i < 4; i++) {
        if (!_m_equipCards[i]) continue;
        if (_m_equipCards[i]->getCard()->objectName() == skillName)
            break;
    }
    if (i != 4)
        _setEquipBorderAnimation(i, highlight);
}

void Dashboard::_createExtraButtons() {
    m_btnReverseSelection = new QSanButton("handcard", "reverse-selection", this);
    m_btnSortHandcard = new QSanButton("handcard", "sort", this);
    m_btnNoNullification = new QSanButton("handcard", "nullification", this);
    m_btnNoNullification->setStyle(QSanButton::S_STYLE_TOGGLE);
    // @todo: auto hide.
    m_btnReverseSelection->setPos(G_DASHBOARD_LAYOUT.m_rswidth, -m_btnReverseSelection->boundingRect().height());
    m_btnSortHandcard->setPos(m_btnReverseSelection->boundingRect().right() + G_DASHBOARD_LAYOUT.m_rswidth,
                              -m_btnReverseSelection->boundingRect().height());
    m_btnNoNullification->setPos(m_btnReverseSelection->boundingRect().right() + m_btnSortHandcard->boundingRect().width() + G_DASHBOARD_LAYOUT.m_rswidth,
                                 -m_btnReverseSelection->boundingRect().height());

    m_btnNoNullification->hide();
    connect(m_btnReverseSelection, SIGNAL(clicked()), this, SLOT(reverseSelection()));
    connect(m_btnSortHandcard, SIGNAL(clicked()), this, SLOT(sortCards()));
    connect(m_btnNoNullification, SIGNAL(clicked()), this, SLOT(cancelNullification()));
}

void Dashboard::skillButtonActivated() {
    QSanSkillButton *button = qobject_cast<QSanSkillButton *>(sender());
    foreach (QSanSkillButton *btn, _m_skillDock->getAllSkillButtons()) {
        if (button == btn) continue;

        if (btn->getViewAsSkill() != NULL && btn->isDown())
            btn->setState(QSanButton::S_STATE_UP);
    }

    for (int i = 0; i < 4; i++) {
        if (button == _m_equipSkillBtns[i]) continue;

        if (_m_equipSkillBtns[i] != NULL)
            _m_equipSkillBtns[i]->setEnabled(false);
    }
}

void Dashboard::skillButtonDeactivated() {
    foreach (QSanSkillButton *btn, _m_skillDock->getAllSkillButtons()) {
        if (btn->getViewAsSkill() != NULL && btn->isDown())
            btn->setState(QSanButton::S_STATE_UP);
    }

    for (int i = 0; i < 4; i++) {
        if (_m_equipSkillBtns[i] != NULL) {
            _m_equipSkillBtns[i]->setEnabled(true);
            if (_m_equipSkillBtns[i]->isDown())
                _m_equipSkillBtns[i]->click();
        }
    }
}

void Dashboard::selectAll() {
    if (view_as_skill) {
        unselectAll();
        foreach (CardItem *card_item, m_handCards) {
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
    for (i = 0; i < 4; i++) {
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
        for (int i = 0; i < 4; i++) {
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
        for (int i = 0; i < 4; i++) {
            if (_m_equipCards[i] == equip) {
                _setEquipBorderAnimation(i, equip->isMarked());
                break;
            }
        }
    }
}

void Dashboard::_createEquipBorderAnimations() {
    for (int i = 0; i < 4; i++) {
        _m_equipBorders[i] = new PixmapAnimation();
        _m_equipBorders[i]->setParentItem(_getEquipParent());
        _m_equipBorders[i]->setPath("image/system/emotion/equipborder/");
        if (!_m_equipBorders[i]->valid()) {
            delete _m_equipBorders[i];
            _m_equipBorders[i] = NULL;
            continue;
        }
        _m_equipBorders[i]->setPos(_dlayout->m_equipBorderPos + _dlayout->m_equipSelectedOffset + _dlayout->m_equipAreas[i].topLeft());
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
        newPos = _dlayout->m_equipSelectedOffset + _dlayout->m_equipAreas[index].topLeft();
    else
        newPos = _dlayout->m_equipAreas[index].topLeft();

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
    } else {
        _m_equipBorders[index]->hide();
        _m_equipBorders[index]->stop();
    }

    _m_isEquipsAnimOn[index] = turnOn;
    _mutexEquipAnim.unlock();
}

void Dashboard::adjustCards(bool playAnimation) {
    _adjustCards();
    foreach (CardItem *card, m_handCards)
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
    int middleWidth = _m_width - layout->m_leftWidth - layout->m_rightWidth - this->getButtonWidgetWidth();
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
    return _m_width - G_DASHBOARD_LAYOUT.m_leftWidth - G_DASHBOARD_LAYOUT.m_rightWidth;
}

QList<CardItem *> Dashboard::cloneCardItems(QList<int> card_ids) {
    QList<CardItem *> result;
    CardItem *card_item;
    CardItem *new_card;

    foreach (int card_id, card_ids) {
        card_item = CardItem::FindItem(m_handCards, card_id);
        new_card = _createCard(card_id);
        Q_ASSERT(card_item);
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
    foreach (int card_id, card_ids) {
        card_item = CardItem::FindItem(m_handCards, card_id);
        if (card_item == selected) selected = NULL;
        Q_ASSERT(card_item);
        if (card_item) {
            m_handCards.removeOne(card_item);
            card_item->hideFrame();
            card_item->disconnect(this);
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
        foreach (int card_id, card_ids) {
            card_item = _createCard(card_id);
            card_item->setOpacity(0.0);
            result.push_back(card_item);
        }
    } else
        Q_ASSERT(false);

    Q_ASSERT(result.size() == card_ids.size());
    if (place == Player::PlaceHand)
        adjustCards();
    else if (result.size() > 1 || place == Player::PlaceSpecial) {
        QRect rect(0, 0, _dlayout->m_disperseWidth, 0);
        QPointF center(0, 0);
        if (place == Player::PlaceEquip || place == Player::PlaceDelayedTrick) {
            for (int i = 0; i < result.size(); i++)
                center += result[i]->pos();
            center = 1.0 / result.length() * center;
        } else if (place == Player::PlaceSpecial)
            center = mapFromItem(_getAvatarParent(), _dlayout->m_avatarArea.center());
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
    if (!view_as_skill) return;

    QList<CardItem *> selected_items;
    foreach (CardItem *item, m_handCards)
        if (item->isSelected()) {
            item->clickItem();
            selected_items << item;
        }
    foreach (CardItem *item, m_handCards)
        if (item->isEnabled() && !selected_items.contains(item))
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

void Dashboard::controlNullificationButton(bool show) {
    if (ClientInstance->getReplayer()) return;
    m_btnNoNullification->setState(QSanButton::S_STATE_UP);
    m_btnNoNullification->setVisible(show);
}

void Dashboard::disableAllCards() {
    m_mutexEnableCards.lock();
    foreach (CardItem *card_item, m_handCards)
        card_item->setEnabled(false);
    m_mutexEnableCards.unlock();
}

void Dashboard::enableCards() {
    m_mutexEnableCards.lock();
    foreach (CardItem *card_item, m_handCards)
        card_item->setEnabled(card_item->getCard()->isAvailable(Self));
    m_mutexEnableCards.unlock();
}

void Dashboard::enableAllCards() {
    m_mutexEnableCards.lock();
    foreach (CardItem *card_item, m_handCards)
        card_item->setEnabled(true);
    m_mutexEnableCards.unlock();
}

void Dashboard::startPending(const ViewAsSkill *skill) {
    m_mutexEnableCards.lock();
    view_as_skill = skill;
    pendings.clear();
    unselectAll();

    for (int i = 0; i < 4; i++) {
        if (_m_equipCards[i] != NULL)
            connect(_m_equipCards[i], SIGNAL(mark_changed()), this, SLOT(onMarkChanged()));
    }

    updatePending();
    m_mutexEnableCards.unlock();
}

void Dashboard::stopPending() {
    m_mutexEnableCards.lock();
    if (view_as_skill && view_as_skill->objectName().contains("guhuo")) {
        foreach (CardItem *item, m_handCards)
            item->hideFootnote();
    }
    view_as_skill = NULL;
    pending_card = NULL;
    emit card_selected(NULL);

    foreach (CardItem *item, m_handCards) {
        item->setEnabled(false);
        animations->effectOut(item);
    }

    for (int i = 0; i < 4; i++) {
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

void Dashboard::onCardItemClicked() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (!card_item) return;

    if (view_as_skill) {
        if (card_item->isSelected()) {
            selectCard(card_item, false);
            pendings.removeOne(card_item);
        } else {
            if (view_as_skill->inherits("OneCardViewAsSkill"))
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

#include "wind.h"
void Dashboard::updatePending() {
    if (!view_as_skill) return;
    QList<const Card *> cards;
    foreach (CardItem *item, pendings)
        cards.append(item->getCard());

    QList<const Card *> pended;
    if (!view_as_skill->inherits("OneCardViewAsSkill"))
        pended = cards;
    foreach (CardItem *item, m_handCards) {
        if (!item->isSelected() || pendings.isEmpty())
            item->setEnabled(view_as_skill->viewFilter(pended, item->getCard()));
        if (!item->isEnabled())
            animations->effectOut(item);
    }

    for (int i = 0; i < 4; i++) {
        CardItem *equip = _m_equipCards[i];
        if (equip && !equip->isMarked())
            equip->setMarkable(view_as_skill->viewFilter(pended, equip->getCard()));
        if (equip) {
            if (!equip->isMarkable() && (!_m_equipSkillBtns[i] || !_m_equipSkillBtns[i]->isEnabled()))
                _m_equipRegions[i]->setOpacity(0.7);
            else
                _m_equipRegions[i]->setOpacity(1.0);
        }
    }

    const Card *new_pending_card = view_as_skill->viewAs(cards);
    if (pending_card != new_pending_card) {
        if (pending_card && !pending_card->parent() && pending_card->isVirtualCard()) {
            delete pending_card;
            pending_card = NULL;
        }
        if (view_as_skill->objectName().contains("guhuo")
            && Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY) {
            foreach (CardItem *item, m_handCards) {
                item->hideFootnote();
                if (new_pending_card && item->getCard() == cards.first()) {
                    const SkillCard *guhuo = qobject_cast<const SkillCard *>(new_pending_card);
                    item->setFootnote(Sanguosha->translate(guhuo->getUserString()));
                    item->showFootnote();
                }
            }
        }
        pending_card = new_pending_card;
        emit card_selected(pending_card);
    }
}

void Dashboard::onCardItemDoubleClicked() {
    if (!Config.EnableDoubleClick) return;
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        if (!view_as_skill) selected = card_item;
        animations->effectOut(card_item);
        emit card_to_use();
    }
}

void Dashboard::onCardItemThrown() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        if (!view_as_skill) selected = card_item;
        emit card_to_use();
    }
}

void Dashboard::onCardItemHover() {
    QGraphicsItem *card_item = qobject_cast<QGraphicsItem *>(sender());
    if (!card_item) return;

    animations->emphasize(card_item);
}

void Dashboard::onCardItemLeaveHover() {
    QGraphicsItem *card_item = qobject_cast<QGraphicsItem *>(sender());
    if (!card_item) return;

    animations->effectOut(card_item);
}

void Dashboard::onMarkChanged() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());

    Q_ASSERT(card_item->isEquipped());

    if (card_item) {
        if (card_item->isMarked()) {
            if (!pendings.contains(card_item)) {
                if (view_as_skill && view_as_skill->inherits("OneCardViewAsSkill"))
                    unselectAll(card_item);
                pendings.append(card_item);
            }
        } else
            pendings.removeOne(card_item);

        updatePending();
    }
}

const ViewAsSkill *Dashboard::currentSkill() const{
    return view_as_skill;
}

const Card *Dashboard::pendingCard() const{
    return pending_card;
}

