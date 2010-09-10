#include "photo.h"
#include "clientplayer.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"

#include <QPainter>
#include <QMimeData>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMessageBox>
#include <QGraphicsProxyWidget>

Photo::Photo(int order)
    :Pixmap(":/photo-back.png"),
    player(NULL),
    avatar_frame(":/avatar-frame.png"),
    handcard(":/handcard.png"),
    chain(":/chain.png"),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL),
    order_item(new QGraphicsPixmapItem(QPixmap(QString(":/number/%1.png").arg(order+1)),this)),
    hide_avatar(false)
{
    setAcceptHoverEvents(true);

    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/magatamas/%1.png").arg(i+1));
        magatamas[i] = magatamas[i].scaled(20,20);
    }
    magatamas[5] = magatamas[4];

    role_combobox = new QComboBox;
    role_combobox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    role_combobox->addItem(tr("unknown"));    

    QGraphicsProxyWidget *widget = new QGraphicsProxyWidget(this);
    widget->setWidget(role_combobox);
    widget->setPos(pixmap.width()/2, pixmap.height()-10);

    order_item->setVisible(false);
}

void Photo::setPlayer(const ClientPlayer *player)
{
    this->player = player;

    if(player){        
        role_combobox->addItem(QIcon(":/roles/loyalist.png"), tr("loyalist"));
        role_combobox->addItem(QIcon(":/roles/rebel.png"), tr("rebel"));
        role_combobox->addItem(QIcon(":/roles/renegade.png"), tr("renegade"));

        connect(player, SIGNAL(role_changed(QString)), this, SLOT(updateRoleCombobox(QString)));
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));        
        connect(player, SIGNAL(state_changed()), this, SLOT(refresh()));
    }

    updateAvatar();
}

void Photo::hideAvatar(){
    hide_avatar = true;

    update();
}

void Photo::updateAvatar(){
    if(player){
        const General *general = player->getAvatarGeneral();
        avatar.load(general->getPixmapPath("small"));
        avatar = avatar.scaled(QSize(128,58));
        kingdom = QPixmap(general->getKingdomPath());
    }else{
        avatar = QPixmap();
        kingdom = QPixmap();
    }

    hide_avatar = false;

    update();
}

void Photo::refresh(){
    // just simply call update() to redraw itself
    update();
}

void Photo::updateRoleCombobox(const QString &new_role){
    role_combobox->clear();
    QIcon icon(QString(":/roles/%1.png").arg(new_role));
    QString caption = Sanguosha->translate(new_role);
    role_combobox->addItem(icon, caption);
    role_combobox->setEnabled(false);
}

const ClientPlayer *Photo::getPlayer() const{
    return player;
}

void Photo::speak(const QString &content)
{

}

CardItem *Photo::takeCardItem(int card_id, Player::Place place){
    CardItem *card_item = NULL;

    if(place == Player::Hand){
        card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setPos(pos());
        card_item->shift();
    }else if(place == Player::Equip){
        if(weapon && weapon->getCard()->getId() == card_id){
            card_item = weapon;
            weapon = NULL;
        }else if(armor && armor->getCard()->getId() == card_id){            
            card_item = armor;
            armor = NULL;
        }else if(defensive_horse && defensive_horse->getCard()->getId() == card_id){
            card_item = defensive_horse;
            defensive_horse = NULL;
        }else if(offensive_horse && offensive_horse->getCard()->getId() == card_id){
            card_item = offensive_horse;
            offensive_horse = NULL;
        }

        if(card_item)
            card_item->setOpacity(1.0);
    }else if(place == Player::DelayedTrick){
        QMutableVectorIterator<CardItem *> itor(judging_area);
        while(itor.hasNext()){
            CardItem *item = itor.next();
            if(item->getCard()->getId() == card_id){
                card_item = item;

                int index = judging_area.indexOf(item);
                QGraphicsPixmapItem *pixmap_item = judging_pixmaps.at(index);
                judging_pixmaps.remove(index);
                delete pixmap_item;
                itor.remove();

                break;
            }
        }
    }

    update();
    return card_item;
}

void Photo::installEquip(CardItem *equip){
    const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard());
    switch(equip_card->location()){
    case EquipCard::WeaponLocation: weapon = equip; break;
    case EquipCard::ArmorLocation: armor = equip; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = equip; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = equip; break;
    }

    equip->setHomePos(pos());
    equip->goBack(true);

    update();
}

