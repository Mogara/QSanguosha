#ifndef ROOMSCENE_H
#define ROOMSCENE_H

#include "photo.h"
#include "dashboard.h"
#include "TablePile.h"
#include "card.h"
#include "client.h"
#include "aux-skills.h"
#include "clientlogbox.h"
#include "sprite.h"
#include "chatwidget.h"
#include "SkinBank.h"

class Window;
class Button;
class CardContainer;
class GuanxingBox;
class QSanButton;
class QGroupBox;
struct RoomLayout;

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
#include <QMutex>
#include <QStack>

class ScriptExecutor: public QDialog{
    Q_OBJECT

public:
    ScriptExecutor(QWidget *parent);

public slots:
    void doScript();
};

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

    void fillComboBox(QComboBox *ComboBox);

private slots:
    void disableSource();
};

class KOFOrderBox: public QGraphicsPixmapItem{
public:
    KOFOrderBox(bool self, QGraphicsScene *scene);
    void revealGeneral(const QString &name);
    void killPlayer(const QString &general_name);

private:
    QSanSelectableItem *avatars[3];
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

#ifdef CHAT_VOICE

class QAxObject;

class SpeakThread: public QThread{
    Q_OBJECT

public:
    SpeakThread();

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
    void showPromptBox();
    static void FillPlayerNames(QComboBox *ComboBox, bool add_none);
    void updateTable();

public slots:
    void addPlayer(ClientPlayer *player);
    void removePlayer(const QString &player_name);
    void loseCards(int moveId, QList<CardsMoveStruct> moves);
    void getCards(int moveId, QList<CardsMoveStruct> moves);
    void keepLoseCardLog(const CardsMoveStruct &move);
    void keepGetCardLog(const CardsMoveStruct &move);
    // choice dialog
    void chooseGeneral(const QStringList &generals);
    void chooseSuit(const QStringList &suits);
    void chooseCard(const ClientPlayer *playerName, const QString &flags, const QString &reason);
    void chooseKingdom(const QStringList &kingdoms);
    void chooseOption(const QString& skillName, const QStringList &options);
    void chooseOrder(QSanProtocol::Game3v3ChooseOrderCommand reason);
    void chooseRole(const QString &scheme, const QStringList &roles);
    void chooseDirection();

    void bringToFront(QGraphicsItem* item);
    void arrangeSeats(const QList<const ClientPlayer*> &seats);
    void toggleDiscards();
    void enableTargets(const Card *card);
    void useSelectedCard();
    void updateStatus(Client::Status oldStatus, Client::Status newStatus);
    void killPlayer(const QString &who);
    void revivePlayer(const QString &who);
    void showServerInformation();
    void kick();
    void surrender();
    void saveReplayRecord();
    void makeDamage();
    void makeKilling();
    void makeReviving();
    void doScript();

    void handleEventEffect(const Json::Value &arg);

    EffectAnimation * getEA() const{return animations;}
    
protected:    
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    //this method causes crashes
    //virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QMutex m_roomMutex;
    QMutex m_zValueMutex;	

private:
    bool _shouldIgnoreDisplayMove(Player::Place from, Player::Place to);
    bool _m_isMouseButtonDown;
    bool _m_isInDragAndUseMode;
    const QSanRoomSkin::RoomLayout* _m_roomLayout;
    const QSanRoomSkin::PhotoLayout* _m_photoLayout;
    const QSanRoomSkin::CommonLayout* _m_commonLayout;
    const QSanRoomSkin* _m_roomSkin;
    QGraphicsItem* _m_last_front_item;
    double _m_last_front_ZValue;
    GeneralCardContainer* _getGeneralCardContainer(Player::Place place, Player* player);
    QMap<int, QList<QList<CardItem*> > > _m_cardsMoveStash;
    Button* add_robot, *fill_robots;
    QList<Photo*> photos;
    QMap<QString, Photo*> name2photo;
    Photo *focused;
    CardItem *special_card;
    Dashboard *dashboard;
    TablePile *m_tablePile;
    // QQueue<CardItem*> piled_discards;
    QMainWindow *main_window;
    QSanButton *ok_button, *cancel_button, *discard_button;
    QSanButton *trust_button;
    QPushButton *m_reverseSelectionButton, *m_sortHandcardButton, *m_freeDiscardButton;
    QMenu *known_cards_menu, *change_general_menu;
    Window *prompt_box;
    QGraphicsItem *control_panel;
    QMap<PlayerCardContainer *, const ClientPlayer *> item2player;
    QDialog *m_choiceDialog; // Dialog for choosing generals, suits, card/equip, or kingdoms

