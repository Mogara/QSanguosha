#include "roomscene.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "cardoverview.h"
#include "distanceviewdialog.h"
#include "choosegeneraldialog.h"
#include "window.h"
#include "button.h"
#include "cardcontainer.h"
#include "recorder.h"
#include "indicatoritem.h"
#include "pixmapanimation.h"
#include "audio.h"

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
#include <QStatusBar>
#include <QMovie>
#include <QtCore>

#ifdef Q_OS_WIN32
#include <QAxObject>
#endif

#ifdef JOYSTICK_SUPPORT

#include "joystick.h"

#endif

using namespace QSanProtocol;

static QPointF DiscardedPos(-6, 8);
static QPointF DrawPilePos(-108, 8);

RoomScene *RoomSceneInstance;

#include "irregularbutton.h"

RoomScene::RoomScene(QMainWindow *main_window)
    :focused(NULL), special_card(NULL), viewing_discards(false),
      main_window(main_window),game_started(false)
{
    RoomSceneInstance = this;

    int player_count = Sanguosha->getPlayerCount(ServerInfo.GameMode);

    bool circular = Config.value("CircularView", false).toBool();
    if(circular){
        DiscardedPos = QPointF(-140, 30);
        DrawPilePos = QPointF(-260, 30);
    }

    // create photos
    int i;
    for(i=0;i<player_count-1;i++){
        Photo *photo = new Photo;
        photos << photo;
        addItem(photo);
        photo->setZValue(-0.5);
    }

    {
        createControlButtons();
        QGraphicsItem *button_widget = NULL;
        if(ClientInstance->getReplayer() == NULL){
            QString path = "image/system/button/irregular/background.png";
            button_widget = new QGraphicsPixmapItem(QPixmap(path));

            ok_button->setParentItem(button_widget);
            cancel_button->setParentItem(button_widget);
            discard_button->setParentItem(button_widget);
            trust_button->setParentItem(button_widget);
        }

        // create dashboard
        dashboard = new Dashboard(button_widget);
        dashboard->setObjectName("dashboard");
        //dashboard->setZValue(0.8);
        addItem(dashboard);

        dashboard->setPlayer(Self);
        connect(Self, SIGNAL(general_changed()), dashboard, SLOT(updateAvatar()));
        connect(Self, SIGNAL(general2_changed()), dashboard, SLOT(updateSmallAvatar()));
        connect(ClientInstance, SIGNAL(do_filter()), dashboard, SLOT(doFilter()));
        connect(dashboard, SIGNAL(card_selected(const Card*)), this, SLOT(enableTargets(const Card*)));
        connect(dashboard, SIGNAL(card_to_use()), this, SLOT(doOkButton()));

        sort_combobox = new QComboBox;

        sort_combobox->addItem(tr("No sort"));
        sort_combobox->addItem(tr("Sort by color"));
        sort_combobox->addItem(tr("Sort by suit"));
        sort_combobox->addItem(tr("Sort by type"));
        sort_combobox->addItem(tr("Sort by availability"));

        connect(sort_combobox, SIGNAL(currentIndexChanged(int)), dashboard, SLOT(sortCards(int)));
    }

    connect(Self, SIGNAL(pile_changed(QString)), this, SLOT(updatePileButton(QString)));

    // add role combobox
    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Your role"));
    role_combobox->addItem(tr("Unknown"));
    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(updateRoleComboBox(QString)));

    createExtraButtons();
    if(ClientInstance->getReplayer())
        createReplayControlBar();

    response_skill = new ResponseSkill;
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

    // get dashboard's avatar
    avatar = dashboard->getAvatar();

    // do signal-slot connections
    connect(ClientInstance, SIGNAL(player_added(ClientPlayer*)), SLOT(addPlayer(ClientPlayer*)));
    connect(ClientInstance, SIGNAL(player_removed(QString)), SLOT(removePlayer(QString)));
    connect(ClientInstance, SIGNAL(generals_got(QStringList)), this, SLOT(chooseGeneral(QStringList)));
    connect(ClientInstance, SIGNAL(seats_arranged(QList<const ClientPlayer*>)), SLOT(arrangeSeats(QList<const ClientPlayer*>)));
    connect(ClientInstance, SIGNAL(status_changed(Client::Status)), this, SLOT(updateStatus(Client::Status)));
    connect(ClientInstance, SIGNAL(avatars_hiden()), this, SLOT(hideAvatars()));
    connect(ClientInstance, SIGNAL(hp_changed(QString,int,DamageStruct::Nature,bool)), SLOT(changeHp(QString,int,DamageStruct::Nature,bool)));
    connect(ClientInstance, SIGNAL(pile_cleared()), this, SLOT(clearPile()));
    connect(ClientInstance, SIGNAL(player_killed(QString)), this, SLOT(killPlayer(QString)));
    connect(ClientInstance, SIGNAL(player_revived(QString)), this, SLOT(revivePlayer(QString)));
    connect(ClientInstance, SIGNAL(card_shown(QString,int)), this, SLOT(showCard(QString,int)));
    connect(ClientInstance, SIGNAL(gongxin(QList<int>, bool)), this, SLOT(doGongxin(QList<int>, bool)));
    connect(ClientInstance, SIGNAL(focus_moved(QString)), this, SLOT(moveFocus(QString)));
    connect(ClientInstance, SIGNAL(emotion_set(QString,QString)), this, SLOT(setEmotion(QString,QString)));
    connect(ClientInstance, SIGNAL(skill_invoked(QString,QString)), this, SLOT(showSkillInvocation(QString,QString)));
    connect(ClientInstance, SIGNAL(skill_acquired(const ClientPlayer*,QString)), this, SLOT(acquireSkill(const ClientPlayer*,QString)));
    connect(ClientInstance, SIGNAL(animated(QString,QStringList)), this, SLOT(doAnimation(QString,QStringList)));
    connect(ClientInstance, SIGNAL(judge_result(QString,QString)), this, SLOT(showJudgeResult(QString,QString)));
    connect(ClientInstance, SIGNAL(role_state_changed(QString)),this, SLOT(updateStateItem(QString)));

    connect(ClientInstance, SIGNAL(game_started()), this, SLOT(onGameStart()));
    connect(ClientInstance, SIGNAL(game_over()), this, SLOT(onGameOver()));
    connect(ClientInstance, SIGNAL(standoff()), this, SLOT(onStandoff()));

    connect(ClientInstance, SIGNAL(card_moved(CardMoveStructForClient)), this, SLOT(moveCard(CardMoveStructForClient)));
    connect(ClientInstance, SIGNAL(n_cards_moved(int,QString,QString)), this, SLOT(moveNCards(int,QString,QString)));

    connect(ClientInstance, SIGNAL(cards_drawed(QList<const Card*>)), this, SLOT(drawCards(QList<const Card*>)));
    connect(ClientInstance, SIGNAL(n_cards_drawed(ClientPlayer*,int)), SLOT(drawNCards(ClientPlayer*,int)));

    connect(ClientInstance, SIGNAL(assign_asked()), this, SLOT(startAssign()));
    connect(ClientInstance, SIGNAL(card_used()), this, SLOT(hideDiscards()));
    connect(ClientInstance, SIGNAL(start_in_xs()), this, SLOT(startInXs()));

    {
        guanxing_box = new GuanxingBox;
        guanxing_box->hide();
        guanxing_box->shift();
        addItem(guanxing_box);
        guanxing_box->setZValue(9.0);

        connect(ClientInstance, SIGNAL(guanxing(QList<int>,bool)), guanxing_box, SLOT(doGuanxing(QList<int>,bool)));

        if(circular)
            guanxing_box->moveBy(-120, 0);
    }

    {
        card_container = new CardContainer();
        card_container->hide();
        addItem(card_container);
        card_container->shift();
        card_container->setZValue(guanxing_box->zValue());

        connect(card_container, SIGNAL(item_chosen(int)), ClientInstance, SLOT(onPlayerChooseAG(int)));
        connect(card_container, SIGNAL(item_gongxined(int)), ClientInstance, SLOT(onPlayerReplyGongxin(int)));

        connect(ClientInstance, SIGNAL(ag_filled(QList<int>)), card_container, SLOT(fillCards(QList<int>)));
        connect(ClientInstance, SIGNAL(ag_taken(const ClientPlayer*,int)), this, SLOT(takeAmazingGrace(const ClientPlayer*,int)));
        connect(ClientInstance, SIGNAL(ag_cleared()), card_container, SLOT(clear()));
        connect(ClientInstance, SIGNAL(ag_disabled(bool)), card_container, SLOT(freezeCards(bool)));

        if(circular)
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

            if(circular){
                enemy_box->setPos(-361, -343);
                self_box->setPos(201, -90);
            }else{
                enemy_box->setPos(-216, -327);
                self_box->setPos(360, -90);
            }

            connect(ClientInstance, SIGNAL(general_revealed(bool,QString)), this, SLOT(revealGeneral(bool,QString)));
        }
    }

    int widen_width = 0;
    if(player_count != 6 && player_count <= 8)
        widen_width = 148;

    if(ServerInfo.GameMode == "02_1v1" && !circular)
        widen_width = 0;

    {
        // chat box
        chat_box = new QTextEdit;
        chat_box->resize(230 + widen_width, 175);
        chat_box->setObjectName("chat_box");

        chat_box_widget = addWidget(chat_box);
        chat_box_widget->setPos(-343 - widen_width, -83);
        chat_box_widget->setZValue(-2.0);
        chat_box_widget->setObjectName("chat_box_widget");

        chat_box->setReadOnly(true);
        chat_box->setTextColor(Config.TextEditColor);
        connect(ClientInstance, SIGNAL(line_spoken(QString)), this, SLOT(appendChatBox(QString)));

        // chat edit
        chat_edit = new QLineEdit;
        chat_edit->setFixedWidth(chat_box->width());
        chat_edit->setObjectName("chat_edit");

        // chatwidget chatface and easytext
        chat_widget = new ChatWidget();
        chat_widget->setX(chat_box_widget->x()+chat_edit->width() - 77);
        chat_widget->setY(chat_box_widget->y()+chat_box->height() + 9);
        chat_widget->setZValue(-0.2);
        addItem(chat_widget);
        connect(chat_widget,SIGNAL(return_button_click()),this, SLOT(speak()));
        connect(chat_widget,SIGNAL(chat_widget_msg(QString)),this, SLOT(appendChatEdit(QString)));

#if QT_VERSION >= 0x040700
        chat_edit->setPlaceholderText(tr("Please enter text to chat ... "));
#endif

        QGraphicsProxyWidget *chat_edit_widget = new QGraphicsProxyWidget(chat_box_widget);
        chat_edit_widget->setWidget(chat_edit);
        chat_edit_widget->setX(0);
        chat_edit_widget->setY(chat_box->height());
        chat_edit_widget->setObjectName("chat_edit_widget");
        connect(chat_edit, SIGNAL(returnPressed()), this, SLOT(speak()));

        if(circular){
            chat_box->resize(268, 165);
            chat_box_widget->setPos(367 , -38);

            chat_edit->setFixedWidth(chat_box->width());
            chat_edit_widget->setX(0);
            chat_edit_widget->setY(chat_box->height()+1);

            chat_widget->setX(chat_box_widget->x()+chat_edit->width() - 77);
            chat_widget->setY(chat_box_widget->y()+chat_box->height() + 9);
        }

        if(ServerInfo.DisableChat)
            chat_edit_widget->hide();
    }

    {
        // log box
        log_box = new ClientLogBox;
        log_box->resize(chat_box->width(), 205);
        log_box->setTextColor(Config.TextEditColor);
        log_box->setObjectName("log_box");

        QGraphicsProxyWidget *log_box_widget = addWidget(log_box);
        log_box_widget->setPos(114, -83);
        log_box_widget->setZValue(-2.0);
        log_box_widget->setObjectName("log_box_widget");
        connect(ClientInstance, SIGNAL(log_received(QString)), log_box, SLOT(appendLog(QString)));

        if(circular){
            log_box->resize(chat_box->width(), 210);
            log_box_widget->setPos(367, -246);
        }
    }

    {
        prompt_box = new Window(tr("QSanguosha"), QSize(480, 200));
        prompt_box->setOpacity(0);
        prompt_box->setFlag(QGraphicsItem::ItemIsMovable);
        prompt_box->shift();
        prompt_box->setZValue(10);
        prompt_box->keepWhenDisappear();

        QGraphicsTextItem *text_item = new QGraphicsTextItem(prompt_box);
        text_item->setParent(prompt_box);
        text_item->setPos(40, 45);
        text_item->setDefaultTextColor(Qt::white);

        QTextDocument *prompt_doc = ClientInstance->getPromptDoc();
        prompt_doc->setTextWidth(prompt_box->boundingRect().width() - 80);
        text_item->setDocument(prompt_doc);

        QFont qf = Config.SmallFont;
        qf.setPixelSize(18);
        qf.setStyleStrategy(QFont::PreferAntialias);
        //qf.setBold(true);
        text_item->setFont(qf);

        QGraphicsDropShadowEffect *drp = new QGraphicsDropShadowEffect;
        drp->setOffset(0);
        drp->setColor(Qt::white);
        drp->setBlurRadius(5);
        //text_item->setGraphicsEffect(drp);

        connect(prompt_doc,SIGNAL(contentsChanged()),this,SLOT(adjustPrompt()));

        addItem(prompt_box);
    }

