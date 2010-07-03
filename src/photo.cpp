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
    avatar_frame(":/images/avatar-frame.png")
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsSelectable);
}

void Photo::setPlayer(const Player *player)
{
    this->player = player;
    const General *general = player->getGeneral();    
    if(general == NULL){
        QString general_name = player->property("avatar").toString();
        if(general_name.isEmpty())
            return;
        general = Sanguosha->getGeneral(general_name);
    }
    avatar.load(general->getPixmapPath("small"));
    avatar = avatar.scaled(QSize(128,58));
    kingdom.load(general->getKingdomPath());
}

void Photo::speak(const QString &content)
{

}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);
    if(!avatar.isNull()){
        painter->drawPixmap(1, 13, avatar);
        painter->drawPixmap(0, 10, avatar_frame);
        painter->drawPixmap(0,  0, kingdom);
    }
}

void Photo::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsObject *obj = static_cast<QGraphicsObject*>(scene()->focusItem());
    CardItem *card_item = qobject_cast<CardItem*>(obj);
    if(card_item && card_item->isUnderMouse()){
        QMessageBox::information(NULL, "", card_item->getCard()->objectName());
    }
}


