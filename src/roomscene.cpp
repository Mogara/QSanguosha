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

static const QPointF DiscardedPos(-494, -115);
static QSize GeneralSize(200 * 0.8, 290 * 0.8);

RoomScene::RoomScene(int player_count, QMainWindow *main_window)
    :bust(NULL), main_window(main_window)
{
    connect(this, SIGNAL(selectionChanged()), this, SLOT(updateSelectedTargets()));

    ClientInstance->setParent(this);
    const ClientPlayer *player = ClientInstance->getPlayer();
    setBackgroundBrush(Config.BackgroundBrush);

    // create pile
    pile = new Pixmap(":/pile.png");
    addItem(pile);
    pile->setPos(387, -132);

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
    dashboard->setPlayer(player);
    connect(player, SIGNAL(general_changed()), dashboard, SLOT(updateAvatar()));
    connect(player, SIGNAL(general_changed()), this, SLOT(updateSkillButtons()));
    connect(ClientInstance, SIGNAL(card_requested(QString)), dashboard, SLOT(enableCards(QString)));
    connect(dashboard, SIGNAL(card_selected(const Card*)), this, SLOT(enableTargets(const Card*)));
    connect(dashboard, SIGNAL(card_to_use()), this, SLOT(useSelectedCard()));

    // add role combobox
    role_combobox = new QComboBox;
    role_combobox->addItem(tr("Your role"));
    role_combobox->addItem(tr("Unknown"));
    connect(player, SIGNAL(role_changed(QString)), this, SLOT(updateRoleComboBox(QString)));

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
    connect(ClientInstance, SIGNAL(lords_got(QList<const General*>)), SLOT(chooseLord(QList<const General*>)));
    connect(ClientInstance, SIGNAL(generals_got(const General*,QList<const General*>)),
            SLOT(chooseGeneral(const General*,QList<const General*>)));
    connect(ClientInstance, SIGNAL(prompt_changed(QString)),  SLOT(changePrompt(QString)));
    connect(ClientInstance, SIGNAL(seats_arranged(QList<const ClientPlayer*>)), SLOT(updatePhotos(QList<const ClientPlayer*>)));
    connect(ClientInstance, SIGNAL(n_card_drawed(ClientPlayer*,int)), SLOT(drawNCards(ClientPlayer*,int)));    
    connect(ClientInstance, SIGNAL(card_moved(CardMoveStructForClient)), this, SLOT(moveCard(CardMoveStructForClient)));
    connect(ClientInstance, SIGNAL(status_changed(Client::Status)), this, SLOT(updateStatus(Client::Status)));
    connect(ClientInstance, SIGNAL(avatars_hiden()), this, SLOT(hideAvatars()));

    daqiao = new Daqiao;
    daqiao->shift();
    daqiao->hide();
    addItem(daqiao);

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

void RoomScene::showBust(const QString &name)
{
    const General *general = Sanguosha->getGeneral(name);
    QString filename = general->getPixmapPath("bust");
    if(!bust){
        bust = new Pixmap(filename);
        bust->shift();
        addItem(bust);
    }else
        bust->changePixmap(filename);

    QPropertyAnimation *appear = new QPropertyAnimation(bust, "scale");
    appear->setStartValue(0.2);    
    appear->setEndValue(1.0);

    appear->start();

    connect(appear, SIGNAL(finished()), bust, SIGNAL(visibleChanged()));
}

void RoomScene::drawCards(const QList<const Card *> &cards){
    foreach(const Card * card, cards){
        CardItem *item = new CardItem(card);
        item->setPos(893, -235);
        switch(ClientInstance->getStatus()){
        case Client::NotActive: item->setEnabled(false); break;
        case Client::Playing: item->setEnabled(card->isAvailable()); break;
        case Client::Responsing: item->setEnabled(ClientInstance->pattern->match(card)); break;
        case Client::Discarding: item->setEnabled(true); break;
        }

        dashboard->addCardItem(item);
    }
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

    case Qt::Key_Left: dashboard->selectCard("", false); break;
    case Qt::Key_Right:
    case Qt::Key_Space:  dashboard->selectCard(); break; // iterate all cards
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

void RoomScene::chooseLord(const QList<const General *> &lords){
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Choose lord"));
    dialog->setModal(true);
    QHBoxLayout *layout = new QHBoxLayout;
    QSignalMapper *mapper = new QSignalMapper(dialog);

    foreach(const General *lord, lords){
        QString icon_path = lord->getPixmapPath("card");
        QString caption = Sanguosha->translate(lord->objectName());
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setIconSize(GeneralSize);
        layout->addWidget(button);

        mapper->setMapping(button, lord->objectName());
        connect(button, SIGNAL(double_clicked()), mapper, SLOT(map()));        
        connect(button, SIGNAL(double_clicked()), dialog, SLOT(accept()));
    }

    mapper->setMapping(dialog, lords.first()->objectName());
    connect(dialog, SIGNAL(rejected()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QString)), ClientInstance, SLOT(itemChosen(QString)));

    dialog->setLayout(layout);    
    dialog->show();
}

void RoomScene::chooseGeneral(const General *lord, const QList<const General *> &generals){
    if(photos.length()>1)
        changePrompt(tr("Please wait for other players choosing their generals"));

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Choose general"));
    dialog->setModal(true);

    QHBoxLayout *layout = new QHBoxLayout;
    QSignalMapper *mapper = new QSignalMapper(dialog);

    {
        QString icon_path = lord->getPixmapPath("bust");
        QString lord_name = Sanguosha->translate(lord->objectName());

        QString role_str = ClientInstance->getPlayer()->getRole();
        QString role_tip;
        if(role_str == "loyalist")
            role_tip = tr("This is your boss, help him kill all rebels and renegades");
        else if(role_str == "rebel")
            role_tip = tr("Kill this guy and you will win");
        else if(role_str == "renegade")
            role_tip = tr("Kill all other guys, and beat him at final PK");

        role_str = Sanguosha->translate(role_str);

        QString caption = tr("Lord is %1\nYour role is %2").arg(lord_name).arg(role_str);
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setIconSize(GeneralSize);
        button->setToolTip(role_tip);

        layout->addWidget(button);
    }

    foreach(const General *general, generals){
        QString icon_path = general->getPixmapPath("card");
        QString caption = Sanguosha->translate(general->objectName());
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setIconSize(GeneralSize);
        layout->addWidget(button);

        mapper->setMapping(button, general->objectName());
        connect(button, SIGNAL(double_clicked()), mapper, SLOT(map()));
        connect(button, SIGNAL(double_clicked()), dialog, SLOT(accept()));
    }

    mapper->setMapping(dialog, generals.first()->objectName());
    connect(dialog, SIGNAL(rejected()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QString)), ClientInstance, SLOT(itemChosen(QString)));

    dialog->setLayout(layout);
    dialog->show();
}