#ifdef AUDIO_SUPPORT
    memory = new QSharedMemory("QSanguosha", this);
#endif

    progress_bar = dashboard->addProgressBar();
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

    QHBoxLayout* skill_dock_layout = new QHBoxLayout;
    QMargins margins = skill_dock_layout->contentsMargins();
    margins.setTop(0);
    margins.setBottom(5);
    skill_dock_layout->setContentsMargins(margins);
    skill_dock_layout->addStretch();

    main_window->statusBar()->setObjectName("skill_bar_container");
    //main_window->statusBar()->setLayout(skill_dock_layout);
    addWidgetToSkillDock(sort_combobox, true);

    createStateItem();

    animations = new EffectAnimation();
    drawPile = NULL;
    view_transform = QMatrix();
}

void RoomScene::createControlButtons(){
    ok_button = new IrregularButton("ok");
    ok_button->setPos(5, 3);

    cancel_button = new IrregularButton("cancel");
    cancel_button->setPos(5, 92);

    discard_button = new IrregularButton("discard");
    discard_button->setPos(70, 45);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(doOkButton()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(doCancelButton()));
    connect(discard_button, SIGNAL(clicked()), this, SLOT(doDiscardButton()));

    trust_button = new TrustButton;
    trust_button->setPos(69, 133);
    connect(trust_button, SIGNAL(clicked()), ClientInstance, SLOT(trust()));
    connect(Self, SIGNAL(state_changed()), this, SLOT(updateTrustButton()));

    // set them all disabled
    ok_button->setEnabled(false);
    cancel_button->setEnabled(false);
    discard_button->setEnabled(false);
    trust_button->setEnabled(false);
}

void RoomScene::createExtraButtons(){
    reverse_button = dashboard->createButton("reverse-select");
    reverse_button->setEnabled(true);

    dashboard->addWidget(reverse_button, 100, true);
    connect(reverse_button, SIGNAL(clicked()), dashboard, SLOT(reverseSelection()));

    free_discard = NULL;
}

ReplayerControlBar::ReplayerControlBar(Dashboard *dashboard){
    QHBoxLayout *layout = new QHBoxLayout;

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

    duration_str = FormatTime(replayer->getDuration());
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
    reverse_button->hide();

    new ReplayerControlBar(dashboard);
}

void RoomScene::adjustItems(QMatrix matrix){
    if(matrix.m11()>1)matrix.setMatrix(1,0,0,1,matrix.dx(),matrix.dy());

    dashboard->setWidth((main_window->width()-10)/ matrix.m11()) ;

    qreal dashboard_width = dashboard->boundingRect().width();
    qreal x = - dashboard_width/2;
    qreal main_height = main_window->centralWidget()->height() / matrix.m22();
    qreal y = main_height/2 - dashboard->boundingRect().height();

    dashboard->setPos(x, y);

    QList<QPointF> positions = getPhotoPositions();
    int i;
    for(i=0; i<positions.length(); i++)
        photos.at(i)->setPos(positions.at(i));

    reLayout(matrix);
}

QList<QPointF> RoomScene::getPhotoPositions() const{
    static int four=0;
    static int five=0;
    static int six=0;
    static int seven=0;
    static int eight=0;
    static int nine=0;
    static int cxw=0;
    static int cxw2=1;

    int player_count = photos.length() + 1;
    switch(player_count){
    case 4: four = 1; break;
    case 5: five = 1; break;
    case 6: six = 1; break;
    case 7: seven = 1; break;
    case 8: eight = 1; break;
    case 9: nine = 1; break;
    }

    if(ServerInfo.GameMode == "06_3v3" )
    {
        six   = 0;
        nine = 1;
    }

    if(Config.value("CircularView").toBool()){
        cxw=1;
        cxw2=0;
    }

    static const QPointF pos[] = {
        QPointF((-630+cxw2*129)+(cxw*four*70)+(cxw*six*50), (-70+cxw2)+(-four*cxw*80)+(-six*cxw*50)), // 0:zhugeliang
        QPointF((-630+cxw2*129)+(cxw*eight*50)+(cxw*five*50)+(cxw*nine*20), (-270-cxw2*3)+(cxw*five*100)), // 1:wolong
        QPointF((-487+cxw2*131)+(cxw*six*80)+(-seven*cxw*25)+(cxw*nine*45), (-316+cxw2*22)+(cxw*six*15)+(cxw*seven*30)), // 2:shenzhugeliang
        QPointF((-344+cxw2*133)+(-eight*cxw*50)+(cxw*five*15)+(cxw*seven*50)+(cxw*nine*65), (-320+cxw2*26)), // 3:lusu
        QPointF((-201+cxw2*135), -324+cxw2*30), // 4:dongzhuo
        QPointF((-58+cxw2*137)+(cxw*eight*50)+(-five*cxw*15)+(-seven*cxw*50)+(-nine*cxw*65), (-320+cxw2*26)), // 5:caocao
        QPointF((85+cxw2*139)+(-six*cxw*80)+(seven*cxw*25)+(-nine*cxw*45), (-316+cxw2*22)+(six*cxw*15)+(seven*cxw*30)), // 6:shuangxiong
        QPointF((228+cxw2*141)+(-eight*cxw*50)+(-five*cxw*50)+(-nine*cxw*20), (-270-cxw2*3)+(five*cxw*100)), // 7:shenguanyu
        QPointF((228+cxw2*141)+(-four*cxw*70)+(-six*cxw*50), (-70+cxw2)+(-four*cxw*80)+(-six*cxw*50)), // 8:xiaoqiao
    };

    static int indices_table[][9] = {
        {4 }, // 2
        {3, 5}, // 3
        {2-cxw*2, 4, 6+cxw*2}, // 4
        {1, 3, 5, 7}, // 5
        {0, 2, 4, 6, 8}, // 6
        {1-cxw, 2, 3, 5, 6, 7+cxw}, // 7
        {1-cxw, 2-cxw, 3, 4, 5, 6+cxw, 7+cxw}, // 8
        {0, 1, 2, 3, 5, 6, 7, 8}, // 9
        {0, 1, 2, 3, 4, 5, 6, 7, 8} // 10
    };

    static int indices_table_3v3[][5] = {
        {0, 2, 4, 6, 8}, // lord
        {0, 1, 5, 6, 7}, // loyalist (right), same with rebel (right)
        {1, 2, 3, 7, 8}, // rebel (left), same with loyalist (left)
        {0, 2, 4, 6, 8}, // renegade, same with lord
        {0, 1, 5, 6, 7}, // rebel (right)
        {1, 2, 3, 7, 8}, // loyalist (left)
    };

    QList<QPointF> positions;
    int *indices;
    if(ServerInfo.GameMode == "06_3v3" && !Self->getRole().isEmpty())
        indices = indices_table_3v3[Self->getSeat() - 1];
    else
        indices = indices_table[photos.length() - 1];

    qreal stretch_x = dashboard->boundingRect().width() - chat_box->width();
    stretch_x/=1060;
    qreal stretch_y = (state_item->boundingRect().height()
                       + log_box->height()
                       + chat_box->height()
                       + chat_edit->height())/480;

    QPointF offset = QPoint( - chat_box->width()*(1-stretch_x)/2 - 20,
                             - dashboard->boundingRect().height()*(1-stretch_y)/2);


    if(!Config.value("CircularView",false).toBool())
    {
        stretch_x = 1;
        stretch_y = 1;
        offset=QPoint(0,0);
    }

    int i;
    for(i=0; i<photos.length(); i++){
        int index = indices[i];
        QPointF aposition = pos[index];

        aposition.rx()*=stretch_x;
        aposition.ry()*=stretch_y;

        aposition.rx()+=offset.x();
        aposition.ry()+=offset.y();

        positions << aposition;
    }

    return positions;
}

void RoomScene::changeTextEditBackground(){
    chat_box->setStyleSheet("background-color: rgba(0,0,0,50%);");
    log_box->setStyleSheet("background-color: rgba(0,0,0,50%);");
}

void RoomScene::addPlayer(ClientPlayer *player){
    int i;
    for(i=0; i<photos.length(); i++){
        Photo *photo = photos[i];
        if(photo->getPlayer() == NULL){
            photo->setPlayer(player);
            name2photo[player->objectName()] = photo;

            if(!Self->hasFlag("marshalling"))
                Sanguosha->playAudio("add-player");

            return;
        }
    }
}

void RoomScene::removePlayer(const QString &player_name){
    Photo *photo = name2photo[player_name];
    if(photo){
        photo->setPlayer(NULL);
        name2photo.remove(player_name);

        Sanguosha->playAudio("remove-player");
    }
}

