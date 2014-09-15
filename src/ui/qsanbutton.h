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

#ifndef _QSAN_BUTTON_H
#define _QSAN_BUTTON_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QRegion>
#include <QRect>

#include "skill.h"

class QSanButton : public QGraphicsObject{
    Q_OBJECT

public:
    //************************************
    // Method:    QSanButton
    // FullName:  QSanButton::QSanButton
    // Access:    public
    // Returns:
    // Qualifier:
    // Parameter: QGraphicsItem * parent
    // Description: Construct a powerful button with parent.
    //
    // Last Updated By Yanguam Siliagim
    // To optimize performance
    //
    // QSanguosha-Rara
    // March 14 2014
    //************************************
    QSanButton(QGraphicsItem *parent);
    //************************************
    // Method:    QSanButton
    // FullName:  QSanButton::QSanButton
    // Access:    public
    // Returns:
    // Qualifier:
    // Parameter: const QString & groupName
    // Parameter: const QString & buttonName
    // Parameter: QGraphicsItem * parent
    // Parameter: const bool & multi_state
    // Description: Construct a powerful button with parent. The button will belong to a group named
    //              groupName and be named buttonName. The names will influence the path of its resource file. It will be a button with 4 states if multi_state is false, or 8 states if true.
    //
    // Last Updated By Yanguam Siliagim
    // To optimize performance
    //
    // QSanguosha-Rara
    // March 14 2014
    //************************************
    QSanButton(const QString &groupName, const QString &buttonName, QGraphicsItem *parent, const bool &multi_state = false);
    enum ButtonState {
        S_STATE_UP, S_STATE_HOVER, S_STATE_DOWN, S_STATE_CANPRESHOW,
        S_STATE_DISABLED, S_NUM_BUTTON_STATES
    };
    enum ButtonStyle { S_STYLE_PUSH, S_STYLE_TOGGLE };
    void setSize(QSize size);
    void setStyle(ButtonStyle style);
    virtual void setState(ButtonState state);
    inline void setButtonName(QString buttonName) { _m_buttonName = buttonName; }
    inline QString getButtonName() { return _m_buttonName; }
    inline ButtonState getState() const{ return _m_state; }
    inline ButtonStyle getStyle() const{ return _m_style; }
    void setRect(QRect rect);
    virtual QRectF boundingRect() const;
    bool insideButton(QPointF pos) const;
    bool isMouseInside() const;
    virtual void setEnabled(bool enabled);
    bool isDown();

public slots:
    void click();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void _onMouseClick(bool inside);
    ButtonState _m_state;
    ButtonStyle _m_style;
    QString _m_groupName;
    QString _m_buttonName;
    QRegion _m_mask;
    QSize _m_size;
    // @todo: currently this is an extremely dirty hack. Refactor the button states to
    // get rid of it.
    bool _m_mouseEntered;
    QPixmap _m_bgPixmap[S_NUM_BUTTON_STATES * 2];
    //this property is designed for trust button.
    bool multi_state;
    bool first_state;

signals:
    void clicked();
    void clicked_outside();
    void enable_changed();
};

class QSanSkillButton : public QSanButton {
    Q_OBJECT

public:
    enum SkillType {
        S_SKILL_PROACTIVE, S_SKILL_COMPULSORY,
        S_SKILL_ONEOFF_SPELL, S_SKILL_ARRAY, S_NUM_SKILL_TYPES
    };

    inline static QString getSkillTypeString(SkillType type) {
        QString arg1;
        if (type == QSanSkillButton::S_SKILL_ARRAY) arg1 = "array";
        else if (type == QSanSkillButton::S_SKILL_COMPULSORY) arg1 = "compulsory";
        else if (type == QSanSkillButton::S_SKILL_ONEOFF_SPELL) arg1 = "oneoff";
        else if (type == QSanSkillButton::S_SKILL_PROACTIVE) arg1 = "proactive";
        return arg1;
    }
    virtual void setSkill(const Skill *skill);
    inline virtual const Skill *getSkill() const{ return _m_skill; }
    QSanSkillButton(QGraphicsItem *parent = NULL);
    inline const ViewAsSkill *getViewAsSkill() const{ return _m_viewAsSkill; }
    void setState(ButtonState state);
    void setEnabled(bool enabled);

protected:
    //methods
    virtual void _setSkillType(SkillType type);
    virtual void _repaint() = 0;
    const ViewAsSkill *_parseViewAsSkill() const;
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    //properties
    SkillType _m_skillType;
    bool _m_emitActivateSignal;
    bool _m_emitDeactivateSignal;
    const Skill *_m_skill;
    const ViewAsSkill *_m_viewAsSkill;

protected slots:
    void onMouseClick();

signals:
    void skill_activated(const Skill *);
    void skill_activated();
    void skill_deactivated(const Skill *);
    void skill_deactivated();
};

class QSanInvokeSkillButton : public QSanSkillButton {
    Q_OBJECT

public:
    inline QSanInvokeSkillButton(QGraphicsItem *parent = NULL) : QSanSkillButton(parent)
    {
        _m_enumWidth = S_WIDTH_NARROW;
    }
    enum SkillButtonWidth { S_WIDTH_WIDE, S_WIDTH_MED, S_WIDTH_NARROW, S_NUM_BUTTON_WIDTHS };
    inline void setButtonWidth(SkillButtonWidth width) { _m_enumWidth = width; _repaint(); }
    inline SkillButtonWidth getButtonWidth() { return _m_enumWidth; }

protected:
    // this function does not update the button's bg and is therefore not exposed to outside
    // classes.
    virtual void _repaint();
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    SkillButtonWidth _m_enumWidth;
};

class QSanInvokeSkillDock : public QGraphicsObject {
    Q_OBJECT

public:
    QSanInvokeSkillDock(QGraphicsItem *parent) : QGraphicsObject(parent) {}
    int width() const;
    int height() const;
    void setWidth(int width);
    inline void addSkillButton(QSanInvokeSkillButton *button) { _m_buttons.push_back(button); }
    inline void removeSkillButton(QSanInvokeSkillButton *button) {
        if (button == NULL) return;
        disconnect(button);
        _m_buttons.removeAll(button);
    }
    // Any one who call the following functions are responsible for
    // destroying the buttons returned
    QSanSkillButton *addSkillButtonByName(const QString &skillName);
    inline QSanSkillButton *removeSkillButtonByName(const QString &skillName) {
        QSanInvokeSkillButton *button = getSkillButtonByName(skillName);
        if (button != NULL) removeSkillButton(button);
        update();
        return button;
    }
    QSanInvokeSkillButton *getSkillButtonByName(const QString &skillName) const;
    void update();
    virtual QRectF boundingRect() const{ return QRectF(0, -height(), width(), height()); }
    inline QList<QSanInvokeSkillButton *> getAllSkillButtons() { return _m_buttons; }

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QList<QSanInvokeSkillButton *> _m_buttons;
    int _m_width;

signals:
    void skill_activated(const Skill *skill);
    void skill_deactivated(const Skill *skill);
};

#endif

