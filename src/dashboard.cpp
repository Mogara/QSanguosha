#include "dashboard.h"
#include "engine.h"
#include "settings.h"
#include "client.h"
#include "standard.h"
#include "magatamawidget.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>

Dashboard::Dashboard()
    :left_pixmap(":/dashboard-equip.png"), right_pixmap(":/dashboard-avatar.png"),
    selected(NULL), player(NULL), avatar(NULL),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL),
    view_as_skill(NULL), filter(NULL)
{
    createLeft();
    createMiddle();
    createRight();

    int left_width = left_pixmap.width();
    int middle_width = middle->rect().width();
    int right_width = right->rect().width();
    min_width = left_width + middle_width + right_width;

    setMiddleWidth(middle_width);

    sort_type = 0;
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

void Dashboard::createMiddle(){
    middle = new QGraphicsRectItem(this);

    QPixmap middle_pixmap(":/dashboard-hand.png");
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

    kingdom = new QGraphicsPixmapItem(right);
    kingdom->setPos(91, 54);

    chain_icon = new Pixmap(":/chain.png");
    chain_icon->setParentItem(right);
    chain_icon->setPos(small_avatar->pos());
    chain_icon->hide();
    chain_icon->setZValue(1.0);

    back_icon = new Pixmap(":/big-back.png");
    back_icon->setParentItem(right);
    back_icon->setPos(59, 105);
    back_icon->setZValue(1.0);
    back_icon->hide();

    QGraphicsPixmapItem *handcard_pixmap = new QGraphicsPixmapItem(right);
    handcard_pixmap->setPixmap(QPixmap(":/handcard.png"));
    handcard_pixmap->setPos(25, 127);

    handcard_num = new QGraphicsSimpleTextItem(handcard_pixmap);
    handcard_num->setFont(Config.TinyFont);
    handcard_num->setBrush(Qt::white);

    handcard_pixmap->hide();
}

void Dashboard::setFilter(const FilterSkill *filter){
    this->filter = filter;
}

void Dashboard::setTrust(bool trust){
    trusting_item->setVisible(trust);
    trusting_text->setVisible(trust);
}

void Dashboard::addCardItem(CardItem *card_item){
    card_item->filter(filter);

    if(ClientInstance->getStatus() == Client::Playing)
        card_item->setEnabled(card_item->getFilteredCard()->isAvailable());
    else
        card_item->setEnabled(false);

    card_item->setPos(mapFromScene(card_item->pos()));
    card_item->setParentItem(this);
    card_item->setRotation(0.0);
    card_item->setFlags(ItemIsFocusable);
    card_items << card_item;

    connect(card_item, SIGNAL(clicked()), this, SLOT(onCardItemClicked()));
    connect(card_item, SIGNAL(thrown()), this, SLOT(onCardItemThrown()));

    sortCards(sort_type);

    handcard_num->setText(QString::number(Self->getHandcardNum()));
    handcard_num->parentItem()->show();
}

void Dashboard::setPlayer(const Player *player){
    this->player = player;

    connect(player, SIGNAL(state_changed()), this, SLOT(refresh()));
    connect(player, SIGNAL(kingdom_changed()), this, SLOT(updateAvatar()));
    connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));

    updateAvatar();
}

void Dashboard::updateAvatar(){
    const General *general = player->getAvatarGeneral();
    avatar->setToolTip(general->getSkillDescription());
    avatar->changePixmap(general->getPixmapPath("big"));

    kingdom->setPixmap(QPixmap(player->getKingdomIcon()));

    avatar->show();
    kingdom->show();

    update();
}

