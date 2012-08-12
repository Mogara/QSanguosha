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
#include <QInputDialog>
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
#include <qmath.h>
#include "uiUtils.h"

#ifdef Q_OS_WIN32
#include <QAxObject>
#endif

#ifdef JOYSTICK_SUPPORT

#include "joystick.h"

#endif

using namespace QSanProtocol;

RoomScene *RoomSceneInstance;

void RoomScene::resetPiles()
{
    // @todo: fix this...
}

#include "qsanbutton.h"

RoomScene::RoomScene(QMainWindow *main_window):
           main_window(main_window),game_started(false)
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

    // create photos
    for(int i = 0; i < player_count - 1;i++){
        Photo *photo = new Photo;
        photos << photo;
        addItem(photo);
        photo->setZValue(-0.5);
    }

    {
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
        connect(dashboard, SIGNAL(card_selected(const Card*)), this, SLOT(enableTargets(const Card*)));
        connect(dashboard, SIGNAL(card_to_use()), this, SLOT(doOkButton()));
        connect(dashboard, SIGNAL(add_equip_skill(const Skill*,bool)), this, SLOT(addSkillButton(const Skill*,bool)));
        connect(dashboard, SIGNAL(remove_equip_skill(QString)), this, SLOT(detachSkill(QString)));
    }

    connect(Self, SIGNAL(pile_changed(QString)), dashboard, SLOT(updatePile(QString)));

    // add role ComboBox
    connect(Self, SIGNAL(role_changed(QString)), dashboard, SLOT(updateRole(QString)));

    if(ClientInstance->getReplayer())
        createReplayControlBar();

    response_skill = new ResponseSkill;
    showorpindian_skill = new ShowOrPindianSkill;
    discard_skill = new DiscardSkill;
    yiji_skill = new YijiViewAsSkill;
    choose_skill = new ChoosePlayerSkill;

    known_cards_menu = new QMenu(main_window);

    {
        change_general_menu = new QMenu(main_window);
        QAction *action = change_general_menu->addAction(tr("Change general ..."));
        FreeChooseDialog *general_changer = new FreeChooseDialog(main_window);
        connect(action, SIGNAL(triggered()), general_changer, SLOT(exec()));
        connect(general_changer, SIGNAL(general_chosen(QString)), this, SLOT(changeGeneral(QString)));
        to_change = NULL;
    }

    // do signal-slot connections
    connect(ClientInstance, SIGNAL(player_added(ClientPlayer*)), SLOT(addPlayer(ClientPlayer*)));
    connect(ClientInstance, SIGNAL(player_removed(QString)), SLOT(removePlayer(QString)));
    connect(ClientInstance, SIGNAL(generals_got(QStringList)), this, SLOT(chooseGeneral(QStringList)));
    connect(ClientInstance, SIGNAL(suits_got(QStringList)), this, SLOT(chooseSuit(QStringList)));
    connect(ClientInstance, SIGNAL(options_got(QString, QStringList)), this, SLOT(chooseOption(QString, QStringList)));
    connect(ClientInstance, SIGNAL(cards_got(const ClientPlayer*, QString, QString)), this, SLOT(chooseCard(const ClientPlayer*, QString, QString)));
    connect(ClientInstance, SIGNAL(roles_got(QString, QStringList)), this, SLOT(chooseRole(QString, QStringList)));
    connect(ClientInstance, SIGNAL(directions_got()), this, SLOT(chooseDirection()));
    connect(ClientInstance, SIGNAL(orders_got(QSanProtocol::Game3v3ChooseOrderCommand)), this, SLOT(chooseOrder(QSanProtocol::Game3v3ChooseOrderCommand)));
    connect(ClientInstance, SIGNAL(kingdoms_got(QStringList)), this, SLOT(chooseKingdom(QStringList)));
    connect(ClientInstance, SIGNAL(seats_arranged(QList<const ClientPlayer*>)), SLOT(arrangeSeats(QList<const ClientPlayer*>)));
    connect(ClientInstance, SIGNAL(status_changed(Client::Status, Client::Status)), this, SLOT(updateStatus(Client::Status, Client::Status)));
    connect(ClientInstance, SIGNAL(avatars_hiden()), this, SLOT(hideAvatars()));
    connect(ClientInstance, SIGNAL(hp_changed(QString,int,DamageStruct::Nature,bool)), SLOT(changeHp(QString,int,DamageStruct::Nature,bool)));
    connect(ClientInstance, SIGNAL(pile_reset()), this, SLOT(resetPiles()));
    connect(ClientInstance, SIGNAL(player_killed(QString)), this, SLOT(killPlayer(QString)));
    connect(ClientInstance, SIGNAL(player_revived(QString)), this, SLOT(revivePlayer(QString)));
    connect(ClientInstance, SIGNAL(card_shown(QString,int)), this, SLOT(showCard(QString,int)));
    connect(ClientInstance, SIGNAL(gongxin(QList<int>, bool)), this, SLOT(doGongxin(QList<int>, bool)));
    connect(ClientInstance, SIGNAL(focus_moved(QStringList, QSanProtocol::Countdown)), this, SLOT(moveFocus(QStringList, QSanProtocol::Countdown)));
    connect(ClientInstance, SIGNAL(emotion_set(QString,QString)), this, SLOT(setEmotion(QString,QString)));
    connect(ClientInstance, SIGNAL(skill_invoked(QString,QString)), this, SLOT(showSkillInvocation(QString,QString)));
    connect(ClientInstance, SIGNAL(skill_acquired(const ClientPlayer*,QString)), this, SLOT(acquireSkill(const ClientPlayer*,QString)));
    connect(ClientInstance, SIGNAL(animated(QString,QStringList)), this, SLOT(doAnimation(QString,QStringList)));
    connect(ClientInstance, SIGNAL(role_state_changed(QString)),this, SLOT(updateRoles(QString)));
    connect(ClientInstance, SIGNAL(event_received(const Json::Value)), this, SLOT(handleGameEvent(const Json::Value))); 

    connect(ClientInstance, SIGNAL(game_started()), this, SLOT(onGameStart()));
    connect(ClientInstance, SIGNAL(game_over()), this, SLOT(onGameOver()));
    connect(ClientInstance, SIGNAL(standoff()), this, SLOT(onStandoff()));

    connect(ClientInstance, SIGNAL(move_cards_lost(int, QList<CardsMoveStruct>)), this, SLOT(loseCards(int, QList<CardsMoveStruct>)));
    connect(ClientInstance, SIGNAL(move_cards_got(int, QList<CardsMoveStruct>)), this, SLOT(getCards(int, QList<CardsMoveStruct>)));

    connect(ClientInstance, SIGNAL(assign_asked()), this, SLOT(startAssign()));
    connect(ClientInstance, SIGNAL(start_in_xs()), this, SLOT(startInXs()));

    {
        guanxing_box = new GuanxingBox;
        guanxing_box->hide();
        addItem(guanxing_box);
        guanxing_box->setZValue(20000.0);

        connect(ClientInstance, SIGNAL(guanxing(QList<int>,bool)), guanxing_box, SLOT(doGuanxing(QList<int>,bool)));

        guanxing_box->moveBy(-120, 0);
    }

    {
        card_container = new CardContainer();
        card_container->hide();
        addItem(card_container);
        // card_container->shift();
        card_container->setZValue(9.0);

        connect(card_container, SIGNAL(item_chosen(int)), ClientInstance, SLOT(onPlayerChooseAG(int)));
        connect(card_container, SIGNAL(item_gongxined(int)), ClientInstance, SLOT(onPlayerReplyGongxin(int)));

        connect(ClientInstance, SIGNAL(ag_filled(QList<int>)), this, SLOT(fillCards(QList<int>)));
        connect(ClientInstance, SIGNAL(ag_taken(ClientPlayer*,int)), this, SLOT(takeAmazingGrace(ClientPlayer*,int)));
        connect(ClientInstance, SIGNAL(ag_cleared()), card_container, SLOT(clear()));

        card_container->moveBy(-120, 0);
    }

    connect(ClientInstance, SIGNAL(skill_attached(QString, bool)), this, SLOT(attachSkill(QString,bool)));
    connect(ClientInstance, SIGNAL(skill_detached(QString)), this, SLOT(detachSkill(QString)));

    enemy_box = NULL;
    self_box = NULL;

    if(ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "02_1v1"){
        // 1v1 & 3v3 mode
        connect(ClientInstance, SIGNAL(generals_filled(QStringList)), this, SLOT(fillGenerals(QStringList)));
        connect(ClientInstance, SIGNAL(general_asked()), this, SLOT(startGeneralSelection()));
        connect(ClientInstance, SIGNAL(general_taken(QString,QString)), this, SLOT(takeGeneral(QString,QString)));
        connect(ClientInstance, SIGNAL(arrange_started()), this, SLOT(startArrange()));
        connect(ClientInstance, SIGNAL(general_recovered(int,QString)), this, SLOT(recoverGeneral(int,QString)));

        arrange_button = NULL;

        if(ServerInfo.GameMode == "02_1v1"){
            enemy_box = new KOFOrderBox(false, this);
            self_box = new KOFOrderBox(true, this);

            enemy_box->hide();
            self_box->hide();           

            connect(ClientInstance, SIGNAL(general_revealed(bool,QString)), this, SLOT(revealGeneral(bool,QString)));
        }
    }

    {
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
        chat_edit_widget = addWidget(chat_edit);
        chat_edit_widget->setObjectName("chat_edit_widget");
        chat_edit_widget->setZValue(-2.0);
        connect(chat_edit, SIGNAL(returnPressed()), this, SLOT(speak()));
#if QT_VERSION >= 0x040700
        chat_edit->setPlaceholderText(tr("Please enter text to chat ... "));
#endif
        
        chat_widget = new ChatWidget();
        chat_widget->setZValue(-0.2);
        addItem(chat_widget);
        connect(chat_widget,SIGNAL(return_button_click()),this, SLOT(speak()));
        connect(chat_widget,SIGNAL(chat_widget_msg(QString)),this, SLOT(appendChatEdit(QString)));        

        if(ServerInfo.DisableChat)
            chat_edit_widget->hide();
    }

    {
        // log box
        log_box = new ClientLogBox;
        log_box->setTextColor(Config.TextEditColor);
        log_box->setObjectName("log_box");

        log_box_widget = addWidget(log_box);
        log_box_widget->setObjectName("log_box_widget");
        log_box_widget->setZValue(-1.0);
        connect(ClientInstance, SIGNAL(log_received(QString)), log_box, SLOT(appendLog(QString)));
    }

    {
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
        //qf.setBold(true);
        prompt_box_widget->setFont(qf);

        addItem(prompt_box);
    }

    timer_id = 0;
    tick = 0;

#ifdef JOYSTICK_SUPPORT

    if(Config.value("JoystickEnabled", false).toBool()){
        Joystick *js = new Joystick(this);
        connect(js, SIGNAL(button_clicked(int)), this, SLOT(onJoyButtonClicked(int)));
        connect(js, SIGNAL(direction_clicked(int)), this, SLOT(onJoyDirectionClicked(int)));

        js->start();
    }

#endif

    m_tableBg = new QGraphicsPixmapItem(NULL, this);
    m_tableBg->setPos(0, 0);
    m_tableBg->setZValue(-100000);

    QHBoxLayout* skill_dock_layout = new QHBoxLayout;
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
    m_pileCardNumInfoTextBox->setDefaultTextColor(Qt::white);
    updateRoles(roles);

    add_robot = NULL;
    fill_robots = NULL;
    if(ServerInfo.EnableAI){
        control_panel = addRect(0, 0, 500, 150, Qt::NoPen);
        // control_panel->translate(-control_panel->boundingRect().width() / 2, -control_panel->boundingRect().height() / 2);
        control_panel->hide();

        add_robot = new Button(tr("Add a robot"));
        add_robot->setParentItem(control_panel);
        add_robot->translate(-add_robot->boundingRect().width() / 2, -add_robot->boundingRect().height() / 2);
        add_robot->setPos(0, -add_robot->boundingRect().height() - 10);

        fill_robots = new Button(tr("Fill robots"));
        fill_robots->setParentItem(control_panel);
        fill_robots->translate(-fill_robots->boundingRect().width() / 2, -fill_robots->boundingRect().height() / 2);
        add_robot->setPos(0, add_robot->boundingRect().height() + 10);

        connect(add_robot, SIGNAL(clicked()), ClientInstance, SLOT(addRobot()));
        connect(fill_robots, SIGNAL(clicked()), ClientInstance, SLOT(fillRobots()));
        connect(Self, SIGNAL(owner_changed(bool)), this, SLOT(showOwnerButtons(bool)));
    } else {
        control_panel = NULL;
    }
    animations = new EffectAnimation();
}

