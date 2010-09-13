#include "roomscene.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "optionbutton.h"
#include "cardoverview.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>
#include <QStatusBar>
#include <QListWidget>
#include <QHBoxLayout>
#include <QSignalMapper>
#include <QKeyEvent>
#include <QCheckBox>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QMenu>
#include <QGroupBox>


static const QPointF DiscardedPos(-494, -115);
static const QPointF DrawPilePos(893, -235);
static QSize GeneralSize(200 * 0.8, 290 * 0.8);

RoomScene::RoomScene(int player_count, QMainWindow *main_window)
    :main_window(main_window)
{
    connect(this, SIGNAL(selectionChanged()), this, SLOT(updateSelectedTargets()));

    ClientInstance->setParent(this);
    setBackgroundBrush(Config.BackgroundBrush);

    // create pile
    pile = new Pixmap(":/pile.png");
    addItem(pile);
    pile->setPos(387, -132);

    pile_number_item = addText("", Config.SmallFont);
    pile_number_item->setPos(pile->pos());

    // create photos
    int i;
    for(i=0;i<player_count-1;i++){
        Photo *photo = new Photo(i);
        photos << photo;
        addItem(photo);
    }   

    // create dashboard
    dashboard = new Dashboard;
    addItem(dashboard);
    dashboard->setPlayer(Self);
    connect(Self, SIGNAL(general_changed()), dashboard, SLOT(updateAvatar()));
    connect(Self, SIGNAL(general_changed()), this, SLOT(updateSkillButtons()));
    connect(dashboard, SIGNAL(card_selected(const Card*)), this, SLOT(enableTargets(const Card*)));
    connect(dashboard, SIGNAL(card_to_use()), this, SLOT(doOkButton()));

    // add role combobox
    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Your role"));
    role_combobox->addItem(tr("Unknown"));
    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(updateRoleComboBox(QString)));

    QGraphicsLinearLayout *button_layout = new QGraphicsLinearLayout(Qt::Horizontal);

    // add buttons
    ok_button = new QPushButton(tr("OK"));
    cancel_button = new QPushButton(tr("Cancel"));
    discard_button = new QPushButton(tr("Discard cards"));

    ok_button->setEnabled(false);
    cancel_button->setEnabled(false);
    discard_button->setEnabled(false);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(doOkButton()));
    connect(cancel_button, SIGNAL(clicked()), this, SLOT(doCancelButton()));
    connect(discard_button, SIGNAL(clicked()), this, SLOT(doDiscardButton()));

    button_layout->addItem(addWidget(ok_button));
    button_layout->addItem(addWidget(cancel_button));
    button_layout->addItem(addWidget(discard_button));

    QGraphicsWidget *form = new QGraphicsWidget(dashboard);
    form->setLayout(button_layout);
    form->setPos(dashboard->boundingRect().width() - button_layout->preferredWidth(), -25);

    discard_skill = new DiscardSkill;
    discard_skill->setNum(2);

    known_cards_menu = new QMenu(main_window);

    // get dashboard's avatar
    avatar = dashboard->getAvatar();    

    // do signal-slot connections
    connect(ClientInstance, SIGNAL(player_added(ClientPlayer*)), SLOT(addPlayer(ClientPlayer*)));
    connect(ClientInstance, SIGNAL(player_removed(QString)), SLOT(removePlayer(QString)));
    connect(ClientInstance, SIGNAL(cards_drawed(QList<const Card*>)), this, SLOT(drawCards(QList<const Card*>)));    
    connect(ClientInstance, SIGNAL(generals_got(QList<const General*>)), this, SLOT(chooseGeneral(QList<const General*>)));
    connect(ClientInstance, SIGNAL(seats_arranged(QList<const ClientPlayer*>)), SLOT(updatePhotos(QList<const ClientPlayer*>)));
    connect(ClientInstance, SIGNAL(n_card_drawed(ClientPlayer*,int)), SLOT(drawNCards(ClientPlayer*,int)));    
    connect(ClientInstance, SIGNAL(card_moved(CardMoveStructForClient)), this, SLOT(moveCard(CardMoveStructForClient)));
    connect(ClientInstance, SIGNAL(status_changed(Client::Status)), this, SLOT(updateStatus(Client::Status)));
    connect(ClientInstance, SIGNAL(avatars_hiden()), this, SLOT(hideAvatars()));
    connect(ClientInstance, SIGNAL(hp_changed(QString,int)), this, SLOT(changeHp(QString,int)));
    connect(ClientInstance, SIGNAL(message_changed(QString)), this, SLOT(changeMessage(QString)));
    connect(ClientInstance, SIGNAL(pile_cleared()), this, SLOT(clearPile()));
    connect(ClientInstance, SIGNAL(pile_num_set(int)), this, SLOT(setPileNumber(int)));
    connect(ClientInstance, SIGNAL(player_killed(QString)), this, SLOT(killPlayer(QString)));
    connect(ClientInstance, SIGNAL(game_over(bool,QList<bool>)), this, SLOT(gameOver(bool,QList<bool>)));
    connect(ClientInstance, SIGNAL(card_shown(QString,int)), this, SLOT(showCard(QString,int)));

    connect(ClientInstance, SIGNAL(ag_filled(QList<int>)), this, SLOT(fillAmazingGrace(QList<int>)));
    connect(ClientInstance, SIGNAL(ag_taken(const ClientPlayer*,int)), this, SLOT(takeAmazingGrace(const ClientPlayer*,int)));
    connect(ClientInstance, SIGNAL(ag_cleared()), this, SLOT(clearAmazingGrace()));

    connect(ClientInstance, SIGNAL(skill_attached(QString)), this, SLOT(attachSkill(QString)));
    connect(ClientInstance, SIGNAL(skill_detached(QString)), this, SLOT(detachSkill(QString)));

    daqiao = new Daqiao;
    daqiao->shift();
    daqiao->hide();
    addItem(daqiao);

    connect(ClientInstance, SIGNAL(prompt_changed(QString)), daqiao, SLOT(setContent(QString)));

    ClientInstance->signup();

    startEnterAnimation();    
}