void RoomScene::arrangeSeats(const QList<const ClientPlayer*> &seats){
    // rearrange the photos
    Q_ASSERT(seats.length() == photos.length());

    int i, j;
    for(i=0; i<seats.length(); i++){
        const Player *player = seats.at(i);
        for(j=0; j<photos.length(); j++){
            if(photos.at(j)->getPlayer() == player){
                photos.swap(i, j);
                break;
            }
        }
    }

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    QList<QPointF> positions = getPhotoPositions();
    for(i=0; i<positions.length(); i++){
        Photo *photo = photos.at(i);
        photo->setOrder(photo->getPlayer()->getSeat());

        QPropertyAnimation *translation = new QPropertyAnimation(photo, "pos");
        translation->setEndValue(positions.at(i));
        translation->setEasingCurve(QEasingCurve::OutBounce);

        group->addAnimation(translation);

        connect(group, SIGNAL(finished()), photos.at(i), SLOT(updateRoleComboboxPos()));
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);

    // set item to player mapping
    if(item2player.isEmpty()){
        item2player.insert(avatar, Self);
        connect(avatar, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
        connect(avatar, SIGNAL(selected_changed()), this, SLOT(onSelectChange()));
        foreach(Photo *photo, photos){
            item2player.insert(photo, photo->getPlayer());
            connect(photo, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
            connect(photo, SIGNAL(selected_changed()), this, SLOT(onSelectChange()));
            connect(photo, SIGNAL(enable_changed()), this, SLOT(onEnabledChange()));
        }
    }
}

void RoomScene::drawCards(const QList<const Card *> &cards){
    foreach(const Card * card, cards){
        CardItem *item = new CardItem(card);
        item->setPos(DrawPilePos);
        item->setEnabled(false);
        dashboard->addCardItem(item);
    }

    log_box->appendLog("#DrawNCards", Self->getGeneralName(), QStringList(), QString(), QString::number(cards.length()));
}

void RoomScene::drawNCards(ClientPlayer *player, int n){
    QSequentialAnimationGroup *group =  new QSequentialAnimationGroup;
    QParallelAnimationGroup *moving = new QParallelAnimationGroup;
    QParallelAnimationGroup *disappering = new QParallelAnimationGroup;

    Photo *photo = name2photo[player->objectName()];
    int i;
    for(i=0; i<n; i++){
        Pixmap *pixmap = new Pixmap("image/system/card-back.png");
        addItem(pixmap);

        QPropertyAnimation *ugoku = new QPropertyAnimation(pixmap, "pos");
        ugoku->setStartValue(DrawPilePos);
        ugoku->setDuration(800);
        ugoku->setEasingCurve(QEasingCurve::OutQuad);
        ugoku->setEndValue(photo->pos() + QPointF(20 *i, 0));

        QPropertyAnimation *kieru = new QPropertyAnimation(pixmap, "opacity");
        kieru->setKeyValueAt(0, 1.0);
        kieru->setKeyValueAt(0.8, 1.0);
        kieru->setEndValue(0.0);
        kieru->setDuration(800);

        moving->addAnimation(ugoku);
        moving->addAnimation(kieru);

        connect(kieru, SIGNAL(finished()), pixmap, SLOT(deleteLater()));
    }

    group->addAnimation(moving);
    group->addAnimation(disappering);

    group->start(QAbstractAnimation::DeleteWhenStopped);

    photo->update();

    log_box->appendLog(
            "#DrawNCards",
            player->getGeneralName(),
            QStringList(),
            QString(),
            QString::number(n)
            );
}

void RoomScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    foreach(Photo *photo, photos){
        if(photo->isUnderMouse() && photo->isEnabled() && photo->flags() & QGraphicsItem::ItemIsSelectable){
            photo->setSelected(!photo->isSelected());
            return;
        }
    }

    if(avatar->isUnderMouse() && avatar->isEnabled() && avatar->flags() & QGraphicsItem::ItemIsSelectable){
        avatar->setSelected(!avatar->isSelected());
        return;
    }

    QGraphicsScene::mousePressEvent(event);
}

void RoomScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mouseMoveEvent(event);

    QGraphicsObject *obj = static_cast<QGraphicsObject*>(focusItem());
    CardItem *card_item = qobject_cast<CardItem*>(obj);
    if(!card_item || !card_item->isUnderMouse())
        return;

    foreach(Photo *photo, photos){
        if(photo->isUnderMouse()){
            photo->setSelected(true);
            break;
        }
    }

    if(avatar->isUnderMouse()){
        avatar->setSelected(true);
    }
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
    case Qt::Key_F3: sort_combobox->showPopup(); break;

    case Qt::Key_S: dashboard->selectCard("slash");  break;
    case Qt::Key_J: dashboard->selectCard("jink"); break;
    case Qt::Key_P: dashboard->selectCard("peach"); break;

    case Qt::Key_E: dashboard->selectCard("equip"); break;
    case Qt::Key_W: dashboard->selectCard("weapon"); break;
    case Qt::Key_H: dashboard->selectCard("horse"); break;

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
            if(!discarded_queue.isEmpty() && discarded_queue.first()->rotation() == 0.0)
                hideDiscards();
            else if(ClientInstance->getStatus() == Client::Playing){
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

            if(enemy_box && self_box){
                QString msg = QString("enemy:(%1, %2), self:(%3, %4)")
                              .arg(enemy_box->x()).arg(enemy_box->y())
                              .arg(self_box->x()).arg(self_box->y());

                QMessageBox::information(main_window, "", msg);
            }
        }
    }
}

void RoomScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
    QGraphicsScene::contextMenuEvent(event);

    QGraphicsItem *item = itemAt(event->scenePos());
    if(!item)
        return;

    const ClientPlayer *player = item2player.value(item, NULL);
    if(player){
        if(player == Self)
            return;

        QList<const Card *> cards = player->getCards();
        QMenu *menu = known_cards_menu;
        menu->clear();
        menu->setTitle(player->objectName());

        QAction *view = menu->addAction(tr("View in popup window ..."));
        view->setData(player->objectName());
        connect(view, SIGNAL(triggered()), this, SLOT(showPlayerCards()));

        menu->addSeparator();

        if(cards.isEmpty()){
            menu->addAction(tr("There is no known cards"))->setEnabled(false);
        }else{
            foreach(const Card *card, cards)
                menu->addAction(card->getSuitIcon(), card->getFullName());
        }

        // acquired skills
        QSet<QString> skill_names = player->getAcquiredSkills();
        QList<const Skill *> skills;
        foreach(QString skill_name, skill_names){
            const Skill *skill = Sanguosha->getSkill(skill_name);
            if(skill && !skill->inherits("WeaponSkill") && !skill->inherits("ArmorSkill"))
                skills << skill;
        }

        if(!skills.isEmpty()){
            menu->addSeparator();
            foreach(const Skill *skill, skills){
                QString tooltip = skill->getDescription();
                menu->addAction(Sanguosha->translate(skill->objectName()))->setToolTip(tooltip);
            }
        }

        menu->popup(event->screenPos());
    }else if(ServerInfo.FreeChoose && arrange_button){
        QGraphicsObject *obj = item->toGraphicsObject();
        if(obj && Sanguosha->getGeneral(obj->objectName())){
            to_change = qobject_cast<CardItem *>(obj);
            change_general_menu->popup(event->screenPos());
        }
    }
}

void RoomScene::timerEvent(QTimerEvent *event){
    tick ++;

    int timeout = ServerInfo.OperationTimeout;
    if(ClientInstance->getStatus() == Client::AskForGuanxing)
        timeout = Config.S_GUANXING_TIMEOUT;

    int step = 100 / double(timeout * 5);
    int new_value = progress_bar->value() + step;
    new_value = qMin(tick * step, progress_bar->maximum());
    progress_bar->setValue(new_value);

    if(new_value >= progress_bar->maximum()){
        killTimer(event->timerId());
        timer_id = 0;
        tick = 0;
        doTimeout();
    }else{
        progress_bar->setValue(new_value);
    }
}

void RoomScene::chooseGeneral(const QStringList &generals){
    QApplication::alert(main_window);
    if(!main_window->isActiveWindow())
        Sanguosha->playAudio("prelude");

    QDialog *dialog = NULL;
    if(generals.isEmpty())
        dialog = new FreeChooseDialog(main_window);
    else
        dialog = new ChooseGeneralDialog(generals, main_window);

    dialog->exec();
}

void RoomScene::putToDiscard(CardItem *item)
{
    discarded_queue.enqueue(item);
    item->setEnabled(true);
    item->setFlag(QGraphicsItem::ItemIsFocusable, false);
    item->setOpacity(1.0);
    item->setZValue(0.0001*ClientInstance->discarded_list.length());

    viewDiscards();
}

void RoomScene::viewDiscards(){
    if(ClientInstance->discarded_list.isEmpty()){
        QMessageBox::information(NULL, tr("No discarded cards"), tr("There are no discarded cards yet"));
        return;
    }

    if(!sender()->inherits("QAction")){
        int width = getPhotoPositions().last().x() - getPhotoPositions().first().x();
        int mid   = getPhotoPositions().last().x() + getPhotoPositions().first().x();

        width -= photos.first()->boundingRect().width() + 50;
        mid   += 93;
        width = qMin(width,discarded_queue.length()*93);
        width = qMax(width, 200);

        int start = (mid - width)/2;
        int y     = DiscardedPos.y() - 140;
        if(!Config.value("CircularView", false).toBool())
        {
            width = 0;
            start = DiscardedPos.x();
            y     = DiscardedPos.y();
        }

        int i;
        for(i=0; i< discarded_queue.length(); i++){
            CardItem *card_item = discarded_queue.at(i);
            card_item->setEnabled(true);
            card_item->setOpacity(1.0);
            card_item->setHomePos(QPointF(start + i*width/discarded_queue.length(), y));
            QAbstractAnimation* gb;
            if(card_item->zValue()>0)
                gb =card_item->goBack(true,false,false);
            else gb =card_item->goBack();

            if(gb)connect(gb,SIGNAL(finished()),card_item,SLOT(reduceZ()));
        }
    }else{
        CardOverview *overview = new CardOverview;
        overview->loadFromList(ClientInstance->discarded_list);
        overview->show();
    }
}

void RoomScene::hideDiscards(){

    if(discarded_queue.size()<3)return;

    CardItem* top = NULL;
    if(piled_discards.size())top = piled_discards.last();
    foreach(CardItem *card_item,piled_discards)
        if(card_item != top)removeItem(card_item);

    piled_discards.clear();
    if(top)
    {
        piled_discards.append(top);
        top->setZValue(-0.9);
    }

    int i = 1;
    foreach(CardItem *card_item, discarded_queue){
        card_item->setZValue(0.0001 * i++ - 0.8);
        card_item->setHomePos(DiscardedPos);
        card_item->goBack();
        card_item->setEnabled(true);
        card_item->setOpacity(1.0);
        piled_discards.enqueue(card_item);
    }
    discarded_queue.clear();
}

void RoomScene::toggleDiscards(){
    CardOverview *overview = new CardOverview;
    overview->loadFromList(ClientInstance->discarded_list);
    overview->show();
}

