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

#include "qsanbutton.h"
#include "clientplayer.h"
#include "SkinBank.h"
#include "engine.h"
#include "client.h"
#include "roomscene.h"

#include <QPixmap>
#include <QBitMap>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>

QSanButton::QSanButton(QGraphicsItem *parent)
    : QGraphicsObject(parent), _m_state(S_STATE_UP), _m_style(S_STYLE_PUSH),
    _m_mouseEntered(false), multi_state(false), first_state(true)
{
    setSize(QSize(0, 0));
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    setAcceptsHoverEvents(true);
#else
    setAcceptHoverEvents(true);
#endif
    setAcceptedMouseButtons(Qt::LeftButton);
}

QSanButton::QSanButton(const QString &groupName, const QString &buttonName, QGraphicsItem *parent, const bool &multi_state)
    : QGraphicsObject(parent), _m_state(S_STATE_UP), _m_style(S_STYLE_PUSH),
    _m_groupName(groupName), _m_buttonName(buttonName), _m_mouseEntered(false),
    multi_state(multi_state), first_state(true)
{
    const int state_count = multi_state ? (int)S_NUM_BUTTON_STATES * 2 : (int)S_NUM_BUTTON_STATES;
    for (int i = 0; i < state_count; i++) {
        const bool state1 = i < S_NUM_BUTTON_STATES;
        _m_bgPixmap[i] = G_ROOM_SKIN.getButtonPixmap(groupName, buttonName, (QSanButton::ButtonState)(state1 ? i : (i - S_NUM_BUTTON_STATES)), state1);
    }
    setSize(_m_bgPixmap[0].size());

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    setAcceptsHoverEvents(true);
#else
    setAcceptHoverEvents(true);
#endif
    setAcceptedMouseButtons(Qt::LeftButton);
}

void QSanButton::click() {
    if (isEnabled())
        _onMouseClick(true);
}

QRectF QSanButton::boundingRect() const{
    return QRectF(0, 0, _m_size.width(), _m_size.height());
}

void QSanButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->drawPixmap(0, 0, _m_bgPixmap[(int)_m_state + (first_state ? 0 : S_NUM_BUTTON_STATES)]);
}

void QSanButton::setSize(QSize newSize) {
    _m_size = newSize;
    if (_m_size.width() == 0 || _m_size.height() == 0) {
        _m_mask = QRegion();
        return;
    }
    Q_ASSERT(!_m_bgPixmap[0].isNull());
    QPixmap pixmap = _m_bgPixmap[0];
    _m_mask = QRegion(pixmap.mask().scaled(newSize));
}

void QSanButton::setRect(QRect rect) {
    setSize(rect.size());
    setPos(rect.topLeft());
}

void QSanButton::setStyle(ButtonStyle style) {
    _m_style = style;
}

void QSanButton::setEnabled(bool enabled) {
    bool changed = (enabled != isEnabled());
    if (!changed) return;
    if (enabled) {
        setState(S_STATE_UP);
        _m_mouseEntered = false;
    }
    QGraphicsObject::setEnabled(enabled);
    if (!enabled) setState(S_STATE_DISABLED);
    update();
    emit enable_changed();
}

void QSanButton::setState(QSanButton::ButtonState state) {
    if (this->_m_state != state) {
        this->_m_state = state;
        update();
    }
}

bool QSanButton::insideButton(QPointF pos) const{
    return _m_mask.contains(QPoint(pos.x(), pos.y()));
}

bool QSanButton::isMouseInside() const
{
    QGraphicsScene *scenePtr = scene();
    if (NULL == scenePtr) {
        return false;
    }

    QPoint cursorPos = QCursor::pos();
    foreach (QGraphicsView *view, scenePtr->views()) {
        QPointF pos = mapFromScene(view->mapToScene(view->mapFromGlobal(cursorPos)));
        if (insideButton(pos)) {
            return true;
        }
    }

    return false;
}

void QSanButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if (_m_state == S_STATE_DISABLED || _m_state == S_STATE_CANPRESHOW) return;
    QPointF point = event->pos();
    if (_m_mouseEntered || !insideButton(point)) return; // fake event;

    Q_ASSERT(_m_state != S_STATE_HOVER);
    _m_mouseEntered = true;
    if (_m_state == S_STATE_UP)
        setState(S_STATE_HOVER);
}

void QSanButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
    if (_m_state == S_STATE_DISABLED || _m_state == S_STATE_CANPRESHOW) return;
    if (!_m_mouseEntered) return;

    Q_ASSERT(_m_state != S_STATE_DISABLED && _m_state != S_STATE_CANPRESHOW);
    if (_m_state == S_STATE_HOVER)
        setState(S_STATE_UP);
    _m_mouseEntered = false;
}

void QSanButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event) {
    QPointF point = event->pos();
    if (insideButton(point)) {
        if (!_m_mouseEntered) hoverEnterEvent(event);
    }
    else {
        if (_m_mouseEntered) hoverLeaveEvent(event);
    }
}

void QSanButton::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QPointF point = event->pos();
    if (!insideButton(point)) return;

    if ((_m_style == S_STYLE_TOGGLE && !multi_state)
        || _m_state == S_STATE_DISABLED
        || _m_state == S_STATE_CANPRESHOW) return;
    setState(S_STATE_DOWN);
}

void QSanButton::_onMouseClick(bool inside) {
    bool changeState = true;
    if (inherits("QSanSkillButton")) {
        const Skill * skill = qobject_cast<const QSanSkillButton *>(this)->getSkill();
        if (skill->canPreshow() && !Self->hasShownSkill(skill)) changeState = false;
    }
    if (multi_state && inside)
        first_state = !first_state;
    if (_m_style == S_STYLE_PUSH && changeState)
        setState(S_STATE_UP);
    else if (_m_style == S_STYLE_TOGGLE) {
        if (_m_state == S_STATE_HOVER)
            _m_state = S_STATE_UP; // temporarily set, do not use setState!
        if (_m_state == S_STATE_DOWN) {
            if (inside)
                _m_state = S_STATE_HOVER;
            else
                _m_state = S_STATE_UP;
        }
        else if (_m_state == S_STATE_UP && inside)
            _m_state = S_STATE_DOWN;
    }
    update();

    if (inside) {
        emit clicked();
    } else {
        _m_mouseEntered = false;
        emit clicked_outside();
    }
}

void QSanButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QPointF point = event->pos();
    bool inside = insideButton(point);
    _onMouseClick(inside);
}

bool QSanButton::isDown() {
    return (_m_state == S_STATE_DOWN);
}

QSanSkillButton::QSanSkillButton(QGraphicsItem *parent)
    : QSanButton(parent)
{
    _m_groupName = QSanRoomSkin::S_SKIN_KEY_BUTTON_SKILL;
    _m_emitActivateSignal = false;
    _m_emitDeactivateSignal = false;
    _m_skill = NULL;
    _m_viewAsSkill = NULL;
    connect(this, SIGNAL(clicked()), this, SLOT(onMouseClick()));
    _m_skill = NULL;
}

void QSanSkillButton::_setSkillType(SkillType type) {
    _m_skillType = type;
}

void QSanSkillButton::onMouseClick() {
    if (_m_skill == NULL) return;

    if (!Self->hasPreshowedSkill(_m_skill) && _m_state == QSanButton::S_STATE_CANPRESHOW) {
        setState(S_STATE_DISABLED);
        ClientInstance->preshow(_m_skill->objectName(), true);
    } else if (Self->hasPreshowedSkill(_m_skill) && _m_state == QSanButton::S_STATE_DISABLED
             && _m_skill->canPreshow() && !Self->hasShownSkill(_m_skill)) {
        setState(QSanButton::S_STATE_CANPRESHOW);
        ClientInstance->preshow(_m_skill->objectName(), false);
    } else {
        if ((_m_style == S_STYLE_TOGGLE && isDown() && _m_emitActivateSignal) || _m_style == S_STYLE_PUSH) {
            emit skill_activated();
            emit skill_activated(_m_skill);
        } else if (!isDown() && _m_emitDeactivateSignal) {
            emit skill_deactivated();
            emit skill_deactivated(_m_skill);
        }
    }
}

