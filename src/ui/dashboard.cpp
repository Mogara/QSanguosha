#include "dashboard.h"
#include "engine.h"
#include "settings.h"
#include "client.h"
#include "standard.h"
#include "playercarddialog.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPixmapCache>
#include <QParallelAnimationGroup>

using namespace QSanProtocol;

const QRect Dashboard::S_EQUIP_CARD_MOVE_REGION(0, 0,
    CardItem::S_NORMAL_CARD_WIDTH * 1.5, CardItem::S_NORMAL_CARD_HEIGHT);
const QRect Dashboard::S_JUDGE_CARD_MOVE_REGION(0, 0, 
    CardItem::S_NORMAL_CARD_WIDTH * 1.5, CardItem::S_NORMAL_CARD_HEIGHT);

Dashboard::Dashboard(QGraphicsItem *button_widget)
    :left_pixmap("image/system/dashboard-equip.png"), right_pixmap("image/system/dashboard-avatar.png"),
    button_widget(button_widget), selected(NULL), avatar(NULL),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL),
    view_as_skill(NULL), filter(NULL)
{
    createLeft();
    createMiddle();
    createRight();
    death_item = NULL;

    if(button_widget)
        button_widget->setParentItem(this);

    int middle_width = middle->rect().width();
    setMiddleWidth(middle_width);

    sort_type = 0;

    animations = new EffectAnimation();
    
    QPointF specialCenter = mapFromItem(avatar, avatar->boundingRect().width() / 2, 0);
    m_cardSpecialRegion = QRectF(specialCenter.x() - CardItem::S_NORMAL_CARD_WIDTH * 1.5,
                                  specialCenter.y() - CardItem::S_NORMAL_CARD_HEIGHT,
                                  CardItem::S_NORMAL_CARD_WIDTH * 3,
                                  CardItem::S_NORMAL_CARD_HEIGHT);
    _addProgressBar();
}

void Dashboard::createLeft(){
    left = new QGraphicsRectItem(QRectF(left_pixmap.rect()), this);

    equips << &weapon << &armor << &defensive_horse << &offensive_horse;

    int i;
    for(i=0; i<4; i++){
        QRectF rect(8, 40 + 32 *i, 117, 25);
        equip_rects[i] = new QGraphicsRectItem(rect, left);
        equip_rects[i]->setPen(Qt::NoPen);
    }
}

int Dashboard::getButtonWidgetWidth() const{
    if(button_widget)
        return button_widget->boundingRect().width();
    else
        return 0;
}

void Dashboard::createMiddle(){
    middle = new QGraphicsRectItem(this);

    QPixmap middle_pixmap("image/system/dashboard-hand.png");
    QBrush middle_brush(middle_pixmap);
    middle->setBrush(middle_brush);
    middle->setRect(0, 0, middle_pixmap.width(), middle_pixmap.height());

    trusting_item = new QGraphicsRectItem(this);
    trusting_item->setRect(middle->rect());
    QBrush trusting_brush(QColor(0x26, 0x1A, 0x42));
    trusting_item->setBrush(trusting_brush);
    trusting_item->setOpacity(0.36);
    trusting_item->setZValue(2.0);

    trusting_text = new QGraphicsSimpleTextItem(tr("Trusting ..."), this);
    trusting_text->setFont(Config.BigFont);
    trusting_text->setBrush(Qt::white);
    trusting_text->setZValue(2.1);

    trusting_item->hide();
    trusting_text->hide();
}

