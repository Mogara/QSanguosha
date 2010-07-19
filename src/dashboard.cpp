#include "dashboard.h"
#include "engine.h"
#include "settings.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

Dashboard::Dashboard()
    :Pixmap(":/images/dashboard.png"), selected(NULL), player(NULL), avatar(NULL), use_skill(false),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL)
{
    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/images/magatamas/%1.png").arg(i+1));
    }

    sort_combobox = new QComboBox;
    sort_combobox->addItem(tr("No sort"));
    sort_combobox->addItem(tr("Sort by suit"));
    sort_combobox->addItem(tr("Sort by type"));
    sort_combobox->move(0, 32);
    QGraphicsProxyWidget *sort_widget = new QGraphicsProxyWidget(this);
    sort_widget->setWidget(sort_combobox);
    connect(sort_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(sortCards()));

    avatar = new Pixmap("");
    avatar->setPos(837, 35);
    avatar->setFlag(ItemIsSelectable);
    avatar->setParentItem(this);

    kingdom = new QGraphicsPixmapItem(this);
    kingdom->setPos(avatar->pos());

    effect = Phonon::createPlayer(Phonon::MusicCategory);
}

void Dashboard::addCardItem(CardItem *card_item){
    card_item->setParentItem(this);
    card_items << card_item;

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
    }

}

CardItem *Dashboard::getSelected() const{
    return selected;
}

void Dashboard::unselectAll(){
    if(selected){
        selected->unselect();
        selected = NULL;
    }
}

void Dashboard::sort(int order){
    sort_combobox->setCurrentIndex(order);
}

void Dashboard::installDelayedTrick(CardItem *card){
    judging_area.push(card);
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
        for(i=0; i<judging_area.count(); i++)
            ;
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
}

void Dashboard::adjustCards(){
    int n = card_items.size();
    if(n == 0)
        return;

    int card_width = card_items.front()->boundingRect().width();
    int card_skip;
    if(n > 5)
        card_skip = (530 - n * card_width)/n + card_width;
    else
        card_skip = card_width;

    int i;
    for(i=0; i<n; i++){
        card_items[i]->setZValue(0.1 * i);
        QPointF home_pos(180 + i*card_skip, 45);
        card_items[i]->setHomePos(home_pos);
        card_items[i]->goBack();
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

CardItem *Dashboard::takeCardItem(int card_id, const QString &location) {
    CardItem *card_item = NULL;

    if(location == "hand"){
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
    }else if(location == "equip"){
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

void Dashboard::sortCards(){
    int sort_type = sort_combobox->currentIndex();
    switch(sort_type){
    case 0: break;
    case 1: qSort(card_items.begin(), card_items.end(), CompareBySuitNumber); break;
    case 2: qSort(card_items.begin(), card_items.end(), CompareByType); break;
    }

    adjustCards();
}

void Dashboard::disableAllCards(){
    foreach(CardItem *card_item, card_items){
        card_item->setEnabled(false);
    }
}