void RoomScene::handleGameEvent(const Json::Value &arg)
{
    GameEventType eventType = (GameEventType)arg[0].asInt();
    switch(eventType){

    case S_GAME_EVENT_PLAYER_DYING: {
        /* the codes below causes crash
        const Player* player = name2photo[arg[1].asCString()]->getPlayer();
        Sanguosha->playAudioEffect(G_ROOM_SKIN.getPlayerAudioEffectPath("sos", player->isMale())); */
        break;
    }

    case S_GAME_EVENT_HUASHEN: {

        ClientPlayer* player = ClientInstance->getPlayer(arg[1].asCString());
        QString huashenGeneral = arg[2].asCString();
        QString huashenSkill = arg[3].asCString();
        PlayerCardContainer *container = (PlayerCardContainer*)_getGenericCardContainer(Player::PlaceHand, player);
        container->startHuaShen(huashenGeneral, huashenSkill);
        break;
    }

    case S_GAME_EVENT_SKILL_INVOKED:{
        QString skillName = arg[1].asCString();
        QString category;
        if(arg[2].isBool()) {
            bool isMale = arg[2].asBool();
            category = isMale ? "male" : "female";
        }
        else if(arg[2].isString()) 
            category = arg[2].asCString();
        int type = arg[3].asInt();
        Sanguosha->playAudioEffect(G_ROOM_SKIN.getPlayerAudioEffectPath(skillName, category, type));

        break;
    }
    case S_GAME_EVENT_JUDGE_RESULT:{
        int cardId = arg[1].asInt();
        bool takeEffect = arg[2].asBool();
        m_tablePile->showJudgeResult(cardId, takeEffect);
        break;
    }
    case S_GAME_EVENT_DETACH_SKILL:{
        QString player_name = arg[1].asCString();
        QString skill_name =  arg[2].asCString();

        ClientPlayer *player = ClientInstance->getPlayer(player_name);
        player->detachSkill(skill_name);
        if(player == Self)
            detachSkill(skill_name);
        
        // stop huashen animation
        PlayerCardContainer *container = (PlayerCardContainer*)_getGenericCardContainer(Player::PlaceHand, player);
        if (!player->hasSkill("huashen"))
        {
            container->stopHuaShen();
        }
        break;
    }
    case S_GAME_EVENT_ACQUIRE_SKILL:{
        QString player_name = arg[1].asCString();
        QString skill_name =  arg[2].asCString();

        ClientPlayer *player = ClientInstance->getPlayer(player_name);

        player->acquireSkill(skill_name);
        acquireSkill(player, skill_name);

        break;
    }
    case S_GAME_EVENT_ADD_SKILL:{
        QString player_name = arg[1].asCString();
        QString skill_name =  arg[2].asCString();

        ClientPlayer *player = ClientInstance->getPlayer(player_name);

        player->addSkill(skill_name);

        break;
    }
    case S_GAME_EVENT_LOSE_SKILL:{
        QString player_name = arg[1].asCString();
        QString skill_name =  arg[2].asCString();

        ClientPlayer *player = ClientInstance->getPlayer(player_name);

        player->loseSkill(skill_name);

        break;
    }
    case S_GAME_EVENT_UPDATE_SKILL:{
        updateSkillButtons();
        break;
    }

    case S_GAME_EVENT_CHANGE_GENDER:{
        QString player_name = arg[1].asCString();
        General::Gender gender =  (General::Gender)arg[2].asInt();

        ClientPlayer *player = ClientInstance->getPlayer(player_name);

        player->setGender(gender);

        break;
    }

    case S_GAME_EVENT_CHANGE_HERO: {
        QString playerName = arg[1].asCString();
        QString newHeroName =  arg[2].asCString();
        bool isSecondaryHero = arg[3].asBool();
        bool senLog = arg[4].asBool();
        ClientPlayer *player = ClientInstance->getPlayer(playerName);
        if(senLog)
            log_box->appendLog("#Transfigure", player->getGeneralName(), QStringList(), QString(), newHeroName);
        if (player != Self) break;     
        const General* oldHero = isSecondaryHero ? player->getGeneral2() : player->getGeneral();
        const General* newHero = Sanguosha->getGeneral(newHeroName);
        if (oldHero) 
        {
            foreach (const Skill *skill, oldHero->getVisibleSkills()){
                detachSkill(skill->objectName());
            }
        }

        if (newHero)
        {
            foreach (const Skill *skill, newHero->getVisibleSkills()){
                attachSkill(skill->objectName(), false);
            }            
        }
        break;
    }

    default:
        break;
    }
}
     

