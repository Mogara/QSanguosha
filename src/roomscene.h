#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"

#include <QGraphicsScene>
#include <QMainWindow>

class RoomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    RoomScene(Client *client, int player_count);
    void updatePhotos();

public slots:
    void showBust(const QString &name);
    void addPlayer(Player *player);
    void removePlayer(const QString &player_name);
    void drawCards(const QList<Card *> &cards);
    void chooseLord(const QList<const General *> &lords);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    Client *client;
    QList<Photo*> photos;
    QMap<QString, Photo*> name2photo;
    Dashboard *dashboard;
    Pixmap *pile;
    QGraphicsSimpleTextItem *skill_label;
    Pixmap *avatar;
    Pixmap *bust;

    void startEnterAnimation();
    void createSkillButtons(QMainWindow *main_window, Player *player);
};

#endif // ROOMSCENE_H
