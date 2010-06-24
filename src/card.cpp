#include "card.h"
#include "settings.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <QGraphicsScene>

static QRect CardRect(0, 0, 150*0.8, 210*0.8);
static QFont CardNumberFont("Times", 20, QFont::Bold);
static const char *SuitNames[] = {"spade", "club", "heart", "diamond"};

Card::Card(const QString name, enum Suit suit, int number)
    :name(name), suit(suit), number(number), view_card(NULL)
{
    if(number < 1 || number > 13)
        number = 0;

    setObjectName(name);
    suit_pixmap.load(QString(":/images/suit/%1.png").arg(SuitNames[suit]));
    pixmap.load("cards/" + name + ".png");
    setFlags(QGraphicsItem::ItemIsFocusable);
    setAcceptedMouseButtons(Qt::LeftButton);
    setPos(home_pos);

    monochrome_effect = new QGraphicsColorizeEffect;
    monochrome_effect->setColor(QColor(20,20,20));

    connect(this, SIGNAL(enabledChanged()), SLOT(setMonochrome()));
}

QString Card::getSuit() const{
    switch(suit){
    case Spade: return "Spade";
    case Heart: return "Heart";
    case Club: return "Club";
    case Diamond: return "Diamond";
    default: return "";
    }
}

bool Card::isRed() const{
    return suit == Heart || suit == Diamond;
}

bool Card::isBlack() const{
    return suit == Spade || suit == Club;
}

int Card::getNumber() const{
    return number;
}

QString Card::getNumberString() const{
    if(number == 10)
        return "10";
    else{
        static const char *number_string = "-A23456789-JQK";
        return QChar(number_string[number]);
    }
}

QString Card::getType() const{
    switch(type){
    case Basic: return "Basic"; break;
    case Equip: return "Equip"; break;
    case Trick: return "Trick"; break;
    default: return "Other";
    }
}

void Card::setHomePos(QPointF home_pos){
    this->home_pos = home_pos;
}

void Card::goBack(){
    QPropertyAnimation *goback = new QPropertyAnimation(this, "pos");
    goback->setEndValue(home_pos);
    goback->setEasingCurve(QEasingCurve::OutBounce);
    goback->start();
}

void Card::viewAs(const QString &view_card_name){
    QPixmap view_card_pixmap("cards/" + view_card_name + ".png");
    if(view_card){
        if(view_card->pixmap().cacheKey() == view_card_pixmap.cacheKey()){
            view_card->setVisible(true);
            return;
        }

        scene()->removeItem(view_card);
        delete view_card;
    }

    view_card = scene()->addPixmap(view_card_pixmap);
    view_card->setScale(0.2);
    view_card->setParentItem(this);
    view_card->setPos(50, 80);
}

QRectF Card::boundingRect() const{
    return CardRect;
}

bool Card::CompareBySuitNumber(Card *a, Card *b){
    if(a->suit != b->suit)
        return a->suit < b->suit;
    else
        return a->number < b->number;
}

bool Card::CompareByType(Card *a, Card *b){
    // FIXME: Not implemented
    return true;
}

void Card::mousePressEvent(QGraphicsSceneMouseEvent *event){
    //event->accept();
    setOpacity(0.7);

    if(isRed())
        viewAs("slash");
    else
        viewAs("jink");
}

void Card::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    setOpacity(1.0);
    if(view_card){
        view_card->setVisible(false);
    }

    goBack();
}

void Card::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        setPos(this->mapToParent(event->pos()) - event->buttonDownPos(Qt::LeftButton));
    }
}

void Card::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    static QRect suit_rect(8,8,18,18);
    painter->drawPixmap(CardRect, pixmap);
    painter->drawPixmap(suit_rect, suit_pixmap);

    painter->setFont(CardNumberFont);
    if(isRed())
        painter->setPen(Qt::red);
    painter->drawText(8, 50, getNumberString());
}

void Card::setMonochrome(){
    setGraphicsEffect(isEnabled()? NULL : monochrome_effect);
}