CardItem *RoomScene::takeCardItem(ClientPlayer *src, Player::Place src_place, int card_id){
    if(src){
        // from players
        if(src == Self){
            if(src_place == Player::Special){
                CardItem *card_item = card_container->take(NULL, card_id);
                if(card_item)
                    return card_item;
                else{
                    card_item = new CardItem(Sanguosha->getCard(card_id));
                    card_item->setPos(avatar->scenePos());
                    return card_item;
                }
            }

            CardItem *card_item = dashboard->takeCardItem(card_id, src_place);
            if(card_item == NULL)
                return NULL;

            card_item->setParentItem(NULL);
            card_item->setPos(dashboard->mapToScene(card_item->pos()));
            return card_item;
        }else{
            Photo *photo = name2photo.value(src->objectName(), NULL);
            if(photo)
                return photo->takeCardItem(card_id, src_place);
            else
                return NULL;
        }
    }

    // from system, i.e. from draw pile or discard pile
    CardItem *card_item = NULL;

    // from draw pile
    if(src_place == Player::DrawPile){
        card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setPos(DrawPilePos);
        return card_item;
    }

    if(src_place == Player::Special){
        card_item = special_card;
        card_item->hideFrame();
        card_item->showAvatar(NULL);
        special_card = NULL;
        return card_item;
    }

    // from discard pile
    int i;
    for(i=0; i<discarded_queue.length(); i++){
        if(discarded_queue.at(i)->getCard()->getId() == card_id){
            card_item = discarded_queue.takeAt(i);
        }
    }

    if(card_item == NULL){
        card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setPos(DiscardedPos);
    }

    card_item->disconnect(this);

    card_item->promoteZ();
    return card_item;
}

void RoomScene::moveNCards(int n, const QString &from, const QString &to){
    Photo *src = name2photo.value(from, NULL);
    Photo *dest = name2photo.value(to, NULL);

    if(src == NULL || dest == NULL){
        QMessageBox::warning(main_window, tr("Warning"), tr("Can not find moving targets!"));
        return;
    }

    QParallelAnimationGroup *group = new QParallelAnimationGroup;

    int i;
    for(i=0; i<n; i++){
        Pixmap *card_pixmap = new Pixmap("image/system/card-back.png");
        addItem(card_pixmap);

        QPropertyAnimation *ugoku = new QPropertyAnimation(card_pixmap, "pos");
        ugoku->setStartValue(src->pos());
        ugoku->setEndValue(dest->pos() + QPointF(i * 10, 0));
        ugoku->setDuration(1000);

        QPropertyAnimation *kieru = new QPropertyAnimation(card_pixmap, "opacity");
        kieru->setStartValue(0.0);
        kieru->setKeyValueAt(0.2, 1.0);
        kieru->setKeyValueAt(0.8, 1.0);
        kieru->setEndValue(0.0);
        kieru->setDuration(1000);

        group->addAnimation(ugoku);
        group->addAnimation(kieru);

        connect(group, SIGNAL(finished()), card_pixmap, SLOT(deleteLater()));
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);

    src->update();
    dest->update();

    QString type = "#MoveNCards";
    QString from_general = src->getPlayer()->getGeneralName();
    QStringList tos;
    tos << dest->getPlayer()->getGeneralName();
    QString n_str = QString::number(n);
    log_box->appendLog(type, from_general, tos, QString(), n_str);
}

void RoomScene::moveCard(const CardMoveStructForClient &move){
    ClientPlayer *src = move.from;
    ClientPlayer *dest = move.to;
    Player::Place src_place = move.from_place;
    Player::Place dest_place = move.to_place;
    int card_id = move.card_id;

    CardItem *card_item = takeCardItem(src, src_place, card_id);
    if(card_item == NULL)
        return;

    if(src)
        card_item->setOpacity(src == Self ? 1.0 : 0.0);

    if(card_item->scene() == NULL)
        addItem(card_item);

    if(src != NULL && src_place != Player::Judging)
    {
        QString from_general;
        from_general= src->getGeneralName();
        from_general = Sanguosha->translate(from_general);
        putCardItem(dest, dest_place, card_item, from_general);
    }
    else{
        if(src_place == Player::DiscardedPile || dest_place == Player::Hand){
            card_item->deleteCardDesc();
        }
        putCardItem(dest, dest_place, card_item);
    }

    QString card_str = QString::number(card_id);
    if(src && dest){
        if(src == dest)
            return;

        // both src and dest are player
        QString type;
        if(dest_place == Player::Judging){
            const Card *trick = Sanguosha->getCard(move.card_id);
            if(trick->objectName() == "lightning")
                type = "$LightningMove";
            else
                type = "$PasteCard";
        }else if(dest_place == Player::Hand)
            type = "$MoveCard";

        if(!type.isNull()){
            QString from_general = src->objectName();
            QStringList tos;
            tos << dest->objectName();
            log_box->appendLog(type, from_general, tos, card_str);
        }

    }else if(src){
        // src throw card
        if(dest_place == Player::DrawPile){
            QString type = "$PutCard";
            QString from_general = src->getGeneralName();
            log_box->appendLog(type, from_general, QStringList(), card_str);
        }
    }else if(dest){
        if(src_place == Player::DiscardedPile){
            QString type = "$RecycleCard";
            QString from_general = dest->getGeneralName();
            log_box->appendLog(type, from_general, QStringList(), card_str);
        }
    }
}

void RoomScene::putCardItem(const ClientPlayer *dest, Player::Place dest_place, CardItem *card_item, QString show_name){
    if(dest == NULL){
        if(dest_place == Player::DiscardedPile){
            if(!show_name.isEmpty())
                card_item->writeCardDesc(show_name);

            putToDiscard(card_item);
//              if(discarded_queue.length() > 8){
//                CardItem *first = discarded_queue.dequeue();
//                delete first;
//            }

            connect(card_item, SIGNAL(toggle_discards()), this, SLOT(toggleDiscards()));

        }else if(dest_place == Player::DrawPile){
            card_item->setHomePos(DrawPilePos);
            card_item->goBack(true);
        }else if(dest_place == Player::Special){
            special_card = card_item;
            card_item->setHomePos(DrawPilePos);
            card_item->goBack();
        }

    }else if(dest->objectName() == Self->objectName()){
        switch(dest_place){
        case Player::Equip:{
                dashboard->installEquip(card_item);
                break;
            }

        case Player::Hand:{
                dashboard->addCardItem(card_item);
                break;
            }

        case Player::Judging:{
                dashboard->installDelayedTrick(card_item);
                break;
            }

        case Player::Special:{
                card_item->setHomePos(avatar->scenePos());
                card_item->goBack(true);
            }

        default:
            ;
            // FIXME
        }
    }else{
        Photo *photo = name2photo.value(dest->objectName(), NULL);
        if(photo){
            switch(dest_place){
            case Player::Equip:
                photo->installEquip(card_item);
                break;
            case Player::Hand:
                photo->addCardItem(card_item);
                break;
            case Player::Judging:
                photo->installDelayedTrick(card_item);
                break;
            case Player::Special:
                card_item->setHomePos(photo->pos());
                card_item->goBack(true);
                break;
            default:
                ;
            }
        }

        photo->update();
    }
}

void RoomScene::addSkillButton(const Skill *skill, bool from_left){
    if(ClientInstance->getReplayer())
        return;

    // check duplication
    foreach(QAbstractButton *button, skill_buttons){
        if(button->objectName() == skill->objectName())
            return;
    }

    QAbstractButton *button = NULL;

    if(skill->inherits("TriggerSkill")){
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        switch(trigger_skill->getFrequency()){
        case Skill::Frequent:{
                QCheckBox *checkbox = new QCheckBox();
                checkbox->setObjectName(skill->objectName());
                checkbox->setChecked(true);

                button = checkbox;
                break;
        }
        case Skill::Limited:
        case Skill::NotFrequent:{
                const ViewAsSkill *view_as_skill = trigger_skill->getViewAsSkill();
                button = new QPushButton();
                if(view_as_skill){
                    button2skill.insert(button, view_as_skill);
                    connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
                }

                break;
        }

        case Skill::Wake:
        case Skill::Compulsory: button = new QPushButton(); break;
        }
    }else if(skill->inherits("FilterSkill")){
        const FilterSkill *filter = qobject_cast<const FilterSkill *>(skill);
        if(filter && dashboard->getFilter() == NULL)
            dashboard->setFilter(filter);        
        button = new QPushButton();

    }else if(skill->inherits("ViewAsSkill")){
        button = new QPushButton();
        button2skill.insert(button, qobject_cast<const ViewAsSkill *>(skill));
        connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
    }else{
        button = new QPushButton;
    }

    QDialog *dialog = skill->getDialog();
    if(dialog){
        dialog->setParent(main_window, Qt::Dialog);
        connect(button, SIGNAL(clicked()), dialog, SLOT(popup()));
    }

    button->setObjectName(skill->objectName());
    button->setText(skill->getText());
    button->setToolTip(skill->getDescription());
    button->setDisabled(skill->getFrequency() == Skill::Compulsory);
    //button->setStyleSheet(Config.value("style/button").toString());

    if(skill->isLordSkill())
        button->setIcon(QIcon("image/system/roles/lord.png"));

    skill_buttons << button;
    addWidgetToSkillDock(button, from_left);
}

void RoomScene::addWidgetToSkillDock(QWidget *widget, bool from_left){
    if(widget->inherits("QComboBox"))widget->setFixedHeight(20);
    else widget->setFixedHeight(26);

    if(!from_left)
        main_window->statusBar()->addPermanentWidget(widget);
    else
        main_window->statusBar()->addWidget(widget);
}

void RoomScene::removeWidgetFromSkillDock(QWidget *widget){
    QStatusBar * bar = main_window->statusBar();
    bar->removeWidget(widget);
}

void RoomScene::acquireSkill(const ClientPlayer *player, const QString &skill_name){
    QGraphicsObject *dest = getAnimationObject(player->objectName());
    QGraphicsTextItem *item = new QGraphicsTextItem(Sanguosha->translate(skill_name), NULL, this);
    item->setFont(Config.BigFont);

    QGraphicsDropShadowEffect *drop = new QGraphicsDropShadowEffect;
    drop->setBlurRadius(5);
    drop->setOffset(0);
    drop->setColor(Qt::yellow);
    item->setGraphicsEffect(drop);

    QPropertyAnimation *move = new QPropertyAnimation(item, "pos");
    QRectF rect = item->boundingRect();
    move->setStartValue(QPointF(- rect.width()/2, - rect.height()/2));
    move->setEndValue(dest->scenePos());
    move->setDuration(1500);

    move->start(QAbstractAnimation::DeleteWhenStopped);
    connect(move, SIGNAL(finished()), item, SLOT(deleteLater()));

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

    addWidgetToSkillDock(role_combobox);

    // disable all skill buttons
    foreach(QAbstractButton *button, skill_buttons)
        button->setDisabled(true);
}