void Dashboard::createRight(){
    right = new QGraphicsRectItem(QRectF(right_pixmap.rect()), this);

    avatar = new Pixmap;
    avatar->setPos(22, 64);
    avatar->setParentItem(right);

    small_avatar = new Pixmap;
    small_avatar->setPos(21, 63);
    small_avatar->setParentItem(right);
    small_avatar->setOpacity(0.75);

    if(button_widget){
        kingdom = new QGraphicsPixmapItem(button_widget);
        kingdom->setPos(57, 0);
    }else{
        kingdom = new QGraphicsPixmapItem(right);
        kingdom->setPos(91, 54);
    }

    ready_item = new QGraphicsPixmapItem(QPixmap("image/system/ready.png"), avatar);
    ready_item->setPos(2, 43);
    ready_item->hide();

    chain_icon = new Pixmap("image/system/chain.png");
    chain_icon->setParentItem(right);
    chain_icon->setPos(small_avatar->pos());
    chain_icon->moveBy(-25 ,-45);
    chain_icon->hide();
    chain_icon->setZValue(1.0);

    back_icon = new Pixmap("image/system/big-back.png");
    back_icon->setParentItem(right);
    back_icon->setPos(59, 105);
    back_icon->setZValue(1.0);
    back_icon->hide();

    QGraphicsPixmapItem *handcard_pixmap = new QGraphicsPixmapItem(right);
    handcard_pixmap->setPixmap(QPixmap("image/system/handcard.png"));
    handcard_pixmap->setPos(25, 127);

    handcard_num = new QGraphicsSimpleTextItem(handcard_pixmap);
    handcard_num->setPos(6,8);

     QFont serifFont("Times", 10, QFont::Bold);
    handcard_num->setFont(serifFont);
    handcard_num->setBrush(Qt::white);

    handcard_pixmap->hide();

    mark_item = new QGraphicsTextItem(right);
    mark_item->setPos(-128 - getButtonWidgetWidth(), 0);
    mark_item->setDefaultTextColor(Qt::white);

    action_item = NULL;
}

void Dashboard::setActionState(){
    if(action_item == NULL){
        action_item = new QGraphicsPixmapItem(right);
        action_item->setPixmap(QPixmap("image/system/3v3/actioned.png"));
        action_item->setPos(64, 138);
    }

    action_item->setVisible(Self->hasFlag("actioned"));
}

void Dashboard::setFilter(const FilterSkill *filter){
    this->filter = filter;
    doFilter();
}

const FilterSkill *Dashboard::getFilter() const{
    return filter;
}

void Dashboard::doFilter(){
    foreach(CardItem *card_item, m_handCards)
        card_item->filter(filter);
}

void Dashboard::setTrust(bool trust){
    trusting_item->setVisible(trust);
    trusting_text->setVisible(trust);
}

bool Dashboard::_addCardItems(QList<CardItem*> &card_items, Player::Place place)
{
    if (place == Player::Equip)
        _disperseCards(card_items, S_EQUIP_CARD_MOVE_REGION, Qt::AlignCenter, true);
    else if (place == Player::Judging)
        _disperseCards(card_items, S_JUDGE_CARD_MOVE_REGION, Qt::AlignCenter, true);
    else if (place == Player::PlaceTakeoff)
    {
        m_takenOffCards.append(card_items);
        foreach (CardItem* card, card_items)
        {
            card->setHomeOpacity(1.0);
        }
        _disperseCards(card_items, m_cardTakeOffRegion, Qt::AlignCenter, true);
        return false;
    }
    else if (place == Player::Special)
    {
        foreach(CardItem* card, card_items)
        {
            card->setHomeOpacity(0.0);            
        }
        _disperseCards(card_items, m_cardSpecialRegion, Qt::AlignCenter, true);
        return true;
    }

    foreach (CardItem* card_item, card_items)
    {
        if (place == Player::Equip)        
            _installEquip(card_item);
        else if (place == Player::Judging)
            _installDelayedTrick(card_item);
        else if (place == Player::Hand)
            _addHandCard(card_item);
    }
    
    if (place == Player::Hand)
        sortCards(sort_type, true);        
    return false;
}

void Dashboard::_addHandCard(CardItem* card_item){
    card_item->filter(filter);

    if(ClientInstance->getStatus() == Client::Playing)
        card_item->setEnabled(card_item->getFilteredCard()->isAvailable(Self));
    else
        card_item->setEnabled(false);
    
    card_item->setHomeOpacity(1.0); // @todo: it's 1.0 even if disabled?
    card_item->setRotation(0.0);
    card_item->setFlags(ItemIsFocusable);
    card_item->setZValue(0.1);
    m_handCards << card_item;

    connect(card_item, SIGNAL(clicked()), this, SLOT(onCardItemClicked()));
    connect(card_item, SIGNAL(thrown()), this, SLOT(onCardItemThrown()));
    connect(card_item, SIGNAL(enter_hover()), this, SLOT(onCardItemHover()));
    connect(card_item, SIGNAL(leave_hover()), this, SLOT(onCardItemLeaveHover()));      
}

