#include "cardcontainer.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "client.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>


CardContainer::CardContainer()    
{
    QSanSelectableItem::load("image/system/card-container.png", true);
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
    close_button = new CloseButton;
    close_button->setParentItem(this);
    close_button->setPos(517, 21);
    close_button->hide();
}

void CardContainer::fillCards(const QList<int> &card_ids){
    if(card_ids.isEmpty()) return;
    else if(!items.isEmpty()){
        items_stack.push(items);
        items.clear();
    }
    QList<CardItem*> card_items = _createCards(card_ids);

    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    QPointF pos1(30 + card_width / 2, 40 + G_COMMON_LAYOUT.m_cardNormalHeight / 2);
    QPointF pos2(30 + card_width / 2, 184 + G_COMMON_LAYOUT.m_cardNormalHeight / 2);
    int skip = 102;
    qreal whole_width = skip * 4 + card_width;
    items.append(card_items);
    int n = items.length();

    for (int i = 0; i < n; i++) {
        QPointF pos;
        if(n <= 10){                    
            if(i < 5){
                pos = pos1;
                pos.setX(pos.x() + i * skip);
            }else{
                pos = pos2;
                pos.setX(pos.x() + (i - 5) * skip);
            }            
        }else{
            int half = n / 2 + 1;
            qreal real_skip = whole_width / half;

            if(i < half){
                pos = pos1;
                pos.setX(pos.x() + i * real_skip);
            }else{
                pos = pos2;
                pos.setX(pos.x() + (i - half) * real_skip);
            }        
        }      
        CardItem* item = items[i];
        item->setPos(pos);
        item->setHomePos(pos);
        item->setOpacity(1.0);
        item->setHomeOpacity(1.0);
        item->setFlag(QGraphicsItem::ItemIsFocusable);
    }    
}

bool CardContainer::_addCardItems(QList<CardItem*> &card_items, Player::Place place){
    // foreach(CardItem* card_item, card_items) card_item->setHomePos

    return true;    
}

void CardContainer::clear(){
    foreach(CardItem *item, items){
        item->deleteLater();
    }

    items.clear();
    if(!items_stack.isEmpty()){
        items = items_stack.pop();
        fillCards();
    }
    else{
        close_button->hide();
        hide();
    }
}

void CardContainer::freezeCards(bool is_frozen){
    foreach (CardItem *item, items){
        item->setFrozen(is_frozen);
    }
}

QList<CardItem*> CardContainer::removeCardItems(const QList<int> &card_ids, Player::Place place){
    QList<CardItem*> result;
    foreach (int card_id, card_ids)
    {
        CardItem *to_take = NULL;

        foreach (CardItem *item, items){
            if(item->getCard()->getId() == card_id){
                to_take = item;
                break;
            }
        }

        if(to_take == NULL) continue;

        to_take->setEnabled(false);

        CardItem *copy = new CardItem(to_take->getCard());
        copy->setPos(mapToScene(to_take->pos()));
        copy->setEnabled(false);
        result.append(copy);

        if(m_currentPlayer){
            to_take->showAvatar(m_currentPlayer->getGeneral());
        }
    }
    return result;
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
    foreach (CardItem *item, items){
        connect(item, SIGNAL(leave_hover()), this, SLOT(grabItem()));
        connect(item, SIGNAL(double_clicked()), this, SLOT(chooseItem()));
    }
}

void CardContainer::startGongxin(){
    foreach (CardItem *item, items){
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

void CardContainer::gongxinItem(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item){
        emit item_gongxined(card_item->getCard()->getId());
        clear();
    }
}

CloseButton::CloseButton()
    :QSanSelectableItem("image/system/close.png", false)
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
}

GuanxingBox::GuanxingBox()
    :QSanSelectableItem("image/system/guanxing-box.png", true)
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

    int c = (item->x() - start_x) / G_COMMON_LAYOUT.m_cardNormalWidth;
    c = qBound(0, c, items->length());
    items->insert(c, item);

    int i;
    for(i = 0; i < up_items.length(); i++){
        QPointF pos(start_x + i * skip, start_y1);
        up_items.at(i)->setHomePos(pos);
        up_items.at(i)->goBack(false);
    }

    for(i = 0; i < down_items.length(); i++){
        QPointF pos(start_x + i * skip, start_y2);
        down_items.at(i)->setHomePos(pos);
        down_items.at(i)->goBack(false);
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

    ClientInstance->onPlayerReplyGuanxing(up_cards, down_cards);
    clear();
}

