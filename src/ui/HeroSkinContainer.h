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

#ifndef HEROSKINCONTAINER_H
#define HEROSKINCONTAINER_H

#include <QGraphicsObject>

class SkinItem;
class QScrollBar;

class HeroSkinContainer : public QGraphicsObject
{
    Q_OBJECT

public:
    HeroSkinContainer(const QString &generalName,
        const QString &kingdom, QGraphicsItem *parent = 0);

    ~HeroSkinContainer() {
        if (this == m_currentTopMostContainer) {
            m_currentTopMostContainer = NULL;
        }
    }

    const QString &getGeneralName() const { return m_generalName; }

    void bringToTopMost();

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);

    static bool hasSkin(const QString &generalName);
    static int getNextSkinIndex(const QString &generalName, int skinIndex);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

private:
    void initSkins();
    void createSkinItem(int skinIndex, QGraphicsItem *parent, bool used = false);
    void fillSkins();
    void swapWithSkinItemUsed(int skinIndex);
    int getCurrentSkinId() const;
    void setCurrentSkinId(const int skinId);

    const QString m_generalName;
    const QPixmap m_backgroundPixmap;

    QList<SkinItem *> m_skins;
    QMap<int, SkinItem *> m_skinIndexToItem;

    //Deal with z value to keep me on the top in particular cases
    qreal m_originalZValue;

    QScrollBar *m_vScrollBar;
    int m_oldScrollValue;

    static HeroSkinContainer *m_currentTopMostContainer;

private slots:
    void close();
    void skinSelected(const int skinId);
    void scrollBarValueChanged(int newValue);
};

#endif // HEROSKINCONTAINER_H
