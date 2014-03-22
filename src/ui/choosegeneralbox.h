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
#ifndef _CHOOSE_GENERAL_BOX_H
#define _CHOOSE_GENERAL_BOX_H

#include "carditem.h"
#include "qsanbutton.h"
#include "TimedProgressBar.h"
#include "sprite.h"

class GeneralCardItem: public CardItem {
    Q_OBJECT

public:
    GeneralCardItem(const QString &general_name);

    void showCompanion();
    void hideCompanion();

    virtual void setFrozen(bool is_frozen);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    bool has_companion;
};

class ChooseGeneralBox: public QGraphicsObject {
    Q_OBJECT

public:
    explicit ChooseGeneralBox();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;
    void clear();
    
    inline void setSingleResult(const bool single_result) {    this->single_result = single_result;    };

public slots:
    void chooseGeneral(QStringList generals);
    void reply();
    void adjustItems();

private:
    int general_number;
    QList<GeneralCardItem *> items, selected;
    static const int top_dark_bar = 27;
    static const int top_blank_width = 42;
    static const int bottom_blank_width = 68;
    static const int card_bottom_to_split_line = 23;
    static const int card_to_center_line = 5;
    static const int left_blank_width = 37;
    static const int split_line_to_card_seat = 15;

    //data index
    static const int S_DATA_INITIAL_HOME_POS = 9527;

    QSanButton *confirm;
    QGraphicsProxyWidget *progress_bar_item;
    QSanCommandProgressBar *progress_bar;

    bool single_result;

    EffectAnimation *animations;

    void _initializeItems();

private slots:
    void _adjust();
    void _onItemClicked();
    void _onCardItemHover();
    void _onCardItemLeaveHover();
};

#endif // _CHOOSE_GENERAL_BOX_H