void RoomScene::startEnterAnimation(){
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    const qreal photo_width = photos.first()->boundingRect().width();
    const qreal start_x = (Config.Rect.width() - photo_width*photos.length())/2 + Config.Rect.x();

    int i;
    for(i=0;i<photos.length();i++){
        Photo *photo = photos[i];
        qreal x = i * photo_width + start_x;
        qreal y =  Config.Rect.y();
        int duration = 1500.0 * qrand()/ RAND_MAX;

        QPropertyAnimation *translation = new QPropertyAnimation(photo, "pos");
        translation->setEndValue(QPointF(x,y));
        translation->setEasingCurve(QEasingCurve::OutBounce);
        translation->setDuration(duration);

        group->addAnimation(translation);
    }

    QPointF start_pos(Config.Rect.topLeft());
    QPointF end_pos(Config.Rect.x(), Config.Rect.bottom() - dashboard->boundingRect().height());
    int duration = 1500;

    QPropertyAnimation *translation = new QPropertyAnimation(dashboard, "pos");
    translation->setStartValue(start_pos);
    translation->setEndValue(end_pos);
    translation->setEasingCurve(QEasingCurve::OutBounce);
    translation->setDuration(duration);

    QPropertyAnimation *enlarge = new QPropertyAnimation(dashboard, "scale");
    enlarge->setStartValue(0.2);
    enlarge->setEndValue(1.0);
    enlarge->setEasingCurve(QEasingCurve::OutBounce);
    enlarge->setDuration(duration);

    group->addAnimation(translation);
    group->addAnimation(enlarge);

    group->start(QAbstractAnimation::DeleteWhenStopped);
}

