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

#ifndef _ROLE_COMBO_BOX_H
#define _ROLE_COMBO_BOX_H

#include <QGraphicsObject>
#include <QPainter>
#include <QGraphicsSceneEvent>

class RoleComboBox : public QGraphicsObject {
    Q_OBJECT

public:
    RoleComboBox(QGraphicsItem *photo, bool circle = false);
    static const int COMPACT_BORDER_WIDTH = 1;
    static const int COMPACT_ITEM_LENGTH = 10;

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    inline bool isExpanding() const { return expanding; };

private:

    bool circle;
    bool expanding;
    QMap<QString, bool> kingdoms_excluded;
    QString fixed_role;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void fix(const QString &role);
    void mouseClickedOutside();
};

#endif
