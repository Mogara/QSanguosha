#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"

#include <QGraphicsScene>

class RoomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    RoomScene(int player_count = 8);
    void updatePhotos();

public slots:
    void showBust(const QString &name);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    QList<Photo*> photos;
    Dashboard *dashboard;
    Pixmap *pile;
    QGraphicsSimpleTextItem *skill_label;
    Pixmap *avatar;
    Pixmap *bust;
};

#endif // ROOMSCENE_H
