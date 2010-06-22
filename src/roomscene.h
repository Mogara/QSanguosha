#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "bottom.h"
#include "card.h"

#include <QGraphicsScene>

class RoomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    RoomScene();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    Photo *photos[7];
    Bottom *bottom;
    Pixmap *pile;
    QGraphicsSimpleTextItem *skill_label;
};

#endif // ROOMSCENE_H