void Dashboard::setPlayer(const ClientPlayer *player){
    connect(player, SIGNAL(state_changed()), this, SLOT(refresh()));
    connect(player, SIGNAL(kingdom_changed()), this, SLOT(updateAvatar()));
    connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));
    connect(player, SIGNAL(action_taken()), this, SLOT(setActionState()));
    connect(player, SIGNAL(ready_changed(bool)), this, SLOT(updateReadyItem(bool)));

    mark_item->setDocument(player->getMarkDoc());

    updateAvatar();
}

void Dashboard::updateAvatar(){
    const General *general = Self->getAvatarGeneral();
    avatar->setToolTip(general->getSkillDescription());
    if(!avatar->changePixmap(general->getPixmapPath("big"))){
        QPixmap pixmap(General::BigIconSize);
        pixmap.fill(Qt::black);

        QPainter painter(&pixmap);

        painter.setPen(Qt::white);
        painter.setFont(Config.SmallFont);
        painter.drawText(0, 0, pixmap.width(), pixmap.height(),
                         Qt::AlignCenter,
                         Sanguosha->translate(Self->getGeneralName()));

        avatar->setPixmap(pixmap);
    }

    QString folder = button_widget ? "button" : "icon";
    kingdom->setPixmap(QPixmap(QString("image/kingdom/%1/%2.png").arg(folder).arg(Self->getKingdom())));

    avatar->show();
    kingdom->show();

    update();
}

void Dashboard::updateSmallAvatar(){
    const General *general2 = Self->getGeneral2();
    if(general2){
        small_avatar->setToolTip(general2->getSkillDescription());
        bool success = small_avatar->changePixmap(general2->getPixmapPath("tiny"));

        if(!success){
            QPixmap pixmap(General::TinyIconSize);
            pixmap.fill(Qt::black);

            QPainter painter(&pixmap);

            painter.setPen(Qt::white);
            painter.drawText(0, 0, pixmap.width(), pixmap.height(),
                             Qt::AlignCenter,
                             Sanguosha->translate(Self->getGeneral2Name()));

            small_avatar->setPixmap(pixmap);
        }
    }

    update();
}

void Dashboard::updateReadyItem(bool visible){
    ready_item->setVisible(visible);
}

// similar with Photo::refresh, just an alias to update
void Dashboard::refresh(){
    update();
}

Pixmap *Dashboard::getAvatar(){
    return avatar;
}

void Dashboard::selectCard(const QString &pattern, bool forward){
    if(selected)
        selectCard(selected, true); // adjust the position

    // find all cards that match the card type
    QList<CardItem*> matches;

    foreach(CardItem *card_item, m_handCards){
        if(card_item->isEnabled()){
            if(pattern == "." || card_item->getFilteredCard()->match(pattern))
                matches << card_item;
        }
    }

    if(matches.isEmpty()){
        unselectAll();
        return;
    }

    int index = matches.indexOf(selected);
    int n = matches.length();
    if(forward)
        index = (index + 1) % n;
    else
        index = (index - 1 + n) % n;

    CardItem *to_select = matches[index];

    if(to_select != selected){
        if(selected)
            selectCard(selected, false);
        selectCard(to_select, true);
        selected = to_select;

        emit card_selected(selected->getFilteredCard());
    }
}

const Card *Dashboard::getSelected() const{
    if(view_as_skill)
        return pending_card;
    else if(selected)
        return selected->getFilteredCard();
    else
        return NULL;
}

void Dashboard::selectCard(CardItem* item, bool isSelected){
    //if(Self && Self->getHandcardNum() > Config.MaxCards)
    //    frame->show();    
    bool oldState = item->isSelected();
    if (oldState == isSelected) return;
    m_mutex.lock();     
    item->setSelected(isSelected);
    QPointF oldPos = item->homePos();
    QPointF newPos = oldPos;
    if (isSelected)
        newPos.setY(newPos.y() + S_PENDING_OFFSET_Y);
    else 
        newPos.setY(newPos.y() - S_PENDING_OFFSET_Y);
    item->setHomePos(newPos);    
    item->setHomeOpacity(item->isEnabled() ? 1.0 : 0.7);
    //setY(PendingY);
    if (!hasFocus()) item->goBack(true);    
    m_mutex.unlock();
}

