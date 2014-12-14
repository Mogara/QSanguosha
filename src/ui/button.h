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

#ifndef BUTTON_H
#define BUTTON_H

#include "settings.h"

#include <QGraphicsObject>
#include <QGraphicsRotation>

class QGraphicsDropShadowEffect;

class Button : public QGraphicsObject{
    Q_OBJECT

public:
    explicit Button(const QString &label, qreal scale = 1.0);
    explicit Button(const QPixmap &pixmap, qreal scale = 1.0);
    Button(const QString &label, const QSizeF &size);
    Button(const QPixmap &pixmap, const QSizeF &size);

    inline void setFontName(const QString &name) { this->font_name = name; }
    inline void setFontSize(const int &size) { this->font_size = size; }
    inline void setText(const QString &text) { label = text; }

    virtual QRectF boundingRect() const;

    static QFont defaultFont();

protected:
    void init();
    void initTextItems();
    void prepareIcons();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void mousePressEvent(QGraphicsSceneMouseEvent *) {}
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void setTextColorReversed(bool reversed);
    void updateIconsPosition();

    virtual QColor edgeColor() const { return Qt::white; }
    virtual QColor backgroundColor() const;
    virtual int edgeWidth() const { return 2; }

    QString label;
    QSizeF size;
    QString font_name;
    int font_size;

    QGraphicsPixmapItem *m_icon;
    QGraphicsPixmapItem *m_colorReversedIcon;

signals:
    void clicked();

private slots:
    void onEnabledChanged();

};

#endif // BUTTON_H

