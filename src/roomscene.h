#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"

#include <QGraphicsScene>

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
    void changePrompt(const QString &prompt_str = QString());
    void setActivity(bool active);
    void moveCard(const QString &src, const QString &dest, int card_id);

    void updatePhotos(const QList<const ClientPlayer*> &seats);
    void viewDiscards();
    void hideDiscards();
    void enableTargets(const Card *card);
    void useSelectedCard();

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
    Pixmap *avatar;
    Pixmap *bust;
    QList<const Card*> discarded_list;
    QQueue<CardItem*> discarded_queue;
    QMainWindow *main_window;
    QComboBox *role_combobox;
    QPushButton *ok_button, *cancel_button, *discard_button;

    QList<QPushButton *> skill_buttons;

    int max_targets, min_targets;
    bool target_fixed;    
    QList<const ClientPlayer *> selected_targets, available_targets;

    void startEnterAnimation();
    CardItem *takeCardItem(const QString &src, int card_id);
    void clickSkillButton(int order);

private slots:
    void updateSkillButtons();
    void updateRoleComboBox(const QString &new_role);
    void updateSelectedTargets();
};

#endif // ROOMSCENE_H
