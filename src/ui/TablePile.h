#ifndef _DISCARD_PILE_H
#define _DISCARD_PILE_H

#include "QSanSelectableItem.h"
#include "player.h"
#include "carditem.h"
#include "protocol.h"
#include "GenericCardContainerUI.h"
#include <QGraphicsObject>
#include <QPixmap>

class TablePile: public GenericCardContainer
{
    Q_OBJECT
public:  
    inline TablePile() : GenericCardContainer(), m_currentTime(0) { m_timer = startTimer(S_CLEARANCE_UPDATE_INTERVAL_MSEC); }
    virtual QList<CardItem*> removeCardItems(const QList<int> &card_ids, Player::Place place);
    inline void setSize(QSize newSize) 
    {
        setSize(newSize.width(), newSize.height());
    }
    void setSize(double width, double height);
    inline void setNumCardsVisible(int num) { m_numCardsVisible = num; }
    inline int getNumCardsVisible() { return m_numCardsVisible; }
    inline virtual void paint(QPainter *,const QStyleOptionGraphicsItem *,QWidget *) {}    
    void adjustCards();
    virtual QRectF boundingRect() const;
    void showJudgeResult(int cardId, bool takeEffect);
public slots:
    void clear(bool delayRequest = true);
protected:
    // This function must be called with mutex_pileCards locked.
    void _fadeOutCardsLocked(const QList<CardItem *> &cards);
    static const int S_CLEARANCE_UPDATE_INTERVAL_MSEC = 1000;
    static const int S_CLEARANCE_DELAY_BUCKETS = 3;
    virtual void timerEvent(QTimerEvent *);
    virtual bool _addCardItems(QList<CardItem*> &card_items, Player::Place place);
    void _markClearance(CardItem* item);
    QList<CardItem*> m_visibleCards;
    QMutex _m_mutex_pileCards;
    int m_numCardsVisible;
    QRect m_cardsDisplayRegion;
    int m_timer;
    int m_currentTime;
};

#endif
