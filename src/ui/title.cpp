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

#include "title.h"
#include "skinbank.h"

Title::Title(QGraphicsObject *parent, const QString &text, const QString &font_name, const int &font_size)
: QGraphicsObject(parent), text(text), font_name(font_name), font_size(font_size)
{
}

QRectF Title::boundingRect() const
{
    return QRectF(0, 0, font_size * text.length(), font_size + 1);
}

void Title::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor textColor = Qt::white;
    IQSanComponentSkin::QSanSimpleTextFont ft;
    JsonArray val;
    val << font_name;
    val << font_size;
    val << 2;

    JsonArray val3;
    val3 << textColor.red();
    val3 << textColor.green();
    val3 << textColor.blue();
    val << QVariant(val3);

    ft.tryParse(val);
    ft.paintText(painter, boundingRect().toRect(), Qt::AlignCenter, text);
}
