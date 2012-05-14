#ifndef _GENERAL_CARD_CONTAINER_UI_H
#define _GENERAL_CARD_CONTAINER_UI_H
#include "carditem.h"
#include "player.h"
#include <QGraphicsScene>
#include <QGraphicsItem>
#include "pixmap.h"
#include <QMutex>

class PlayerCardContainer: public Pixmap
{    
    Q_OBJECT
public:
    inline PlayerCardContainer() {}
    inline PlayerCardContainer(QString filename):Pixmap(filename){}
    virtual QList<CardItem*> removeCardItems(const QList<int> &card_ids,  Player::Place place) = 0;
    virtual void addCardItems(QList<CardItem*> &card_items, Player::Place place);
protected:
    // @return Whether the card items should be destroyed after animation
    virtual bool _addCardItems(QList<CardItem*> &card_items, Player::Place place) = 0;
    QList<CardItem*> _createCards(QList<int> card_ids);
    CardItem* _createCard(int card_id);
    void _disperseCards(QList<CardItem*> &cards, QRectF fillRegion, Qt::Alignment align, bool useHomePos);
    void _playMoveCardsAnimation(QList<CardItem*> &cards, bool destroyCards);    
protected slots:
    virtual void onAnimationFinished();
private slots:
    void _doUpdate();
    void _destroyCards();
private:
    QList<CardItem*> _cardsToBeDestroyed;
    QMutex _mutex_cardsToBeDestroyed;
signals:
    void animation_finished();
};
#endif