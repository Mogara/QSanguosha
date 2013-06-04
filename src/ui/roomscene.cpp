#include "roomscene.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "cardoverview.h"
#include "distanceviewdialog.h"
#include "playercarddialog.h"
#include "choosegeneraldialog.h"
#include "window.h"
#include "button.h"
#include "cardcontainer.h"
#include "recorder.h"
#include "indicatoritem.h"
#include "pixmapanimation.h"
#include "audio.h"
#include "SkinBank.h"
#include "wind.h"
#include "record-analysis.h"
#include "mountainpackage.h"
#include "jsonutils.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QListWidget>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QCheckBox>
#include <QGraphicsLinearLayout>
#include <QMenu>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QFileDialog>
#include <QDesktopServices>
#include <QRadioButton>
#include <QApplication>
#include <QTimer>
#include <QCommandLinkButton>
#include <QFormLayout>
#include <QCoreApplication>
#include <QInputDialog>
#include <qmath.h>
#include "uiUtils.h"

#ifdef Q_OS_WIN32
#include <QAxObject>
#endif

using namespace QSanProtocol;

RoomScene *RoomSceneInstance;

void RoomScene::resetPiles() {
    // @todo: fix this...
}

#include "qsanbutton.h"

RoomScene::RoomScene(QMainWindow *main_window)
    : main_window(main_window), game_started(false)
{
    m_choiceDialog = NULL;
    RoomSceneInstance = this;
    _m_last_front_item = NULL;
    _m_last_front_ZValue = 0;
    int player_count = Sanguosha->getPlayerCount(ServerInfo.GameMode);

    _m_roomSkin = &(QSanSkinFactory::getInstance().getCurrentSkinScheme().getRoomSkin());
    _m_roomLayout = &(G_ROOM_SKIN.getRoomLayout());
    _m_photoLayout = &(G_ROOM_SKIN.getPhotoLayout());
    _m_commonLayout = &(G_ROOM_SKIN.getCommonLayout());

    m_skillButtonSank = false;

    // create photos
    for (int i = 0; i < player_count - 1;i++) {
        Photo *photo = new Photo;
        photos << photo;
        addItem(photo);
        photo->setZValue(-0.5);
    }

    // create table pile
    m_tablePile = new TablePile;
    addItem(m_tablePile);
    connect(ClientInstance, SIGNAL(card_used()), m_tablePile, SLOT(clear()));

    // create dashboard
    dashboard = new Dashboard(createDashboardButtons());
    dashboard->setObjectName("dashboard");
    dashboard->setZValue(0.8);
    addItem(dashboard);

    dashboard->setPlayer(Self);
    connect(Self, SIGNAL(general_changed()), dashboard, SLOT(updateAvatar()));
    connect(Self, SIGNAL(general2_changed()), dashboard, SLOT(updateSmallAvatar()));
    connect(dashboard, SIGNAL(card_selected(const Card *)), this, SLOT(enableTargets(const Card *)));
    connect(dashboard, SIGNAL(card_to_use()), this, SLOT(doOkButton()));
    //connect(dashboard, SIGNAL(add_equip_skill(const Skill *, bool)), this, SLOT(addSkillButton(const Skill *, bool)));
    //connect(dashboard, SIGNAL(remove_equip_skill(QString)), this, SLOT(detachSkill(QString)));

    connect(Self, SIGNAL(pile_changed(QString)), dashboard, SLOT(updatePile(QString)));

    // add role ComboBox
    connect(Self, SIGNAL(role_changed(QString)), dashboard, SLOT(updateRole(QString)));

    m_replayControl = NULL;
    if (ClientInstance->getReplayer()) {
        dashboard->hideControlButtons();
        createReplayControlBar();
    }

    response_skill = new ResponseSkill;
    showorpindian_skill = new ShowOrPindianSkill;
    discard_skill = new DiscardSkill;
    yiji_skill = new YijiViewAsSkill;
    choose_skill = new ChoosePlayerSkill;

    miscellaneous_menu = new QMenu(main_window);

    change_general_menu = new QMenu(main_window);
    QAction *action = change_general_menu->addAction(tr("Change general ..."));
    FreeChooseDialog *general_changer = new FreeChooseDialog(main_window);
    connect(action, SIGNAL(triggered()), general_changer, SLOT(exec()));
    connect(general_changer, SIGNAL(general_chosen(QString)), this, SLOT(changeGeneral(QString)));
    to_change = NULL;

    // do signal-slot connections
    connect(ClientInstance, SIGNAL(player_added(ClientPlayer *)), SLOT(addPlayer(ClientPlayer *)));
    connect(ClientInstance, SIGNAL(player_removed(QString)), SLOT(removePlayer(QString)));
    connect(ClientInstance, SIGNAL(generals_got(QStringList)), this, SLOT(chooseGeneral(QStringList)));
    connect(ClientInstance, SIGNAL(suits_got(QStringList)), this, SLOT(chooseSuit(QStringList)));
    connect(ClientInstance, SIGNAL(options_got(QString, QStringList)), this, SLOT(chooseOption(QString, QStringList)));
    connect(ClientInstance, SIGNAL(cards_got(const ClientPlayer *, QString, QString, bool, Card::HandlingMethod)), this, SLOT(chooseCard(const ClientPlayer *, QString, QString, bool, Card::HandlingMethod)));
    connect(ClientInstance, SIGNAL(roles_got(QString, QStringList)), this, SLOT(chooseRole(QString, QStringList)));
    connect(ClientInstance, SIGNAL(directions_got()), this, SLOT(chooseDirection()));
    connect(ClientInstance, SIGNAL(orders_got(QSanProtocol::Game3v3ChooseOrderCommand)), this, SLOT(chooseOrder(QSanProtocol::Game3v3ChooseOrderCommand)));
    connect(ClientInstance, SIGNAL(kingdoms_got(QStringList)), this, SLOT(chooseKingdom(QStringList)));
    connect(ClientInstance, SIGNAL(seats_arranged(QList<const ClientPlayer *>)), SLOT(arrangeSeats(QList<const ClientPlayer *>)));
    connect(ClientInstance, SIGNAL(status_changed(Client::Status, Client::Status)), this, SLOT(updateStatus(Client::Status, Client::Status)));
    connect(ClientInstance, SIGNAL(avatars_hiden()), this, SLOT(hideAvatars()));
    connect(ClientInstance, SIGNAL(hp_changed(QString, int, DamageStruct::Nature, bool)), SLOT(changeHp(QString, int, DamageStruct::Nature, bool)));
    connect(ClientInstance, SIGNAL(maxhp_changed(QString, int)), SLOT(changeMaxHp(QString, int)));
    connect(ClientInstance, SIGNAL(pile_reset()), this, SLOT(resetPiles()));
    connect(ClientInstance, SIGNAL(player_killed(QString)), this, SLOT(killPlayer(QString)));
    connect(ClientInstance, SIGNAL(player_revived(QString)), this, SLOT(revivePlayer(QString)));
    connect(ClientInstance, SIGNAL(card_shown(QString, int)), this, SLOT(showCard(QString, int)));
    connect(ClientInstance, SIGNAL(gongxin(QList<int>, bool)), this, SLOT(doGongxin(QList<int>, bool)));
    connect(ClientInstance, SIGNAL(focus_moved(QStringList, QSanProtocol::Countdown)), this, SLOT(moveFocus(QStringList, QSanProtocol::Countdown)));
    connect(ClientInstance, SIGNAL(emotion_set(QString, QString)), this, SLOT(setEmotion(QString, QString)));
    connect(ClientInstance, SIGNAL(skill_invoked(QString, QString)), this, SLOT(showSkillInvocation(QString, QString)));
    connect(ClientInstance, SIGNAL(skill_acquired(const ClientPlayer *, QString)), this, SLOT(acquireSkill(const ClientPlayer *, QString)));
    connect(ClientInstance, SIGNAL(animated(int, QStringList)), this, SLOT(doAnimation(int, QStringList)));
    connect(ClientInstance, SIGNAL(role_state_changed(QString)), this, SLOT(updateRoles(QString)));
    connect(ClientInstance, SIGNAL(event_received(const Json::Value)), this, SLOT(handleGameEvent(const Json::Value)));

    connect(ClientInstance, SIGNAL(game_started()), this, SLOT(onGameStart()));
    connect(ClientInstance, SIGNAL(game_over()), this, SLOT(onGameOver()));
    connect(ClientInstance, SIGNAL(standoff()), this, SLOT(onStandoff()));

    connect(ClientInstance, SIGNAL(move_cards_lost(int, QList<CardsMoveStruct>)), this, SLOT(loseCards(int, QList<CardsMoveStruct>)));
    connect(ClientInstance, SIGNAL(move_cards_got(int, QList<CardsMoveStruct>)), this, SLOT(getCards(int, QList<CardsMoveStruct>)));

    connect(ClientInstance, SIGNAL(nullification_asked(bool)), dashboard, SLOT(controlNullificationButton(bool)));

    connect(ClientInstance, SIGNAL(assign_asked()), this, SLOT(startAssign()));
    connect(ClientInstance, SIGNAL(start_in_xs()), this, SLOT(startInXs()));

    guanxing_box = new GuanxingBox;
    guanxing_box->hide();
    addItem(guanxing_box);
    guanxing_box->setZValue(20000.0);

    connect(ClientInstance, SIGNAL(guanxing(QList<int>, bool)), guanxing_box, SLOT(doGuanxing(QList<int>, bool)));
    guanxing_box->moveBy(-120, 0);

    card_container = new CardContainer();
    card_container->hide();
    addItem(card_container);
    card_container->setZValue(9.0);

    connect(card_container, SIGNAL(item_chosen(int)), ClientInstance, SLOT(onPlayerChooseAG(int)));
    connect(card_container, SIGNAL(item_gongxined(int)), ClientInstance, SLOT(onPlayerReplyGongxin(int)));

    connect(ClientInstance, SIGNAL(ag_filled(QList<int>, QList<int>)), this, SLOT(fillCards(QList<int>, QList<int>)));
    connect(ClientInstance, SIGNAL(ag_taken(ClientPlayer *, int, bool)), this, SLOT(takeAmazingGrace(ClientPlayer *, int, bool)));
    connect(ClientInstance, SIGNAL(ag_cleared()), card_container, SLOT(clear()));

    card_container->moveBy(-120, 0);

    connect(ClientInstance, SIGNAL(skill_attached(QString, bool)), this, SLOT(attachSkill(QString, bool)));
    connect(ClientInstance, SIGNAL(skill_detached(QString)), this, SLOT(detachSkill(QString)));

    enemy_box = NULL;
    self_box = NULL;

    if (ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "02_1v1" || ServerInfo.GameMode == "06_XMode") {
        if (ServerInfo.GameMode != "06_XMode") {
            connect(ClientInstance, SIGNAL(generals_filled(QStringList)), this, SLOT(fillGenerals(QStringList)));
            connect(ClientInstance, SIGNAL(general_asked()), this, SLOT(startGeneralSelection()));
            connect(ClientInstance, SIGNAL(general_taken(QString, QString)), this, SLOT(takeGeneral(QString, QString)));
            connect(ClientInstance, SIGNAL(general_recovered(int, QString)), this, SLOT(recoverGeneral(int, QString)));
        }
        connect(ClientInstance, SIGNAL(arrange_started(QString)), this, SLOT(startArrange(QString)));

        arrange_button = NULL;

        if (ServerInfo.GameMode == "02_1v1") {
            enemy_box = new KOFOrderBox(false, this);
            self_box = new KOFOrderBox(true, this);

            enemy_box->hide();
            self_box->hide();

            connect(ClientInstance, SIGNAL(general_revealed(bool, QString)), this, SLOT(revealGeneral(bool, QString)));
        }
    }

    // chat box
    chat_box = new QTextEdit;
    chat_box->setObjectName("chat_box");
    chat_box_widget = addWidget(chat_box);
    chat_box_widget->setZValue(-2.0);
    chat_box_widget->setObjectName("chat_box_widget");
    chat_box->setReadOnly(true);
    chat_box->setTextColor(Config.TextEditColor);
    connect(ClientInstance, SIGNAL(line_spoken(QString)), this, SLOT(appendChatBox(QString)));

    // chat edit
    chat_edit = new QLineEdit;
    chat_edit->setObjectName("chat_edit");
    chat_edit->setMaxLength(500);
    chat_edit_widget = addWidget(chat_edit);
    chat_edit_widget->setObjectName("chat_edit_widget");
    chat_edit_widget->setZValue(-2.0);
    connect(chat_edit, SIGNAL(returnPressed()), this, SLOT(speak()));
#if QT_VERSION >= 0x040700
    chat_edit->setPlaceholderText(tr("Please enter text to chat ... "));
#endif

    chat_widget = new ChatWidget();
    chat_widget->setZValue(-0.1);
    addItem(chat_widget);
    connect(chat_widget, SIGNAL(return_button_click()), this, SLOT(speak()));
    connect(chat_widget, SIGNAL(chat_widget_msg(QString)), this, SLOT(appendChatEdit(QString)));

    if (ServerInfo.DisableChat)
        chat_edit_widget->hide();

    // log box
    log_box = new ClientLogBox;
    log_box->setTextColor(Config.TextEditColor);
    log_box->setObjectName("log_box");

    log_box_widget = addWidget(log_box);
    log_box_widget->setObjectName("log_box_widget");
    log_box_widget->setZValue(-1.0);
    connect(ClientInstance, SIGNAL(log_received(QStringList)), log_box, SLOT(appendLog(QStringList)));

    prompt_box = new Window(tr("QSanguosha"), QSize(480, 200));
    prompt_box->setOpacity(0);
    prompt_box->setFlag(QGraphicsItem::ItemIsMovable);
    prompt_box->shift();
    prompt_box->setZValue(10);
    prompt_box->keepWhenDisappear();

    prompt_box_widget = new QGraphicsTextItem(prompt_box);
    prompt_box_widget->setParent(prompt_box);
    prompt_box_widget->setPos(40, 45);
    prompt_box_widget->setDefaultTextColor(Qt::white);

    QTextDocument *prompt_doc = ClientInstance->getPromptDoc();
    prompt_doc->setTextWidth(prompt_box->boundingRect().width() - 80);
    prompt_box_widget->setDocument(prompt_doc);

    QFont qf = Config.SmallFont;
    qf.setPixelSize(21);
    qf.setStyleStrategy(QFont::PreferAntialias);
    prompt_box_widget->setFont(qf);

    addItem(prompt_box);

    m_tableBg = new QGraphicsPixmapItem(NULL, this);
    m_tableBg->setZValue(-100000);

    QHBoxLayout *skill_dock_layout = new QHBoxLayout;
    QMargins margins = skill_dock_layout->contentsMargins();
    margins.setTop(0);
    margins.setBottom(5);
    skill_dock_layout->setContentsMargins(margins);
    skill_dock_layout->addStretch();

    m_rolesBoxBackground.load("image/system/state.png");
    m_rolesBox = new QGraphicsPixmapItem;
    addItem(m_rolesBox);
    QString roles = Sanguosha->getRoles(ServerInfo.GameMode);
    m_pileCardNumInfoTextBox = addText("");
    m_pileCardNumInfoTextBox->setParentItem(m_rolesBox);
    m_pileCardNumInfoTextBox->setDocument(ClientInstance->getLinesDoc());
    m_pileCardNumInfoTextBox->setDefaultTextColor(Config.TextEditColor);
    updateRoles(roles);

    add_robot = NULL;
    fill_robots = NULL;
    if (ServerInfo.EnableAI) {
        control_panel = addRect(0, 0, 500, 150, Qt::NoPen);
        control_panel->hide();

        add_robot = new Button(tr("Add a robot"));
        add_robot->setParentItem(control_panel);
        add_robot->setTransform(QTransform::fromTranslate(-add_robot->boundingRect().width() / 2, -add_robot->boundingRect().height() / 2), true);
        add_robot->setPos(0, -add_robot->boundingRect().height() - 10);

        fill_robots = new Button(tr("Fill robots"));
        fill_robots->setParentItem(control_panel);
        fill_robots->setTransform(QTransform::fromTranslate(-fill_robots->boundingRect().width() / 2, -fill_robots->boundingRect().height() / 2), true);
        add_robot->setPos(0, add_robot->boundingRect().height() + 10);

        connect(add_robot, SIGNAL(clicked()), ClientInstance, SLOT(addRobot()));
        connect(fill_robots, SIGNAL(clicked()), ClientInstance, SLOT(fillRobots()));
        connect(Self, SIGNAL(owner_changed(bool)), this, SLOT(showOwnerButtons(bool)));
    } else {
        control_panel = NULL;
    }
    animations = new EffectAnimation();

    pausing_item = new QGraphicsRectItem;
    pausing_text = new QGraphicsSimpleTextItem(tr("Paused ..."));
    addItem(pausing_item);
    addItem(pausing_text);

    pausing_item->setOpacity(0.36);
    pausing_item->setZValue(1002.0);

    QFont font= Config.BigFont;
    font.setPixelSize(100);
    pausing_text->setFont(font);
    pausing_text->setBrush(Qt::white);
    pausing_text->setZValue(1002.1);

    pausing_item->hide();
    pausing_text->hide();

    pindian_box = new Window(tr("pindian"), QSize(255, 200),"image/system/pindian.png");
    pindian_box->setOpacity(0);
    pindian_box->setFlag(QGraphicsItem::ItemIsMovable);
    pindian_box->shift();
    pindian_box->setZValue(10);
    pindian_box->keepWhenDisappear();
    addItem(pindian_box);

    pindian_from_card = NULL;
    pindian_to_card = NULL;
}

