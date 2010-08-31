#include "carditem.h"
#include "engine.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFocusEvent>
#include <QParallelAnimationGroup>

CardItem::CardItem(const Card *card)
    :Pixmap(card->getPixmapPath(), false), card(card), markable(true), marked(false)
{
    Q_ASSERT(card != NULL);

    suit_pixmap.load(QString(":/suit/%1.png").arg(card->getSuitString()));
    icon_pixmap.load(card->getIconPath());
    pixmap = pixmap.scaled(150*0.8, 210*0.8);    
    setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);
}

const Card *CardItem::getCard() const{
    return card;
}

void CardItem::setHomePos(QPointF home_pos){
    this->home_pos = home_pos;
}

void CardItem::goBack(bool kieru){
    if(home_pos == pos())
        return;

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

const QPixmap &CardItem::getSuitPixmap() const{
    return suit_pixmap;
}

const QPixmap &CardItem::getIconPixmap() const{
    return icon_pixmap;
}

void CardItem::select(){
    home_pos.setY(10);
    setY(10);
}

void CardItem::unselect(){
    home_pos.setY(45);
    setY(45);
}

bool CardItem::isPending() const{
    return home_pos.y() == 10;
}

bool CardItem::isEquipped() const{
    return opacity() == 0.0;
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        emit clicked();
    }else if(rotation() != 0.0)
        emit show_discards();
    else
        emit hide_discards();
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(parentItem() && y() < -80)
        emit thrown();

    goBack();
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        QPointF down_pos = event->buttonDownPos(Qt::LeftButton);
        setPos(this->mapToParent(event->pos()) - down_pos);
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
    else
        painter->setPen(Qt::black);
    painter->drawText(8, 50, card->getNumberString());
}

bool CardItem::isMarked() const{
    return marked;
}

bool CardItem::isMarkable() const{
    return markable;
}

void CardItem::mark(bool marked){
    if(markable)
        this->marked = marked;
}

void CardItem::setMarkable(bool markable){
    this->markable = markable;

    setEnabled(markable);
}
