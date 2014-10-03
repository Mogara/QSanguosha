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

#ifndef _CHOOSE_GENERAL_BOX_H
#define _CHOOSE_GENERAL_BOX_H

#include "carditem.h"
#include "timedprogressbar.h"
#include "graphicsbox.h"

class Button;
class QGraphicsDropShadowEffect;

class GeneralCardItem : public CardItem {
    Q_OBJECT

public:
    friend class ChooseGeneralBox;
    void showCompanion();
    void hideCompanion();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    GeneralCardItem(const QString &generalName, const int skinId);

    bool hasCompanion;

public slots:
    virtual void changeGeneral(const QString &generalName);
};

class ChooseGeneralBox : public GraphicsBox {
    Q_OBJECT

public:
    explicit ChooseGeneralBox();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;
    void clear();

public slots:
    void chooseGeneral(const QStringList &generals, bool view_only = false,
                       bool single_result = false, const QString &reason = QString(),
                       const Player *player = NULL);
    void reply();
    void adjustItems();

private:
    int general_number;
    bool single_result;
    bool view_only;
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

    Button *confirm;
    QGraphicsProxyWidget *progress_bar_item;
    QSanCommandProgressBar *progress_bar;

    void _initializeItems();

private slots:
    void _adjust();
    void _onItemClicked();
};

#endif // _CHOOSE_GENERAL_BOX_H