void RoomScene::handleGameEvent(const Json::Value &arg) {
    GameEventType eventType = (GameEventType)arg[0].asInt();
    switch (eventType) {
    case S_GAME_EVENT_PLAYER_DYING: {
            ClientPlayer *player = ClientInstance->getPlayer(arg[1].asCString());
            PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
            container->setSaveMeIcon(true);
            Photo *photo = qobject_cast<Photo *>(container);
            if (photo) photo->setFrame(Photo::S_FRAME_SOS);
            break;
        }
    case S_GAME_EVENT_PLAYER_QUITDYING: {
            ClientPlayer *player = ClientInstance->getPlayer(arg[1].asCString());
            PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
            container->setSaveMeIcon(false);
            Photo *photo = qobject_cast<Photo *>(container);
            if (photo) photo->setFrame(Photo::S_FRAME_NO_FRAME);
            break;
        }
    case S_GAME_EVENT_HUASHEN: {
            ClientPlayer *player = ClientInstance->getPlayer(arg[1].asCString());
            QString huashenGeneral = arg[2].asCString();
            QString huashenSkill = arg[3].asCString();
            PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
            container->startHuaShen(huashenGeneral, huashenSkill);
            break;
        }
    case S_GAME_EVENT_PLAY_EFFECT: {
            QString skillName = arg[1].asCString();
            QString category;
            if (arg[2].isBool()) {
                bool isMale = arg[2].asBool();
                category = isMale ? "male" : "female";
            } else if (arg[2].isString())
                category = arg[2].asCString();
            int type = arg[3].asInt();
            Sanguosha->playAudioEffect(G_ROOM_SKIN.getPlayerAudioEffectPath(skillName, category, type));
            break;
        }
    case S_GAME_EVENT_JUDGE_RESULT: {
            int cardId = arg[1].asInt();
            bool takeEffect = arg[2].asBool();
            m_tablePile->showJudgeResult(cardId, takeEffect);
            break;
        }
    case S_GAME_EVENT_DETACH_SKILL: {
            QString player_name = arg[1].asCString();
            QString skill_name =  arg[2].asCString();

            ClientPlayer *player = ClientInstance->getPlayer(player_name);
            player->detachSkill(skill_name);
            if (player == Self) detachSkill(skill_name);

            // stop huashen animation
            PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
            if (!player->hasSkill("huashen"))
                container->stopHuaShen();
            container->updateAvatarTooltip();
            break;
        }
    case S_GAME_EVENT_ACQUIRE_SKILL: {
            QString player_name = arg[1].asCString();
            QString skill_name =  arg[2].asCString();

            ClientPlayer *player = ClientInstance->getPlayer(player_name);
            player->acquireSkill(skill_name);
            acquireSkill(player, skill_name);

            PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
            container->updateAvatarTooltip();
            break;
        }
    case S_GAME_EVENT_ADD_SKILL: {
            QString player_name = arg[1].asCString();
            QString skill_name =  arg[2].asCString();

            ClientPlayer *player = ClientInstance->getPlayer(player_name);
            player->addSkill(skill_name);

            PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
            container->updateAvatarTooltip();
            break;
        }
    case S_GAME_EVENT_LOSE_SKILL: {
            QString player_name = arg[1].asCString();
            QString skill_name =  arg[2].asCString();

            ClientPlayer *player = ClientInstance->getPlayer(player_name);
            player->loseSkill(skill_name);

            PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
            container->updateAvatarTooltip();
            break;
        }
    case S_GAME_EVENT_UPDATE_SKILL: {
            foreach (Photo *photo, photos)
                photo->updateAvatarTooltip();
            dashboard->updateAvatarTooltip();
            updateSkillButtons();
            break;
        }
    case S_GAME_EVENT_CHANGE_GENDER: {
            QString player_name = arg[1].asCString();
            General::Gender gender =  (General::Gender)arg[2].asInt();

            ClientPlayer *player = ClientInstance->getPlayer(player_name);
            player->setGender(gender);

            PlayerCardContainer *container = (PlayerCardContainer*)_getGenericCardContainer(Player::PlaceHand, player);
            container->updateAvatar(); // For Lu Boyan
            break;
        }
    case S_GAME_EVENT_CHANGE_HERO: {
            QString playerName = arg[1].asCString();
            QString newHeroName =  arg[2].asCString();
            bool isSecondaryHero = arg[3].asBool();
            bool sendLog = arg[4].asBool();
            ClientPlayer *player = ClientInstance->getPlayer(playerName);
            if (sendLog) {
                QString type = "#Transfigure";
                QString arg2 = QString();
                if (player->getGeneral2() && !isSecondaryHero) {
                    type = "#TransfigureDual";
                    arg2 = "GeneralA";
                } else if (isSecondaryHero) {
                    type = "#TransfigureDual";
                    arg2 = "GeneralB";
                }
                log_box->appendLog(type, player->objectName(), QStringList(), QString(), newHeroName, arg2);
            }
            if (player->getGeneralName() == "shenlvbu1" && newHeroName == "shenlvbu2"
                && player->getMark("secondMode") > 0)
                Sanguosha->playSystemAudioEffect("stagechange");
            if (player != Self) break;
            const General* oldHero = isSecondaryHero ? player->getGeneral2() : player->getGeneral();
            const General* newHero = Sanguosha->getGeneral(newHeroName);
            if (oldHero) {
                foreach (const Skill *skill, oldHero->getVisibleSkills())
                    detachSkill(skill->objectName());
                if (oldHero->hasSkill("huashen")) {
                    PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
                    container->stopHuaShen();
                }
            }

            if (newHero) {
                foreach (const Skill *skill, newHero->getVisibleSkills())
                    attachSkill(skill->objectName(), false);
            }
            break;
        }
    case S_GAME_EVENT_PLAYER_REFORM: {
            ClientPlayer *player = ClientInstance->getPlayer(arg[1].asCString());
            PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
            container->updateReformState();
            break;
        }
    case S_GAME_EVENT_SKILL_INVOKED: {
            QString player_name = arg[1].asCString();
            QString skill_name =  arg[2].asCString();
            const Skill *skill = Sanguosha->getSkill(skill_name);
            if (skill && (skill->isAttachedLordSkill() || skill->inherits("SPConvertSkill"))) return;

            ClientPlayer *player = ClientInstance->getPlayer(player_name);
            if (!player || !player->hasSkill(skill_name)) return;
            if (player != Self) {
                PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
                Photo *photo = qobject_cast<Photo *>(container);
                if (photo) photo->showSkillName(skill_name);
            }
            break;
        }
    case S_GAME_EVENT_PAUSE: {
            bool paused = arg[1].asBool();
            if (pausing_item->isVisible() != paused) {
                if (paused) {
                    QBrush pausing_brush(QColor(qrand() % 256, qrand() % 256, qrand() % 256));
                    pausing_item->setBrush(pausing_brush);
                    bringToFront(pausing_item);
                    bringToFront(pausing_text);
                }
                pausing_item->setVisible(paused);
                pausing_text->setVisible(paused);
            }
            break;
        }
    case S_GAME_EVENT_REVEAL_PINDIAN: {
            QString from_name = arg[1].asCString(), to_name = arg[3].asCString();
            int from_id = arg[2].asInt(), to_id = arg[4].asInt();
            bool success = arg[5].asBool();
            pindian_success = success;
            QString reason = arg[6].asCString();

            if (Config.value("EnablePindianBox", true).toBool())
                showPindianBox(from_name, from_id, to_name, to_id, reason);
            else
                setEmotion(from_name, success ? "success" : "no-success");
        }
    default:
            break;
    }
}

QGraphicsItem *RoomScene::createDashboardButtons() {
    QGraphicsItem *widget = new QGraphicsPixmapItem(G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_DASHBOARD_BUTTON_SET_BG)
                                                    .scaled(G_DASHBOARD_LAYOUT.m_buttonSetSize));

    ok_button = new QSanButton("platter", "confirm", widget);
    ok_button->setRect(G_DASHBOARD_LAYOUT.m_confirmButtonArea);
    cancel_button = new QSanButton("platter", "cancel", widget);
    cancel_button->setRect(G_DASHBOARD_LAYOUT.m_cancelButtonArea);
    discard_button = new QSanButton("platter", "discard", widget);
    discard_button->setRect(G_DASHBOARD_LAYOUT.m_discardButtonArea);
    connect(ok_button, SIGNAL(clicked()), this, SLOT(doOkButton()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(doCancelButton()));
    connect(discard_button, SIGNAL(clicked()), this, SLOT(doDiscardButton()));

    trust_button = new QSanButton("platter", "trust", widget);
    trust_button->setStyle(QSanButton::S_STYLE_TOGGLE);
    trust_button->setRect(G_DASHBOARD_LAYOUT.m_trustButtonArea);
    connect(trust_button, SIGNAL(clicked()), this, SLOT(trust()));
    connect(Self, SIGNAL(state_changed()), this, SLOT(updateTrustButton()));

    // set them all disabled
    ok_button->setEnabled(false);
    cancel_button->setEnabled(false);
    discard_button->setEnabled(false);
    trust_button->setEnabled(false);
    return widget;
}

QRectF ReplayerControlBar::boundingRect() const{
    return QRectF(0, 0, S_BUTTON_WIDTH * 4 + S_BUTTON_GAP * 3, S_BUTTON_HEIGHT);
}

void ReplayerControlBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
}

ReplayerControlBar::ReplayerControlBar(Dashboard *dashboard) {
    QSanButton *play, *uniform, *slow_down, *speed_up;

    uniform = new QSanButton("replay", "uniform", this);
    slow_down = new QSanButton("replay", "slow-down", this);
    play = new QSanButton("replay", "pause", this);
    speed_up = new QSanButton("replay", "speed-up", this);
    play->setStyle(QSanButton::S_STYLE_TOGGLE);
    uniform->setStyle(QSanButton::S_STYLE_TOGGLE);

    int step = S_BUTTON_GAP + S_BUTTON_WIDTH;
    uniform->setPos(0, 0);
    slow_down->setPos(step, 0);
    play->setPos(step * 2, 0);
    speed_up->setPos(step * 3, 0);

    time_label = new QLabel;
    time_label->setAttribute(Qt::WA_NoSystemBackground);
    time_label->setText("-----------------------------------------------------");
    QPalette palette;
    palette.setColor(QPalette::WindowText, Config.TextEditColor);
    time_label->setPalette(palette);

    QGraphicsProxyWidget *widget = new QGraphicsProxyWidget(this);
    widget->setWidget(time_label);
    widget->setPos(step * 4, 0);

    Replayer *replayer = ClientInstance->getReplayer();
    connect(play, SIGNAL(clicked()), replayer, SLOT(toggle()));
    connect(uniform, SIGNAL(clicked()), replayer, SLOT(uniform()));
    connect(slow_down, SIGNAL(clicked()), replayer, SLOT(slowDown()));
    connect(speed_up, SIGNAL(clicked()), replayer, SLOT(speedUp()));
    connect(replayer, SIGNAL(elasped(int)), this, SLOT(setTime(int)));
    connect(replayer, SIGNAL(speed_changed(qreal)), this, SLOT(setSpeed(qreal)));

    speed = replayer->getSpeed();
    setParentItem(dashboard);
    setPos(S_BUTTON_GAP, -S_BUTTON_GAP - S_BUTTON_HEIGHT);

    duration_str = FormatTime(replayer->getDuration());
}

QString ReplayerControlBar::FormatTime(int secs) {
    int minutes = secs / 60;
    int remainder  = secs % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(remainder, 2, 10, QChar('0'));
}

void ReplayerControlBar::setSpeed(qreal speed) {
    this->speed = speed;
}

void ReplayerControlBar::setTime(int secs) {
    time_label->setText(QString("<b>x%1 </b> [%2/%3]").arg(speed).arg(FormatTime(secs)).arg(duration_str));
}

void RoomScene::createReplayControlBar() {
    m_replayControl = new ReplayerControlBar(dashboard);
}

void RoomScene::_getSceneSizes(QSize &minSize, QSize &maxSize) {
    if (photos.size() >= 8) {
        minSize = _m_roomLayout->m_minimumSceneSize10Player;
        maxSize = _m_roomLayout->m_maximumSceneSize10Player;
    } else {
        minSize = _m_roomLayout->m_minimumSceneSize;
        maxSize = _m_roomLayout->m_maximumSceneSize;
    }
}

void RoomScene::adjustItems() {
    QRectF displayRegion = sceneRect();

    // switch between default & compact skin depending on scene size
    QSanSkinFactory &factory = QSanSkinFactory::getInstance();
    QString skinName = factory.getCurrentSkinName();

    QSize minSize, maxSize;
    _getSceneSizes(minSize, maxSize);
    if (skinName == factory.S_DEFAULT_SKIN_NAME) {
        if (displayRegion.width() < minSize.width() || displayRegion.height() < minSize.height()) {
            QThread *thread = QCoreApplication::instance()->thread();
            thread->blockSignals(true);
            factory.switchSkin(factory.S_COMPACT_SKIN_NAME);
            thread->blockSignals(false);
            foreach (Photo *photo, photos)
                photo->repaintAll();
            dashboard->repaintAll();
        }
    } else if (skinName == factory.S_COMPACT_SKIN_NAME) {
        if (displayRegion.width() > maxSize.width() && displayRegion.height() > maxSize.height()) {
            QThread *thread = QCoreApplication::instance()->thread();
            thread->blockSignals(true);
            factory.switchSkin(factory.S_DEFAULT_SKIN_NAME);
            thread->blockSignals(false);
            foreach (Photo *photo, photos)
                photo->repaintAll();
            dashboard->repaintAll();
        }
    }

    // update the sizes since we have reloaded the skin.
    _getSceneSizes(minSize, maxSize);

    if (displayRegion.left() != 0 || displayRegion.top() != 0
        || displayRegion.bottom() < minSize.height()
        || displayRegion.right() < minSize.width()) {
        displayRegion.setLeft(0); displayRegion.setTop(0);
        double sy = minSize.height() / displayRegion.height();
        double sx = minSize.width() / displayRegion.width();
        double scale = qMax(sx, sy);
        displayRegion.setBottom(scale * displayRegion.height());
        displayRegion.setRight(scale * displayRegion.width());
        setSceneRect(displayRegion);
    }

    int padding = _m_roomLayout->m_scenePadding;
    displayRegion.moveLeft(displayRegion.x() + padding);
    displayRegion.moveTop(displayRegion.y() + padding);
    displayRegion.setWidth(displayRegion.width() - padding * 2);
    displayRegion.setHeight(displayRegion.height() - padding * 2);

    // set dashboard
    dashboard->setX(displayRegion.x());
    dashboard->setWidth(displayRegion.width());
    dashboard->setY(displayRegion.height() - dashboard->boundingRect().height());

    // set infoplane
    QRectF infoPlane;
    infoPlane.setWidth(displayRegion.width() * _m_roomLayout->m_infoPlaneWidthPercentage);
    infoPlane.moveRight(displayRegion.right());
    infoPlane.setTop(displayRegion.top() + _m_roomLayout->m_roleBoxHeight);
    infoPlane.setBottom(dashboard->y() - _m_roomLayout->m_chatTextBoxHeight);
    m_rolesBoxBackground = m_rolesBoxBackground.scaled(infoPlane.width(), _m_roomLayout->m_roleBoxHeight);
    m_rolesBox->setPixmap(m_rolesBoxBackground);
    m_rolesBox->setPos(infoPlane.left(), displayRegion.top());

    log_box_widget->setPos(infoPlane.topLeft());
    log_box->resize(infoPlane.width(), infoPlane.height() * _m_roomLayout->m_logBoxHeightPercentage);
    chat_box_widget->setPos(infoPlane.left(), infoPlane.bottom() - infoPlane.height() * _m_roomLayout->m_chatBoxHeightPercentage);
    chat_box->resize(infoPlane.width(), infoPlane.bottom() - chat_box_widget->y());
    chat_edit_widget->setPos(infoPlane.left(), infoPlane.bottom());
    chat_edit->resize(infoPlane.width() - chat_widget->boundingRect().width(), _m_roomLayout->m_chatTextBoxHeight);
    chat_widget->setPos(infoPlane.right() - chat_widget->boundingRect().width(),
                        chat_edit_widget->y() + (_m_roomLayout->m_chatTextBoxHeight - chat_widget->boundingRect().height()) / 2);

    padding += _m_roomLayout->m_photoRoomPadding;
    if (self_box)
        self_box->setPos(infoPlane.left() - padding - self_box->boundingRect().width(),
                         sceneRect().height() - padding - self_box->boundingRect().height()
                         - G_DASHBOARD_LAYOUT.m_normalHeight - G_DASHBOARD_LAYOUT.m_floatingAreaHeight);
    if (enemy_box)
        enemy_box->setPos(padding * 2, padding * 2);

    padding -= _m_roomLayout->m_photoRoomPadding;
    m_tablew = displayRegion.width() - infoPlane.width();
    m_tableh = displayRegion.height() - dashboard->boundingRect().height();
    QPixmap tableBg = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_TABLE_BG)
                                 .scaled(m_tablew, m_tableh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_tableh -= _m_roomLayout->m_photoDashboardPadding;
    m_tableBg->setPos(padding, padding);
    m_tableBg->setPixmap(tableBg);
    updateTable();
    updateRolesBox();
}

void RoomScene::_dispersePhotos(QList<Photo *> &photos, QRectF fillRegion,
                                Qt::Orientation orientation, Qt::Alignment align) {
    double photoWidth = _m_photoLayout->m_normalWidth;
    double photoHeight = _m_photoLayout->m_normalHeight;
    int numPhotos = photos.size();
    if (numPhotos == 0) return;
    Qt::Alignment hAlign = align & Qt::AlignHorizontal_Mask;
    Qt::Alignment vAlign = align & Qt::AlignVertical_Mask;

    double startX = 0, startY = 0, stepX, stepY;

    if (orientation == Qt::Horizontal) {
        double maxWidth = fillRegion.width();
        stepX = qMax(photoWidth  + G_ROOM_LAYOUT.m_photoHDistance, maxWidth / numPhotos);
        stepY = 0;
    } else {
        stepX = 0;
        stepY = G_ROOM_LAYOUT.m_photoVDistance + photoHeight;
    }

    switch (vAlign) {
    case Qt::AlignTop: startY = fillRegion.top() + photoHeight / 2; break;
    case Qt::AlignBottom: startY = fillRegion.bottom() - photoHeight / 2 - stepY * (numPhotos - 1); break;
    case Qt::AlignVCenter: startY = fillRegion.center().y() - stepY * (numPhotos - 1) / 2.0; break;
    default: Q_ASSERT(false);
    }
    switch (hAlign) {
    case Qt::AlignLeft: startX = fillRegion.left() + photoWidth / 2; break;
    case Qt::AlignRight: startX = fillRegion.right() - photoWidth / 2 - stepX * (numPhotos - 1); break;
    case Qt::AlignHCenter: startX = fillRegion.center().x() - stepX * (numPhotos - 1) / 2.0; break;
    default: Q_ASSERT(false);
    }

    for (int i = 0; i < numPhotos; i++) {
        Photo *photo = photos[i];
        QPointF newPos = QPointF(startX + stepX * i, startY + stepY * i);
        photo->setPos(newPos);
    }
}

