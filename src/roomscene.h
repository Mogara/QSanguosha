#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"

#include <QGraphicsScene>
#include <MediaObject>
#include <QQueue>
#include <QMainWindow>

class RoomScene : public QGraphicsScene
{
    Q_OBJECT
public:
    RoomScene(Client *client, int player_count, QMainWindow *main_window);

public slots:
    void showBust(const QString &name);
    void addPlayer(ClientPlayer *player);
    void removePlayer(const QString &player_name);
    void drawCards(const QList<Card *> &cards);
    void drawNCards(ClientPlayer *player, int n);
    void chooseLord(const QList<const General *> &lords);
    void chooseGeneral(const General *lord, const QList<const General*> &generals);
    void changePrompt(const QString &prompt_str);
    void updatePhotos(const QList<const ClientPlayer*> &seats);
    void viewDiscards();
    void hideDiscards();
    void setActivity(bool active);
    void moveCard(const QString &src, const QString &dest, int card_id);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    Client *client;
    QList<Photo*> photos;
    QMap<QString, Photo*> name2photo;
    Dashboard *dashboard;
    Pixmap *pile;
    QGraphicsSimpleTextItem *prompt_label;
    Pixmap *avatar;
    Pixmap *bust;
    QList<const Card*> discarded_list;
    QQueue<CardItem*> discarded_queue;
    Phonon::MediaObject *effect;
    QMainWindow *main_window;
    QComboBox *role_combobox;

    void startEnterAnimation();
    CardItem *takeCardItem(const QString &src, int card_id);

private slots:
    void updateSkillButtons();
    void updateRoleComboBox(const QString &new_role);
};

#endif // ROOMSCENE_H
