#include "roomscene.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsSceneMouseEvent>
#include <MediaObject>
#include <QMessageBox>
#include <QStatusBar>
#include <QCheckBox>

RoomScene::RoomScene(Client *client, int player_count, QMainWindow *main_window)
    :client(client), bust(NULL)
{
    Q_ASSERT(client != NULL);

    client->setParent(this);
    setBackgroundBrush(Config.BackgroundBrush);

    // create skill label
    skill_label = addSimpleText(Config.UserName, Config.BigFont);
    skill_label->setPos(-400, -100);

    // create photos
    int i;
    for(i=0;i<player_count-1;i++){
        Photo *photo = new Photo;
        photos << photo;
        addItem(photo);
    }

    // create dashboard
    dashboard = new Dashboard;
    Player *self = new Player(this);
    self->setObjectName(Config.UserName);
    self->setProperty("avatar", Config.UserAvatar);
    client->setSelf(self);
    dashboard->setPlayer(self);
    createSkillButtons(main_window, self);
    addItem(dashboard);

    // get dashboard's avatar
    avatar = dashboard->getAvatar();

    startEnterAnimation();
    client->signup();
}

void RoomScene::startEnterAnimation(){
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    const qreal photo_width = photos.front()->boundingRect().width();
    const qreal start_x = (Config.Rect.width() - photo_width*photos.length())/2 + Config.Rect.x();

    int i;
    for(i=0;i<photos.length();i++){
        Photo *photo = photos[i];
        qreal x = i * photo_width + start_x;
        qreal y =  Config.Rect.y() + 10;
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

void RoomScene::createSkillButtons(QMainWindow *main_window, Player *player){
    QStatusBar *status_bar = main_window->statusBar();
    const General *general = player->getGeneral();
    if(general == NULL)
        general = Sanguosha->getGeneral(player->property("avatar").toString());

    QObjectList skills = general->getSkills();
    foreach(QObject *skill_obj, skills){
        Skill *skill = qobject_cast<Skill*>(skill_obj);
        QPushButton *button = new QPushButton(Sanguosha->translate(skill->objectName()));
        if(skill->isCompulsory()){
            button->setText(button->text() + tr("[Compulsory]"));
            button->setDisabled(true);
        }

        if(skill->isLordSkill()){
            button->setText(button->text() + tr("[Lord Skill]"));
        }

        status_bar->addPermanentWidget(button);
        if(skill->isFrequent()){
            QCheckBox *checkbox = new QCheckBox(tr("Auto use"));
            checkbox->setChecked(true);
            status_bar->addPermanentWidget(checkbox);
        }
    }
}

void RoomScene::addPlayer(const QString &player_info){    
    QStringList words = player_info.split(":");
    if(words.length() >=2){
        Player *player = new Player(this);
        QString name = words[0];
        QString avatar = words[1];
        player->setObjectName(name);
        player->setProperty("avatar", avatar);

        int i;
        for(i=0; i<photos.length(); i++){
            Photo *photo = photos[i];
            if(!photo->isOccupied()){
                photo->setPlayer(player);
                photo_map[name] = photo;

                Phonon::MediaSource source("audio/add-player.wav");
                Phonon::MediaObject *effect = Phonon::createPlayer(Phonon::MusicCategory, source);
                effect->play();
                return;
            }
        }
    }
}

void RoomScene::removePlayer(const QString &player_name){
    Photo *photo = photo_map[player_name];
    if(photo)
        photo->setPlayer(NULL);
}

void RoomScene::updatePhotos(){
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    int i;
    for(i=0; i<photos.size(); i++){
        Photo *photo = photos[i];
        QPropertyAnimation *translation = new QPropertyAnimation(photo, "x");
        translation->setEndValue(i * photo->boundingRect().width() + Config.Rect.x());
        translation->setEasingCurve(QEasingCurve::OutBounce);

        group->addAnimation(translation);
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);
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

void RoomScene::drawCards(const QString &cards_str)
{
    QStringList card_list = cards_str.split("+");
    foreach(QString card_str, card_list){
        int card_id = card_str.toInt();
        Card *card = Sanguosha->getCard(card_id);
        dashboard->addCardItem(new CardItem(card));
    }
}

void RoomScene::nameDuplication(const QString &name){
    QMessageBox::critical(NULL, tr("Error"), tr("Name %1 duplication, you've to be offline").arg(name));
    exit(1);
}

void RoomScene::focusWarn(const QString &){
    QMessageBox::warning(NULL, tr("Warning"), tr("You are not focus"));
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

