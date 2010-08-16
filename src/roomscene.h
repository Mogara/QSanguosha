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
    RoomScene(int player_count, QMainWindow *main_window);

public slots:
    void showBust(const QString &name);

    void addPlayer(ClientPlayer *player);
    void removePlayer(const QString &player_name);
    void drawCards(const QList<const Card *> &cards);
    void drawNCards(ClientPlayer *player, int n);
    void chooseLord(const QList<const General *> &lords);
    void chooseGeneral(const General *lord, const QList<const General*> &generals);
    void changePrompt(const QString &prompt_str = QString());
    void moveCard(ClientPlayer *src, Player::Place src_place,
                  ClientPlayer *dest, Player::Place dest_place,
                  int card_id);

    void updatePhotos(const QList<const ClientPlayer*> &seats);
    void viewDiscards();
    void hideDiscards();
    void enableTargets(const Card *card);
    void useSelectedCard();
    void updateStatus(Client::Status status);

protected:
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    QList<Photo*> photos;
    QMap<QString, Photo*> name2photo;
    Dashboard *dashboard;
    Pixmap *pile;
    Pixmap *avatar;
    Pixmap *bust;
    QQueue<CardItem*> discarded_queue;
    QMainWindow *main_window;
    QComboBox *role_combobox;
    QPushButton *ok_button, *cancel_button, *discard_button;
    QMenu *known_cards_menu;

    QList<QAbstractButton *> skill_buttons;
    QMap<QAbstractButton *, const ViewAsSkill *> button2skill;

    QList<const ClientPlayer *> selected_targets;

    void startEnterAnimation();
    CardItem *takeCardItem(ClientPlayer *src, Player::Place src_place, int card_id);
    void clickSkillButton(int order);
    void useCard(const Card *card);

    void selectTarget(int order, bool multiple);
    void selectNextTarget(bool multiple);
    void unselectAllTargets(const QGraphicsItem *except = NULL);
    void callViewAsSkill();
    void cancelViewAsSkill();
    void setSkillButtonEnablity(bool enablity);

private slots:
    void updateSkillButtons();
    void updateRoleComboBox(const QString &new_role);
    void updateSelectedTargets();
    void startViewAsSkill();
    void doOkButton();
    void doCancelButton();
};

#endif // ROOMSCENE_H
