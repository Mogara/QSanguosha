#include "cardcontainer.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "client.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

GrabCardItem::GrabCardItem(const Card *card)
    :CardItem(card)
{
}

void GrabCardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *){
    if(!isFrozen())
        emit grabbed();

    goBack();
}

CardContainer::CardContainer() :
    Pixmap("image/system/card-container.png", false)
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
    close_button = new CloseButton;
    close_button->setParentItem(this);
    close_button->setPos(517, 21);
    close_button->hide();
}

void CardContainer::fillCards(const QList<int> &card_ids){
    static const QPointF pos1(30, 40);
    static const QPointF pos2(30, 184);
    static const int card_width = 93;
    static const int skip = 102;
    static const qreal whole_width = skip * 4 + card_width;

    int i, n = card_ids.length();

    if(n <= 10){
        for(i=0; i<n; i++){
            QPointF pos;
            if(i<5){
                pos = pos1;
                pos.setX(pos.x() + i * skip);
            }else{
                pos = pos2;
                pos.setX(pos.x() + (i-5) * skip);
            }

            addCardItem(card_ids.at(i), pos);
        }
    }else{
        int half = n/2 + 1;
        qreal real_skip = whole_width / half;
        for(i=0; i<n; i++){
            QPointF pos;
            if(i < half){
                pos = pos1;
                pos.setX(pos.x() + i * real_skip);
            }else{
                pos = pos2;
                pos.setX(pos.x() + (i-half) * real_skip);
            }

            addCardItem(card_ids.at(i), pos);
        }
    }

    show();
}

void CardContainer::clear(){
    foreach(CardItem *item, items){
        item->deleteLater();
    }

    items.clear();
    close_button->hide();
    hide();
}

void CardContainer::freezeCards(bool is_frozen){
    foreach(GrabCardItem *item, items){
        item->setFrozen(is_frozen);
    }
}

CardItem *CardContainer::take(const ClientPlayer *taker, int card_id){
    CardItem *to_take = NULL;

    foreach(GrabCardItem *item, items){
        if(item->getCard()->getId() == card_id){
            to_take = item;
            break;
        }
    }

    if(to_take == NULL)
        return NULL;

    to_take->setEnabled(false);

    CardItem *copy = new CardItem(to_take->getCard());
    copy->setPos(mapToScene(to_take->pos()));
    copy->setEnabled(false);

    if(taker){
        to_take->showAvatar(taker->getGeneral());
    }

    return copy;
}

int CardContainer::getFirstEnabled() const{
    foreach(CardItem *card, items){
        if(card->isEnabled())
            return card->getCard()->getId();
    }

    return -1;
}

void CardContainer::startChoose(){
    close_button->hide();
    foreach(GrabCardItem *item, items){
        connect(item, SIGNAL(grabbed()), this, SLOT(grabItem()));
        connect(item, SIGNAL(double_clicked()), this, SLOT(chooseItem()));
    }
}

void CardContainer::startGongxin(){
    foreach(GrabCardItem *item, items){
        if(item->getCard()->getSuit() == Card::Heart){
            connect(item, SIGNAL(double_clicked()), this, SLOT(gongxinItem()));
        }
    }
}

void CardContainer::addCloseButton(bool dispose){
    close_button->show();

    if(dispose)
        connect(close_button, SIGNAL(clicked()), this, SLOT(deleteLater()));
    else
        connect(close_button, SIGNAL(clicked()), this, SLOT(clear()));
}

void CardContainer::grabItem(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item && !collidesWithItem(card_item)){
        card_item->disconnect(this);
        emit item_chosen(card_item->getCard()->getId());
    }
}

void CardContainer::chooseItem(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item){
        card_item->disconnect(this);
        emit item_chosen(card_item->getCard()->getId());
    }
}

void CardContainer::addCardItem(int card_id, const QPointF &pos){
    GrabCardItem *item = new GrabCardItem(Sanguosha->getCard(card_id));
    item->setParentItem(this);

    item->setPos(pos);
    item->setHomePos(pos);
    item->setFlag(QGraphicsItem::ItemIsFocusable);

    items << item;
}

void CardContainer::gongxinItem(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item){
        emit item_gongxined(card_item->getCard()->getId());
        clear();
    }
}

CloseButton::CloseButton()
    :Pixmap("image/system/close.png", false)
{
    setFlag(ItemIsFocusable);

    setAcceptedMouseButtons(Qt::LeftButton);
}

void CloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event){
    event->accept();
}

void CloseButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *){
    emit clicked();
}

void CardContainer::view(const ClientPlayer *player){
    QList<int> card_ids;
    QList<const Card *> cards = player->getCards();
    foreach(const Card *card, cards)
        card_ids << card->getEffectiveId();

    fillCards(card_ids);

    QGraphicsPixmapItem *avatar = new QGraphicsPixmapItem(this);
    avatar->setPixmap(QPixmap(player->getGeneral()->getPixmapPath("tiny")));
    avatar->setPos(496, 288);    
}

GuanxingBox::GuanxingBox()
    :Pixmap("image/system/guanxing-box.png", false)
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
}

void GuanxingBox::doGuanxing(const QList<int> &card_ids, bool up_only){
    if(card_ids.isEmpty()){
        clear();
        return;
    }

    this->up_only = up_only;
    up_items.clear();

    foreach(int card_id, card_ids){
        CardItem *card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setAutoBack(false);
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        connect(card_item, SIGNAL(released()), this, SLOT(adjust()));

        up_items << card_item;
        card_item->setParentItem(this);
    }

    int i;
    for(i=0; i<up_items.length(); i++){
        CardItem *card_item = up_items.at(i);
        QPointF pos(start_x + i*skip, start_y1);
        card_item->setPos(pos);
        card_item->setHomePos(pos);
    }

    show();
}

void GuanxingBox::adjust(){
    CardItem *item = qobject_cast<CardItem*>(sender());

    if(item == NULL)
        return;

    up_items.removeOne(item);
    down_items.removeOne(item);

    QList<CardItem *> *items = NULL;
    if(up_only || item->y() <= middle_y)
        items = &up_items;
    else
        items = &down_items;

    int c = (item->x() - start_x) / card_width;
    c = qBound(0, c, items->length());
    items->insert(c, item);

    int i;
    for(i=0; i<up_items.length(); i++){
        QPointF pos(start_x + i*skip, start_y1);
        up_items.at(i)->setHomePos(pos);
        up_items.at(i)->goBack();
    }

    for(i=0; i<down_items.length(); i++){
        QPointF pos(start_x + i*skip, start_y2);
        down_items.at(i)->setHomePos(pos);
        down_items.at(i)->goBack();
    }
}

void GuanxingBox::clear()
{
    foreach(CardItem *card_item, up_items)
        card_item->deleteLater();

    foreach(CardItem *card_item, down_items)
        card_item->deleteLater();

    up_items.clear();
    down_items.clear();

    hide();
}

void GuanxingBox::reply(){
    QList<int> up_cards, down_cards;
    foreach(CardItem *card_item, up_items)
        up_cards << card_item->getCard()->getId();

    foreach(CardItem *card_item, down_items)
        down_cards << card_item->getCard()->getId();

    ClientInstance->replyGuanxing(up_cards, down_cards);
    clear();
}

