#include "roomscene.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "cardoverview.h"
#include "distanceviewdialog.h"
#include "choosegeneraldialog.h"
#include "joystick.h"
#include "irrKlang.h"
#include "window.h"
#include "button.h"
#include "cardcontainer.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QListWidget>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QCheckBox>
#include <QGraphicsProxyWidget>
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

extern irrklang::ISoundEngine *SoundEngine;

static QPointF DiscardedPos(-6, -2);
static QPointF DrawPilePos(-102, -2);

RoomScene::RoomScene(QMainWindow *main_window)
    :focused(NULL), special_card(NULL), viewing_discards(false),
    main_window(main_window)
{
    int player_count = Sanguosha->getPlayerCount(ServerInfo.GameMode);

    bool circular = Config.value("CircularView", false).toBool();
    if(circular){
        DiscardedPos = QPointF(-140, -60);
        DrawPilePos = QPointF(-260, -60);
    }

    // create photos
    int i;
    for(i=0;i<player_count-1;i++){
        Photo *photo = new Photo;
        photos << photo;
        addItem(photo);
    }

    {
        // create dashboard
        dashboard = new Dashboard;
        addItem(dashboard);

        dashboard->setPlayer(Self);
        connect(Self, SIGNAL(general_changed()), dashboard, SLOT(updateAvatar()));
        connect(Self, SIGNAL(general2_changed()), dashboard, SLOT(updateSmallAvatar()));
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

    // add buttons that above the equipment area of dashboard
    trust_button = dashboard->addButton("trust", circular ? 10 : 4, true);
    untrust_button = dashboard->addButton("untrust", circular ? 10 : 4, true);
    connect(trust_button, SIGNAL(clicked()), ClientInstance, SLOT(trust()));
    connect(untrust_button, SIGNAL(clicked()), ClientInstance, SLOT(trust()));
    connect(Self, SIGNAL(state_changed()), this, SLOT(updateTrustButton()));
    untrust_button->hide();

    // add buttons that above the avatar area of dashbaord
    if(circular){
        ok_button = dashboard->addButton("ok", -245-146, false);
        cancel_button = dashboard->addButton("cancel", -155-146, false);
        discard_button = dashboard->addButton("discard", -70-146, false);
        dashboard->setWidth(main_window->width()-10);
    }else{
        ok_button = dashboard->addButton("ok", -72, false);
        cancel_button = dashboard->addButton("cancel", -7, false);
        discard_button = dashboard->addButton("discard", 75, false);
    }

    connect(ok_button, SIGNAL(clicked()), this, SLOT(doOkButton()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(doCancelButton()));
    connect(discard_button, SIGNAL(clicked()), this, SLOT(doDiscardButton()));

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
    connect(ClientInstance, SIGNAL(hp_changed(QString,int,DamageStruct::Nature)), SLOT(changeHp(QString,int,DamageStruct::Nature)));
    connect(ClientInstance, SIGNAL(pile_cleared()), this, SLOT(clearPile()));
    connect(ClientInstance, SIGNAL(player_killed(QString)), this, SLOT(killPlayer(QString)));
    connect(ClientInstance, SIGNAL(player_revived(QString)), this, SLOT(revivePlayer(QString)));
    connect(ClientInstance, SIGNAL(card_shown(QString,int)), this, SLOT(showCard(QString,int)));
    connect(ClientInstance, SIGNAL(gongxin(QList<int>, bool)), this, SLOT(doGongxin(QList<int>, bool)));
    connect(ClientInstance, SIGNAL(focus_moved(QString)), this, SLOT(moveFocus(QString)));
    connect(ClientInstance, SIGNAL(emotion_set(QString,QString)), this, SLOT(setEmotion(QString,QString)));
    connect(ClientInstance, SIGNAL(skill_invoked(QString,QString)), this, SLOT(showSkillInvocation(QString,QString)));
    connect(ClientInstance, SIGNAL(skill_acquired(const ClientPlayer*,QString)),
            this, SLOT(acquireSkill(const ClientPlayer*,QString)));
    connect(ClientInstance, SIGNAL(animated(QString,QStringList)),
            this, SLOT(doAnimation(QString,QStringList)));
    connect(ClientInstance, SIGNAL(judge_result(QString,QString)), this, SLOT(showJudgeResult(QString,QString)));

    connect(ClientInstance, SIGNAL(game_started()), this, SLOT(onGameStart()));
    connect(ClientInstance, SIGNAL(game_over(bool,QList<bool>)), this, SLOT(onGameOver(bool,QList<bool>)));
    connect(ClientInstance, SIGNAL(standoff()), this, SLOT(onStandoff()));

    connect(ClientInstance, SIGNAL(card_moved(CardMoveStructForClient)), this, SLOT(moveCard(CardMoveStructForClient)));
    connect(ClientInstance, SIGNAL(n_cards_moved(int,QString,QString)), this, SLOT(moveNCards(int,QString,QString)));

    connect(ClientInstance, SIGNAL(cards_drawed(QList<const Card*>)), this, SLOT(drawCards(QList<const Card*>)));
    connect(ClientInstance, SIGNAL(n_cards_drawed(ClientPlayer*,int)), SLOT(drawNCards(ClientPlayer*,int)));

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

        connect(card_container, SIGNAL(item_chosen(int)), ClientInstance, SLOT(chooseAG(int)));
        connect(card_container, SIGNAL(item_gongxined(int)), ClientInstance, SLOT(replyGongxin(int)));

        connect(ClientInstance, SIGNAL(ag_filled(QList<int>)), card_container, SLOT(fillCards(QList<int>)));
        connect(ClientInstance, SIGNAL(ag_taken(const ClientPlayer*,int)), this, SLOT(takeAmazingGrace(const ClientPlayer*,int)));
        connect(ClientInstance, SIGNAL(ag_cleared()), card_container, SLOT(clear()));

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
        chat_box->resize(230 + widen_width, 195);

        QGraphicsProxyWidget *chat_box_widget = addWidget(chat_box);
        chat_box_widget->setPos(-343 - widen_width, -83);
        chat_box_widget->setZValue(-2.0);
        QPalette palette;
        palette.setBrush(QPalette::Base, backgroundBrush());
        chat_box->setPalette(palette);
        chat_box->setReadOnly(true);
        chat_box->setTextColor(Config.TextEditColor);
        connect(ClientInstance, SIGNAL(words_spoken(QString)), chat_box, SLOT(append(QString)));

        // chat edit
        chat_edit = new QLineEdit;
        chat_edit->setPlaceholderText(tr("Please enter text to chat ... "));
        chat_edit->setFixedWidth(chat_box->width());

        QGraphicsProxyWidget *chat_edit_widget = new QGraphicsProxyWidget(chat_box_widget);
        chat_edit_widget->setWidget(chat_edit);
        chat_edit_widget->setX(0);
        chat_edit_widget->setY(chat_box->height());
        connect(chat_edit, SIGNAL(returnPressed()), this, SLOT(speak()));

        if(circular){
            chat_box->resize(268, 180);
            chat_box_widget->setPos(367 , -38);

            chat_edit->setFixedWidth(chat_box->width());
            chat_edit->setFixedHeight(24);
            chat_edit_widget->setX(0);
            chat_edit_widget->setY(chat_box->height()+1);
        }

        if(ServerInfo.DisableChat)
            chat_edit_widget->hide();
    }

    {
        // log box
        log_box = new ClientLogBox;
        log_box->resize(chat_box->width(), 213);
        log_box->setTextColor(Config.TextEditColor);

        QGraphicsProxyWidget *log_box_widget = addWidget(log_box);
        log_box_widget->setPos(114, -83);
        log_box_widget->setZValue(-2.0);
        connect(ClientInstance, SIGNAL(log_received(QString)), log_box, SLOT(appendLog(QString)));

        if(circular){
            log_box->resize(chat_box->width(), 210);
            log_box_widget->setPos(367, -246);
        }
    }

    {
        prompt_box = new Window(tr("Sanguosha"), QSize(480, 200));
        prompt_box->setOpacity(0.8);
        prompt_box->setFlag(QGraphicsItem::ItemIsMovable);
        prompt_box->shift();
        prompt_box->setZValue(10);
        prompt_box->keepWhenDisappear();

        QGraphicsTextItem *text_item = new QGraphicsTextItem(prompt_box);
        text_item->setPos(66, 45);
        text_item->setDefaultTextColor(Qt::white);

        QTextDocument *prompt_doc = ClientInstance->getPromptDoc();
        text_item->setDocument(prompt_doc);

        addItem(prompt_box);
    }

    memory = new QSharedMemory("QSanguosha", this);

    progress_bar = dashboard->addProgressBar();
    timer_id = 0;
    tick = 0;

    skill_dock = new QDockWidget(main_window);
    skill_dock->setTitleBarWidget(new QWidget);
    skill_dock->titleBarWidget()->hide();
    main_window->addDockWidget(Qt::BottomDockWidgetArea, skill_dock);

    addWidgetToSkillDock(sort_combobox, true);

    adjustItems();

    if(Config.value("JoystickEnabled", false).toBool()){
        Joystick *js = new Joystick(this);
        connect(js, SIGNAL(button_clicked(int)), this, SLOT(onJoyButtonClicked(int)));
        connect(js, SIGNAL(direction_clicked(int)), this, SLOT(onJoyDirectionClicked(int)));

        js->start();
    }

    createStateItem();
}

void RoomScene::adjustItems(){
    qreal dashboard_width = dashboard->boundingRect().width();
    qreal x = - dashboard_width/2;
    qreal main_height = main_window->centralWidget()->height();
    qreal y = main_height/2 - dashboard->boundingRect().height();

    dashboard->setPos(x, y);

    QList<QPointF> positions = getPhotoPositions();
    int i;
    for(i=0; i<positions.length(); i++)
        photos.at(i)->setPos(positions.at(i));
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
        {0, 3, 4, 5, 8}, // lord
        {0, 1, 4, 5, 6}, // loyalist (right), same with rebel (right)
        {2, 3, 4, 7, 8}, // rebel (left), same with loyalist (left)
        {0, 3, 4, 5, 8}, // renegade, same with lord
        {0, 1, 4, 5, 6}, // rebel (right)
        {2, 3, 4, 7, 8}, // loyalist (left)
    };

    QList<QPointF> positions;
    int *indices;
    if(ServerInfo.GameMode == "06_3v3" && !Self->getRole().isEmpty())
        indices = indices_table_3v3[Self->getSeat() - 1];
    else
        indices = indices_table[photos.length() - 1];

    int i;
    for(i=0; i<photos.length(); i++){
        int index = indices[i];
        positions << pos[index];
    }

    return positions;
}