void QSanSkillButton::setSkill(const Skill *skill) {
    Q_ASSERT(skill != NULL);
    if (skill == NULL)
        return;

    _m_skill = skill;
    // This is a nasty trick because the server side decides to choose a nasty design
    // such that sometimes the actual viewas skill is nested inside a trigger skill.
    // Since the trigger skill is not relevant, we flatten it before we create the button.
    _m_viewAsSkill = ViewAsSkill::parseViewAsSkill(_m_skill);
    if (skill == NULL) skill = _m_skill;

    Skill::Frequency freq = skill->getFrequency();
    if (skill->inherits("BattleArraySkill")) {
        setStyle(QSanButton::S_STYLE_TOGGLE);
        setState(QSanButton::S_STATE_DISABLED);
        _setSkillType(QSanInvokeSkillButton::S_SKILL_ARRAY);
        _m_emitActivateSignal = true;
        _m_emitDeactivateSignal = true;
    }
    else if ((freq == Skill::Frequent || freq == Skill::NotFrequent)
        && skill->inherits("TriggerSkill") && !skill->inherits("WeaponSkill")
        && !skill->inherits("ArmorSkill") && _m_viewAsSkill == NULL) {
        setStyle(QSanButton::S_STYLE_TOGGLE);
        setState(QSanButton::S_STATE_DISABLED);
        _setSkillType(QSanInvokeSkillButton::S_SKILL_PROACTIVE);
        _m_emitActivateSignal = false;
        _m_emitDeactivateSignal = false;
    }
    else if (freq == Skill::Limited || freq == Skill::NotFrequent) {
        setState(QSanButton::S_STATE_DISABLED);
        if (freq == Skill::Limited)
            _setSkillType(QSanInvokeSkillButton::S_SKILL_ONEOFF_SPELL);
        else
            _setSkillType(QSanInvokeSkillButton::S_SKILL_PROACTIVE);

        setStyle(QSanButton::S_STYLE_TOGGLE);
        _m_emitDeactivateSignal = true;
        _m_emitActivateSignal = true;
    }
    else if (freq == Skill::Compulsory) {
        setState(QSanButton::S_STATE_DISABLED);
        setStyle(QSanButton::S_STYLE_PUSH);
        _setSkillType(QSanInvokeSkillButton::S_SKILL_COMPULSORY);
        _m_emitActivateSignal = false;
        _m_emitDeactivateSignal = false;
    }
    else return;
    QString desc = skill->getDescription();
    desc = desc.simplified();
    setToolTip(desc);

    if (!Self->hasShownSkill(skill) && skill->canPreshow())
        setState(QSanButton::S_STATE_CANPRESHOW);

    Q_ASSERT((int)_m_skillType <= 3 && _m_state <= 4);
    _repaint();
}

void QSanSkillButton::setState(ButtonState state) {
    //refine state here for certain conditions
    if (_m_skillType == S_SKILL_COMPULSORY && state != S_STATE_CANPRESHOW)
        state = S_STATE_DISABLED;
    QSanButton::setState(state);
}

void QSanSkillButton::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QPointF point = event->pos();
    if (!insideButton(point)) return;

    if (_m_skillType == S_SKILL_COMPULSORY)
        return;
    else
        QSanButton::mousePressEvent(event);
}

void QSanSkillButton::setEnabled(bool enabled) {
    if (!enabled && _m_skill->canPreshow()
            && (!Self->hasShownSkill(_m_skill) || Self->hasFlag("hiding"))) {
        setState(Self->hasPreshowedSkill(_m_skill) ? S_STATE_DISABLED : S_STATE_CANPRESHOW);
    } else {
        QSanButton::setEnabled(enabled);
    }
}

