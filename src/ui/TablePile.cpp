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
            oldCards.append(toRemove);
        else if (m_currentTime > toRemove->m_uiHelper.tablePileClearTimeStamp)
            toRemove->setEnabled(false); // @todo: this is a dirty trick. Use another property in the future
    }
    
    if (oldCards.empty()) {
        _m_mutex_pileCards.unlock();
        return;
    }

    QParallelAnimationGroup* group = new QParallelAnimationGroup;
    foreach (CardItem* toRemove, oldCards)
    {
        toRemove->setZValue(0.0);
        toRemove->setHomeOpacity(0.0);
        toRemove->setHomePos(QPointF(toRemove->homePos().x(), toRemove->homePos().y()));
        connect(toRemove, SIGNAL(movement_animation_finished()), this, SLOT(_destroyCard()));
        group->addAnimation(toRemove->getGoBackAnimation(true));
        m_visibleCards.removeAll(toRemove);
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);
    _m_mutex_pileCards.unlock();
    
    adjustCards();
}

void TablePile::clear(bool playAnimation)
{
    if (m_visibleCards.empty()) return;
    _m_mutex_pileCards.lock();
    // check again since we just gain the lock.
    if (m_visibleCards.empty()) 
    {
        _m_mutex_pileCards.unlock();
        return;
    }

    
    foreach (CardItem* toRemove, m_visibleCards)
    {        
        if (playAnimation)
        {
            toRemove->m_uiHelper.tablePileClearTimeStamp = m_currentTime;
        }
        else
        {
            m_visibleCards.removeAll(toRemove);
            toRemove->deleteLater();
        }
    }

    _m_mutex_pileCards.unlock();
}

void TablePile::_showJudgeResult(){
    if(m_judge_card == NULL)
        return;

    m_judge_card->setParentItem(this);
    m_visibleCards.append(m_judge_card);
    _disperseCards(m_visibleCards, m_cardsDisplayRegion, Qt::AlignCenter, true, true);
    m_judge_card->setOpacity(1.0);
    m_judge_card->goBack(false);
    PixmapAnimation::GetPixmapAnimation(m_judge_card, m_judge_emotion);
    m_judge_card = NULL;
}

void TablePile::showJudgeResult(CardItem *card, bool take_effect){
    m_judge_card = card;
    m_judge_emotion = take_effect ? "judgegood" : "judgebad";
    clear(false);
    QTimer::singleShot(Settings::S_MOVE_CARD_ANIMATION_DURAION + 100, this, SLOT(_showJudgeResult()));
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
        toRemove->m_uiHelper.tablePileClearTimeStamp = m_currentTime;
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