void Photo::installDelayedTrick(CardItem *trick){
    trick->setHomePos(pos());
    trick->goBack(true);

    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(this);
    const Card *card = trick->getCard();
    if(card->objectName() == "indulgence" || card->getSuit() == Card::Diamond){
        static QPixmap indulgence;
        if(indulgence.isNull())
            indulgence.load("standard/cards/icon/indulgence.png");
        item->setPixmap(indulgence);
    }else if(card->objectName() == "lightning"){
        static QPixmap lightning;
        if(lightning.isNull())
            lightning.load("standard/cards/icon/lightning.png");
        item->setPixmap(lightning);
    }else{
        // supply shortage
    }

    item->setPos(-25, judging_area.count() * 50);
    judging_area.push(trick);
    judging_pixmaps.push(item);
}

void Photo::addCardItem(CardItem *card_item){
    card_item->setHomePos(pos());
    card_item->goBack(true);

    update();
}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);
    if(!player || hide_avatar)
        return;

    painter->drawPixmap(1, 13, avatar);
    painter->drawPixmap(0, 10, avatar_frame);
    painter->drawPixmap(0,  0, kingdom);

    if(!player->isAlive()){
        if(death_pixmap.isNull()){
            death_pixmap.load(QString(":/death/%1.png").arg(player->getRole()));
            death_pixmap = death_pixmap.scaled(death_pixmap.size() / (1.5));
        }

        painter->drawPixmap(5, 30, death_pixmap);

        return;
    }

    painter->setPen(Qt::white);
    painter->drawText(QRectF(0,0,132,19), player->objectName(), QTextOption(Qt::AlignHCenter));

    // draw magatamas, similar with dashboard, but magatama is smaller
    int hp = player->getHp();
    if(hp > 0){
        hp = qMin(hp, 6);

        QPixmap *magatama;
        if(player->isWounded())
            magatama = &magatamas[hp-1];
        else
            magatama = &magatamas[4]; // the green magatama which denote full blood state

        int i;
        for(i=0; i<hp; i++)
            painter->drawPixmap(28 + i*magatama->width(), 78, *magatama);

        if(player->hasSkill("benghuai")){
            QString hp_str = QString("%1/%2").arg(player->getHp()).arg(player->getMaxHP());
            painter->drawText(28, 78, hp_str);
        }
    }

    int n = player->getHandcardNum();
    if(n > 0){
        painter->drawPixmap(0, 72, handcard);
        painter->drawText(8, 95, QString::number(n));
    }

    if(!player->getState().isEmpty()){
        painter->drawText(100, 100, Sanguosha->translate(player->getState()));
    }

    if(player->getPhase() != Player::NotActive){
        painter->drawRect(boundingRect());
        painter->drawText(0, pixmap.height(), Sanguosha->translate(player->getPhaseString()));
    }

    drawEquip(painter, weapon, 0);
    drawEquip(painter, armor, 1);
    drawEquip(painter, defensive_horse, 2);
    drawEquip(painter, offensive_horse, 3);

    // draw iron chain
    painter->drawPixmap(0, 0, chain);
}

void Photo::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsObject *obj = static_cast<QGraphicsObject*>(scene()->focusItem());
    CardItem *card_item = qobject_cast<CardItem*>(obj);
    if(card_item && card_item->isUnderMouse()){
        QMessageBox::information(NULL, "", card_item->getCard()->objectName());
    }
}

void Photo::drawEquip(QPainter *painter, CardItem *equip, int order){
    if(!equip)
        return;

    QRect suit_rect(2, 104 + order * 17, 13, 13);
    painter->drawPixmap(suit_rect, equip->getSuitPixmap());

    const Card *card = equip->getCard();
    painter->setPen(Qt::black);
    QFont bold_font;
    bold_font.setBold(true);
    painter->setFont(bold_font);
    painter->drawText(20, 115 + order * 17, card->getNumberString());
    painter->drawText(35, 115 + order * 17, Sanguosha->translate(card->objectName()));
}

QVariant Photo::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == ItemFlagsHaveChanged)
        order_item->setVisible(flags() & ItemIsSelectable);

    return Pixmap::itemChange(change, value);
}
