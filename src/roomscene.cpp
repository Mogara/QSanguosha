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

extern irrklang::ISoundEngine *SoundEngine;

static const QPointF DiscardedPos(-6, -2);
static const QPointF DrawPilePos(-102, -2);
static const QPointF TinyAvatarOffset(44, 87);

RoomScene::RoomScene(int player_count, QMainWindow *main_window)
    :focused(NULL), special_card(NULL), viewing_discards(false),
    main_window(main_window)
{
    ClientInstance->setParent(this);

    // create photos
    int i;
    for(i=0;i<player_count-1;i++){
        Photo *photo = new Photo(i);
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
        connect(Self, SIGNAL(general_changed()), this, SLOT(updateSkillButtons()));
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

    // add role combobox
    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Your role"));
    role_combobox->addItem(tr("Unknown"));
    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(updateRoleComboBox(QString)));

    // add buttons that above the equipment area of dashboard
    trust_button = dashboard->addButton(tr("Trust"), 4, true);
    connect(trust_button, SIGNAL(clicked()), ClientInstance, SLOT(trust()));
    connect(Self, SIGNAL(state_changed()), this, SLOT(updateTrustButton()));

    QPushButton *expand_button = new QPushButton(tr("Expand to window width"));
    dashboard->addWidget(expand_button, 67, true);
    connect(expand_button, SIGNAL(clicked()), this, SLOT(adjustDashboard()));

    // add buttons that above the avatar area of dashbaord
    ok_button = dashboard->addButton(tr("OK"), -72, false);
    cancel_button = dashboard->addButton(tr("Cancel"), -7, false);
    discard_button = dashboard->addButton(tr("Discard cards"), 75, false);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(doOkButton()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(doCancelButton()));
    connect(discard_button, SIGNAL(clicked()), this, SLOT(doDiscardButton()));

    discard_skill = new DiscardSkill;
    yiji_skill = new YijiViewAsSkill;
    choose_skill = new ChoosePlayerSkill;

    known_cards_menu = new QMenu(main_window);

    // get dashboard's avatar
    avatar = dashboard->getAvatar();

    // do signal-slot connections
    connect(ClientInstance, SIGNAL(player_added(ClientPlayer*)), SLOT(addPlayer(ClientPlayer*)));
    connect(ClientInstance, SIGNAL(player_removed(QString)), SLOT(removePlayer(QString)));    
    connect(ClientInstance, SIGNAL(generals_got(QList<const General*>)), this, SLOT(chooseGeneral(QList<const General*>)));
    connect(ClientInstance, SIGNAL(seats_arranged(QList<const ClientPlayer*>)), SLOT(arrangeSeats(QList<const ClientPlayer*>)));
    connect(ClientInstance, SIGNAL(status_changed(Client::Status)), this, SLOT(updateStatus(Client::Status)));
    connect(ClientInstance, SIGNAL(avatars_hiden()), this, SLOT(hideAvatars()));
    connect(ClientInstance, SIGNAL(hp_changed(QString,int)), this, SLOT(changeHp(QString,int)));
    connect(ClientInstance, SIGNAL(pile_cleared()), this, SLOT(clearPile()));
    connect(ClientInstance, SIGNAL(player_killed(QString)), this, SLOT(killPlayer(QString)));
    connect(ClientInstance, SIGNAL(card_shown(QString,int)), this, SLOT(showCard(QString,int)));
    connect(ClientInstance, SIGNAL(guanxing(QList<int>)), this, SLOT(doGuanxing(QList<int>)));
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

    connect(ClientInstance, SIGNAL(ag_filled(QList<int>)), this, SLOT(fillAmazingGrace(QList<int>)));
    connect(ClientInstance, SIGNAL(ag_taken(const ClientPlayer*,int)), this, SLOT(takeAmazingGrace(const ClientPlayer*,int)));
    connect(ClientInstance, SIGNAL(ag_cleared()), this, SLOT(clearAmazingGrace()));

    connect(ClientInstance, SIGNAL(skill_attached(QString)), this, SLOT(attachSkill(QString)));
    connect(ClientInstance, SIGNAL(skill_detached(QString)), this, SLOT(detachSkill(QString)));

    int widen_width = 0;
    if(player_count != 6 && player_count <= 8)
        widen_width = 148;

    {
        // chat box
        chat_box = new QTextEdit;
        chat_box->resize(230 + widen_width, 213);

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

        QGraphicsProxyWidget *chat_edit_widget = new QGraphicsProxyWidget(chat_box_widget);
        chat_edit_widget->setWidget(chat_edit);
        chat_edit_widget->setX(widen_width + 10);
        chat_edit_widget->setY(chat_box->height());
        connect(chat_edit, SIGNAL(returnPressed()), this, SLOT(speak()));

        if(ServerInfo.DisableChat)
            chat_edit_widget->hide();
    }


    // log box
    log_box = new ClientLogBox;
    log_box->resize(chat_box->size());
    log_box->setTextColor(Config.TextEditColor);

    QGraphicsProxyWidget *log_box_widget = addWidget(log_box);
    log_box_widget->setPos(114, -83);
    log_box_widget->setZValue(-2.0);
    connect(ClientInstance, SIGNAL(log_received(QString)), log_box, SLOT(appendLog(QString)));

    {
        prompt_box = new Window(tr("Sanguosha"), QSize(480, 177));
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

    Joystick *js = new Joystick(this);
    connect(js, SIGNAL(button_clicked(int)), this, SLOT(onJoyButtonClicked(int)));
    connect(js, SIGNAL(direction_clicked(int)), this, SLOT(onJoyDirectionClicked(int)));

    js->start();

    createStateItem();

    judge_avatar = new QGraphicsPixmapItem;
    judge_avatar->setPos(DrawPilePos + TinyAvatarOffset);
    judge_avatar->setZValue(10.0);

    addItem(judge_avatar);
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
    static const QPointF pos[] = {
        QPointF(-501, -69), // 0:zhugeliang
        QPointF(-501, -273), // 1:wolong
        QPointF(-356, -294), // 2:shenzhugeliang
        QPointF(-211, -294), // 3:lusu
        QPointF(-66, -294), // 4:dongzhuo
        QPointF(79, -294), // 5:caocao
        QPointF(224, -296), // 6:shuangxiong
        QPointF(369, -273), // 7:shenguanyu
        QPointF(369, -69), // 8:xiaoqiao
    };

    static int indices_table[][9] = {
        {4 }, // 2
        {3, 5}, // 3
        {2, 4, 6}, // 4
        {1, 3, 5, 7}, // 5
        {0, 2, 4, 6, 8}, // 6
        {1, 2, 3, 5, 6, 7}, // 7
        {1, 2, 3, 4, 5, 6, 7}, // 8
        {0, 1, 2, 3, 5, 6, 7, 8}, // 9
        {0, 1, 2, 3, 4, 5, 6, 7, 8} // 10
    };

    QList<QPointF> positions;
    int *indices = indices_table[photos.length() - 1];

    int i;
    for(i=0; i<photos.length(); i++){
        int index = indices[i];
        positions << pos[index];
    }

    return positions;
}

void RoomScene::changeTextEditBackground(){
    QPalette palette;
    QBrush brush(backgroundBrush().texture());
    palette.setBrush(QPalette::Base, brush);

    log_box->setPalette(palette);
    chat_box->setPalette(palette);
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
        QPropertyAnimation *translation = new QPropertyAnimation(photos.at(i), "pos");
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
        ugoku->setEndValue(photo->pos() + QPointF(10 *i, 0));

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
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4: sort_combobox->setCurrentIndex(event->key() - Qt::Key_F1); break;

    case Qt::Key_F5:
    case Qt::Key_F6:
    case Qt::Key_F7:
    case Qt::Key_F8: break;

    case Qt::Key_F9:
    case Qt::Key_F10:
    case Qt::Key_F11:
    case Qt::Key_F12: clickSkillButton(event->key() - Qt::Key_F9); break;

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

    }
}

void RoomScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
    QGraphicsScene::contextMenuEvent(event);

    QGraphicsItem *item = itemAt(event->scenePos());
    if(!item)
        return;

    const ClientPlayer *player = item2player.value(item, NULL);
    if(player && player != Self){
        QList<const Card *> cards = player->getCards();
        QMenu *menu = known_cards_menu;
        menu->clear();
        menu->setTitle(player->objectName());

        if(cards.isEmpty()){
            menu->addAction(tr("There is no known cards"));
        }else{
            foreach(const Card *card, cards)
                menu->addAction(card->getSuitIcon(), card->getFullName());
        }

        menu->popup(event->screenPos());
    }
}

void RoomScene::timerEvent(QTimerEvent *event){
    tick ++;

    int timeout = ServerInfo.OperationTimeout;
    if(ClientInstance->getStatus() == Client::AskForGuanxing)
        timeout = 20;

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

        int left_tick = timeout * 5 - tick;
        if(left_tick % 5 == 0 && left_tick < 20)
            Sanguosha->playAudio("count-down");
    }
}

void RoomScene::chooseGeneral(const QList<const General *> &generals){
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
                if(amazing_grace.isEmpty()){
                    CardItem *card_item = new CardItem(Sanguosha->getCard(card_id));
                    card_item->setPos(avatar->scenePos());
                    return card_item;
                }else{
                    takeAmazingGrace(NULL, card_id);
                    return NULL;
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
        judge_avatar->hide();
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
        if(dest_place == Player::DiscardedPile){
            if(src->getPhase() == Player::Discard){
                QString type = "$DiscardCard";
                QString from_general = src->getGeneralName();
                log_box->appendLog(type, from_general, QStringList(), card_str);
            }
        }else if(dest_place == Player::DrawPile){
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
            default:
                ;
            }
        }

        photo->update();
    }
}