void RoomScene::updateTable() {
    int pad = _m_roomLayout->m_scenePadding + _m_roomLayout->m_photoRoomPadding;
    int tablew = log_box_widget->x() - pad * 2;
    int tableh = sceneRect().height() - pad * 2 - dashboard->boundingRect().height();
    if ((ServerInfo.GameMode == "04_1v3" || ServerInfo.GameMode == "06_3v3") && game_started)
        tableh -= _m_roomLayout->m_photoVDistance;
    int photow = _m_photoLayout->m_normalWidth;
    int photoh = _m_photoLayout->m_normalHeight;

    // Layout:
    //    col1           col2
    // _______________________
    // |_2_|______1_______|_0_| row1
    // |   |              |   |
    // | 4 |    table     | 3 |
    // |___|______________|___|
    // |      dashboard       |
    // ------------------------
    // region 5 = 0 + 3, region 6 = 2 + 4, region 7 = 0 + 1 + 2

    static int regularSeatIndex[][9] = {
        {1},
        {5, 6},
        {5, 1, 6},
        {3, 1, 1, 4},
        {3, 1, 1, 1, 4},
        {5, 5, 1, 1, 6, 6},
        {5, 5, 1, 1, 1, 6, 6},
        {3, 3, 7, 7, 7, 7, 4, 4},
        {3, 3, 7, 7, 7, 7, 7, 4, 4}
    };
    static int hulaoSeatIndex[][3] = {
        {1, 1, 1}, // if self is shenlvbu
        {3, 3, 1},
        {3, 1, 4},
        {1, 4, 4}
    };
    static int kof3v3SeatIndex[][5] = {
        {3, 1, 1, 1, 4}, // lord
        {1, 1, 1, 4, 4}, // rebel (left), same with loyalist (left)
        {3, 3, 1, 1, 1} // loyalist (right), same with rebel (right)
    };

    double hGap = _m_roomLayout->m_photoHDistance;
    double vGap = _m_roomLayout->m_photoVDistance;
    double col1 = photow + hGap;
    double col2 = tablew - col1;
    double row1 = photoh + vGap;
    double row2 = tableh;

    const int C_NUM_REGIONS = 8;
    QRectF seatRegions[] = {
        QRectF(col2, pad, col1, row1),
        QRectF(col1, pad, col2 - col1, row1),
        QRectF(pad, pad, col1, row1),
        QRectF(col2, row1, col1, row2 - row1),
        QRectF(pad, row1, col1, row2 - row1),
        QRectF(col2, pad, col1, row2),
        QRectF(pad, pad, col1, row2),
        QRectF(pad, pad, col1 + col2, row1)
    };

    static Qt::Alignment aligns[] = {
        Qt::AlignRight | Qt::AlignTop,
        Qt::AlignHCenter | Qt::AlignTop,
        Qt::AlignLeft | Qt::AlignTop,
        Qt::AlignRight | Qt::AlignVCenter,
        Qt::AlignLeft | Qt::AlignVCenter,
        Qt::AlignRight | Qt::AlignVCenter,
        Qt::AlignLeft | Qt::AlignVCenter,
        Qt::AlignHCenter | Qt::AlignTop,
    };

    static Qt::Alignment kofAligns[] = {
        Qt::AlignRight | Qt::AlignTop,
        Qt::AlignHCenter | Qt::AlignTop,
        Qt::AlignLeft | Qt::AlignTop,
        Qt::AlignRight | Qt::AlignBottom,
        Qt::AlignLeft | Qt::AlignBottom,
        Qt::AlignRight | Qt::AlignBottom,
        Qt::AlignLeft | Qt::AlignBottom,
        Qt::AlignHCenter | Qt::AlignTop,
    };

    Qt::Orientation orients[] = {
        Qt::Horizontal,
        Qt::Horizontal,
        Qt::Horizontal,
        Qt::Vertical,
        Qt::Vertical,
        Qt::Vertical,
        Qt::Vertical,
        Qt::Horizontal
    };

    QRectF tableRect(col1, row1, col2 - col1, row2 - row1);

    QRect tableBottomBar(0, 0, log_box_widget->x() - col1, G_DASHBOARD_LAYOUT.m_floatingAreaHeight);
    tableBottomBar.moveBottomLeft(QPoint((int)tableRect.left(), 0));
    dashboard->setFloatingArea(tableBottomBar);

    m_tableCenterPos = tableRect.center();
    control_panel->setPos(m_tableCenterPos);
    m_tablePile->setPos(m_tableCenterPos);
    m_tablePile->setSize(qMax((int)tableRect.width() - _m_roomLayout->m_discardPilePadding * 2,
                         _m_roomLayout->m_discardPileMinWidth), _m_commonLayout->m_cardNormalHeight);
    m_tablePile->adjustCards();
    card_container->setPos(m_tableCenterPos);
    guanxing_box->setPos(m_tableCenterPos);
    prompt_box->setPos(m_tableCenterPos);
    pausing_text->setPos(m_tableCenterPos - pausing_text->boundingRect().center());
    pausing_item->setRect(sceneRect());
    pausing_item->setPos(0, 0);

    int *seatToRegion;
    bool pkMode = false;
    if (ServerInfo.GameMode == "04_1v3" && game_started) {
        seatToRegion = hulaoSeatIndex[Self->getSeat() - 1];
        pkMode = true;
    } else if (ServerInfo.GameMode == "06_3v3" && game_started) {
        seatToRegion = kof3v3SeatIndex[(Self->getSeat() - 1) % 3];
        pkMode = true;
    } else {
        seatToRegion = regularSeatIndex[photos.length() - 1];
    }
    QList<Photo *> photosInRegion[C_NUM_REGIONS];
    int n = photos.length();
    for (int i = 0; i < n; i++) {
        int regionIndex = seatToRegion[i];
        if (regionIndex == 4 || regionIndex == 6 || regionIndex == 9)
            photosInRegion[regionIndex].append(photos[i]);
        else
            photosInRegion[regionIndex].prepend(photos[i]);
    }
    for (int i = 0; i < C_NUM_REGIONS; i++) {
        if (photosInRegion[i].isEmpty()) continue;
        Qt::Alignment align;
        if (pkMode) align = kofAligns[i];
        else align = aligns[i];
        Qt::Orientation orient = orients[i];

        int hDist = G_ROOM_LAYOUT.m_photoHDistance;
        QRect floatingArea(0, 0, hDist, G_PHOTO_LAYOUT.m_normalHeight);
        // if the photo is on the right edge of table
        if (i == 0 || i == 3 || i == 5 || i == 8)
            floatingArea.moveRight(0);
        else
            floatingArea.moveLeft(G_PHOTO_LAYOUT.m_normalWidth);

        foreach (Photo *photo, photosInRegion[i])
            photo->setFloatingArea(floatingArea);
        _dispersePhotos(photosInRegion[i], seatRegions[i], orient, align);
    }
}

void RoomScene::addPlayer(ClientPlayer *player) {
    for (int i = 0; i < photos.length(); i++) {
        Photo *photo = photos[i];
        if (photo->getPlayer() == NULL) {
            photo->setPlayer(player);
            name2photo[player->objectName()] = photo;

            if (!Self->hasFlag("marshalling"))
                Sanguosha->playSystemAudioEffect("add-player");

            return;
        }
    }
}

void RoomScene::removePlayer(const QString &player_name) {
    Photo *photo = name2photo[player_name];
    if (photo) {
        photo->setPlayer(NULL);
        name2photo.remove(player_name);
        Sanguosha->playSystemAudioEffect("remove-player");
    }
}

void RoomScene::arrangeSeats(const QList<const ClientPlayer *> &seats) {
    // rearrange the photos
    Q_ASSERT(seats.length() == photos.length());

    for (int i = 0; i < seats.length(); i++) {
        const Player *player = seats.at(i);
        for (int j = i; j < photos.length(); j++) {
            if (photos.at(j)->getPlayer() == player) {
                photos.swap(i, j);
                break;
            }
        }
    }
    game_started = true;
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    updateTable();

    group->start(QAbstractAnimation::DeleteWhenStopped);

    // set item to player mapping
    if (item2player.isEmpty()) {
        item2player.insert(dashboard, Self);
        connect(dashboard, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
        connect(dashboard, SIGNAL(selected_changed()), this, SLOT(onSelectChange()));
        foreach (Photo *photo, photos) {
            item2player.insert(photo, photo->getPlayer());
            connect(photo, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
            connect(photo, SIGNAL(selected_changed()), this, SLOT(onSelectChange()));
            connect(photo, SIGNAL(enable_changed()), this, SLOT(onEnabledChange()));
        }
    }
}

// @todo: The following 3 fuctions are for drag & use feature. Currently they are very buggy and
// cause a lot of major problems. We should look into this later.
void RoomScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mousePressEvent(event);
    /*
    _m_isMouseButtonDown = true;
    _m_isInDragAndUseMode = false;
    */
}

void RoomScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseReleaseEvent(event);
    /*
    if (_m_isInDragAndUseMode) {
        bool accepted = false;
        if (ok_button->isEnabled()) {
            foreach (Photo *photo, photos) {
                if (photo->isUnderMouse()) {
                    accepted = true;
                    break;
                }
            }

            if (!accepted && dashboard->isAvatarUnderMouse())
                accepted = true;
        }
        if (accepted)
            ok_button->click();
        else {
            enableTargets(NULL);
            dashboard->unselectAll();
        }
        _m_isInDragAndUseMode = false;
    }*/
}

void RoomScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsScene::mouseMoveEvent(event);
    /*
    QGraphicsObject *obj = static_cast<QGraphicsObject *>(focusItem());
    CardItem *card_item = qobject_cast<CardItem *>(obj);
    if (!card_item || !card_item->isUnderMouse())
        return;
    PlayerCardContainer *victim = NULL;

    foreach (Photo *photo, photos) {
        if (photo->isUnderMouse())
            victim = photo;
    }

    if (dashboard->isAvatarUnderMouse())
        victim = dashboard;

    _m_isInDragAndUseMode = true;
    if (!dashboard->isSelected()) hasUpdate = true;
    if (victim != NULL && !victim->isSelected()) {
        if (!_m_isInDragAndUseMode)
            enableTargets(card_item->getCard());
        _m_isInDragAndUseMode = true;
        dashboard->selectCard(card_item, true);
        victim->setSelected(true);
    } */
}

void RoomScene::enableTargets(const Card *card) {
    bool enabled = true;
    if (card != NULL) {
        Client::Status status = ClientInstance->getStatus();
        if (status == Client::Playing && !card->isAvailable(Self))
            enabled = false;
        if (status == Client::Responding || status == Client::RespondingUse) {
            Card::HandlingMethod method = card->getHandlingMethod();
            if (status == Client::Responding && method == Card::MethodUse)
                method = Card::MethodResponse;
            if (Self->isCardLimited(card, method))
                enabled = false;
        }
        if (status == Client::RespondingForDiscard && Self->isCardLimited(card, Card::MethodDiscard))
            enabled = false;
    }
    if (!enabled) {
        ok_button->setEnabled(false);
        return;
    }

    selected_targets.clear();

    // unset avatar and all photo
    foreach (PlayerCardContainer *item, item2player.keys())
        item->setSelected(false);

    if (card == NULL) {
        foreach (PlayerCardContainer *item, item2player.keys()) {
            QGraphicsItem *animationTarget = item->getMouseClickReceiver();
            animations->effectOut(animationTarget);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            item->setEnabled(true);
        }

        ok_button->setEnabled(false);
        return;
    }

    Client::Status status = ClientInstance->getStatus();
    if (card->targetFixed()
        || ((status & Client::ClientStatusBasicMask) == Client::Responding
            && (status == Client::Responding || (card->getTypeId() != Card::TypeSkill && status != Client::RespondingUse)))
        || ClientInstance->getStatus() == Client::AskForShowOrPindian) {
        foreach (PlayerCardContainer *item, item2player.keys()) {
            QGraphicsItem *animationTarget = item->getMouseClickReceiver();
            animations->effectOut(animationTarget);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }

        ok_button->setEnabled(true);
        return;
    }

    updateTargetsEnablity(card);

    if (selected_targets.isEmpty()) {
        if (card->isKindOf("Slash") && Self->hasFlag("slashTargetFixToOne")) {
            unselectAllTargets();
            foreach (Photo *photo, photos) {
                if (photo->flags() & QGraphicsItem::ItemIsSelectable)
                    if (!photo->isSelected()) {
                        photo->setSelected(true);
                        break;
                    }
            }
        } else if (Config.EnableAutoTarget) {
            if (!card->targetsFeasible(selected_targets, Self)) {
                unselectAllTargets();
                int count = 0;
                foreach (Photo *photo, photos)
                    if (photo->flags() & QGraphicsItem::ItemIsSelectable)
                        count++;
                if (dashboard->flags() & QGraphicsItem::ItemIsSelectable)
                    count++;
                if (count == 1)
                    selectNextTarget(false);
            }
        }
    }

    ok_button->setEnabled(card->targetsFeasible(selected_targets, Self));
}

#include "yjcm2012-package.h"
void RoomScene::updateTargetsEnablity(const Card *card) {
    QMapIterator<PlayerCardContainer *, const ClientPlayer *> itor(item2player);
    while (itor.hasNext()) {
        itor.next();

        PlayerCardContainer *item = itor.key();
        const ClientPlayer *player = itor.value();
        int maxVotes = 0;

        if (card) {
            card->targetFilter(selected_targets, player, Self, maxVotes);
            item->setMaxVotes(maxVotes);
        }

        if (item->isSelected()) continue;

        //=====================================
        bool isCollateral = false;
        if (card) {
            if (card->isKindOf("Collateral"))
                isCollateral = true;
            else if (card->isKindOf("QiceCard")) {
                const QiceCard *qice_card = qobject_cast<const QiceCard *>(card);
                isCollateral = (qice_card->getUserString() == "collateral");
            }
        }
        bool weimuFailure = isCollateral && selected_targets.length() == 1;
        //=====================================

        bool enabled = (card == NULL) || ((weimuFailure || !Sanguosha->isProhibited(Self, player, card, selected_targets)) && maxVotes > 0);

        QGraphicsItem *animationTarget = item->getMouseClickReceiver();
        if (enabled)
            animations->effectOut(animationTarget);
        else if (!animationTarget->graphicsEffect()
                 || !animationTarget->graphicsEffect()->inherits("SentbackEffect"))
            animations->sendBack(animationTarget);

        if (card)
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    }
}

void RoomScene::updateSelectedTargets() {
    PlayerCardContainer *item = qobject_cast<PlayerCardContainer *>(sender());
    if (item == NULL) return;

    const Card *card = dashboard->getSelected();
    if (card) {
        const ClientPlayer *player = item2player.value(item, NULL);
        if (item->isSelected())
            selected_targets.append(player);
        else {
            selected_targets.removeAll(player);
            foreach (const Player *cp, selected_targets) {
                QList<const Player *> tempPlayers = QList<const Player *>(selected_targets);
                tempPlayers.removeAll(cp);
                if (!card->targetFilter(tempPlayers, cp, Self) || Sanguosha->isProhibited(Self, cp, card, selected_targets)) {
                    selected_targets.clear();
                    unselectAllTargets();
                    return;
                }
            }
        }
        ok_button->setEnabled(card->targetsFeasible(selected_targets, Self));
    } else {
        selected_targets.clear();
    }

    updateTargetsEnablity(card);
}

void RoomScene::keyReleaseEvent(QKeyEvent *event) {
    if (!Config.EnableHotKey) return;
    if (chat_edit->hasFocus()) return;

    bool control_is_down = event->modifiers() & Qt::ControlModifier;
    bool alt_is_down = event->modifiers() & Qt::AltModifier;

    switch (event->key()) {
    case Qt::Key_F1: break;
    case Qt::Key_F2: chooseSkillButton(); break;
    case Qt::Key_F3: dashboard->beginSorting(); break;
    case Qt::Key_F4: dashboard->reverseSelection(); break;
    case Qt::Key_F5: {
            foreach (Photo *photo, photos)
                photo->repaintAll();
            dashboard->repaintAll();
            break;
        }
    case Qt::Key_F6: {
            if (!Self || !Self->isOwner() || ClientInstance->getPlayers().length() < Sanguosha->getPlayerCount(ServerInfo.GameMode)) break;
            foreach (const ClientPlayer *p, ClientInstance->getPlayers()) {
                if (p != Self && p->isAlive() && p->getState() != "robot")
                    break;
            }
            bool paused = pausing_text->isVisible();
            QString message = QString("pause %1").arg((paused ? "false" : "true"));
            ClientInstance->request(message);
            break;
        }
    case Qt::Key_F7: {
            if (control_is_down) {
                if (add_robot && add_robot->isVisible())
                    ClientInstance->addRobot();
            } else if (fill_robots && fill_robots->isVisible())
                ClientInstance->fillRobots();
            break;
        }
    case Qt::Key_F12: {
            if (Self->hasSkill("huashen")) {
                const Skill *huashen_skill = Sanguosha->getSkill("huashen");
                if (huashen_skill) {
                    HuashenDialog *dialog = qobject_cast<HuashenDialog *>(huashen_skill->getDialog());
                    if (dialog) dialog->popup();
                }
            }
            break;
        }

    case Qt::Key_S: dashboard->selectCard("slash");  break;
    case Qt::Key_J: dashboard->selectCard("jink"); break;
    case Qt::Key_P: dashboard->selectCard("peach"); break;
    case Qt::Key_O: dashboard->selectCard("analeptic"); break;

    case Qt::Key_E: dashboard->selectCard("equip"); break;
    case Qt::Key_W: dashboard->selectCard("weapon"); break;
    case Qt::Key_F: dashboard->selectCard("armor"); break;
    case Qt::Key_H: dashboard->selectCard("defensive_horse+offensive_horse"); break;

    case Qt::Key_T: dashboard->selectCard("trick"); break;
    case Qt::Key_A: dashboard->selectCard("aoe"); break;
    case Qt::Key_N: dashboard->selectCard("nullification"); break;
    case Qt::Key_Q: dashboard->selectCard("snatch"); break;
    case Qt::Key_C: dashboard->selectCard("dismantlement"); break;
    case Qt::Key_U: dashboard->selectCard("duel"); break;
    case Qt::Key_L: dashboard->selectCard("lightning"); break;
    case Qt::Key_I: dashboard->selectCard("indulgence"); break;
    case Qt::Key_B: dashboard->selectCard("supply_shortage"); break;

    case Qt::Key_Left: dashboard->selectCard(".", false, control_is_down); break;
    case Qt::Key_Right: dashboard->selectCard(".", true, control_is_down); break; // iterate all cards

    case Qt::Key_Return : {
            if (ok_button->isEnabled()) doOkButton();
            break;
        }
    case Qt::Key_Escape : {
            if (ClientInstance->getStatus() == Client::Playing) {
                dashboard->unselectAll();
                enableTargets(NULL);
            } else
                dashboard->unselectAll();
            break;
        }
    case Qt::Key_Space: {
            if (cancel_button->isEnabled())
                doCancelButton();
            else if (discard_button->isEnabled())
                doDiscardButton();
        }

    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4: {
            int position = event->key() - Qt::Key_0;
            if (position != 0 && alt_is_down) {
                dashboard->selectEquip(position);
                break;
            }
        }
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9: {
            int order = event->key() - Qt::Key_0;
            selectTarget(order, control_is_down);
            break;
        }

    case Qt::Key_D: {
            if (Self == NULL) return;
            foreach (Photo *photo, photos) {
                if (photo->getPlayer() && photo->getPlayer()->isAlive())
                    photo->showDistance();
            }
            break;
        }
    case Qt::Key_Z: {
            if (dashboard) {
                m_skillButtonSank = !m_skillButtonSank;
                dashboard->updateSkillButton();
            }
            break;
        }
    }
}

void RoomScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    QGraphicsScene::contextMenuEvent(event);

    QGraphicsItem *item = itemAt(event->scenePos());
    if (item->zValue() < -99999) { // @todo_P: tableBg?
        QMenu *menu = miscellaneous_menu;
        menu->clear();
        menu->setTitle(tr("Miscellaneous"));

        QMenu *private_pile = menu->addMenu(tr("Private Piles"));

        bool enabled = false;
        foreach (PlayerCardContainer *container, item2player.keys()) {
            const ClientPlayer *player = item2player.value(container, NULL);
            QStringList piles = player->getPileNames();
            if (!piles.isEmpty()) {
                foreach (QString pile_name, piles) {
                    bool add = false;
                    foreach (int id, player->getPile(pile_name)) {
                        if (id != Card::S_UNKNOWN_CARD_ID) {
                            add = true;
                            break;
                        }
                    }
                    if (add) {
                        enabled = true;
                        QAction *action = private_pile->addAction(QString("%1 %2")
                                                                  .arg(ClientInstance->getPlayerName(player->objectName()))
                                                                  .arg(Sanguosha->translate(pile_name)));
                        action->setData(QString("%1.%2").arg(player->objectName()).arg(pile_name));
                        connect(action, SIGNAL(triggered()), this, SLOT(showPlayerCards()));
                    }
                }
            }
        }
        private_pile->setEnabled(enabled);
        menu->addSeparator();

        if (ServerInfo.EnableCheat) {
            QMenu *known_cards = menu->addMenu(tr("Known cards"));

            foreach (PlayerCardContainer *container, item2player.keys()) {
                const ClientPlayer *player = item2player.value(container, NULL);
                if (player == Self) continue;
                QList<const Card *> known = player->getHandcards();
                if (known.isEmpty()) {
                    known_cards->addAction(ClientInstance->getPlayerName(player->objectName()))->setEnabled(false);
                } else {
                    QMenu *submenu = known_cards->addMenu(ClientInstance->getPlayerName(player->objectName()));
                    QAction *action = submenu->addAction(tr("View in new dialog"));
                    action->setData(player->objectName());
                    connect(action, SIGNAL(triggered()), this, SLOT(showPlayerCards()));

                    submenu->addSeparator();
                    foreach (const Card *card, known) {
                        const Card *engine_card = Sanguosha->getEngineCard(card->getId());
                        submenu->addAction(G_ROOM_SKIN.getCardSuitPixmap(engine_card->getSuit()), engine_card->getFullName());
                    }
                }
            }
            menu->addSeparator();
        }

        QAction *distance = menu->addAction(tr("View distance"));
        connect(distance, SIGNAL(triggered()), this, SLOT(viewDistance()));
        QAction *discard = menu->addAction(tr("View Discard pile"));
        connect(discard, SIGNAL(triggered()), this, SLOT(toggleDiscards()));

        menu->popup(event->screenPos());
    } else if (ServerInfo.FreeChoose && arrange_button) {
        QGraphicsObject *obj = item->toGraphicsObject();
        if (obj && Sanguosha->getGeneral(obj->objectName())) {
            to_change = qobject_cast<CardItem *>(obj);
            change_general_menu->popup(event->screenPos());
        }
    }
}