    int timer_id;
    int tick;

    
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
    QGraphicsProxyWidget *log_box_widget;
    QGraphicsProxyWidget *chat_edit_widget;
    QGraphicsTextItem *prompt_box_widget;
    ChatWidget *chat_widget;
    QPixmap m_rolesBoxBackground;
    QGraphicsPixmapItem *m_rolesBox;
    QGraphicsTextItem *m_pileCardNumInfoTextBox;
    
#ifdef AUDIO_SUPPORT
    QSharedMemory *memory;
#endif

    // for 3v3 & 1v1 mode
    QSanSelectableItem *selector_box;
    QList<CardItem *> general_items, up_generals, down_generals;
    CardItem *to_change;
    QList<QGraphicsRectItem *> arrange_rects;
    QList<CardItem *> arrange_items;
    Button *arrange_button;
    KOFOrderBox *enemy_box, *self_box;
    QPointF m_tableCenterPos;

    // @todo: this function shouldn't be here. But it's here anyway, before someone find a better
    // home for it.
    QString _translateMovementReason(const CardMoveReason& reason);

    void useCard(const Card *card);
    void fillTable(QTableWidget *table, const QList<const ClientPlayer *> &players);
    void chooseSkillButton();

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
    QGraphicsItem *createDashboardButtons();
    void createExtraButtons();
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
    void animatePopup(const QString &name, const QStringList &args);
    EffectAnimation *animations;

    // re-layout attempts
    bool game_started;
    void _dispersePhotos(QList<Photo*> &photos, QRectF disperseRegion,
                         Qt::Orientation orientation, Qt::Alignment align);


private slots:
    void fillCards(const QList<int>& card_ids);
    void updateSkillButtons();
    void acquireSkill(const ClientPlayer *player, const QString &skill_name);
    void updateSelectedTargets();
    void updateTrustButton();
    void doSkillButton();
    void doOkButton();
    void doCancelButton();
    void doDiscardButton();
    void doTimeout();
    void startInXs();
    void hideAvatars();
    void changeHp(const QString &who, int delta, DamageStruct::Nature nature, bool losthp);
    void moveFocus(const QString &who, QSanProtocol::Countdown);
    void setEmotion(const QString &who, const QString &emotion,bool permanent = false);
    void showSkillInvocation(const QString &who, const QString &skill_name);
    void doAnimation(const QString &name, const QStringList &args);
    void showOwnerButtons(bool owner);
    void showJudgeResult(const QString &who, const QString &result);
    void showPlayerCards();
    void updateRolesBox();
    void updateRoles(const QString &roles);
    void adjustPrompt();

    void resetPiles();
    void removeLightBox();

    void showCard(const QString &player_name, int card_id);
    void viewDistance();

    void speak();

    void onGameStart();
    void onGameOver();
    void onStandoff();

    void appendChatEdit(QString txt);
    void appendChatBox(QString txt);

    //animations
    void onSelectChange();
    void onEnabledChange();

#ifdef JOYSTICK_SUPPORT
    void onJoyButtonClicked(int bit);
    void onJoyDirectionClicked(int direction);
#endif

    void takeAmazingGrace(ClientPlayer *taker, int card_id);

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
    void return_to_start();
};

extern RoomScene *RoomSceneInstance;

#endif // ROOMSCENE_H
