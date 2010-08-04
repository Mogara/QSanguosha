#include "photo.h"
#include "clientplayer.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"

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
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL)
{
    setAcceptHoverEvents(true);

    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/magatamas/%1.png").arg(i+1));
        magatamas[i] = magatamas[i].scaled(20,20);
    }

    role_combobox = new QComboBox;
    role_combobox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    role_combobox->addItem(tr("unknown"));    

    QGraphicsProxyWidget *widget = new QGraphicsProxyWidget(this);
    widget->setWidget(role_combobox);
    widget->setPos(pixmap.width()/2, pixmap.height()-10);

    order_item = new QGraphicsPixmapItem(QPixmap(QString(":/number/%1.png").arg(order+1)),this);
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
        connect(player, SIGNAL(state_changed(QString)), this, SLOT(updateStateStr(QString)));
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));

    }

    updateAvatar();
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
        if(weapon && weapon->getCard()->getID() == card_id){
            card_item = weapon;
            weapon = NULL;
        }else if(armor && armor->getCard()->getID() == card_id){            
            card_item = armor;
            armor = NULL;
        }else if(defensive_horse && defensive_horse->getCard()->getID() == card_id){
            card_item = defensive_horse;
            defensive_horse = NULL;
        }else if(offensive_horse && offensive_horse->getCard()->getID() == card_id){
            card_item = offensive_horse;
            offensive_horse = NULL;
        }

        if(card_item)
            card_item->setOpacity(1.0);
    }

    return card_item;
}

void Photo::installEquip(CardItem *equip){
    QString subtype = equip->getCard()->getSubtype();
    if(subtype == "weapon")
        weapon = equip;
    else if(subtype == "armor")
        armor = equip;
    else if(subtype == "defensive_horse")
        defensive_horse = equip;
    else if(subtype == "offensive_horse")
        offensive_horse = equip;

    equip->setHomePos(pos());
    equip->goBack(true);

    update();
}

void Photo::installDelayedTrick(CardItem *trick){
    judging_area.push(trick);

    trick->setHomePos(pos());
    trick->goBack(true);

    update();
}

void Photo::addCardItem(CardItem *card_item){
    card_item->setHomePos(pos());
    card_item->goBack(true);

    update();
}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);
    if(player){
        painter->drawPixmap(1, 13, avatar);
        painter->drawPixmap(0, 10, avatar_frame);
        painter->drawPixmap(0,  0, kingdom);

        painter->setPen(Qt::white);
        painter->drawText(QRectF(0,0,132,19), player->objectName(), QTextOption(Qt::AlignHCenter));

        // draw magatamas, similar with dashboard, but magatama is smaller
        int hp = player->getHp();
        if(hp > 0){
            QPixmap *magatama;
            if(player->isWounded())
                magatama = &magatamas[hp-1];
            else
                magatama = &magatamas[4]; // the green magatama which denote full blood state
            int i;
            for(i=0; i<hp; i++)
                painter->drawPixmap(34 + i*(magatama->width()+2), 78, *magatama);
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

        int i;
        for(i=0; i<judging_area.count(); i++){
            QRect rect(i * 25, 171, 20, 20);
            CardItem *trick = judging_area.at(i);
            painter->drawPixmap(rect, trick->getIconPixmap());
        }
    }
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