void RoomScene::chooseGeneral(const QStringList &generals) {
    QApplication::alert(main_window);
    if (!main_window->isActiveWindow())
        Sanguosha->playSystemAudioEffect("prelude");

    QDialog *dialog;
    if (generals.isEmpty())
        dialog = new FreeChooseDialog(main_window);
    else
        dialog = new ChooseGeneralDialog(generals, main_window);

    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::chooseSuit(const QStringList &suits) {
    QDialog *dialog = new QDialog;
    QVBoxLayout *layout = new QVBoxLayout;

    foreach (QString suit, suits) {
        QCommandLinkButton *button = new QCommandLinkButton;
        button->setIcon(QIcon(QString("image/system/suit/%1.png").arg(suit)));
        button->setText(Sanguosha->translate(suit));
        button->setObjectName(suit);

        layout->addWidget(button);

        connect(button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerChooseSuit()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
    }

    connect(dialog, SIGNAL(rejected()), ClientInstance, SLOT(onPlayerChooseSuit()));

    dialog->setObjectName(".");
    dialog->setWindowTitle(tr("Please choose a suit"));
    dialog->setLayout(layout);
    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::chooseKingdom(const QStringList &kingdoms) {
    QDialog *dialog = new QDialog;
    QVBoxLayout *layout = new QVBoxLayout;

    foreach (QString kingdom, kingdoms) {
        QCommandLinkButton *button = new QCommandLinkButton;
        QPixmap kingdom_pixmap(QString("image/kingdom/icon/%1.png").arg(kingdom));
        QIcon kingdom_icon(kingdom_pixmap);

        button->setIcon(kingdom_icon);
        button->setIconSize(kingdom_pixmap.size());
        button->setText(Sanguosha->translate(kingdom));
        button->setObjectName(kingdom);

        layout->addWidget(button);

        connect(button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerChooseKingdom()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
    }

    dialog->setObjectName(".");
    connect(dialog, SIGNAL(rejected()), ClientInstance, SLOT(onPlayerChooseKingdom()));

    dialog->setObjectName(".");
    dialog->setWindowTitle(tr("Please choose a kingdom"));
    dialog->setLayout(layout);
    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::chooseOption(const QString &skillName, const QStringList &options) {
    QDialog *dialog = new QDialog;
    QVBoxLayout *layout = new QVBoxLayout;
    QString title = Sanguosha->translate(skillName);
    dialog->setWindowTitle(title);
    layout->addWidget(new QLabel(tr("Please choose:")));

    foreach (QString option, options) {
        QCommandLinkButton *button = new QCommandLinkButton;
        QString text = QString("%1:%2").arg(skillName).arg(option);
        QString translated = Sanguosha->translate(text);
        if (text == translated)
            translated = Sanguosha->translate(option);

        button->setObjectName(option);
        button->setText(translated);

        QString original_tooltip = QString(":%1").arg(text);
        QString tooltip = Sanguosha->translate(original_tooltip);
        if (tooltip == original_tooltip) {
            original_tooltip = QString(":%1").arg(option);
            tooltip = Sanguosha->translate(original_tooltip);
        }
        if (tooltip != original_tooltip) button->setToolTip(tooltip);

        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerMakeChoice()));

        layout->addWidget(button);
    }

    dialog->setObjectName(options.first());
    connect(dialog, SIGNAL(rejected()), ClientInstance, SLOT(onPlayerMakeChoice()));

    dialog->setLayout(layout);
    Sanguosha->playSystemAudioEffect("pop-up");
    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::chooseCard(const ClientPlayer *player, const QString &flags, const QString &reason,
                           bool handcard_visible, Card::HandlingMethod method) {
    PlayerCardDialog *dialog = new PlayerCardDialog(player, flags, handcard_visible, method);
    dialog->setWindowTitle(Sanguosha->translate(reason));
    connect(dialog, SIGNAL(card_id_chosen(int)), ClientInstance, SLOT(onPlayerChooseCard(int)));
    connect(dialog, SIGNAL(rejected()), ClientInstance, SLOT(onPlayerChooseCard()));
    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::chooseOrder(QSanProtocol::Game3v3ChooseOrderCommand reason) {
    QDialog *dialog = new QDialog;
    if (reason == S_REASON_CHOOSE_ORDER_SELECT)
        dialog->setWindowTitle(tr("The order who first choose general"));
    else if (reason == S_REASON_CHOOSE_ORDER_TURN)
        dialog->setWindowTitle(tr("The order who first in turn"));

    QLabel *prompt = new QLabel(tr("Please select the order"));
    OptionButton *warm_button = new OptionButton("image/system/3v3/warm.png", tr("Warm"));
    warm_button->setObjectName("warm");
    OptionButton *cool_button = new OptionButton("image/system/3v3/cool.png", tr("Cool"));
    cool_button->setObjectName("cool");

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(warm_button);
    hlayout->addWidget(cool_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(prompt);
    layout->addLayout(hlayout);
    dialog->setLayout(layout);

    connect(warm_button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerChooseOrder()));
    connect(cool_button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerChooseOrder()));
    connect(warm_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cool_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(dialog, SIGNAL(rejected()), ClientInstance, SLOT(onPlayerChooseOrder()));
    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::chooseRole(const QString &scheme, const QStringList &roles) {
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Select role in 3v3 mode"));

    QLabel *prompt = new QLabel(tr("Please select a role"));
    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(prompt);

    static QMap<QString, QString> jargon;
    if (jargon.isEmpty()) {
        jargon["lord"] = tr("Warm leader");
        jargon["loyalist"] = tr("Warm guard");
        jargon["renegade"] = tr("Cool leader");
        jargon["rebel"] = tr("Cool guard");

        jargon["leader1"] = tr("Leader of Team 1");
        jargon["guard1"] = tr("Guard of Team 1");
        jargon["leader2"] = tr("Leader of Team 2");
        jargon["guard2"] = tr("Guard of Team 2");
    }

    foreach (QString role, roles) {
        QCommandLinkButton *button = new QCommandLinkButton(jargon[role]);
        if (scheme == "AllRoles")
            button->setIcon(QIcon(QString("image/system/roles/%1.png").arg(role)));
        layout->addWidget(button);
        button->setObjectName(role);
        connect(button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerChooseRole3v3()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
    }

    QCommandLinkButton *abstain_button = new QCommandLinkButton(tr("Abstain"));
    connect(abstain_button, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(abstain_button);

    dialog->setObjectName("abstain");
    connect(dialog, SIGNAL(rejected()), ClientInstance, SLOT(onPlayerChooseRole3v3()));

    dialog->setLayout(layout);
    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::chooseDirection() {
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Please select the direction"));

    QLabel *prompt = new QLabel(dialog->windowTitle());

    OptionButton *cw_button = new OptionButton("image/system/3v3/cw.png", tr("CW"));
    cw_button->setObjectName("cw");

    OptionButton *ccw_button = new OptionButton("image/system/3v3/ccw.png", tr("CCW"));
    ccw_button->setObjectName("ccw");

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(cw_button);
    hlayout->addWidget(ccw_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(prompt);
    layout->addLayout(hlayout);
    dialog->setLayout(layout);

    dialog->setObjectName("ccw");
    connect(ccw_button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerMakeChoice()));
    connect(ccw_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cw_button, SIGNAL(clicked()), ClientInstance, SLOT(onPlayerMakeChoice()));
    connect(cw_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(dialog, SIGNAL(rejected()), ClientInstance, SLOT(onPlayerMakeChoice()));
    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::toggleDiscards() {
    CardOverview *overview = new CardOverview;
    overview->setWindowTitle(tr("Discarded pile"));
    QList<const Card *> cards;
    foreach (const Card *card, ClientInstance->discarded_list)
        cards << Sanguosha->getEngineCard(card->getId());
    overview->loadFromList(cards);
    overview->show();
}

GenericCardContainer *RoomScene::_getGenericCardContainer(Player::Place place, Player *player) {
    if (place == Player::DiscardPile || place == Player::PlaceJudge
        || place == Player::DrawPile || place == Player::PlaceTable)
        return m_tablePile;
    // @todo: AG must be a pile with name rather than simply using the name special...
    else if (player == NULL && place == Player::PlaceSpecial)
        return card_container;
    else if (player == Self)
        return dashboard;
    else if (player != NULL)
        return name2photo.value(player->objectName(), NULL);
    else Q_ASSERT(false);
    return NULL;
}

bool RoomScene::_shouldIgnoreDisplayMove(CardsMoveStruct &movement) {
    Player::Place from = movement.from_place, to = movement.to_place;
    QString from_pile = movement.from_pile_name, to_pile = movement.to_pile_name;
    if ((from == Player::PlaceSpecial && !from_pile.isEmpty() && from_pile.startsWith('#'))
        || (to == Player::PlaceSpecial && !to_pile.isEmpty() && to_pile.startsWith('#')))
        return true;
    else {
        static QList<Player::Place> ignore_place;
        if (ignore_place.isEmpty())
            ignore_place << Player::DiscardPile << Player::PlaceTable << Player::PlaceJudge;
        return movement.reason.m_skillName != "manjuan"
               && ignore_place.contains(from) && ignore_place.contains(to);
    }
}

bool RoomScene::_processCardsMove(CardsMoveStruct &move, bool isLost) {
    _MoveCardsClassifier cls(move);
    // delayed trick processed;
    if (move.from_place == Player::PlaceDelayedTrick && move.to_place == Player::PlaceTable) {
        if (isLost) m_move_cache[cls] = move;
        return true;
    }
    CardsMoveStruct tmpMove = m_move_cache.value(cls, CardsMoveStruct());
    if (tmpMove.from_place != Player::PlaceUnknown) {
        move.from = tmpMove.from;
        move.from_place = tmpMove.from_place;
        move.from_pile_name = tmpMove.from_pile_name;
    }
    if (!isLost) m_move_cache.remove(cls);
    return false;
}

void RoomScene::getCards(int moveId, QList<CardsMoveStruct> card_moves) {
    int count = 0;
    for (int i = 0; i < card_moves.size(); i++) {
        CardsMoveStruct &movement = card_moves[i];
        bool skipMove = _processCardsMove(movement, false);
        if (skipMove) continue;
        if (_shouldIgnoreDisplayMove(movement)) continue;
        card_container->m_currentPlayer = (ClientPlayer *)movement.to;
        GenericCardContainer *to_container = _getGenericCardContainer(movement.to_place, movement.to);
        QList<CardItem *> cards = _m_cardsMoveStash[moveId][count];
        count++;
        for (int j = 0; j < cards.size(); j++) {
            CardItem *card = cards[j];
            card->setFlag(QGraphicsItem::ItemIsMovable, false);
            int card_id = card->getId();
            if (!card_moves[i].card_ids.contains(card_id)) {
                card->setVisible(false);
                card->deleteLater();
                cards.removeAt(j);
                j--;
            } else
                card->setEnabled(true);
            card->setFootnote(_translateMovement(movement));
            card->hideFootnote();
        }
        bringToFront(to_container);
        to_container->addCardItems(cards, movement);
        keepGetCardLog(movement);
    }
    _m_cardsMoveStash[moveId].clear();
}

void RoomScene::loseCards(int moveId, QList<CardsMoveStruct> card_moves) {
    for (int i = 0; i < card_moves.size(); i++) {
        CardsMoveStruct &movement = card_moves[i];
        bool skipMove = _processCardsMove(movement, true);
        if (skipMove) continue;
        if (_shouldIgnoreDisplayMove(movement)) continue;
        card_container->m_currentPlayer = (ClientPlayer *)movement.to;
        GenericCardContainer *from_container = _getGenericCardContainer(movement.from_place, movement.from);
        QList<CardItem *> cards = from_container->removeCardItems(movement.card_ids, movement.from_place);
        foreach (CardItem *card, cards)
            card->setEnabled(false);

        _m_cardsMoveStash[moveId].append(cards);
        keepLoseCardLog(movement);
    }
}

QString RoomScene::_translateMovement(const CardsMoveStruct &move) {
    CardMoveReason reason = move.reason;
    if (reason.m_reason == CardMoveReason::S_REASON_UNKNOWN) return QString();
    // ============================================
    if (move.from && move.card_ids.length() == 1 && move.to_place == Player::DrawPile
        && move.from->property("zongxuan_move").toString() == QString::number(move.card_ids.first()))
        reason = CardMoveReason(CardMoveReason::S_REASON_PUT, move.from_player_name, QString(), "zongxuan", QString());
    // ============================================
    Photo *srcPhoto = name2photo[reason.m_playerId];
    Photo *dstPhoto = name2photo[reason.m_targetId];
    QString playerName, targetName;

    if (srcPhoto != NULL)
        playerName = Sanguosha->translate(srcPhoto->getPlayer()->getGeneralName());
    else if (reason.m_playerId == Self->objectName())
        playerName = QString("%1(%2)").arg(Sanguosha->translate(Self->getGeneralName())).arg(Sanguosha->translate("yourself"));

    if (dstPhoto != NULL)
        targetName = Sanguosha->translate("use upon").append(Sanguosha->translate(dstPhoto->getPlayer()->getGeneralName()));
    else if (reason.m_targetId == Self->objectName())
        targetName = QString("%1%2(%3)").arg(Sanguosha->translate("use upon"))
                                        .arg(Sanguosha->translate(Self->getGeneralName()))
                                        .arg(Sanguosha->translate("yourself"));

    QString result(playerName + targetName);
    result.append(Sanguosha->translate(reason.m_eventName));
    result.append(Sanguosha->translate(reason.m_skillName));
    if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_USE && reason.m_skillName.isEmpty()) {
        result.append(Sanguosha->translate("use"));
    } else if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_RESPONSE) {
        if (reason.m_reason == CardMoveReason::S_REASON_RETRIAL)
            result.append(Sanguosha->translate("retrial"));
        else if (reason.m_skillName.isEmpty())
            result.append(Sanguosha->translate("response"));
    } else if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD) {
        if (reason.m_reason == CardMoveReason::S_REASON_RULEDISCARD)
            result.append(Sanguosha->translate("discard"));
        else if (reason.m_reason == CardMoveReason::S_REASON_THROW)
            result.append(Sanguosha->translate("throw"));
        else if (reason.m_reason == CardMoveReason::S_REASON_CHANGE_EQUIP)
            result.append(Sanguosha->translate("change equip"));
        else if (reason.m_reason == CardMoveReason::S_REASON_DISMANTLE)
            result.append(Sanguosha->translate("throw"));
    } else if (reason.m_reason == CardMoveReason::S_REASON_RECAST) {
        result.append(Sanguosha->translate("recast"));
    } else if (reason.m_reason == CardMoveReason::S_REASON_PINDIAN) {
        result.append(Sanguosha->translate("pindian"));
    } else if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_SHOW) {
        if (reason.m_reason == CardMoveReason::S_REASON_JUDGE)
            result.append(Sanguosha->translate("judge"));
        else if (reason.m_reason == CardMoveReason::S_REASON_TURNOVER)
            result.append(Sanguosha->translate("turnover"));
        else if (reason.m_reason == CardMoveReason::S_REASON_DEMONSTRATE)
            result.append(Sanguosha->translate("show"));
        else if (reason.m_reason == CardMoveReason::S_REASON_PREVIEW)
            result.append(Sanguosha->translate("preview"));
    } else if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_PUT) {
        if (reason.m_reason == CardMoveReason::S_REASON_PUT) {
            result.append(Sanguosha->translate("put"));
            if (move.to_place == Player::DiscardPile)
                result.append(Sanguosha->translate("discardPile"));
            else if (move.to_place == Player::DrawPile)
                result.append(Sanguosha->translate("drawPileTop"));
        } else if (reason.m_reason == CardMoveReason::S_REASON_NATURAL_ENTER) {
            result.append(Sanguosha->translate("enter"));
            if (move.to_place == Player::DiscardPile)
                result.append(Sanguosha->translate("discardPile"));
            else if (move.to_place == Player::DrawPile)
                result.append(Sanguosha->translate("drawPileTop"));
        } else if (reason.m_reason == CardMoveReason::S_REASON_JUDGEDONE) {
            result.append(Sanguosha->translate("judgedone"));
        } else if (reason.m_reason == CardMoveReason::S_REASON_REMOVE_FROM_PILE) {
            result.append(Sanguosha->translate("backinto"));
        }
    }
    return result;
}

void RoomScene::keepLoseCardLog(const CardsMoveStruct &move) {
    if (move.from && move.to_place == Player::DrawPile) {
        bool hidden = false;
        foreach (int id, move.card_ids) {
            if (id == Card::S_UNKNOWN_CARD_ID) {
                hidden = true;
                break;
            }
        }

        QString type = hidden ? "#PutCard" : "$PutCard";
        QString from_general = move.from->objectName();
        if (hidden)
            log_box->appendLog(type, from_general, QStringList(), QString(), QString::number(move.card_ids.length()));
        else
            log_box->appendLog(type, from_general, QStringList(), IntList2StringList(move.card_ids).join("+"));
    }
}

void RoomScene::keepGetCardLog(const CardsMoveStruct &move) {
    if (move.card_ids.isEmpty()) return;
    if (move.to
        && (move.to_place == Player::PlaceHand
            || move.to_place == Player::PlaceEquip
            || move.to_place == Player::PlaceSpecial)
        && move.from_place != Player::DrawPile) {
        foreach (QString flag, move.to->getFlagList())
            if (flag.endsWith("_InTempMoving"))
                return;
    }
    // private pile
    if (move.to_place == Player::PlaceSpecial && !move.to_pile_name.isNull() && !move.to_pile_name.startsWith('#')) {
        bool hidden = (move.card_ids.contains(Card::S_UNKNOWN_CARD_ID));
        if (hidden)
            log_box->appendLog("#RemoveFromGame", QString(), QStringList(), QString(), move.to_pile_name, QString::number(move.card_ids.length()));
        else
            log_box->appendLog("$AddToPile", QString(), QStringList(), IntList2StringList(move.card_ids).join("+"), move.to_pile_name);
    }
    if (move.from_place == Player::PlaceSpecial && move.to
        && move.reason.m_reason == CardMoveReason::S_REASON_EXCHANGE_FROM_PILE) {
        bool hidden = (move.card_ids.contains(Card::S_UNKNOWN_CARD_ID));
        if (!hidden)
            log_box->appendLog("$GotCardFromPile", move.to->objectName(), QStringList(), IntList2StringList(move.card_ids).join("+"), move.from_pile_name);
        else
            log_box->appendLog("#GotNCardFromPile", move.to->objectName(), QStringList(), QString(), move.from_pile_name, QString::number(move.card_ids.length()));
    }
    //DrawNCards
    if (move.from_place == Player::DrawPile && move.to_place == Player::PlaceHand) {
        QString to_general = move.to->objectName();
        bool hidden = (move.card_ids.contains(Card::S_UNKNOWN_CARD_ID));
        if (!hidden)
            log_box->appendLog("$DrawCards", to_general, QStringList(), IntList2StringList(move.card_ids).join("+"),
                               QString::number(move.card_ids.length()));
        else
            log_box->appendLog("#DrawNCards", to_general, QStringList(), QString(),
                               QString::number(move.card_ids.length()));

    }
    if ((move.from_place == Player::PlaceTable || move.from_place == Player::PlaceJudge)
        && move.to_place == Player::PlaceHand
        && move.reason.m_reason != CardMoveReason::S_REASON_PREVIEW) {
        QString to_general = move.to->objectName();
        QList<int> ids = move.card_ids;
        ids.removeAll(Card::S_UNKNOWN_CARD_ID);
        if (!ids.isEmpty()) {
            QString card_str = IntList2StringList(ids).join("+");
            log_box->appendLog("$GotCardBack", to_general, QStringList(), card_str);
        }
    }
    if (move.from_place == Player::DiscardPile && move.to_place == Player::PlaceHand) {
        QString to_general = move.to->objectName();
        QString card_str = IntList2StringList(move.card_ids).join("+");
        log_box->appendLog("$RecycleCard", to_general, QStringList(), card_str);
    }
    if (move.from && move.from_place != Player::PlaceHand && move.to_place != Player::PlaceDelayedTrick
        && move.from_place != Player::PlaceJudge && move.to && move.from != move.to) {
        QString from_general = move.from->objectName();
        QStringList tos;
        tos << move.to->objectName();
        QList<int> ids = move.card_ids;
        ids.removeAll(Card::S_UNKNOWN_CARD_ID);
        int hide = move.card_ids.length() - ids.length();
        if (!ids.isEmpty()) {
            QString card_str = IntList2StringList(ids).join("+");
            log_box->appendLog("$MoveCard", from_general, tos, card_str);
        }
        if (hide > 0)
            log_box->appendLog("#MoveNCards", from_general, tos, QString(), QString::number(hide));
    }
    if (move.from_place == Player::PlaceHand && move.to_place == Player::PlaceHand) {
        QString from_general = move.from->objectName();
        QStringList tos;
        tos << move.to->objectName();
        bool hidden = (move.card_ids.contains(Card::S_UNKNOWN_CARD_ID));
        if (hidden)
            log_box->appendLog("#MoveNCards", from_general, tos, QString(), QString::number(move.card_ids.length()));
        else
            log_box->appendLog("$MoveCard", from_general, tos, IntList2StringList(move.card_ids).join("+"));
    }
    if (move.from && move.to) {
        // both src and dest are player
        QString type;
        if (move.to_place == Player::PlaceDelayedTrick) {
            if (move.from_place == Player::PlaceDelayedTrick && move.from != move.to)
                type = "$LightningMove";
            else
                type = "$PasteCard";
        }
        if (!type.isNull()) {
            QString from_general = move.from->objectName();
            QStringList tos;
            tos << move.to->objectName();
            log_box->appendLog(type, from_general, tos, QString::number(move.card_ids.first()));
        }
    }
    if (move.from && move.to && move.from_place == Player::PlaceEquip && move.to_place == Player::PlaceEquip) {
        QString type = "$Install";
        QString to_general = move.to->objectName();
        foreach (int card_id, move.card_ids)
            log_box->appendLog(type, to_general, QStringList(), QString::number(card_id));
    }
    if (move.reason.m_reason == CardMoveReason::S_REASON_TURNOVER)
        log_box->appendLog("$TurnOver", move.reason.m_playerId, QStringList(), IntList2StringList(move.card_ids).join("+"));
}

void RoomScene::addSkillButton(const Skill *skill, bool) {
    if (skill->inherits("SPConvertSkill")) return;
    // check duplication
    QSanSkillButton *btn = dashboard->addSkillButton(skill->objectName());
    if (btn == NULL) return;

    if (btn->getViewAsSkill() != NULL  && !m_replayControl) {
        connect(btn, SIGNAL(skill_activated()), dashboard, SLOT(skillButtonActivated()));
        connect(btn, SIGNAL(skill_activated()), this, SLOT(onSkillActivated()));
        connect(btn, SIGNAL(skill_deactivated()), dashboard, SLOT(skillButtonDeactivated()));
        connect(btn, SIGNAL(skill_deactivated()), this, SLOT(onSkillDeactivated()));
        if (btn->getViewAsSkill()->objectName() == "mizhao")
            connect(btn, SIGNAL(skill_activated()), dashboard, SLOT(selectAll()));
    }

    QDialog *dialog = skill->getDialog();
    if (dialog && !m_replayControl) {
        dialog->setParent(main_window, Qt::Dialog);
        connect(btn, SIGNAL(skill_activated()), dialog, SLOT(popup()));
        connect(btn, SIGNAL(skill_deactivated()), dialog, SLOT(reject()));
        if (dialog->objectName() == "qice")
            connect(dialog, SIGNAL(onButtonClick()), dashboard, SLOT(selectAll()));
    }

    m_skillButtons.append(btn);
}

void RoomScene::acquireSkill(const ClientPlayer *player, const QString &skill_name) {
    QString type = "#AcquireSkill";
    QString from_general = player->objectName();
    QString arg = skill_name;
    log_box->appendLog(type, from_general, QStringList(), QString(), arg);

    if (player == Self) addSkillButton(Sanguosha->getSkill(skill_name));
}

void RoomScene::updateSkillButtons() {
    foreach (const Skill *skill, Self->getVisibleSkillList()) {
        if (skill->isLordSkill()
            && (Self->getRole() != "lord"
                || ServerInfo.GameMode == "06_3v3"
                || ServerInfo.GameMode == "06_XMode"
                || ServerInfo.GameMode == "02_1v1"
                || Config.value("WithoutLordskill", false).toBool()))
                continue;

        addSkillButton(skill);
    }

    // disable all skill buttons
    foreach (QSanSkillButton *button, m_skillButtons)
        button->setEnabled(false);
}

void RoomScene::useSelectedCard() {
    switch (ClientInstance->getStatus() & Client::ClientStatusBasicMask) {
    case Client::Playing: {
            const Card *card = dashboard->getSelected();
            if (card) useCard(card);
            break;
        }
    case Client::Responding: {
            const Card *card = dashboard->getSelected();
            if (card) {
                if (ClientInstance->getStatus() == Client::Responding) {
                    Q_ASSERT(selected_targets.isEmpty());
                    selected_targets.clear();
                }
                ClientInstance->onPlayerResponseCard(card, selected_targets);
                prompt_box->disappear();
            }

            dashboard->unselectAll();
            break;
        }
    case Client::AskForShowOrPindian: {
            const Card *card = dashboard->getSelected();
            if (card) {
                ClientInstance->onPlayerResponseCard(card);
                prompt_box->disappear();
            }
            dashboard->unselectAll();
            break;
        }
    case Client::Discarding:
    case Client::Exchanging: {
            const Card *card = dashboard->pendingCard();
            if (card) {
                ClientInstance->onPlayerDiscardCards(card);
                dashboard->stopPending();
                prompt_box->disappear();
            }
            break;
        }
    case Client::NotActive: {
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is not active!"));
            return;
        }
    case Client::AskForAG: {
            ClientInstance->onPlayerChooseAG(-1);
            return;
        }
    case Client::ExecDialog: {
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is in executing dialog"));
            return;
        }
    case Client::AskForSkillInvoke: {
            prompt_box->disappear();
            QString skill_name = ClientInstance->getSkillNameToInvoke();
            dashboard->highlightEquip(skill_name, false);
            ClientInstance->onPlayerInvokeSkill(true);
            break;
        }
    case Client::AskForPlayerChoose: {
            ClientInstance->onPlayerChoosePlayer(selected_targets.first());
            prompt_box->disappear();
            break;
        }
    case Client::AskForYiji: {
            const Card *card = dashboard->pendingCard();
            if (card) {
                ClientInstance->onPlayerReplyYiji(card, selected_targets.first());
                dashboard->stopPending();
                prompt_box->disappear();
            }
            break;
        }
    case Client::AskForGuanxing: {
            guanxing_box->reply();
            break;
        }
    case Client::AskForGongxin: {
            ClientInstance->onPlayerReplyGongxin();
            card_container->clear();
            break;
        }
    }

    const ViewAsSkill *skill = dashboard->currentSkill();
    if (skill) dashboard->stopPending();
}

void RoomScene::onSelectChange() {
}

void RoomScene::onEnabledChange() {
    QGraphicsItem *photo = qobject_cast<QGraphicsItem *>(sender());
    if (!photo) return;
    if (photo->isEnabled())
        animations->effectOut(photo);
    else
        animations->sendBack(photo);
}

void RoomScene::useCard(const Card *card) {
    if (card->targetFixed() || card->targetsFeasible(selected_targets, Self))
        ClientInstance->onPlayerResponseCard(card, selected_targets);
     enableTargets(NULL);
}

void RoomScene::callViewAsSkill() {
    const Card *card = dashboard->pendingCard();
    if (card == NULL) return;

    if (card->isAvailable(Self)) {
        // use card
        dashboard->stopPending();
        useCard(card);
    }
}

void RoomScene::cancelViewAsSkill() {
    dashboard->stopPending();
    Client::Status status = ClientInstance->getStatus();
    updateStatus(status, status);
}

void RoomScene::selectTarget(int order, bool multiple) {
    if (!multiple) unselectAllTargets();

    QGraphicsItem *to_select = NULL;
    if (order == 0)
        to_select = dashboard;
    else if (order > 0 && order <= photos.length())
        to_select = photos.at(order - 1);

    if (!to_select) return;
    if (!(to_select->isSelected()
          || (!to_select->isSelected() && (to_select->flags() & QGraphicsItem::ItemIsSelectable))))
        return;

    to_select->setSelected(!to_select->isSelected());
}

void RoomScene::selectNextTarget(bool multiple) {
    if (!multiple)
        unselectAllTargets();

    QList<QGraphicsItem *> targets;
    foreach (Photo *photo, photos) {
        if (photo->flags() & QGraphicsItem::ItemIsSelectable)
            targets << photo;
    }

    if (dashboard->flags() & QGraphicsItem::ItemIsSelectable)
        targets << dashboard;

    for (int i = 0; i < targets.length(); i++) {
        if (targets.at(i)->isSelected()) {
            for (int j = i + 1; j < targets.length(); j++) {
                if (!targets.at(j)->isSelected()) {
                    targets.at(j)->setSelected(true);
                    return;
                }
            }
        }
    }

    foreach (QGraphicsItem *target, targets) {
        if (!target->isSelected()) {
            target->setSelected(true);
            break;
        }
    }
}

void RoomScene::unselectAllTargets(const QGraphicsItem *except) {
    if (dashboard != except) dashboard->setSelected(false);

    foreach (Photo *photo, photos) {
        if (photo != except && photo->isSelected())
            photo->setSelected(false);
    }
}

void RoomScene::doTimeout() {
    switch (ClientInstance->getStatus() & Client::ClientStatusBasicMask) {
    case Client::Playing: {
            discard_button->click();
            break;
        }
    case Client::Responding:
    case Client::Discarding:
    case Client::Exchanging:
    case Client::ExecDialog:
    case Client::AskForShowOrPindian: {
            doCancelButton();
            break;
        }
    case Client::AskForPlayerChoose: {
            ClientInstance->onPlayerChoosePlayer(NULL);
            dashboard->stopPending();
            prompt_box->disappear();
            break;
        }
    case Client::AskForAG: {
            int card_id = card_container->getFirstEnabled();
            if (card_id != -1)
                ClientInstance->onPlayerChooseAG(card_id);
            break;
        }
    case Client::AskForSkillInvoke: {
            cancel_button->click();
            break;
        }
    case Client::AskForYiji: {
            if (cancel_button->isEnabled())
                cancel_button->click();
            else {
                prompt_box->disappear();
                doCancelButton();
            }
            break;
        }
    case Client::AskForGuanxing:
    case Client::AskForGongxin: {
            ok_button->click();
            break;
        }
    case Client::AskForGeneralTaken: {
            break;
        }
    case Client::AskForArrangement: {
            arrange_items << down_generals.mid(0, 3 - arrange_items.length());
            finishArrange();
        }
    default:
            break;
    }
}

void RoomScene::showPromptBox() {
    bringToFront(prompt_box);
    prompt_box->appear();
}

void RoomScene::updateStatus(Client::Status oldStatus, Client::Status newStatus) {
    foreach (QSanSkillButton *button, m_skillButtons) {
        Q_ASSERT(button != NULL);
        const ViewAsSkill *vsSkill = button->getViewAsSkill();
        if (vsSkill != NULL) {
            QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
            QRegExp rx("@@?([_A-Za-z]+)(\\d+)?!?");
            CardUseStruct::CardUseReason reason = CardUseStruct::CARD_USE_REASON_UNKNOWN;
            if ((newStatus & Client::ClientStatusBasicMask) == Client::Responding) {
                if (newStatus == Client::RespondingUse)
                    reason = CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
                else if (newStatus == Client::Responding || rx.exactMatch(pattern))
                    reason = CardUseStruct::CARD_USE_REASON_RESPONSE;
            } else if (newStatus == Client::Playing)
                reason = CardUseStruct::CARD_USE_REASON_PLAY;
            button->setEnabled(vsSkill->isAvailable(Self, reason, pattern) && !pattern.endsWith("!"));
        } else {
            const Skill *skill = button->getSkill();
            if (skill->getFrequency() == Skill::Wake)
                button->setEnabled(Self->getMark(skill->objectName()) > 0);
            else
                button->setEnabled(false);
        }
    }

    switch (newStatus & Client::ClientStatusBasicMask) {
    case Client::NotActive: {
            if (oldStatus == Client::ExecDialog) {
                if (m_choiceDialog != NULL && m_choiceDialog->isVisible()) {
                    m_choiceDialog->hide();
                }
            } else if (oldStatus == Client::AskForGuanxing || oldStatus == Client::AskForGongxin) {
                guanxing_box->clear();
                if (!card_container->retained())
                    card_container->clear();
            }
            prompt_box->disappear();
            ClientInstance->getPromptDoc()->clear();

            dashboard->disableAllCards();
            selected_targets.clear();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            if (dashboard->currentSkill())
                dashboard->stopPending();

            dashboard->hideProgressBar();

            break;
        }
    case Client::Responding: {
            showPromptBox();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            discard_button->setEnabled(false);

            QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
            QRegExp rx("@@?([_A-Za-z]+)(\\d+)?!?");
            if (rx.exactMatch(pattern)) {
                QString skill_name = rx.capturedTexts().at(1);
                const ViewAsSkill *skill = Sanguosha->getViewAsSkill(skill_name);
                if (skill) {
                    CardUseStruct::CardUseReason reason = CardUseStruct::CARD_USE_REASON_RESPONSE;
                    if (newStatus == Client::RespondingUse)
                        reason = CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
                    if (!Self->hasFlag(skill_name))
                        Self->setFlags(skill_name);
                    bool available = skill->isAvailable(Self, reason, pattern);
                    Self->setFlags("-" + skill_name);
                    if (!available) {
                        ClientInstance->onPlayerResponseCard(NULL);
                        return;
                    }
                    if (Self->hasSkill(skill_name, true)) {
                        foreach (QSanSkillButton *button, m_skillButtons) {
                            Q_ASSERT(button != NULL);
                            const ViewAsSkill *vsSkill = button->getViewAsSkill();
                            if (vsSkill != NULL && vsSkill->objectName() == skill_name
                                && vsSkill->isAvailable(Self, reason, pattern))
                                button->click();
                                break;
                        }
                    }
                    dashboard->startPending(skill);
                    if (skill->inherits("OneCardViewAsSkill") && Config.EnableIntellectualSelection)
                        dashboard->selectOnlyCard();
                }
            } else {
                if (pattern.endsWith("!"))
                    pattern = pattern.mid(0, pattern.length() - 1);
                response_skill->setPattern(pattern);
                if (newStatus == Client::RespondingForDiscard)
                    response_skill->setRequest(Card::MethodDiscard);
                else if (newStatus == Client::RespondingNonTrigger)
                    response_skill->setRequest(Card::MethodNone);
                else if (newStatus == Client::RespondingUse)
                    response_skill->setRequest(Card::MethodUse);
                else
                    response_skill->setRequest(Card::MethodResponse);
                dashboard->startPending(response_skill);
                if (Config.EnableIntellectualSelection)
                    dashboard->selectOnlyCard();
            }
            break;
        }
    case Client::AskForShowOrPindian: {
            showPromptBox();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
            showorpindian_skill->setPattern(pattern);
            dashboard->startPending(showorpindian_skill);

            break;
        }
    case Client::Playing: {
            dashboard->enableCards();
            bringToFront(dashboard);
            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(true);
            break;
        }
    case Client::Discarding:
    case Client::Exchanging: {
            showPromptBox();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            discard_button->setEnabled(false);

            discard_skill->setNum(ClientInstance->discard_num);
            discard_skill->setMinNum(ClientInstance->min_num);
            discard_skill->setIncludeEquip(ClientInstance->m_canDiscardEquip);
            discard_skill->setIsDiscard(newStatus != Client::Exchanging);
            dashboard->startPending(discard_skill);
            break;
        }
    case Client::ExecDialog: {
            if (m_choiceDialog != NULL) {
                m_choiceDialog->setParent(main_window, Qt::Dialog);
                m_choiceDialog->show();
                ok_button->setEnabled(false);
                cancel_button->setEnabled(true);
                discard_button->setEnabled(false);
            }
            break;
        }
    case Client::AskForSkillInvoke: {
            QString skill_name = ClientInstance->getSkillNameToInvoke();
            dashboard->highlightEquip(skill_name, true);
            foreach (QSanSkillButton *button, m_skillButtons) {
                if (button->getSkill()->objectName() == skill_name) {
                    if (button->getStyle() == QSanSkillButton::S_STYLE_TOGGLE
                        && button->isEnabled() && button->isDown()) {
                        ClientInstance->onPlayerInvokeSkill(true);
                        return;
                    }
                }
            }

            showPromptBox();
            ok_button->setEnabled(true);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);
            break;
        }
    case Client::AskForPlayerChoose: {
            showPromptBox();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            discard_button->setEnabled(false);

            choose_skill->setPlayerNames(ClientInstance->players_to_choose);
            dashboard->startPending(choose_skill);

            break;
        }
    case Client::AskForAG: {
            dashboard->disableAllCards();

            ok_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            card_container->startChoose();

            break;
        }
    case Client::AskForYiji: {
            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            discard_button->setEnabled(false);

            QStringList yiji_info = Sanguosha->currentRoomState()->getCurrentCardUsePattern().split("=");
            yiji_skill->setCards(yiji_info.at(1));
            yiji_skill->setMaxNum(yiji_info.first().toInt());
            yiji_skill->setPlayerNames(yiji_info.last().split("+"));
            dashboard->startPending(yiji_skill);

            showPromptBox();

            break;
        }
    case Client::AskForGuanxing: {
            ok_button->setEnabled(true);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }
    case Client::AskForGongxin: {
            ok_button->setEnabled(true);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }
    case Client::AskForGeneralTaken:
    case Client::AskForArrangement: {
            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }
    }
    if (newStatus != oldStatus && newStatus != Client::Playing && newStatus != Client::NotActive)
        QApplication::alert(QApplication::focusWidget());

    if (ServerInfo.OperationTimeout == 0)
        return;

    // do timeout
    if (newStatus != Client::NotActive && newStatus != oldStatus) {
        QApplication::alert(main_window);
        connect(dashboard, SIGNAL(progressBarTimedOut()), this, SLOT(doTimeout()));
        dashboard->showProgressBar(ClientInstance->getCountdown());
    }
}

void RoomScene::onSkillDeactivated() {
    const ViewAsSkill *current = dashboard->currentSkill();
    if (current) cancel_button->click();
}

void RoomScene::onSkillActivated() {
    QSanSkillButton *button = qobject_cast<QSanSkillButton *>(sender());
    const ViewAsSkill *skill = button->getViewAsSkill();

    if (skill) {
        dashboard->startPending(skill);
        //ok_button->setEnabled(false);
        cancel_button->setEnabled(true);

        const Card *card = dashboard->pendingCard();
        if (card && card->targetFixed() && card->isAvailable(Self)) {
            useSelectedCard();
        } else if (skill->inherits("OneCardViewAsSkill") && Config.EnableIntellectualSelection)
            dashboard->selectOnlyCard(ClientInstance->getStatus() == Client::Playing);
    }
}

void RoomScene::updateTrustButton() {
    if (!ClientInstance->getReplayer()) {
        bool trusting = Self->getState() == "trust";
        trust_button->update();
        dashboard->setTrust(trusting);
    }
}

void RoomScene::doOkButton() {
    if (!ok_button->isEnabled()) return;
    if (card_container->retained()) card_container->clear();
    useSelectedCard();
}

void RoomScene::doCancelButton() {
    if (card_container->retained()) card_container->clear();
    switch (ClientInstance->getStatus() & Client::ClientStatusBasicMask) {
    case Client::Playing: {
            dashboard->skillButtonDeactivated();
            const ViewAsSkill *skill = dashboard->currentSkill();
            dashboard->unselectAll();
            if (skill)
                cancelViewAsSkill();
            else
                dashboard->stopPending();
            dashboard->enableCards();
            break;
        }
    case Client::Responding: {
            dashboard->skillButtonDeactivated();
            QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
            if (pattern.isEmpty()) return;

            dashboard->unselectAll();

            if (!pattern.startsWith("@")) {
                const ViewAsSkill *skill = dashboard->currentSkill();
                if (!skill->inherits("ResponseSkill")) {
                    cancelViewAsSkill();
                    break;
                }
            }

            ClientInstance->onPlayerResponseCard(NULL);
            prompt_box->disappear();
            dashboard->stopPending();
            break;
        }
    case Client::AskForShowOrPindian: {
            dashboard->unselectAll();
            ClientInstance->onPlayerResponseCard(NULL);
            prompt_box->disappear();
            dashboard->stopPending();
            break;
        }
    case Client::Discarding:
    case Client::Exchanging: {
            dashboard->unselectAll();
            dashboard->stopPending();
            ClientInstance->onPlayerDiscardCards(NULL);
            prompt_box->disappear();
            break;
        }
    case Client::ExecDialog: {
            m_choiceDialog->reject();
            break;
        }
    case Client::AskForSkillInvoke: {
            QString skill_name = ClientInstance->getSkillNameToInvoke();
            dashboard->highlightEquip(skill_name, false);
            ClientInstance->onPlayerInvokeSkill(false);
            prompt_box->disappear();
            break;
        }
    case Client::AskForYiji: {
            dashboard->stopPending();
            ClientInstance->onPlayerReplyYiji(NULL, NULL);
            prompt_box->disappear();
            break;
        }
    case Client::AskForPlayerChoose: {
            dashboard->stopPending();
            ClientInstance->onPlayerChoosePlayer(NULL);
            prompt_box->disappear();
            break;
        }
    default:
            break;
    }
}

void RoomScene::doDiscardButton() {
    dashboard->stopPending();
    dashboard->unselectAll();

    if (card_container->retained()) card_container->clear();
    if (ClientInstance->getStatus() == Client::Playing)
        ClientInstance->onPlayerResponseCard(NULL);
}

void RoomScene::hideAvatars() {
    if (control_panel) control_panel->hide();
}

void RoomScene::startInXs() {
    if (add_robot) add_robot->hide();
    if (fill_robots) fill_robots->hide();
}

void RoomScene::changeHp(const QString &who, int delta, DamageStruct::Nature nature, bool losthp) {
    // update
    Photo *photo = name2photo.value(who, NULL);
    if (photo)
        photo->updateHp();
    else
        dashboard->update();

    if (delta < 0) {
        if (losthp) {
            Sanguosha->playSystemAudioEffect("hplost");
            QString from_general = ClientInstance->getPlayer(who)->objectName();
            log_box->appendLog("#GetHp", from_general, QStringList(), QString(),
                               QString::number(ClientInstance->getPlayer(who)->getHp()), QString::number(ClientInstance->getPlayer(who)->getMaxHp()));
            return;
        }

        QString damage_effect;
        QString from_general = ClientInstance->getPlayer(who)->objectName();
        log_box->appendLog("#GetHp", from_general, QStringList(), QString(),
                           QString::number(ClientInstance->getPlayer(who)->getHp()), QString::number(ClientInstance->getPlayer(who)->getMaxHp()));
        switch (delta) {
        case -1: damage_effect = "injure1"; break;
        case -2: damage_effect = "injure2"; break;
        case -3:
        default: damage_effect = "injure3"; break;
        }

        Sanguosha->playSystemAudioEffect(damage_effect);

        if (photo) {
            setEmotion(who, "damage");
            photo->tremble();
        }

        if (nature == DamageStruct::Fire)
            doAnimation(S_ANIMATE_FIRE, QStringList() << who);
        else if (nature == DamageStruct::Thunder)
            doAnimation(S_ANIMATE_LIGHTNING, QStringList() << who);
    } else {
        QString type = "#Recover";
        QString from_general = ClientInstance->getPlayer(who)->objectName();
        QString n = QString::number(delta);

        log_box->appendLog(type, from_general, QStringList(), QString(), n);
        log_box->appendLog("#GetHp", from_general, QStringList(), QString(),
                           QString::number(ClientInstance->getPlayer(who)->getHp()), QString::number(ClientInstance->getPlayer(who)->getMaxHp()));
    }
}

void RoomScene::changeMaxHp(const QString &who, int delta) {
    if (delta < 0)
        Sanguosha->playSystemAudioEffect("maxhplost");
}

void RoomScene::onStandoff() {
    log_box->append(QString(tr("<font color='%1'>---------- Game Finish ----------</font>").arg(Config.TextEditColor.name())));

    freeze();
    Sanguosha->playSystemAudioEffect("standoff");

    QDialog *dialog = new QDialog(main_window);
    dialog->resize(500, 600);
    dialog->setWindowTitle(tr("Standoff"));

    QVBoxLayout *layout = new QVBoxLayout;

    QTableWidget *table = new QTableWidget;
    fillTable(table, ClientInstance->getPlayers());

    layout->addWidget(table);
    dialog->setLayout(layout);

    addRestartButton(dialog);

    dialog->exec();
}

void RoomScene::onGameOver() {
    log_box->append(QString(tr("<font color='%1'>---------- Game Finish ----------</font>").arg(Config.TextEditColor.name())));

    m_roomMutex.lock();
    freeze();

    bool victory = Self->property("win").toBool();
#ifdef AUDIO_SUPPORT
    QString win_effect;
    if (victory) {
        win_effect = "win";
        foreach (const Player *player, ClientInstance->getPlayers()) {
            if (player->property("win").toBool() && player->getGeneralName().contains("caocao")) {
                Audio::stop();
                win_effect = "win-cc";
                break;
            }
        }
    } else
        win_effect = "lose";

    Sanguosha->playSystemAudioEffect(win_effect);
#endif
    QDialog *dialog = new QDialog(main_window);
    dialog->resize(800, 600);
    dialog->setWindowTitle(victory ? tr("Victory") : tr("Failure"));

    QGroupBox *winner_box = new QGroupBox(tr("Winner(s)"));
    QGroupBox *loser_box = new QGroupBox(tr("Loser(s)"));

    QTableWidget *winner_table = new QTableWidget;
    QTableWidget *loser_table = new QTableWidget;

    QVBoxLayout *winner_layout = new QVBoxLayout;
    winner_layout->addWidget(winner_table);
    winner_box->setLayout(winner_layout);

    QVBoxLayout *loser_layout = new QVBoxLayout;
    loser_layout->addWidget(loser_table);
    loser_box->setLayout(loser_layout);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(winner_box);
    layout->addWidget(loser_box);
    dialog->setLayout(layout);

    QList<const ClientPlayer *> winner_list, loser_list;
    foreach (const ClientPlayer *player, ClientInstance->getPlayers()) {
        bool win = player->property("win").toBool();
        if (win)
            winner_list << player;
        else
            loser_list << player;
    }

    fillTable(winner_table, winner_list);
    fillTable(loser_table, loser_list);

    addRestartButton(dialog);
    m_roomMutex.unlock();
    dialog->exec();
}

void RoomScene::addRestartButton(QDialog *dialog) {
    dialog->resize(main_window->width() / 2, dialog->height());
    bool goto_next = false;
    if (ServerInfo.GameMode.contains("_mini_") && Self->property("win").toBool())
        goto_next = (_m_currentStage < Sanguosha->getMiniSceneCounts());

    QPushButton *restart_button;
    restart_button = new QPushButton(goto_next ? tr("Next Stage") : tr("Restart Game"));
    QPushButton *return_button = new QPushButton(tr("Return to main menu"));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(restart_button);

    QPushButton *save_button = new QPushButton(tr("Save record"));
    hlayout->addWidget(save_button);
    hlayout->addWidget(return_button);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(dialog->layout());
    if (layout) layout->addLayout(hlayout);

    connect(restart_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(return_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(saveReplayRecord()));
    connect(dialog, SIGNAL(accepted()), this, SIGNAL(restart()));
    connect(return_button, SIGNAL(clicked()), this, SIGNAL(return_to_start()));
}

void RoomScene::saveReplayRecord() {
    QString location = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    QString filename = QFileDialog::getSaveFileName(main_window,
                                                    tr("Save replay record"),
                                                    location,
                                                    tr("Pure text replay file (*.txt);; Image replay file (*.png)"));

    if (!filename.isEmpty()) ClientInstance->save(filename);
}

ScriptExecutor::ScriptExecutor(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Script execution"));
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(new QLabel(tr("Please input the script that should be executed at server side:\n P = you, R = your room")));

    QTextEdit *box = new QTextEdit;
    box->setObjectName("scriptBox");
    vlayout->addWidget(box);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();

    QPushButton *ok_button = new QPushButton(tr("OK"));
    hlayout->addWidget(ok_button);

    vlayout->addLayout(hlayout);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(this, SIGNAL(accepted()), this, SLOT(doScript()));

    setLayout(vlayout);
}

void ScriptExecutor::doScript() {
    QTextEdit *box = findChild<QTextEdit *>("scriptBox");
    if (box == NULL) return;

    QString script = box->toPlainText();
    QByteArray data = script.toAscii();
    data = qCompress(data);
    script = data.toBase64();

    ClientInstance->requestCheatRunScript(script);
}

DeathNoteDialog::DeathNoteDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Death note"));

    killer = new QComboBox;
    RoomScene::FillPlayerNames(killer, true);

    victim = new QComboBox;
    RoomScene::FillPlayerNames(victim, false);

    QPushButton *ok_button = new QPushButton(tr("OK"));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));

    QFormLayout *layout = new QFormLayout;
    layout->addRow(tr("Killer"), killer);
    layout->addRow(tr("Victim"), victim);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(ok_button);
    layout->addRow(hlayout);

    setLayout(layout);
}

void DeathNoteDialog::accept() {
    QDialog::accept();
    ClientInstance->requestCheatKill(killer->itemData(killer->currentIndex()).toString(),
                                     victim->itemData(victim->currentIndex()).toString());
}

DamageMakerDialog::DamageMakerDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Damage maker"));

    damage_source = new QComboBox;
    RoomScene::FillPlayerNames(damage_source, true);

    damage_target = new QComboBox;
    RoomScene::FillPlayerNames(damage_target, false);

    damage_nature = new QComboBox;
    damage_nature->addItem(tr("Normal"), S_CHEAT_NORMAL_DAMAGE);
    damage_nature->addItem(tr("Thunder"), S_CHEAT_THUNDER_DAMAGE);
    damage_nature->addItem(tr("Fire"), S_CHEAT_FIRE_DAMAGE);
    damage_nature->addItem(tr("Recover HP"), S_CHEAT_HP_RECOVER);
    damage_nature->addItem(tr("Lose HP"), S_CHEAT_HP_LOSE);
    damage_nature->addItem(tr("Lose Max HP"), S_CHEAT_MAX_HP_LOSE);
    damage_nature->addItem(tr("Reset Max HP"), S_CHEAT_MAX_HP_RESET);

    damage_point = new QSpinBox;
    damage_point->setRange(1, 1000);
    damage_point->setValue(1);

    QPushButton *ok_button = new QPushButton(tr("OK"));
    connect(ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(ok_button);

    QFormLayout *layout = new QFormLayout;

    layout->addRow(tr("Damage source"), damage_source);
    layout->addRow(tr("Damage target"), damage_target);
    layout->addRow(tr("Damage nature"), damage_nature);
    layout->addRow(tr("Damage point"), damage_point);
    layout->addRow(hlayout);

    setLayout(layout);

    connect(damage_nature, SIGNAL(currentIndexChanged(int)), this, SLOT(disableSource()));
}

void DamageMakerDialog::disableSource() {
    QString nature = damage_nature->itemData(damage_nature->currentIndex()).toString();
    damage_source->setEnabled(nature != "L");
}

void RoomScene::FillPlayerNames(QComboBox *ComboBox, bool add_none) {
    if (add_none) ComboBox->addItem(tr("None"), ".");
    ComboBox->setIconSize(G_COMMON_LAYOUT.m_tinyAvatarSize);
    foreach (const ClientPlayer *player, ClientInstance->getPlayers()) {
        QString general_name = Sanguosha->translate(player->getGeneralName());
        if (!player->getGeneral()) continue;
        QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(player->getGeneralName(), QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY);
        ComboBox->addItem(QIcon(pixmap),
                          QString("%1 [%2]").arg(general_name).arg(player->screenName()),
                          player->objectName());
    }
}

void DamageMakerDialog::accept() {
    QDialog::accept();

    ClientInstance->requestCheatDamage(damage_source->itemData(damage_source->currentIndex()).toString(),
                                       damage_target->itemData(damage_target->currentIndex()).toString(),
                                       (DamageStruct::Nature)damage_nature->itemData(damage_nature->currentIndex()).toInt(),
                                       damage_point->value());
}

void RoomScene::makeDamage() {
    if (Self->getPhase() != Player::Play) {
        QMessageBox::warning(main_window, tr("Warning"), tr("This function is only allowed at your play phase!"));
        return;
    }

    DamageMakerDialog *damage_maker = new DamageMakerDialog(main_window);
    damage_maker->exec();
}

void RoomScene::makeKilling() {
    if (Self->getPhase() != Player::Play) {
        QMessageBox::warning(main_window, tr("Warning"), tr("This function is only allowed at your play phase!"));
        return;
    }

    DeathNoteDialog *dialog = new DeathNoteDialog(main_window);
    dialog->exec();
}

void RoomScene::makeReviving() {
    if (Self->getPhase() != Player::Play) {
        QMessageBox::warning(main_window, tr("Warning"), tr("This function is only allowed at your play phase!"));
        return;
    }

    QStringList items;
    QList<const ClientPlayer *> victims;
    foreach (const ClientPlayer *player, ClientInstance->getPlayers()) {
        if (player->isDead()) {
            QString general_name = Sanguosha->translate(player->getGeneralName());
            items << QString("%1 [%2]").arg(player->screenName()).arg(general_name);
            victims << player;
        }
    }

    if (items.isEmpty()) {
        QMessageBox::warning(main_window, tr("Warning"), tr("No victims now!"));
        return;
    }

    bool ok;
    QString item = QInputDialog::getItem(main_window, tr("Reviving wand"),
                                         tr("Please select a player to revive"), items, 0, false, &ok);
    if (ok) {
        int index = items.indexOf(item);
        ClientInstance->requestCheatRevive(victims.at(index)->objectName());
    }
}

void RoomScene::doScript() {
    ScriptExecutor *dialog = new ScriptExecutor(main_window);
    dialog->exec();
}

void RoomScene::fillTable(QTableWidget *table, const QList<const ClientPlayer *> &players) {
    table->setColumnCount(10);
    table->setRowCount(players.length());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    RecAnalysis *record = new RecAnalysis(ClientInstance->getReplayPath());
    QMap<QString, PlayerRecordStruct *> record_map = record->getRecordMap();

    static QStringList labels;
    if (labels.isEmpty()) {
        labels << tr("General") << tr("Name") << tr("Alive");
        if (ServerInfo.EnableHegemony)
            labels << tr("Nationality");
        else
            labels << tr("Role");

        labels << tr("TurnCount");
        labels << tr("Recover") << tr("Damage") << tr("Damaged") << tr("Kill") << tr("Designation");
        labels << tr("Handcards");

    }
    table->setHorizontalHeaderLabels(labels);
    table->setSelectionBehavior(QTableWidget::SelectRows);

    for (int i = 0; i < players.length(); i++) {
        const ClientPlayer *player = players[i];

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(ClientInstance->getPlayerName(player->objectName()));
        table->setItem(i, 0, item);

        item = new QTableWidgetItem;
        item->setText(player->screenName());
        table->setItem(i, 1, item);

        item = new QTableWidgetItem;
        if (player->isAlive())
            item->setText(tr("Alive"));
        else
            item->setText(tr("Dead"));
        table->setItem(i, 2, item);

        item = new QTableWidgetItem;

        if (ServerInfo.EnableHegemony) {
            QIcon icon(QString("image/kingdom/icon/%1.png").arg(player->getKingdom()));
            item->setIcon(icon);
            item->setText(Sanguosha->translate(player->getKingdom()));
        } else {
            QIcon icon(QString("image/system/roles/%1.png").arg(player->getRole()));
            item->setIcon(icon);
            QString role = player->getRole();
            if (ServerInfo.GameMode.startsWith("06_")) {
                if (role == "lord" || role == "renegade")
                    role = "leader";
                else
                    role = "guard";
            } else if (ServerInfo.GameMode == "04_1v3") {
                int seat = player->getSeat();
                switch (seat) {
                case 1: role = "lvbu"; break;
                case 2: role = "vanguard"; break;
                case 3: role = "mainstay"; break;
                case 4: role = "general"; break;
                }
            } else if (ServerInfo.GameMode == "02_1v1") {
                if (role == "lord")
                    role = "defensive";
                else
                    role = "offensive";
            }
            item->setText(Sanguosha->translate(role));
        }
        if (!player->isAlive())
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        table->setItem(i, 3, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(player->getMark("Global_TurnCount")));
        table->setItem(i, 4, item);

        PlayerRecordStruct *rec = record_map.value(player->objectName());
        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_recover));
        table->setItem(i, 5, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_damage));
        table->setItem(i, 6, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_damaged));
        table->setItem(i, 7, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(rec->m_kill));
        table->setItem(i, 8, item);

        item = new QTableWidgetItem;
        item->setText(rec->m_designation.join(", "));
        table->setItem(i, 9, item);

        item = new QTableWidgetItem;
        QString handcards = QString::fromUtf8(QByteArray::fromBase64(player->property("last_handcards").toString().toAscii()));
        handcards.replace("<img src='image/system/log/spade.png' height = 12/>", tr("Spade"));
        handcards.replace("<img src='image/system/log/heart.png' height = 12/>", tr("Heart"));
        handcards.replace("<img src='image/system/log/club.png' height = 12/>", tr("Club"));
        handcards.replace("<img src='image/system/log/diamond.png' height = 12/>", tr("Diamond"));
        item->setText(handcards);
        table->setItem(i, 10, item);
    }

    for (int i = 2; i <= 10; i++)
        table->resizeColumnToContents(i);
}

