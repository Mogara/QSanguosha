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

static Phonon::MediaSource AddPlayerSource("audio/add-player.wav");

RoomScene::RoomScene(Client *client, int player_count)
    :client(client), bust(NULL),  effect(Phonon::createPlayer(Phonon::MusicCategory))
{
    Q_ASSERT(client != NULL);

    client->setParent(this);
    const ClientPlayer *player = client->getPlayer();
    setBackgroundBrush(Config.BackgroundBrush);

    // create skill label
    prompt_label = addSimpleText("", Config.BigFont);
    prompt_label->setPos(-350, -100);

    // create pile
    pile = new Pixmap(":/images/pile.png");
    addItem(pile);
    pile->setPos(387, -162);

    // create photos
    int i;
    for(i=0;i<player_count-1;i++){
        Photo *photo = new Photo;
        photos << photo;
        addItem(photo);
    }

    // create dashboard
    dashboard = new Dashboard;
    addItem(dashboard);
    dashboard->setPlayer(player);
    connect(player, SIGNAL(general_changed()), dashboard, SLOT(updateAvatar()));
    connect(dashboard, SIGNAL(card_discarded(CardItem*)), this, SLOT(discardCard(CardItem*)));

    // get dashboard's avatar
    avatar = dashboard->getAvatar();    

    // do signal-slot connections
    connect(client, SIGNAL(player_added(ClientPlayer*)), this, SLOT(addPlayer(ClientPlayer*)));
    connect(client, SIGNAL(player_removed(QString)), this, SLOT(removePlayer(QString)));    
    connect(client, SIGNAL(cards_drawed(QList<Card*>)), this, SLOT(drawCards(QList<Card*>)));    
    connect(client, SIGNAL(lords_got(QList<const General*>)), this, SLOT(chooseLord(QList<const General*>)));    
    connect(client, SIGNAL(generals_got(const General*,QList<const General*>)),
            this, SLOT(chooseGeneral(const General*,QList<const General*>)));

    connect(client, SIGNAL(prompt_changed(QString)), this, SLOT(changePrompt(QString)));
    connect(client, SIGNAL(seats_arranged(QList<const ClientPlayer*>)), this, SLOT(updatePhotos(QList<const Player*>)));
    connect(client, SIGNAL(n_card_drawed(ClientPlayer*,int)), this, SLOT(drawNCards(ClientPlayer*,int)));

    client->signup();

    startEnterAnimation();
}

