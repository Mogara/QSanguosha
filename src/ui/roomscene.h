/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef _ROOM_SCENE_H
#define _ROOM_SCENE_H

#include "photo.h"
#include "dashboard.h"
#include "TablePile.h"
#include "card.h"
#include "client.h"
#include "aux-skills.h"
#include "clientlogbox.h"
#include "chatwidget.h"
#include "SkinBank.h"
#include "sprite.h"
#include "qsanbutton.h"

class Window;
class Button;
class CardContainer;
class GuanxingBox;
class QSanButton;
class QGroupBox;
class ChooseGeneralBox;
class ChooseOptionsBox;
class ChooseTriggerOrderBox;
class BubbleChatBox;
class PlayerCardBox;
struct RoomLayout;

#include <QGraphicsScene>
#include <QTableWidget>
#include <QMainWindow>
#include <QTextEdit>
#include <QSpinBox>
#include <QDialog>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include <QThread>
#include <QHBoxLayout>
#include <QMutex>
#include <QStack>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeComponent>
#else
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlComponent>
#endif
class ScriptExecutor : public QDialog {
    Q_OBJECT

public:
    ScriptExecutor(QWidget *parent);

public slots:
    void doScript();
};

class DeathNoteDialog : public QDialog {
    Q_OBJECT

public:
    DeathNoteDialog(QWidget *parent);

protected:
    virtual void accept();

private:
    QComboBox *killer, *victim;
};

class DamageMakerDialog : public QDialog {
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

class ReplayerControlBar : public QGraphicsObject{
    Q_OBJECT

public:
    ReplayerControlBar(Dashboard *dashboard);
    static QString FormatTime(int secs);
    virtual QRectF boundingRect() const;

public slots:
    void setTime(int secs);
    void setSpeed(qreal speed);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    static const int S_BUTTON_GAP = 3;
    static const int S_BUTTON_WIDTH = 25;
    static const int S_BUTTON_HEIGHT = 21;

private:
    QLabel *time_label;
    QString duration_str;
    qreal speed;
};

class RoomScene : public QGraphicsScene {
    Q_OBJECT

public:
    RoomScene(QMainWindow *main_window);
    void changeTextEditBackground();
    void adjustItems();
    void showIndicator(const QString &from, const QString &to);
    void showPromptBox();
    static void FillPlayerNames(QComboBox *ComboBox, bool add_none);
    void updateTable();
    inline QMainWindow *mainWindow() { return main_window; }

    void changeTableBg();

    inline bool isCancelButtonEnabled() const{ return cancel_button != NULL && cancel_button->isEnabled(); }

    void stopHeroSkinChangingAnimations();

    bool m_skillButtonSank;

    bool game_started;

    void addHeroSkinContainer(HeroSkinContainer *heroSkinContainer);
    HeroSkinContainer *findHeroSkinContainer(const QString &generalName) const;

public slots:
    void addPlayer(ClientPlayer *player);
    void removePlayer(const QString &player_name);
    void loseCards(int moveId, QList<CardsMoveStruct> moves);
    void getCards(int moveId, QList<CardsMoveStruct> moves);
    void keepLoseCardLog(const CardsMoveStruct &move);
    void keepGetCardLog(const CardsMoveStruct &move);
    // choice dialog
    void chooseGeneral(const QStringList &generals, const bool single_result);
    void chooseSuit(const QStringList &suits);
    void chooseCard(const ClientPlayer *playerName, const QString &flags, const QString &reason,
        bool handcard_visible, Card::HandlingMethod method, QList<int> disabled_ids);
    void chooseKingdom(const QStringList &kingdoms);
    void chooseOption(const QString &skillName, const QStringList &options);
    //void chooseOrder(QSanProtocol::Game3v3ChooseOrderCommand reason);
    void chooseRole(const QString &scheme, const QStringList &roles);
    //void chooseDirection();
    void chooseTriggerOrder(const QString &reason, const QStringList &options, const bool optional);

    void bringToFront(QGraphicsItem *item);
    void arrangeSeats(const QList<const ClientPlayer *> &seats);
    void toggleDiscards();
    void enableTargets(const Card *card);
    void useSelectedCard();
    void updateStatus(Client::Status oldStatus, Client::Status newStatus);
    void killPlayer(const QString &who);
    void revivePlayer(const QString &who);
    void setDashboardShadow(const QString &who);
    void showServerInformation();
    void surrender();
    void saveReplayRecord(const bool auto_save = false, const bool network_only = false);
    void makeDamage();
    void makeKilling();
    void makeReviving();
    void doScript();
    void viewGenerals(const QString &reason, const QStringList &names);

