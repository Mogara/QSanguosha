#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "card.h"
#include "client.h"
#include "aux-skills.h"
#include "clientlogbox.h"
#include "sprite.h"

class Window;
class Button;
class CardContainer;
class GuanxingBox;

class QGroupBox;

#include <QGraphicsScene>
#include <QTableWidget>
#include <QQueue>
#include <QMainWindow>
#include <QSharedMemory>
#include <QProgressBar>
#include <QTextEdit>
#include <QDockWidget>
#include <QSpinBox>
#include <QDialog>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QThread>
#include <QHBoxLayout>

class DeathNoteDialog: public QDialog{
    Q_OBJECT

public:
    DeathNoteDialog(QWidget *parent);

protected:
    virtual void accept();

private:
    QComboBox *killer, *victim;
};

class DamageMakerDialog: public QDialog{
    Q_OBJECT

public:
    DamageMakerDialog(QWidget *parent);

protected:
    virtual void accept();

private:
    QComboBox *damage_source;
    QComboBox *damage_target;
    QComboBox *damage_nature;
    QSpinBox *damage_point;

    void fillCombobox(QComboBox *combobox);

private slots:
    void disableSource();
};

class KOFOrderBox: public QGraphicsPixmapItem{
public:
    KOFOrderBox(bool self, QGraphicsScene *scene);
    void revealGeneral(const QString &name);
    void killPlayer(const QString &general_name);

private:
    Pixmap *avatars[3];
    int revealed;
};

class ReplayerControlBar: public QGraphicsProxyWidget{
    Q_OBJECT

public:
    ReplayerControlBar(Dashboard *dashboard);
    static QString FormatTime(int secs);

public slots:
    void toggle();
    void setTime(int secs);
    void setSpeed(qreal speed);

private:
    QLabel *time_label;
    QString duration_str;
    qreal speed;
};

#ifdef Q_OS_WIN32

class QAxObject;

class SpeakThread: public QThread{
    Q_OBJECT

public:
    SpeakThread(QObject *parent);

public slots:
    void speak(const QString &text);
    void finish();

protected:
    virtual void run();

private:
    QAxObject *voice_obj;
    QSemaphore sem;
    QString to_speak;
};

#endif

class RoomScene : public QGraphicsScene{
    Q_OBJECT

public:
    RoomScene(QMainWindow *main_window);
    void changeTextEditBackground();
    void adjustItems();
    void showIndicator(const QString &from, const QString &to);

    static void FillPlayerNames(QComboBox *combobox, bool add_none);

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
    void revivePlayer(const QString &who);
    void showServerInformation();
    void kick();
    void surrender();
    void saveReplayRecord();
    void makeDamage();
    void makeKilling();
    void makeReviving();

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
    CardItem *special_card;
    bool viewing_discards;
    Dashboard *dashboard;
    Pixmap *avatar;
    QQueue<CardItem*> discarded_queue;
    QMainWindow *main_window;
    QComboBox *role_combobox;
    QPushButton *trust_button, *untrust_button;
    QPushButton *ok_button, *cancel_button, *discard_button;
    QPushButton *reverse_button, *free_discard;
    QMenu *known_cards_menu, *change_general_menu;
    Window *prompt_box;
    QGraphicsItem *control_panel;
    QMap<QGraphicsItem *, const ClientPlayer *> item2player;
    QComboBox *sort_combobox;


    QProgressBar *progress_bar;
    int timer_id;
    int tick;

    QGraphicsItem *state_item;
    QList<QGraphicsPixmapItem *> role_items;

    CardContainer *card_container;

    QList<QAbstractButton *> skill_buttons;
    QMap<QAbstractButton *, const ViewAsSkill *> button2skill;

    ResponseSkill *response_skill;
    DiscardSkill *discard_skill;
    YijiViewAsSkill *yiji_skill;
    ChoosePlayerSkill *choose_skill;

    QList<const Player *> selected_targets;

    GuanxingBox *guanxing_box;

    QList<CardItem *> gongxin_items;

    ClientLogBox *log_box;
    QTextEdit *chat_box;
    QLineEdit *chat_edit;
    QGraphicsProxyWidget *chat_box_widget;

#ifdef AUDIO_SUPPORT
    QSharedMemory *memory;
#endif

