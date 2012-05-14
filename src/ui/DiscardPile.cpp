#include "DiscardPile.h"
#include <QParallelAnimationGroup>

QList<CardItem*> DiscardPile::removeCardItems(const QList<int> &card_ids, Player::Place place)
{
    QList<CardItem*> result;
    _m_mutex_pileCards.lock();    
    foreach (int card_id, card_ids)
    {
        CardItem* card = NULL;
        for (int i = 0; i < m_visibleCards.size(); i++)
        {
            if (m_visibleCards[i]->getCard()->getId() == card_id)
            {
                card = m_visibleCards[i];
                m_visibleCards.removeAt(i);
                break;
            }
        }
        if (card == NULL)
            card = _createCard(card_id);
        if (card != NULL)
            result.append(card);
    }
    _m_mutex_pileCards.unlock();
    return result;
}

bool DiscardPile::_addCardItems(QList<CardItem*> &card_items, Player::Place place)
{
    _m_mutex_pileCards.lock();
    m_visibleCards.append(card_items);
    int numAdded = card_items.size();
    int numRemoved = m_visibleCards.size() - qMax(m_numCardsVisible, numAdded + 1);
    for (int i = 0; i <  numRemoved; i++)
    {
        CardItem* toRemove = m_visibleCards.first();
        toRemove->setZValue(0.0);
        m_dyingCards.append(toRemove);
        m_visibleCards.removeFirst();
    }
    foreach (CardItem* card_item, m_visibleCards)
    {
        card_item->setHomeOpacity(0.7);
    }
    foreach (CardItem* card_item, card_items)
    {
        card_item->setHomeOpacity(1.0);
    }
    m_visibleCards.last()->setHomeOpacity(1.0);
    _m_mutex_pileCards.unlock();
    adjustCards();    
    return false;
}
    
void DiscardPile::adjustCards()
{        
    _disperseCards(m_visibleCards, m_cardsDisplayRegion, Qt::AlignLeft, true);
    QParallelAnimationGroup* animation = new QParallelAnimationGroup;
    foreach (CardItem* card_item, m_visibleCards)
        animation->addAnimation(card_item->getGoBackAnimation(true));
    animation->start();
}

void DiscardPile::onAnimationFinished()
{
    _m_mutex_pileCards.lock();
    foreach (CardItem* card, m_dyingCards)
    {
        card->deleteLater();
    }
    m_dyingCards.clear();
    _m_mutex_pileCards.unlock();
}

// @todo: adjust here!!!
const QRect DrawPile::S_DISPLAY_CARD_REGION(0, 0, CardItem::S_NORMAL_CARD_WIDTH, CardItem::S_NORMAL_CARD_HEIGHT);

QList<CardItem*> DrawPile::removeCardItems(const QList<int> &card_ids, Player::Place place)
{
    QList<CardItem*> result = _createCards(card_ids);
    _disperseCards(result, S_DISPLAY_CARD_REGION, Qt::AlignCenter, false);  
    return result;
}

bool DrawPile::_addCardItems(QList<CardItem*> &card_items, Player::Place place)
{    
    foreach (CardItem* card_item, card_items)
    {
        card_item->setHomeOpacity(1.0);
    }
    _disperseCards(card_items, S_DISPLAY_CARD_REGION, Qt::AlignCenter, true);    
    return false;
}
/*
    drawPile = new Pixmap("image/system/card-back.png");
    addItem(drawPile);
    drawPile->setZValue(-2.0);
    drawPile->setPos(room_layout->drawpile);
    QGraphicsDropShadowEffect *drp = new QGraphicsDropShadowEffect;
    drp->setOffset(6);
    drp->setColor(QColor(0,0,0));
    drawPile->setGraphicsEffect(drp); */