    void handleGameEvent(const QVariant &args);

    void doOkButton();
    void doCancelButton();
    void doDiscardButton();

    void doTimeout();

    inline QPointF tableCenterPos() { return m_tableCenterPos; }

    void doGongxin(const QList<int> &card_ids, bool enable_heart, QList<int> enabled_ids);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    //this method causes crashes
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QMutex m_roomMutex;
    QMutex m_zValueMutex;

private:
    void _getSceneSizes(QSize &minSize, QSize &maxSize);
    bool _shouldIgnoreDisplayMove(CardsMoveStruct &movement);
    bool _processCardsMove(CardsMoveStruct &move, bool isLost);
    bool _m_isInDragAndUseMode;
    bool _m_superDragStarted;
    const QSanRoomSkin::RoomLayout *_m_roomLayout;
    const QSanRoomSkin::PhotoLayout *_m_photoLayout;
    const QSanRoomSkin::CommonLayout *_m_commonLayout;
    const QSanRoomSkin* _m_roomSkin;
    QGraphicsItem *_m_last_front_item;
    double _m_last_front_ZValue;
    GenericCardContainer *_getGenericCardContainer(Player::Place place, Player *player);
    QMap<int, QList<QList<CardItem *> > > _m_cardsMoveStash;
    Button *add_robot, *fill_robots, *return_to_start_scene;
    QList<Photo *> photos;
    QMap<QString, Photo *> name2photo;
    Dashboard *dashboard;
    TablePile *m_tablePile;
    QMainWindow *main_window;
    QSanButton *ok_button, *cancel_button, *discard_button;
    QMenu *miscellaneous_menu;
    Window *prompt_box;
    Window *pindian_box;
    CardItem *pindian_from_card, *pindian_to_card;
    QGraphicsItem *control_panel;
    QMap<PlayerCardContainer *, const ClientPlayer *> item2player;
    QDialog *m_choiceDialog; // Dialog for choosing generals, suits, card/equip, or kingdoms

    QGraphicsRectItem *pausing_item;
    QGraphicsSimpleTextItem *pausing_text;

    QList<QGraphicsPixmapItem *> role_items;
    CardContainer *card_container;

    QList<QSanSkillButton *> m_skillButtons;

    ResponseSkill *response_skill;
    ShowOrPindianSkill *showorpindian_skill;
    DiscardSkill *discard_skill;
    YijiViewAsSkill *yiji_skill;
    ChoosePlayerSkill *choose_skill;

    QList<const Player *> selected_targets;

    GuanxingBox *guanxing_box;

    ChooseGeneralBox *choose_general_box;

    ChooseOptionsBox *choose_options_box;

    ChooseTriggerOrderBox *chooseTriggerOrderBox;

    PlayerCardBox *playerCardBox;

    QList<CardItem *> gongxin_items;

    ClientLogBox *log_box;
    QTextEdit *chatBox;
    QLineEdit *chatEdit;
    QGraphicsProxyWidget *chat_box_widget;
    QGraphicsProxyWidget *log_box_widget;
    QGraphicsProxyWidget *chat_edit_widget;
    QGraphicsTextItem *prompt_box_widget;
    ChatWidget *chat_widget;
    QPixmap m_rolesBoxBackground;
    QGraphicsPixmapItem *m_rolesBox;
    QGraphicsTextItem *m_pileCardNumInfoTextBox;
    QGraphicsPixmapItem *m_tableBg;
    int m_tablew;
    int m_tableh;

    QMap<QString, BubbleChatBox *> bubbleChatBoxes;

    // for 3v3 & 1v1 mode
    QSanSelectableItem *selector_box;
    QList<CardItem *> general_items, up_generals, down_generals;
    QList<QGraphicsRectItem *> arrange_rects;
    QList<CardItem *> arrange_items;
    Button *arrange_button;
    QPointF m_tableCenterPos;
    ReplayerControlBar *m_replayControl;

    struct _MoveCardsClassifier {
        inline _MoveCardsClassifier(const CardsMoveStruct &move) { m_card_ids = move.card_ids; }
        inline bool operator ==(const _MoveCardsClassifier &other) const{ return m_card_ids == other.m_card_ids; }
        inline bool operator <(const _MoveCardsClassifier &other) const{ return m_card_ids.first() < other.m_card_ids.first(); }
        QList<int> m_card_ids;
    };

    QMap<_MoveCardsClassifier, CardsMoveStruct> m_move_cache;

