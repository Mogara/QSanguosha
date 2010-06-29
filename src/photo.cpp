#include "photo.h"
#include "settings.h"
#include "carditem.h"

#include <QPainter>
#include <QMimeData>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMessageBox>

Photo::Photo()
    :Pixmap(":/images/photo-back.png"),
    avatar_frame(":/images/avatar-frame.png")
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsSelectable);
}

void Photo::loadAvatar(const QString &filename){
    avatar.load(filename);
    avatar = avatar.scaled(QSize(128,58));
}

void Photo::speak(const QString &content){

}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);
    if(!avatar.isNull()){
        painter->drawPixmap(1, 13, avatar);
        painter->drawPixmap(0, 10, avatar_frame);
    }
}

void Photo::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsObject *obj = static_cast<QGraphicsObject*>(scene()->focusItem());
    CardItem *card_item = qobject_cast<CardItem*>(obj);
    if(card_item && card_item->isUnderMouse()){
        QMessageBox::information(NULL, "", card_item->getCard()->objectName());
    }
}