void RoomScene::addSkillButton(const Skill *skill){
    QAbstractButton *button = NULL;
    QString skill_name = Sanguosha->translate(skill->objectName());
    if(skill->inherits("TriggerSkill")){
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        switch(trigger_skill->getFrequency()){
        case Skill::Frequent:{
                QCheckBox *checkbox = new QCheckBox(skill_name);

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
                button = new QPushButton(skill_name);
                if(view_as_skill){
                    button2skill.insert(button, view_as_skill);
                    connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
                }

                break;
        }

        case Skill::Compulsory:{
                button = new QPushButton(skill_name + tr(" [Compulsory]"));
                button->setEnabled(false);
                break;
            }

        default:
            break;
        }
    }else if(skill->inherits("FilterSkill")){
        const FilterSkill *filter = qobject_cast<const FilterSkill *>(skill);
        if(filter){
            dashboard->setFilter(filter);
            button = new QPushButton(skill_name + tr(" [Compulsory]"));
            button->setEnabled(false);
        }
    }else if(skill->inherits("ViewAsSkill")){
        button = new QPushButton(skill_name);
        button2skill.insert(button, qobject_cast<const ViewAsSkill *>(skill));
        connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
    }else{
        button = new QPushButton(skill_name);
        if(skill->getFrequency() == Skill::Compulsory){
            button->setText(skill_name + tr(" [Compulsory]"));
            button->setEnabled(false);
        }
    }

    button->setObjectName(skill->objectName());
    if(skill->isLordSkill())
        button->setText(button->text() + tr(" [Lord Skill]"));
    if(skill->getFrequency() == Skill::Limited)
        button->setText(button->text() + tr(" [Limited]"));

    addWidgetToSkillDock(button);

    skill_buttons << button;
    button->setToolTip(skill->getDescription());
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
    const Player *player = qobject_cast<const Player *>(sender());
    const General *general = player->getGeneral();

    skill_buttons.clear();
    button2skill.clear();

    const QList<const Skill*> &skills = general->findChildren<const Skill *>();
    foreach(const Skill* skill, skills){
        if(skill->objectName().startsWith("#"))
            continue;

        if(skill->isLordSkill() && Self->getRole() != "lord")
            continue;

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

void RoomScene::clickSkillButton(int order){
    if(order >= 0 && order < skill_buttons.length())
        skill_buttons.at(order)->click();
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
    const Card *card = dashboard->getSelected();
    if(card)
        useCard(card);
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
    switch(bit){
    case 1: doOkButton(); break;
    case 2: doCancelButton(); break;
    case 3: doDiscardButton(); break;
    }
}

void RoomScene::onJoyDirectionClicked(int direction){
    switch(direction){
    case Joystick::Left: dashboard->selectCard(".", false); break;
    case Joystick::Right: dashboard->selectCard(".", true); break;
    case Joystick::Up:
    case Joystick::Down: selectNextTarget(false); break;
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
            foreach(CardItem *item, amazing_grace){
                if(item->isEnabled()){
                    ClientInstance->chooseAG(item->getCard()->getId());
                    break;
                }
            }

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

            if(ClientInstance->card_pattern.endsWith("!")){
                QRegExp rx("@@?(\\w+)!");
                if(rx.exactMatch(ClientInstance->card_pattern)){
                    QString skill_name = rx.capturedTexts().at(1);
                    const Skill *skill = Sanguosha->getSkill(skill_name);
                    const ViewAsSkill *view_as_skill = qobject_cast<const ViewAsSkill *>(skill);
                    dashboard->startPending(view_as_skill);
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

            foreach(CardItem *item, amazing_grace){
                connect(item, SIGNAL(double_clicked()), this, SLOT(chooseAmazingGrace()));
                connect(item, SIGNAL(grabbed()), this, SLOT(grabCardItem()));
            }

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
        else if(button->inherits("QCheckBox"))
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
    if(status == Client::NotActive){
        if(timer_id != 0){
            killTimer(timer_id);
            timer_id = 0;
        }
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
            dashboard->stopPending();
        }
    }
}

void RoomScene::updateTrustButton(){
    bool trusting = Self->getState() == "trust";
    if(trusting)
        trust_button->setText(tr("Cancel trust"));
    else
        trust_button->setText(tr("Trust"));

    dashboard->setTrust(trusting);
}

void RoomScene::doOkButton(){
    if(!ok_button->isEnabled())
        return;

    switch(ClientInstance->getStatus()){
    case Client::Playing:{
            useSelectedCard();
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
            QList<int> up_cards, down_cards;
            foreach(CardItem *card_item, up_items)
                up_cards << card_item->getCard()->getId();

            foreach(CardItem *card_item, down_items)
                down_cards << card_item->getCard()->getId();

            ClientInstance->replyGuanxing(up_cards, down_cards);
            clearGuanxing();

            break;
        }

    case Client::AskForGongxin:{
            ClientInstance->replyGongxin();
            clearGongxinCards();

            break;
        }
    }

    const ViewAsSkill *skill = dashboard->currentSkill();
    if(skill)
        dashboard->stopPending();
}

void RoomScene::clearGuanxing()
{
    foreach(CardItem *card_item, up_items)
        delete card_item;

    foreach(CardItem *card_item, down_items)
        delete card_item;

    up_items.clear();
    down_items.clear();
}

void RoomScene::clearGongxinCards(){
    foreach(CardItem *card_item, gongxin_items)
        delete card_item;

    gongxin_items.clear();
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
    if(add_robot)
        add_robot->hide();

    foreach(Photo *photo, photos)
        photo->hideAvatar();

    dashboard->hideAvatar();
}

void RoomScene::changeHp(const QString &who, int delta){
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
    if(SoundEngine)
        SoundEngine->stopAllSounds();

    progress_bar->hide();

    main_window->setStatusBar(NULL);
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

void RoomScene::fillTable(QTableWidget *table, const QList<const ClientPlayer *> &players){
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
        dashboard->update();

        general = Self->getGeneral();
    }else{
        Photo *photo = name2photo[who];
        photo->setFrame(Photo::NoFrame);
        photo->setEnabled(false);
        photo->update();

        general = photo->getPlayer()->getGeneral();
    }

    QMutableMapIterator<QGraphicsItem *, const ClientPlayer *> itor(item2player);
    while(itor.hasNext()){
        itor.next();
        if(itor.value()->objectName() == who){
            itor.key()->setEnabled(false);
            itor.remove();
            break;
        }
    }

    if(Config.EnableLastWord)
        general->lastWord();
}

void RoomScene::fillAmazingGrace(const QList<int> &card_ids){
    static const int columns = 5;

    int i;
    for(i=0; i<card_ids.length(); i++){
        int card_id = card_ids.at(i);
        CardItem *card_item = new CardItem(Sanguosha->getCard(card_id));

        QRectF rect = card_item->boundingRect();
        int row = i / columns;
        int column = i % columns;
        QPointF pos(column * rect.width(), row * rect.height());

        card_item->setPos(pos);
        card_item->setHomePos(pos);
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        card_item->setZValue(1.0);

        amazing_grace << card_item;
        addItem(card_item);
    }

    int row_count = (amazing_grace.length() - 1) / columns + 1;
    int column_count = amazing_grace.length() > columns ? columns : amazing_grace.length();
    QRectF rect = amazing_grace.first()->boundingRect();

    int width = rect.width() * column_count;
    int height = rect.height() * row_count;
    qreal dx = - width/2;
    qreal dy = - height/2;

    foreach(CardItem *card_item, amazing_grace){
        card_item->moveBy(dx, dy);
        card_item->setHomePos(card_item->pos());
    }
}

void RoomScene::clearAmazingGrace(){
    foreach(CardItem *item, amazing_grace){
        removeItem(item);
        delete item;
    }

    foreach(QGraphicsPixmapItem *taker_avatar, taker_avatars){
        removeItem(taker_avatar);
        delete taker_avatar;
    }

    amazing_grace.clear();
    taker_avatars.clear();
}

void RoomScene::takeAmazingGrace(const ClientPlayer *taker, int card_id){
    if(taker){
        QString type = "$TakeAG";
        QString from_general = taker->getGeneralName();
        QString card_str = QString::number(card_id);
        log_box->appendLog(type, from_general, QStringList(), card_str);
    }


    CardItem *to_take = CardItem::FindItem(amazing_grace, card_id);
    if(to_take){
        to_take->setEnabled(false);

        // make a copy on the amazing grace item and put it to dashboard later
        CardItem *item = new CardItem(to_take->getCard());
        addItem(item);
        item->setPos(to_take->pos());
        item->setEnabled(false);

        QPixmap avatar_pixmap;
        if(taker){
            putCardItem(taker, Player::Hand, item);
            avatar_pixmap.load(taker->getGeneral()->getPixmapPath("tiny"));
        }else{
            putCardItem(NULL, Player::DiscardedPile, item);
            avatar_pixmap.load("image/system/card-back.png");
            avatar_pixmap = avatar_pixmap.scaled(avatar_pixmap.size() / 2);
        }

        QGraphicsPixmapItem *taker_avatar = addPixmap(avatar_pixmap);
        taker_avatar->setPos(to_take->homePos() + TinyAvatarOffset);
        taker_avatar->setZValue(1.1);

        taker_avatars << taker_avatar;
    }
}

void RoomScene::chooseAmazingGrace(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item){
        ClientInstance->chooseAG(card_item->getCard()->getId());
        foreach(CardItem *item, amazing_grace)
            item->disconnect(this);
    }
}

void RoomScene::grabCardItem(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item && card_item->collidesWithItem(dashboard)){
        ClientInstance->chooseAG(card_item->getCard()->getId());
        foreach(CardItem *item, amazing_grace)
            item->disconnect(this);
    }
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

const ViewAsSkill *RoomScene::getViewAsSkill(const QString &skill_name){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(!skill){
        QMessageBox::warning(main_window, tr("Warning"), tr("No such skill named %1").arg(skill_name));
        return NULL;
    }

    const ViewAsSkill *view_as_skill = qobject_cast<const ViewAsSkill *>(skill);
    if(!view_as_skill){
        QMessageBox::warning(main_window, tr("Warning"), tr("The skill %1 must be view as skill!").arg(skill_name));
        return NULL;
    }

    return view_as_skill;
}

void RoomScene::attachSkill(const QString &skill_name){
    const ViewAsSkill *skill = getViewAsSkill(skill_name);

    QPushButton *button = new QPushButton(Sanguosha->translate(skill_name));

    skill_buttons << button;
    button2skill.insert(button, skill);

    button->setEnabled(skill->isAvailable());
    addWidgetToSkillDock(button, true);

    connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
}

void RoomScene::detachSkill(const QString &skill_name){
    const ViewAsSkill *skill = getViewAsSkill(skill_name);

    QAbstractButton *button = button2skill.key(skill, NULL);
    if(button){
        skill_buttons.removeOne(button);
        button2skill.remove(button);
        QPushButton *push_button = qobject_cast<QPushButton *>(button);
        removeWidgetFromSkillDock(push_button);

        delete button;
    }
}

void RoomScene::doGuanxing(const QList<int> &card_ids){
    if(card_ids.isEmpty()){
        clearGuanxing();
        return;
    }

    up_items.clear();
    foreach(int card_id, card_ids){
        GuanxingCardItem *card_item = new GuanxingCardItem(Sanguosha->getCard(card_id));
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        connect(card_item, SIGNAL(released()), this, SLOT(adjustGuanxing()));

        up_items << card_item;
        addItem(card_item);
    }

    QRectF rect = up_items.first()->boundingRect();
    static qreal CardWidth = rect.width();
    static qreal CardHeight = rect.height();

    qreal width = CardWidth * up_items.length();
    qreal height = CardHeight * 2;

    qreal start_x = - width/2;
    qreal start_y = - height + 120;

    int i;
    for(i=0; i<up_items.length(); i++){
        CardItem *card_item = up_items.at(i);
        QPointF pos(start_x + i*CardWidth, start_y);
        card_item->setPos(pos);
        card_item->setHomePos(pos);
    }

    guanxing_origin.setX(start_x);
    guanxing_origin.setY(start_y);
}

void RoomScene::adjustGuanxing(){
    GuanxingCardItem *item = qobject_cast<GuanxingCardItem*>(sender());

    if(item == NULL)
        return;

    up_items.removeOne(item);
    down_items.removeOne(item);

    QRectF rect(item->boundingRect());
    static qreal card_width = rect.width();
    static qreal card_height = rect.height();

    int r = (item->y() - guanxing_origin.y()) / card_height;
    r = qBound(0, r, 1);
    QList<GuanxingCardItem *> &items = r == 0 ? up_items : down_items;
    int c = (item->x() - guanxing_origin.x()) / card_width;

    c = qBound(0, c, items.length());
    items.insert(c, item);

    int i;
    for(i=0; i<up_items.length(); i++){
        QPointF pos = guanxing_origin;
        pos += QPointF(i * card_width, 0);
        up_items.at(i)->setHomePos(pos);
        up_items.at(i)->goBack();
    }

    for(i=0; i<down_items.length(); i++){
        QPointF pos = guanxing_origin;
        pos += QPointF(i * card_width, card_height);
        down_items.at(i)->setHomePos(pos);
        down_items.at(i)->goBack();
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
    gongxin_items.clear();

    foreach(int card_id, card_ids){
        const Card *card = Sanguosha->getCard(card_id);
        CardItem *card_item = new CardItem(card);
        if(enable_heart && card->getSuit() == Card::Heart){
            card_item->setEnabled(true);
            card_item->setFlag(QGraphicsItem::ItemIsFocusable);
            connect(card_item, SIGNAL(double_clicked()), this, SLOT(chooseGongxinCard()));
        }else
            card_item->setEnabled(false);

        addItem(card_item);
        gongxin_items << card_item;
    }

    QRectF rect = gongxin_items.first()->boundingRect();
    qreal card_width = rect.width();
    qreal card_height = rect.height();

    qreal width = card_width * gongxin_items.length();
    qreal start_x = - width / 2;
    qreal start_y = - card_height / 2;

    int i;
    for(i=0; i<gongxin_items.length(); i++){
        QPoint pos;
        pos.setX(start_x + i * card_width);
        pos.setY(start_y);

        gongxin_items.at(i)->setPos(pos);
        gongxin_items.at(i)->setHomePos(pos);
    }

    if(!enable_heart)
        QTimer::singleShot(4000, this, SLOT(clearGongxinCards()));
}

void RoomScene::chooseGongxinCard(){
    CardItem *item = qobject_cast<CardItem *>(sender());

    if(item){
        ClientInstance->replyGongxin(item->getCard()->getId());

        foreach(CardItem *card_item, gongxin_items)
            delete card_item;

        gongxin_items.clear();
    }
}

void RoomScene::createStateItem(){
    QGraphicsItem *state_item = addPixmap(QPixmap("image/system/state.png"));
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

    if(ServerInfo.EnableAI){
        add_robot = new Button(tr("Add robot"));
        add_robot->setPos(state_item->x() + 10, state_item->y() + state_item->boundingRect().height() + 10);
        addItem(add_robot);
        add_robot->hide();

        connect(add_robot, SIGNAL(clicked()), ClientInstance, SLOT(addRobot()));
        connect(Self, SIGNAL(owner_changed(bool)), this, SLOT(showOwnerButtons(bool)));
    }else
        add_robot = NULL;
}

void RoomScene::showOwnerButtons(bool owner){
    if(add_robot)
        add_robot->setVisible(owner);
}

void RoomScene::showJudgeResult(const QString &who, const QString &result){
    if(special_card){
        const ClientPlayer *player = ClientInstance->getPlayer(who);
        QString path = player->getGeneral()->getPixmapPath("tiny");
        judge_avatar->setPixmap(QPixmap(path));
        judge_avatar->show();
        special_card->setFrame(result);
    }
}

void RoomScene::onGameStart(){
    // add free discard button
    if(ServerInfo.FreeChoose){
        QPushButton *free_discard = new QPushButton(tr("Free discard"));
        free_discard->setToolTip(tr("Discard cards freely"));

        addWidgetToSkillDock(free_discard, true);

        FreeDiscardSkill *discard_skill = new FreeDiscardSkill(this);
        button2skill.insert(free_discard, discard_skill);
        skill_buttons << free_discard;
        connect(free_discard, SIGNAL(clicked()), this, SLOT(doSkillButton()));
    }

    trust_button->setEnabled(true);
    updateStatus(ClientInstance->getStatus());

    QList<const ClientPlayer *> players = ClientInstance->getPlayers();
    foreach(const ClientPlayer *player, players){
        connect(player, SIGNAL(turn_started()), log_box, SLOT(appendSeparator()));
    }

    foreach(Photo *photo, photos)
        photo->createRoleCombobox();

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
    QString bgmusic_path = Config.value("BackgroundMusic", "audio/system/background.mp3").toString();
    const char *filename = bgmusic_path.toLocal8Bit().data();
    irrklang::ISoundSource *bgmusic = SoundEngine->addSoundSourceFromFile(filename);

    if(bgmusic){
        bgmusic->setDefaultVolume(Config.Volume);
        SoundEngine->play2D(bgmusic, true);
    }
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

void RoomScene::moveAndDisappear(QGraphicsObject *item, const QPointF &from, const QPointF &to) const{
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup;

    QPropertyAnimation *move = new QPropertyAnimation(item, "pos");
    move->setStartValue(from);
    move->setEndValue(to);
    move->setDuration(1000);

    QPropertyAnimation *disappear = new QPropertyAnimation(item, "opacity");
    disappear->setEndValue(0.0);

    group->addAnimation(move);
    group->addAnimation(disappear);

    group->start(QAbstractAnimation::DeleteWhenStopped);
    connect(group, SIGNAL(finished()), item, SLOT(deleteLater()));
}

void RoomScene::doAnimation(const QString &name, const QStringList &args){
    if(name == "peach" || name == "nullification"){
        Pixmap *item = new Pixmap(QString("image/system/animation/%1.png").arg(name));
        addItem(item);

        QPointF from = getAnimationObject(args.at(0))->scenePos();
        QPointF to = getAnimationObject(args.at(1))->scenePos();

        moveAndDisappear(item, from, to);
    }else if(name == "lightbox"){
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
    }else if(name == "fire" || name == "thunder"){
        Pixmap *item = new Pixmap(QString("image/system/animation/%1.png").arg(name));
        addItem(item);

        QPointF from = getAnimationObject(args.at(0))->scenePos();
        moveAndDisappear(item, from, from);
    }
}

void RoomScene::adjustDashboard(){
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if(button){
        bool expand = button->text() == tr("Expand to window width");
        if(expand){
            dashboard->setWidth(main_window->width()-10);
            button->setText(tr("Reset to default width"));
        }else{
            dashboard->setWidth(0);
            button->setText(tr("Expand to window width"));
        }

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
        items << QString("%1[%2]").arg(player->screenName()).arg(general_name);
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

