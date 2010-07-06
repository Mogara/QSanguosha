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
}

void Photo::setPlayer(const Player *player)
{
    this->player = player;

    if(player)
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));

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

        if(handcard_num != 0){
            painter->drawPixmap(0, 72, handcard);
            painter->drawText(8, 95, QString::number(handcard_num));
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


