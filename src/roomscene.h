#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"
#include "aux-skills.h"
#include "clientlogbox.h"

class Window;
class Button;

#include <QGraphicsScene>
#include <QTableWidget>
#include <QQueue>
#include <QMainWindow>
#include <QSharedMemory>
#include <QProgressBar>
#include <QTextEdit>
#include <QDockWidget>

class RoomScene : public QGraphicsScene{    
    Q_OBJECT

public:
    RoomScene(QMainWindow *main_window);
    void changeTextEditBackground();
    void adjustItems();

public slots:
    void addPlayer(ClientPlayer *player);
    void removePlayer(const QString &player_name);
    void drawCards(const QList<const Card *> &cards);
    void drawNCards(ClientPlayer *player, int n);
    void chooseGeneral(const QStringList &generals);
    void arrangeSeats(const QList<const ClientPlayer*> &seats);
    void toggleDiscards();
    void enableTargets(const Card *card);
    void useSelectedCard();
    void updateStatus(Client::Status status);    
    void killPlayer(const QString &who);
    void showServerInformation();
    void kick();
    void surrender();
    void saveReplayRecord();

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
    QGraphicsPixmapItem *judge_avatar;
    CardItem *special_card;
    bool viewing_discards;
    Dashboard *dashboard;
    Pixmap *avatar;
    QQueue<CardItem*> discarded_queue;
    QMainWindow *main_window;
    QComboBox *role_combobox;
    QPushButton *trust_button;
    QPushButton *ok_button, *cancel_button, *discard_button;
    QMenu *known_cards_menu;   
    Window *prompt_box;
    QGraphicsItem *control_panel;
    QMap<QGraphicsItem *, const ClientPlayer *> item2player;    
    QDockWidget *skill_dock;
    QComboBox *sort_combobox;

    QProgressBar *progress_bar;
    int timer_id;
    int tick;

    QList<QGraphicsPixmapItem *> role_items;

    QList<CardItem *> amazing_grace;
    QList<QGraphicsPixmapItem *> taker_avatars;

    QList<QAbstractButton *> skill_buttons;
    QMap<QAbstractButton *, const ViewAsSkill *> button2skill;

    DiscardSkill *discard_skill;
    YijiViewAsSkill *yiji_skill;
    ChoosePlayerSkill *choose_skill;

    QList<const ClientPlayer *> selected_targets;

    QList<CardItem *> up_items, down_items;
    QPointF guanxing_origin;
    bool up_only;

    QList<CardItem *> gongxin_items;

    ClientLogBox *log_box;
    QTextEdit *chat_box;
    QLineEdit *chat_edit;

    QSharedMemory *memory;

    // for 3v3 mode use only
    Pixmap *selector_box;
    QList<CardItem *> general_items, up_generals, down_generals;
    QList<QGraphicsRectItem *> arrange_rects;
    QList<CardItem *> arrange_items;
    Button *arrange_button;

    CardItem *takeCardItem(ClientPlayer *src, Player::Place src_place, int card_id);
    void putCardItem(const ClientPlayer *dest, Player::Place dest_place, CardItem *card_item);
    void useCard(const Card *card);
    void fillTable(QTableWidget *table, const QList<const ClientPlayer *> &players);
    const ViewAsSkill *getViewAsSkill(const QString &skill_name);
    void chooseSkillButton();

    void viewDiscards();
    void hideDiscards();

    void selectTarget(int order, bool multiple);
    void selectNextTarget(bool multiple);
    void unselectAllTargets(const QGraphicsItem *except = NULL);
    void updateTargetsEnablity(const Card *card);

    void callViewAsSkill();
    void cancelViewAsSkill();

    void freeze();
    void addRestartButton(QDialog *dialog);
    void addSkillButton(const Skill *skill);
    void addWidgetToSkillDock(QWidget *widget, bool from_left = false);
    void removeWidgetFromSkillDock(QWidget *widget);
    QList<QPointF> getPhotoPositions() const;
    void createStateItem();

    // animation related functions
    QGraphicsObject *getAnimationObject(const QString &name) const;
    void moveAndDisappear(QGraphicsObject *item, const QPointF &from, const QPointF &to) const;

private slots:
    void updateSkillButtons();
    void acquireSkill(const ClientPlayer *player, const QString &skill_name);
    void updateRoleComboBox(const QString &new_role);
    void updateSelectedTargets();
    void updateTrustButton();
    void updatePileButton(const QString &pile_name);
    void doSkillButton();
    void doOkButton();
    void doCancelButton();
    void doDiscardButton();
    void doTimeout();
    void hideAvatars();
    void changeHp(const QString &who, int delta, DamageStruct::Nature nature);
    void moveFocus(const QString &who);
    void setEmotion(const QString &who, const QString &emotion);
    void showSkillInvocation(const QString &who, const QString &skill_name);
    void doAnimation(const QString &name, const QStringList &args);
    void adjustDashboard();
    void showOwnerButtons(bool owner);
    void showJudgeResult(const QString &who, const QString &result);

    void clearPile();
    void removeLightBox();

    void showCard(const QString &player_name, int card_id);    
    void viewDistance();

    void speak();    

    void onGameStart();
    void onGameOver(bool victory, const QList<bool> &result_list);
    void onStandoff();

    void onJoyButtonClicked(int bit);
    void onJoyDirectionClicked(int direction);

    void moveCard(const CardMoveStructForClient &move);
    void moveNCards(int n, const QString &from, const QString &to);

    void fillAmazingGrace(const QList<int> &card_ids);    
    void takeAmazingGrace(const ClientPlayer *taker, int card_id);
    void grabCardItem();
    void chooseAmazingGrace();
    void clearAmazingGrace();

    void attachSkill(const QString &skill_name);
    void detachSkill(const QString &skill_name);

    void doGuanxing(const QList<int> &card_ids, bool up_only);
    void clearGuanxing();
    void adjustGuanxing();

    void doGongxin(const QList<int> &card_ids, bool enable_heart);
    void chooseGongxinCard();
    void clearGongxinCards();

    // 3v3 mode use only
    void fillGenerals(const QStringList &names);
    void takeGeneral(const QString &who, const QString &name);
    void startGeneralSelection();
    void selectGeneral();
    void startArrange();
    void toggleArrange();
    void finishArrange();

signals:
    void restart();
};

#endif // ROOMSCENE_H