void RoomScene::killPlayer(const QString &who) {
    const General *general = NULL;
    m_roomMutex.lock();
    if (who == Self->objectName()) {
        dashboard->killPlayer();
        dashboard->update();
        general = Self->getGeneral();
        item2player.remove(dashboard);
        if (ServerInfo.GameMode == "02_1v1") self_box->killPlayer(general->objectName());
    } else {
        Photo *photo = name2photo[who];
        photo->killPlayer();
        photo->setFrame(Photo::S_FRAME_NO_FRAME);
        photo->update();
        item2player.remove(photo);
        general = photo->getPlayer()->getGeneral();
        if (ServerInfo.GameMode == "02_1v1") enemy_box->killPlayer(general->objectName());
    }

    ClientPlayer *player = ClientInstance->getPlayer(who);
    if (player) {
        PlayerCardContainer *container = (PlayerCardContainer *)_getGenericCardContainer(Player::PlaceHand, player);
        container->stopHuaShen();
    }

    if (Config.EnableEffects && Config.EnableLastWord && !Self->hasFlag("marshalling"))
        general->lastWord();
    m_roomMutex.unlock();
}

void RoomScene::revivePlayer(const QString &who) {
    if (who == Self->objectName()) {
        dashboard->revivePlayer();
        item2player.insert(dashboard, Self);
        updateSkillButtons();
    } else {
        Photo *photo = name2photo[who];
        photo->revivePlayer();
        item2player.insert(photo, photo->getPlayer());
    }
}

