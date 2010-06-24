#include "photo.h"
#include "settings.h"
#include "card.h"

#include <QPainter>
#include <QMimeData>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMessageBox>
#include <QGraphicsColorizeEffect>

Photo::Photo()
    :Pixmap(":/images/photo-back.png"),
    avatar_frame(":/images/avatar-frame.png")
{
    setAcceptHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsSelectable);
}

void Photo::loadAvatar(const QString &filename){
    avatar.load(filename);
    avatar = avatar.scaled(QSize(128,58));
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
    Card *card = qobject_cast<Card*>(obj);
    if(card && card->isUnderMouse()){
        QMessageBox::information(NULL, "", card->objectName());
    }
}

QVariant Photo::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == QGraphicsItem::ItemSelectedChange){
        if(value.toBool()){
            QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect(this);
            effect->setColor(QColor(0xCC, 0x00, 0x00));
            setGraphicsEffect(effect);
        }else
            setGraphicsEffect(NULL);
    }

    return Pixmap::itemChange(change, value);
}