QGraphicsItem *RoomScene::createDashboardButtons(){
    QGraphicsItem *widget = new QGraphicsPixmapItem(
        G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_DASHBOARD_BUTTON_SET_BG)
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

ReplayerControlBar::ReplayerControlBar(Dashboard *dashboard){
    /*QHBoxLayout *layout = new QHBoxLayout;

    QPushButton *play, *uniform, *slow_down, *speed_up;

    uniform = dashboard->createButton("uniform");
    slow_down = dashboard->createButton("slow-down");
    play = dashboard->createButton("pause");
    speed_up = dashboard->createButton("speed-up");

    time_label = new QLabel;
    QPalette palette;
    palette.setColor(QPalette::WindowText, Config.TextEditColor);
    time_label->setPalette(palette);

    QWidgetList widgets;
    widgets << uniform << slow_down << play << speed_up << time_label;

    foreach(QWidget *widget, widgets){
        widget->setEnabled(true);
        layout->addWidget(widget);
    }

    Replayer *replayer = ClientInstance->getReplayer();
    connect(play, SIGNAL(clicked()), replayer, SLOT(toggle()));
    connect(play, SIGNAL(clicked()), this, SLOT(toggle()));
    connect(uniform, SIGNAL(clicked()), replayer, SLOT(uniform()));
    connect(slow_down, SIGNAL(clicked()), replayer, SLOT(slowDown()));
    connect(speed_up, SIGNAL(clicked()), replayer, SLOT(speedUp()));
    connect(replayer, SIGNAL(elasped(int)), this, SLOT(setTime(int)));
    connect(replayer, SIGNAL(speed_changed(qreal)), this, SLOT(setSpeed(qreal)));

    speed = replayer->getSpeed();

    QWidget *widget = new QWidget;
    widget->setAttribute(Qt::WA_TranslucentBackground);
    widget->setLayout(layout);
    setWidget(widget);

    setParentItem(dashboard);
    setPos(0,-35);

    duration_str = FormatTime(replayer->getDuration());*/
}

QString ReplayerControlBar::FormatTime(int secs){
    int minutes = secs / 60;
    int remainder  = secs % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(remainder, 2, 10, QChar('0'));
}

void ReplayerControlBar::toggle(){
    QPushButton *button = qobject_cast<QPushButton *>(sender());

    if(button){
        QString new_name = button->objectName() == "pause" ? "play" : "pause";
        button->setObjectName(new_name);
        button->setIcon(QIcon(QString("image/system/button/%1.png").arg(new_name)));
    }
}

void ReplayerControlBar::setSpeed(qreal speed){
    this->speed = speed;
}

void ReplayerControlBar::setTime(int secs){
    time_label->setText(QString("<b>x%1 </b> [%2/%3]")
                        .arg(speed)
                        .arg(FormatTime(secs))
                        .arg(duration_str));
}

void RoomScene::createReplayControlBar(){
    // hide all buttons    
    m_reverseSelectionButton->hide();

    new ReplayerControlBar(dashboard);
}

void RoomScene::_getSceneSizes(QSize& minSize, QSize &maxSize)
{
    if (photos.size() >= 8)
    {
        minSize = _m_roomLayout->m_minimumSceneSize10Player;
        maxSize = _m_roomLayout->m_maximumSceneSize10Player;
    }
    else
    {
        minSize = _m_roomLayout->m_minimumSceneSize;
        maxSize = _m_roomLayout->m_maximumSceneSize;
    }
}

void RoomScene::adjustItems()
{
    QRectF displayRegion = sceneRect();

    // switch between default & compact skin depending on scene size
    QSanSkinFactory &factory =  QSanSkinFactory::getInstance();
    QString skinName = factory.getCurrentSkinName();

    QSize minSize, maxSize;
    _getSceneSizes(minSize, maxSize);
    if (skinName == "default")
    {
        if (displayRegion.width() < minSize.width() ||
            displayRegion.height() < minSize.height())
        {
            QThread* thread = QCoreApplication::instance()->thread(); 
            thread->blockSignals(true);
            factory.switchSkin("compact");
            thread->blockSignals(false);
            foreach (Photo* photo, photos)
                photo->repaintAll();
        }
    }
    else if (skinName == "compact")
    {
        if (displayRegion.width() > maxSize.width() &&
            displayRegion.height() > maxSize.height())
        {
            QThread* thread = QCoreApplication::instance()->thread(); 
            thread->blockSignals(true);
            factory.switchSkin("default");
            thread->blockSignals(false);
            foreach (Photo* photo, photos)
                photo->repaintAll();
        }
    }

    // update the sizes since we have reloaded the skin.
    _getSceneSizes(minSize, maxSize);

    if (displayRegion.left() != 0 || displayRegion.top() != 0 ||
        displayRegion.bottom() < minSize.height() ||
        displayRegion.right() < minSize.width())
    {
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

    if (self_box)
        self_box->setPos(infoPlane.left() - padding - self_box->boundingRect().width(), 
        sceneRect().height() - padding * 3 - self_box->boundingRect().height()
        - G_DASHBOARD_LAYOUT.m_normalHeight - G_DASHBOARD_LAYOUT.m_floatingAreaHeight);
    if (enemy_box)
        enemy_box->setPos(padding * 2, padding * 2);

    int tablew = log_box_widget->x();
    int tableh = sceneRect().height() - padding - dashboard->boundingRect().height();
    QPixmap tableBg = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_TABLE_BG)
        .scaled(tablew, tableh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_tableBg->setPixmap(tableBg);
    updateTable();
    updateRolesBox();     
}

void RoomScene::_dispersePhotos(QList<Photo*> &photos, QRectF fillRegion,
                                Qt::Orientation orientation, Qt::Alignment align)
{
    double photoWidth = _m_photoLayout->m_normalWidth;
    double photoHeight = _m_photoLayout->m_normalHeight; 
    int numPhotos = photos.size();
    if (numPhotos == 0) return;
    Qt::Alignment hAlign = align & Qt::AlignHorizontal_Mask;
    Qt::Alignment vAlign = align & Qt::AlignVertical_Mask;

    double startX = 0, startY = 0, stepX, stepY;

    if (orientation == Qt::Horizontal)
    {
        double maxWidth = fillRegion.width();
        stepX = qMax(photoWidth  + G_ROOM_LAYOUT.m_photoHDistance, maxWidth / numPhotos);
        stepY = 0;
    }
    else 
    {
        stepX = 0;
        stepY = G_ROOM_LAYOUT.m_photoVDistance + photoHeight;
    }

    switch (vAlign)
    {
    case Qt::AlignTop:
        startY = fillRegion.top() + photoHeight / 2;
        break;
    case Qt::AlignBottom:
        startY = fillRegion.bottom() - photoHeight / 2 - stepY * (numPhotos - 1);
        break;
    case Qt::AlignVCenter:
        startY = fillRegion.center().y() - stepY * (numPhotos - 1) / 2.0;
        break;
    default:
        Q_ASSERT(false);
    }
    switch (hAlign)
    {
    case Qt::AlignLeft:
        startX = fillRegion.left() + photoWidth / 2;
        break;
    case Qt::AlignRight:
        startX = fillRegion.right() - photoWidth / 2 - stepX * (numPhotos - 1);
        break;    
    case Qt::AlignHCenter:
        startX = fillRegion.center().x() - stepX * (numPhotos - 1) / 2.0;
        break;
    default:
        Q_ASSERT(false);
    }
    
    for (int i = 0; i < numPhotos; i++)
    {
        Photo* photo = photos[i];
        QPointF newPos = QPointF(startX + stepX * i, startY + stepY * i);
        photo->setPos(newPos);            
    }
        
}

void RoomScene::updateTable()
{
    int pad = _m_roomLayout->m_scenePadding;
    int tablew = log_box_widget->x() - pad * 2;
    int tableh = sceneRect().height() - pad * 2 -
                 dashboard->boundingRect().height() -
                 _m_roomLayout->m_photoDashboardPadding;
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

    static int regularSeatIndex[][9] = 
    {
        {1},    // 2 players
        {5, 6}, // 3 players
        {5, 1, 6},
        {3, 1, 1, 4},
        {3, 1, 1, 1, 4},
        {5, 5, 1, 1, 6, 6},
        {5, 5, 1, 1, 1, 6, 6}, // 8 players
        {3, 3, 7, 7, 7, 7, 4, 4}, // 9 players
        {3, 3, 7, 7, 7, 7, 7, 4, 4} // 10 players
    };
    static int hulaoSeatIndex[][3] =
    {
        {1, 1, 1}, // if self is lubu
        {3, 3, 1},
        {3, 1, 4},
        {1, 4, 4}
    };
    static int kof3v3SeatIndex[][5] = 
    {
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
    QRectF seatRegions[] = 
    {
        QRectF(col2, 0, col1, row1),
        QRectF(col1, 0, col2 - col1, row1),
        QRectF(0, 0, col1, row1),
        QRectF(col2, row1, col1, row2 - row1),
        QRectF(0, row1, col1, row2 - row1),
        QRectF(col2, 0, col1, row2),
        QRectF(0, 0, col1, row2),
        QRectF(0, 0, col1 + col2, row1)
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

    int* seatToRegion;
    bool pkMode = false;
    if (ServerInfo.GameMode == "04_1v3" && game_started)
    {
        seatToRegion = hulaoSeatIndex[Self->getSeat() - 1];
        pkMode = true;
    }
    else if (ServerInfo.GameMode == "06_3v3" && game_started)
    {
        seatToRegion = kof3v3SeatIndex[(Self->getSeat() - 1) % 3];
        pkMode = true;
    }
    else
    {
        seatToRegion = regularSeatIndex[photos.length() - 1];
    }
    QList<Photo*> photosInRegion[C_NUM_REGIONS];
    int n = photos.length();
    for (int i = 0; i < n; i++)
    {
        int regionIndex = seatToRegion[i];
        if (regionIndex == 4 || regionIndex == 6 || regionIndex == 9)
            photosInRegion[regionIndex].append(photos[i]);            
        else
            photosInRegion[regionIndex].prepend(photos[i]);
    }
    for (int i = 0; i < C_NUM_REGIONS; i++)
    {
        if (photosInRegion[i].isEmpty()) continue;
        Qt::Alignment align;
        if (pkMode) align = kofAligns[i];
        else align = aligns[i];
        Qt::Orientation orient = orients[i];
        
        // if (pkMode) align = Qt::AlignBottom;
        
        int hDist = G_ROOM_LAYOUT.m_photoHDistance;
        QRect floatingArea(0, 0, hDist, G_PHOTO_LAYOUT.m_normalHeight);
        // if the photo is on the right edge of table
        if (i == 0 || i == 3 || i == 5 || i == 8)
            floatingArea.moveRight(0);
        else
            floatingArea.moveLeft(G_PHOTO_LAYOUT.m_normalWidth);
        
        foreach (Photo* photo, photosInRegion[i])
        {
            photo->setFloatingArea(floatingArea);
        }
        _dispersePhotos(photosInRegion[i], seatRegions[i], orient, align);
    }

}

void RoomScene::addPlayer(ClientPlayer *player){
    int i;
    for(i=0; i<photos.length(); i++){
        Photo *photo = photos[i];
        if(photo->getPlayer() == NULL){
            photo->setPlayer(player);
            name2photo[player->objectName()] = photo;

            if(!Self->hasFlag("marshalling"))
                Sanguosha->playSystemAudioEffect("add-player");

            return;
        }
    }
}

void RoomScene::removePlayer(const QString &player_name){
    Photo *photo = name2photo[player_name];
    if(photo){
        photo->setPlayer(NULL);
        name2photo.remove(player_name);

        Sanguosha->playSystemAudioEffect("remove-player");
    }
}

void RoomScene::arrangeSeats(const QList<const ClientPlayer*> &seats){
    // rearrange the photos
    Q_ASSERT(seats.length() == photos.length());

    for(int i = 0; i < seats.length(); i++){
        const Player *player = seats.at(i);
        for(int j = i; j < photos.length(); j++){
            if(photos.at(j)->getPlayer() == player){
                photos.swap(i, j);
                break;
            }
        }
    }
    game_started = true;
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    updateTable();

    //feature changed
    /*
    for(int i = 0; i < photos.length(); i++){
        Photo *photo = photos.at(i);
        photo->setOrder(photo->getPlayer()->getSeat());
    }
    */
        
    group->start(QAbstractAnimation::DeleteWhenStopped);

    // set item to player mapping
    if(item2player.isEmpty()) {
        item2player.insert(dashboard, Self);
        connect(dashboard, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
        connect(dashboard, SIGNAL(selected_changed()), this, SLOT(onSelectChange()));
        foreach(Photo *photo, photos){
            item2player.insert(photo, photo->getPlayer());
            connect(photo, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
            connect(photo, SIGNAL(selected_changed()), this, SLOT(onSelectChange()));
            connect(photo, SIGNAL(enable_changed()), this, SLOT(onEnabledChange()));
        }
    }
}

// @todo: The following 3 fuctions are for drag&use feature. Currently they are very buggy and
// cause a lot of major problems. We should look into this later.
void RoomScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    /*
    _m_isMouseButtonDown = true;
    _m_isInDragAndUseMode = false;
    */    
}

void RoomScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    /*
    if (_m_isInDragAndUseMode)
    {
        bool accepted = false;
        if (ok_button->isEnabled())
        {
            foreach (Photo *photo, photos) {
                if(photo->isUnderMouse()) {
                    accepted = true;
                    break;
                }
            }    

            if (!accepted && dashboard->isAvatarUnderMouse()) {
                accepted = true;
            }
        }
        if (accepted) ok_button->click();
        else
        {
            enableTargets(NULL);
            dashboard->unselectAll();
        }
        _m_isInDragAndUseMode = false;
    } */
}

void RoomScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
    /*
    QGraphicsObject *obj = static_cast<QGraphicsObject*>(focusItem());
    CardItem *card_item = qobject_cast<CardItem*>(obj);
    if(!card_item || !card_item->isUnderMouse())
        return;
    PlayerCardContainer* victim = NULL;

    foreach (Photo *photo, photos) {
        if(photo->isUnderMouse()) {
            victim = photo;
        }
    }

    if (dashboard->isAvatarUnderMouse()) {
        victim = dashboard;
    }
    
    //    _m_isInDragAndUseMode = true;
    //    if (!dashboard->isSelected()) hasUpdate = true;
    if (victim != NULL && !victim->isSelected())
    {
        if (!_m_isInDragAndUseMode)
            enableTargets(card_item->getCard());
        _m_isInDragAndUseMode = true;
        dashboard->selectCard(card_item, true);
        victim->setSelected(true);        
    } */
}

void RoomScene::enableTargets(const Card *card) {
    
    if (card != NULL && (Self->isJilei(card) || Self->isLocked(card))
            && ClientInstance->getStatus() != Client::AskForShowOrPindian){
        ok_button->setEnabled(false);
        return;
    }

    selected_targets.clear();

    // unset avatar and all photo
    foreach(QGraphicsItem *item, item2player.keys()){
        item->setSelected(false);
    }

    if (card == NULL) {

        foreach(PlayerCardContainer *item, item2player.keys()){
            QGraphicsItem* animationTarget = item->getMouseClickReceiver();
            animations->effectOut(animationTarget);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            item->setEnabled(true);
        }

        ok_button->setEnabled(false);
        return;
    }

    if (card->targetFixed() || ClientInstance->hasNoTargetResponsing() ||
            ClientInstance->getStatus() == Client::AskForShowOrPindian) {
        foreach(PlayerCardContainer *item, item2player.keys()) {
            QGraphicsItem* animationTarget = item->getMouseClickReceiver();
            animations->effectOut(animationTarget);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }

        ok_button->setEnabled(true);
        return;
    }

    updateTargetsEnablity(card);

    if(Config.EnableAutoTarget)
        selectNextTarget(false);

    ok_button->setEnabled(card->targetsFeasible(selected_targets, Self));
}

void RoomScene::updateTargetsEnablity(const Card *card){
    QMapIterator<PlayerCardContainer *, const ClientPlayer *> itor(item2player);
    
    while(itor.hasNext()){
        itor.next();

        PlayerCardContainer *item = itor.key();
        const ClientPlayer *player = itor.value();

        int maxVotes = 0;

        if (card)
        {
            card->targetFilter(selected_targets, player, Self, maxVotes);
            item->setMaxVotes(maxVotes);
        }

        if (item->isSelected()) continue;

        //=====================================
        bool weimuFailure = player->hasSkill("weimu") && card && card->isKindOf("Collateral")
                && !selected_targets.isEmpty();
        //=====================================

        bool enabled = (card == NULL) ||
                    ((weimuFailure || !Sanguosha->isProhibited(Self, player, card))
                        && maxVotes > 0);
        
        QGraphicsItem* animationTarget = item->getMouseClickReceiver();
        if (enabled)
            animations->effectOut(animationTarget);
        else if(!animationTarget->graphicsEffect() ||
                !animationTarget->graphicsEffect()->inherits("SentbackEffect"))
            animations->sendBack(animationTarget);
        
        if (card)
        {
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        }
    }
}

void RoomScene::updateSelectedTargets(){
    PlayerCardContainer *item = qobject_cast<PlayerCardContainer *>(sender());
    
    if(item == NULL)
        return;

    const Card *card = dashboard->getSelected();
    if (card) {
        const ClientPlayer *player = item2player.value(item, NULL);
        if (item->isSelected())      
            selected_targets.append(player);
        else{
            selected_targets.removeAll(player);
            foreach(const Player *cp, selected_targets)
            {
                QList<const Player*> tempPlayers = QList<const Player*>(selected_targets);
                tempPlayers.removeAll(cp);
                if(!card->targetFilter(tempPlayers, cp, Self)){
                    selected_targets.clear();
                    unselectAllTargets();
                    return;
                }
            }
        }

        ok_button->setEnabled(card->targetsFeasible(selected_targets, Self));
    }else{
        selected_targets.clear();
    }

    updateTargetsEnablity(card);
}

void RoomScene::keyReleaseEvent(QKeyEvent *event){
    if(!Config.EnableHotKey)
        return;

    if(chat_edit->hasFocus())
        return;

    bool control_is_down = event->modifiers() & Qt::ControlModifier;

    switch(event->key()){
    case Qt::Key_F1: break;
    case Qt::Key_F2: chooseSkillButton(); break;
    case Qt::Key_F3: dashboard->sortCards(); break;

    case Qt::Key_S: dashboard->selectCard("slash");  break;
    case Qt::Key_J: dashboard->selectCard("jink"); break;
    case Qt::Key_P: dashboard->selectCard("peach"); break;

    case Qt::Key_E: dashboard->selectCard("equip"); break;
    case Qt::Key_W: dashboard->selectCard("weapon"); break;
    case Qt::Key_H: dashboard->selectCard("Horse"); break;

    case Qt::Key_T: dashboard->selectCard("trick"); break;
    case Qt::Key_A: dashboard->selectCard("aoe"); break;
    case Qt::Key_N: dashboard->selectCard("nullification"); break;
    case Qt::Key_Q: dashboard->selectCard("snatch"); break;
    case Qt::Key_C: dashboard->selectCard("dismantlement"); break;
    case Qt::Key_U: dashboard->selectCard("duel"); break;
    case Qt::Key_L: dashboard->selectCard("lightning"); break;
    case Qt::Key_I: dashboard->selectCard("indulgence"); break;
    case Qt::Key_R: dashboard->selectCard("collateral"); break;
    case Qt::Key_Y: dashboard->selectCard("god_salvation"); break;

    case Qt::Key_Left: dashboard->selectCard(".", false); break;
    case Qt::Key_Right:
    case Qt::Key_Space:  dashboard->selectCard("."); break; // iterate all cards
    case Qt::Key_F:  break; // fix the selected

    case Qt::Key_G: selectNextTarget(control_is_down); break; // iterate generals

    case Qt::Key_Return : {
            if(ok_button->isEnabled())
                doOkButton();
            break;
        }

    case Qt::Key_Escape : {            
            if(ClientInstance->getStatus() == Client::Playing){
                dashboard->unselectAll();
                enableTargets(NULL);
            }
            break;
        }

    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:{
            int seat = event->key() - Qt::Key_0 + 1;
            int i;
            for(i=0; i<photos.length(); i++){
                if(photos.at(i)->getPlayer()->getSeat() == seat){
                    selectTarget(i, control_is_down);
                    break;
                }
            }
            break;
        }

    case Qt::Key_D:{
            // for debugging use

            if(chat_box_widget){
                QString msg = QString("chat_box_widget (%1, %2)")
                              .arg(chat_box_widget->x()).arg(chat_box_widget->y());

                QMessageBox::information(main_window, "", msg);
            }
        }
    }
}


void RoomScene::chooseGeneral(const QStringList &generals){
    QApplication::alert(main_window);
    if(!main_window->isActiveWindow())
        Sanguosha->playSystemAudioEffect("prelude");
    QDialog *dialog;
    
    if(generals.isEmpty())
        dialog = new FreeChooseDialog(main_window);
    else    
        dialog = new ChooseGeneralDialog(generals, main_window);
    
    delete m_choiceDialog;
    m_choiceDialog = dialog;
}

void RoomScene::chooseSuit(const QStringList &suits)
{
    QDialog *dialog = new QDialog;
    QVBoxLayout *layout = new QVBoxLayout;   

    foreach(QString suit, suits){
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

void RoomScene::chooseKingdom(const QStringList &kingdoms)
{
    QDialog *dialog = new QDialog;
    QVBoxLayout *layout = new QVBoxLayout;

    foreach(QString kingdom, kingdoms){
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

void RoomScene::chooseOption(const QString &skillName, const QStringList &options)
{    
    QDialog *dialog = new QDialog;
    QVBoxLayout *layout = new QVBoxLayout;
    dialog->setWindowTitle(Sanguosha->translate(skillName));   
    layout->addWidget(new QLabel(tr("Please choose:")));

    foreach(QString option, options){
        QCommandLinkButton *button = new QCommandLinkButton;
        QString text = QString("%1:%2").arg(skillName).arg(option);
        QString translated = Sanguosha->translate(text);
        if(text == translated)
            translated = Sanguosha->translate(option);

        button->setObjectName(option);
        button->setText(translated);

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

void RoomScene::chooseCard(const ClientPlayer *player, const QString &flags, const QString &reason)
{
    PlayerCardDialog *dialog = new PlayerCardDialog(player, flags);
    dialog->setWindowTitle(Sanguosha->translate(reason));
    connect(dialog, SIGNAL(card_id_chosen(int)), ClientInstance, SLOT(onPlayerChooseCard(int)));
    connect(dialog, SIGNAL(rejected()), ClientInstance, SLOT(onPlayerChooseCard()));
    delete m_choiceDialog;
    m_choiceDialog = dialog;    
}

void RoomScene::chooseOrder(QSanProtocol::Game3v3ChooseOrderCommand reason)
{
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

void RoomScene::chooseRole(const QString &scheme, const QStringList &roles)
{
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Select role in 3v3 mode"));

    QLabel *prompt = new QLabel(tr("Please select a role"));
    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(prompt);    

    static QMap<QString, QString> jargon;
    if(jargon.isEmpty()){
        jargon["lord"] = tr("Warm leader");
        jargon["loyalist"] = tr("Warm guard");
        jargon["renegade"] = tr("Cool leader");
        jargon["rebel"] = tr("Cool guard");

        jargon["leader1"] = tr("Leader of Team 1");
        jargon["guard1"] = tr("Guard of Team 1");
        jargon["leader2"] = tr("Leader of Team 2");
        jargon["guard2"] = tr("Guard of Team 2");
    }
    
    QStringList possibleRoles;
    if(scheme == "AllRoles")
        possibleRoles << "lord" << "loyalist" << "renegade" << "rebel";
    else
        possibleRoles << "leader1" << "guard1" << "leader2" << "guard2";

    foreach (QString role, possibleRoles)
    {
        QCommandLinkButton *button = new QCommandLinkButton(jargon[role]);
        if(scheme == "AllRoles")
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

void RoomScene::chooseDirection()
{
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

void RoomScene::toggleDiscards(){
    CardOverview *overview = new CardOverview;
    overview->loadFromList(ClientInstance->discarded_list);
    overview->show();
}

GenericCardContainer* RoomScene::_getGenericCardContainer(Player::Place place, Player* player)
{
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

bool RoomScene::_shouldIgnoreDisplayMove(Player::Place from, Player::Place to)
{
    if (from == Player::DiscardPile && to == Player::DiscardPile)
        return true;
    else if (from == Player::PlaceTable && to == Player::DiscardPile)
        return true;
    else if (from == Player::PlaceJudge && to == Player::DiscardPile)
        return true;

    return false;
}

bool RoomScene::_processCardsMove(CardsMoveStruct &move, bool isLost){
    _MoveCardsClassifier cls(move);
    // delayed trick processed;
    if (move.from_place == Player::PlaceDelayedTrick && move.to_place == Player::PlaceTable){
        if(isLost)
            m_move_cache[cls] = move;
        return true;
    }
    CardsMoveStruct tmpMove = m_move_cache.value(cls, CardsMoveStruct());
    if(tmpMove.from_place != Player::PlaceUnknown){
        move.from = tmpMove.from;
        move.from_place = tmpMove.from_place;
        move.from_pile_name = tmpMove.from_pile_name;
    }
    if(!isLost)
        m_move_cache.remove(cls);
    return false;
}

void RoomScene::getCards(int moveId, QList<CardsMoveStruct> card_moves)
{
    for (int i = 0; i < card_moves.size(); i++) 
    {
        CardsMoveStruct &movement = card_moves[i];
        bool skipMove = _processCardsMove(movement, false);
        if(skipMove) continue;
        if (_shouldIgnoreDisplayMove(movement.from_place, movement.to_place)) continue;
        card_container->m_currentPlayer = (ClientPlayer*)movement.to;
        GenericCardContainer* to_container = _getGenericCardContainer(movement.to_place, movement.to);
        QList<CardItem*> cards = _m_cardsMoveStash[moveId][i];
        for (int j = 0; j < cards.size(); j++)
        {            
            CardItem* card = cards[j];
            int card_id = card->getId();
            if (!card_moves[i].card_ids.contains(card_id))
            {
                cards.removeAt(j);
                j--;
            }
            else card->setEnabled(true);
            card->setFootnote(_translateMovementReason(movement.reason));
            card->hideFootnote();
        }
        bringToFront(to_container);
        to_container->addCardItems(cards, movement);
        keepGetCardLog(movement);
    }
    _m_cardsMoveStash[moveId].clear();
}


void RoomScene::loseCards(int moveId, QList<CardsMoveStruct> card_moves)
{
    for (int i = 0; i < card_moves.size(); i++) 
    {
        CardsMoveStruct &movement = card_moves[i];
        bool skipMove = _processCardsMove(movement, true);
        if(skipMove) continue;
        if (_shouldIgnoreDisplayMove(movement.from_place, movement.to_place)) continue;
        card_container->m_currentPlayer = (ClientPlayer*)movement.to;
        GenericCardContainer* from_container = _getGenericCardContainer(movement.from_place, movement.from);
        QList<CardItem*> cards = from_container->removeCardItems(movement.card_ids, movement.from_place);
        foreach (CardItem* card, cards)
        {      
            card->setEnabled(false);
            // card->setHomePos(from_container->mapToScene(card->homePos()));
            // card->setPos(from_container->mapToScene(card->pos()));
            // card->setParentItem(NULL);
        }
        _m_cardsMoveStash[moveId].append(cards);
        keepLoseCardLog(movement);
    }
}

QString RoomScene::_translateMovementReason(const CardMoveReason &reason)
{
    if (reason.m_reason == CardMoveReason::S_REASON_UNKNOWN) return QString();
    Photo* srcPhoto = name2photo[reason.m_playerId];
    Photo* dstPhoto = name2photo[reason.m_targetId];
    QString playerName, targetName;
    
    if (srcPhoto != NULL)
        playerName = Sanguosha->translate(srcPhoto->getPlayer()->getGeneralName());
    else if (reason.m_playerId == Self->objectName())
        playerName = QString("%1(%2)").arg(Sanguosha->translate(Self->getGeneralName()))
                        .arg(Sanguosha->translate("yourself"));
    
    if (dstPhoto != NULL){
        targetName = Sanguosha->translate("use upon")
            .append(Sanguosha->translate(dstPhoto->getPlayer()->getGeneralName()));
    }
    else if (reason.m_targetId == Self->objectName()){
        targetName = QString("%1%2(%3)").arg(Sanguosha->translate("use upon"))
        .arg(Sanguosha->translate(Self->getGeneralName())).arg(Sanguosha->translate("yourself"));
    }
    QString result(playerName + targetName);
    result.append(Sanguosha->translate(reason.m_eventName));
    result.append(Sanguosha->translate(reason.m_skillName));
    if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_USE && reason.m_skillName.isEmpty()){
        result.append(Sanguosha->translate("use"));
    }
    else if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_RESPONSE){
        if (reason.m_reason == CardMoveReason::S_REASON_RETRIAL){
            result.append(Sanguosha->translate("retrial"));
        }
        else if(reason.m_skillName.isEmpty()){
            result.append(Sanguosha->translate("response"));
        }
    }
    else if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_DISCARD){
            if(reason.m_reason == CardMoveReason::S_REASON_RULEDISCARD){
                result.append(Sanguosha->translate("discard"));
            }
            if(reason.m_reason == CardMoveReason::S_REASON_THROW){
                result.append(Sanguosha->translate("throw"));
            }
            else if (reason.m_reason == CardMoveReason::S_REASON_CHANGE_EQUIP){
                result.append(Sanguosha->translate("change equip"));
            }
            else if (reason.m_reason == CardMoveReason::S_REASON_DISMANTLE){
                    result.append(Sanguosha->translate("throw"));
            }
    }
    else if (reason.m_reason == CardMoveReason::S_REASON_RECAST){
        result.append(Sanguosha->translate("recast"));
    }
    else if (reason.m_reason == CardMoveReason::S_REASON_PINDIAN){
        result.append(Sanguosha->translate("pindian"));
    }
    else if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_SHOW){
        if (reason.m_reason == CardMoveReason::S_REASON_JUDGE){
            result.append(Sanguosha->translate("judge"));
        }
        else if (reason.m_reason == CardMoveReason::S_REASON_TURNOVER){
            result.append(Sanguosha->translate("turnover"));
        }
        else if (reason.m_reason == CardMoveReason::S_REASON_DEMONSTRATE){
            result.append(Sanguosha->translate("show"));
        }
    }
    else if ((reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_PUT){
        if(reason.m_reason == CardMoveReason::S_REASON_PUT)
            result.append(Sanguosha->translate("put"));
        else if (reason.m_reason == CardMoveReason::S_REASON_NATURAL_ENTER){
            result.append(Sanguosha->translate("enter"));
        }
        else if (reason.m_reason == CardMoveReason::S_REASON_JUDGEDONE){
            result.append(Sanguosha->translate("judgedone"));
        }
        else if (reason.m_reason == CardMoveReason::S_REASON_REMOVE_FROM_PILE){
                result.append(Sanguosha->translate("backinto"));
        }
    }
    return result;

    //QString("%1:%2:%3:%4").arg(movement.reason.m_reason)
    //            .arg(movement.reason.m_skillName).arg(movement.reason.m_eventName
}

void RoomScene::keepLoseCardLog(const CardsMoveStruct &move)
{
    if(move.from && move.to_place == Player::DrawPile){
        QString type = "$PutCard";
        QString from_general = move.from->getGeneralName();
        log_box->appendLog(type, from_general, QStringList(), QString::number(move.card_ids.first()));
    }
}

void RoomScene::keepGetCardLog(const CardsMoveStruct &move)
{
    if (move.card_ids.isEmpty()) return;
    //DrawNCards
    if (move.from_place == Player::DrawPile && move.to_place == Player::PlaceHand)
    {
        QString to_general = move.to->getGeneralName();
        log_box->appendLog("#DrawNCards", to_general, QStringList(), QString(),
                            QString::number(move.card_ids.length()));
    }
    if(move.from_place == Player::PlaceTable && move.to_place == Player::PlaceHand)
    {
        QString to_general = move.to->getGeneralName();
        foreach(int card_id, move.card_ids){
            if(card_id != -1)
                log_box->appendLog("$GotCardBack", to_general, QStringList(), QString::number(card_id));
        }
    }
    if(move.from_place == Player::DiscardPile && move.to_place == Player::PlaceHand)
    {
        QString to_general = move.to->getGeneralName();
        foreach(int card_id, move.card_ids)
            log_box->appendLog("$RecycleCard", to_general, QStringList(), QString::number(card_id));
    }
    if(move.from && move.from_place != Player::PlaceHand && move.to_place != Player::PlaceDelayedTrick
        && move.from_place != Player::PlaceJudge && move.to && move.from != move.to)
    {
        QString from_general = move.from->getGeneralName();
        QStringList tos;
        tos << move.to->getGeneralName();
        int hide = 0;
        foreach(int card_id, move.card_ids)
        {
            if(card_id != Card::S_UNKNOWN_CARD_ID)
                log_box->appendLog("$MoveCard", from_general, tos, QString::number(card_id));
            else
                hide++;
        }
        if(hide > 0)
            log_box->appendLog("#MoveNCards", from_general, tos, QString(),
            QString::number(hide));
    }
    if(move.from_place == Player::PlaceHand && move.to_place == Player::PlaceHand)
    {
        QString from_general = move.from->getGeneralName();
        QStringList tos;
        tos << move.to->getGeneralName();
        bool hiden = false;
        foreach(int card_id, move.card_ids)
            if(card_id == Card::S_UNKNOWN_CARD_ID){
                hiden = true;
                break;
            }
        if(hiden)
            log_box->appendLog("#MoveNCards", from_general, tos, QString(),
                               QString::number(move.card_ids.length()));
        else
        {
            foreach(int card_id, move.card_ids)
            log_box->appendLog("$MoveCard", from_general, tos, QString::number(card_id));
        }
    }
    if(move.from && move.to){
        // both src and dest are player
        QString type;
        if(move.to_place == Player::PlaceDelayedTrick){
            const Card *trick = Sanguosha->getCard(move.card_ids.first());
            if(trick->objectName() == "lightning"){
                if(move.from != move.to)
                    type = "$LightningMove";
            }
            else
                type = "$PasteCard";
        }
        if(!type.isNull()){
            QString from_general = move.from->objectName();
            QStringList tos;
            tos << move.to->objectName();
            log_box->appendLog(type, from_general, tos, QString::number(move.card_ids.first()));
        }
    }
    if(move.from && move.to && move.from_place == Player::PlaceEquip && move.to_place == Player::PlaceEquip){
        QString type = "$Install";
        QString to_general = move.to->getGeneralName();
        foreach(int card_id, move.card_ids)
            log_box->appendLog(type, to_general, QStringList(), QString::number(card_id));
    }
}

inline uint qHash(const QPointF p) { return qHash((int)p.x()+(int)p.y()); }

void RoomScene::addSkillButton(const Skill *skill, bool from_left){
    //SPConvertSkill is not important around the game, except on game start.
    //Even it isn't a skill, it's only a temporary product of generals replacement system.
    //So I think it is not necessary to exist in dashboard.
    if(skill->inherits("SPConvertSkill") || ClientInstance->getReplayer())
        return;
    // check duplication
    QSanSkillButton* btn = dashboard->addSkillButton(skill->objectName());
    if(btn == NULL)
        return;

    if (btn->getViewAsSkill() != NULL)
    {
        connect(btn, SIGNAL(skill_activated()), dashboard, SLOT(skillButtonActivated()));
        connect(btn, SIGNAL(skill_activated()), this, SLOT(onSkillActivated()));
        connect(btn, SIGNAL(skill_deactivated()), dashboard, SLOT(skillButtonDeactivated()));
        connect(btn, SIGNAL(skill_deactivated()), this, SLOT(onSkillDeactivated()));
    }
    
    QDialog *dialog = skill->getDialog();
    if(dialog){
        dialog->setParent(main_window, Qt::Dialog);
        connect(btn, SIGNAL(skill_activated()), dialog, SLOT(popup()));
        connect(btn, SIGNAL(skill_deactivated()), dialog, SLOT(reject()));
        if(dialog->objectName() == "qice")
            connect(dialog, SIGNAL(onButtonClick()), dashboard, SLOT(selectAll()));
    }
    m_skillButtons.append(btn);
}

void RoomScene::acquireSkill(const ClientPlayer *player, const QString &skill_name){
    QString type = "#AcquireSkill";
    QString from_general = player->getGeneralName();
    QString arg = skill_name;

    log_box->appendLog(type, from_general, QStringList(), QString(), arg);

    if(player == Self){
        addSkillButton(Sanguosha->getSkill(skill_name));
    }
}

void RoomScene::updateSkillButtons(){
    foreach(const Skill* skill, Self->getVisibleSkillList()){
        if(skill->isLordSkill()){
            if(Self->getRole() != "lord" || ServerInfo.GameMode == "06_3v3")
                continue;
        }

        addSkillButton(skill);
    }

    // disable all skill buttons
    foreach (QSanSkillButton *button, m_skillButtons)
        button->setEnabled(false);
}

void RoomScene::useSelectedCard(){
    switch(ClientInstance->getStatus()){
    case Client::Playing:{
            const Card *card = dashboard->getSelected();
            if(card)
                useCard(card);
            break;
        }
    case Client::Responsing:{
            const Card *card = dashboard->getSelected();
            if(card){
                if(ClientInstance->hasNoTargetResponsing())
                    ClientInstance->onPlayerResponseCard(card);
                else
                    ClientInstance->onPlayerUseCard(card, selected_targets);
                prompt_box->disappear();
            }

            dashboard->unselectAll();
            break;
        }

    case Client::AskForShowOrPindian:{
        const Card *card = dashboard->getSelected();
        if(card){
            ClientInstance->onPlayerResponseCard(card);
            prompt_box->disappear();
        }
        dashboard->unselectAll();
        break;
    }

    case Client::Discarding: {
            const Card *card = dashboard->pendingCard();
            if(card){
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
    case Client::AskForAG:{
        ClientInstance->onPlayerChooseAG(-1);
            return;
        }

    case Client::ExecDialog:{
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is in executing dialog"));
            return;
        }

    case Client::AskForSkillInvoke:{
            prompt_box->disappear();
            QString skill_name = ClientInstance->getSkillNameToInvoke();
            dashboard->highlightEquip(skill_name, false);
            ClientInstance->onPlayerInvokeSkill(true);
            break;
        }

    case Client::AskForPlayerChoose:{
            ClientInstance->onPlayerChoosePlayer(selected_targets.first());
            prompt_box->disappear();

            break;
        }

    case Client::AskForYiji:{
            const Card *card = dashboard->pendingCard();
            if(card){
                ClientInstance->onPlayerReplyYiji(card, selected_targets.first());
                dashboard->stopPending();
                prompt_box->disappear();
            }

            break;
        }

    case Client::AskForGuanxing:{
            guanxing_box->reply();

            break;
        }

    case Client::AskForGongxin:{
            ClientInstance->onPlayerReplyGongxin();
            card_container->clear();

            break;
        }
    }

    const ViewAsSkill *skill = dashboard->currentSkill();
    if(skill)
        dashboard->stopPending();
}

void RoomScene::onSelectChange()
{
    /*
    QGraphicsItem * photo = qobject_cast<QGraphicsItem*>(sender());
    if (!photo) return;
    if (photo->isSelected()) animations->emphasize(photo);
    else animations->effectOut(photo); */
}

void RoomScene::onEnabledChange()
{
    QGraphicsItem * photo = qobject_cast<QGraphicsItem*>(sender());
    if(!photo)return;
    if(photo->isEnabled())animations->effectOut(photo);
    else animations->sendBack(photo);
}


void RoomScene::useCard(const Card *card){
    if (card->targetFixed() || card->targetsFeasible(selected_targets, Self))
        ClientInstance->onPlayerUseCard(card, selected_targets);
    
    /*selected_targets.clear();
    foreach(QGraphicsItem *item, item2player.keys()){
        item->setSelected(false);
        animations->effectOut(item);
    }*/
     enableTargets(NULL);
}

void RoomScene::callViewAsSkill(){
    const Card *card = dashboard->pendingCard();

    if(card == NULL)
        return;

    if(card->isAvailable(Self)){
        // use card
        dashboard->stopPending();
        useCard(card);
    }
}

void RoomScene::cancelViewAsSkill(){
    //const ViewAsSkill *skill = dashboard->currentSkill();
    dashboard->stopPending();
    //QAbstractButton *button = button2skill.key(skill, NULL);

    //if(button)
    Client::Status status = ClientInstance->getStatus();
    updateStatus(status, status);
}

void RoomScene::selectTarget(int order, bool multiple){
    QGraphicsItem *to_select = NULL;

    if(order == 0)
        to_select = dashboard;
    else if(order > 0 && order <= photos.length())
        to_select = photos.at(order - 1);

    if(!multiple)
        unselectAllTargets(to_select);

    if(to_select)
        to_select->setSelected(! to_select->isSelected());
}

void RoomScene::selectNextTarget(bool multiple){
    if(!multiple)
        unselectAllTargets();

    QList<QGraphicsItem *> targets;
    foreach(Photo *photo, photos){
        if(photo->flags() & QGraphicsItem::ItemIsSelectable)
            targets << photo;
    }

    if(dashboard->flags() & QGraphicsItem::ItemIsSelectable)
        targets << dashboard;

    int i, j;
    for (i = 0; i < targets.length(); i++){
        if(targets.at(i)->isSelected()){
            for(j = i + 1; j < targets.length(); j++){
                if(!targets.at(j)->isSelected()){
                    targets.at(j)->setSelected(true);
                    return;
                }
            }
        }
    }

    foreach(QGraphicsItem *target, targets){
        if(!target->isSelected()){
            target->setSelected(true);
            break;
        }
    }
}

void RoomScene::unselectAllTargets(const QGraphicsItem *except){
    if(dashboard != except)
        dashboard->setSelected(false);

    foreach(Photo *photo, photos){
        if(photo != except && photo->isSelected())
            photo->setSelected(false);
    }
}

void RoomScene::doTimeout(){
    switch(ClientInstance->getStatus())
    {    
    case Client::Playing:{
        discard_button->click();
        break;}
    case Client::Responsing:
    case Client::Discarding:
    case Client::ExecDialog:
    case Client::AskForShowOrPindian:{
        doCancelButton();
        break;}                     
    case Client::AskForPlayerChoose:{
        ClientInstance->onPlayerChoosePlayer(NULL);
        dashboard->stopPending();
        prompt_box->disappear();
        break;}
    case Client::AskForAG:{
        int card_id = card_container->getFirstEnabled();
        if(card_id != -1)
            ClientInstance->onPlayerChooseAG(card_id);
        break;}        
    case Client::AskForSkillInvoke:
    case Client::AskForYiji:{
        cancel_button->click();
        break;}
    case Client::AskForGuanxing:
    case Client::AskForGongxin:{
        ok_button->click();
        break;}
    default:
        break;
    }
}

void RoomScene::showPromptBox()
{
    bringToFront(prompt_box);
    prompt_box->appear();
}

void RoomScene::updateStatus(Client::Status oldStatus, Client::Status newStatus){
    foreach (QSanSkillButton *button, m_skillButtons){
        Q_ASSERT(button != NULL);
        const ViewAsSkill* vsSkill = button->getViewAsSkill();
        if (vsSkill != NULL)
        {
            CardUseStruct::CardUseReason reason = CardUseStruct::CARD_USE_REASON_UNKNOWN;
            if (newStatus == Client::Responsing)
                reason = CardUseStruct::CARD_USE_REASON_RESPONSE;
            else if (newStatus == Client::Playing)
                reason = CardUseStruct::CARD_USE_REASON_PLAY;
            QString pattern = ClientInstance->getPattern();
            button->setEnabled(vsSkill->isAvailable(reason, pattern)
                               && !pattern.endsWith("!"));
        } 
        else
        {
            const Skill *skill = button->getSkill();
            if(skill->getFrequency() == Skill::Wake)
                button->setEnabled(Self->getMark(skill->objectName()) > 0);
            else
                button->setEnabled(false);
        }
    }

    switch(newStatus){
    case Client::NotActive:{
            if (oldStatus == Client::ExecDialog)
            {
                if (m_choiceDialog != NULL && m_choiceDialog->isVisible())
                {
                    m_choiceDialog->hide();
                }
            }
            else if (oldStatus == Client::AskForGuanxing ||
                     oldStatus == Client::AskForGongxin)
            {
                guanxing_box->clear();
                if(!card_container->retained())
                    card_container->clear();
            }
            prompt_box->disappear();
            ClientInstance->getPromptDoc()->clear();

            dashboard->disableAllCards();
            selected_targets.clear();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            if(dashboard->currentSkill())
                dashboard->stopPending();

            dashboard->hideProgressBar();

            break;
        }

    case Client::Responsing: {
            showPromptBox();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            discard_button->setEnabled(false);

            QString pattern = ClientInstance->getPattern();
            QRegExp rx("@@?([A-Za-z]+)(\\d+)?!?");
            if(rx.exactMatch(pattern)){
                QString skill_name = rx.capturedTexts().at(1);
                const ViewAsSkill *skill = Sanguosha->getViewAsSkill(skill_name);
                if (skill){
                    foreach (QSanSkillButton *button, m_skillButtons){
                        Q_ASSERT(button != NULL);
                        const ViewAsSkill* vsSkill = button->getViewAsSkill();
                        if (vsSkill != NULL && vsSkill->objectName() == skill_name && 
                            vsSkill->isAvailable(CardUseStruct::CARD_USE_REASON_RESPONSE, pattern))
                            button->click();
                    }
                    dashboard->startPending(skill);
                }
            }else{
                response_skill->setPattern(pattern);
                dashboard->startPending(response_skill);
            }
            break;
        }

    case Client::AskForShowOrPindian:{
        showPromptBox();

        ok_button->setEnabled(false);
        cancel_button->setEnabled(false);
        discard_button->setEnabled(false);

        QString pattern = ClientInstance->getPattern();
        showorpindian_skill->setPattern(pattern);
        dashboard->startPending(showorpindian_skill);

        break;
    }

    case Client::Playing:{
            dashboard->enableCards();
            bringToFront(dashboard);
            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(true);
            break;
        }

    case Client::Discarding:{
            showPromptBox();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            discard_button->setEnabled(false);

            discard_skill->setNum(ClientInstance->discard_num);
            discard_skill->setMinNum(ClientInstance->min_num);
            discard_skill->setIncludeEquip(ClientInstance->m_canDiscardEquip);
            dashboard->startPending(discard_skill);
            break;
        }

    case Client::ExecDialog:{
            if (m_choiceDialog != NULL)
            {
                m_choiceDialog->setParent(main_window, Qt::Dialog);
                m_choiceDialog->show();
                ok_button->setEnabled(false);
                cancel_button->setEnabled(true);
                discard_button->setEnabled(false);                
            }
            break;
        }

    case Client::AskForSkillInvoke:{
            QString skill_name = ClientInstance->getSkillNameToInvoke();
            dashboard->highlightEquip(skill_name, true);
            // @todo: refactor this
            foreach (QSanSkillButton *button, m_skillButtons){
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

    case Client::AskForPlayerChoose:{
            showPromptBox();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            QString description = Sanguosha->translate(ClientInstance->skill_name);

            if(!description.isEmpty() && description != ClientInstance->skill_name)
                ClientInstance->getPromptDoc()->setHtml(tr("Please choose a player<br/> <b>Source</b>: %1<br/>").arg(description));
            else
                ClientInstance->getPromptDoc()->setHtml(tr("Please choose a player"));


            choose_skill->setPlayerNames(ClientInstance->players_to_choose);
            dashboard->startPending(choose_skill);

            break;
        }

    case Client::AskForAG:{
            dashboard->disableAllCards();

            ok_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            card_container->startChoose();

            break;
        }

    case Client::AskForYiji:{
            ok_button->setEnabled(false);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);

            yiji_skill->setCards(ClientInstance->getPattern());
            dashboard->startPending(yiji_skill);

            showPromptBox();

            break;
        }
    case Client::AskForGuanxing:{
            ok_button->setEnabled(true);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }

    case Client::AskForGongxin:{
            ok_button->setEnabled(true);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }
    }

    if(ServerInfo.OperationTimeout == 0)
        return;

    // do timeout    
    if(newStatus != Client::NotActive){    
        // _cancelAllFocus();
        QApplication::alert(main_window);
        connect(dashboard, SIGNAL(progressBarTimedOut()), this, SLOT(doTimeout()));        
        dashboard->showProgressBar(ClientInstance->getCountdown());
    }
}

void RoomScene::onSkillDeactivated() {
    const ViewAsSkill *current = dashboard->currentSkill();
    if(current){
        cancel_button->click();
        /*dashboard->stopPending();
        QSanSkillButton *button = qobject_cast<QSanSkillButton *>(sender());
        Q_ASSERT(button);
        doCancelButton();*/
    }
}

void RoomScene::onSkillActivated() {    
    QSanSkillButton *button = qobject_cast<QSanSkillButton *>(sender());
    const ViewAsSkill *skill = button->getViewAsSkill();

    if (skill) {
        dashboard->startPending(skill);

        ok_button->setEnabled(false);
        cancel_button->setEnabled(true);

        const Card *card = dashboard->pendingCard();
        if(card && card->targetFixed()){
            useSelectedCard();
        }
    }
}

void RoomScene::updateTrustButton(){
    if(!ClientInstance->getReplayer()){
        bool trusting = Self->getState() == "trust";
        trust_button->update();
        dashboard->setTrust(trusting);
    }
}

void RoomScene::doOkButton(){
    if(!ok_button->isEnabled())
        return;

    useSelectedCard();
}

void RoomScene::doCancelButton(){
    switch(ClientInstance->getStatus()){
    case Client::Playing:{
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

    case Client::Responsing:{
            dashboard->skillButtonDeactivated();
            QString pattern = ClientInstance->getPattern();
            if(pattern.isEmpty())
                return;

            dashboard->unselectAll();

            if(!pattern.startsWith("@")){
                const ViewAsSkill *skill = dashboard->currentSkill();
                if(!skill->inherits("ResponseSkill")){
                    cancelViewAsSkill();
                    break;
                }
            }

            if(ClientInstance->hasNoTargetResponsing())
                ClientInstance->onPlayerResponseCard(NULL);
            else
                ClientInstance->onPlayerUseCard(NULL);
            prompt_box->disappear();
            dashboard->stopPending();
            break;
        }

    case Client::AskForShowOrPindian:{
        dashboard->unselectAll();
        ClientInstance->onPlayerResponseCard(NULL);
        prompt_box->disappear();
        dashboard->stopPending();
        break;
    }

    case Client::Discarding:{
            dashboard->unselectAll();
            dashboard->stopPending();
            ClientInstance->onPlayerDiscardCards(NULL);
            prompt_box->disappear();
            break;
        }

    case Client::ExecDialog:{
            m_choiceDialog->reject();
            break;
        }

    case Client::AskForSkillInvoke:{
            QString skill_name = ClientInstance->getSkillNameToInvoke();
            dashboard->highlightEquip(skill_name, false);
            ClientInstance->onPlayerInvokeSkill(false);
            prompt_box->disappear();
            break;
        }

    case Client::AskForYiji:{
            dashboard->stopPending();
            ClientInstance->onPlayerReplyYiji(NULL, NULL);
            prompt_box->disappear();
            break;
        }

    default:{
            break;
        }
    }
}

void RoomScene::doDiscardButton(){
    dashboard->stopPending();
    dashboard->unselectAll();

    if(ClientInstance->getStatus() == Client::Playing){
        ClientInstance->onPlayerUseCard(NULL);
    }
}

void RoomScene::hideAvatars(){
    if(control_panel)
        control_panel->hide();
}

void RoomScene::startInXs(){
    if(add_robot) add_robot->hide();
    if(fill_robots) fill_robots->hide();
}

void RoomScene::changeHp(const QString &who, int delta, DamageStruct::Nature nature, bool losthp){
    // update
    Photo *photo = name2photo.value(who, NULL);
    if(photo)
        photo->updateHp();
    else
        dashboard->update();

    if(delta < 0){
        if(losthp){
            Sanguosha->playSystemAudioEffect("hplost");
            return;
        }

        QString damage_effect;
        switch(delta){
        case -1: {
                ClientPlayer *player = ClientInstance->getPlayer(who);
                int r = qrand() % 3 + 1;
                if(player->isMale())
                    damage_effect = QString("injure1-male%1").arg(r);
                else
                    damage_effect = QString("injure1-female%1").arg(r);
                break;
            }

        case -2:{
                damage_effect = "injure2";
                break;
            }

        case -3:
        default:{
                damage_effect = "injure3";
                break;
            }
        }

        Sanguosha->playSystemAudioEffect(damage_effect);

        if(photo){
            //photo->setEmotion("damage");
            setEmotion(who, "damage");
            photo->tremble();
        }

        if(nature == DamageStruct::Fire)
            doAnimation("fire", QStringList() << who);
        else if(nature == DamageStruct::Thunder)
            doAnimation("lightning", QStringList() << who);

    }else{
        QString type = "#Recover";
        QString from_general = ClientInstance->getPlayer(who)->getGeneralName();
        QString n = QString::number(delta);

        log_box->appendLog(type, from_general, QStringList(), QString(), n);
    }
}

void RoomScene::onStandoff(){
    freeze();

#ifdef AUDIO_SUPPORT
    Sanguosha->playSystemAudioEffect("standoff");
#endif

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

void RoomScene::onGameOver(){
    m_roomMutex.lock();
    freeze();

    bool victory = Self->property("win").toBool();

#ifdef AUDIO_SUPPORT
    QString win_effect;
    if(victory){
        win_effect = "win";
        foreach(const Player *player, ClientInstance->getPlayers()){
            if(player->property("win").toBool() && player->isCaoCao()){
                Audio::stop();

                win_effect = "win-cc";
                break;
            }
        }
    }else
        win_effect = "lose";

    Sanguosha->playSystemAudioEffect(win_effect);
#endif

    QDialog *dialog = new QDialog(main_window);
    dialog->resize(500, 600);
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
    foreach(const ClientPlayer *player, ClientInstance->getPlayers()){
        bool win = player->property("win").toBool();
        if(win)
            winner_list << player;
        else
            loser_list << player;

        if(player != Self){
            setEmotion(player->objectName(),win ? "good" : "bad",true);
        }
    }

    fillTable(winner_table, winner_list);
    fillTable(loser_table, loser_list);

    addRestartButton(dialog);
    m_roomMutex.unlock();
    dialog->exec();
    
}

void RoomScene::addRestartButton(QDialog *dialog){
    dialog->resize(main_window->width()/2, dialog->height());

    bool goto_next =false;
    if(Config.GameMode.contains("_mini_") && Self->property("win").toBool())
    {
        QString id = Config.GameMode;
        id.replace("_mini_", "");
        int current = id.toInt();
        if (current != _m_currentStage)
        {
            Q_ASSERT(_m_currentStage == current - 1);
            goto_next = true;
        }
    }

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
    if(layout)
        layout->addLayout(hlayout);

    connect(restart_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(return_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(saveReplayRecord()));
    connect(dialog, SIGNAL(accepted()), this, SIGNAL(restart()));
    connect(return_button, SIGNAL(clicked()), this, SIGNAL(return_to_start()));
}

void RoomScene::saveReplayRecord(){
    QString location = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    QString filename = QFileDialog::getSaveFileName(main_window,
                                                    tr("Save replay record"),
                                                    location,
                                                    tr("Pure text replay file (*.txt);; Image replay file (*.png)"));

    if(!filename.isEmpty()){
        ClientInstance->save(filename);
    }
}

ScriptExecutor::ScriptExecutor(QWidget *parent)
    :QDialog(parent)
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

void ScriptExecutor::doScript(){
    QTextEdit *box = findChild<QTextEdit *>("scriptBox");
    if(box == NULL)
        return;

    QString script = box->toPlainText();
    QByteArray data = script.toAscii();
    data = qCompress(data);
    script = data.toBase64();

    ClientInstance->requestCheatRunScript(script);
}

DeathNoteDialog::DeathNoteDialog(QWidget *parent)
    :QDialog(parent)
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

void DeathNoteDialog::accept(){
    QDialog::accept();
    ClientInstance->requestCheatKill(killer->itemData(killer->currentIndex()).toString(),
                            victim->itemData(victim->currentIndex()).toString());
}

DamageMakerDialog::DamageMakerDialog(QWidget *parent)
    :QDialog(parent)
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
    damage_nature->addItem(tr("HP recover"), S_CHEAT_HP_RECOVER);
    damage_nature->addItem(tr("Lose HP"), S_CHEAT_HP_LOSE);

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

void DamageMakerDialog::disableSource(){
    QString nature = damage_nature->itemData(damage_nature->currentIndex()).toString();
    damage_source->setEnabled(nature != "L");
}

void RoomScene::FillPlayerNames(QComboBox *ComboBox, bool add_none){
    if(add_none)
        ComboBox->addItem(tr("None"), ".");
    QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(Self->getGeneralName(),
                          QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY);
    ComboBox->setIconSize(pixmap.size());
    foreach(const ClientPlayer *player, ClientInstance->getPlayers()){
        QString general_name = Sanguosha->translate(player->getGeneralName());
        if(!player->getGeneral()) continue;
        QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap(player->getGeneralName(),
                          QSanRoomSkin::S_GENERAL_ICON_SIZE_TINY);
        ComboBox->addItem(QIcon(pixmap),
                          QString("%1 [%2]").arg(general_name).arg(player->screenName()),
                          player->objectName());
    }
}

void DamageMakerDialog::accept(){
    QDialog::accept();

    ClientInstance->requestCheatDamage(damage_source->itemData(damage_source->currentIndex()).toString(),
                            damage_target->itemData(damage_target->currentIndex()).toString(),
                            (DamageStruct::Nature)damage_nature->itemData(damage_nature->currentIndex()).toInt(),
                            damage_point->value());
}

void RoomScene::makeDamage(){
    if(Self->getPhase() != Player::Play){
        QMessageBox::warning(main_window, tr("Warning"), tr("This function is only allowed at your play phase!"));
        return;
    }

    DamageMakerDialog *damage_maker = new DamageMakerDialog(main_window);
    damage_maker->exec();
}

void RoomScene::makeKilling(){
    if(Self->getPhase() != Player::Play){
        QMessageBox::warning(main_window, tr("Warning"), tr("This function is only allowed at your play phase!"));
        return;
    }

    DeathNoteDialog *dialog = new DeathNoteDialog(main_window);
    dialog->exec();
}

void RoomScene::makeReviving(){
    if(Self->getPhase() != Player::Play){
        QMessageBox::warning(main_window, tr("Warning"), tr("This function is only allowed at your play phase!"));
        return;
    }

    QStringList items;
    QList<const ClientPlayer*> victims;;
    foreach(const ClientPlayer *player, ClientInstance->getPlayers()){
        if(player->isDead()){
            QString general_name = Sanguosha->translate(player->getGeneralName());
            items << QString("%1 [%2]").arg(player->screenName()).arg(general_name);
            victims << player;
        }
    }

    if(items.isEmpty()){
        QMessageBox::warning(main_window, tr("Warning"), tr("No victims now!"));
        return;
    }

    bool ok;
    QString item = QInputDialog::getItem(main_window, tr("Reviving wand"),
                                         tr("Please select a player to revive"), items, 0, false, &ok);
    if(ok){
        int index = items.indexOf(item);
        ClientInstance->requestCheatRevive(victims.at(index)->objectName());
    }
}

void RoomScene::doScript(){
    ScriptExecutor *dialog = new ScriptExecutor(main_window);
    dialog->exec();
}

void RoomScene::fillTable(QTableWidget *table, const QList<const ClientPlayer *> &players){
   // table->setColumnCount(9);
    table->setColumnCount(4);
    table->setRowCount(players.length());
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    static QStringList labels;
    if(labels.isEmpty()){
        labels << tr("General") << tr("Name") << tr("Alive");
        if(ServerInfo.EnableHegemony)
            labels << tr("Nationality");
        else
            labels << tr("Role");

    }
    table->setHorizontalHeaderLabels(labels);

    table->setSelectionBehavior(QTableWidget::SelectRows);
        
    for(int i = 0; i < players.length(); i++){
        const ClientPlayer *player = players.at(i);

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(Sanguosha->translate(player->getGeneralName()));
        table->setItem(i, 0, item);

        item = new QTableWidgetItem;
        item->setText(player->screenName());
        table->setItem(i, 1, item);

        item = new QTableWidgetItem;
        if(player->isAlive())
            item->setText(tr("Alive"));
        else
            item->setText(tr("Dead"));
        table->setItem(i, 2, item);

        item = new QTableWidgetItem;

        if(ServerInfo.EnableHegemony){
            QIcon icon(QString("image/kingdom/icon/%1.png").arg(player->getKingdom()));
            item->setIcon(icon);
            item->setText(Sanguosha->translate(player->getKingdom()));
        }else{
            QIcon icon(QString("image/system/roles/%1.png").arg(player->getRole()));
            item->setIcon(icon);
            item->setText(Sanguosha->translate(player->getRole()));
        }
        if(!player->isAlive())
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        table->setItem(i, 3, item);
    }
}

void RoomScene::killPlayer(const QString &who){
    const General *general = NULL;
    m_roomMutex.lock();
    if(who == Self->objectName()){
        dashboard->killPlayer();
        general = Self->getGeneral();
        item2player.remove(dashboard);

        if(ServerInfo.GameMode == "02_1v1")
            self_box->killPlayer(Self->getGeneralName());
    }else{
        Photo *photo = name2photo[who];
        photo->killPlayer();
        photo->setFrame(Photo::S_FRAME_NO_FRAME);
        photo->update();
        item2player.remove(photo);

        general = photo->getPlayer()->getGeneral();

        if(ServerInfo.GameMode == "02_1v1")
            enemy_box->killPlayer(general->objectName());
    }

    if(Config.EnableLastWord && !Self->hasFlag("marshalling"))
        general->lastWord();
    m_roomMutex.unlock();
}

void RoomScene::revivePlayer(const QString &who){
    if(who == Self->objectName()){
        dashboard->revivePlayer();
        item2player.insert(dashboard, Self);
        updateSkillButtons();
    }else{
        Photo *photo = name2photo[who];
        photo->revivePlayer();
        item2player.insert(photo, photo->getPlayer());
    }
}

void RoomScene::takeAmazingGrace(ClientPlayer *taker, int card_id){
    QList<int> card_ids;
    card_ids.append(card_id);
    m_tablePile->clear();

    card_container->m_currentPlayer = taker;
    CardItem *copy = card_container->removeCardItems(card_ids, Player::PlaceHand).first();
    if(copy == NULL)
        return;
        
    QList<CardItem*> items;
    items << copy;

    if(taker){
        QString type = "$TakeAG";
        QString from_general = taker->getGeneralName();
        QString card_str = QString::number(card_id);
        log_box->appendLog(type, from_general, QStringList(), card_str);
        GenericCardContainer* container = _getGenericCardContainer(Player::PlaceHand, taker);
        bringToFront(container);
        CardsMoveStruct move;
        move.card_ids.append(card_id);
        move.from_place = Player::PlaceWuGu;
        move.to_place = Player::PlaceHand;
        move.to = taker;
        container->addCardItems(items, move);
    }
    else delete copy;
}

void RoomScene::showCard(const QString &player_name, int card_id){
    QList<int> card_ids;
    card_ids << card_id;
    const ClientPlayer *player = ClientInstance->getPlayer(player_name);

    GenericCardContainer* container = _getGenericCardContainer(Player::PlaceHand, (Player*)player);
    QList<CardItem*> card_items = container->cloneCardItems(card_ids);
    CardMoveReason reason(CardMoveReason::S_REASON_DEMONSTRATE, player->objectName());
    card_items[0]->setFootnote(_translateMovementReason(reason));
    bringToFront(m_tablePile);
    CardsMoveStruct move;
    move.from_place = Player::PlaceHand;
    move.to_place = Player::PlaceTable;
    m_tablePile->addCardItems(card_items, move);

    QString card_str = QString::number(card_id);
    log_box->appendLog("$ShowCard", player->getGeneralName(), QStringList(), card_str);
}

void RoomScene::chooseSkillButton(){
    QList<QSanSkillButton *> enabled_buttons;
    foreach (QSanSkillButton* btn, m_skillButtons){
        if(btn->isEnabled())
            enabled_buttons << btn;
    }

    if(enabled_buttons.isEmpty())
        return;

    QDialog *dialog = new QDialog(main_window);
    dialog->setWindowTitle(tr("Select skill"));

    QVBoxLayout *layout = new QVBoxLayout;

    foreach (QSanSkillButton* btn, enabled_buttons) {
        QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(btn->getSkill()->objectName()));
        connect(button, SIGNAL(clicked()), btn, SIGNAL(clicked()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
        layout->addWidget(button);
    }

    dialog->setLayout(layout);
    dialog->exec();
}

void RoomScene::attachSkill(const QString &skill_name, bool from_left)
{
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(skill)
        addSkillButton(skill, from_left);
}

void RoomScene::detachSkill(const QString &skill_name){
    QSanSkillButton* btn = dashboard->removeSkillButton(skill_name);
    if (btn == NULL) return;    //be care LordSkill and SPConvertSkill
    m_skillButtons.removeAll(btn);
    btn->deleteLater();
}

void RoomScene::viewDistance(){
    DistanceViewDialog *dialog = new DistanceViewDialog(main_window);
    dialog->show();
}

void RoomScene::speak(){
    if (game_started && ServerInfo.DisableChat)
        chat_box->append(tr("This room does not allow chatting!"));
    else
        ClientInstance->speakToServer(chat_edit->text());
    chat_edit->clear();
}

void RoomScene::fillCards(const QList<int> &card_ids)
{
    bringToFront(card_container);
    card_container->fillCards(card_ids);
    card_container->show();
}

void RoomScene::doGongxin(const QList<int> &card_ids, bool enable_heart){
    fillCards(card_ids);
    if(enable_heart)
        card_container->startGongxin();
    else
        card_container->addCloseButton();
    
}


void RoomScene::showOwnerButtons(bool owner){
    if(control_panel && !game_started)
        control_panel->setVisible(owner);
}

void RoomScene::showPlayerCards(){
    QAction *action = qobject_cast<QAction *>(sender());

    if(action){
        QString name = action->data().toString();
        const ClientPlayer *player = ClientInstance->getPlayer(name);

        CardContainer *viewer = new CardContainer();
        viewer->addCloseButton(true);
        addItem(viewer);
        // viewer->shift();
        viewer->view(player);
        viewer->setZValue(card_container->zValue());
    }
}

KOFOrderBox::KOFOrderBox(bool self, QGraphicsScene *scene)
{
    QString basename = self ? "self" : "enemy";
    QString path = QString("image/system/1v1/%1.png").arg(basename);
    setPixmap(QPixmap(path));

    scene->addItem(this);

    int i;
    for(i=0; i<3; i++){
        QSanSelectableItem *avatar = new QSanSelectableItem("image/system/1v1/unknown.png");
        avatar->setParentItem(this);
        avatar->setPos(5, 23 + 62 *i);
        avatar->setObjectName("unknown");

        avatars[i] = avatar;
    }

    revealed = 0;
}

void KOFOrderBox::revealGeneral(const QString &name){
    if(revealed < 3){
        QSanSelectableItem *avatar = avatars[revealed ++];
        avatar->setPixmap(G_ROOM_SKIN.getGeneralPixmap(name, QSanRoomSkin::S_GENERAL_ICON_SIZE_SMALL));
        avatar->setObjectName(name);        
    }
}

void KOFOrderBox::killPlayer(const QString &general_name){
    int i;
    for(i = 0; i < revealed; i++){
        QSanSelectableItem *avatar = avatars[i];
        if(avatar->isEnabled() && avatar->objectName() == general_name){
            QPixmap pixmap("image/system/death/unknown.png");
            QGraphicsPixmapItem *death = new QGraphicsPixmapItem(pixmap, avatar);
            death->moveBy(10, 0);
            avatar->makeGray();
            avatar->setEnabled(false);

            return;
        }
    }
}

#ifdef CHAT_VOICE

SpeakThread::SpeakThread()
    :voice_obj(NULL)
{

}

void SpeakThread::run(){
    voice_obj = new QAxObject("SAPI.SpVoice", this);

    while(true){
        sem.acquire();

        if(to_speak.isEmpty())
            return;

        const QMetaObject *meta = voice_obj->metaObject();
        meta->invokeMethod(voice_obj, "Speak", Q_ARG(QString, to_speak));
    }
}

void SpeakThread::finish(){
    to_speak.clear();

    sem.release();
}

void SpeakThread::speak(const QString &text){
    to_speak = text;

    sem.release();
}

#endif

void RoomScene::onGameStart(){
#ifdef CHAT_VOICE

    if(Config.value("EnableVoice", false).toBool()){
        SpeakThread *thread = new SpeakThread;
        connect(ClientInstance, SIGNAL(text_spoken(QString)), thread, SLOT(speak(QString)));
        connect(this, SIGNAL(destroyed()), thread, SLOT(finish()));

        thread->start();
    }

#endif
    if (Config.GameMode.contains("_mini_"))
    {
        QString id = Config.GameMode;
        id.replace("_mini_", "");
        _m_currentStage = id.toInt();
    }
    else if (ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "02_1v1"){
        selector_box->deleteLater();
        selector_box = NULL;

        chat_widget->show();
        log_box->show();

        if(self_box && enemy_box){
            self_box->show();
            enemy_box->show();
        }
    }

    if(control_panel)
        control_panel->hide();

    log_box->append(tr("<font color='white'>------- Game Start --------</font>"));

    // updateStatus(ClientInstance->getStatus(), ClientInstance->getStatus());

    QList<const ClientPlayer *> players = ClientInstance->getPlayers();
    foreach(const ClientPlayer *player, players){
        connect(player, SIGNAL(phase_changed()), log_box, SLOT(appendSeparator()));
    }

    connect(Self, SIGNAL(skill_state_changed(QString)), this, SLOT(skillStateChange(QString)));

    trust_button->setEnabled(true);


#ifdef AUDIO_SUPPORT

    if(!Config.EnableBgMusic)
        return;

    // start playing background music
    QString bgmusic_path = Config.value("BackgroundMusic", "audio/system/background.ogg").toString();

    Audio::playBGM(bgmusic_path);
    Audio::setBGMVolume(Config.BGMVolume);

#endif

    game_started = true;
}

void RoomScene::freeze(){
    dashboard->setEnabled(false);
    foreach(Photo *photo, photos)
    {
        photo->hideProgressBar();
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

void RoomScene::_cancelAllFocus()
{
    foreach (Photo* photo, photos)
    {
        photo->hideProgressBar();
        if (photo->getPlayer()->getPhase() == Player::NotActive)
            photo->setFrame(Photo::S_FRAME_NO_FRAME);        
    }
}

void RoomScene::moveFocus(const QStringList &players, Countdown countdown)
{
    _cancelAllFocus();
    foreach (QString player, players)
    {
        Photo *photo = name2photo[player];
        if (!photo)
        {
            Q_ASSERT(player == Self->objectName());
            continue;
        }

        if (ServerInfo.OperationTimeout > 0)
            photo->showProgressBar(countdown);
        else if (photo->getPlayer()->getPhase() == Player::NotActive)
            photo->setFrame(Photo::S_FRAME_RESPONSING);
    }
}

void RoomScene::setEmotion(const QString &who, const QString &emotion){
    bool permanent = false;
    if (emotion == "question" || emotion == "no-question")
        permanent = true;
    setEmotion(who, emotion, permanent);
}

void RoomScene::setEmotion(const QString &who, const QString &emotion ,bool permanent){
    Photo *photo = name2photo[who];
    if(photo){
        photo->setEmotion(emotion, permanent);
        return;
    }
    PixmapAnimation * pma = PixmapAnimation::GetPixmapAnimation(dashboard, emotion);
    if(pma)
    {
        pma->moveBy(0,- dashboard->boundingRect().height() / 1.5);
        pma->setZValue(8.0);
    }
}

void RoomScene::showSkillInvocation(const QString &who, const QString &skill_name){
    QString type = "#InvokeSkill";
    const ClientPlayer *player = ClientInstance->findChild<const ClientPlayer *>(who);
    QString from_general = player->getGeneralName();
    QString arg = skill_name;
    log_box->appendLog(type, from_general, QStringList(), QString(), arg);

    if(player != Self){
        Photo *photo = name2photo.value(who);
        photo->showSkillName(skill_name);
    }
}

void RoomScene::removeLightBox(){    
    QPropertyAnimation *animation = qobject_cast<QPropertyAnimation *>(sender());
    QGraphicsTextItem *line = qobject_cast<QGraphicsTextItem *>(animation->targetObject());
    removeItem(line->parentItem());
}

QGraphicsObject *RoomScene::getAnimationObject(const QString &name) const{
    if(name == Self->objectName())
        return dashboard;
    else
        return name2photo.value(name);
}

void RoomScene::doMovingAnimation(const QString &name, const QStringList &args){
    QSanSelectableItem *item = new QSanSelectableItem(QString("image/system/animation/%1.png").arg(name));
    item->setZValue(10086.0);
    addItem(item);

    QGraphicsObject *fromItem = getAnimationObject(args.at(0));
    QGraphicsObject *toItem = getAnimationObject(args.at(1));

    QPointF from = fromItem->scenePos();
    QPointF to = toItem->scenePos();
    if(fromItem == dashboard)
    {
        from.setX(fromItem->boundingRect().width() / 2);
    }
    if(toItem == dashboard)
    {
        to.setX(toItem->boundingRect().width() / 2);
    }
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

#include "playercarddialog.h"

void RoomScene::animatePopup(const QString &name, const QStringList &args)
{
    QPointF pos = getAnimationObject(args.at(0))->scenePos();

    QPixmap *item = new QPixmap(QString("image/system/animation/%1.png").arg(name));
    pos.rx()+=item->width()/2;
    pos.ry()+=item->height()/2;

    Sprite *sprite = new Sprite();
    sprite->setParent(this);
    sprite->setPixmapAtMid(*item);
    Sprite *glare = new Sprite();
    glare->setPixmapAtMid(*item);

    sprite->setResetTime(200);
    sprite->addKeyFrame(0,"opacity",0);
    sprite->addKeyFrame(400,"opacity",1);
    sprite->addKeyFrame(600,"opacity",1);
    //sprite->addKeyFrame(1000,"opacity",0);
    sprite->addKeyFrame(0,"scale",0.2,QEasingCurve::OutQuad);
    sprite->addKeyFrame(400,"scale",1);
    sprite->addKeyFrame(600,"scale",1.2);

    sprite->start();

    addItem(sprite);
    sprite->setPos(pos);
}

void RoomScene::doAppearingAnimation(const QString &name, const QStringList &args){

    if(name == "analeptic" || name == "peach")
    {
        setEmotion(args.at(0),name);
        return;
    }
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

void RoomScene::doLightboxAnimation(const QString &, const QStringList &args){
    // hide discarded card
    QString word = args.first();
    word = Sanguosha->translate(word);

    QRect rect = main_window->rect();
    QGraphicsRectItem *lightbox = addRect(rect);

    lightbox->setBrush(QColor(32, 32, 32, 204));
    lightbox->setZValue(20000.0);

    QGraphicsTextItem *line = addText(word, Config.BigFont);
    line->setDefaultTextColor(Qt::white);
    QRectF line_rect = line->boundingRect();
    line->setParentItem(lightbox);
    line->setPos(m_tableCenterPos - line_rect.center());        

    QPropertyAnimation *appear = new QPropertyAnimation(line, "opacity");
    appear->setStartValue(0.0);
    appear->setKeyValueAt(0.8, 1.0);
    appear->setEndValue(1.0);

    int duration = args.value(1, "2000").toInt();
    appear->setDuration(duration);

    appear->start();

    connect(appear, SIGNAL(finished()), this, SLOT(removeLightBox()));
}

void RoomScene::doHuashen(const QString &, const QStringList &args){
    QVariantList huashen_list = Self->tag["Huashens"].toList();
    QList<CardItem*> generals;
    foreach(QString arg, args){
        huashen_list << arg;
        CardItem *item = new CardItem(arg);
        item->setPos(this->m_tableCenterPos);
        addItem(item);
        generals.append(item);
    }
    CardsMoveStruct move;
    move.from_place = Player::DrawPile;
    move.to_place = Player::PlaceSpecial;
    move.to_pile_name = "huashen";
    dashboard->addCardItems(generals, move);

    Self->tag["Huashens"] = huashen_list;
}

void RoomScene::showIndicator(const QString &from, const QString &to){
    if(Config.value("NoIndicator", false).toBool())
        return;

    QGraphicsObject *obj1 = getAnimationObject(from);
    QGraphicsObject *obj2 = getAnimationObject(to);

    if(obj1 == NULL || obj2 == NULL || obj1 == obj2)
        return;

    QPointF start = obj1->sceneBoundingRect().center();
    QPointF finish = obj2->sceneBoundingRect().center();

    IndicatorItem *indicator = new IndicatorItem(start,
                                                 finish,
                                                 ClientInstance->getPlayer(from));

    qreal x = qMin(start.x(), finish.x());
    qreal y = qMin(start.y(), finish.y());
    indicator->setPos(x, y);
    indicator->setZValue(9.0);

    addItem(indicator);

    indicator->doAnimation();
}

void RoomScene::doIndicate(const QString &, const QStringList &args){
    showIndicator(args.first(), args.last());
}

void RoomScene::doAnimation(const QString &name, const QStringList &args){
    static QMap<QString, AnimationFunc> map;
    if(map.isEmpty()){
        map["peach"] = &RoomScene::doAppearingAnimation;
        map["jink"] = &RoomScene::animatePopup;
        map["nullification"] = &RoomScene::doMovingAnimation;

        map["analeptic"] = &RoomScene::doAppearingAnimation;
        map["fire"] = &RoomScene::doAppearingAnimation;
        map["lightning"] = &RoomScene::doAppearingAnimation;
        map["typhoon"] = &RoomScene::doAppearingAnimation;

        map["lightbox"] = &RoomScene::doLightboxAnimation;
        map["huashen"] = &RoomScene::doHuashen;
        map["indicate"] = &RoomScene::doIndicate;
    }

    AnimationFunc func = map.value(name, NULL);
    if(func)
        (this->*func)(name, args);
}

void RoomScene::showServerInformation()
{
    QDialog *dialog = new QDialog(main_window);
    dialog->setWindowTitle(tr("Server information"));

    QHBoxLayout *layout = new QHBoxLayout;
    ServerInfoWidget *widget = new ServerInfoWidget;
    widget->fill(ServerInfo, Config.HostAddress);
    layout->addWidget(widget);
    dialog->setLayout(layout);

    dialog->show();
}

void RoomScene::kick(){
    if(Self->getRole() != "lord"){
        QMessageBox::warning(main_window, tr("Warning"), tr("Only the lord can kick!"));
        return;
    }

    if(!Config.Password.isEmpty()){
        QMessageBox::warning(main_window, tr("Warning"), tr("This function is disabled in contest mode"));
        return;
    }

    QStringList items;
    QList<const ClientPlayer *> players = ClientInstance->getPlayers();
    if(players.isEmpty())
        return;

    foreach(const ClientPlayer *player, players){
        QString general_name = Sanguosha->translate(player->getGeneralName());
        items << QString("%1 [%2]").arg(player->screenName()).arg(general_name);
    }

    bool ok;
    QString item = QInputDialog::getItem(main_window, tr("Kick"),
                                         tr("Please select the player to kick"), items, 0, false, &ok);
    if(ok){
        int index = items.indexOf(item);
        ClientInstance->kick(players.at(index)->objectName());
    }
}

void RoomScene::surrender(){
    
     if(Self->getPhase() != Player::Play){
        QMessageBox::warning(main_window, tr("Warning"), tr("You can only initiate a surrender poll at your play phase!"));
        return;
    }

    QMessageBox::StandardButton button;
    button = QMessageBox::question(main_window, tr("Surrender"), tr("Are you sure to surrender ?"));
    if(button == QMessageBox::Ok){
        ClientInstance->requestSurrender();
    }
}

void RoomScene::fillGenerals1v1(const QStringList &names){
    selector_box = new QSanSelectableItem("image/system/1v1/select.png", true);
    selector_box->setPos(m_tableCenterPos);
    addItem(selector_box);
    selector_box->setZValue(10000); 
    
    const static int start_x = 42  + G_COMMON_LAYOUT.m_cardNormalWidth / 2;
    const static int width = 86;
    const static int start_y = 59  + G_COMMON_LAYOUT.m_cardNormalHeight / 2;
    const static int height = 121;

    foreach(QString name, names){
        CardItem *item =  new CardItem(name);
        item->setObjectName(name);
        general_items << item;
    }

    qShuffle(general_items);

    int n = names.length();
    double scaleRatio = (double)116 / G_COMMON_LAYOUT.m_cardNormalHeight;
    for(int i = 0; i < n; i++){

        int row, column;
        if(i < 5){
            row = 1;
            column = i;
        }else{
            row = 2;
            column = i - 5;
        }

        CardItem *general_item = general_items.at(i);
        general_item->scaleSmoothly(scaleRatio);
        general_item->setParentItem(selector_box);
        general_item->setPos(start_x + width * column,
                             start_y + height * row);
        general_item->setHomePos(general_item->pos());
    }
}

void RoomScene::fillGenerals3v3(const QStringList &names){
    QString temperature;
    if(Self->getRole().startsWith("l"))
        temperature = "warm";
    else
        temperature = "cool";

    QString path = QString("image/system/3v3/select-%1.png").arg(temperature);
    selector_box = new QSanSelectableItem(path, true);
    addItem(selector_box);
    selector_box->setZValue(10000);
    selector_box->setPos(m_tableCenterPos);

    const static int start_x = 109;
    const static int width = 86;
    const static int row_y[4] = {150, 271, 394, 516};

    int n = names.length();
    double scaleRatio = (double)116 / G_COMMON_LAYOUT.m_cardNormalHeight;
    for(int i = 0; i < n; i++){

        int row, column;
        if(i < 8){
            row = 1;
            column = i;
        }else{
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

void RoomScene::fillGenerals(const QStringList &names){
    if(ServerInfo.GameMode == "06_3v3")
        fillGenerals3v3(names);
    else if(ServerInfo.GameMode == "02_1v1")
        fillGenerals1v1(names);
}

void RoomScene::bringToFront(QGraphicsItem* front_item)
{
    m_zValueMutex.lock();    
    if (_m_last_front_item != NULL)
        _m_last_front_item->setZValue(_m_last_front_ZValue);
    _m_last_front_item = front_item;
    _m_last_front_ZValue = front_item->zValue();
    front_item->setZValue(10000);    
    m_zValueMutex.unlock();
}

void RoomScene::takeGeneral(const QString &who, const QString &name){
    bool self_taken;
    if(who == "warm")
        self_taken = Self->getRole().startsWith("l");
    else
        self_taken = Self->getRole().startsWith("r");
    QList<CardItem *> *to_add = self_taken ? &down_generals : &up_generals;

    CardItem *general_item = NULL;
    foreach(CardItem *item, general_items){
        if(item->objectName() == name){
            general_item = item;
            break;
        }
    }

    Q_ASSERT(general_item);

    general_item->disconnect(this);
    general_items.removeOne(general_item);
    to_add->append(general_item);

    int x , y;
    if(ServerInfo.GameMode == "06_3v3"){
        x = 63 + (to_add->length() - 1) * (148 - 62);
        y = self_taken ? 452 : 85;
    }else{
        x = 43 + (to_add->length() - 1) * 86;
        y = self_taken ? 60 + 120 * 3 : 60;
    }
    x = x + G_COMMON_LAYOUT.m_cardNormalWidth / 2;
    y = y + G_COMMON_LAYOUT.m_cardNormalHeight / 2;
    general_item->setHomePos(QPointF(x, y));    
    general_item->goBack(true);
}

void RoomScene::recoverGeneral(int index, const QString &name){
    QString obj_name = QString("x%1").arg(index);

    foreach(CardItem *item, general_items){
        if(item->objectName() == obj_name){
            item->changeGeneral(name);
            break;
        }
    }
}

void RoomScene::startGeneralSelection(){
    foreach(CardItem *item, general_items){
        item->setFlag(QGraphicsItem::ItemIsFocusable);
        connect(item, SIGNAL(double_clicked()), this, SLOT(selectGeneral()));
    }
}

void RoomScene::selectGeneral(){
    CardItem *item = qobject_cast<CardItem *>(sender());

    if(item){
        ClientInstance->request("takeGeneral " + item->objectName());

        foreach(CardItem *item, general_items){
            item->setFlag(QGraphicsItem::ItemIsFocusable, false);
            item->disconnect(this);
        }
    }
}

void RoomScene::changeGeneral(const QString &general){
    if(to_change && arrange_button){
        to_change->changeGeneral(general);
        to_change->scaleSmoothly(0.4);
    }
}

void RoomScene::revealGeneral(bool self, const QString &general){
    if(self)
        self_box->revealGeneral(general);
    else
        enemy_box->revealGeneral(general);
}

void RoomScene::skillStateChange(const QString &skill_name){
    if(skill_name == "shuangxiong"){
        const Skill *skill = Sanguosha->getSkill("shuangxiong");
        addSkillButton(skill);
    }
    else if(skill_name == "-shuangxiong")
        detachSkill("shuangxiong");
}

void RoomScene::trust(){
    if(Self->getState() != "trust")
        doCancelButton();
    ClientInstance->trust();
}

void RoomScene::startArrange(){
    QString mode;
    QList<QPointF> positions;
    if(ServerInfo.GameMode == "06_3v3"){
        mode = "3v3";
        positions << QPointF(279, 356)
                << QPointF(407, 356)
                << QPointF(535, 356);
    }else{
        mode = "1v1";
        positions << QPointF(130, 335)
                << QPointF(260, 335)
                << QPointF(390, 335);
    }

    selector_box->load(QString("image/system/%1/arrange.png").arg(mode));

    foreach(CardItem *item, down_generals){
        item->setFlag(QGraphicsItem::ItemIsFocusable);
        item->setAutoBack(false);
        connect(item, SIGNAL(released()), this, SLOT(toggleArrange()));
    }

    QRect rect(0, 0, 80, 120);

    foreach(QPointF pos, positions){
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

void RoomScene::toggleArrange(){
    CardItem *item = qobject_cast<CardItem *>(sender());

    if(item == NULL)
        return;

    QGraphicsItem *arrange_rect = NULL;
    int index = -1, i;
    for(i = 0; i < 3; i++){
        QGraphicsItem *rect = arrange_rects.at(i);
        if(item->collidesWithItem(rect)){
            arrange_rect = rect;
            index = i;
        }
    }

    if(arrange_rect == NULL){
        if(arrange_items.contains(item)){
            arrange_items.removeOne(item);
            down_generals << item;
        }
    }else{
        arrange_items.removeOne(item);
        down_generals.removeOne(item);

        arrange_items.insert(index, item);
    }

    int n = qMin(arrange_items.length(), 3);
    for(i = 0; i < n; i++){
        QPointF pos = arrange_rects.at(i)->pos();
        CardItem *item = arrange_items.at(i);
        item->setHomePos(pos);
        item->goBack(true);
    }

    while(arrange_items.length() > 3){
        CardItem *last = arrange_items.takeLast();
        down_generals << last;
    }

    for(i = 0; i < down_generals.length(); i++) {
        QPointF pos;
        if(ServerInfo.GameMode == "06_3v3")
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

void RoomScene::finishArrange(){
    if(arrange_items.length() != 3)
        return;

    arrange_button->deleteLater();
    arrange_button = NULL;

    QStringList names;
    foreach(CardItem *item, arrange_items)
        names << item->objectName();

    ClientInstance->request("arrange " + names.join("+"));
}

static inline void AddRoleIcon(QMap<QChar, QPixmap> &map, char c, const QString &role){
    QPixmap pixmap(QString("image/system/roles/small-%1.png").arg(role));

    QChar qc(c);
    map[qc.toUpper()] = pixmap;

    QSanUiUtils::makeGray(pixmap);
    map[qc.toLower()] = pixmap;
}

void RoomScene::updateRoles(const QString &roles)
{
    foreach(QGraphicsItem *item, role_items)
        removeItem(item);
    role_items.clear();

    if(ServerInfo.EnableHegemony) return;
    static QMap<QChar, QPixmap> map;
    if(map.isEmpty()){
        AddRoleIcon(map, 'Z', "lord");
        AddRoleIcon(map, 'C', "loyalist");
        AddRoleIcon(map, 'F', "rebel");
        AddRoleIcon(map, 'N', "renegade");
    }

    foreach(QChar c, roles){
        if(map.contains(c)){
            QGraphicsPixmapItem *item = addPixmap(map.value(c));    
            role_items << item;
        }
    }
    updateRolesBox();
}

void RoomScene::updateRolesBox()
{
    double centerX = m_rolesBox->boundingRect().width() / 2;
    int n = role_items.length();
    for (int i = 0; i < n; i++)
    {
        QGraphicsPixmapItem *item = role_items[i];
        item->setParentItem(m_rolesBox);
        item->setPos(21 * (i - n / 2) + centerX, 6);
    }
    m_pileCardNumInfoTextBox->setTextWidth(m_rolesBox->boundingRect().width());
    m_pileCardNumInfoTextBox->setPos(0, 35);
}


void RoomScene::appendChatEdit(QString txt){
    chat_edit->setText(chat_edit->text() +  " " + txt);
    chat_edit->setFocus();
}

void RoomScene::appendChatBox(QString txt){
    QString prefix = "<img src='image/system/chatface/";
    QString suffix = ".png'></img>";
    txt=txt.replace("<#", prefix);
    txt=txt.replace("#>", suffix);
    chat_box->append(txt);
}

#ifdef JOYSTICK_SUPPORT

void RoomScene::onJoyButtonClicked(int bit){
    QWidget *active_window = QApplication::activeWindow();

    if(active_window == main_window){
        switch(bit){
        case 1: doOkButton(); break;
        case 2: doCancelButton(); break;
        case 3: doDiscardButton(); break;
        case 4: chooseSkillButton(); break;
        }
    }else{
        switch(bit){
        case 1: {
                QList<QAbstractButton *> buttons = active_window->findChildren<QAbstractButton *>();
                foreach(QAbstractButton *button, buttons){
                    if(button->underMouse() && button->isEnabled()){
                        button->click();
                        break;
                    }
                }

                break;
            }
        case 2: {
                QDialog *dialog = qobject_cast<QDialog *>(active_window);
                if(dialog)
                    dialog->reject();
                break;
            }
        }
    }
}

void RoomScene::onJoyDirectionClicked(int direction){
    QWidget *active_window = QApplication::activeWindow();

    if(active_window == main_window){
        switch(direction){
        case Joystick::Left: dashboard->selectCard(".", false); break;
        case Joystick::Right: dashboard->selectCard(".", true); break;
        case Joystick::Up: selectNextTarget(true); break;
        case Joystick::Down: selectNextTarget(false); break;
        }
    }else{
        bool next = (direction == Joystick::Right || direction == Joystick::Down);
        int index = -1;
        QList<QAbstractButton *> list = active_window->findChildren<QAbstractButton *>();

        QMutableListIterator<QAbstractButton *> itor(list);
        while(itor.hasNext()){
            QAbstractButton *button = itor.next();
            if(!button->isEnabled())
                itor.remove();
        }

        if(list.isEmpty())
            return;

        int i, n = list.length();
        for(i=0; i<n; i++){
            QAbstractButton *button = list.at(i);
            if(button->underMouse() && button->isEnabled()){
                index = i;
                break;
            }
        }

        QAbstractButton *dest = NULL;
        if(index == -1){
            dest = list.first();
        }else{
            n = list.length();
            if(!next){
                index--;
                if(index == -1)
                    index += n;
            }else{
                index++;
                if(index >= n)
                    index -= n;
            }

            dest = list.at(index);
        }

        QPoint center(dest->width()/2, dest->height()/2);
        QCursor::setPos(dest->mapToGlobal(center));
    }
}

#endif

