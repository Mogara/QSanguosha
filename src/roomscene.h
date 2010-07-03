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
    RoomScene(Client *client, int player_count, QMainWindow *main_window);
    void updatePhotos();

    Q_INVOKABLE void addPlayer(const QString &player_info);
    Q_INVOKABLE void removePlayer(const QString &player_name);
    Q_INVOKABLE void drawCards(const QString &cards_str);
    Q_INVOKABLE void nameDuplication(const QString &name);
    Q_INVOKABLE void focusWarn(const QString &);

public slots:
    void showBust(const QString &name);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    Client *client;
    QList<Photo*> photos;
    QMap<QString, Photo*> photo_map;
    Dashboard *dashboard;
    Pixmap *pile;
    QGraphicsSimpleTextItem *skill_label;
    Pixmap *avatar;
    Pixmap *bust;

    void startEnterAnimation();
    void createSkillButtons(QMainWindow *main_window, Player *player);
};

#endif // ROOMSCENE_H
