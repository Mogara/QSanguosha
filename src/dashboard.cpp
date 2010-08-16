#include "dashboard.h"
#include "engine.h"
#include "settings.h"
#include "client.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>

Dashboard::Dashboard()
    :Pixmap(":/dashboard.png"),  selected(NULL), player(NULL), avatar(NULL), use_skill(false),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL), view_as_skill(NULL)
{
    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/magatamas/%1.png").arg(i+1));
    }

    sort_combobox = new QComboBox;
    sort_combobox->addItem(tr("No sort"));
    sort_combobox->addItem(tr("Sort by suit"));
    sort_combobox->addItem(tr("Sort by type"));
    sort_combobox->addItem(tr("Sort by availability"));
    QGraphicsProxyWidget *sort_widget = new QGraphicsProxyWidget;
    sort_widget->setWidget(sort_combobox);
    connect(sort_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(sortCards()));

    button_layout  = new QGraphicsLinearLayout(Qt::Horizontal);
    button_layout->addItem(sort_widget);

    QGraphicsWidget *form = new QGraphicsWidget(this);
    form->setLayout(button_layout);
    form->setPos(0, 20);

    avatar = new Pixmap("");
    avatar->setPos(837, 35);    
    avatar->setParentItem(this);

    kingdom = new QGraphicsPixmapItem(this);
    kingdom->setPos(avatar->pos());
}

void Dashboard::addCardItem(CardItem *card_item){
    card_item->setParentItem(this);
    card_items << card_item;

    connect(card_item, SIGNAL(card_selected(CardItem*)), this, SLOT(setSelectedItem(CardItem*)));
    connect(card_item, SIGNAL(pending(CardItem*,bool)), this, SLOT(doPending(CardItem*,bool)));

    sortCards();
}

void Dashboard::setPlayer(const Player *player){
    this->player = player;
    updateAvatar();
}

void Dashboard::updateAvatar(){
    const General *general = player->getAvatarGeneral();
    avatar->changePixmap(general->getPixmapPath("big"));
    kingdom->setPixmap(QPixmap(general->getKingdomPath()));

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
    if(selected)
        return selected->getCard();
    else
        return NULL;
}

void Dashboard::unselectAll(){
    setSelectedItem(NULL);
}

void Dashboard::sort(int order){
    sort_combobox->setCurrentIndex(order);
}

void Dashboard::installDelayedTrick(CardItem *trick){
    judging_area.push(trick);

    trick->setHomePos(mapToScene(QPointF(34, 37)));
    trick->goBack(true);

    update();
}

void Dashboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    // draw player's name
    painter->setPen(Qt::white);
    painter->drawText(QRectF(847, 15, 121, 17), Config.UserName, QTextOption(Qt::AlignHCenter));

    if(player){
        // draw player's hp
        int hp = player->getHp();
        QPixmap *magatama;
        if(player->isWounded())
            magatama = &magatamas[hp-1];
        else
            magatama = &magatamas[4]; // the green magatama which denote full blood state
        int i;
        for(i=0; i<hp; i++)
            painter->drawPixmap(985, 24 + i*(magatama->height()+4), *magatama);

        // draw player's equip area
        painter->setFont(Config.TinyFont);

        drawEquip(painter, weapon, 0);
        drawEquip(painter, armor, 1);
        drawEquip(painter, defensive_horse, 2);
        drawEquip(painter, offensive_horse, 3);

        // draw player's judging area
        for(i=0; i<judging_area.count(); i++){
            CardItem *trick = judging_area.at(i);
            drawDelayedTrick(painter, trick, i);
        }
    }
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

    if(to_select){
        to_select->mark(!to_select->isMarked());
        update();
    }
}

void Dashboard::drawEquip(QPainter *painter, CardItem *equip, int order){
    if(!equip)
        return;

    QRect suit_rect(11, 83 + order*32, 20, 20);
    painter->drawPixmap(suit_rect, equip->getSuitPixmap());
    const Card *card = equip->getCard();
    painter->drawText(32, 83+20 + order*32, card->getNumberString());
    painter->drawText(58, 83+20 + order*32, Sanguosha->translate(card->objectName()));

    painter->setPen(Qt::white);
    if(equip->isMarked()){
        painter->drawRect(11, 78 + order * 34, 157, 30);
    }
}

