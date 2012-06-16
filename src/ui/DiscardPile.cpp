#include "DiscardPile.h"
#include "SkinBank.h"
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

void DiscardPile::setSize(double width, double height) 
{
    m_cardsDisplayRegion = QRect(0, 0, width, height);
    m_numCardsVisible = width / G_COMMON_LAYOUT.m_cardNormalHeight + 1;
    resetTransform();
    translate(-width / 2, -height / 2);
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
        connect(toRemove, SIGNAL(movement_animation_finished()), this, SLOT(_destroyCard()));
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

QList<CardItem*> DrawPile::removeCardItems(const QList<int> &card_ids, Player::Place place)
{
    QList<CardItem*> result = _createCards(card_ids);
    _disperseCards(result, QRect(0, 0, G_COMMON_LAYOUT.m_cardNormalWidth, G_COMMON_LAYOUT.m_cardNormalHeight), Qt::AlignCenter, false, true);
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
