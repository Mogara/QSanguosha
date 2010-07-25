#include "carditem.h"
#include "engine.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFocusEvent>
#include <QParallelAnimationGroup>

CardItem::CardItem(const Card *card)
    :Pixmap(card->getPixmapPath(), false), card(card), view_card_item(NULL)
{
    Q_ASSERT(card != NULL);

    suit_pixmap.load(QString(":/suit/%1.png").arg(card->getSuitString()));
    pixmap = pixmap.scaled(150*0.8, 210*0.8);
    setFlags(ItemIsFocusable);
    setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);
}

const Card *CardItem::getCard() const{
    return card;
}

void CardItem::setHomePos(QPointF home_pos){
    this->home_pos = home_pos;
}

void CardItem::goBack(bool kieru){
    QPropertyAnimation *goback = new QPropertyAnimation(this, "pos");
    goback->setEndValue(home_pos);
    goback->setEasingCurve(QEasingCurve::OutBounce);

    if(kieru){
        QParallelAnimationGroup *group = new QParallelAnimationGroup;

        QPropertyAnimation *disappear = new QPropertyAnimation(this, "opacity");
        disappear->setEndValue(0.0);

        group->addAnimation(goback);
        group->addAnimation(disappear);

        group->start(QParallelAnimationGroup::DeleteWhenStopped);
    }else
        goback->start(QPropertyAnimation::DeleteWhenStopped);
}

//void CardItem::viewAs(const QString &name){
//    QPixmap view_card_pixmap(card_class->getPixmapPath());
//
//    if(view_card_item == NULL){
//        view_card_item = scene()->addPixmap(view_card_pixmap);
//        view_card_item->setScale(0.2);
//        view_card_item->setParentItem(this);
//        view_card_item->setPos(50, 80);
//    }else
//        view_card_item->setPixmap(view_card_pixmap);
//
//    view_card_item->setVisible(true);
//}

const QPixmap &CardItem::getSuitPixmap() const{
    return suit_pixmap;
}

void CardItem::select(){
    setY(10);
}

void CardItem::unselect(){   
    setY(45);
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        setOpacity(0.8);
        emit card_selected(card);
    }else if(rotation() != 0.0)
        emit show_discards();
    else
        emit hide_discards();
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    setOpacity(1.0);
    if(view_card_item){
        view_card_item->setVisible(false);
    }

    goBack();
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        setPos(this->mapToParent(event->pos()) - event->buttonDownPos(Qt::LeftButton));
    }
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    static QRect suit_rect(8,8,18,18);
    static QFont card_number_font("Times", 20, QFont::Bold);

    painter->drawPixmap(suit_rect, suit_pixmap);

    painter->setFont(card_number_font);
    if(card->isRed())
        painter->setPen(Qt::red);
    painter->drawText(8, 50, card->getNumberString());
}