void RoomScene::changePrompt(const QString &prompt_str){
    if(prompt_str.isNull())
        main_window->statusBar()->clearMessage();
    else
        main_window->statusBar()->showMessage(prompt_str);
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
        if(src->objectName() == Config.UserName){
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
    }else{
        CardItem *card_item = NULL;
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

        return card_item;
    }
}

void RoomScene::moveCard(const CardMoveStructForClient &move){
    ClientPlayer *src = move.from;
    ClientPlayer *dest = move.to;
    Player::Place src_place = move.from_place;
    Player::Place dest_place = move.to_place;
    int card_id = move.card_id;

    static Phonon::MediaSource install_equip_source("audio/install-equip.wav");

    CardItem *card_item = takeCardItem(src, src_place, card_id);
    if(card_item->scene() == NULL)
        addItem(card_item);

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

        case Player::DelayedTrick:{
                dashboard->installDelayedTrick(card_item);
                break;
            }

        default:
            ;
            // FIXME
        }
    }else{
        Photo *photo = name2photo.value(dest->objectName());
        if(photo){
            if(dest_place == Player::Equip){
                photo->installEquip(card_item);
                Sanguosha->playEffect(install_equip_source);
            }else if(dest_place == Player::Hand)
                photo->addCardItem(card_item);
        }
    }
}