    // for 3v3 & 1v1 mode
    Pixmap *selector_box;
    QList<CardItem *> general_items, up_generals, down_generals;
    CardItem *to_change;
    QList<QGraphicsRectItem *> arrange_rects;
    QList<CardItem *> arrange_items;
    Button *arrange_button;
    KOFOrderBox *enemy_box, *self_box;

    CardItem *takeCardItem(ClientPlayer *src, Player::Place src_place, int card_id);
    void putCardItem(const ClientPlayer *dest, Player::Place dest_place, CardItem *card_item);
    void useCard(const Card *card);
    void fillTable(QTableWidget *table, const QList<const ClientPlayer *> &players);
    void chooseSkillButton();

    void viewDiscards();
    void hideDiscards();

    void selectTarget(int order, bool multiple);
    void selectNextTarget(bool multiple);
    void unselectAllTargets(const QGraphicsItem *except = NULL);
    void updateTargetsEnablity(const Card *card = NULL);

    void callViewAsSkill();
    void cancelViewAsSkill();

    void freeze();
    void addRestartButton(QDialog *dialog);
    void addSkillButton(const Skill *skill, bool from_left = false);
    void addWidgetToSkillDock(QWidget *widget, bool from_left = false);
    void removeWidgetFromSkillDock(QWidget *widget);
    QList<QPointF> getPhotoPositions() const;
    void createStateItem();
    void createButtons();
    void createReplayControlBar();

    void fillGenerals1v1(const QStringList &names);
    void fillGenerals3v3(const QStringList &names);

    // animation related functions
    typedef void (RoomScene::*AnimationFunc)(const QString &, const QStringList &);
    QGraphicsObject *getAnimationObject(const QString &name) const;

    void doMovingAnimation(const QString &name, const QStringList &args);
    void doAppearingAnimation(const QString &name, const QStringList &args);
    void doLightboxAnimation(const QString &name, const QStringList &args);
    void doHuashen(const QString &name, const QStringList &args);
    void doIndicate(const QString &name, const QStringList &args);

    void animateHpChange(const QString &name, const QStringList &args);
    void animatePopup(const QString &name, const QStringList &args);
    EffectAnimation *animations;

    //re-layout attempts
    bool game_started;
    void reLayout();
    void alignTo(Pixmap *object, QPoint pos, const QString &flags);
    void alignTo(QWidget *object, QPoint pos, const QString &flags);
    void alignTo(QGraphicsItem *object, QPoint pos, const QString &flags);

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
    void setEmotion(const QString &who, const QString &emotion,bool permanent = false);
    void showSkillInvocation(const QString &who, const QString &skill_name);
    void doAnimation(const QString &name, const QStringList &args);
    void adjustDashboard();
    void showOwnerButtons(bool owner);
    void showJudgeResult(const QString &who, const QString &result);
    void showPlayerCards();
    void updateStateItem(const QString &roles);

    void clearPile();
    void removeLightBox();

    void showCard(const QString &player_name, int card_id);
    void viewDistance();

    void speak();

    void onGameStart();
    void onGameOver();
    void onStandoff();

    //animations
    void onSelectChange();
    void onEnabledChange();

#ifdef AUDIO_SUPPORT
#ifndef  Q_OS_WIN32
    void onMusicFinish();
#endif
#endif

#ifdef JOYSTICK_SUPPORT
    void onJoyButtonClicked(int bit);
    void onJoyDirectionClicked(int direction);
#endif

    void moveCard(const CardMoveStructForClient &move);
    void moveNCards(int n, const QString &from, const QString &to);

    void takeAmazingGrace(const ClientPlayer *taker, int card_id);

    void attachSkill(const QString &skill_name, bool from_left);
    void detachSkill(const QString &skill_name);

    void doGongxin(const QList<int> &card_ids, bool enable_heart);

    void startAssign();

    // 3v3 mode & 1v1 mode
    void fillGenerals(const QStringList &names);
    void takeGeneral(const QString &who, const QString &name);
    void recoverGeneral(int index, const QString &name);
    void startGeneralSelection();
    void selectGeneral();
    void startArrange();
    void toggleArrange();
    void finishArrange();
    void changeGeneral(const QString &general);
    void revealGeneral(bool self, const QString &general);

signals:
    void restart();
};

extern RoomScene *RoomSceneInstance;

#endif // ROOMSCENE_H
