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

#ifndef BUBBLECHATBOX_H
#define BUBBLECHATBOX_H

#include <QGraphicsObject>
#include <QTimer>
#include <QTextOption>
#include <QTextDocument>
#include <QGraphicsTextItem>

class QPropertyAnimation;

class BubbleChatBox : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit BubbleChatBox(const QRect &area, QGraphicsItem *parent = 0);
    ~BubbleChatBox();

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QPainterPath shape() const;

    void setText(const QString &text);
    void setArea(const QRect &newArea);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    class BubbleChatLabel : public QGraphicsTextItem
    {

    public:
        explicit BubbleChatLabel(QGraphicsItem *parent = 0);

        virtual QRectF boundingRect() const;

        void setBoundingRect(const QRectF &newRect);

        void setAlignment(Qt::Alignment alignment);

        void setWrapMode(QTextOption::WrapMode wrap);

    private:
        QRectF rect;
        QTextDocument *doc;

    };

    void updatePos();

    QPixmap backgroundPixmap;
    QRectF rect;
    QRect area;
    QTimer timer;
    BubbleChatLabel *chatLabel;

    QPropertyAnimation *appearAndDisappear;

private slots:
    void clear();
};

#endif // BUBBLECHATBOX_H
