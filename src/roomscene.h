#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"
#include "discardskill.h"
#include "daqiao.h"

#include <QGraphicsScene>
#include <QTableWidget>
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
    void chooseGeneral(const QList<const General*> &generals);
    void changeMessage(const QString &message = QString());
    void moveCard(const CardMoveStructForClient &move);
    void updatePhotos(const QList<const ClientPlayer*> &seats);
    void viewDiscards();
    void hideDiscards();
    void enableTargets(const Card *card);
    void useSelectedCard();
    void updateStatus(Client::Status status);
    void gameOver(bool victory, const QList<bool> &result_list);
    void killPlayer(const QString &who);

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
    Daqiao *daqiao;

    int pile_number;
    QGraphicsTextItem *pile_number_item;

    QList<CardItem *> amazing_grace;
    QList<QGraphicsSimpleTextItem *> taker_names;

    QList<QAbstractButton *> skill_buttons;
    QMap<QAbstractButton *, const ViewAsSkill *> button2skill;
    DiscardSkill *discard_skill;

    QList<const ClientPlayer *> selected_targets;

    void startEnterAnimation();
    CardItem *takeCardItem(ClientPlayer *src, Player::Place src_place, int card_id);
    void putCardItem(const ClientPlayer *dest, Player::Place dest_place, CardItem *card_item);
    void clickSkillButton(int order);
    void useCard(const Card *card);
    void fillTable(QTableWidget *table, const QList<ClientPlayer *> &players);
    const ViewAsSkill *getViewAsSkill(const QString &skill_name);

    void selectTarget(int order, bool multiple);
    void selectNextTarget(bool multiple);
    void unselectAllTargets(const QGraphicsItem *except = NULL);

    void callViewAsSkill();
    void cancelViewAsSkill();    

private slots:
    void updateSkillButtons();
    void updateRoleComboBox(const QString &new_role);
    void updateSelectedTargets();
    void doSkillButton();
    void doOkButton();
    void doCancelButton();
    void doDiscardButton();
    void hideAvatars();
    void changeHp(const QString &who, int delta);
    void clearPile();
    void setPileNumber(int n);
    void showCard(const QString &player_name, int card_id);

    void fillAmazingGrace(const QList<int> &card_ids);    
    void takeAmazingGrace(const ClientPlayer *taker, int card_id);
    void chooseAmazingGrace();
    void clearAmazingGrace();

    void attachSkill(const QString &skill_name);
    void detachSkill(const QString &skill_name);
};

#endif // ROOMSCENE_H