void RoomScene::changeTextEditBackground(){
    QPalette palette;
    QPalette l_palette;
    QPixmap chat_pixmap;
    QPixmap log_pixmap;
    QBrush brush;
    QBrush chat_brush;
    QBrush log_brush;
    bool circular = Config.value("CircularView", false).toBool();
    if(circular){
        chat_pixmap=QPixmap("image/system/chat_background.png");
        log_pixmap=QPixmap("image/system/log_background.png");
        chat_brush=(chat_pixmap);
        log_brush=(log_pixmap);
        palette.setBrush(QPalette::Base, chat_brush);
        l_palette.setBrush(QPalette::Base, log_brush);
        log_box->setPalette(l_palette);
        chat_box->setPalette(palette);
    }else{
        brush=(backgroundBrush().texture());
        palette.setBrush(QPalette::Base, brush);
        log_box->setPalette(palette);
        chat_box->setPalette(palette);
    }
}

void RoomScene::addPlayer(ClientPlayer *player){
    int i;
    for(i=0; i<photos.length(); i++){
        Photo *photo = photos[i];
        if(photo->getPlayer() == NULL){
            photo->setPlayer(player);
            name2photo[player->objectName()] = photo;
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
        photo->setOrder(i+1);

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
        foreach(Photo *photo, photos){
            item2player.insert(photo, photo->getPlayer());
            connect(photo, SIGNAL(selected_changed()), this, SLOT(updateSelectedTargets()));
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
        ugoku->setDuration(500);
        ugoku->setEasingCurve(QEasingCurve::OutBounce);
        ugoku->setEndValue(photo->pos() + QPointF(20 *i, 0));

        QPropertyAnimation *kieru = new QPropertyAnimation(pixmap, "opacity");
        kieru->setDuration(900);
        kieru->setKeyValueAt(0.8, 1.0);
        kieru->setEndValue(0.0);

        moving->addAnimation(ugoku);
        disappering->addAnimation(kieru);

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
    case Qt::Key_7: selectTarget(event->key() - Qt::Key_0, control_is_down); break;

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
            menu->addAction(tr("There is no known cards"));
        }else{
            foreach(const Card *card, cards)
                menu->addAction(card->getSuitIcon(), card->getFullName());
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
        timeout = 20;

    /*
    if(ClientInstance->getStatus() == Client::Responsing &&
       ClientInstance->card_pattern == "nullification")
       timeout = Config.NullificationCountDown;
       */

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

void RoomScene::viewDiscards(){
    if(ClientInstance->discarded_list.isEmpty()){
        QMessageBox::information(NULL, tr("No discarded cards"), tr("There are no discarded cards yet"));
        return;
    }

    if(sender()->inherits("CardItem")){
        int i;
        for(i=0; i< discarded_queue.length(); i++){
            CardItem *card_item = discarded_queue.at(i);
            card_item->setHomePos(QPointF(card_item->x() + i*card_item->boundingRect().width(), card_item->y()));
            card_item->goBack();
        }
    }else{
        CardOverview *overview = new CardOverview;
        overview->loadFromList(ClientInstance->discarded_list);
        overview->show();
    }
}

void RoomScene::hideDiscards(){
    foreach(CardItem *card_item, discarded_queue){
        card_item->setHomePos(DiscardedPos);
        card_item->goBack();
    }
}

void RoomScene::toggleDiscards(){
    viewing_discards = ! viewing_discards;

    if(viewing_discards)
        viewDiscards();
    else
        hideDiscards();
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
        kieru->setStartValue(1.0);
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

    card_item->setOpacity(1.0);

    if(card_item->scene() == NULL)
        addItem(card_item);

    putCardItem(dest, dest_place, card_item);

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

        QString from_general = src->getGeneralName();
        QStringList tos;
        tos << dest->getGeneralName();
        log_box->appendLog(type, from_general, tos, card_str);
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

void RoomScene::putCardItem(const ClientPlayer *dest, Player::Place dest_place, CardItem *card_item){
    if(dest == NULL){
        if(dest_place == Player::DiscardedPile){
            card_item->setHomePos(DiscardedPos);
            card_item->goBack();
            card_item->setEnabled(true);

            card_item->setFlag(QGraphicsItem::ItemIsFocusable, false);

            card_item->setZValue(0.0001*ClientInstance->discarded_list.length());
            discarded_queue.enqueue(card_item);

            if(discarded_queue.length() > 8){
                CardItem *first = discarded_queue.dequeue();
                delete first;
            }

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
                card_item->setEnabled(false);
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
                checkbox->setChecked(false);
                connect(checkbox, SIGNAL(stateChanged(int)), ClientInstance, SLOT(updateFrequentFlags(int)));
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

                    // Guhuo special case
                    if(skill->objectName() == "guhuo"){
                        GuhuoDialog *dialog = GuhuoDialog::GetInstance();
                        dialog->setParent(main_window, Qt::Dialog);
                        connect(button, SIGNAL(clicked()), dialog, SLOT(popup()));
                    }
                }

                break;
        }

        case Skill::Compulsory:{
                button = new QPushButton();
                break;
            }

        default:
            break;
        }
    }else if(skill->inherits("FilterSkill")){
        const FilterSkill *filter = qobject_cast<const FilterSkill *>(skill);
        if(filter && dashboard->getFilter() == NULL){
            dashboard->setFilter(filter);
            button = new QPushButton();
        }
    }else if(skill->inherits("ViewAsSkill")){
        button = new QPushButton();
        button2skill.insert(button, qobject_cast<const ViewAsSkill *>(skill));
        connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
    }else{
        button = new QPushButton;
    }

    button->setObjectName(skill->objectName());
    button->setText(skill->getText());
    button->setToolTip(skill->getDescription());
    button->setDisabled(skill->getFrequency() == Skill::Compulsory);

    if(skill->isLordSkill())
        button->setIcon(QIcon("image/system/roles/lord.png"));

    skill_buttons << button;
    addWidgetToSkillDock(button, from_left);
}

void RoomScene::addWidgetToSkillDock(QWidget *widget, bool from_left){
    widget->setFixedHeight(30);

    QWidget *container = skill_dock->widget();
    QHBoxLayout *container_layout = NULL;
    if(container == NULL){
        container = new QWidget;
        QHBoxLayout *layout = new QHBoxLayout;
        QMargins margins = layout->contentsMargins();
        margins.setTop(0);
        margins.setBottom(5);
        layout->setContentsMargins(margins);
        container->setLayout(layout);
        layout->addStretch();

        skill_dock->setWidget(container);

        container_layout = layout;
    }else{
        QLayout *layout = container->layout();
        container_layout = qobject_cast<QHBoxLayout *>(layout);
    }

    if(from_left)
        container_layout->insertWidget(0, widget);
    else
        container_layout->addWidget(widget);
}

void RoomScene::removeWidgetFromSkillDock(QWidget *widget){
    QWidget *container = skill_dock->widget();
    if(container)
        container->layout()->removeWidget(widget);
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
    const QList<const Skill*> skills = Self->getVisibleSkills();
    foreach(const Skill* skill, skills){
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
    QMap<QString, QString> normal_mode, boss_mode, challenge_mode, threeV3_mode;
    normal_mode["lord"] = tr("Lord");
    normal_mode["loyalist"] = tr("Loyalist");
    normal_mode["rebel"] = tr("Rebel");
    normal_mode["renegade"] = tr("Renegade");

    boss_mode["lord"] = tr("Boss");
    boss_mode["loyalist"] = tr("Hero");
    boss_mode["rebel"] = tr("Citizen");
    boss_mode["renegade"] = tr("Guard");

    challenge_mode["lord"] = challenge_mode["loyalist"] = tr("Defense");
    challenge_mode["rebel"] = challenge_mode["renegade"] = tr("Attack");

    threeV3_mode["lord"] = threeV3_mode["renegade"] = tr("Marshal");
    threeV3_mode["loyalist"] = threeV3_mode["rebel"] = tr("Vanguard");

    QMap<QString, QString> *map = NULL;
    switch(Sanguosha->getRoleIndex()){
    case 2: map = &boss_mode; break;
    case 3: map = &challenge_mode; break;
    case 4: map = &threeV3_mode; break;
    default:
        map = &normal_mode;
    }

    role_combobox->setItemText(1, map->value(new_role));
    role_combobox->setItemIcon(1, QIcon(QString("image/system/roles/%1.png").arg(new_role)));
    role_combobox->setCurrentIndex(1);
}

void RoomScene::enableTargets(const Card *card){
    if(ClientInstance->getStatus() == Client::AskForCardShow && card){
        ok_button->setEnabled(true);
        return;
    }

    if(card && ClientInstance->isJilei(card)){
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
            item->setEnabled(false);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }

        ok_button->setEnabled(false);
        return;
    }

    if(card->targetFixed() || ClientInstance->noTargetResponsing()){
        foreach(QGraphicsItem *item, item2player.keys()){
            item->setEnabled(true);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }

        ok_button->setEnabled(true);
        return;
    }

    updateTargetsEnablity(card);

    if(Config.EnableAutoTarget)
        selectNextTarget(false);

    ok_button->setEnabled(card->targetsFeasible(selected_targets));
}

void RoomScene::updateTargetsEnablity(const Card *card){
    QMapIterator<QGraphicsItem *, const ClientPlayer *> itor(item2player);
    while(itor.hasNext()){
        itor.next();

        QGraphicsItem *item = itor.key();
        const ClientPlayer *player = itor.value();

        if(item->isSelected())
            continue;

        bool enabled = !ClientInstance->isProhibited(player, card) && card->targetFilter(selected_targets, player);
        item->setEnabled(enabled);
        item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
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

        updateTargetsEnablity(card);
        ok_button->setEnabled(card->targetsFeasible(selected_targets));
    }else{
        selected_targets.clear();
    }
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
                if(ClientInstance->noTargetResponsing())
                    ClientInstance->responseCard(card);
                else
                    ClientInstance->useCard(card, selected_targets);
                prompt_box->disappear();
            }

            dashboard->unselectAll();
            break;
        }

    case Client::Discarding: {
            const Card *card = dashboard->pendingCard();
            if(card){
                ClientInstance->discardCards(card);
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
            ClientInstance->chooseAG(-1);
            return;
        }

    case Client::AskForCardShow:{
            const Card *card = dashboard->getSelected();
            if(card){
                ClientInstance->responseCard(card);
                dashboard->unselectAll();
            }

            break;
        }

    case Client::ExecDialog:{
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is in executing dialog"));
            return;
        }

    case Client::AskForPlayerChoose:{
            ClientInstance->choosePlayer(selected_targets.first());
            prompt_box->disappear();

            break;
        }

    case Client::AskForYiji:{
            const Card *card = dashboard->pendingCard();
            if(card){
                ClientInstance->replyYiji(card, selected_targets.first());
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
            ClientInstance->replyGongxin();
            card_container->clear();

            break;
        }
    }

    const ViewAsSkill *skill = dashboard->currentSkill();
    if(skill)
        dashboard->stopPending();
}

void RoomScene::useCard(const Card *card){
    if(card->targetFixed() || card->targetsFeasible(selected_targets))
        ClientInstance->useCard(card, selected_targets);

    enableTargets(NULL);
}

void RoomScene::callViewAsSkill(){
    const Card *card = dashboard->pendingCard();

    if(card == NULL)
        return;    

    if(card->isAvailable()){
        // use card
        dashboard->stopPending();
        useCard(card);
    }
}

void RoomScene::cancelViewAsSkill(){
    const ViewAsSkill *skill = dashboard->currentSkill();
    dashboard->stopPending();
    QAbstractButton *button = button2skill.key(skill, NULL);

    if(button)
        updateStatus(ClientInstance->getStatus());
}

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
            ClientInstance->choosePlayer(NULL);
            dashboard->stopPending();
            prompt_box->disappear();
            break;
        }

    case Client::AskForAG:{
            int card_id = card_container->getFirstEnabled();
            if(card_id != -1)
                ClientInstance->chooseAG(card_id);

            break;
        }

    case Client::AskForCardShow:{
            ClientInstance->responseCard(NULL);
            break;
        }

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

            break;
        }

    case Client::Responsing: {
            prompt_box->appear();
            if(ClientInstance->card_pattern.startsWith("@"))
                dashboard->disableAllCards();
            else
                dashboard->enableCards(ClientInstance->card_pattern);

            ok_button->setEnabled(false);

            QString pattern = ClientInstance->card_pattern;
            if(pattern.startsWith("@")){
                QRegExp rx("@@?(\\w+)!?");
                if(rx.exactMatch(pattern)){
                    QString skill_name = rx.capturedTexts().at(1);
                    const ViewAsSkill *skill = Sanguosha->getViewAsSkill(skill_name);
                    if(skill)
                        dashboard->startPending(skill);
                }
            }

            cancel_button->setEnabled(ClientInstance->refusable);
            discard_button->setEnabled(false);
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
            cancel_button->setEnabled(ClientInstance->refusable);
            discard_button->setEnabled(false);

            discard_skill->setNum(ClientInstance->discard_num);
            discard_skill->setIncludeEquip(ClientInstance->include_equip);
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

    case Client::AskForPlayerChoose:{
            prompt_box->appear();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            ClientInstance->getPromptDoc()->setHtml(tr("Please choose a player"));

            choose_skill->setPlayerNames(ClientInstance->players_to_choose);
            dashboard->startPending(choose_skill);

            break;
        }

    case Client::AskForAG:{
            dashboard->disableAllCards();

            ok_button->setEnabled(ClientInstance->refusable);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            card_container->startChoose();

            break;
        }

    case Client::AskForCardShow:{
            prompt_box->appear();
            dashboard->enableAllCards();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }

    case Client::AskForYiji:{
            ok_button->setEnabled(false);
            cancel_button->setEnabled(true);
            discard_button->setEnabled(false);

            yiji_skill->setCards(ClientInstance->card_pattern);
            dashboard->startPending(yiji_skill);

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
    bool trusting = Self->getState() == "trust";
    trust_button->setVisible(!trusting);
    untrust_button->setVisible(trusting);

    dashboard->setTrust(trusting);
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
            if(!ClientInstance->card_pattern.startsWith("@")){
                const ViewAsSkill *skill = dashboard->currentSkill();
                if(skill){
                    cancelViewAsSkill();
                    break;
                }
            }

            if(ClientInstance->noTargetResponsing())
                ClientInstance->responseCard(NULL);
            else
                ClientInstance->useCard(NULL, QList<const ClientPlayer *>());
            prompt_box->disappear();
            dashboard->stopPending();
            break;
        }

    case Client::Discarding:{
            dashboard->stopPending();
            ClientInstance->discardCards(NULL);
            prompt_box->disappear();
            break;
        }

    case Client::ExecDialog:{
            ClientInstance->ask_dialog->reject();
            break;
        }

    case Client::AskForYiji:{
            dashboard->stopPending();
            ClientInstance->replyYiji(NULL, NULL);
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
        ClientInstance->useCard(NULL);
    }
}