void QSanInvokeSkillButton::_repaint() {
    for (int i = 0; i < (int)S_NUM_BUTTON_STATES; i++) {
        _m_bgPixmap[i] = G_ROOM_SKIN.getSkillButtonPixmap((ButtonState)i, _m_skillType, _m_enumWidth);
        if (_m_bgPixmap[i].isNull()) continue;

        if (i == S_STATE_CANPRESHOW) {
            QPixmap temp(_m_bgPixmap[i]);
            temp.fill(Qt::transparent);
            QPainter painter(&temp);
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawPixmap(0, 0, _m_bgPixmap[i]);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            painter.fillRect(temp.rect(), QColor(0, 0, 0, 160));
            _m_bgPixmap[i] = temp;
        }

        const IQSanComponentSkin::QSanShadowTextFont &font = G_DASHBOARD_LAYOUT.getSkillTextFont((ButtonState)i, _m_skillType, _m_enumWidth);
        QPainter painter(&_m_bgPixmap[i]);
        QString skillName = Sanguosha->translate(_m_skill->objectName());
        if (_m_enumWidth != S_WIDTH_WIDE) skillName = skillName.left(2);
        font.paintText(&painter,
            (ButtonState)i == S_STATE_DOWN ? G_DASHBOARD_LAYOUT.m_skillTextAreaDown[_m_enumWidth] :
            G_DASHBOARD_LAYOUT.m_skillTextArea[_m_enumWidth],
            Qt::AlignCenter, skillName);
    }
    setSize(_m_bgPixmap[0].size());
}

void QSanInvokeSkillButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->drawPixmap(0, 0, _m_bgPixmap[(int)_m_state]);
}

QSanSkillButton *QSanInvokeSkillDock::addSkillButtonByName(const QString &skillName) {
    Q_ASSERT(getSkillButtonByName(skillName) == NULL);
    QSanInvokeSkillButton *button = new QSanInvokeSkillButton(this);

    const Skill *skill = Sanguosha->getSkill(skillName);
    button->setSkill(skill);
    connect(button, SIGNAL(skill_activated(const Skill *)), this, SIGNAL(skill_activated(const Skill *)));
    connect(button, SIGNAL(skill_deactivated(const Skill *)), this, SIGNAL(skill_deactivated(const Skill *)));
    _m_buttons.append(button);
    update();
    return button;
}

int QSanInvokeSkillDock::width() const{
    return _m_width;
}

int QSanInvokeSkillDock::height() const{
    return _m_buttons.length() / 2 * G_DASHBOARD_LAYOUT.m_skillButtonsSize[0].height();
}

void QSanInvokeSkillDock::setWidth(int width) {
    _m_width = width;
}

void QSanInvokeSkillDock::update() {
    int numButtons = _m_buttons.length();
    int rows = (numButtons == 0) ? 0 : (numButtons - 1) / 2 + 1;
    int rowH = G_DASHBOARD_LAYOUT.m_skillButtonsSize[0].height();
    int *btnNum = new int[rows + 1]; // we allocate one more row in case we need it.
    int remainingBtns = numButtons;
    for (int i = 0; i < rows; i++) {
        btnNum[i] = qMin(2, remainingBtns);
        remainingBtns -= 2;
    }

    int m = 0;
    for (int i = 0; i < rows; i++) {
        int rowTop = (RoomSceneInstance->m_skillButtonSank) ? (-rowH - 2 * (rows - i - 1)) :
            ((-rows + i + 1) * rowH - 8);
        int btnWidth = _m_width / btnNum[i];
        int pix_wid = G_DASHBOARD_LAYOUT.m_skillButtonsSize[btnNum[i] - 1].width();
        for (int j = 0; j < btnNum[i]; j++) {
            int adj_value = (btnWidth - pix_wid) / 2;
            QSanInvokeSkillButton *button = _m_buttons[m++];
            button->setButtonWidth((QSanInvokeSkillButton::SkillButtonWidth)(btnNum[i] - 1));
            if (btnNum[i] == 2)
                adj_value += (j ? -2 : 2);
            button->setPos(btnWidth * j + adj_value, rowTop);
        }
    }
    delete[] btnNum;
    QGraphicsObject::update();
}

void QSanInvokeSkillDock::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
    // do nothing
}

QSanInvokeSkillButton *QSanInvokeSkillDock::getSkillButtonByName(const QString &skillName) const{
    foreach(QSanInvokeSkillButton *button, _m_buttons) {
        if (button->getSkill()->objectName() == skillName)
            return button;
    }
    return NULL;
}