void Dashboard::unselectAll(){
    selected = NULL;

    foreach(CardItem *card_item, m_handCards){
        selectCard(card_item, false);
        //card_item->goBack();
    }
}

void Dashboard::hideAvatar(){
    avatar->hide();

    if(button_widget == NULL)
        kingdom->hide();
}

void Dashboard::_installDelayedTrick(CardItem *card){
    card->setHomeOpacity(0.0);
    judging_area << card;
    const DelayedTrick *trick = DelayedTrick::CastFrom(card->getCard());
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(this);
    item->setPixmap(QPixmap(trick->getIconPath()));
    QString tooltip;
    if(trick->isVirtualCard())
        tooltip=Sanguosha->getCard((trick->getSubcards()).at(0))->getDescription();
    else
        tooltip=trick->getDescription();
    item->setToolTip(tooltip);
    item->setPos(3 + delayed_tricks.length() * 27, 5);
    delayed_tricks << item;
}

QRectF Dashboard::boundingRect() const{
    qreal width = left->boundingRect().width() + middle->rect().width() + right->boundingRect().width();
    qreal height = middle->rect().height();
    return QRectF(0, 0, width, height);
}

int Dashboard::getTextureWidth() const{
    return middle->brush().texture().width() + left_pixmap.width() + right_pixmap.width();
}

void Dashboard::setMiddleWidth(int middle_width){
    int left_width = left_pixmap.width();
    qreal middle_height = middle->rect().height();

    middle->setRect(0, 0, middle_width + getButtonWidgetWidth(), middle_height);
    middle->setX(left_width);

    if(button_widget)
        button_widget->setX(left_width + middle_width);

    right->setX(left_width + middle_width + getButtonWidgetWidth());

    trusting_item->setRect(middle->rect());
    trusting_item->setX(left_width);
    trusting_text->setPos(middle_width/2, middle_height/2);
}

void Dashboard::setWidth(int width){
    qreal left_width = left->boundingRect().width();
    qreal right_width = right->boundingRect().width();
    qreal button_width = getButtonWidgetWidth();
    qreal middle_width = width - left_width - right_width - button_width;
    
    setMiddleWidth(middle_width);
    prepareGeometryChange();
    adjustCards();
    setX(-boundingRect().width()/2);
    m_cardTakeOffRegion = middle->boundingRect();    
    m_cardTakeOffRegion.setY(middle->pos().y() - CardItem::S_NORMAL_CARD_HEIGHT * 1.5);
    m_cardTakeOffRegion.setHeight(CardItem::S_NORMAL_CARD_HEIGHT);
}

QGraphicsProxyWidget *Dashboard::addWidget(QWidget *widget, int x, bool from_left){
    QGraphicsProxyWidget *proxy_widget = new QGraphicsProxyWidget(this);
    proxy_widget->setWidget(widget);
    proxy_widget->setParentItem(from_left ? left : right);
    proxy_widget->setPos(x, -32);

    return proxy_widget;
}

QPushButton *Dashboard::createButton(const QString &name){
    QPushButton *button = new QPushButton;
    button->setEnabled(false);

    QPixmap icon_pixmap(QString("image/system/button/%1.png").arg(name));
    QPixmap icon_pixmap_disabled(QString("image/system/button/%1-disabled.png").arg(name));

    QIcon icon(icon_pixmap);
    icon.addPixmap(icon_pixmap_disabled, QIcon::Disabled);

    //button->setIcon(icon);
    //button->setIconSize(icon_pixmap.size());
    button->setFixedSize(icon_pixmap.size());
    button->setObjectName(name);
    button->setProperty("game_control",true);

    button->setAttribute(Qt::WA_TranslucentBackground);

    return button;
}

QPushButton *Dashboard::addButton(const QString &name, int x, bool from_left){
    QPushButton *button = createButton(name);
    addWidget(button, x, from_left);
    return button;
}

void Dashboard::_addProgressBar()
{    
    m_progressBar.setFixedSize(300, 15);
    m_progressBar.setTextVisible(false);
    QGraphicsProxyWidget *widget = new QGraphicsProxyWidget(right);
    widget->setWidget(&m_progressBar);
    widget->setParentItem(middle);
    widget->setPos(300, - 25);
    connect(&m_progressBar, SIGNAL(timedOut()), this, SIGNAL(progressBarTimedOut()));
    m_progressBar.hide();    
}

