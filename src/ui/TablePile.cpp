#include "TablePile.h"
#include "SkinBank.h"
#include <QParallelAnimationGroup>

QList<CardItem*> TablePile::removeCardItems(const QList<int> &card_ids, Player::Place place)
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

QRectF TablePile::boundingRect() const
{
    return m_cardsDisplayRegion;
}

void TablePile::setSize(double width, double height) 
{
    m_cardsDisplayRegion = QRect(0, 0, width, height);
    m_numCardsVisible = width / G_COMMON_LAYOUT.m_cardNormalHeight + 1;
    resetTransform();
    translate(-width / 2, -height / 2);
}

void TablePile::clear(bool playAnimation)
{
    if (m_visibleCards.empty()) return;
    _m_mutex_pileCards.lock();
    // check again since we just gain the lock.
    int shift = 1 * G_COMMON_LAYOUT.m_cardNormalWidth;
    
    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    foreach (CardItem* toRemove, m_visibleCards)
    {        
        toRemove->setZValue(0.0);
        toRemove->setHomeOpacity(0.0);
        toRemove->setHomePos(QPointF(toRemove->x() - shift, toRemove->y()));
        if (playAnimation)
        {
            connect(toRemove, SIGNAL(movement_animation_finished()), this, SLOT(_destroyCard()));
            group->addAnimation(toRemove->getGoBackAnimation(true));
        }
        else delete toRemove;
    }
    m_visibleCards.clear();
    group->start(QAbstractAnimation::DeleteWhenStopped);
    _m_mutex_pileCards.unlock();
}

bool TablePile::_addCardItems(QList<CardItem*> &card_items, Player::Place place)
{
    _m_mutex_pileCards.lock();
    m_visibleCards.append(card_items);
    int numAdded = card_items.size();
    int numRemoved = m_visibleCards.size() - qMax(m_numCardsVisible, numAdded + 1);
    int shift;
    if (numRemoved > 0)
    {
        CardItem* forerunner = m_visibleCards.first();
        QPointF oldPos = forerunner->pos();
        shift = oldPos.x() + 10;
    }
    for (int i = 0; i <  numRemoved; i++)
    {
        CardItem* toRemove = m_visibleCards.takeFirst();
        toRemove->setZValue(0.0);
        toRemove->setHomeOpacity(0.0);
        toRemove->setHomePos(QPointF(toRemove->x() - shift, toRemove->y()));
        connect(toRemove, SIGNAL(movement_animation_finished()), this, SLOT(_destroyCard()));
        toRemove->goBack(true);
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
    
void TablePile::adjustCards()
{        
    _disperseCards(m_visibleCards, m_cardsDisplayRegion, Qt::AlignCenter, true, true);
    QParallelAnimationGroup* animation = new QParallelAnimationGroup;
    foreach (CardItem* card_item, m_visibleCards)
        animation->addAnimation(card_item->getGoBackAnimation(true));
    animation->start();
}

