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

void CardItem::_initialize()
{
    this->_m_roomSkin = &(QSanSkinFactory::getInstance().getCurrentSkinScheme().getRoomSkin());
    this->_m_layout = &(_m_roomSkin->getCommonLayout());
}

CardItem::CardItem(const Card *card)
{
    _initialize();
    setCard(card);    
    m_isSelected = false;
    auto_back = true;
    frozen = false;
    m_isDescriptionVisible = true;

    setTransformOriginPoint(pixmap.width()/2, pixmap.height()/2);    
    setAcceptHoverEvents(true);

    QPixmap frame_pixmap("image/system/frame/good.png");
    frame = new QGraphicsPixmapItem(frame_pixmap, this);
    frame->setPos(-6, -6);
    frame->hide();

    avatar = NULL;
    m_opacityAtHome = 1.0;
    m_currentAnimation = NULL;
}

CardItem::CardItem(const QString &general_name)
    :m_card(NULL), filtered_card(NULL), auto_back(true), frozen(false)
{
    _initialize();
    changeGeneral(general_name);
    m_currentAnimation = NULL;
    m_opacityAtHome = 1.0;
}

void CardItem::setCard(const Card* card)
{      
    if (card != NULL)        
    {
        Pixmap::load(card->getPixmapPath(), false);
        icon_pixmap.load(card->getIconPath());
        suit_pixmap.load(QString("image/system/suit/%1.png").arg(card->getSuitString()));
        cardsuit_pixmap.load(QString("image/system/cardsuit/%1.png").arg(card->getSuitString()));
        number_pixmap.load(QString("image/system/%1/%2.png").arg(card->isBlack()?"black":"red").arg(card->getNumberString()));
        setToolTip(card->getDescription());
    }
    else
    {
        Pixmap::load("image/system/card-back.png");
    }
    m_card = card;
    filtered_card = card;
}

void CardItem::setEnabled(bool enabled)
{
     Pixmap::setEnabled(enabled);    
}

CardItem::~CardItem()
{
    m_animationMutex.lock();
    if (m_currentAnimation != NULL)
    {
        m_currentAnimation->deleteLater();
        m_currentAnimation = NULL;
    }
    m_animationMutex.unlock();
}

void CardItem::changeGeneral(const QString &general_name){
    setObjectName(general_name);

    const General *general = Sanguosha->getGeneral(general_name);
    if(general){
        load(general->getPixmapPath("card"));
        setToolTip(general->getSkillDescription());
    }else{
        load("image/system/unknown.png");
        setToolTip(QString());
    }
}

const Card *CardItem::getCard() const{
    return m_card;
}

void CardItem::filter(const FilterSkill *filter_skill){
    if(filter_skill){
        if(filter_skill->viewFilter(this))
            filtered_card = filter_skill->viewAs(this);
    }else
        filtered_card = m_card;
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

void CardItem::goBack(bool playAnimation, bool doFade){
    if (playAnimation)
    {
        getGoBackAnimation(doFade);
        if (m_currentAnimation != NULL)
        {
            m_currentAnimation->start();
        }
    }
    else
    {
        m_animationMutex.lock();
        if (m_currentAnimation != NULL)
        {
            m_currentAnimation->stop();
            delete m_currentAnimation;
            m_currentAnimation = NULL;
        }
        setPos(homePos());        
        m_animationMutex.unlock();
    }
}

QAbstractAnimation* CardItem::getGoBackAnimation(bool doFade)
{
    m_animationMutex.lock();
    if (m_currentAnimation != NULL)
    {
        m_currentAnimation->stop();
        delete m_currentAnimation;
        m_currentAnimation = NULL;
    }
    QPropertyAnimation *goback = new QPropertyAnimation(this, "pos");
    goback->setEndValue(home_pos);
    goback->setEasingCurve(QEasingCurve::OutQuad);
    goback->setDuration(Config.S_MOVE_CARD_ANIMATION_DURAION);

    if(doFade){
        QParallelAnimationGroup *group = new QParallelAnimationGroup;

        QPropertyAnimation *disappear = new QPropertyAnimation(this, "opacity");        
        double middleOpacity = qMax(opacity(), m_opacityAtHome);
        if (middleOpacity == 0) middleOpacity = 1.0;        
        disappear->setEndValue(m_opacityAtHome);
        disappear->setKeyValueAt(0.2, middleOpacity);
        disappear->setKeyValueAt(0.8, middleOpacity);
        disappear->setDuration(Config.S_MOVE_CARD_ANIMATION_DURAION);

        group->addAnimation(goback);
        group->addAnimation(disappear);

        // card is disabled while moving
        // setEnabled(false);       
        m_currentAnimation = group;
    }
    else
    {      
        m_currentAnimation = goback;
    }
    m_animationMutex.unlock();
    connect(m_currentAnimation, SIGNAL(finished()), this, SIGNAL(movement_animation_finished()));
    return m_currentAnimation;
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

bool CardItem::isEquipped() const{
    return Self->hasEquip(m_card);
}

void CardItem::setFrozen(bool is_frozen){
    frozen = is_frozen;
}

bool CardItem::isFrozen() const{
    return frozen;
}

CardItem *CardItem::FindItem(const QList<CardItem *> &items, int card_id){
    foreach(CardItem *item, items){
        if (item->getCard() == NULL)
        {
            if (card_id == Card::S_UNKNOWN_CARD_ID) return item;
            else continue;
        }
        if(item->getCard()->getId() == card_id)
            return item;
    }

    return NULL;
}

void CardItem::reduceZ()
{
    if (this->zValue()>0) this->setZValue(this->zValue()-0.8);
}

void CardItem::promoteZ()
{
    if (this->zValue()<0) this->setZValue(this->zValue()+0.8);
}

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *){
    if(isFrozen())
        return;

    if(hasFocus())
        emit clicked();
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *){
    if (isFrozen()) return;

    if (auto_back){        
        goBack(true, false);
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
    
    if (!isEnabled())
    {
        painter->fillRect(this->boundingRect(), QColor(100, 100, 100, 255 * opacity()));
        painter->setOpacity(0.7 * opacity());
    }

    Pixmap::paint(painter, option, widget);

    if (m_card) {
        painter->drawPixmap(_m_layout->m_cardSuitArea, cardsuit_pixmap);
        painter->drawPixmap(_m_layout->m_cardNumberArea, number_pixmap);        
        if (this->m_isDescriptionVisible)
        {
            // first, draw shadow
            const QSanRoomSkin::QSanTextFont& font = _m_layout->m_cardFootnoteFont;
            font.paintText(painter, _m_layout->m_cardFootnoteArea, Qt::AlignCenter, owner_text);            
        }
    }
}


void CardItem::setFootnote(QString desc)
{
     owner_text = desc;
}