void Dashboard::hideProgressBar()
{
    m_progressBar.hide();
}

void Dashboard::showProgressBar(Countdown countdown)
{
    m_progressBar.setCountdown(countdown);
    m_progressBar.show();
}

void Dashboard::drawHp(QPainter *painter) const{
    int hp = qMax(0, Self->getHp());
    int max_hp = Self->getMaxHp();
    QPixmap *magatama, *zero_magatama;
    int index = Self->isWounded() ? qMin(hp, 5) : 5;
    if(max_hp > 6){
        magatama = MagatamaWidget::GetSmallMagatama(index);
        zero_magatama = MagatamaWidget::GetSmallMagatama(0);
    }else{
        magatama = MagatamaWidget::GetMagatama(index);
        zero_magatama = MagatamaWidget::GetMagatama(0);
    }

    qreal total_width = magatama->width() * max_hp;
    qreal skip = (121 - total_width)/ (max_hp + 1);
    qreal start_x = left_pixmap.width() + middle->rect().width();

    int i;
    for(i=0; i<hp; i++)
        painter->drawPixmap(start_x + skip *(i+1) + i * magatama->width(), 5, *magatama);
    for(i=hp; i<max_hp; i++)
        painter->drawPixmap(start_x + skip *(i+1) + i * magatama->width(), 5, *zero_magatama);
}

void Dashboard::killPlayer(){
    if(death_item){
        delete death_item;
        death_item = NULL;
    }

    death_item = new QGraphicsPixmapItem(QPixmap(Self->getDeathPixmapPath()), this);
    death_item->setPos(397, 55);

    filter = NULL;

    avatar->makeGray();
    small_avatar->makeGray();
}

void Dashboard::revivePlayer(){
    if(death_item){
        delete death_item;
        death_item = NULL;
    }

    updateAvatar();
}

void Dashboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    
    if(Self->isKongcheng())
        handcard_num->parentItem()->hide();
    else{
        handcard_num->setText(QString::number(Self->getHandcardNum()));
    }

    // draw the left side and right side
    painter->drawPixmap(left->pos(), left_pixmap);
    painter->drawPixmap(right->pos(), right_pixmap);

    // draw player's name
    painter->setPen(Qt::white);
    QRectF name_rect(24 + right->x(), 42 + right->y(), 90, 12);
    painter->drawText(name_rect, Config.UserName, QTextOption(Qt::AlignHCenter));

    if(!Self)
        return;

    // draw player's equip area
    painter->setPen(Qt::white);

    drawEquip(painter, weapon, 0);
    drawEquip(painter, armor, 1);
    drawEquip(painter, defensive_horse, 2);
    drawEquip(painter, offensive_horse, 3);

    drawHp(painter);

    chain_icon->setVisible(Self->isChained());
    back_icon->setVisible(!Self->faceUp());
}

void Dashboard::mousePressEvent(QGraphicsSceneMouseEvent *){
    CardItem *to_select = NULL;
    int i;
    for(i=0; i<4; i++){
        if(equip_rects[i]->isUnderMouse()){
            to_select = *equips.at(i);
            break;
        }
    }

    if(to_select && to_select->isMarkable()){
        to_select->mark(!to_select->isMarked());

        update();
    }
}

void Dashboard::drawEquip(QPainter *painter, const CardItem *equip, int order){
    if(!equip)
        return;

    static const int width = 145;
    static const int start_y = 40;

    int y = start_y + order * 32;

    const EquipCard *card = qobject_cast<const EquipCard *>(equip->getCard());
    painter->setPen(Qt::black);

    // draw image or name
    QString path = QString("image/equips/%1.png").arg(card->objectName());
    QPixmap label;
    if(!QPixmapCache::find(path, &label)){
        label.load(path);
        QPixmapCache::insert(path, label);
    }

    if(label.isNull())
    {
        painter->setPen(Qt::white);
        QString text = QString("%1").arg(card->label());
        painter->drawText(10, y + 20, text);
    }else
    {
        QFont font("Algerian",12);
        font.setBold(true);
        painter->setFont(font);
        painter->drawPixmap(8,y + 2,label.width(),label.height(),label);
    }

    // draw the suit of equip
    QRect suit_rect(width - 19, y + 10, 13, 13);
    painter->drawPixmap(suit_rect, equip->getSuitPixmap());


    // draw the number of equip

    //painter->drawText(width - 4,y + 23,QString("%1").arg(card->getNumberString()));
    painter->drawPixmap(width - 14,y + 3,equip->getNumberPixmap());


    painter->setPen(Qt::white);
    if(equip->isMarked()){
        painter->drawRect(8,y + 2,label.width(),label.height());
    }
}    