void RoomScene::updateSkillButtons(){
    const Player *player = qobject_cast<const Player *>(sender());
    QString general_name = player->getGeneral();
    if(general_name.isEmpty())
        return;
    const General *general = Sanguosha->getGeneral(general_name);

    main_window->setStatusBar(NULL);
    skill_buttons.clear();
    button2skill.clear();
    QStatusBar *status_bar = main_window->statusBar();

    const QList<const Skill*> &skills = general->findChildren<const Skill *>();
    foreach(const Skill* skill, skills){
        QAbstractButton *button = NULL;
        QString skill_name = Sanguosha->translate(skill->objectName());
        if(skill->inherits("PassiveSkill")){
            const PassiveSkill *passive_skill = qobject_cast<const PassiveSkill *>(skill);
            switch(passive_skill->getFrequency()){
            case PassiveSkill::Frequent:{
                QCheckBox *checkbox = new QCheckBox(skill_name);

                checkbox->setObjectName(skill->objectName());
                checkbox->setChecked(false);
                connect(checkbox, SIGNAL(stateChanged(int)), ClientInstance, SLOT(updateFrequentFlags(int)));
                checkbox->setChecked(true);

                button = checkbox;
                break;
            }
            case PassiveSkill::NotFrequent:
                button = new QPushButton(skill_name);
                break;

            case PassiveSkill::Compulsory:
                button = new QPushButton(skill_name);
                button->setEnabled(false);
                break;
            }
        }else
            button = new QPushButton(skill_name);

        if(skill->isLordSkill())
            button->setText(button->text() + tr("[Lord Skill]"));

        button->setToolTip(skill->getDescription());

        status_bar->addPermanentWidget(button);
        skill_buttons << button;

        if(skill->inherits("ViewAsSkill")){
            button2skill.insert(button, qobject_cast<const ViewAsSkill *>(skill));
            connect(button, SIGNAL(clicked()), this, SLOT(doSkillButton()));
        }
    }

    status_bar->addPermanentWidget(role_combobox);
}

