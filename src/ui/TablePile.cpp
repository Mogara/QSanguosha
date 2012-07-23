#include "TablePile.h"
#include "SkinBank.h"
#include <QParallelAnimationGroup>
#include "pixmapanimation.h"

#include <QTimer>

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

void TablePile::timerEvent(QTimerEvent *)
{    
    QList<CardItem *> oldCards;
    _m_mutex_pileCards.lock();
    m_currentTime++;
    foreach (CardItem* toRemove, m_visibleCards)
    {
        if (m_currentTime - toRemove->m_uiHelper.tablePileClearTimeStamp
            > S_CLEARANCE_DELAY_BUCKETS)
        {
            oldCards.append(toRemove);
            m_visibleCards.removeOne(toRemove);
        }
        else if (m_currentTime > toRemove->m_uiHelper.tablePileClearTimeStamp)
            toRemove->setEnabled(false); // @todo: this is a dirty trick. Use another property in the future
    }
    
    if (oldCards.empty()) {
        _m_mutex_pileCards.unlock();
        return;
    }

    _fadeOutCardsLocked(oldCards);
    _m_mutex_pileCards.unlock();
    
    adjustCards();
}

void TablePile::_markClearance(CardItem* item)
{
    if (item->m_uiHelper.tablePileClearTimeStamp > m_currentTime)
        item->m_uiHelper.tablePileClearTimeStamp = m_currentTime;
}

void TablePile::clear(bool delayRequest)
{
    if (m_visibleCards.empty()) return;
    _m_mutex_pileCards.lock();
    // check again since we just gain the lock.
    if (m_visibleCards.empty()) 
    {
        _m_mutex_pileCards.unlock();
        return;
    }

    if (delayRequest)
    {    
        foreach (CardItem* toRemove, m_visibleCards)
        {      
            _markClearance(toRemove);
        }
    }
    else
    {
        _fadeOutCardsLocked(m_visibleCards);
        m_visibleCards.clear();
    }

    _m_mutex_pileCards.unlock();
}

void TablePile::_fadeOutCardsLocked(const QList<CardItem *> &cards)
{
    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    foreach (CardItem* toRemove, cards)
    {
        toRemove->setZValue(0.0);
        toRemove->setHomeOpacity(0.0);
        toRemove->setHomePos(QPointF(toRemove->homePos().x(), toRemove->homePos().y()));
        toRemove->deleteLater();
        group->addAnimation(toRemove->getGoBackAnimation(true, false, 100));
    }
    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void TablePile::showJudgeResult(int cardId, bool takeEffect){
    _m_mutex_pileCards.lock();
    CardItem* judgeCard = NULL;
    QList<CardItem*> cardsToClear;
    for (int i = m_visibleCards.size() - 1; i >= 0; i--)
    {
        CardItem* item = m_visibleCards[i];
        if (item->getCard()->getId() == cardId)
        {
            judgeCard = m_visibleCards[i];
        }
        else
        {
            cardsToClear.append(item);
        }
    }
    if (judgeCard == NULL)
        judgeCard = _createCard(cardId);
    m_visibleCards.clear();
    m_visibleCards.append(judgeCard);
    _fadeOutCardsLocked(cardsToClear);
    PixmapAnimation::GetPixmapAnimation(judgeCard, takeEffect ? "judgegood" : "judgebad");
    _m_mutex_pileCards.unlock();
    adjustCards();
}

bool TablePile::_addCardItems(QList<CardItem*> &card_items, Player::Place place)
{
    if (card_items.isEmpty()) return false;

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
        CardItem* toRemove = m_visibleCards[i];
        _markClearance(toRemove);
    }
    
    foreach (CardItem* card_item, card_items)
    {
        card_item->setHomeOpacity(1.0);
        card_item->m_uiHelper.tablePileClearTimeStamp = INT_MAX;
    }

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