void RoomScene::startEnterAnimation(){
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    const qreal photo_width = photos.front()->boundingRect().width();
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

            effect->setCurrentSource(AddPlayerSource);
            effect->play();

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

void RoomScene::updatePhotos(const QList<const Player*> &seats){
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

    const qreal photo_width = photos.front()->boundingRect().width();
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

void RoomScene::drawCards(const QList<Card *> &cards){
    foreach(Card * card, cards){
        CardItem *item = new CardItem(card);
        item->setPos(893, -265);
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
        Pixmap *pixmap = new Pixmap(":/images/card-back.png");
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


void RoomScene::discardCard(CardItem *card_item){
    card_item->setParentItem(NULL);
    card_item->setOpacity(1.0);
    card_item->setPos(dashboard->mapToScene(card_item->pos()));
    card_item->setHomePos(QPointF(-494 + discarded_queue.length() * 2, -115));
    QSizeF size = card_item->boundingRect().size();
    card_item->setTransformOriginPoint(size.width()/2, size.height()/2);
    card_item->setRotation(qrand() % 360);
    card_item->goBack();
    card_item->setFlags(card_item->flags() & (~QGraphicsItem::ItemIsFocusable));

    card_item->setZValue(0.1*discarded_list.length());
    discarded_list << card_item->getCard();
    discarded_queue.enqueue(card_item);

    if(discarded_queue.length() > 10){
        CardItem *first = discarded_queue.dequeue();
        delete first;
    }
}

void RoomScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mousePressEvent(event);
    if(event->button() == Qt::RightButton){
        // use skill
    }
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

    switch(event->key()){        
    case Qt::Key_F1: dashboard->sort(0); break;
    case Qt::Key_F2: dashboard->sort(1); break;
    case Qt::Key_F3: dashboard->sort(2); break;

    case Qt::Key_S: dashboard->selectCard("slash");  break;
    case Qt::Key_J: dashboard->selectCard("jink"); break;
    case Qt::Key_P: dashboard->selectCard("peach"); break;

    case Qt::Key_E: dashboard->selectCard("equip"); break;
    case Qt::Key_W: dashboard->selectCard("weapon"); break;
    case Qt::Key_H: dashboard->selectCard("horse"); break;

    case Qt::Key_T: dashboard->selectCard("trick"); break;
    case Qt::Key_A: dashboard->selectCard("aoe"); break;
    case Qt::Key_N: dashboard->selectCard("nullification"); break;
    case Qt::Key_C: dashboard->selectCard("dismantlement"); break;
    case Qt::Key_Q: dashboard->selectCard("snatch"); break;
    case Qt::Key_U: dashboard->selectCard("duel"); break;
    case Qt::Key_L: dashboard->selectCard("lightning"); break;
    case Qt::Key_I: dashboard->selectCard("indulgence"); break;
    case Qt::Key_R: dashboard->selectCard("collateral"); break;
    case Qt::Key_Y: dashboard->selectCard("god_salvation"); break;
    case Qt::Key_F: dashboard->selectCard("amazing_grace"); break;

    case Qt::Key_Space :  dashboard->selectCard(); break; // iterate all cards

    case Qt::Key_G: break; // iterate generals
    case Qt::Key_Return : dashboard->useSelected(); break;
    case Qt::Key_Escape : dashboard->unselectAll(); break;

    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
        {
            //int order = event->key() - Qt::Key_0;
            break;
        }

#ifndef _NDEBUG
    case Qt::Key_D: {
            // do some debugging things
            client->drawCards("1+2+3+4+5+6");
        }
#endif
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
        button->setIconSize(button->iconSize() * 0.8);
        layout->addWidget(button);

        mapper->setMapping(button, lord->objectName());
        connect(button, SIGNAL(double_clicked()), mapper, SLOT(map()));        
        connect(button, SIGNAL(double_clicked()), dialog, SLOT(accept()));
    }

    mapper->setMapping(dialog, lords.front()->objectName());
    connect(dialog, SIGNAL(rejected()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QString)), client, SLOT(itemChosen(QString)));

    dialog->setLayout(layout);    
    dialog->show();
}

void RoomScene::chooseGeneral(const General *lord, const QList<const General *> &generals){
    if(photos.length()>1)
        prompt_label->setText(tr("Please wait for other players choosing their generals"));

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Choose general"));
    dialog->setModal(true);

    QHBoxLayout *layout = new QHBoxLayout;
    QSignalMapper *mapper = new QSignalMapper(dialog);

    {
        QString icon_path = lord->getPixmapPath("bust");
        QString lord_name = Sanguosha->translate(lord->objectName());

        QString role_str = client->getPlayer()->getRole();
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
        button->setIconSize(button->iconSize() * 0.8);
        button->setToolTip(role_tip);

        layout->addWidget(button);
    }

    foreach(const General *general, generals){
        QString icon_path = general->getPixmapPath("card");
        QString caption = Sanguosha->translate(general->objectName());
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setIconSize(button->iconSize() * 0.8);
        layout->addWidget(button);

        mapper->setMapping(button, general->objectName());
        connect(button, SIGNAL(double_clicked()), mapper, SLOT(map()));
        connect(button, SIGNAL(double_clicked()), dialog, SLOT(accept()));
    }

    mapper->setMapping(dialog, generals.front()->objectName());
    connect(dialog, SIGNAL(rejected()), mapper, SLOT(map()));

    connect(mapper, SIGNAL(mapped(QString)), client, SLOT(itemChosen(QString)));

    dialog->setLayout(layout);
    dialog->show();
}

void RoomScene::changePrompt(const QString &prompt_str){
    prompt_label->setText(prompt_str);
}

void RoomScene::viewDiscarded(){
    if(discarded_list.isEmpty()){
        QMessageBox::information(NULL, tr("No discarded cards"), tr("There are no discarded cards yet"));
        return;
    }

    CardOverview *overview = new CardOverview;
    overview->loadFromList(discarded_list);
    overview->show();
}

