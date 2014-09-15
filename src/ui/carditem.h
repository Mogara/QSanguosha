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

#ifndef _CARD_ITEM_H
#define _CARD_ITEM_H

#include "card.h"
#include "QSanSelectableItem.h"
#include "settings.h"
#include <QAbstractAnimation>
#include <QMutex>
#include <QSize>
#include "SkinBank.h"
#include "qsanbutton.h"

class FilterSkill;
class General;
class QGraphicsDropShadowEffect;

class TransferButton : public QSanButton {
    Q_OBJECT

    friend class CardItem;
public:
    int getCardId() const;
    CardItem *getCardItem() const;

private:
    TransferButton(CardItem *parent);

    int _id;
    CardItem *_cardItem;

private slots:
    void onClicked();

signals:
    void _activated();
    void _deactivated();
};

class CardItem : public QSanSelectableItem {
    Q_OBJECT

public:
    CardItem(const Card *card);
    CardItem(const QString &general_name);
    ~CardItem();

    virtual QRectF boundingRect() const;
    virtual void setEnabled(bool enabled);

    const Card *getCard() const;
    void setCard(const Card *card);
    inline int getId() const{ return m_cardId; }

    // For move card animation
    void setHomePos(QPointF home_pos);
    QPointF homePos() const;
    QAbstractAnimation *getGoBackAnimation(bool doFadeEffect, bool smoothTransition = false,
        int duration = Config.S_MOVE_CARD_ANIMATION_DURATION);
    void goBack(bool playAnimation, bool doFade = true);
    inline QAbstractAnimation *getCurrentAnimation(bool doFade) {
        Q_UNUSED(doFade);
        return m_currentAnimation;
    }
    inline void setHomeOpacity(double opacity) { m_opacityAtHome = opacity; }
    inline double getHomeOpacity() { return m_opacityAtHome; }

    void showFrame(const QString &frame);
    void hideFrame();
    void showAvatar(const General *general);
    void hideAvatar();
    void setAutoBack(bool auto_back);
    void setFootnote(const QString &desc);

    inline bool isSelected() const{ return m_isSelected; }
    inline void setSelected(bool selected) { m_isSelected = selected; }
    bool isEquipped() const;

    virtual void setFrozen(bool is_frozen, bool update_movable = true);
    inline bool isFrozen() { return frozen; }

    inline void showFootnote() { _m_showFootnote = true; }
    inline void hideFootnote() { _m_showFootnote = false; }

    static CardItem *FindItem(const QList<CardItem *> &items, int card_id);

    struct UiHelper {
        int tablePileClearTimeStamp;
    } m_uiHelper;

    void clickItem() { emit clicked(); }

    void setOuterGlowEffectEnabled(const bool &willPlay);
    bool isOuterGlowEffectEnabled() const;

    void setOuterGlowColor(const QColor &color);
    QColor getOuterGlowColor() const;

    void setTransferable(const bool transferable);
    TransferButton *getTransferButton() const;

    void setSkinId(const int id);

protected:
    void _initialize();
    QAbstractAnimation *m_currentAnimation;
    QImage _m_footnoteImage;
    bool _m_showFootnote;
    // QGraphicsPixmapItem *_m_footnoteItem;
    QMutex m_animationMutex;
    double m_opacityAtHome;
    bool m_isSelected;
    bool _m_isUnknownGeneral;
    int _skinId;
    bool auto_back, frozen;
    QPointF _m_lastMousePressScenePos;

    static const int _S_CLICK_JITTER_TOLERANCE;
    static const int _S_MOVE_JITTER_TOLERANCE;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    int m_cardId;
    QString _m_frameType, _m_avatarName;
    QPointF home_pos;
    bool outerGlowEffectEnabled;
    QColor outerGlowColor;
    QGraphicsDropShadowEffect *outerGlowEffect;
    TransferButton *_transferButton;
    bool _transferable;

signals:
    void toggle_discards();
    void clicked();
    void double_clicked();
    void thrown();
    void released();
    void enter_hover();
    void leave_hover();
    void movement_animation_finished();
    void general_changed();
    void hoverChanged(const bool &enter);

public slots:
    virtual void changeGeneral(const QString &generalName);
    void onTransferEnabledChanged();
};

#endif

