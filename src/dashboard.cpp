#include "dashboard.h"
#include "engine.h"
#include "settings.h"
#include "client.h"
#include "standard.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>

Dashboard::Dashboard()
    :Pixmap(":/dashboard.png"),  selected(NULL), player(NULL), avatar(NULL),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL), view_as_skill(NULL)
{
    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/magatamas/%1.png").arg(i+1));
    }
    magatamas[5] = magatamas[4];

    sort_combobox = new QComboBox;
    sort_combobox->addItem(tr("No sort"));
    sort_combobox->addItem(tr("Sort by suit"));
    sort_combobox->addItem(tr("Sort by type"));
    sort_combobox->addItem(tr("Sort by availability"));
    QGraphicsProxyWidget *sort_widget = new QGraphicsProxyWidget;
    sort_widget->setWidget(sort_combobox);
    connect(sort_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(sortCards()));

    sort_widget->setParentItem(this);
    sort_widget->setPos(0, 28);

    button_layout  = new QGraphicsLinearLayout(Qt::Horizontal);

    QGraphicsWidget *form = new QGraphicsWidget(this);
    form->setLayout(button_layout);
    form->setPos(sort_widget->pos());
    form->moveBy(sort_widget->boundingRect().width(), -10);

    avatar = new Pixmap;
    avatar->setPos(837, 35);    
    avatar->setParentItem(this);

    small_avatar = new Pixmap;
    small_avatar->setPos(912, 35);
    small_avatar->setScale(0.5);
    small_avatar->setParentItem(this);
    small_avatar->setOpacity(0.75);

    kingdom = new QGraphicsPixmapItem(this);
    kingdom->setPos(avatar->pos());

    chain_icon = new Pixmap(":/chain.png");
    chain_icon->setParentItem(this);
    chain_icon->setPos(avatar->pos());
    chain_icon->hide();
    chain_icon->setZValue(1.0);

    back_icon = new Pixmap(":/big-back.png");
    back_icon->setParentItem(this);
    back_icon->setPos(922, 104);
    back_icon->hide();
    back_icon->setZValue(1.0);

    equips << &weapon << &armor << &defensive_horse << &offensive_horse;

    QGraphicsPixmapItem *handcard_pixmap = new QGraphicsPixmapItem(this);
    handcard_pixmap->setPixmap(QPixmap(":/handcard.png"));
    handcard_pixmap->setPos(841, 146);

    handcard_num = new QGraphicsSimpleTextItem(handcard_pixmap);
    handcard_num->setFont(Config.TinyFont);
    handcard_num->setBrush(Qt::white);

    handcard_pixmap->hide();
}

void Dashboard::addCardItem(CardItem *card_item){
    if(ClientInstance->getStatus() == Client::Playing)
        card_item->setEnabled(card_item->getCard()->isAvailable());
    else
        card_item->setEnabled(false);

    card_item->setPos(mapFromScene(card_item->pos()));
    card_item->setParentItem(this);
    card_item->setRotation(0.0);
    card_item->setFlags(ItemIsFocusable);
    card_items << card_item;

    connect(card_item, SIGNAL(clicked()), this, SLOT(onCardItemClicked()));
    connect(card_item, SIGNAL(thrown()), this, SLOT(onCardItemThrown()));

    sortCards();

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

    const General *general2 = player->getGeneral2();
    if(general2){
        small_avatar->setToolTip(general2->getSkillDescription());
        small_avatar->changePixmap(general2->getPixmapPath("big"));
    }

    kingdom->setPixmap(QPixmap(player->getKingdomPath()));

    avatar->show();
    kingdom->show();

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
        if(card_item->isEnabled() && card_item->getCard()->match(pattern))
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

        emit card_selected(selected->getCard());
    }
}