void RoomScene::takeAmazingGrace(ClientPlayer *taker, int card_id, bool move_cards) {
    QList<int> card_ids;
    card_ids.append(card_id);
    m_tablePile->clear();

    card_container->m_currentPlayer = taker;
    CardItem *copy = card_container->removeCardItems(card_ids, Player::PlaceHand).first();
    if (copy == NULL) return;

    QList<CardItem *> items;
    items << copy;

    if (taker) {
        GenericCardContainer *container = _getGenericCardContainer(Player::PlaceHand, taker);
        bringToFront(container);
        if (move_cards) {
            QString type = "$TakeAG";
            QString from_general = taker->objectName();
            QString card_str = QString::number(card_id);
            log_box->appendLog(type, from_general, QStringList(), card_str);
            CardsMoveStruct move;
            move.card_ids.append(card_id);
            move.from_place = Player::PlaceWuGu;
            move.to_place = Player::PlaceHand;
            move.to = taker;
            container->addCardItems(items, move);
        } else {
            delete copy;
        }
    } else
        delete copy;
}

void RoomScene::showCard(const QString &player_name, int card_id) {
    QList<int> card_ids;
    card_ids << card_id;
    const ClientPlayer *player = ClientInstance->getPlayer(player_name);

    GenericCardContainer *container = _getGenericCardContainer(Player::PlaceHand, (Player *)player);
    QList<CardItem *> card_items = container->cloneCardItems(card_ids);
    CardMoveReason reason(CardMoveReason::S_REASON_DEMONSTRATE, player->objectName());
    bringToFront(m_tablePile);
    CardsMoveStruct move;
    move.from_place = Player::PlaceHand;
    move.to_place = Player::PlaceTable;
    move.reason = reason;
    card_items[0]->setFootnote(_translateMovement(move));
    m_tablePile->addCardItems(card_items, move);

    QString card_str = QString::number(card_id);
    log_box->appendLog("$ShowCard", player->objectName(), QStringList(), card_str);
}