void RoomScene::hideAvatars(){
    if(control_panel)
        control_panel->hide();

    foreach(Photo *photo, photos)
        photo->hideAvatar();

    dashboard->hideAvatar();
}

void RoomScene::changeHp(const QString &who, int delta, DamageStruct::Nature nature){
    // update
    Photo *photo = name2photo.value(who, NULL);
    if(photo)
        photo->update();
    else
        dashboard->update();

    if(delta < 0){
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
            photo->setEmotion("damage");
            photo->tremble();
        }

        if(nature == DamageStruct::Fire)
            doAnimation("fire", QStringList() << who);
        else if(nature == DamageStruct::Thunder)
            doAnimation("lightning", QStringList() << who);

    }else if(delta > 0){
        QString type = "#Recover";
        QString from_general = ClientInstance->getPlayer(who)->getGeneralName();
        QString n = QString::number(delta);

        log_box->appendLog(type, from_general, QStringList(), QString(), n);
    }
}

void RoomScene::clearPile(){
    foreach(CardItem *item, discarded_queue){
        removeItem(item);
        delete item;
    }

    discarded_queue.clear();
}

void RoomScene::onStandoff(){
    freeze();

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

void RoomScene::onGameOver(bool victory, const QList<bool> &result_list){
    freeze();

    Sanguosha->playAudio(victory ? "win" : "lose");

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

    winner_table->setColumnCount(4);
    loser_table->setColumnCount(4);

    QList<const ClientPlayer *> players = ClientInstance->getPlayers();
    QList<const ClientPlayer *> winner_list, loser_list;
    int i;
    for(i=0; i<players.length(); i++){
        const ClientPlayer *player = players.at(i);
        bool result = result_list.at(i);

        if(result)
            winner_list << player;            
        else
            loser_list << player;

        if(player != Self){
            Photo *photo = name2photo.value(player->objectName());
            photo->setEmotion(result ? "good" : "bad", true);
        }
    }

    winner_table->setRowCount(winner_list.length());
    loser_table->setRowCount(loser_list.length());

    fillTable(winner_table, winner_list);
    fillTable(loser_table, loser_list);

    addRestartButton(dialog);

    dialog->exec();
}

void RoomScene::addRestartButton(QDialog *dialog){
    dialog->resize(main_window->width()/2, dialog->height());

    QPushButton *restart_button = new QPushButton(tr("Restart Game"));
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(restart_button);

    QPushButton *save_button = new QPushButton(tr("Save record"));
    hlayout->addWidget(save_button);

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(dialog->layout());
    if(layout)
        layout->addLayout(hlayout);

    connect(restart_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(save_button, SIGNAL(clicked()), this, SLOT(saveReplayRecord()));
    connect(dialog, SIGNAL(accepted()), this, SIGNAL(restart()));
}

void RoomScene::saveReplayRecord(){
    QString location = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    QString last_dir = Config.value("LastReplayDir").toString();
    if(!last_dir.isEmpty())
        location = last_dir;

    QString filename = QFileDialog::getSaveFileName(main_window,
                                                    tr("Save replay record"),
                                                    location,
                                                    tr("Replay file (*.txt)"));

    if(filename.isEmpty())
        return;

    QFileInfo file_info(filename);
    last_dir = file_info.absoluteDir().path();
    Config.setValue("LastReplayDir", last_dir);

    ClientInstance->save(filename);
}

DamageMakerDialog::DamageMakerDialog(QWidget *parent)
    :QDialog(parent)
{
    setWindowTitle(tr("Damage maker"));

    damage_source = new QComboBox;
    damage_source->addItem(tr("None"), ".");
    fillCombobox(damage_source);

    damage_target = new QComboBox;
    fillCombobox(damage_target);

    damage_nature = new QComboBox;
    damage_nature->addItem(tr("Normal"), "N");
    damage_nature->addItem(tr("Thunder"), "T");
    damage_nature->addItem(tr("Fire"), "F");
    damage_nature->addItem(tr("HP recover"), "R");
    damage_nature->addItem(tr("Lose HP"), "L");

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
    connect(this, SIGNAL(accepted()), this, SLOT(makeDamage()));
}

void DamageMakerDialog::disableSource(){
    QString nature = damage_nature->itemData(damage_nature->currentIndex()).toString();
    damage_source->setEnabled(nature != "L");
}

void DamageMakerDialog::fillCombobox(QComboBox *combobox){
    combobox->setIconSize(General::TinyIconSize);

    foreach(const ClientPlayer *player, ClientInstance->getPlayers()){
        QString general_name = Sanguosha->translate(player->getGeneralName());
        combobox->addItem(QIcon(player->getGeneral()->getPixmapPath("tiny")),
                          QString("%1 [%2]").arg(general_name).arg(player->screenName()),
                          player->objectName());
    }
}

void DamageMakerDialog::makeDamage(){
    ClientInstance->request(QString("useCard :%1->%2:%3%4")
                            .arg(damage_source->itemData(damage_source->currentIndex()).toString())
                            .arg(damage_target->itemData(damage_target->currentIndex()).toString())
                            .arg(damage_nature->itemData(damage_nature->currentIndex()).toString())
                            .arg(damage_point->value()));
}

void RoomScene::makeDamage(){
    if(Self->getPhase() != Player::Play){
        QMessageBox::warning(main_window, tr("Warning"), tr("This function is only allowed at your play phase!"));
        return;
    }

    DamageMakerDialog *damage_maker = new DamageMakerDialog(main_window);
    damage_maker->exec();
}

void RoomScene::fillTable(QTableWidget *table, const QList<const ClientPlayer *> &players){
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    static QStringList labels;
    if(labels.isEmpty())
        labels << tr("General") << tr("Name") << tr("Alive") << tr("Role");
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
        QIcon icon(QString("image/system/roles/%1.png").arg(player->getRole()));
        item->setIcon(icon);
        if(!player->isAlive())
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setText(Sanguosha->translate(player->getRole()));
        table->setItem(i, 3, item);
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
        photo->setEnabled(false);
        photo->update();

        general = photo->getPlayer()->getGeneral();

        QMutableMapIterator<QGraphicsItem *, const ClientPlayer *> itor(item2player);
        while(itor.hasNext()){
            itor.next();
            if(itor.value()->objectName() == who){
                itor.key()->setEnabled(false);
                itor.remove();
                break;
            }
        }

        if(ServerInfo.GameMode == "02_1v1")
            enemy_box->killPlayer(general->objectName());
    }

    if(Config.EnableLastWord)
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

            return;
        }
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

    QPixmap state;
    if(circular)
        state=QPixmap("image/system/state2.png");
    else
        state=QPixmap("image/system/state.png");

    QGraphicsItem *state_item = addPixmap(state);//QPixmap("image/system/state.png"));
    state_item->setPos(-110, -90);
    char roles[100] = {0}, *role;
    Sanguosha->getRoles(ServerInfo.GameMode, roles);
    for(role = roles; *role!='\0'; role++){
        static QPixmap lord("image/system/roles/small-lord.png");
        static QPixmap loyalist("image/system/roles/small-loyalist.png");
        static QPixmap rebel("image/system/roles/small-rebel.png");
        static QPixmap renegade("image/system/roles/small-renegade.png");

        QPixmap *to_add = NULL;
        switch(*role){
        case 'Z': to_add = &lord; break;
        case 'C': to_add = &loyalist; break;
        case 'N': to_add = &renegade; break;
        case 'F': to_add = &rebel; break;
        default:
            break;
        }

        if(to_add){
            QGraphicsPixmapItem *item = addPixmap(*to_add);
            item->setPos(21*role_items.length(), 6);
            item->setParentItem(state_item);

            role_items << item;
        }
    }

    QGraphicsTextItem *text_item = addText("");
    text_item->setParentItem(state_item);
    text_item->setPos(2, 30);
    text_item->setDocument(ClientInstance->getLinesDoc());
    text_item->setTextWidth(220);
    text_item->setDefaultTextColor(Qt::white);

    if(circular)
        state_item->setPos(367, -338);

    if(ServerInfo.EnableAI){
        QRectF state_rect = state_item->boundingRect();
        control_panel = addRect(0, 0, state_rect.width(), 150, Qt::NoPen);
        control_panel->setX(state_item->x());
        control_panel->setY(state_item->y() + state_rect.height() + 10);
        control_panel->hide();

        Button *add_robot = new Button(tr("Add a robot"));
        add_robot->setParentItem(control_panel);
        add_robot->setPos(15, 5);

        Button *fill_robots = new Button(tr("Fill robots"));
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
    if(control_panel && !trust_button->isEnabled())
        control_panel->setVisible(owner);
}

void RoomScene::showJudgeResult(const QString &who, const QString &result){
    if(special_card){
        const ClientPlayer *player = ClientInstance->getPlayer(who);

        special_card->showAvatar(player->getGeneral());
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

static irrklang::ISound *BackgroundMusic;

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
            avatar->setEnabled(false);

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

GuhuoDialog *GuhuoDialog::GetInstance(){
    static GuhuoDialog *instance;
    if(instance == NULL)
        instance = new GuhuoDialog;

    return instance;
}

GuhuoDialog::GuhuoDialog()
{
    setWindowTitle(tr("Guhuo"));

    group = new QButtonGroup(this);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(createLeft());
    layout->addWidget(createRight());

    setLayout(layout);

    connect(group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(selectCard(QAbstractButton*)));
}

void GuhuoDialog::popup(){
    if(ClientInstance->getStatus() != Client::Playing)
        return;

    foreach(QAbstractButton *button, group->buttons()){
        const Card *card = map[button->objectName()];
        button->setEnabled(card->isAvailable());
    }

    Self->tag.remove("Guhuo");
    exec();
}

void GuhuoDialog::selectCard(QAbstractButton *button){
    CardStar card = map.value(button->objectName());
    Self->tag["Guhuo"] = QVariant::fromValue(card);
    accept();
}

QGroupBox *GuhuoDialog::createLeft(){
    QGroupBox *box = new QGroupBox;
    box->setTitle(tr("Basic cards"));

    QVBoxLayout *layout = new QVBoxLayout;

    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->getTypeId() == Card::Basic && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setParent(this);

            layout->addWidget(createButton(c));
        }
    }

    layout->addStretch();

    box->setLayout(layout);
    return box;
}

QGroupBox *GuhuoDialog::createRight(){
    QGroupBox *box = new QGroupBox(tr("Non delayed tricks"));
    QHBoxLayout *layout = new QHBoxLayout;

    QGroupBox *box1 = new QGroupBox(tr("Single target"));
    QVBoxLayout *layout1 = new QVBoxLayout;

    QGroupBox *box2 = new QGroupBox(tr("Multiple targets"));
    QVBoxLayout *layout2 = new QVBoxLayout;


    QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
    foreach(const Card *card, cards){
        if(card->isNDTrick() && !map.contains(card->objectName())){
            Card *c = Sanguosha->cloneCard(card->objectName(), Card::NoSuit, 0);
            c->setParent(this);

            QVBoxLayout *layout = c->inherits("SingleTargetTrick") ? layout1 : layout2;
            layout->addWidget(createButton(c));
        }
    }

    box->setLayout(layout);
    box1->setLayout(layout1);
    box2->setLayout(layout2);

    layout1->addStretch();
    layout2->addStretch();

    layout->addWidget(box1);
    layout->addWidget(box2);
    return box;
}

QAbstractButton *GuhuoDialog::createButton(const Card *card){
    QCommandLinkButton *button = new QCommandLinkButton(Sanguosha->translate(card->objectName()));
    button->setObjectName(card->objectName());
    button->setToolTip(card->getDescription());

    map.insert(card->objectName(), card);
    group->addButton(button);

    return button;
}

void RoomScene::onGameStart(){
    if(ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "02_1v1"){
        selector_box->deleteLater();
        selector_box = NULL;

        chat_box->show();
        log_box->show();

        if(self_box && enemy_box){
            self_box->show();
            enemy_box->show();
        }
    }

    updateSkillButtons();

    if(control_panel)
        control_panel->hide();

    log_box->append(tr("------- Game Start --------"));

    // add free discard button
    if(ServerInfo.FreeChoose){
        QPushButton *free_discard = dashboard->addButton("free_discard", 100, true);
        free_discard->setToolTip(tr("Discard cards freely"));
        FreeDiscardSkill *discard_skill = new FreeDiscardSkill(this);
        button2skill.insert(free_discard, discard_skill);
        connect(free_discard, SIGNAL(clicked()), this, SLOT(doSkillButton()));

        skill_buttons << free_discard;
    }

    trust_button->setEnabled(true);
    untrust_button->setEnabled(true);
    updateStatus(ClientInstance->getStatus());

    QList<const ClientPlayer *> players = ClientInstance->getPlayers();
    foreach(const ClientPlayer *player, players){
        connect(player, SIGNAL(phase_changed()), log_box, SLOT(appendSeparator()));
    }

    foreach(Photo *photo, photos)
        photo->createRoleCombobox();

    if(!Config.EnableBgMusic || SoundEngine == NULL)
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
    QString bgmusic_path = Config.value("BackgroundMusic", "audio/system/background.mp3").toString();
    const char *filename = bgmusic_path.toLocal8Bit().data();
    BackgroundMusic = SoundEngine->play2D(filename, true, false, true);

    if(BackgroundMusic)
        BackgroundMusic->setVolume(Config.Volume);
}

void RoomScene::freeze(){
    main_window->removeDockWidget(skill_dock);
    delete skill_dock;
    skill_dock = NULL;

    ClientInstance->disconnectFromHost();
    dashboard->setEnabled(false);
    avatar->setEnabled(false);
    foreach(Photo *photo, photos)
        photo->setEnabled(false);
    item2player.clear();
    trust_button->setEnabled(false);
    chat_edit->setEnabled(false);
    if(BackgroundMusic)
        BackgroundMusic->stop();

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

void RoomScene::setEmotion(const QString &who, const QString &emotion){
    Photo *photo = name2photo[who];
    if(photo){
        photo->setEmotion(emotion);
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

void RoomScene::doAppearingAnimation(const QString &name, const QStringList &args){
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

void RoomScene::doLightboxAnimation(const QString &name, const QStringList &args){
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
    appear->setDuration(2000);

    appear->start();

    connect(appear, SIGNAL(finished()), this, SLOT(removeLightBox()));
}

void RoomScene::doAnimation(const QString &name, const QStringList &args){
    static QMap<QString, AnimationFunc> map;
    if(map.isEmpty()){
        map["peach"] = &RoomScene::doMovingAnimation;
        map["nullification"] = &RoomScene::doMovingAnimation;

        map["analeptic"] = &RoomScene::doAppearingAnimation;
        map["fire"] = &RoomScene::doAppearingAnimation;
        map["lightning"] = &RoomScene::doAppearingAnimation;
        map["typhoon"] = &RoomScene::doAppearingAnimation;

        map["lightbox"] = &RoomScene::doLightboxAnimation;
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
        adjustItems();
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
        general_item->setScale(0.4);
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
        general_item->setScale(0.4);
        general_item->setParentItem(selector_box);
        general_item->setPos(start_x + width * column, row_y[row]);
        general_item->setHomePos(general_item->pos());
        general_item->setObjectName(names.at(i));

        general_items << general_item;
    }
}

void RoomScene::fillGenerals(const QStringList &names){
    chat_box->hide();
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
