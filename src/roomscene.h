#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"

#include <QGraphicsScene>

class RoomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    RoomScene(Client *client, int player_count = 8);
    void updatePhotos();

    Q_INVOKABLE void addPlayer(const QString &player_info);
    Q_INVOKABLE void drawCards(const QString &cards_str);
    Q_INVOKABLE void nameDuplication(const QString &name);

public slots:
    void showBust(const QString &name);


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    Client *client;
    QList<Photo*> photos;
    Dashboard *dashboard;
    Pixmap *pile;
    QGraphicsSimpleTextItem *skill_label;
    Pixmap *avatar;
    Pixmap *bust;

    void startEnterAnimation();
};

#endif // ROOMSCENE_H