void RoomScene::addPlayer(ClientPlayer *player){
    int i;
    for(i=0; i<photos.length(); i++){
        Photo *photo = photos[i];
        if(photo->getPlayer() == NULL){
            photo->setPlayer(player);

            name2photo[player->objectName()] = photo;

            static Phonon::MediaSource add_player_source("audio/add-player.wav");
            Sanguosha->playEffect(add_player_source);

            return;
        }
    }
}

void RoomScene::removePlayer(const QString &player_name){
    Photo *photo = name2photo[player_name];
    if(photo){
        photo->setPlayer(NULL);
        name2photo.remove(player_name);
    }
}

void RoomScene::updatePhotos(const QList<const ClientPlayer*> &seats){
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

    const qreal photo_width = photos.first()->boundingRect().width();
    const qreal start_x = (Config.Rect.width() - photo_width*photos.length())/2 + Config.Rect.x();

    for(i=0;i<photos.length();i++){
        Photo *photo = photos[i];
        qreal x = i * photo_width + start_x;

        QPropertyAnimation *translation = new QPropertyAnimation(photo, "x");
        translation->setEndValue(x);
        translation->setEasingCurve(QEasingCurve::OutBounce);

        group->addAnimation(translation);
    }
}

void RoomScene::drawCards(const QList<const Card *> &cards){
    foreach(const Card * card, cards){
        CardItem *item = new CardItem(card);
        item->setPos(DrawPilePos);
        dashboard->addCardItem(item);
    }

    setPileNumber(pile_number - cards.length());
}

