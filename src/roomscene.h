#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"
#include "aux-skills.h"
#include "daqiao.h"
#include "clientlogbox.h"
#include "audiere.h"

#include <QGraphicsScene>
#include <QTableWidget>
#include <QQueue>
#include <QMainWindow>
#include <QSharedMemory>
#include <QProgressBar>

class RoomScene : public QGraphicsScene{    
    Q_OBJECT

public:
    RoomScene(int player_count, QMainWindow *main_window);

public slots:
    void addPlayer(ClientPlayer *player);
    void removePlayer(const QString &player_name);
    void drawCards(const QList<const Card *> &cards);
    void drawNCards(ClientPlayer *player, int n);
    void chooseGeneral(const QList<const General*> &generals);
    void changeMessage(const QString &message = QString());    
    void arrangeSeats(const QList<const ClientPlayer*> &seats);
    void viewDiscards();
    void hideDiscards();
    void enableTargets(const Card *card);
    void useSelectedCard();
    void updateStatus(Client::Status status);    
    void killPlayer(const QString &who);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void timerEvent(QTimerEvent *event);

private:
    QList<Photo*> photos;
    QMap<QString, Photo*> name2photo;
    Photo *focused;

    Dashboard *dashboard;
    Pixmap *pile;
    Pixmap *avatar;
    QQueue<CardItem*> discarded_queue;
    QMainWindow *main_window;
    QComboBox *role_combobox;
    QPushButton *trust_button;
    QPushButton *ok_button, *cancel_button, *discard_button;
    QProgressBar *progress_bar;
    int timer_id;
    QMenu *known_cards_menu;
    Daqiao *daqiao;
    QMap<QGraphicsItem *, const ClientPlayer *> item2player;    

    int pile_number;
    QGraphicsTextItem *pile_number_item;

    QList<CardItem *> amazing_grace;
    QList<QGraphicsPixmapItem *> taker_avatars;

    QList<QAbstractButton *> skill_buttons;
    QMap<QAbstractButton *, const ViewAsSkill *> button2skill;

    DiscardSkill *discard_skill;
    YijiViewAsSkill *yiji_skill;
    ChoosePlayerSkill *choose_skill;

    QList<const ClientPlayer *> selected_targets;

    QList<GuanxingCardItem *> up_items, down_items;
    QPointF guanxing_origin;

    QList<CardItem *> gongxin_items;

    ClientLogBox *log_box;
    QTextEdit *chat_box;
    QLineEdit *chat_edit;

    QSharedMemory *memory;
    audiere::OutputStreamPtr bgmusic;

    CardItem *takeCardItem(ClientPlayer *src, Player::Place src_place, int card_id);
    void putCardItem(const ClientPlayer *dest, Player::Place dest_place, CardItem *card_item);
    void clickSkillButton(int order);
    void useCard(const Card *card);
    void fillTable(QTableWidget *table, const QList<ClientPlayer *> &players);
    const ViewAsSkill *getViewAsSkill(const QString &skill_name);

    void selectTarget(int order, bool multiple);
    void selectNextTarget(bool multiple);
    void unselectAllTargets(const QGraphicsItem *except = NULL);
    void updateTargetsEnablity(const Card *card);

    void callViewAsSkill();
    void cancelViewAsSkill();

    void freeze();
    void addRestartButton(QDialog *dialog);

private slots:
    void updateSkillButtons();
    void updateRoleComboBox(const QString &new_role);
    void updateSelectedTargets();
    void updateTrustButton();
    void doSkillButton();
    void doOkButton();
    void doCancelButton();
    void doDiscardButton();
    void doTimeout();
    void hideAvatars();
    void changeHp(const QString &who, int delta);
    void moveFocus(const QString &who);
    void setEmotion(const QString &who, const QString &emotion);

    void clearPile();
    void setPileNumber(int n);

    void showCard(const QString &player_name, int card_id);    
    void viewDistance();

    void speak(const QString &who, const QString &text);
    void speak();    

    void onGameStart();
    void onGameOver(bool victory, const QList<bool> &result_list);
    void onStandoff();

    void moveCard(const CardMoveStructForClient &move);
    void moveNCards(int n, const QString &from, const QString &to);
    void moveCardToDrawPile(const QString &from);

    void fillAmazingGrace(const QList<int> &card_ids);    
    void takeAmazingGrace(const ClientPlayer *taker, int card_id);
    void chooseAmazingGrace();
    void clearAmazingGrace();

    void attachSkill(const QString &skill_name);
    void detachSkill(const QString &skill_name);

    void doGuanxing(const QList<int> &card_ids);
    void adjustGuanxing();

    void doGongxin(const QList<int> &card_ids);
    void chooseGongxinCard();

signals:
    void restart();
};

#endif // ROOMSCENE_H