    // @todo: this function shouldn't be here. But it's here anyway, before someone find a better
    // home for it.
    QString _translateMovement(const CardsMoveStruct &move);

    void useCard(const Card *card);
    void fillTable(QTableWidget *table, const QList<const ClientPlayer *> &players);
    void chooseSkillButton();

    void selectTarget(int order, bool multiple);
    void selectNextTarget(bool multiple);
    void unselectAllTargets(const QGraphicsItem *except = NULL);
    void updateTargetsEnablity(const Card *card = NULL);

    void callViewAsSkill();
    void cancelViewAsSkill();
    void highlightSkillButton(const QString &skillName,
                              const CardUseStruct::CardUseReason reason = CardUseStruct::CARD_USE_REASON_UNKNOWN,
                              const QString &pattern = QString());

    void freeze();
    void addRestartButton(QDialog *dialog);
    QGraphicsItem *createDashboardButtons();
    void createReplayControlBar();

    void showPindianBox(const QString &from_name, int from_id, const QString &to_name, int to_id, const QString &reason);
    void setChatBoxVisible(bool show);

    QRect getBubbleChatBoxShowArea(const QString &who) const;

    // animation related functions
    typedef void (RoomScene::*AnimationFunc)(const QString &, const QStringList &);
    QGraphicsObject *getAnimationObject(const QString &name) const;

    void doMovingAnimation(const QString &name, const QStringList &args);
    void doAppearingAnimation(const QString &name, const QStringList &args);
    void doLightboxAnimation(const QString &name, const QStringList &args);
    void doIndicate(const QString &name, const QStringList &args);
    EffectAnimation *animations;
    bool pindian_success;

    // re-layout attempts
    void _dispersePhotos(QList<Photo *> &photos, QRectF disperseRegion, Qt::Orientation orientation, Qt::Alignment align);

    void _cancelAllFocus();
    // for miniscenes
    int _m_currentStage;

    QRectF _m_infoPlane;

    // for animation effects
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDeclarativeEngine *_m_animationEngine;
    QDeclarativeContext *_m_animationContext;
    QDeclarativeComponent *_m_animationComponent;
#else
    QQmlEngine *_m_animationEngine;
    QQmlContext *_m_animationContext;
    QQmlComponent *_m_animationComponent;
#endif

    QSet<HeroSkinContainer *> m_heroSkinContainers;

private slots:
    void fillCards(const QList<int> &card_ids, const QList<int> &disabled_ids = QList<int>());
    void updateSkillButtons();
    void acquireSkill(const ClientPlayer *player, const QString &skill_name, const bool &head = true);
    void updateSelectedTargets();
    void onSkillActivated();
    void onTransferButtonActivated();
    void onSkillDeactivated();
    void startInXs();
    void hideAvatars();
    void changeHp(const QString &who, int delta, DamageStruct::Nature nature, bool losthp);
    void changeMaxHp(const QString &who, int delta);
    void moveFocus(const QStringList &who, QSanProtocol::Countdown);
    void setEmotion(const QString &who, const QString &emotion);
    void setEmotion(const QString &who, const QString &emotion, bool permanent);
    void showSkillInvocation(const QString &who, const QString &skill_name);
    void doAnimation(int name, const QStringList &args);
    void showOwnerButtons(bool owner);
    void showPlayerCards();
    void updateRolesBox();
    void addSkillButton(const Skill *skill, const bool &head = true);

    void resetPiles();
    void removeLightBox();

    void showCard(const QString &player_name, int card_id);
    void viewDistance();

    void speak();

    void onGameStart();
    void onGameOver();
    void onStandoff();

    void appendChatEdit(QString txt);
    void showBubbleChatBox(const QString &who, const QString &words);

    //animations
    void onEnabledChange();

    void takeAmazingGrace(ClientPlayer *taker, int card_id, bool move_cards);

    void attachSkill(const QString &skill_name, const bool &head = true);
    void detachSkill(const QString &skill_name);

    void doPindianAnimation();

    void updateHandcardNum();

    // 3v3 mode & 1v1 mode
    void fillGenerals(const QStringList &);
    void takeGeneral(const QString &who, const QString &name, const QString &);
    void recoverGeneral(int index, const QString &name);
    void startArrange(const QString &);
    void toggleArrange();
    void finishArrange();
    //void revealGeneral(bool self, const QString &general);

    //void skillStateChange(const QString &skill_name);
    void trust();

signals:
    void restart();
    void return_to_start();
    void cancel_role_box_expanding();
};

extern RoomScene *RoomSceneInstance;

#endif