void RoomScene::drawNCards(ClientPlayer *player, int n){
    QSequentialAnimationGroup *group =  new QSequentialAnimationGroup;
    QParallelAnimationGroup *moving = new QParallelAnimationGroup;
    QParallelAnimationGroup *disappering = new QParallelAnimationGroup;

    Photo *photo = name2photo[player->objectName()];
    int i;
    for(i=0; i<n; i++){
        Pixmap *pixmap = new Pixmap(":/card-back.png");
        addItem(pixmap);

        QPropertyAnimation *ugoku = new QPropertyAnimation(pixmap, "pos");
        ugoku->setStartValue(QPointF(387, -162));
        ugoku->setDuration(500);
        ugoku->setEasingCurve(QEasingCurve::OutBounce);
        ugoku->setEndValue(photo->pos() + QPointF(10 *i, 0));

        QPropertyAnimation *kieru = new QPropertyAnimation(pixmap, "opacity");
        kieru->setDuration(900);
        kieru->setEndValue(0.0);

        moving->addAnimation(ugoku);
        disappering->addAnimation(kieru);

        connect(kieru, SIGNAL(finished()), pixmap, SLOT(deleteLater()));
    }

    group->addAnimation(moving);
    group->addAnimation(disappering);

    group->start(QAbstractAnimation::DeleteWhenStopped);

    photo->update();

    setPileNumber(pile_number - n);
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

    bool control_is_down = event->modifiers() & Qt::ControlModifier;

    switch(event->key()){        
    case Qt::Key_F1:
    case Qt::Key_F2:
    case Qt::Key_F3:
    case Qt::Key_F4: dashboard->sort(event->key() - Qt::Key_F1); break;

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

    case Qt::Key_Return : useSelectedCard(); break;

    case Qt::Key_Escape : {
            if(!discarded_queue.isEmpty() && discarded_queue.first()->rotation() == 0.0)
                hideDiscards();
            else{
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

#ifndef QT_NO_DEBUG
    case Qt::Key_D: {
            // do some debugging things
            ClientInstance->drawCards("1+2+3+4+5+6");
        }
#endif
    }
}

void RoomScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
    QGraphicsItem *item = itemAt(event->scenePos());
    if(!item)
        return;

    QGraphicsObject *item_obj = static_cast<QGraphicsObject *>(item);
    Photo *photo = qobject_cast<Photo *>(item_obj);

    if(!photo)
        return;

    const ClientPlayer *player = photo->getPlayer();
    if(player){
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

void RoomScene::chooseGeneral(const QList<const General *> &generals){
    if(photos.length()>1)
        changeMessage(tr("Please wait for other players choosing their generals"));

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Choose general"));

    QSignalMapper *mapper = new QSignalMapper(dialog);
    QList<OptionButton *> buttons;
    foreach(const General *general, generals){
        QString icon_path = general->getPixmapPath("card");
        QString caption = Sanguosha->translate(general->objectName());
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setIconSize(GeneralSize);
        buttons << button;

        mapper->setMapping(button, general->objectName());
        connect(button, SIGNAL(double_clicked()), mapper, SLOT(map()));
        connect(button, SIGNAL(double_clicked()), dialog, SLOT(accept()));
    }

    QLayout *layout = NULL;
    const static int columns = 5;
    if(generals.length() <= columns){
        layout = new QHBoxLayout;
        foreach(OptionButton *button, buttons)
            layout->addWidget(button);
    }else{
        QGridLayout *grid_layout = new QGridLayout;
        layout = grid_layout;

        int i;
        for(i=0; i<buttons.length(); i++){
            int row = i / columns;
            int column = i % columns;
            grid_layout->addWidget(buttons.at(i), row, column);
        }
    }

    mapper->setMapping(dialog, generals.first()->objectName());
    connect(dialog, SIGNAL(rejected()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QString)), ClientInstance, SLOT(itemChosen(QString)));

    dialog->setLayout(layout);
    dialog->exec();
}

void RoomScene::changeMessage(const QString &message){
    if(message.isNull())
        main_window->statusBar()->clearMessage();
    else
        main_window->statusBar()->showMessage(message);
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
            card_item->setRotation(0);
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
        card_item->setRotation(qrand() % 359 + 1);
        card_item->setHomePos(DiscardedPos);
        card_item->goBack();
    }
}

CardItem *RoomScene::takeCardItem(ClientPlayer *src, Player::Place src_place, int card_id){
    if(src){
        // from players

        if(src == Self){
            CardItem *card_item = dashboard->takeCardItem(card_id, src_place);
            card_item->setOpacity(1.0);
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
        setPileNumber(pile_number - 1);
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

void RoomScene::moveCard(const CardMoveStructForClient &move){
    ClientPlayer *src = move.from;
    ClientPlayer *dest = move.to;
    Player::Place src_place = move.from_place;
    Player::Place dest_place = move.to_place;
    int card_id = move.card_id;    

    CardItem *card_item = takeCardItem(src, src_place, card_id);
    if(card_item->scene() == NULL)
        addItem(card_item);

    putCardItem(dest, dest_place, card_item);
}

void RoomScene::putCardItem(const ClientPlayer *dest, Player::Place dest_place, CardItem *card_item){
    static Phonon::MediaSource install_equip_source("audio/install-equip.wav");

    if(dest == NULL){
        card_item->setHomePos(DiscardedPos);
        card_item->setRotation(qrand() % 359 + 1);
        card_item->goBack();
        card_item->setEnabled(true);

        card_item->setFlag(QGraphicsItem::ItemIsFocusable, false);

        card_item->setZValue(0.1*ClientInstance->discarded_list.length());
        discarded_queue.enqueue(card_item);

        if(discarded_queue.length() > 8){
            CardItem *first = discarded_queue.dequeue();
            delete first;
        }

        connect(card_item, SIGNAL(show_discards()), this, SLOT(viewDiscards()));
        connect(card_item, SIGNAL(hide_discards()), this, SLOT(hideDiscards()));
    }else if(dest->objectName() == Config.UserName){
        switch(dest_place){
        case Player::Equip:{
                dashboard->installEquip(card_item);
                Sanguosha->playEffect(install_equip_source);
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
                Sanguosha->playEffect(install_equip_source);
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

void RoomScene::updateSkillButtons(){
    const Player *player = qobject_cast<const Player *>(sender());
    const General *general = player->getGeneral();
    main_window->setStatusBar(NULL);
    skill_buttons.clear();
    button2skill.clear();
    QStatusBar *status_bar = main_window->statusBar();

    const QList<const Skill*> &skills = general->findChildren<const Skill *>();
    foreach(const Skill* skill, skills){
        QAbstractButton *button = NULL;
        QString skill_name = Sanguosha->translate(skill->objectName());
        if(skill->inherits("TriggerSkill")){
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            switch(trigger_skill->getFrequency()){
            case TriggerSkill::Frequent:{
                    QCheckBox *checkbox = new QCheckBox(skill_name);

                    checkbox->setObjectName(skill->objectName());
                    checkbox->setChecked(false);
                    connect(checkbox, SIGNAL(stateChanged(int)), ClientInstance, SLOT(updateFrequentFlags(int)));
                    checkbox->setChecked(true);

                    button = checkbox;
                    break;
            }
            case TriggerSkill::NotFrequent:{
                    const ViewAsSkill *view_as_skill = trigger_skill->getViewAsSkill();
                    button = new QPushButton(skill_name);
                    if(view_as_skill){
                        button2skill.insert(button, view_as_skill);
                        connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
                    }

                    break;
            }

            case TriggerSkill::Compulsory:{
                    button = new QPushButton(skill_name + tr(" [Compulsory]"));
                    button->setEnabled(false);
                    break;
                }
            }
        }else if(skill->inherits("ViewAsSkill")){
            button = new QPushButton(skill_name);
            button2skill.insert(button, qobject_cast<const ViewAsSkill *>(skill));
            connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
        }else
            button = new QPushButton(skill_name);
            // QMessageBox::warning(NULL, tr("Warning"), tr("Only view as skill and passive skill is supported now!"));

        button->setObjectName(skill->objectName());
        if(skill->isLordSkill())
            button->setText(button->text() + tr(" [Lord Skill]"));

        button->setToolTip(skill->getDescription());

        status_bar->addPermanentWidget(button);
        skill_buttons << button;
    }

    status_bar->addPermanentWidget(role_combobox);
}

void RoomScene::updateRoleComboBox(const QString &new_role){
    role_combobox->setItemText(1, Sanguosha->translate(new_role));
    role_combobox->setItemIcon(1, QIcon(QString(":/roles/%1.png").arg(new_role)));

    if(new_role != "lord"){
        foreach(QAbstractButton *button, skill_buttons){
            const Skill *skill = Sanguosha->getSkill(button->objectName());

            Q_ASSERT(skill != NULL);
            if(skill->isLordSkill())
                button->hide();
        }
    }
}

void RoomScene::clickSkillButton(int order){
    if(order >= 0 && order < skill_buttons.length())
        skill_buttons.at(order)->click();
}

void RoomScene::enableTargets(const Card *card){
    selected_targets.clear();

    // unset avatar and all photo
    avatar->setSelected(false);
    foreach(Photo *photo, photos)
        photo->setSelected(false);

    if(card == NULL){
        avatar->setEnabled(false);
        avatar->setFlag(QGraphicsItem::ItemIsSelectable, false);
        foreach(Photo *photo, photos){
            photo->setEnabled(false);
            photo->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }
        changeMessage();
        ok_button->setEnabled(false);
        return;
    }

    changeMessage(tr("You choosed card [%1]").arg(card->getName()));

    if(card->targetFixed() || ClientInstance->noTargetResponsing()){
        avatar->setEnabled(true);
        avatar->setFlag(QGraphicsItem::ItemIsSelectable, false);
        foreach(Photo *photo, photos){
            photo->setEnabled(true);
            photo->setFlag(QGraphicsItem::ItemIsSelectable, false);
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
    if(card->targetFilter(selected_targets, Self)){
        avatar->setEnabled(true);
        avatar->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }else{
        avatar->setEnabled(false);
        avatar->setFlag(QGraphicsItem::ItemIsSelectable, false);
    }

    foreach(Photo *photo, photos){
        if(card->targetFilter(selected_targets, photo->getPlayer())){
            photo->setEnabled(true);
            photo->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }else{
            photo->setEnabled(false);
            photo->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }
    }
}

void RoomScene::updateSelectedTargets(){
    const Card *card = dashboard->getSelected();
    if(!card){
        selected_targets.clear();
        changeMessage();
        return;
    }

    QSet<const ClientPlayer *> old_set = selected_targets.toSet();
    QSet<const ClientPlayer *> new_set;

    if(avatar->isSelected())
        new_set << Self;
    foreach(Photo *photo, photos){
        if(photo->isSelected())
            new_set << photo->getPlayer();
    }

    if(new_set.isEmpty()){
        selected_targets.clear();
        changeMessage();
        return;
    }

    if(new_set.count() == 1){
        selected_targets.clear();
        selected_targets << *new_set.begin();
    }else{
        int change = qAbs(old_set.count() - new_set.count());
        if(change > 1){
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("New target set and old target set is changed out of range!"));
            return;
        }

        if(old_set.count() > new_set.count()){
            const ClientPlayer *unselected = *(old_set - new_set).begin();
            selected_targets.removeOne(unselected);
        }else if(old_set.count() < new_set.count()){
            const ClientPlayer *new_added = *(new_set - old_set).begin();
            selected_targets.append(new_added);
        }
    }

    QStringList target_names;
    foreach(const ClientPlayer *target, selected_targets)
        target_names << Sanguosha->translate(target->getGeneralName());
    changeMessage(tr("You choose %1 as [%2]'s target").arg(target_names.join(",")).arg(card->getName()));

    // updateTargetsEnablity(card);
    ok_button->setEnabled(card->targetsFeasible(selected_targets));
}

void RoomScene::useSelectedCard(){
    const Card *card = dashboard->getSelected();
    if(card)
        useCard(card);
    else
        changeMessage(tr("You didn't choose any card to use yet!"));
}

void RoomScene::useCard(const Card *card){
    if(card->targetFixed() || card->targetsFeasible(selected_targets))
        ClientInstance->useCard(card, selected_targets);
    else
        changeMessage(tr("Not enough targets"));

    enableTargets(NULL);
}

void RoomScene::callViewAsSkill(){
    const Card *card = dashboard->pendingCard();

    if(card == NULL){
        changeMessage(tr("Not enough cards to call skill"));
        return;
    }

    if(card->isAvailable()){
        // use card
        dashboard->stopPending();
        useCard(card);
    }else{
        changeMessage(tr("Card [%1] can not be used right now").arg(card->getName()));
    }
}

void RoomScene::cancelViewAsSkill(){
    const ViewAsSkill *skill = dashboard->currentSkill();
    dashboard->stopPending();
    QAbstractButton *button = button2skill.key(skill, NULL);

    if(button){
        button->setEnabled(true);

        ok_button->setEnabled(false);
        cancel_button->setEnabled(false);

        dashboard->enableCards();
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

void RoomScene::updateStatus(Client::Status status){
    switch(status){
    case Client::NotActive:{
            dashboard->disableAllCards();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }

    case Client::Responsing: {
            dashboard->enableCards(ClientInstance->card_pattern);

            ok_button->setEnabled(false);
            cancel_button->setEnabled(true);
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
            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            discard_skill->setNum(ClientInstance->discard_num);
            dashboard->startPending(discard_skill);
            break;
        }

    case Client::AskForAG:{
            dashboard->disableAllCards();

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            foreach(CardItem *item, amazing_grace)
                connect(item, SIGNAL(double_clicked()), this, SLOT(chooseAmazingGrace()));

            break;
        }
    }

    foreach(QAbstractButton *button, skill_buttons){
        const ViewAsSkill *skill = button2skill.value(button, NULL);
        if(skill)
            button->setEnabled(skill->isAvailable());
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
            doOkButton();
        }
    }
}

void RoomScene::doOkButton(){
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
                    ClientInstance->responseCard(card, selected_targets);
                daqiao->hide();
            }

            dashboard->unselectAll();
            break;
        }

    case Client::Discarding: {
            const Card *card = dashboard->pendingCard();
            if(card){
                ClientInstance->discardCards(card);
                dashboard->stopPending();
                daqiao->hide();
            }
            break;
        }

    case Client::NotActive: {
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is not active!"));
            return;
        }
    case Client::AskForAG:{
            QMessageBox::warning(main_window, tr("Warning"),
                                 tr("The OK button should be disabled when client is in asking for amazing grace status"));
            return;
        }
    }

    const ViewAsSkill *skill = dashboard->currentSkill();
    if(skill)
        dashboard->stopPending();
}

void RoomScene::doCancelButton(){
    // FIXME

    switch(ClientInstance->getStatus()){
    case Client::Playing:{
            const ViewAsSkill *skill = dashboard->currentSkill();
            if(skill)
                cancelViewAsSkill();
            else
                dashboard->unselectAll();
            break;
        }

    case Client::Responsing:{
            if(ClientInstance->noTargetResponsing())
                ClientInstance->responseCard(NULL);
            else
                ClientInstance->responseCard(NULL, QList<const ClientPlayer *>());
            daqiao->hide();
            break;
        }

    default:
        ;
    }
}

void RoomScene::doDiscardButton(){
    dashboard->unselectAll();

    if(ClientInstance->getStatus() == Client::Playing){
        ClientInstance->useCard(NULL);
    }
}

void RoomScene::hideAvatars(){
    foreach(Photo *photo, photos)
        photo->hideAvatar();

    dashboard->hideAvatar();
}

void RoomScene::changeHp(const QString &who, int delta){
    if(who == Config.UserName){
        dashboard->update();
    }else{
        Photo *photo = name2photo.value(who, NULL);
        if(photo)
            photo->update();
    }

    if(delta < 0){
        static Phonon::MediaSource male_damage_effect("audio/male-damage.mp3");
        static Phonon::MediaSource female_damage_effect("audio/female-damage.mp3");

        ClientPlayer *player = ClientInstance->findChild<ClientPlayer *>(who);

        if(player->getGeneral()->isMale())
            Sanguosha->playEffect(male_damage_effect);
        else
            Sanguosha->playEffect(female_damage_effect);
    }
}

void RoomScene::clearPile(){
    foreach(CardItem *item, discarded_queue){
        removeItem(item);
        delete item;
    }

    discarded_queue.clear();
}

void RoomScene::setPileNumber(int n){
    pile_number = n;
    pile_number_item->setPlainText(QString::number(pile_number));
}

void RoomScene::gameOver(bool victory, const QList<bool> &result_list){
    dashboard->setEnabled(false);

    QDialog *dialog = new QDialog(main_window);
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

    QList<ClientPlayer *> players = ClientInstance->getPlayers();
    QList<ClientPlayer *> winner_list, loser_list;
    int i;
    for(i=0; i<players.length(); i++){
        ClientPlayer *player = players.at(i);
        bool result = result_list.at(i);
        if(result)
            winner_list << player;
        else
            loser_list << player;
    }

    winner_table->setRowCount(winner_list.length());
    loser_table->setRowCount(loser_list.length());

    fillTable(winner_table, winner_list);
    fillTable(loser_table, loser_list);

    QStringList labels;
    labels << tr("General") << tr("Name") << tr("Alive") << tr("Role");
    winner_table->setHorizontalHeaderLabels(labels);
    loser_table->setHorizontalHeaderLabels(labels);

    dialog->resize(main_window->width()/2, dialog->height());
    dialog->exec();
}

void RoomScene::fillTable(QTableWidget *table, const QList<ClientPlayer *> &players){
    int i;
    for(i=0; i<players.length(); i++){
        ClientPlayer *player = players.at(i);

        QTableWidgetItem *item = new QTableWidgetItem;
        item->setText(Sanguosha->translate(player->getGeneralName()));
        table->setItem(i, 0, item);

        item = new QTableWidgetItem;
        item->setText(player->objectName());
        table->setItem(i, 1, item);

        item = new QTableWidgetItem;
        if(player->isAlive())
            item->setText(tr("Alive"));
        else
            item->setText(tr("Dead"));
        table->setItem(i, 2, item);

        item = new QTableWidgetItem;
        QIcon icon(QString(":/roles/%1.png").arg(player->getRole()));
        item->setIcon(icon);
        if(!player->isAlive())
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setText(Sanguosha->translate(player->getRole()));
        table->setItem(i, 3, item);
    }
}

void RoomScene::killPlayer(const QString &who){
    const General *general = NULL;

    if(who == Config.UserName){
        dashboard->update();

        general = Self->getGeneral();
    }else{
        Photo *photo = name2photo[who];
        photo->update();

        general = photo->getPlayer()->getGeneral();
    }

    general->lastWord();
}

void RoomScene::fillAmazingGrace(const QList<int> &card_ids){
    amazing_grace.clear();

    static const int columns = 4;
    static const qreal ratio = 0.8;

    int i;
    for(i=0; i<card_ids.length(); i++){
        int card_id = card_ids.at(i);
        CardItem *card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setScale(ratio);

        QRectF rect = card_item->boundingRect();
        int row = i / columns;
        int column = i % columns;
        QPointF pos(column * rect.width() * ratio, row * rect.height() * ratio);

        card_item->setPos(pos);
        card_item->setHomePos(pos);
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        amazing_grace << card_item;
        addItem(card_item);
    }

    int row_count = amazing_grace.length() > 4 ? 2 : 1;
    int column_count = amazing_grace.length() > 4 ? 4 : amazing_grace.length();
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

    foreach(QGraphicsSimpleTextItem *item, taker_names){
        removeItem(item);
        delete item;
    }

    amazing_grace.clear();
    taker_names.clear();
}

void RoomScene::takeAmazingGrace(const ClientPlayer *taker, int card_id){
    foreach(CardItem *card_item, amazing_grace){
        if(card_item->getCard()->getId() == card_id){
            card_item->setEnabled(false);

            CardItem *item = new CardItem(card_item->getCard());
            addItem(item);
            putCardItem(taker, Player::Hand, item);

            QString name = Sanguosha->translate(taker->getGeneralName());
            QGraphicsSimpleTextItem *text_item = addSimpleText(name, Config.SmallFont);
            text_item->setPos(card_item->pos() + QPointF(20, 70));
            taker_names << text_item;

            break;
        }
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

void RoomScene::showCard(const QString &player_name, int card_id){
    Photo *photo = name2photo.value(player_name, NULL);
    if(photo){
        photo->showCard(card_id);
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
        QMessageBox::warning(main_window, tr("Warning"), tr("The skill %1 must be view as skill!"));
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
    dashboard->addSkillButton(button);    

    connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
}

void RoomScene::detachSkill(const QString &skill_name){
    const ViewAsSkill *skill = getViewAsSkill(skill_name);

    QAbstractButton *button = button2skill.key(skill, NULL);
    if(button){
        skill_buttons.removeOne(button);
        button2skill.remove(button);
        QPushButton *push_button = qobject_cast<QPushButton *>(button);
        dashboard->removeSkillButton(push_button);

        delete button;
    }
}
