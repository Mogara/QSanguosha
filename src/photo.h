#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"
#include "player.h"

#include <QGraphicsObject>
#include <QPixmap>

class Photo : public Pixmap
{
public:
    explicit Photo();
    void setPlayer(const Player *player);
    bool isOccupied() const;
    void speak(const QString &content);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

private:
    const Player *player;
    QPixmap avatar;
    QPixmap avatar_frame;
    QPixmap kingdom;
};

#endif // PHOTOBACK_H