void RoomScene::chooseSkillButton() {
    QList<QSanSkillButton *> enabled_buttons;
    foreach (QSanSkillButton *btn, m_skillButtons) {
        if (btn->isEnabled())
            enabled_buttons << btn;
    }

    if (enabled_buttons.isEmpty()) return;

    QDialog *dialog = new QDialog(main_window);
    dialog->setWindowTitle(tr("Select skill"));

    QVBoxLayout *layout = new QVBoxLayout;

    foreach (QSanSkillButton *btn, enabled_buttons) {
        Q_ASSERT(btn->getSkill());
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(btn->getSkill()->objectName()));
        connect(button, SIGNAL(clicked()), btn, SLOT(click()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
        layout->addWidget(button);
    }

    dialog->setLayout(layout);
    dialog->exec();
}

void RoomScene::attachSkill(const QString &skill_name, bool from_left) {
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill)
        addSkillButton(skill, from_left);
}

void RoomScene::detachSkill(const QString &skill_name) {
    QSanSkillButton *btn = dashboard->removeSkillButton(skill_name);
    if (btn == NULL) return;    //be care LordSkill and SPConvertSkill
    m_skillButtons.removeAll(btn);
    btn->deleteLater();
}

void RoomScene::viewDistance() {
    DistanceViewDialog *dialog = new DistanceViewDialog(main_window);
    dialog->show();
}

void RoomScene::speak() {
    if (game_started && ServerInfo.DisableChat)
        chat_box->append(tr("This room does not allow chatting!"));
    else {
        bool broadcast = true;
        QString text = chat_edit->text();
        if (text == ".StartBgMusic") {
            broadcast = false;
            Config.EnableBgMusic = true;
            Config.setValue("EnableBgMusic", true);
#ifdef AUDIO_SUPPORT
            Audio::stopBGM();
            QString bgmusic_path = Config.value("BackgroundMusic", "audio/system/background.ogg").toString();
            Audio::playBGM(bgmusic_path);
            Audio::setBGMVolume(Config.BGMVolume);
#endif
        } else if (text.startsWith(".StartBgMusic=")) {
            broadcast = false;
            Config.EnableBgMusic = true;
            Config.setValue("EnableBgMusic", true);
            QString path = text.mid(14);
            if (path.startsWith("|")) {
                path = path.mid(1);
                Config.setValue("BackgroundMusic", path);
            }
#ifdef AUDIO_SUPPORT
            Audio::stopBGM();
            Audio::playBGM(path);
            Audio::setBGMVolume(Config.BGMVolume);
#endif
        } else if (text == ".StopBgMusic") {
            broadcast = false;
            Config.EnableBgMusic = false;
            Config.setValue("EnableBgMusic", false);
#ifdef AUDIO_SUPPORT
            Audio::stopBGM();
#endif
        }
        if (broadcast)
            ClientInstance->speakToServer(text);
        else {
            QString title;
            if (Self) {
                title = Self->getGeneralName();
                title = Sanguosha->translate(title);
                title.append(QString("(%1)").arg(Self->screenName()));
                title = QString("<b>%1</b>").arg(title);
            }
            QString line = tr("<font color='%1'>[%2] said: %3 </font>")
                           .arg(Config.TextEditColor.name()).arg(title).arg(text);
            appendChatBox(QString("<p style=\"margin:3px 2px;\">%1</p>").arg(line));
        }
    }
    chat_edit->clear();
}

void RoomScene::fillCards(const QList<int> &card_ids, const QList<int> &disabled_ids) {
    bringToFront(card_container);
    card_container->fillCards(card_ids, disabled_ids);
    card_container->show();
}

void RoomScene::doGongxin(const QList<int> &card_ids, bool enable_heart) {
    fillCards(card_ids);
    if (enable_heart)
        card_container->startGongxin();
    else
        card_container->addCloseButton();
}

void RoomScene::showOwnerButtons(bool owner) {
    if (control_panel && !game_started)
        control_panel->setVisible(owner);
}

void RoomScene::showPlayerCards() {
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QStringList names = action->data().toString().split(".");
        QString player_name = names.first();
        const ClientPlayer *player = ClientInstance->getPlayer(player_name);
        if (names.length() > 1) {
            QString pile_name = names.last();
            QList<const Card *> cards;
            foreach (int id, player->getPile(pile_name)) {
                const Card *card = Sanguosha->getEngineCard(id);
                if (card) cards << card;
            }

            CardOverview *overview = new CardOverview;
            overview->setWindowTitle(QString("%1 %2")
                                     .arg(ClientInstance->getPlayerName(player_name))
                                     .arg(Sanguosha->translate(pile_name)));
            overview->loadFromList(cards);
            overview->show();
        } else {
            QList<const Card *> cards;
            foreach (const Card *card, player->getHandcards()) {
                const Card *engine_card = Sanguosha->getEngineCard(card->getId());
                if (engine_card) cards << engine_card;
            }

            CardOverview *overview = new CardOverview;
            overview->setWindowTitle(QString("%1 %2")
                                     .arg(ClientInstance->getPlayerName(player_name))
                                     .arg(tr("Known cards")));
            overview->loadFromList(cards);
            overview->show();
        }
    }
}

KOFOrderBox::KOFOrderBox(bool self, QGraphicsScene *scene) {
    QString basename = self ? "self" : "enemy";
    QString path = QString("image/system/1v1/%1.png").arg(basename);
    setPixmap(QPixmap(path));
    scene->addItem(this);

    for (int i = 0; i < 3; i++) {
        avatars[i] = new QSanSelectableItem;
        avatars[i]->load("image/system/1v1/unknown.png", QSize(122, 50));
        avatars[i]->setParentItem(this);
        avatars[i]->setPos(5, 23 + 62 *i);
        avatars[i]->setObjectName("unknown");
    }

    revealed = 0;
}

void KOFOrderBox::revealGeneral(const QString &name) {
    if (revealed < 3) {
        avatars[revealed]->setPixmap(G_ROOM_SKIN.getGeneralPixmap(name, QSanRoomSkin::S_GENERAL_ICON_SIZE_KOF));
        avatars[revealed]->setObjectName(name);
        const General *general = Sanguosha->getGeneral(name);
        if (general)
            avatars[revealed]->setToolTip(general->getSkillDescription(true));
        revealed++;
    }
}

void KOFOrderBox::killPlayer(const QString &general_name) {
    for (int i = 0; i < revealed; i++) {
        QSanSelectableItem *avatar = avatars[i];
        if (avatar->isEnabled() && avatar->objectName() == general_name) {
            QPixmap pixmap("image/system/death/unknown.png");
            QGraphicsPixmapItem *death = new QGraphicsPixmapItem(pixmap, avatar);
            death->setScale(0.5);
            death->moveBy(15, 0);
            avatar->makeGray();
            avatar->setEnabled(false);

            return;
        }
    }
}

void RoomScene::onGameStart() {
    main_window->activateWindow();
    if (Config.GameMode.contains("_mini_")) {
        QString id = Config.GameMode;
        id.replace("_mini_", "");
        _m_currentStage = id.toInt();
    } else if (ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "06_XMode" || ServerInfo.GameMode == "02_1v1") {
        chat_widget->show();
        log_box->show();

        if (self_box && enemy_box) {
            self_box->show();
            enemy_box->show();
        }
    }

    if (control_panel)
        control_panel->hide();

    log_box->append(QString(tr("<font color='%1'>---------- Game Start ----------</font>").arg(Config.TextEditColor.name())));

    connect(Self, SIGNAL(skill_state_changed(QString)), this, SLOT(skillStateChange(QString)));
    trust_button->setEnabled(true);
#ifdef AUDIO_SUPPORT
    if (Config.EnableBgMusic) {
        // start playing background music
        QString bgmusic_path = Config.value("BackgroundMusic", "audio/system/background.ogg").toString();

        Audio::playBGM(bgmusic_path);
        Audio::setBGMVolume(Config.BGMVolume);
    }
#endif
    game_started = true;
}

void RoomScene::freeze() {
    dashboard->setEnabled(false);
    dashboard->stopHuaShen();
    foreach (Photo *photo, photos) {
        photo->hideProgressBar();
        photo->stopHuaShen();
        photo->setEnabled(false);
    }
    item2player.clear();
    chat_edit->setEnabled(false);
#ifdef AUDIO_SUPPORT
    Audio::stopBGM();
#endif
    dashboard->hideProgressBar();
    main_window->setStatusBar(NULL);
}

void RoomScene::_cancelAllFocus() {
    foreach (Photo *photo, photos) {
        photo->hideProgressBar();
        if (photo->getPlayer()->getPhase() == Player::NotActive)
            photo->setFrame(Photo::S_FRAME_NO_FRAME);
    }
}

void RoomScene::moveFocus(const QStringList &players, Countdown countdown) {
    _cancelAllFocus();
    foreach (QString player, players) {
        Photo *photo = name2photo[player];
        if (!photo) {
            Q_ASSERT(player == Self->objectName());
            continue;
        }

        if (ServerInfo.OperationTimeout > 0)
            photo->showProgressBar(countdown);
        else if (photo->getPlayer()->getPhase() == Player::NotActive)
            photo->setFrame(Photo::S_FRAME_RESPONDING);
    }
}

void RoomScene::setEmotion(const QString &who, const QString &emotion) {
    bool permanent = false;
    if (emotion == "question" || emotion == "no-question")
        permanent = true;
    setEmotion(who, emotion, permanent);
}

void RoomScene::setEmotion(const QString &who, const QString &emotion, bool permanent) {
    if (Config.value("NoEquipAnim", false).toBool() && (emotion.startsWith("weapon/") || emotion.startsWith("armor/")))
        return;
    Photo *photo = name2photo[who];
    if (photo) {
        photo->setEmotion(emotion, permanent);
    } else {
        PixmapAnimation *pma = PixmapAnimation::GetPixmapAnimation(dashboard, emotion);
        if (pma) {
            pma->moveBy(0, -dashboard->boundingRect().height() / 1.5);
            pma->setZValue(20002.0);
        }
    }
}

void RoomScene::showSkillInvocation(const QString &who, const QString &skill_name) {
    const ClientPlayer *player = ClientInstance->findChild<const ClientPlayer *>(who);
    if (!player->hasSkill(skill_name) && !player->hasEquipSkill(skill_name)) return;
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill && skill->inherits("SPConvertSkill")) return;
    QString type = "#InvokeSkill";
    QString from_general = player->objectName();
    QString arg = skill_name;
    log_box->appendLog(type, from_general, QStringList(), QString(), arg);
}

void RoomScene::removeLightBox() {
    PixmapAnimation *pma = qobject_cast<PixmapAnimation *>(sender());
    if (pma) {
        removeItem(pma->parentItem());
    } else {
        QPropertyAnimation *animation = qobject_cast<QPropertyAnimation *>(sender());
        QGraphicsTextItem *line = qobject_cast<QGraphicsTextItem *>(animation->targetObject());
        if (line) {
            removeItem(line->parentItem());
        } else {
            QSanSelectableItem *line = qobject_cast<QSanSelectableItem *>(animation->targetObject());
            removeItem(line->parentItem());
        }
    }
}

QGraphicsObject *RoomScene::getAnimationObject(const QString &name) const{
    if (name == Self->objectName())
        return dashboard;
    else
        return name2photo.value(name);
}

void RoomScene::doMovingAnimation(const QString &name, const QStringList &args) {
    QSanSelectableItem *item = new QSanSelectableItem(QString("image/system/animation/%1.png").arg(name));
    item->setZValue(10086.0);
    addItem(item);

    QGraphicsObject *fromItem = getAnimationObject(args.at(0));
    QGraphicsObject *toItem = getAnimationObject(args.at(1));

    QPointF from = fromItem->scenePos();
    QPointF to = toItem->scenePos();
    if (fromItem == dashboard)
        from.setX(fromItem->boundingRect().width() / 2);
    if (toItem == dashboard)
        to.setX(toItem->boundingRect().width() / 2);

    QSequentialAnimationGroup *group = new QSequentialAnimationGroup;

    QPropertyAnimation *move = new QPropertyAnimation(item, "pos");
    move->setStartValue(from);
    move->setEndValue(to);
    move->setDuration(1000);

    QPropertyAnimation *disappear = new QPropertyAnimation(item, "opacity");
    disappear->setEndValue(0.0);
    disappear->setDuration(1000);

    group->addAnimation(move);
    group->addAnimation(disappear);

    group->start(QAbstractAnimation::DeleteWhenStopped);
    connect(group, SIGNAL(finished()), item, SLOT(deleteLater()));
}

void RoomScene::doAppearingAnimation(const QString &name, const QStringList &args) {
    QSanSelectableItem *item = new QSanSelectableItem(QString("image/system/animation/%1.png").arg(name));
    addItem(item);

    QPointF from = getAnimationObject(args.at(0))->scenePos();
    item->setPos(from);

    QPropertyAnimation *disappear = new QPropertyAnimation(item, "opacity");
    disappear->setEndValue(0.0);
    disappear->setDuration(1000);

    disappear->start(QAbstractAnimation::DeleteWhenStopped);
    connect(disappear, SIGNAL(finished()), item, SLOT(deleteLater()));
}

void RoomScene::doLightboxAnimation(const QString &, const QStringList &args) {
    QString word = args.first();
    bool reset_size = word.startsWith("_mini_");
    word = Sanguosha->translate(word);

    QRect rect = main_window->rect();
    QGraphicsRectItem *lightbox = addRect(rect);

    lightbox->setBrush(QColor(32, 32, 32, 204));
    lightbox->setZValue(20001.0);

    if (word.startsWith("image=")) {
        QSanSelectableItem *line = new QSanSelectableItem(word.mid(6));
        addItem(line);

        QRectF line_rect = line->boundingRect();
        line->setParentItem(lightbox);
        line->setPos(m_tableCenterPos - line_rect.center());

        QPropertyAnimation *appear = new QPropertyAnimation(line, "opacity");
        appear->setStartValue(0.0);
        appear->setKeyValueAt(0.7, 1.0);
        appear->setEndValue(0.0);

        int duration = args.value(1, "2000").toInt();
        appear->setDuration(duration);

        appear->start(QAbstractAnimation::DeleteWhenStopped);

        connect(appear, SIGNAL(finished()), line, SLOT(deleteLater()));
        connect(appear, SIGNAL(finished()), this, SLOT(removeLightBox()));
    } else if (word.startsWith("anim=")) {
        PixmapAnimation *pma = PixmapAnimation::GetPixmapAnimation(lightbox, word.mid(5));
        if (pma) {
            pma->setZValue(20002.0);
            pma->moveBy(-sceneRect().width() * _m_roomLayout->m_infoPlaneWidthPercentage / 2, 0);
            connect(pma, SIGNAL(finished()), this, SLOT(removeLightBox()));
        }
    } else {
        QFont font = Config.BigFont;
        if (reset_size) font.setPixelSize(100);
        QGraphicsTextItem *line = addText(word, font);
        line->setDefaultTextColor(Qt::white);

        QRectF line_rect = line->boundingRect();
        line->setParentItem(lightbox);
        line->setPos(m_tableCenterPos - line_rect.center());

        QPropertyAnimation *appear = new QPropertyAnimation(line, "opacity");
        appear->setStartValue(0.0);
        appear->setKeyValueAt(0.7, 1.0);
        appear->setEndValue(0.0);

        int duration = args.value(1, "2000").toInt();
        appear->setDuration(duration);

        appear->start(QAbstractAnimation::DeleteWhenStopped);

        connect(appear, SIGNAL(finished()), this, SLOT(removeLightBox()));
    }
}

void RoomScene::doHuashen(const QString &, const QStringList &args) {
    Q_ASSERT(args.length() >= 2);

    QStringList hargs = args;
    QString name = hargs.first();
    hargs.removeOne(name);
    ClientPlayer *player = ClientInstance->getPlayer(name);
    bool owner = (hargs.first() != "unknown");

    QVariantList huashen_list;
    if (owner)
        huashen_list = Self->tag["Huashens"].toList();
    QList<CardItem *> generals;

    foreach (QString arg, hargs) {
        if (owner) huashen_list << arg;
        CardItem *item = new CardItem(arg);
        item->setPos(this->m_tableCenterPos);
        addItem(item);
        generals.append(item);
    }
    CardsMoveStruct move;
    move.to = player;
    move.from_place = Player::DrawPile;
    move.to_place = Player::PlaceSpecial;
    move.to_pile_name = "huashen";

    GenericCardContainer *container = _getGenericCardContainer(Player::PlaceHand, player);
    container->addCardItems(generals, move);

    if (owner)
        Self->tag["Huashens"] = huashen_list;
}

