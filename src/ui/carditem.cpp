#include "carditem.h"
#include "engine.h"
#include "skill.h"
#include "clientplayer.h"
#include "settings.h"

#include <cmath>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QFocusEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

CardItem::CardItem(const Card *card)
    :Pixmap(card->getPixmapPath(), false), card(card), filtered_card(card), auto_back(true), frozen(false)

{
    Q_ASSERT(card != NULL);

    suit_pixmap.load(QString("image/system/suit/%1.png").arg(card->getSuitString()));
    cardsuit_pixmap.load(QString("image/system/cardsuit/%1.png").arg(card->getSuitString()));
    number_pixmap.load(QString("image/system/%1/%2.png").arg(card->isBlack()?"black":"red").arg(card->getNumberString()));
    icon_pixmap.load(card->getIconPath());
    setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);

    setToolTip(card->getDescription());
    setAcceptHoverEvents(true);

    QPixmap frame_pixmap("image/system/frame/good.png");
    frame = new QGraphicsPixmapItem(frame_pixmap, this);
    frame->setPos(-6, -6);
    frame->hide();

    avatar = NULL;

    owner_text = new QGraphicsSimpleTextItem(this);
    QPen pen(Qt::black);
    pen.setWidthF(0.5);
    owner_text->setPen(pen);
    owner_text->setBrush(Qt::yellow);
    owner_text->hide();
}

CardItem::CardItem(const QString &general_name)
    :card(NULL), filtered_card(NULL), auto_back(true), frozen(false)
{
    changeGeneral(general_name);
}

void CardItem::changeGeneral(const QString &general_name){
    setObjectName(general_name);

    const General *general = Sanguosha->getGeneral(general_name);
    if(general){
        changePixmap(general->getPixmapPath("card"));
        setToolTip(general->getSkillDescription());
    }else{
        changePixmap("image/system/unknown.png");
        setToolTip(QString());
    }
}

const Card *CardItem::getCard() const{
    return card;
}

void CardItem::filter(const FilterSkill *filter_skill){
    if(filter_skill){
        if(filter_skill->viewFilter(this))
            filtered_card = filter_skill->viewAs(this);
    }else
        filtered_card = card;
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

QAbstractAnimation* CardItem::goBack(bool kieru,bool fadein,bool fadeout){
    if(home_pos == pos()){
        if(kieru && home_pos != QPointF(-6, 8))
            setOpacity(0.0);
        return NULL;
    }

    QPropertyAnimation *goback = new QPropertyAnimation(this, "pos");
    goback->setEndValue(home_pos);
    goback->setEasingCurve(QEasingCurve::OutQuad);
    goback->setDuration(500);

    if(kieru){
        QParallelAnimationGroup *group = new QParallelAnimationGroup;

        QPropertyAnimation *disappear = new QPropertyAnimation(this, "opacity");
        if(fadein)disappear->setStartValue(0.0);
        disappear->setEndValue(1.0);
        if(fadeout)disappear->setEndValue(0.0);

        disappear->setKeyValueAt(0.2, 1.0);
        disappear->setKeyValueAt(0.8, 1.0);


        qreal dx = home_pos.x()-pos().x();
        qreal dy = home_pos.y()-pos().y();
        int length = sqrt(dx*dx+dy*dy);


        length = qBound(500/3,length,400);

        goback->setDuration(length*3);
        disappear->setDuration(length*3);

        group->addAnimation(goback);
        group->addAnimation(disappear);

        // prevent the cover face bug
        setEnabled(false);

        group->start(QParallelAnimationGroup::DeleteWhenStopped);
        return group;
    }else
    {
        setOpacity(this->isEnabled() ? 1.0 : 0.7);
        goback->start(QPropertyAnimation::DeleteWhenStopped);
        return goback;
    }
}

const QPixmap &CardItem::getSuitPixmap() const{
    return suit_pixmap;
}

const QPixmap &CardItem::getNumberPixmap() const{
    return number_pixmap;
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

void CardItem::showAvatar(const General *general){
    if(general){
        if(avatar == NULL){
            avatar = new QGraphicsPixmapItem(this);
            avatar->setPos(44, 87);
        }

        avatar->setPixmap(QPixmap(general->getPixmapPath("tiny")));
        avatar->show();
    }else{
        if(avatar)
            avatar->hide();
    }
}

void CardItem::hideFrame(){
    frame->hide();
}

void CardItem::setAutoBack(bool auto_back){
    this->auto_back = auto_back;
}

static inline bool IsMultilayer(){
    return Self && Self->getHandcardNum() > Config.MaxCards;
}

void CardItem::select(){
    if(IsMultilayer())
        frame->show();
    else{
        home_pos.setY(PendingY);
        //setY(PendingY);
        if(!hasFocus())goBack();
    }
}

void CardItem::unselect(){
    if(IsMultilayer())
        frame->hide();
    else{
        home_pos.setY(NormalY);
        //setY(NormalY);
        if(!hasFocus())goBack();
    }
}

bool CardItem::isPending() const{
    return IsMultilayer() ? frame->isVisible() :home_pos.y() == PendingY;
}

bool CardItem::isEquipped() const{
    return Self->hasEquip(card);
}

void CardItem::setFrozen(bool is_frozen){
    frozen = is_frozen;
}

bool CardItem::isFrozen() const{
    return frozen;
}

CardItem *CardItem::FindItem(const QList<CardItem *> &items, int card_id){
    foreach(CardItem *item, items){
        if(item->getCard()->getId() == card_id)
            return item;
    }

    return NULL;
}

void CardItem::reduceZ()
{
    if(this->zValue()>0)this->setZValue(this->zValue()-0.8);
}

void CardItem::promoteZ()
{
    if(this->zValue()<0)this->setZValue(this->zValue()+0.8);
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *){
    if(isFrozen())
        return;

    if(hasFocus())
        emit clicked();
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *){
    if(isFrozen())
        return;

    if(auto_back){
        if(parentItem()){
            if(y() < -80)
                emit thrown();
        }

        goBack();
    }else{
        emit released();
    }
    emit leave_hover();
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(hasFocus()){
        QPointF down_pos = event->buttonDownPos(Qt::LeftButton);
        setPos(this->mapToParent(event->pos() - down_pos));
    }
}

void CardItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    if(isFrozen())
        return;

    if(hasFocus()){
        event->accept();
        emit double_clicked();
    }
    else emit toggle_discards();
}

void CardItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    emit enter_hover();
}

void CardItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    emit leave_hover();
}

void CardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    if(card){
        painter->drawPixmap(0, 14, cardsuit_pixmap);
        painter->drawPixmap(0, 2, number_pixmap);
    }
}


void CardItem::writeCardDesc(QString desc)
{
     if(card){
         owner_text->setText(desc);

         QRectF owner_rect = owner_text->boundingRect();
         qreal x = (pixmap.width() - owner_rect.width())/2;
         qreal y = pixmap.height() - owner_rect.height() - 7;

         owner_text->setPos(x, y);
         owner_text->show();
     }
}

void CardItem::deleteCardDesc(){
    owner_text->hide();
}
