#include "carditem.h"
#include "engine.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFocusEvent>

static QRect CardRect(0, 0, 150*0.8, 210*0.8);
static QFont CardNumberFont("Times", 20, QFont::Bold);

CardItem::CardItem(Card *card)
    :card(card), view_card_item(NULL)
{
    Q_ASSERT(card != NULL);

    suit_pixmap.load(QString(":/images/suit/%1.png").arg(card->getSuitString()));
    pixmap.load(card->getPixmapPath());
    setFlags(ItemIsFocusable);
}

const Card *CardItem::getCard() const{
    return card;
}

void CardItem::setHomePos(QPointF home_pos){
    this->home_pos = home_pos;
}

void CardItem::goBack(bool animate){
    if(!animate){
        setPos(home_pos);
        return;
    }

    QPropertyAnimation *goback = new QPropertyAnimation(this, "pos");
    goback->setEndValue(home_pos);
    goback->setEasingCurve(QEasingCurve::OutBounce);
    goback->start();
}

void CardItem::viewAs(const QString &name){
    CardClass *card_class = Sanguosha->getCardClass(name);
    QPixmap view_card_pixmap(card_class->getPixmapPath());

    if(view_card_item == NULL){
        view_card_item = scene()->addPixmap(view_card_pixmap);
        view_card_item->setScale(0.2);
        view_card_item->setParentItem(this);
        view_card_item->setPos(50, 80);
    }else
        view_card_item->setPixmap(view_card_pixmap);

    view_card_item->setVisible(true);
}

void CardItem::select(){
    setY(10);
}

void CardItem::unselect(){   
    setY(45);
}

QRectF CardItem::boundingRect() const{
    return CardRect;
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        setOpacity(0.7);

        if(card->isRed())
            viewAs("slash");
        else
            viewAs("jink");
    }
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
    static QRect suit_rect(8,8,18,18);
    painter->drawPixmap(CardRect, pixmap);
    painter->drawPixmap(suit_rect, suit_pixmap);

    painter->setFont(CardNumberFont);
    if(card->isRed())
        painter->setPen(Qt::red);
    painter->drawText(8, 50, card->getNumberString());
}

QVariant CardItem::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == ItemEnabledChange){
        if(value.toBool()){
            setGraphicsEffect(NULL);
        }else{
            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(this);
            effect->setColor(QColor(20,20,20));
            setGraphicsEffect(effect);
        }
    }

    return QGraphicsObject::itemChange(change, value);
}