void RoomScene::showIndicator(const QString &from, const QString &to) {
    if (Config.value("NoIndicator", false).toBool())
        return;

    QGraphicsObject *obj1 = getAnimationObject(from);
    QGraphicsObject *obj2 = getAnimationObject(to);

    if (obj1 == NULL || obj2 == NULL || obj1 == obj2)
        return;

    QPointF start = obj1->sceneBoundingRect().center();
    QPointF finish = obj2->sceneBoundingRect().center();

    IndicatorItem *indicator = new IndicatorItem(start, finish, ClientInstance->getPlayer(from));

    qreal x = qMin(start.x(), finish.x());
    qreal y = qMin(start.y(), finish.y());
    indicator->setPos(x, y);
    indicator->setZValue(30000.0);

    addItem(indicator);
    indicator->doAnimation();
}

void RoomScene::doIndicate(const QString &, const QStringList &args) {
    showIndicator(args.first(), args.last());
}

void RoomScene::doAnimation(int name, const QStringList &args) {
    static QMap<AnimateType, AnimationFunc> map;
    if (map.isEmpty()) {
        map[S_ANIMATE_NULLIFICATION] = &RoomScene::doMovingAnimation;

        map[S_ANIMATE_FIRE] = &RoomScene::doAppearingAnimation;
        map[S_ANIMATE_LIGHTNING] = &RoomScene::doAppearingAnimation;

        map[S_ANIMATE_LIGHTBOX] = &RoomScene::doLightboxAnimation;
        map[S_ANIMATE_HUASHEN] = &RoomScene::doHuashen;
        map[S_ANIMATE_INDICATE] = &RoomScene::doIndicate;
    }

    static QMap<AnimateType, QString> anim_name;
    if (anim_name.isEmpty()) {
        anim_name[S_ANIMATE_NULLIFICATION] = "nullification";

        anim_name[S_ANIMATE_FIRE] = "fire";
        anim_name[S_ANIMATE_LIGHTNING] = "lightning";

        anim_name[S_ANIMATE_LIGHTBOX] = "lightbox";
        anim_name[S_ANIMATE_HUASHEN] = "huashen";
        anim_name[S_ANIMATE_INDICATE] = "indicate";
    }

    AnimationFunc func = map.value((AnimateType)name, NULL);
    if (func) (this->*func)(anim_name.value((AnimateType)name, QString()), args);
}

void RoomScene::showServerInformation() {
    QDialog *dialog = new QDialog(main_window);
    dialog->setWindowTitle(tr("Server information"));

    QHBoxLayout *layout = new QHBoxLayout;
    ServerInfoWidget *widget = new ServerInfoWidget;
    widget->fill(ServerInfo, Config.HostAddress);
    layout->addWidget(widget);
    dialog->setLayout(layout);

    dialog->show();
}

void RoomScene::surrender() {
    if (Self->getPhase() != Player::Play) {
        QMessageBox::warning(main_window, tr("Warning"), tr("You can only initiate a surrender poll at your play phase!"));
        return;
    }

    QMessageBox::StandardButton button;
    button = QMessageBox::question(main_window, tr("Surrender"), tr("Are you sure to surrender ?"));
    if (button == QMessageBox::Ok)
        ClientInstance->requestSurrender();
}

void RoomScene::fillGenerals1v1(const QStringList &names) {
    int len = names.length() / 2;
    QString path = QString("image/system/1v1/select%1.png").arg(len == 5 ? QString() : "2");
    selector_box = new QSanSelectableItem(path, true);
    selector_box->setPos(m_tableCenterPos);
    selector_box->setFlag(QGraphicsItem::ItemIsMovable);
    addItem(selector_box);
    selector_box->setZValue(10000);

    const static int start_x = 42  + G_COMMON_LAYOUT.m_cardNormalWidth / 2;
    const static int width = 86;
    const static int start_y = 59  + G_COMMON_LAYOUT.m_cardNormalHeight / 2;
    const static int height = 121;

    foreach (QString name, names) {
        CardItem *item = new CardItem(name);
        item->setObjectName(name);
        general_items << item;
    }

    qShuffle(general_items);

    int n = names.length();
    double scaleRatio = 116.0 / G_COMMON_LAYOUT.m_cardNormalHeight;
    for (int i = 0; i < n; i++) {
        int row, column;
        if (i < len) {
            row = 1;
            column = i;
        } else {
            row = 2;
            column = i - len;
        }

        CardItem *general_item = general_items.at(i);
        general_item->scaleSmoothly(scaleRatio);
        general_item->setParentItem(selector_box);
        general_item->setPos(start_x + width * column, start_y + height * row);
        general_item->setHomePos(general_item->pos());
    }
}

void RoomScene::fillGenerals3v3(const QStringList &names) {
    QString temperature;
    if (Self->getRole().startsWith("l"))
        temperature = "warm";
    else
        temperature = "cool";

    QString path = QString("image/system/3v3/select-%1.png").arg(temperature);
    selector_box = new QSanSelectableItem(path, true);
    selector_box->setFlag(QGraphicsItem::ItemIsMovable);
    addItem(selector_box);
    selector_box->setZValue(10000);
    selector_box->setPos(m_tableCenterPos);

    const static int start_x = 109;
    const static int width = 86;
    const static int row_y[4] = {150, 271, 394, 516};

    int n = names.length();
    double scaleRatio = 116.0 / G_COMMON_LAYOUT.m_cardNormalHeight;
    for (int i = 0; i < n; i++) {
        int row, column;
        if (i < 8) {
            row = 1;
            column = i;
        } else {
            row = 2;
            column = i - 8;
        }

        CardItem *general_item = new CardItem(names.at(i));
        general_item->scaleSmoothly(scaleRatio);
        general_item->setParentItem(selector_box);
        general_item->setPos(start_x + width * column, row_y[row]);
        general_item->setHomePos(general_item->pos());
        general_item->setObjectName(names.at(i));

        general_items << general_item;
    }
}

void RoomScene::fillGenerals(const QStringList &names) {
    if (ServerInfo.GameMode == "06_3v3")
        fillGenerals3v3(names);
    else if (ServerInfo.GameMode == "02_1v1")
        fillGenerals1v1(names);
}

void RoomScene::bringToFront(QGraphicsItem *front_item) {
    m_zValueMutex.lock();
    if (_m_last_front_item != NULL)
        _m_last_front_item->setZValue(_m_last_front_ZValue);
    _m_last_front_item = front_item;
    _m_last_front_ZValue = front_item->zValue();
    if (pindian_box && front_item != pindian_box && pindian_box->isVisible()) {
        m_zValueMutex.unlock();
        bringToFront(pindian_box);
        m_zValueMutex.lock();
        front_item->setZValue(9999);
    } else {
        front_item->setZValue(10000);
    }
    m_zValueMutex.unlock();
}

void RoomScene::takeGeneral(const QString &who, const QString &name) {
    bool self_taken;
    if (who == "warm")
        self_taken = Self->getRole().startsWith("l");
    else
        self_taken = Self->getRole().startsWith("r");
    QList<CardItem *> *to_add = self_taken ? &down_generals : &up_generals;

    CardItem *general_item = NULL;
    foreach (CardItem *item, general_items) {
        if (item->objectName() == name) {
            general_item = item;
            break;
        }
    }

    Q_ASSERT(general_item);

    general_item->disconnect(this);
    general_items.removeOne(general_item);
    to_add->append(general_item);

    int x , y;
    if (ServerInfo.GameMode == "06_3v3") {
        x = 63 + (to_add->length() - 1) * (148 - 62);
        y = self_taken ? 452 : 85;
    } else {
        x = 43 + (to_add->length() - 1) * 86;
        y = self_taken ? 60 + 120 * 3 : 60;
    }
    x = x + G_COMMON_LAYOUT.m_cardNormalWidth / 2;
    y = y + G_COMMON_LAYOUT.m_cardNormalHeight / 2;
    general_item->setHomePos(QPointF(x, y));
    general_item->goBack(true);

    if (ServerInfo.GameMode == "06_3v3" && (Self->getRole() != "lord" && Self->getRole() != "renegade")
        && general_items.isEmpty()) {
        if (selector_box) {
            selector_box->hide();
            delete selector_box;
            selector_box = NULL;
        }
    }
}

void RoomScene::recoverGeneral(int index, const QString &name) {
    QString obj_name = QString("x%1").arg(index);
    foreach (CardItem *item, general_items) {
        if (item->objectName() == obj_name) {
            item->changeGeneral(name);
            break;
        }
    }
}

void RoomScene::startGeneralSelection() {
    foreach (CardItem *item, general_items) {
        item->setFlag(QGraphicsItem::ItemIsFocusable);
        connect(item, SIGNAL(double_clicked()), this, SLOT(selectGeneral()));
    }
}

void RoomScene::selectGeneral() {
    CardItem *item = qobject_cast<CardItem *>(sender());
    if (item) {
        ClientInstance->replyToServer(S_COMMAND_ASK_GENERAL, Utils::toJsonString(item->objectName()));
        foreach (CardItem *item, general_items) {
            item->setFlag(QGraphicsItem::ItemIsFocusable, false);
            item->disconnect(this);
        }
        ClientInstance->setStatus(Client::NotActive);
    }
}

void RoomScene::changeGeneral(const QString &general) {
    if (to_change && arrange_button) to_change->changeGeneral(general);
}

void RoomScene::revealGeneral(bool self, const QString &general) {
    if (self)
        self_box->revealGeneral(general);
    else
        enemy_box->revealGeneral(general);
}

void RoomScene::skillStateChange(const QString &skill_name) {
    if (skill_name == "shuangxiong") {
        const Skill *skill = Sanguosha->getSkill("shuangxiong");
        addSkillButton(skill);
    } else if (skill_name == "-shuangxiong" || skill_name == ".")
        detachSkill("shuangxiong");
}

void RoomScene::trust() {
    if (Self->getState() != "trust")
        doCancelButton();
    ClientInstance->trust();
}

void RoomScene::startArrange(const QString &to_arrange) {
    arrange_items.clear();
    QString mode;
    QList<QPointF> positions;
    if (ServerInfo.GameMode == "06_3v3") {
        mode = "3v3";
        positions << QPointF(279, 356) << QPointF(407, 356) << QPointF(535, 356);
    } else if (ServerInfo.GameMode == "02_1v1") {
        mode = "1v1";
        if (down_generals.length() == 5)
            positions << QPointF(130, 335) << QPointF(260, 335) << QPointF(390, 335);
        else
            positions << QPointF(173, 335) << QPointF(303, 335) << QPointF(433, 335);
    }

    if (ServerInfo.GameMode == "06_XMode") {
        QStringList arrangeList = to_arrange.split("+");
        if (arrangeList.length() == 5)
            positions << QPointF(130, 335) << QPointF(260, 335) << QPointF(390, 335);
        else
            positions << QPointF(173, 335) << QPointF(303, 335) << QPointF(433, 335);
        QString path = QString("image/system/XMode/arrange%1.png").arg((arrangeList.length() == 5) ? 1 : 2);
        selector_box = new QSanSelectableItem(path, true);
        selector_box->setFlag(QGraphicsItem::ItemIsMovable);
        selector_box->setPos(m_tableCenterPos);
        addItem(selector_box);
        selector_box->setZValue(10000);
    } else {
        QString suffix = (mode == "1v1" && down_generals.length() == 6) ? "2" : QString();
        QString path = QString("image/system/%1/arrange%2.png").arg(mode).arg(suffix);
        selector_box->load(path);
        selector_box->setPos(m_tableCenterPos);
    }

    if (ServerInfo.GameMode == "06_XMode") {
        Q_ASSERT(!to_arrange.isNull());
        down_generals.clear();
        foreach (QString name, to_arrange.split("+")) {
            CardItem *item = new CardItem(name);
            item->setObjectName(name);
            item->scaleSmoothly(116.0 / G_COMMON_LAYOUT.m_cardNormalHeight);
            item->setParentItem(selector_box);
            int x = 43 + down_generals.length() * 86;
            int y = 60 + 120 * 3;
            x = x + G_COMMON_LAYOUT.m_cardNormalWidth / 2;
            y = y + G_COMMON_LAYOUT.m_cardNormalHeight / 2;
            item->setPos(x, y);
            item->setHomePos(QPointF(x, y));
            down_generals << item;
        }
    }
    foreach (CardItem *item, down_generals) {
        item->setFlag(QGraphicsItem::ItemIsFocusable);
        item->setAutoBack(false);
        connect(item, SIGNAL(released()), this, SLOT(toggleArrange()));
    }

    QRect rect(0, 0, 80, 120);

    foreach (QPointF pos, positions) {
        QGraphicsRectItem *rect_item = new QGraphicsRectItem(rect, selector_box);
        rect_item->setPos(pos);
        rect_item->setPen(Qt::NoPen);
        arrange_rects << rect_item;
    }

    arrange_button = new Button(tr("Complete"), 0.8);
    arrange_button->setParentItem(selector_box);
    arrange_button->setPos(600, 330);
    connect(arrange_button, SIGNAL(clicked()), this, SLOT(finishArrange()));
}

void RoomScene::toggleArrange() {
    CardItem *item = qobject_cast<CardItem *>(sender());
    if (item == NULL) return;

    QGraphicsItem *arrange_rect = NULL;
    int index = -1;
    for (int i = 0; i < 3; i++) {
        QGraphicsItem *rect = arrange_rects.at(i);
        if (item->collidesWithItem(rect)) {
            arrange_rect = rect;
            index = i;
        }
    }

    if (arrange_rect == NULL) {
        if (arrange_items.contains(item)) {
            arrange_items.removeOne(item);
            down_generals << item;
        }
    } else {
        arrange_items.removeOne(item);
        down_generals.removeOne(item);
        arrange_items.insert(index, item);
    }

    int n = qMin(arrange_items.length(), 3);
    for (int i = 0; i < n; i++) {
        QPointF pos = arrange_rects.at(i)->pos();
        CardItem *item = arrange_items.at(i);
        item->setHomePos(pos);
        item->goBack(true);
    }

    while (arrange_items.length() > 3) {
        CardItem *last = arrange_items.takeLast();
        down_generals << last;
    }

    for (int i = 0; i < down_generals.length(); i++) {
        QPointF pos;
        if (ServerInfo.GameMode == "06_3v3")
            pos = QPointF(65 + G_COMMON_LAYOUT.m_cardNormalWidth / 2 + i * 86,
                          452 + G_COMMON_LAYOUT.m_cardNormalHeight / 2);
        else
            pos = QPointF(43 + G_COMMON_LAYOUT.m_cardNormalWidth / 2 + i * 86,
                          60 + G_COMMON_LAYOUT.m_cardNormalHeight / 2 + 3 * 120);
        CardItem *item = down_generals.at(i);
        item->setHomePos(pos);
        item->goBack(true);
    }
}

void RoomScene::finishArrange() {
    if (arrange_items.length() != 3) return;

    arrange_button->deleteLater();
    arrange_button = NULL;

    QStringList names;
    foreach (CardItem *item, arrange_items)
        names << item->objectName();

    if (selector_box) {
        selector_box->deleteLater();
        selector_box = NULL;
    }
    arrange_rects.clear();

    ClientInstance->replyToServer(S_COMMAND_ARRANGE_GENERAL, Utils::toJsonArray(names));
    ClientInstance->setStatus(Client::NotActive);
}

void RoomScene::showPindianBox(const QString &from_name, int from_id, const QString &to_name, int to_id, const QString &reason) {
    pindian_box->setOpacity(0.0);
    pindian_box->setPos(m_tableCenterPos);
    if (!reason.isEmpty())
        pindian_box->setTitle(Sanguosha->translate(reason));
    else
        pindian_box->setTitle(tr("pindian"));

    if (pindian_from_card) {
        delete pindian_from_card;
        pindian_from_card = NULL;
    }
    if (pindian_to_card) {
        delete pindian_to_card;
        pindian_to_card = NULL;
    }

    pindian_from_card = new CardItem(Sanguosha->getCard(from_id));
    pindian_from_card->setParentItem(pindian_box);
    pindian_from_card->setPos(QPointF(28 + pindian_from_card->boundingRect().width() / 2,
                                      44 + pindian_from_card->boundingRect().height() / 2));
    pindian_from_card->setFlag(QGraphicsItem::ItemIsMovable, false);
    pindian_from_card->setHomePos(pindian_from_card->pos());
    pindian_from_card->setFootnote(ClientInstance->getPlayerName(from_name));

    pindian_to_card = new CardItem(Sanguosha->getCard(to_id));
    pindian_to_card->setParentItem(pindian_box);
    pindian_to_card->setPos(QPointF(126 + pindian_to_card->boundingRect().width() / 2,
                                    44 + pindian_to_card->boundingRect().height() / 2));
    pindian_to_card->setFlag(QGraphicsItem::ItemIsMovable, false);
    pindian_to_card->setHomePos(pindian_to_card->pos());
    pindian_to_card->setFootnote(ClientInstance->getPlayerName(to_name));

    bringToFront(pindian_box);
    pindian_box->appear();
    QTimer::singleShot(500, this, SLOT(doPindianAnimation()));
}

void RoomScene::doPindianAnimation() {
    if (!pindian_box->isVisible() || !pindian_from_card || !pindian_to_card) return;

    QString emotion = pindian_success ? "success" : "no-success";
    PixmapAnimation *pma = PixmapAnimation::GetPixmapAnimation(pindian_from_card, emotion);
    if (pma)
        connect(pma, SIGNAL(finished()), pindian_box, SLOT(disappear()));
    else
        pindian_box->disappear();
}

static inline void AddRoleIcon(QMap<QChar, QPixmap> &map, char c, const QString &role) {
    QPixmap pixmap(QString("image/system/roles/small-%1.png").arg(role));

    QChar qc(c);
    map[qc.toUpper()] = pixmap;

    QSanUiUtils::makeGray(pixmap);
    map[qc.toLower()] = pixmap;
}

void RoomScene::updateRoles(const QString &roles) {
    foreach (QGraphicsItem *item, role_items)
        removeItem(item);

    role_items.clear();
    if (ServerInfo.EnableHegemony) return;

    static QMap<QChar, QPixmap> map;
    if (map.isEmpty()) {
        AddRoleIcon(map, 'Z', "lord");
        AddRoleIcon(map, 'C', "loyalist");
        AddRoleIcon(map, 'F', "rebel");
        AddRoleIcon(map, 'N', "renegade");
    }

    foreach (QChar c, roles) {
        if (map.contains(c)) {
            QGraphicsPixmapItem *item = addPixmap(map.value(c));
            role_items << item;
        }
    }
    updateRolesBox();
}

void RoomScene::updateRolesBox() {
    double centerX = m_rolesBox->boundingRect().width() / 2;
    int n = role_items.length();
    for (int i = 0; i < n; i++) {
        QGraphicsPixmapItem *item = role_items[i];
        item->setParentItem(m_rolesBox);
        item->setPos(21 * (i - n / 2) + centerX, 6);
    }
    m_pileCardNumInfoTextBox->setTextWidth(m_rolesBox->boundingRect().width());
    m_pileCardNumInfoTextBox->setPos(0, 35);
}

void RoomScene::appendChatEdit(QString txt) {
    chat_edit->setText(chat_edit->text() +  " " + txt);
    chat_edit->setFocus();
}

void RoomScene::appendChatBox(QString txt) {
    QString prefix = "<img src='image/system/chatface/";
    QString suffix = ".png'></img>";
    txt = txt.replace("<#", prefix).replace("#>", suffix);
    chat_box->append(txt);
}

