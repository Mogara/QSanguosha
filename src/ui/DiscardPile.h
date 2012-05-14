#ifndef _DISCARD_PILE_H
#define _DISCARD_PILE_H

#include "pixmap.h"
#include "player.h"
#include "carditem.h"
#include "protocol.h"
#include "GeneralCardContainerUI.h"
#include <QGraphicsObject>
#include <QPixmap>

class DiscardPile: public PlayerCardContainer
{
    Q_OBJECT
public:    
    virtual QList<CardItem*> removeCardItems(const QList<int> &card_ids, Player::Place place);
    inline void setSize(double width, double height) 
    {
        m_cardsDisplayRegion = QRect(0, 0, width,height);
        m_numCardsVisible = width / CardItem::S_NORMAL_CARD_WIDTH;
    }
    inline void setNumCardsVisible(int num) { m_numCardsVisible = num; }
    inline int getNumCardsVisible() { return m_numCardsVisible; }
    void adjustCards();
protected:
    virtual bool _addCardItems(QList<CardItem*> &card_items, Player::Place place);
    QList<CardItem*> m_visibleCards;
    QList<CardItem*> m_dyingCards;
    QMutex _m_mutex_pileCards;
    int m_numCardsVisible;
    QRect m_cardsDisplayRegion;
protected slots:    
    void onAnimationFinished();
};

class DrawPile: public PlayerCardContainer
{
    Q_OBJECT
public:
    virtual QList<CardItem*> removeCardItems(const QList<int> &card_ids, Player::Place place);    
protected:
    static const QRect S_DISPLAY_CARD_REGION;
    virtual bool _addCardItems(QList<CardItem*> &card_items, Player::Place place);
};

#endif