void Dashboard::adjustCards(bool playAnimation){
    _adjustCards();
    foreach (CardItem* card, m_handCards)
    {
        card->goBack(playAnimation);
    }        
}

void Dashboard::_adjustCards(){
    int MaxCards = Config.MaxCards;

    foreach (CardItem* card, m_handCards)
    {
        card->setSelected(false);
        card->setHomeOpacity(card->isEnabled() ? 1.0 : 0.7);
    }

    int n = m_handCards.length();

    if(n > MaxCards){
        if(n > MaxCards * 2)
            MaxCards *= 2;

        if(n > MaxCards * 2)
            MaxCards *= 2;

        QList<CardItem *> all_items;

        int i, j, row_count = (n-1) / MaxCards + 1;
        int diff = (int) -((double)S_PENDING_OFFSET_Y / (row_count -1));

        for(i = 0; i < row_count; i ++){
            QList<CardItem *> row;
            for(j=0; j<MaxCards; j++){
                int index = i*MaxCards + j;
                if(index >= n)
                    break;

                row << m_handCards.at(index);
            }

            QListIterator<CardItem *> itor(row);
            itor.toBack();
            while(itor.hasPrevious())
                all_items.prepend(itor.previous());

            _adjustCards(row, S_CARD_NORMAL_Y - diff * i);
        }

        // reset Z value
        for(int i = 0; i < n; i++)
            all_items.at(i)->setZValue(2.0 + 0.0001 * i);
    }else{
        _adjustCards(m_handCards, S_CARD_NORMAL_Y);

        if(view_as_skill)
        {
            _adjustCards(pendings, S_CARD_NORMAL_Y);
        }
    }
}

void Dashboard::_adjustCards(const QList<CardItem *> &list, int y){    
    if (list.isEmpty()) return;

    int max_width = middle->rect().width() - getButtonWidgetWidth();
    int start_x = left->boundingRect().width();
    
    if(list.length() == 1){
        list.first()->setHomePos(QPointF(start_x, y));
    }

    int n = list.size();
    int card_width = list.first()->boundingRect().width();
    int total_width = qMin(card_width * n, max_width);
    int card_skip = 0;
    if (n > 1)
        card_skip = (total_width - n * card_width ) / (n - 1) + card_width;
        
    for(int i = 0; i < n; i++){
        if(m_handCards.length() <= Config.MaxCards)
            list[i]->setZValue(2.0 + 0.0001 * i);

        QPointF home_pos(start_x + i * card_skip, y);
        list[i]->setHomePos(home_pos);
    }
}

void Dashboard::_installEquip(CardItem *equip){    
    int index = -1;
    equip->setHomeOpacity(0.0);
    const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard());
    switch(equip_card->location()){
    case EquipCard::WeaponLocation: weapon = equip; index = 0; break;
    case EquipCard::ArmorLocation: armor = equip; index = 1; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = equip; index = 2; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = equip; index = 3; break;
    }

    if(index >= 0)
        equip_rects[index]->setToolTip(equip_card->getDescription());
}