const Card *Dashboard::getSelected() const{
    if(view_as_skill)
        return pending_card;
    else if(selected)
        return selected->getCard();
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

void Dashboard::sort(int order){
    sort_combobox->setCurrentIndex(order);
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

void Dashboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    // draw player's name
    painter->setPen(Qt::white);
    painter->drawText(QRectF(847, 15, 121, 17), Config.UserName, QTextOption(Qt::AlignHCenter));

    if(!player)
        return;

    if(!player->isAlive()){
        if(death_pixmap.isNull())
            death_pixmap.load(QString(":/death/%1.png").arg(player->getRole()));

        painter->drawPixmap(397, 82, death_pixmap);
        return;
    }

    // draw player's hp
    int hp = player->getHp();
    int hp_index = qMin(hp, 6);
    QPixmap *magatama;
    if(player->isWounded())
        magatama = &magatamas[hp_index - 1];
    else
        magatama = &magatamas[4]; // the green magatama which denote full blood state
    int i;

    int basic_hp = qMin(hp, 5);
    for(i=0; i<basic_hp; i++)
        painter->drawPixmap(985, 24 + i*(magatama->height()+4), *magatama);

    // draw extra hp (i.e. Dongzhuo, Shen Lubu, Shen Guanyu)
    int extra_hp = hp - basic_hp;
    for(i=0; i<extra_hp; i++)
        painter->drawPixmap(946 - i * 40, 184, *magatama);

    if(Self->hasSkill("benghuai") && Self->getMaxHP() > 0){
        QString hp_str = QString("%1/%2").arg(Self->getHp()).arg(Self->getMaxHP());
        painter->drawText(792, 180, hp_str);
    }

    // draw player's equip area
    painter->setFont(Config.TinyFont);

    drawEquip(painter, weapon, 0);
    drawEquip(painter, armor, 1);
    drawEquip(painter, defensive_horse, 2);
    drawEquip(painter, offensive_horse, 3);

    // draw player's judging area
    for(i=0; i<delayed_tricks.count(); i++){
        QPoint pos(178 + i * 52, 5);
        painter->drawPixmap(pos, delayed_tricks.at(i));
    }

    chain_icon->setVisible(player->isChained());
    back_icon->setVisible(!player->faceUp());
}

void Dashboard::mousePressEvent(QGraphicsSceneMouseEvent *event){
    static QSizeF equip_size(157, 30);
    static QRectF weapon_rect(QPointF(11, 78), equip_size);
    static QRectF armor_rect(QPointF(11, 112), equip_size);
    static QRectF defensive_horse_rect(QPointF(11, 143), equip_size);
    static QRectF offensive_horse_rect(QPointF(11, 177), equip_size);

    QPointF pos(event->pos());
    CardItem *to_select = NULL;
    if(weapon_rect.contains(pos))
        to_select = weapon;
    else if(armor_rect.contains(pos))
        to_select = armor;
    else if(defensive_horse_rect.contains(pos))
        to_select = defensive_horse;
    else if(offensive_horse_rect.contains(pos))
        to_select = offensive_horse;   

    if(to_select && to_select->isMarkable()){
        to_select->mark(!to_select->isMarked());

        update();
    }
}

void Dashboard::drawEquip(QPainter *painter, const CardItem *equip, int order){
    if(!equip)
        return;

    QRect suit_rect(11, 83 + order*32, 20, 20);
    painter->drawPixmap(suit_rect, equip->getSuitPixmap());
    const EquipCard *card = qobject_cast<const EquipCard *>(equip->getCard());
    painter->drawText(32, 83+20 + order*32, card->getNumberString());
    painter->drawText(58, 83+20 + order*32, card->label());

    painter->setPen(Qt::white);
    if(equip->isMarked()){
        painter->drawRect(11, 78 + order * 34, 157, 30);
    }
}

void Dashboard::adjustCards(){
    adjustCards(card_items, 45);

    if(view_as_skill)
        adjustCards(pendings, -200);
}

void Dashboard::adjustCards(const QList<CardItem *> &list, int y){
    if(list.isEmpty())
        return;

    int n = list.size();
    int card_width = list.first()->boundingRect().width();
    int card_skip;
    if(n > 5)
        card_skip = (530 - n * card_width)/n + card_width;
    else
        card_skip = card_width;

    int i;
    for(i=0; i<n; i++){
        list[i]->setZValue(0.1 * i);
        QPointF home_pos(180 + i*card_skip, y);
        list[i]->setHomePos(home_pos);
        list[i]->goBack();
    }
}

void Dashboard::installEquip(CardItem *equip){
    equip->setHomePos(mapToScene(QPointF(34, 37)));
    equip->goBack(true);

    const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard());
    switch(equip_card->location()){
    case EquipCard::WeaponLocation: weapon = equip; break;
    case EquipCard::ArmorLocation: armor = equip; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = equip; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = equip; break;
    }

    update();
}

CardItem *Dashboard::takeCardItem(int card_id, Player::Place place){
    CardItem *card_item = NULL;

    if(place == Player::Hand){
        int i;

        for(i=0; i<card_items.length(); i++){
            CardItem *item = card_items.at(i);
            if(item->getCard()->getId() == card_id){
                if(item == selected)
                    selected = NULL;
                card_items.removeAt(i);
                adjustCards();

                card_item = item;
                break;
            }
        }

        if(Self->isKongcheng())
            handcard_num->parentItem()->hide();
        else{
            handcard_num->setText(QString::number(Self->getHandcardNum()));
        }
    }else if(place == Player::Equip){
        if(weapon && weapon->getCard()->getId() == card_id){
            card_item = weapon;
            weapon = NULL;
        }

        if(armor && armor->getCard()->getId() == card_id){
            card_item = armor;
            armor = NULL;
        }

        if(defensive_horse && defensive_horse->getCard()->getId() == card_id){
            card_item = defensive_horse;
            defensive_horse = NULL;
        }

        if(offensive_horse && offensive_horse->getCard()->getId() == card_id){
            card_item = offensive_horse;
            offensive_horse = NULL;
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

void Dashboard::sortCards(){
    int sort_type = sort_combobox->currentIndex();
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
        card_item->setEnabled(card_item->getCard()->isAvailable());
}

void Dashboard::enableCards(const QString &pattern){
    static QRegExp id_rx("\\d+");

    if(pattern.contains("+")){
        QStringList subpatterns = pattern.split("+");

        foreach(CardItem *card_item, card_items){
            const Card *card = card_item->getCard();
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
    }else{
        foreach(CardItem *card_item, card_items)
            card_item->setEnabled(card_item->getCard()->match(pattern));
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

            emit card_selected(selected->getCard());
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

void Dashboard::addSkillButton(QPushButton *button){
    QGraphicsScene *the_scene = scene();
    if(the_scene){
        QGraphicsWidget *widget = the_scene->addWidget(button);
        button_layout->addItem(widget);

        button2widget.insert(button, widget);
    }
}

void Dashboard::removeSkillButton(QPushButton *button){
    QGraphicsWidget *widget = button2widget.value(button, NULL);
    if(widget){
        button_layout->removeItem(widget);
        button2widget.remove(button);
    }
}
