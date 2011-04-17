#include "carditem.h"
#include "engine.h"
#include "skill.h"
#include "clientplayer.h"
#include "settings.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFocusEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

CardItem::CardItem(const Card *card)
    :Pixmap(card->getPixmapPath(), false), card(card), filtered_card(card)
{
    Q_ASSERT(card != NULL);

    suit_pixmap.load(QString("image/system/suit/%1.png").arg(card->getSuitString()));
    icon_pixmap.load(card->getIconPath());
    setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);

    setToolTip(card->getDescription());

    QPixmap frame_pixmap("image/system/frame/good.png");
    frame = new QGraphicsPixmapItem(frame_pixmap, this);
    frame->setPos(-6, -6);
    frame->hide();
}

CardItem::CardItem(const QString &general_name)
    :Pixmap(), card(NULL), filtered_card(NULL)
{
    const General *general = Sanguosha->getGeneral(general_name);
    changePixmap(general->getPixmapPath("card"));
}

const Card *CardItem::getCard() const{
    return card;
}

void CardItem::filter(const FilterSkill *filter_skill){
    if(filter_skill && filter_skill->viewFilter(this)){
        filtered_card = filter_skill->viewAs(this);
    }
}

const Card *CardItem::getFilteredCard() const{
    return filtered_card;
}

void CardItem::setHomePos(QPointF home_pos){
    this->home_pos = home_pos;
}

QPointF CardItem::homePos() const{
    return home_pos;
}

void CardItem::goBack(bool kieru){
    if(home_pos == pos()){
        if(kieru)
            setOpacity(0.0);
        return;
    }

    QPropertyAnimation *goback = new QPropertyAnimation(this, "pos");
    goback->setEndValue(home_pos);   
    goback->setEasingCurve(QEasingCurve::OutBounce);

    if(kieru){
        QParallelAnimationGroup *group = new QParallelAnimationGroup;

        QPropertyAnimation *disappear = new QPropertyAnimation(this, "opacity");
        disappear->setKeyValueAt(0.9, 1.0);
        disappear->setEndValue(0.0);

        goback->setDuration(1000);
        disappear->setDuration(1000);

        group->addAnimation(goback);
        group->addAnimation(disappear);

        // prevent the cover face bug
        setEnabled(false);

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

void CardItem::setFrame(const QString &result){
    QString path = QString("image/system/frame/%1.png").arg(result);
    QPixmap frame_pixmap(path);
    if(!frame_pixmap.isNull()){
        frame->setPixmap(frame_pixmap);
        frame->show();
    }
}

void CardItem::hideFrame(){
    frame->hide();
}

static inline bool IsMultilayer(){
    return Self && Self->getHandcardNum() > Config.MaxCards;
}

void CardItem::select(){
    if(IsMultilayer())
        frame->show();
    else{
        home_pos.setY(PendingY);
        setY(PendingY);
    }
}

void CardItem::unselect(){
    if(IsMultilayer())
        frame->hide();
    else{
        home_pos.setY(NormalY);
        setY(NormalY);
    }
}

bool CardItem::isPending() const{
    if(IsMultilayer())
        return frame->isVisible();
    else
        return home_pos.y() == PendingY;
}

bool CardItem::isEquipped() const{
    return Self->hasEquip(card);
}

CardItem *CardItem::FindItem(const QList<CardItem *> &items, int card_id){
    foreach(CardItem *item, items){
        if(item->getCard()->getId() == card_id)
            return item;
    }

    return NULL;
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus())
        emit clicked();
    else
        emit toggle_discards();
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(parentItem()){
        if(y() < -80)
            emit thrown();
    }else{
        emit grabbed();
    }

    goBack();
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        QPointF down_pos = event->buttonDownPos(Qt::LeftButton);
        setPos(this->mapToParent(event->pos()) - down_pos);
    }
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        event->accept();
        emit double_clicked();
    }
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    if(card){
        static QFont card_number_font("Times", 20, QFont::Bold);
        painter->drawPixmap(8, 8, 18, 18, suit_pixmap);

        painter->setFont(card_number_font);
        if(card->isRed())
            painter->setPen(Qt::red);
        else
            painter->setPen(Qt::black);
        painter->drawText(8, 50, card->getNumberString());
    }
}

GuanxingCardItem::GuanxingCardItem(const Card *card)
    :CardItem(card)
{
}

void GuanxingCardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    emit released();
}

