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

void CardItem::_initialize()
{
    m_opacityAtHome = 1.0;
    m_currentAnimation = NULL;
    _m_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    _m_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    _m_footnoteItem = new QGraphicsPixmapItem(this);
    m_isSelected = false;
    auto_back = true;
    frozen = false;
    resetTransform();
    this->translate(-_m_width / 2, -_m_height / 2);
}

CardItem::CardItem(const Card *card)
{
    _initialize();
    setCard(card);    
    setAcceptHoverEvents(true);
}

CardItem::CardItem(const QString &general_name)
    :m_card(NULL), filtered_card(NULL)
{
    _initialize();
    changeGeneral(general_name);
    m_currentAnimation = NULL;
    m_opacityAtHome = 1.0;
}

QRectF CardItem::boundingRect() const
{
    return G_COMMON_LAYOUT.m_cardFrameArea;
}

void CardItem::setCard(const Card* card)
{      
    if (card != NULL) 
    {
        setObjectName(card->objectName());
        setToolTip(card->getDescription());
    }
    else
        setObjectName("unknown");
    m_card = card;
    filtered_card = card;
}

void CardItem::setEnabled(bool enabled)
{
     QSanSelectableItem::setEnabled(enabled);    
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
        setToolTip(general->getSkillDescription());
    }else{
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

void CardItem::showFrame(const QString &result){
    _m_frameType = result;    
}

void CardItem::hideFrame(){
    _m_frameType = QString();
}

void CardItem::showAvatar(const General *general){
    _m_avatarName = general->objectName();
}

void CardItem::hideAvatar()
{
    _m_avatarName = QString();
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

const int CardItem::_S_CLICK_JITTER_TOLERANCE = 20;

void CardItem::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if(isFrozen()) return;
    _m_lastMousePressScenePos = mapToParent(mouseEvent->pos());
}

void CardItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (isFrozen()) return;
    
    QPointF totalMove = mapToParent(mouseEvent->pos()) - _m_lastMousePressScenePos;
    if (totalMove.x() * totalMove.x() + totalMove.y() * totalMove.y() 
        < _S_CLICK_JITTER_TOLERANCE) 
    {
        emit clicked();
    }
    else
    {
        emit released();
    }

    if (auto_back){        
        goBack(true, false);
    }
}

void CardItem::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent){
    QPointF newPos = mapToParent(mouseEvent->pos());
    QPointF totalMove = newPos - _m_lastMousePressScenePos;
    if(totalMove.x() * totalMove.x() + totalMove.y() * totalMove.y() 
        >= _S_CLICK_JITTER_TOLERANCE)// hasFocus()){
    {
        QPointF down_pos = mouseEvent->buttonDownPos(Qt::LeftButton);
        setPos(newPos - this->transform().map(down_pos));
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
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    
    if (!_m_frameType.isEmpty())
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardFrameArea, G_ROOM_SKIN.getCardAvatarPixmap(_m_frameType));
    
    if (!isEnabled())
    {
        painter->fillRect(G_COMMON_LAYOUT.m_cardMainArea, QColor(100, 100, 100, 255 * opacity()));
        painter->setOpacity(0.7 * opacity());
    }

    painter->drawPixmap(G_COMMON_LAYOUT.m_cardMainArea, G_ROOM_SKIN.getCardMainPixmap(objectName()));
    if (m_card) {		
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardSuitArea, G_ROOM_SKIN.getCardSuitPixmap(m_card->getSuit()));
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardNumberArea, G_ROOM_SKIN.getCardNumberPixmap(m_card->getNumber(), m_card->isBlack()));
    }
    
    if (!_m_avatarName.isEmpty())
        painter->drawPixmap(G_COMMON_LAYOUT.m_cardAvatarArea, G_ROOM_SKIN.getCardAvatarPixmap(_m_avatarName));

}


void CardItem::setFootnote(const QString &desc)
{
    const IQSanComponentSkin::QSanShadowTextFont& font = G_COMMON_LAYOUT.m_cardFootnoteFont;
    font.paintText(_m_footnoteItem, G_COMMON_LAYOUT.m_cardFootnoteArea, 
                   (Qt::AlignmentFlag)((int)Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWrapAnywhere), desc);            
}