void RoomScene::updateRoleComboBox(const QString &new_role){
    QMap<QString, QString> normal_mode, boss_mode, threeV3_mode, hegemony_mode;
    normal_mode["lord"] = tr("Lord");
    normal_mode["loyalist"] = tr("Loyalist");
    normal_mode["rebel"] = tr("Rebel");
    normal_mode["renegade"] = tr("Renegade");

    boss_mode["lord"] = tr("Boss");
    boss_mode["loyalist"] = tr("Hero");
    boss_mode["rebel"] = tr("Citizen");
    boss_mode["renegade"] = tr("Guard");

    threeV3_mode["lord"] = threeV3_mode["renegade"] = tr("Marshal");
    threeV3_mode["loyalist"] = threeV3_mode["rebel"] = tr("Vanguard");

    hegemony_mode["lord"] = tr("Wei");
    hegemony_mode["loyalist"] = tr("Shu");
    hegemony_mode["rebel"] = tr("Wu");
    hegemony_mode["renegade"] = tr("Qun");

    QMap<QString, QString> *map = NULL;
    switch(Sanguosha->getRoleIndex()){
    case 2: map = &boss_mode; break;
    case 4: map = &threeV3_mode; break;
    case 5: map = &hegemony_mode; break;
    default:
        map = &normal_mode;
    }

    if(ServerInfo.EnableHegemony){
        QMap<QString, QString> hegemony_roles;

        hegemony_roles["lord"] = "wei";
        hegemony_roles["loyalist"] = "shu";
        hegemony_roles["rebel"] = "wu";
        hegemony_roles["renegade"] = "qun";

        role_combobox->setItemText(1, map->value(new_role));
        role_combobox->setItemIcon(1, QIcon(QString("image/kingdom/icon/%1.png").arg(hegemony_roles[new_role])));
        role_combobox->setCurrentIndex(5);
    }
    else{
        role_combobox->setItemText(1, map->value(new_role));
        role_combobox->setItemIcon(1, QIcon(QString("image/system/roles/%1.png").arg(new_role)));
        role_combobox->setCurrentIndex(1);
    }
}

