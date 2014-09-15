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

#ifndef GRAPHICSPIXMAPHOVERITEM_H
#define GRAPHICSPIXMAPHOVERITEM_H

#include <QObject>
#include <QGraphicsPixmapItem>

class PlayerCardContainer;

class GraphicsPixmapHoverItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    explicit GraphicsPixmapHoverItem(PlayerCardContainer *playerCardContainer,
                                     QGraphicsItem *parent = 0);

    void stopChangeHeroSkinAnimation();
    bool isSkinChangingFinished() const { return (0 == m_timer); }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *);

public slots:
    void startChangeHeroSkinAnimation(const QString &generalName);

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);

    virtual void timerEvent(QTimerEvent *);

private:
    bool isPrimaryAvartarItem() const;
    bool isSecondaryAvartarItem() const;
    bool isAvatarOfDashboard() const;

    static void initSkinChangingFrames();

    PlayerCardContainer *m_playerCardContainer;
    int m_timer;
    int m_val;
    static const int m_max = 100;
    static const int m_step = 1;
    static const int m_interval = 25;
    QPixmap m_heroSkinPixmap;

    static QList<QPixmap> m_skinChangingFrames;
    static int m_skinChangingFrameCount;

    int m_currentSkinChangingFrameIndex;

signals:
    void hover_enter();
    void hover_leave();

    void skin_changing_start();
    void skin_changing_finished();
};

#endif // GRAPHICSPIXMAPHOVERITEM_H