void Dashboard::drawDelayedTrick(QPainter *painter, CardItem *trick, int order){
    painter->drawPixmap(178 + order * 52, 5, trick->getIconPixmap());
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

    const Card *card = equip->getCard();
    QString subtype = card->getSubtype();
    CardItem *uninstall = NULL;
    if(subtype == "weapon"){
        uninstall = weapon;
        weapon = equip;
    }else if(subtype == "armor"){
        uninstall = armor;
        armor = equip;
    }else if(subtype == "defensive_horse"){
        uninstall = defensive_horse;
        defensive_horse = equip;
    }else if(subtype == "offensive_horse"){
        uninstall = offensive_horse;
        offensive_horse = equip;
    }

    update();
}

CardItem *Dashboard::takeCardItem(int card_id, Player::Place place){
    CardItem *card_item = NULL;

    if(place == Player::Hand){
        int i;

        for(i=0; i<card_items.length(); i++){
            CardItem *item = card_items.at(i);
            if(item->getCard()->getID() == card_id){
                if(item == selected)
                    selected = NULL;
                card_items.removeAt(i);
                adjustCards();

                card_item = item;
                break;
            }
        }
    }else if(place == Player::Equip){
        if(weapon && weapon->getCard()->getID() == card_id){
            card_item = weapon;
            weapon = NULL;
        }

        if(armor && armor->getCard()->getID() == card_id){
            card_item = armor;
            armor = NULL;
        }

        if(defensive_horse && defensive_horse->getCard()->getID() == card_id){
            card_item = defensive_horse;
            defensive_horse = NULL;
        }

        if(offensive_horse && offensive_horse->getCard()->getID() == card_id){
            card_item = offensive_horse;
            offensive_horse = NULL;
        }
    }

    if(card_item)
        return card_item;

    qFatal("No such card %d in Dashboard", card_id);
    return NULL;
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
        return x;
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

void Dashboard::updateEnablity(CardItem *card_item){
    if(card_item){
        card_item->setMarkable(view_as_skill->viewFilter(pendings, card_item));
    }
}

void Dashboard::setSelectedItem(CardItem *card_item){
    if(!view_as_skill && selected != card_item){
        if(selected)
            selected->unselect();
        selected = card_item;

        if(card_item)
            emit card_selected(card_item->getCard());
        else
            emit card_selected(NULL);
    }
}

void Dashboard::enableCards(){
    foreach(CardItem *card_item, card_items)
        card_item->setEnabled(card_item->getCard()->isAvailable());
}

void Dashboard::startPending(const ViewAsSkill *skill){
    if(view_as_skill){
        stopPending();
        view_as_skill = NULL;
    }

    view_as_skill = skill;

    foreach(CardItem *card_item, card_items){
        card_item->setEnabled(skill->viewFilter(pendings, card_item));
    }

    pending_card = skill->viewAs(pendings);
}

void Dashboard::stopPending(){
    view_as_skill = NULL;
    pending_card = NULL;

    card_items.append(pendings);
    pendings.clear();
    adjustCards();
}

void Dashboard::doPending(CardItem *card_item, bool add_to_pendings){
    if(!view_as_skill){
        if(add_to_pendings){
            selected = card_item;
            emit card_to_use();
        }
        return;
    }

    if(add_to_pendings && !pendings.contains(card_item)){
        pendings.append(card_item);
        card_items.removeOne(card_item);
        adjustCards();

        emit card_selected(pending_card);
    }else if(!add_to_pendings && !card_items.contains(card_item)){
        card_items.append(card_item);
        pendings.removeOne(card_item);
        sortCards();
    }

    pending_card = view_as_skill->viewAs(pendings);

    foreach(CardItem *card_item, card_items){
        card_item->setEnabled(view_as_skill->viewFilter(pendings, card_item));
    }

    updateEnablity(weapon);
    updateEnablity(armor);
    updateEnablity(defensive_horse);
    updateEnablity(offensive_horse);
}

const ViewAsSkill *Dashboard::currentSkill() const{
    return view_as_skill;
}

const Card *Dashboard::pendingCard() const{
    return pending_card;
}

void Dashboard::enableCards(const QString &pattern){
    // enable card with pattern
}

void Dashboard::addDynamicButton(QPushButton *button){
    QGraphicsScene *the_scene = scene();
    if(the_scene)
        button_layout->addItem(the_scene->addWidget(button));
}