void RoomScene::enableTargets(const Card *card){
    if(card && (Self->isJilei(card) || Self->isLocked(card))){
        ok_button->setEnabled(false);
        return;
    }

    selected_targets.clear();

    // unset avatar and all photo
    foreach(QGraphicsItem *item, item2player.keys()){
        item->setSelected(false);
    }

    if(card == NULL){
        foreach(QGraphicsItem *item, item2player.keys()){
            //if(!inactive)
                animations->effectOut(item);
                //item->setOpacity(0.7);

            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
            item->setEnabled(true);
        }

        ok_button->setEnabled(false);
        return;
    }

    if(card->targetFixed() || ClientInstance->hasNoTargetResponsing()){
        foreach(QGraphicsItem *item, item2player.keys()){
            //item->setOpacity(1.0);
            animations->effectOut(item);
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
    QMapIterator<QGraphicsItem *, const ClientPlayer *> itor(item2player);
    while(itor.hasNext()){
        itor.next();

        QGraphicsItem *item = itor.key();
        const ClientPlayer *player = itor.value();

        if(item->isSelected())
            continue;

        bool enabled;
        if(card)enabled= !Sanguosha->isProhibited(Self, player, card)
                       && card->targetFilter(selected_targets, player, Self);
        else enabled = true;

        //item->setOpacity(enabled ? 1.0 : 0.7);
        if(enabled)animations->effectOut(item);
        else
        {
            if(item->graphicsEffect() &&
                    item->graphicsEffect()->inherits("SentbackEffect"));
            else animations->sendBack(item);
        }

        if(card)item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    }
}

void RoomScene::updateSelectedTargets(){
    Pixmap *item = qobject_cast<Pixmap *>(sender());

    if(item == NULL)
        return;

    const Card *card = dashboard->getSelected();
    if(card){
        const ClientPlayer *player = item2player[item];
        if(item->isSelected()){
            selected_targets.append(player);
        }else{
            selected_targets.removeOne(player);
        }


        ok_button->setEnabled(card->targetsFeasible(selected_targets, Self));
    }else{
        selected_targets.clear();
    }

    updateTargetsEnablity(card);
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
            ClientInstance->invokeSkill(true);
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
    if(!photo)return;
    if(photo->isSelected())animations->emphasize(photo);
    else animations->effectOut(photo);
    */
}
void RoomScene::onEnabledChange()
{
    QGraphicsItem * photo = qobject_cast<QGraphicsItem*>(sender());
    if(!photo)return;
    if(photo->isEnabled())animations->effectOut(photo);
    else animations->sendBack(photo);
}


void RoomScene::useCard(const Card *card){
    if(card->targetFixed() || card->targetsFeasible(selected_targets, Self))
        ClientInstance->onPlayerUseCard(card, selected_targets);

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
        updateStatus(ClientInstance->getStatus());
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

void RoomScene::selectTarget(int order, bool multiple){
    QGraphicsItem *to_select = NULL;

    if(order == 0)
        to_select = avatar;
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

    if(avatar->flags() & QGraphicsItem::ItemIsSelectable)
        targets << avatar;

    int i, j;
    for(i=0; i<targets.length(); i++){
        if(targets.at(i)->isSelected()){
            for(j=i+1; j<targets.length(); j++){
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
    if(avatar != except)
        avatar->setSelected(false);

    foreach(Photo *photo, photos){
        if(photo != except)
            photo->setSelected(false);
    }
}

void RoomScene::doTimeout(){
    switch(ClientInstance->getStatus()){
    case Client::Responsing:{
            doCancelButton();
            break;
        }

    case Client::Playing:{
            discard_button->click();
            break;
        }

    case Client::Discarding:{
            doCancelButton();

            break;
        }

    case Client::ExecDialog:{
            doCancelButton();

            break;
        }

    case Client::AskForPlayerChoose:{
            ClientInstance->onPlayerChoosePlayer(NULL);
            dashboard->stopPending();
            prompt_box->disappear();
            break;
        }

    case Client::AskForAG:{
            int card_id = card_container->getFirstEnabled();
            if(card_id != -1)
                ClientInstance->onPlayerChooseAG(card_id);

            break;
        }

    case Client::AskForSkillInvoke:
    case Client::AskForYiji:{
            cancel_button->click();
            break;
        }

    case Client::AskForGuanxing:
    case Client::AskForGongxin:{
            ok_button->click();
            break;
        }

    default:
        break;
    }
}

void RoomScene::updateStatus(Client::Status status){
    switch(status){
    case Client::NotActive:{
            prompt_box->disappear();
            ClientInstance->getPromptDoc()->clear();

            dashboard->disableAllCards();
            selected_targets.clear();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            if(dashboard->currentSkill())
                dashboard->stopPending();

            foreach(Photo *photo, photos){
                photo->setOpacity(photo->getPlayer()->isAlive() ? 1.0 : 0.7);
            }

            break;
        }

    case Client::Responsing: {
            prompt_box->appear();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            discard_button->setEnabled(false);

            QString pattern = ClientInstance->getPattern();
            QRegExp rx("@@?(\\w+)!?");
            if(rx.exactMatch(pattern)){
                QString skill_name = rx.capturedTexts().at(1);
                const ViewAsSkill *skill = Sanguosha->getViewAsSkill(skill_name);
                if(skill)
                    dashboard->startPending(skill);
            }else{
                response_skill->setPattern(pattern);
                dashboard->startPending(response_skill);
            }

            break;
        }

    case Client::Playing:{
            dashboard->enableCards();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(true);
            break;
        }

    case Client::Discarding:{
            prompt_box->appear();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(ClientInstance->m_isDiscardActionRefusable);
            discard_button->setEnabled(false);

            discard_skill->setNum(ClientInstance->discard_num);
            discard_skill->setIncludeEquip(ClientInstance->m_canDiscardEquip);
            dashboard->startPending(discard_skill);
            break;
        }

    case Client::ExecDialog:{
            ClientInstance->ask_dialog->setParent(main_window, Qt::Dialog);
            ClientInstance->ask_dialog->exec();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);

            break;
        }

    case Client::AskForSkillInvoke:{
            QString skill_name = ClientInstance->getSkillNameToInvoke();
            foreach(QAbstractButton *button, skill_buttons){
                if(button->objectName() == skill_name){
                    QCheckBox *check_box = qobject_cast<QCheckBox *>(button);
                    if(check_box && check_box->isChecked()){
                        ClientInstance->invokeSkill(true);
                        return;
                    }
                }
            }

            prompt_box->appear();
            ok_button->setEnabled(true);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);

            break;
        }

    case Client::AskForPlayerChoose:{
            prompt_box->appear();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            QString description;
            const Skill *skill = Sanguosha->getSkill(ClientInstance->skill_name);
            if(skill)
                description = skill->getDescription();
            else
                description = Sanguosha->translate(ClientInstance->skill_name);

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

            prompt_box->appear();

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


    foreach(QAbstractButton *button, skill_buttons){
        const ViewAsSkill *skill = button2skill.value(button, NULL);
        if(skill)
            button->setEnabled(skill->isAvailable());
        else
            button->setEnabled(true);
    }

    if(status != Client::NotActive){
        if(focused)
            focused->hideProcessBar();

        QApplication::alert(main_window);
    }

    if(ServerInfo.OperationTimeout == 0)
        return;

    // do timeout
    progress_bar->setValue(0);
    if(timer_id != 0){
        killTimer(timer_id);
        timer_id = 0;
    }

    if(status == Client::NotActive){
        progress_bar->hide();
    }else{
        timer_id = startTimer(200);
        tick = 0;
        progress_bar->show();
    }
}

void RoomScene::doSkillButton(){
    const ViewAsSkill *current = dashboard->currentSkill();
    if(current){
        dashboard->stopPending();
        QAbstractButton *button = button2skill.key(current);
        if(button)
            button->setEnabled(true);
    }

    QPushButton *button = qobject_cast<QPushButton *>(sender());
    const ViewAsSkill *skill = button2skill.value(button, NULL);

    if(skill){
        dashboard->startPending(skill);

        button->setEnabled(false);
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

static bool CompareByNumber(const Card *card1, const Card *card2){
    return card1->getNumber() < card2->getNumber();
}

void RoomScene::updatePileButton(const QString &pile_name){
    QPushButton *button = NULL;
    foreach(QAbstractButton *pile_button, skill_buttons){
        if(pile_button->objectName() == pile_name){
            button = qobject_cast<QPushButton *>(pile_button);
            break;
        }
    }

    QMenu *menu = NULL;
    if(button == NULL){
        QPushButton *push_button = new QPushButton;
        push_button->setObjectName(pile_name);

        skill_buttons << push_button;
        addWidgetToSkillDock(push_button);

        menu = new QMenu(push_button);
        push_button->setMenu(menu);
        button = push_button;
    }else{
        QPushButton *push_button = qobject_cast<QPushButton *>(button);
        menu = push_button->menu();
        if(menu == NULL){
            menu = new QMenu(push_button);
            push_button->setMenu(menu);
        }
    }

    QList<int> pile = Self->getPile(pile_name);
    if(pile.isEmpty())
        button->setText(Sanguosha->translate(pile_name));
    else
        button->setText(QString("%1 (%2)").arg(Sanguosha->translate(pile_name)).arg(pile.length()));

    menu->clear();

    QList<const Card *> cards;
    foreach(int card_id, pile){
        const Card *card = Sanguosha->getCard(card_id);
        cards << card;
    }

    qSort(cards.begin(), cards.end(), CompareByNumber);
    foreach(const Card *card, cards){
        menu->addAction(card->getSuitIcon(), card->getFullName());
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
            const ViewAsSkill *skill = dashboard->currentSkill();
            if(skill)
                cancelViewAsSkill();
            else
                dashboard->unselectAll();
            dashboard->stopPending();
            break;
        }

    case Client::Responsing:{
            QString pattern = ClientInstance->getPattern();
            if(! pattern.startsWith("@")){
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

    case Client::Discarding:{
            dashboard->stopPending();
            ClientInstance->onPlayerDiscardCards(NULL);
            prompt_box->disappear();
            break;
        }

    case Client::ExecDialog:{
            ClientInstance->ask_dialog->reject();
            break;
        }

    case Client::AskForSkillInvoke:{
            ClientInstance->invokeSkill(false);
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

    foreach(Photo *photo, photos)
        photo->hideAvatar();

    dashboard->hideAvatar();
}

void RoomScene::startInXs(){
    if(add_robot) add_robot->hide();
    if(fill_robots) fill_robots->hide();
}

void RoomScene::changeHp(const QString &who, int delta, DamageStruct::Nature nature, bool losthp){
    // update
    Photo *photo = name2photo.value(who, NULL);
    if(photo)
        photo->update();
    else
        dashboard->update();

    QStringList list = QString("%1:%2").arg(who).arg(delta).split(":");
    doAnimation("hpChange",list);

    if(delta < 0){
        if(losthp){
            Sanguosha->playAudio("hplost");
            return;
        }

        QString damage_effect;
        switch(delta){
        case -1: {
                ClientPlayer *player = ClientInstance->getPlayer(who);
                int r = qrand() % 3 + 1;
                if(player->getGeneral()->isMale())
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

        Sanguosha->playAudio(damage_effect);

        if(photo){
            //photo->setEmotion("damage");
            setEmotion(who,"damage");
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

void RoomScene::clearPile(){
    foreach(CardItem *item, piled_discards){
        removeItem(item);
        //delete item;
    }

    piled_discards.clear();
}

void RoomScene::onStandoff(){
    freeze();

#ifdef AUDIO_SUPPORT
    Sanguosha->playAudio("standoff");
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

    Sanguosha->playAudio(win_effect);
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

    dialog->exec();
}

void RoomScene::addRestartButton(QDialog *dialog){
    dialog->resize(main_window->width()/2, dialog->height());

    bool goto_next =false;
    if(Config.GameMode.contains("_mini_") && Self->property("win").toBool())
    {
        QString id = Config.GameMode;
        id.replace("_mini_","");
        int stage = Config.value("MiniSceneStage",1).toInt();
        int current = id.toInt();
        if((stage == current) && stage<20)
            goto_next = true;
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

void RoomScene::FillPlayerNames(QComboBox *combobox, bool add_none){
    if(add_none)
        combobox->addItem(tr("None"), ".");

    combobox->setIconSize(General::TinyIconSize);

    foreach(const ClientPlayer *player, ClientInstance->getPlayers()){
        QString general_name = Sanguosha->translate(player->getGeneralName());
        if(!player->getGeneral()) continue;
        combobox->addItem(QIcon(player->getGeneral()->getPixmapPath("tiny")),
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

    //    labels << tr("Designation") << tr("Kill") << tr("Damage") << tr("Save") << tr("Recover");
    }
    table->setHorizontalHeaderLabels(labels);

    table->setSelectionBehavior(QTableWidget::SelectRows);

    int i;
    for(i=0; i<players.length(); i++){
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
/*
        StatisticsStruct *statistics = player->getStatistics();
        item = new QTableWidgetItem;
        QString designations;
        foreach(QString designation, statistics->designation){
            designations.append(Sanguosha->translate(designation) + ", ");
        }
        designations.remove(designations.length()-3, 2);
        table->setItem(i, 4, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(statistics->kill));
        table->setItem(i, 5, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(statistics->damage));
        table->setItem(i, 6, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(statistics->save));
        table->setItem(i, 7, item);

        item = new QTableWidgetItem;
        item->setText(QString::number(statistics->recover));
        table->setItem(i, 8, item);
*/
    }
}

void RoomScene::killPlayer(const QString &who){
    const General *general = NULL;

    if(who == Self->objectName()){
        dashboard->killPlayer();
        general = Self->getGeneral();
        item2player.remove(avatar);

        if(ServerInfo.GameMode == "02_1v1")
            self_box->killPlayer(Self->getGeneralName());
    }else{
        Photo *photo = name2photo[who];
        photo->killPlayer();
        photo->setFrame(Photo::NoFrame);
        photo->setOpacity(0.7);
        photo->update();
        item2player.remove(photo);

        general = photo->getPlayer()->getGeneral();

        if(ServerInfo.GameMode == "02_1v1")
            enemy_box->killPlayer(general->objectName());
    }

    if(Config.EnableLastWord && !Self->hasFlag("marshalling"))
        general->lastWord();
}

void RoomScene::revivePlayer(const QString &who){
    if(who == Self->objectName()){
        dashboard->revivePlayer();
        item2player.insert(avatar, Self);
        updateSkillButtons();
    }else{
        Photo *photo = name2photo[who];
        photo->revivePlayer();

        item2player.insert(photo, photo->getPlayer());
    }
}

void RoomScene::takeAmazingGrace(const ClientPlayer *taker, int card_id){
    CardItem *copy = card_container->take(taker, card_id);
    if(copy == NULL)
        return;

    addItem(copy);

    if(taker){
        QString type = "$TakeAG";
        QString from_general = taker->getGeneralName();
        QString card_str = QString::number(card_id);
        log_box->appendLog(type, from_general, QStringList(), card_str);

        putCardItem(taker, Player::Hand, copy);
    }else
        putCardItem(NULL, Player::DiscardedPile, copy);
}

void RoomScene::showCard(const QString &player_name, int card_id){
    QString card_str = QString::number(card_id);

    if(player_name == Self->objectName())
        log_box->appendLog("$ShowCard", Self->getGeneralName(), QStringList(), card_str);
    else{
        Photo *photo = name2photo.value(player_name, NULL);
        photo->showCard(card_id);

        log_box->appendLog("$ShowCard", photo->getPlayer()->getGeneralName(),
                           QStringList(), card_str);
    }
}

void RoomScene::chooseSkillButton(){
    QList<QAbstractButton *> enabled_buttons;
    foreach(QAbstractButton *skill_button, skill_buttons){
        if(skill_button->isEnabled())
            enabled_buttons << skill_button;
    }

    if(enabled_buttons.isEmpty())
        return;

    QDialog *dialog = new QDialog(main_window);
    dialog->setWindowTitle(tr("Select skill"));

    QVBoxLayout *layout = new QVBoxLayout;

    foreach(QAbstractButton *skill_button, enabled_buttons){
        QCommandLinkButton *button = new QCommandLinkButton(skill_button->text());
        connect(button, SIGNAL(clicked()), skill_button, SIGNAL(clicked()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
        layout->addWidget(button);
    }

    dialog->setLayout(layout);
    dialog->exec();
}

void RoomScene::attachSkill(const QString &skill_name, bool from_left){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(skill)
        addSkillButton(skill, from_left);
}

void RoomScene::detachSkill(const QString &skill_name){
    QMutableListIterator<QAbstractButton *> itor(skill_buttons);

    while(itor.hasNext()){
        itor.next();

        QAbstractButton *button = itor.value();
        if(button->objectName() == skill_name){
            removeWidgetFromSkillDock(button);
            button2skill.remove(button);
            button->deleteLater();
            itor.remove();

            break;
        }
    }

    if(dashboard->getFilter() == Sanguosha->getSkill(skill_name)){
        dashboard->setFilter(NULL);
    }
}

void RoomScene::viewDistance(){
    DistanceViewDialog *dialog = new DistanceViewDialog(main_window);
    dialog->show();
}

void RoomScene::speak(){
    ClientInstance->speakToServer(chat_edit->text());
    chat_edit->clear();
}

void RoomScene::doGongxin(const QList<int> &card_ids, bool enable_heart){
    card_container->fillCards(card_ids);
    if(enable_heart)
        card_container->startGongxin();
    else
        card_container->addCloseButton();
}

void RoomScene::createStateItem(){
    bool circular = Config.value("CircularView", false).toBool();

    QPixmap state("image/system/state.png");

    state_item = addPixmap(state);//QPixmap("image/system/state.png"));
    state_item->setPos(-110, -80);
    state_item->setZValue(-1.0);
    char roles[100] = {0};
    Sanguosha->getRoles(ServerInfo.GameMode, roles);
    updateStateItem(roles);

    QGraphicsTextItem *text_item = addText("");
    text_item->setParentItem(state_item);
    text_item->setPos(2, 30);
    text_item->setDocument(ClientInstance->getLinesDoc());
    text_item->setTextWidth(220);
    text_item->setDefaultTextColor(Qt::white);

    if(circular)
        state_item->setPos(367, -320);

    add_robot = NULL;
    fill_robots = NULL;
    if(ServerInfo.EnableAI){
        QRectF state_rect = state_item->boundingRect();
        control_panel = addRect(0, 0, state_rect.width(), 150, Qt::NoPen);
        control_panel->setX(state_item->x());
        control_panel->setY(state_item->y() + state_rect.height() + 10);
        control_panel->hide();

        add_robot = new Button(tr("Add a robot"));
        add_robot->setParentItem(control_panel);
        add_robot->setPos(15, 5);

        fill_robots = new Button(tr("Fill robots"));
        fill_robots->setParentItem(control_panel);
        fill_robots->setPos(15, 60);

        connect(add_robot, SIGNAL(clicked()), ClientInstance, SLOT(addRobot()));
        connect(fill_robots, SIGNAL(clicked()), ClientInstance, SLOT(fillRobots()));
        connect(Self, SIGNAL(owner_changed(bool)), this, SLOT(showOwnerButtons(bool)));

        if(circular){
            add_robot->setPos(-565,205);
            fill_robots->setPos(-565, 260);
        }
    }else
        control_panel = NULL;
}

void RoomScene::showOwnerButtons(bool owner){
    if(control_panel && !game_started)
        control_panel->setVisible(owner);
}

void RoomScene::showJudgeResult(const QString &who, const QString &result){
    if(special_card){
        const ClientPlayer *player = ClientInstance->getPlayer(who);

        special_card->showAvatar(player->getGeneral());
        QString desc = QString(tr("%1's judge")).arg(Sanguosha->translate(player->getGeneralName()));
        special_card->writeCardDesc(desc);

        special_card->setFrame(result);
    }
}

void RoomScene::showPlayerCards(){
    QAction *action = qobject_cast<QAction *>(sender());

    if(action){
        QString name = action->data().toString();
        const ClientPlayer *player = ClientInstance->getPlayer(name);

        CardContainer *viewer = new CardContainer();
        viewer->addCloseButton(true);
        addItem(viewer);
        viewer->shift();
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
        Pixmap *avatar = new Pixmap("image/system/1v1/unknown.png");
        avatar->setParentItem(this);
        avatar->setPos(5, 23 + 62 *i);
        avatar->setObjectName("unknown");

        avatars[i] = avatar;
    }

    revealed = 0;
}

void KOFOrderBox::revealGeneral(const QString &name){
    if(revealed < 3){
        const General *general = Sanguosha->getGeneral(name);
        if(general){
            Pixmap *avatar = avatars[revealed ++];
            avatar->changePixmap(general->getPixmapPath("small"));
            avatar->setObjectName(name);
        }
    }
}

void KOFOrderBox::killPlayer(const QString &general_name){
    int i;
    for(i=0; i<revealed; i++){
        Pixmap *avatar = avatars[i];
        if(avatar->isEnabled() && avatar->objectName() == general_name){
            QPixmap pixmap("image/system/death/unknown.png");
            QGraphicsPixmapItem *death = new QGraphicsPixmapItem(pixmap, avatar);
            death->moveBy(10, 0);

            avatar->setOpacity(0.7);
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

    if(ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "02_1v1"){
        selector_box->deleteLater();
        selector_box = NULL;

        chat_widget->show();
        log_box->show();

        if(self_box && enemy_box){
            self_box->show();
            enemy_box->show();
        }
    }

    updateSkillButtons();

    if(control_panel)
        control_panel->hide();

    log_box->append(tr("<font color='white'>------- Game Start --------</font>"));

    // add free discard button
    if(ServerInfo.FreeChoose && !ClientInstance->getReplayer()){
        free_discard = dashboard->addButton("free-discard", 190, true);
        free_discard->setToolTip(tr("Discard cards freely"));
        FreeDiscardSkill *discard_skill = new FreeDiscardSkill(this);
        button2skill.insert(free_discard, discard_skill);
        connect(free_discard, SIGNAL(clicked()), this, SLOT(doSkillButton()));

        skill_buttons << free_discard;
        reLayout();
    }

    updateStatus(ClientInstance->getStatus());

    QList<const ClientPlayer *> players = ClientInstance->getPlayers();
    foreach(const ClientPlayer *player, players){
        connect(player, SIGNAL(phase_changed()), log_box, SLOT(appendSeparator()));
    }

    foreach(Photo *photo, photos)
        photo->createRoleCombobox();

    trust_button->setEnabled(true);


#ifdef AUDIO_SUPPORT

    if(!Config.EnableBgMusic)
        return;

    bool play_music = false;
    if(memory->isAttached() || memory->attach()){
        memory->lock();

        char *username = static_cast<char *>(memory->data());
        const char *my_username = Config.UserName.toAscii();
        play_music = qstrcmp(username, my_username) == 0;

        memory->unlock();
    }else if(memory->create(255)){
        memory->lock();

        void *data = memory->data();
        const char *username = Config.UserName.toAscii();
        memcpy(data, username, qstrlen(username));

        play_music = true;

        memory->unlock();
    }

    if(!play_music)
        return;

    // start playing background music
    QString bgmusic_path = Config.value("BackgroundMusic", "audio/system/background.ogg").toString();

    Audio::playBGM(bgmusic_path);
    Audio::setBGMVolume(Config.BGMVolume);

#endif

    game_started = true;
    drawPile = new Pixmap("image/system/card-back.png");
    addItem(drawPile);
    drawPile->setZValue(-2.0);
    drawPile->setPos(DrawPilePos);
    QGraphicsDropShadowEffect *drp = new QGraphicsDropShadowEffect;
    drp->setOffset(6);
    drp->setColor(QColor(0,0,0));
    drawPile->setGraphicsEffect(drp);
    reLayout(view_transform);
}

void RoomScene::freeze(){

    ClientInstance->disconnectFromHost();
    dashboard->setEnabled(false);
    avatar->setEnabled(false);
    foreach(Photo *photo, photos)
        photo->setEnabled(false);
    item2player.clear();

    chat_edit->setEnabled(false);

#ifdef AUDIO_SUPPORT

    Audio::stopBGM();

#endif

    progress_bar->hide();

    main_window->setStatusBar(NULL);
}

void RoomScene::moveFocus(const QString &who){
    Photo *photo = name2photo[who];
    if(photo){
        if(focused != photo && focused){
            focused->hideProcessBar();
            if(focused->getPlayer()->getPhase() == Player::NotActive)
                focused->setFrame(Photo::NoFrame);
        }

        focused = photo;
        focused->showProcessBar();
        if(focused->getPlayer()->getPhase() == Player::NotActive)
            focused->setFrame(Photo::Responsing);
    }
}

void RoomScene::setEmotion(const QString &who, const QString &emotion ,bool permanent){
    Photo *photo = name2photo[who];
    if(photo){
        photo->setEmotion(emotion,permanent);
        return;
    }
    PixmapAnimation * pma = PixmapAnimation::GetPixmapAnimation(dashboard,emotion);
    if(pma)
    {
        pma->moveBy(0,- dashboard->boundingRect().height()/2);
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
    foreach(CardItem *item, discarded_queue){
        item->show();
    }

    QPropertyAnimation *animation = qobject_cast<QPropertyAnimation *>(sender());
    QGraphicsTextItem *line = qobject_cast<QGraphicsTextItem *>(animation->targetObject());

    removeItem(line->parentItem());
}

QGraphicsObject *RoomScene::getAnimationObject(const QString &name) const{
    if(name == Self->objectName())
        return avatar;
    else
        return name2photo.value(name);
}

void RoomScene::doMovingAnimation(const QString &name, const QStringList &args){
    Pixmap *item = new Pixmap(QString("image/system/animation/%1.png").arg(name));
    addItem(item);

    QPointF from = getAnimationObject(args.at(0))->scenePos();
    QPointF to = getAnimationObject(args.at(1))->scenePos();

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

void RoomScene::animateHpChange(const QString &, const QStringList &args)
{
    QString who = args.at(0);
    const ClientPlayer *player = ClientInstance->getPlayer(who);
    int delta = - args.at(1).toInt();
    int hp = qMax(0, player->getHp() + delta);
    int index = 5;
    if(player->getHp() + delta < player->getMaxHP())
        index = qBound(0, hp, 5);

    if(player == Self)
    {
        int max_hp = Self->getMaxHP();

        qreal width = max_hp > 6 ? 14 : 22;
        qreal total_width = width*max_hp;
        qreal skip = (121 - total_width)/(max_hp+1);
        qreal start_x = dashboard->getRightPosition();

        for(int i=0;i<delta;i++)
        {
            Pixmap *aniMaga = new Pixmap;
            QPixmap *qpixmap = max_hp > 6 ? MagatamaWidget::GetSmallMagatama(index) : MagatamaWidget::GetMagatama(index);
            aniMaga->setPixmap(*qpixmap);
            addItem(aniMaga);
            aniMaga->show();
            i+=hp-delta;

            QPoint pos = QPoint(start_x + skip * (i+1) + i * width,5);
            pos.rx() += dashboard->scenePos().x();
            pos.ry() += dashboard->scenePos().y();
            aniMaga->setPos(pos);

            QPropertyAnimation *fade = new QPropertyAnimation(aniMaga,"opacity");
            fade->setEndValue(0);
            QPropertyAnimation *grow = new QPropertyAnimation(aniMaga,"scale");
            grow->setEndValue(4);

            connect(fade,SIGNAL(finished()),aniMaga,SLOT(deleteLater()));

            QParallelAnimationGroup *group = new QParallelAnimationGroup;
            group->addAnimation(fade);
            group->addAnimation(grow);

            group->start(QAbstractAnimation::DeleteWhenStopped);

            i-=hp-delta;
        }

        return;
    }

    Photo *photo = name2photo[who];
    for(int i=0;i<delta;i++)
    {
        i+=player->getHp();
        Pixmap *aniMaga = new Pixmap(QString("image/system/magatamas/small-%1.png").arg(index));
        addItem(aniMaga);

        QPoint pos = i>=5 ? QPoint(42,69):QPoint(26,86);
        pos.rx() += (i%5)*16;
        pos.rx() += photo->scenePos().x();
        pos.ry() += photo->scenePos().y();
        aniMaga->setPos(pos);

        QPropertyAnimation *fade = new QPropertyAnimation(aniMaga,"opacity");
        fade->setEndValue(0);
        QPropertyAnimation *grow = new QPropertyAnimation(aniMaga,"scale");
        grow->setEndValue(4);

        connect(fade,SIGNAL(finished()),aniMaga,SLOT(deleteLater()));

        QParallelAnimationGroup *group = new QParallelAnimationGroup;
        group->addAnimation(fade);
        group->addAnimation(grow);

        group->start(QAbstractAnimation::DeleteWhenStopped);

        aniMaga->show();

        i-=player->getHp();
    }
}

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

    if(name == "analeptic"
            || name == "peach")
    {
        setEmotion(args.at(0),name);
        return;
    }
    Pixmap *item = new Pixmap(QString("image/system/animation/%1.png").arg(name));
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
    foreach(CardItem *item, discarded_queue){
        item->hide();
    }

    QString word = args.first();
    word = Sanguosha->translate(word);

    QGraphicsRectItem *lightbox = addRect(main_window->rect());

    lightbox->setBrush(QColor(0x20, 0x20, 0x20));
    lightbox->setOpacity(0.8);
    lightbox->moveBy(-main_window->width()/2, -main_window->height()/2);

    QGraphicsTextItem *line = addText(word, Config.BigFont);
    line->setDefaultTextColor(Qt::white);
    QRectF line_rect = line->boundingRect();
    line->setPos(-line_rect.width()/2, -line_rect.height());

    line->setParentItem(lightbox);
    line->setPos(lightbox->mapFromScene(line->x(), line->y()));

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
    foreach(QString arg, args){
        huashen_list << arg;
        CardItem *item = new CardItem(arg);
        item->scaleSmoothly(0.5);

        addItem(item);
        item->setHomePos(avatar->scenePos());
        item->goBack(true);
    }

    Self->tag["Huashens"] = huashen_list;
}

void RoomScene::showIndicator(const QString &from, const QString &to){
    if(Config.value("NoIndicator", false).toBool())
        return;

    QGraphicsObject *obj1 = getAnimationObject(from);
    QGraphicsObject *obj2 = getAnimationObject(to);

    if(obj1 == NULL || obj2 == NULL || obj1 == obj2)
        return;

    if(obj1 == avatar)
        obj1 = dashboard;

    if(obj2 == avatar)
        obj2 = dashboard;

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

        map["hpChange"] = &RoomScene::animateHpChange;
    }

    AnimationFunc func = map.value(name, NULL);
    if(func)
        (this->*func)(name, args);
}

void RoomScene::adjustDashboard(){
    QAction *action = qobject_cast<QAction *>(sender());
    if(action){
        bool expand = action->isChecked();
        dashboard->setWidth(expand ? main_window->width()-10 : 0);
        //adjustItems();
    }
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
    if(Self->getRole() != "lord"){
        QMessageBox::warning(main_window, tr("Warning"), tr("Only lord can surrender!"));
        return;
    }

    int alive_count = Self->aliveCount();
    if(alive_count <= 2){
        QMessageBox::warning(main_window, tr("Warning"), tr("When there are more than 2 players, the lord can surrender!"));
        return;
    }

    QMessageBox::StandardButton button;
    button = QMessageBox::question(main_window, tr("Surrender"), tr("Are you sure to surrender ?"));
    if(button == QMessageBox::Ok){
        ClientInstance->surrender();
    }
}

void RoomScene::fillGenerals1v1(const QStringList &names){
    selector_box = new Pixmap("image/system/1v1/select.png", true);
    addItem(selector_box);
    selector_box->shift();

    const static int start_x = 43;
    const static int width = 86;
    const static int row_y[4] = {60, 60+120, 60+120*2, 60+120*3};

    foreach(QString name, names){
        CardItem *item =  new CardItem(name);
        item->setObjectName(name);
        general_items << item;
    }

    qShuffle(general_items);

    int i, n=names.length();
    for(i=0; i<n; i++){

        int row, column;
        if(i < 5){
            row = 1;
            column = i;
        }else{
            row = 2;
            column = i - 5;
        }

        CardItem *general_item = general_items.at(i);
        general_item->scaleSmoothly(0.4);
        general_item->setParentItem(selector_box);
        general_item->setPos(start_x + width * column, row_y[row]);
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
    selector_box = new Pixmap(path, true);
    addItem(selector_box);
    selector_box->setZValue(guanxing_box->zValue());
    selector_box->shift();

    const static int start_x = 62;
    const static int width = 148-62;
    const static int row_y[4] = {85, 206, 329, 451};

    int i, n=names.length();
    for(i=0; i<n; i++){

        int row, column;
        if(i < 8){
            row = 1;
            column = i;
        }else{
            row = 2;
            column = i - 8;
        }

        CardItem *general_item = new CardItem(names.at(i));
        general_item->scaleSmoothly(0.4);
        general_item->setParentItem(selector_box);
        general_item->setPos(start_x + width * column, row_y[row]);
        general_item->setHomePos(general_item->pos());
        general_item->setObjectName(names.at(i));

        general_items << general_item;
    }
}

void RoomScene::fillGenerals(const QStringList &names){
    chat_widget->hide();
    log_box->hide();

    if(ServerInfo.GameMode == "06_3v3")
        fillGenerals3v3(names);
    else if(ServerInfo.GameMode == "02_1v1")
        fillGenerals1v1(names);
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

    int x,y;
    if(ServerInfo.GameMode == "06_3v3"){
        x = 62 + (to_add->length() - 1) * (148-62);
        y = self_taken ? 451 : 85;
    }else{
        x = 43 + (to_add->length() - 1) * 86;
        y = self_taken ? 60+120*3 : 60;
    }

    general_item->setHomePos(QPointF(x, y));
    general_item->goBack();
}

void RoomScene::recoverGeneral(int index, const QString &name){
    QString obj_name = QString("x%1").arg(index);

    foreach(CardItem *item, general_items){
        if(item->objectName() == obj_name){
            item->changeGeneral(name);
            item->scaleSmoothly(0.4);
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

void RoomScene::startArrange(){
    QString mode;
    QList<QPointF> positions;
    if(ServerInfo.GameMode == "06_3v3"){
        mode = "3v3";
        positions << QPointF(233, 291)
                << QPointF(361, 291)
                << QPointF(489, 291);
    }else{
        mode = "1v1";
        positions << QPointF(84, 269)
                << QPointF(214, 269)
                << QPointF(344, 269);
    }

    selector_box->changePixmap(QString("image/system/%1/arrange.png").arg(mode));

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
    for(i=0; i<3; i++){
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
    for(i=0; i<n; i++){
        QPointF pos = arrange_rects.at(i)->pos();
        CardItem *item = arrange_items.at(i);
        item->setHomePos(pos);
        item->goBack();
    }

    while(arrange_items.length() > 3){
        CardItem *last = arrange_items.takeLast();
        down_generals << last;
    }

    for(i=0; i<down_generals.length(); i++){
        QPointF pos;
        if(ServerInfo.GameMode == "06_3v3")
            pos = QPointF(62 + i*86, 451);
        else
            pos = QPointF(43 + i*86, 60 + 120 * 3);

        CardItem *item = down_generals.at(i);
        item->setHomePos(pos);
        item->goBack();
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

    Pixmap::MakeGray(pixmap);
    map[qc.toLower()] = pixmap;
}

void RoomScene::updateStateItem(const QString &roles)
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
            item->setPos(21*role_items.length(), 6);
            item->setParentItem(state_item);

            role_items << item;
        }
    }
}

void RoomScene::adjustPrompt()
{
    static int fitSize = 140 ;
    QGraphicsTextItem *text_item = prompt_box->findChild<QGraphicsTextItem*>();
    int height = ClientInstance->getPromptDoc()->size().height();

    QFont ft=text_item->font();
    int fz = ft.pixelSize() * qSqrt(fitSize*1.0/height);
    if(fz > 21)fz = 21;

    ft.setPixelSize(fz);
    text_item->setFont(ft);

    while(ClientInstance->getPromptDoc()->size().height()>fitSize)
    {
        ft.setPixelSize(ft.pixelSize()-1);
        text_item->setFont(ft);
    }
    //else text_item->setFont(QFont("SimHei",10));
}

void RoomScene::reLayout(QMatrix matrix)
{
    if(matrix.m11()>1)matrix.setMatrix(1,0,0,1,matrix.dx(),matrix.dy());
    view_transform = matrix;
    //if(!Config.value("circularView",false).toBool())
    //    if(!game_started)return;

    QPoint pos = QPoint(dashboard->getMidPosition(),0);

    int skip = 10;
    int padding_left = 5;
    int padding_top = -5;

    pos.rx()+= padding_left;
    pos.ry()+= padding_top;

    //alignTo(trust_button,pos,"xlyb");
//    alignTo(untrust_button,pos,"xlyb");
//    pos.rx()+=trust_button->width();
//    pos.rx()+=skip;

    alignTo(reverse_button,pos,"xlyb");
    pos.rx()+=reverse_button->width();
    pos.rx()+=skip*2;


    if(free_discard)
    {
        alignTo(free_discard,pos,"xlyb");
        pos.rx()+=free_discard->width();
        pos.rx()+=skip;
    }

    pos = QPoint(dashboard->boundingRect().width()-dashboard->getRightPosition(),0);

    pos.rx()-= padding_left;
    pos.ry()+=padding_top;

//    alignTo(discard_button,pos,"xryb");
//    pos.rx()-=discard_button->width();
//    pos.rx()-=skip;

//    alignTo(cancel_button,pos,"xryb");
//    pos.rx()-=cancel_button->width();
//    pos.rx()-=skip;

//    alignTo(ok_button,pos,"xryb");
//    pos.rx()-=ok_button->width();
//    pos.rx()-=skip;
    //ok_button->move(-10,-10);


    if(!Config.value("circularView",false).toBool())
    {
        pos.ry() = state_item->y();
        pos.rx() = state_item->x()-padding_left;
        alignTo(chat_box_widget,pos,"xryt");

        pos.rx() = state_item->x() + state_item->boundingRect().width() + padding_left;
        alignTo(log_box,pos,"xlyt");

        log_box->setFixedHeight(chat_box->height() + chat_edit->height());
    }
    else
    {
        pos.ry() = -main_window->height()/2/matrix.m22() + 30;
        pos.ry() -= padding_top*2;

        pos.rx() = main_window->width()/2/matrix.m22() - padding_left
                - chat_box->width()/2;
                //state_item->x() + state_item->boundingRect().width()/2;

        int height = main_window->height()/matrix.m22() - dashboard->boundingRect().height();
        //height    += padding_top;
        height    -= 60.0;
        height    -= chat_edit->height()*2 + state_item->boundingRect().height();

        chat_box->setFixedHeight(height/2);
        chat_edit->move(0,chat_box->height());
        log_box->setFixedHeight(height/2);

        alignTo(state_item,pos,"xmyt");

        pos.ry()+=state_item->boundingRect().height();
        alignTo(log_box,pos,"xmyt");

        pos.ry()+=log_box->height();
        alignTo(chat_box_widget,pos,"xmyt");
    }

    chat_widget->setX(chat_box_widget->x()+chat_edit->width() - 77);
    chat_widget->setY(chat_box_widget->y()+chat_box->height() + 9);

}

void RoomScene::alignTo(Pixmap *object, QPoint pos, const QString &flags)
{
    if(object == NULL)return;
    QPointF to = object->pos();
    if(flags.contains("xl"))to.rx() = pos.x();
    else if(flags.contains("xr"))to.rx() = pos.x() - object->boundingRect().width();
    else if(flags.contains("xm"))to.rx() = pos.x() - object->boundingRect().width()/2;

    if(flags.contains("yt"))to.ry() = pos.y();
    else if(flags.contains("yb"))to.ry() = pos.y() - object->boundingRect().height();
    else if(flags.contains("ym"))to.ry() = pos.y() - object->boundingRect().height()/2;

    object->setPos(to);
}

void RoomScene::alignTo(QWidget *object, QPoint pos, const QString &flags)
{
    if(object == NULL)return;
    QPoint to = object->pos();
    if(flags.contains("xl"))to.rx() = pos.x();
    else if(flags.contains("xr"))to.rx() = pos.x() - object->width();
    else if(flags.contains("xm"))to.rx() = pos.x() - object->width()/2;

    if(flags.contains("yt"))to.ry() = pos.y();
    else if(flags.contains("yb"))to.ry() = pos.y() - object->height();
    else if(flags.contains("ym"))to.ry() = pos.y() - object->height()/2;

    object->move(to.x(),to.y());
}

void RoomScene::alignTo(QGraphicsItem* object, QPoint pos, const QString &flags)
{
    if(object == NULL)return;
    QPointF to = object->pos();
    if(flags.contains("xl"))to.rx() = pos.x();
    else if(flags.contains("xr"))to.rx() = pos.x() - object->boundingRect().width();
    else if(flags.contains("xm"))to.rx() = pos.x() - object->boundingRect().width()/2;

    if(flags.contains("yt"))to.ry() = pos.y();
    else if(flags.contains("yb"))to.ry() = pos.y() - object->boundingRect().height();
    else if(flags.contains("ym"))to.ry() = pos.y() - object->boundingRect().height()/2;

    object->setPos(to);
}


void RoomScene::appendChatEdit(QString txt){
    chat_edit->setText(chat_edit->text()+" "+txt);
    chat_edit->setFocus();
}

void RoomScene::appendChatBox(QString txt){
    QString prefix = "<img src='image/system/chatface/";
    QString suffix = ".png'></img>";
    txt=txt.replace("<#", prefix);
    txt=txt.replace("#>", suffix);
    chat_box->append(txt);
}
