#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"

#include <QGraphicsObject>
#include <QPixmap>

class Photo : public Pixmap
{
public:
    explicit Photo();
    void loadAvatar(const QString &filename);

    static void init();
    static void quit();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

private:
    QPixmap avatar;
    QPixmap avatar_frame;
};

#endif // PHOTOBACK_H
