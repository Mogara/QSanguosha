#include "DiscardPile.h"
#include <QParallelAnimationGroup>

QList<CardItem*> DiscardPile::removeCardItems(const QList<int> &card_ids, Player::Place place)
{
    QList<CardItem*> result;
    _m_mutex_pileCards.lock();    
    result = _createCards(card_ids);
    _disperseCards(result, m_cardsDisplayRegion, Qt::AlignCenter, false, true);
    foreach (CardItem* card, result)
    {        
        for (int i = m_visibleCards.size() - 1; i >= 0; i--)
        {
            if (m_visibleCards[i]->getCard()->getId() == card->getId())
            {
                card->setPos(m_visibleCards[i]->pos());
                break;
            }
        }
        
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
        toRemove->setHomeOpacity(0.0);
        connect(toRemove, SIGNAL(onAnimationFinished), this, SLOT(_destroyCard()));
        toRemove->goBack(true);
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
    _disperseCards(m_visibleCards, m_cardsDisplayRegion, Qt::AlignCenter, true, true);
    QParallelAnimationGroup* animation = new QParallelAnimationGroup;
    foreach (CardItem* card_item, m_visibleCards)
        animation->addAnimation(card_item->getGoBackAnimation(true));
    animation->start();
}

// @todo: adjust here!!!
const QRect DrawPile::S_DISPLAY_CARD_REGION(0, 0, CardItem::S_NORMAL_CARD_WIDTH, CardItem::S_NORMAL_CARD_HEIGHT);

QList<CardItem*> DrawPile::removeCardItems(const QList<int> &card_ids, Player::Place place)
{
    QList<CardItem*> result = _createCards(card_ids);
    _disperseCards(result, S_DISPLAY_CARD_REGION, Qt::AlignCenter, false, true);
    return result;
}

bool DrawPile::_addCardItems(QList<CardItem*> &card_items, Player::Place place)
{    
    foreach (CardItem* card_item, card_items)
    {
        card_item->setHomeOpacity(0.0);
    }
    return true;
}
