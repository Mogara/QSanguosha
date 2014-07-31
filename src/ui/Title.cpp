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

#include "Title.h"
#include "SkinBank.h"
#include "jsonutils.h"

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
    Json::Value val(Json::arrayValue);
    val[0] = QSanProtocol::Utils::toJsonString(font_name);
    val[1] = font_size;
    val[2] = 2;

    val[3] = Json::Value(Json::arrayValue);
    val[3][0] = textColor.red();
    val[3][1] = textColor.green();
    val[3][2] = textColor.blue();

    ft.tryParse(val);
    ft.paintText(painter, boundingRect().toRect(), Qt::AlignCenter, text);
}