QList<CardItem*> Dashboard::removeCardItems(const QList<int> &card_ids, Player::Place place){
    CardItem *card_item = NULL;
    QList<CardItem*> result;
    foreach (int card_id, card_ids)
    {
        if(place == Player::Hand){
            card_item = CardItem::FindItem(m_handCards, card_id);
            if (card_item == selected) selected = NULL;
            m_handCards.removeOne(card_item);            
            card_item->hideFrame();
        }
        else if(place == Player::Equip){
            foreach(CardItem **equip_ptr, equips){
                CardItem *equip = *equip_ptr;
                if(equip && equip->getCard()->getId() == card_id){
                    card_item = equip;
                    *equip_ptr = NULL;
                    card_item->setParent(this);
                    int index = equips.indexOf(equip_ptr);
                    equip_rects[index]->setToolTip(QString());
                    break;
                }
            }
        }else if(place == Player::Judging){
            card_item = CardItem::FindItem(judging_area, card_id);
            if(card_item){
                card_item->hideFrame();
                card_item->setParent(this);
                int index = judging_area.indexOf(card_item);
                judging_area.removeAt(index);
                delete delayed_tricks.takeAt(index);
                for(int i=0; i<delayed_tricks.count(); i++){
                    delayed_tricks.at(i)->setPos(3 + i * 27, 5);
                }
            }
        }else if(place == Player::PlaceTakeoff){
            card_item = CardItem::FindItem(m_takenOffCards, card_id);
            if (card_item == NULL)
                card_item = CardItem::FindItem(m_takenOffCards, -1);
            if (card_item == NULL)
            {
                Q_ASSERT(!m_takenOffCards.isEmpty());
                card_item = m_takenOffCards.first();
            }
            int index = m_takenOffCards.indexOf(card_item);
            m_takenOffCards.removeAt(index);
            if (card_item->getId() == Card::S_UNKNOWN_CARD_ID)
            {
                const Card* card = Sanguosha->getCard(card_id);
                card_item->setCard(card);
            }
            card_item->setOpacity(1.0);
        }else if (place == Player::Special){
            card_item = _createCard(card_id);
            card_item->setOpacity(0.0);            
        }
        if(card_item)
        {
            card_item->disconnect(this);
            result.append(card_item);            
        }
    }
    if (place == Player::Hand)    
        adjustCards();
    else if (place == Player::Equip)
        _disperseCards(result, S_EQUIP_CARD_MOVE_REGION, Qt::AlignCenter, false);
    else if (place == Player::Judging)
        _disperseCards(result, S_JUDGE_CARD_MOVE_REGION, Qt::AlignCenter, false);
    else if (place == Player::PlaceTakeoff)
        _disperseCards(result, m_cardTakeOffRegion, Qt::AlignCenter, false);
    else if (place == Player::Special)
        _disperseCards(result, m_cardSpecialRegion, Qt::AlignCenter, false);
    update();
    return result;
}

typedef bool (*CompareFunc)(const CardItem *, const CardItem *);

static bool CompareByColor(const CardItem *a, const CardItem *b){
    return Card::CompareByColor(a->getCard(), b->getCard());
}

static bool CompareBySuitNumber(const CardItem *a, const CardItem *b){
    return Card::CompareBySuitNumber(a->getCard(), b->getCard());
}

static bool CompareByType(const CardItem *a, const CardItem *b){
    return Card::CompareByType(a->getCard(), b->getCard());
}

static bool CompareByAvailability(const CardItem *a, const CardItem *b){
    bool x = a->isEnabled();
    bool y = b->isEnabled();
    if(x != y)
        return y;
    else
        return CompareBySuitNumber(a, b);
}

void Dashboard::sortCards(int sort_type, bool doAnimation){
    this->sort_type = sort_type;

    static CompareFunc compare_funcs[] = {
        NULL,
        CompareByColor,
        CompareBySuitNumber,
        CompareByType,
        CompareByAvailability,
    };

    CompareFunc func = compare_funcs[sort_type];
    if(func)
        qSort(m_handCards.begin(), m_handCards.end(), func);    
    if (doAnimation)
        adjustCards();
}

void Dashboard::reverseSelection(){
    m_mutexEnableCards.lock();
    if(view_as_skill == NULL)
        return;

    QSet<CardItem *> selected_set = pendings.toSet();
    unselectAll();

    foreach(CardItem *item, m_handCards)
        item->setEnabled(false);

    pendings.clear();

    foreach(CardItem *item, m_handCards){
        if(view_as_skill->viewFilter(pendings, item) && !selected_set.contains(item)){
            selectCard(item, false);
            pendings << item;

            item->setEnabled(true);
        }
    }

    if(pending_card && pending_card->isVirtualCard() && pending_card->parent() == NULL)
        delete pending_card;
    pending_card = view_as_skill->viewAs(pendings);
    m_mutexEnableCards.unlock();
    emit card_selected(pending_card);
}

