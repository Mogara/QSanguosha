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

#ifndef _CHOOSE_OPTIONS_BOX_H
#define _CHOOSE_OPTIONS_BOX_H

#include "TimedProgressBar.h"
#include "sprite.h"
#include <QGraphicsTextItem>

class Button;

class ToolTipBox : public QGraphicsObject {
    Q_OBJECT

public:
    explicit ToolTipBox(const QString &text);

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString text;
    QSizeF size;
    QGraphicsTextItem *tip_label;

    void init();

public slots:
    void showToolTip();
    void hideToolTip();
};

class ChooseOptionsBox : public QGraphicsObject {
    Q_OBJECT

public:
    explicit ChooseOptionsBox();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    QRectF boundingRect() const;

    inline void setSkillName(const QString &skillName) { skill_name = skillName; }

public slots:
    void chooseOption(const QStringList &options);
    void clear();

private:
    int options_number;
    QString skill_name;
    QList<Button *> buttons;
    static const int default_button_width = 100;
    static const int default_button_height = 30;
    static const int top_dark_bar = 27;
    static const int top_blank_width = 42;
    static const int bottom_blank_width = 25;
    static const int interval = 15;
    static const int left_blank_width = 37;

    QGraphicsProxyWidget *progress_bar_item;
    QSanCommandProgressBar *progress_bar;
};

#endif // _CHOOSE_OPTIONS_BOX_H