void RoomScene::updateRoleComboBox(const QString &new_role){
    role_combobox->setItemText(1, Sanguosha->translate(new_role));
    role_combobox->setItemIcon(1, QIcon(QString(":/roles/%1.png").arg(new_role)));

    if(new_role != "lord"){
        foreach(QAbstractButton *button, skill_buttons){
            if(button->text().contains(tr("[Lord Skill]")))
                button->setDisabled(true);
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
        changePrompt();
        return;
    }

    changePrompt(tr("You choosed card [%1]").arg(card->getName()));

    if(card->targetFixed()){
        avatar->setEnabled(true);
        avatar->setFlag(QGraphicsItem::ItemIsSelectable, false);
        foreach(Photo *photo, photos){
            photo->setEnabled(true);
            photo->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }
        return;
    }

    avatar->setEnabled(card->targetFilter(selected_targets, ClientInstance->getPlayer()));

    foreach(Photo *photo, photos){
        if(card->targetFilter(selected_targets, photo->getPlayer())){
            photo->setEnabled(true);
            photo->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }else{
            photo->setEnabled(false);
            photo->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }
    }

    if(Config.EnableAutoTarget)
        selectNextTarget(false);
}

void RoomScene::updateSelectedTargets(){
    selected_targets.clear();

    QStringList general_names;
    foreach(Photo *photo, photos){
        if(photo->isSelected()){
            const ClientPlayer *target = photo->getPlayer();
            selected_targets << target;
            general_names << Sanguosha->translate(target->getGeneral());
        }
    }

    if(selected_targets.isEmpty()){
        changePrompt();
        return;
    }

    QString targets = general_names.join(",");
    const Card *card = dashboard->getSelected();
    if(card){
        QString card_name = card->getName();
        changePrompt(tr("You choose %1 as [%2]'s target").arg(targets).arg(card_name));
    }else
        changePrompt(tr("You choose %1 as target").arg(targets));
}

void RoomScene::useSelectedCard(){
    const Card *card = dashboard->getSelected();
    if(card)
        useCard(card);
    else
        changePrompt(tr("You didn't choose any card to use yet!"));
}

void RoomScene::useCard(const Card *card){
    if(card->targetFixed() || card->targetsFeasible(selected_targets))
        ClientInstance->useCard(card, selected_targets);
    else
        changePrompt(tr("Not enough targets"));

    enableTargets(NULL);
}

void RoomScene::startViewAsSkill(const ViewAsSkill *skill){
    dashboard->startPending(skill);

    QAbstractButton *button = button2skill.key(skill);
    if(button){
        button->setEnabled(false);
        ok_button->setEnabled(true);
        cancel_button->setEnabled(true);
    }
}

void RoomScene::callViewAsSkill(){
    const Card *card = dashboard->pendingCard();

    if(card == NULL){
        changePrompt(tr("Not enough cards to call skill"));
        return;
    }

    if(card->isAvailable()){
        const ViewAsSkill *skill = dashboard->currentSkill();
        skill->playEffect(); // FIXME: tell the server to play effect on other clients

        // use card
        dashboard->stopPending();
        useCard(card);
    }else{
        changePrompt(tr("Card [%1] can not be used right now").arg(card->getName()));
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
            ok_button->setText("NotActive");

            dashboard->disableAllCards();
            setSkillButtonEnablity(false);            

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);

            break;
        }
    case Client::Responsing: {
            ok_button->setText("Responsing");

            dashboard->enableCards(ClientInstance->pattern);

            ok_button->setEnabled(true);
            cancel_button->setDisabled(ClientInstance->pattern->compulsory);
            discard_button->setEnabled(false);            
            break;
        }

    case Client::Playing:{
            ok_button->setText("Playing");

            dashboard->enableCards();
            setSkillButtonEnablity(true);

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(true);
            break;
        }
    case Client::Discarding:{
            ok_button->setText("Discarding");

            setSkillButtonEnablity(false);

            ok_button->setEnabled(false);
            cancel_button->setEnabled(false);
            discard_button->setEnabled(false);
            break;
        }
    }
}

void RoomScene::setSkillButtonEnablity(bool enablity){
    foreach(QAbstractButton *button, skill_buttons)
        button->setEnabled(enablity);
}

void RoomScene::doSkillButton(){
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    const ViewAsSkill *skill = button2skill.value(button, NULL);

    if(skill)
        startViewAsSkill(skill);
}

void RoomScene::doOkButton(){
    switch(ClientInstance->getStatus()){
    case Client::Playing:{
            const ViewAsSkill *skill = dashboard->currentSkill();
            if(skill)
                callViewAsSkill();
            break;
        }
    case Client::Responsing:{

        }
    case Client::NotActive: break;
    case Client::Discarding: break;
    }
}

void RoomScene::doCancelButton(){
    // FIXME

    switch(ClientInstance->getStatus()){
    case Client::Playing:{
            const ViewAsSkill *skill = dashboard->currentSkill();
            if(skill)
                cancelViewAsSkill();
            break;
        }

    default:
        ;
    }
}

void RoomScene::doDiscardButton(){
    // FIXME
}

void RoomScene::hideAvatars(){
    foreach(Photo *photo, photos)
        photo->hideAvatar();

    dashboard->hideAvatar();
}
