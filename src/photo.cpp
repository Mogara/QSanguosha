#include "photo.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"

#include <QPainter>
#include <QMimeData>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMessageBox>

Photo::Photo()
    :Pixmap(":/images/photo-back.png"),
    player(NULL),
    avatar_frame(":/images/avatar-frame.png"),
    handcard(":/images/handcard.png"),
    handcard_num(0)
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsSelectable);

    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/images/magatamas/%1.png").arg(i+1));
        magatamas[i] = magatamas[i].scaled(20,20);
    }
}

void Photo::setPlayer(const Player *player)
{
    this->player = player;

    if(player){
        connect(player, SIGNAL(state_changed(QString)), this, SLOT(updateStateStr(QString)));
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));
        connect(player, SIGNAL(handcard_num_changed(int)), this, SLOT(changeHandCardNum(int)));
    }

    updateAvatar();
}

void Photo::updateAvatar(){
    if(player){
        const General *general = player->getAvatarGeneral();
        avatar.load(general->getPixmapPath("small"));
        avatar = avatar.scaled(QSize(128,58));
        kingdom.load(general->getKingdomPath());
    }else{
        avatar.detach();
        kingdom.detach();
    }

    update();
}

void Photo::changeHandCardNum(int num){
    handcard_num = num;
}

void Photo::updateStateStr(const QString &new_state){
    state_str = Sanguosha->translate(new_state);
}

const Player *Photo::getPlayer() const{
    return player;
}

void Photo::speak(const QString &content)
{

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

        if(handcard_num != 0){
            painter->drawPixmap(0, 72, handcard);
            painter->drawText(8, 95, QString::number(handcard_num));
        }

        if(!state_str.isEmpty()){
            painter->drawText(100, 100, state_str);
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