void Dashboard::disableAllCards(){
    m_mutexEnableCards.lock();
    foreach(CardItem *card_item, m_handCards){
        card_item->setEnabled(false);
    }
    m_mutexEnableCards.unlock();
}

void Dashboard::enableCards(){
    m_mutexEnableCards.lock();
    foreach(CardItem *card_item, m_handCards){
        card_item->setEnabled(card_item->getFilteredCard()->isAvailable(Self));
    }
    m_mutexEnableCards.unlock();
}

void Dashboard::enableAllCards(){
    m_mutexEnableCards.lock();
    foreach(CardItem *card_item, m_handCards)
        card_item->setEnabled(true);
    m_mutexEnableCards.unlock();
}

void Dashboard::startPending(const ViewAsSkill *skill){
    m_mutexEnableCards.lock();
    view_as_skill = skill;
    pendings.clear();

    foreach(CardItem **equip_ptr, equips){
        CardItem *equip = *equip_ptr;
        if(equip)
            connect(equip, SIGNAL(mark_changed()), this, SLOT(onMarkChanged()));
    }

    updatePending();
    adjustCards(false);
    m_mutexEnableCards.unlock();
}

void Dashboard::stopPending(){
    m_mutexEnableCards.lock();
    view_as_skill = NULL;
    pending_card = NULL;
    emit card_selected(NULL);

    foreach (CardItem* item, m_handCards)
    {
        item->setEnabled(false);
    }

    foreach(CardItem **equip_ptr, equips){
        CardItem *equip = *equip_ptr;
        if(equip){
            equip->setMarkable(false);
            disconnect(equip, SIGNAL(mark_changed()));
        }
    }
    pendings.clear();
    adjustCards(false);
    m_mutexEnableCards.unlock();
}

void Dashboard::onCardItemClicked(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(!card_item)
        return;

    if(view_as_skill){
        if(card_item->isSelected()){
            selectCard(card_item, false);
            pendings.removeOne(card_item);
        }else{
            selectCard(card_item, true);
            pendings << card_item;
        }

        updatePending();

    }else{
        if(card_item->isSelected()){
            unselectAll();
            emit card_selected(NULL);
        }else{
            unselectAll();
            selectCard(card_item, true);          
            selected = card_item;

            emit card_selected(selected->getFilteredCard());
        }
    }
}

void Dashboard::updatePending(){
    foreach(CardItem *c, m_handCards){
        if(!c->isSelected()||pendings.isEmpty()){
            c->setEnabled(view_as_skill->viewFilter(pendings, c));
        }
    }

    foreach(CardItem **equip_ptr, equips){
        CardItem *equip = *equip_ptr;
        if(equip && !equip->isMarked())
            equip->setMarkable(view_as_skill->viewFilter(pendings, equip));
    }

    const Card *new_pending_card = view_as_skill->viewAs(pendings);
    if(pending_card != new_pending_card){
        pending_card = new_pending_card;
        emit card_selected(pending_card);
    }
}


void Dashboard::onCardItemThrown(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item){
        if(!view_as_skill)
            selected = card_item;
        emit card_to_use();
    }
}

void Dashboard::onCardItemHover()
{
    QGraphicsItem *card_item = qobject_cast<QGraphicsItem *>(sender());
    if(!card_item)return;

    animations->emphasize(card_item);
}

void Dashboard::onCardItemLeaveHover()
{
    QGraphicsItem *card_item = qobject_cast<QGraphicsItem *>(sender());
    if(!card_item)return;

    animations->effectOut(card_item);
}

void Dashboard::onMarkChanged(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());

    Q_ASSERT(card_item->isEquipped());

    if(card_item){
        if(card_item->isMarked()){
            if(!pendings.contains(card_item))
                pendings.append(card_item);
        }else
            pendings.removeOne(card_item);

        updatePending();
    }
}

const ViewAsSkill *Dashboard::currentSkill() const{
    return view_as_skill;
}

const Card *Dashboard::pendingCard() const{
    return pending_card;
}

int Dashboard::getRightPosition()
{
    return left_pixmap.width() + middle->rect().width();
}

int Dashboard::getMidPosition()
{
    return left_pixmap.width();
}