void Dashboard::updateSmallAvatar(){
    const General *general2 = player->getGeneral2();
    if(general2){
        small_avatar->setToolTip(general2->getSkillDescription());
        small_avatar->changePixmap(general2->getPixmapPath("tiny"));
    }

    update();
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
        selected->select(); // adjust the position

    // find all cards that match the card type
    QList<CardItem*> matches;

    foreach(CardItem *card_item, card_items){
        if(card_item->isEnabled() && card_item->getFilteredCard()->match(pattern))
            matches << card_item;
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
            selected->unselect();
        to_select->select();
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

void Dashboard::unselectAll(){
    selected = NULL;

    foreach(CardItem *card_item, card_items){
        card_item->unselect();
        card_item->goBack();
    }
}

void Dashboard::hideAvatar(){
    avatar->hide();
    kingdom->hide();
}

void Dashboard::installDelayedTrick(CardItem *card){
    judging_area.push(card);
    const DelayedTrick *trick = DelayedTrick::CastFrom(card->getCard());
    delayed_tricks.push(QPixmap(trick->getIconPath()));

    card->setHomePos(mapToScene(QPointF(34, 37)));
    card->goBack(true);

    update();
}

QRectF Dashboard::boundingRect() const{
    qreal width = left->boundingRect().width() + middle->rect().width() + right->boundingRect().width();
    qreal height = middle->rect().height();
    return QRectF(0, 0, width, height);
}

void Dashboard::setMiddleWidth(int middle_width){
    int left_width = left_pixmap.width();
    qreal middle_height = middle->rect().height();

    middle->setRect(0, 0, middle_width, middle_height);
    middle->setX(left_width);
    right->setX(left_width + middle_width);

    trusting_item->setRect(middle->rect());
    trusting_item->setX(left_width);
    trusting_text->setPos(middle_width/2, middle_height/2);
}

void Dashboard::setWidth(int width){
    if(width == 0){
        setMiddleWidth(middle->brush().texture().width());

        prepareGeometryChange();
        adjustCards();

    }else if(width > min_width){
        qreal left_width = left->boundingRect().width();
        qreal right_width = right->boundingRect().width();
        qreal middle_width = width - left_width - right_width;

        setMiddleWidth(middle_width);

        prepareGeometryChange();
        adjustCards();
    }
}

void Dashboard::addWidget(QWidget *widget, int x, bool from_left){
    const int y = -25;
    QGraphicsProxyWidget *proxy_widget = new QGraphicsProxyWidget(this);
    proxy_widget->setWidget(widget);

    if(from_left)
        proxy_widget->setParentItem(left);
    else
        proxy_widget->setParentItem(right);

    proxy_widget->setPos(x, y);
}

QPushButton *Dashboard::addButton(const QString &label, int x, bool from_left){
    QPushButton *button = new QPushButton(label);
    button->setEnabled(false);
    button->setMaximumWidth(55);

    addWidget(button, x, from_left);

    return button;
}

QProgressBar *Dashboard::addProgressBar(){
    QProgressBar *progress_bar = new QProgressBar;
    progress_bar->setOrientation(Qt::Vertical);
    progress_bar->setMinimum(0);
    progress_bar->setMaximum(100);
    progress_bar->setFixedSize(12, 124);

    QGraphicsProxyWidget *widget = new QGraphicsProxyWidget(right);
    widget->setWidget(progress_bar);
    widget->setParentItem(right);
    widget->setPos(3, 39);

    progress_bar->hide();

    return progress_bar;
}

void Dashboard::drawHp(QPainter *painter) const{
    int hp = Self->getHp();
    int max_hp = Self->getMaxHP();
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

void Dashboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    // draw the left side and right side
    painter->drawPixmap(left->pos(), left_pixmap);
    painter->drawPixmap(right->pos(), right_pixmap);

    // draw player's name
    painter->setPen(Qt::white);
    QRectF name_rect(24 + right->x(), 42 + right->y(), 90, 12);
    painter->drawText(name_rect, Config.UserName, QTextOption(Qt::AlignHCenter));

    if(!player)
        return;

    if(!player->isAlive()){
        if(death_pixmap.isNull())
            death_pixmap.load(QString(":/death/%1.png").arg(player->getRole()));

        painter->drawPixmap(397, 82, death_pixmap);
        return;
    }

    // draw player's equip area
    painter->setPen(Qt::white);

    drawEquip(painter, weapon, 0);
    drawEquip(painter, armor, 1);
    drawEquip(painter, defensive_horse, 2);
    drawEquip(painter, offensive_horse, 3);

    // draw player's judging area
    int i;
    for(i=0; i<delayed_tricks.count(); i++){
        QPoint pos(3 + i * 27, 10);
        painter->drawPixmap(pos, delayed_tricks.at(i));
    }

    drawHp(painter);

    chain_icon->setVisible(player->isChained());
    back_icon->setVisible(!player->faceUp());
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

    static const int width = 117;
    static const int height = 25;
    static const int start_x = 8;
    static const int start_y = 40;

    int y = start_y + order * 32;

    // draw the suit of equip
    QRect suit_rect(10, y + 3, 15, 15);
    painter->drawPixmap(suit_rect, equip->getSuitPixmap());

    // draw the name of equip
    const EquipCard *card = qobject_cast<const EquipCard *>(equip->getCard());
    QString text = QString("%1 %2").arg(card->getNumberString()).arg(card->label());
    painter->drawText(28, y + 20, text);

    painter->setPen(Qt::white);
    if(equip->isMarked()){
        painter->drawRect(start_x, y , width, height);
    }
}

void Dashboard::adjustCards(){
    adjustCards(card_items, CardItem::NormalY);

    if(view_as_skill)
        adjustCards(pendings, CardItem::PendingY);
}

void Dashboard::adjustCards(const QList<CardItem *> &list, int y){
    if(list.isEmpty())
        return;

    int max_width = middle->rect().width();
    int start_x = left->boundingRect().width();

    if(list.length() == 1){
        list.first()->setHomePos(QPointF(start_x, y));
        list.first()->goBack();
        return;
    }

    int n = list.size();
    int card_width = list.first()->boundingRect().width();
    int total_width = qMin(card_width * n, max_width);
    int card_skip = (total_width - n * card_width ) / (n-1) + card_width;

    int i;
    for(i=0; i<n; i++){
        list[i]->setZValue(0.0001 * i);
        QPointF home_pos(start_x + i * card_skip, y);
        list[i]->setHomePos(home_pos);
        list[i]->goBack();
    }
}

void Dashboard::installEquip(CardItem *equip){
    equip->setHomePos(mapToScene(QPointF(34, 37)));
    equip->goBack(true);

    int index = -1;
    const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard());
    switch(equip_card->location()){
    case EquipCard::WeaponLocation: weapon = equip; index = 0; break;
    case EquipCard::ArmorLocation: armor = equip; index = 1; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = equip; index = 2; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = equip; index = 3; break;
    }

    if(index >= 0)
        equip_rects[index]->setToolTip(equip_card->getDescription());

    update();
}

CardItem *Dashboard::takeCardItem(int card_id, Player::Place place){
    CardItem *card_item = NULL;

    if(place == Player::Hand){
        card_item = CardItem::FindItem(card_items, card_id);
        if(card_item == selected)
            selected = NULL;
        card_items.removeOne(card_item);
        adjustCards();

        if(Self->isKongcheng())
            handcard_num->parentItem()->hide();
        else{
            handcard_num->setText(QString::number(Self->getHandcardNum()));
        }
    }else if(place == Player::Equip){
        foreach(CardItem **equip_ptr, equips){
            CardItem *equip = *equip_ptr;
            if(equip && equip->getCard()->getId() == card_id){
                card_item = equip;
                *equip_ptr = NULL;

                int index = equips.indexOf(equip_ptr);
                equip_rects[index]->setToolTip(QString());
                break;
            }
        }

    }else if(place == Player::Judging){
        int i;
        for(i=0; i<judging_area.count(); i++){
            CardItem *item = judging_area.at(i);
            if(item->getCard()->getId() == card_id){
                card_item = item;                

                judging_area.remove(i);
                delayed_tricks.remove(i);
                break;
            }
        }
    }

    if(card_item){
        card_item->disconnect(this);
        update();
        return card_item;
    }else{
        return NULL;
    }
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

void Dashboard::sortCards(int sort_type){
    this->sort_type = sort_type;

    switch(sort_type){
    case 0: break;
    case 1: qSort(card_items.begin(), card_items.end(), CompareBySuitNumber); break;
    case 2: qSort(card_items.begin(), card_items.end(), CompareByType); break;
    case 3: qSort(card_items.begin(), card_items.end(), CompareByAvailability); break;
    }

    adjustCards();
}

void Dashboard::disableAllCards(){
    foreach(CardItem *card_item, card_items){
        card_item->setEnabled(false);
    }
}

void Dashboard::enableCards(){    
    foreach(CardItem *card_item, card_items)
        card_item->setEnabled(card_item->getFilteredCard()->isAvailable());
}

void Dashboard::enableCards(const QString &pattern){
    static QRegExp id_rx("\\d+");
    static QRegExp suit_rx("\\.[SCHD]");

    if(pattern.contains("+")){
        QStringList subpatterns = pattern.split("+");

        foreach(CardItem *card_item, card_items){
            const Card *card = card_item->getFilteredCard();
            bool enabled = false;
            foreach(QString subpattern, subpatterns){
                if(card->match(subpattern)){
                    enabled = true;
                    break;
                }
            }

            card_item->setEnabled(enabled);
        }
    }else if(id_rx.exactMatch(pattern)){
        int id = pattern.toInt();
        foreach(CardItem *card_item, card_items)
            card_item->setEnabled(card_item->getCard()->getId() == id);
    }else if(pattern == "."){
        foreach(CardItem *card_item, card_items)
            card_item->setEnabled(true);
    }else if(suit_rx.exactMatch(pattern)){
        QChar end = pattern.at(1).toLower();
        foreach(CardItem *card_item, card_items){
            bool enabled = card_item->getFilteredCard()->getSuitString().startsWith(end);
            card_item->setEnabled(enabled);
        }
    }else{
        foreach(CardItem *card_item, card_items){
            card_item->setEnabled(card_item->getFilteredCard()->match(pattern));
        }
    }
}

void Dashboard::startPending(const ViewAsSkill *skill){
    view_as_skill = skill;
    pendings.clear();

    foreach(CardItem **equip_ptr, equips){
        CardItem *equip = *equip_ptr;
        if(equip)
            connect(equip, SIGNAL(mark_changed()), this, SLOT(onMarkChanged()));
    }

    updatePending();
}

void Dashboard::stopPending(){
    view_as_skill = NULL;
    pending_card = NULL;
    emit card_selected(NULL);

    foreach(CardItem **equip_ptr, equips){
        CardItem *equip = *equip_ptr;
        if(equip){
            equip->setMarkable(false);
            disconnect(equip, SIGNAL(mark_changed()));
        }

    }

    pendings.clear();
    adjustCards();
}

void Dashboard::onCardItemClicked(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(!card_item)
        return;

    if(view_as_skill){
        if(card_item->isPending()){
            card_item->unselect();
            pendings.removeOne(card_item);
        }else{
            card_item->select();
            pendings << card_item;
        }

        updatePending();

    }else{
        if(card_item->isPending()){
            unselectAll();
        }else{
            unselectAll();
            card_item->select();
            card_item->goBack();
            selected = card_item;

            emit card_selected(selected->getFilteredCard());
        }
    }
}

void Dashboard::updatePending(){
    foreach(CardItem *c, card_items){
        if(!c->isPending())
            c->setEnabled(view_as_skill->viewFilter(pendings, c));
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
        selected = card_item;
        emit card_to_use();
    }
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
