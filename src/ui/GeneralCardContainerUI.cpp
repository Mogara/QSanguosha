#include "GeneralCardContainerUI.h"
#include <QParallelAnimationGroup>
#include "engine.h"

QList<CardItem*> PlayerCardContainer::_createCards(QList<int> card_ids)
{
    QList<CardItem*> result;
    foreach (int card_id, card_ids)
    {
        CardItem* item = _createCard(card_id);
        result.append(item);
    }
    return result;
}

CardItem* PlayerCardContainer::_createCard(int card_id)
{
    const Card* card = Sanguosha->getCard(card_id);
    CardItem *item = new CardItem(card);
    item->setOpacity(0.0);
    item->setParentItem(this);
    return item;
}

void PlayerCardContainer::_destroyCard()
{
    CardItem* card = (CardItem*)sender();
    delete card;
}

void PlayerCardContainer::_disperseCards(QList<CardItem*> &cards, QRectF fillRegion,
                                            Qt::Alignment align, bool useHomePos)
{
    int numCards = cards.size();
    if (numCards == 0) return;
    double maxWidth = fillRegion.width();
    double step = qMin(cards.first()->boundingRect().width(), maxWidth / numCards);
    align &= Qt::AlignHorizontal_Mask;
    for (int i = 0; i < numCards; i++)
    {
        CardItem* card = cards[i];
        double newX = 0;
        if (align == Qt::AlignHCenter)
            newX = fillRegion.center().x() - card->boundingRect().width() / 2.0
                    + step * (i - (numCards - 1) / 2.0);
        else if (align == Qt::AlignLeft)
            newX = fillRegion.left() + step * i;
        else if (align == Qt::AlignRight)
            newX = fillRegion.right() + step * (i - numCards);
        else continue;
        QPointF newPos = QPointF(newX, fillRegion.center().y());
        if (useHomePos)
            card->setHomePos(newPos);
        else
            card->setPos(newPos);
        card->setZValue(_m_highestZ++);
    }
}

void PlayerCardContainer::onAnimationFinished()
{
}

void PlayerCardContainer::_doUpdate()
{
    update();
}

void PlayerCardContainer::_playMoveCardsAnimation(QList<CardItem*> &cards, bool destroyCards)
{    
    if (destroyCards)    
    {
        _mutex_cardsToBeDestroyed.lock();
        _cardsToBeDestroyed.append(cards);
        _mutex_cardsToBeDestroyed.unlock();
    }
    
    QParallelAnimationGroup* animation = new QParallelAnimationGroup;
    foreach (CardItem* card_item, cards)
    {
        if (destroyCards)        
            connect(card_item, SIGNAL(movement_animation_finished()), this, SLOT(_destroyCard()));
        animation->addAnimation(card_item->getGoBackAnimation(true));
    }
    
    connect(animation, SIGNAL(finished()), this, SLOT(_doUpdate())); 
    connect(animation, SIGNAL(finished()), this, SLOT(onAnimationFinished()));
    animation->start();
}

void PlayerCardContainer::addCardItems(QList<CardItem*> &card_items, Player::Place place)
{
    foreach (CardItem* card_item, card_items)
    {        
        card_item->setPos(mapFromScene(card_item->scenePos()));
        card_item->setParentItem(this);        
    }
    bool destroy = _addCardItems(card_items, place);
    _playMoveCardsAnimation(card_items, destroy);
}