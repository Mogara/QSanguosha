#include "roomscene.h"
#include "settings.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsSceneMouseEvent>

RoomScene::RoomScene():bust(NULL)
{
    setBackgroundBrush(QBrush(QPixmap(":/images/background.png")));
    skill_label = addSimpleText(Config.UserName, Config.BigFont);
    skill_label->setPos(-400, -100);

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);

    int i;
    for(i=0;i<7;i++){
        Photo *photo = new Photo;
        photos << photo;

        addItem(photo);

        qreal x = i * photo->boundingRect().width() + Config.Rect.x();
        qreal y =  Config.Rect.y() + 10;
        int duration = 1500.0 * qrand()/ RAND_MAX;

        QPropertyAnimation *translation = new QPropertyAnimation(photo, "pos");
        translation->setEndValue(QPointF(x,y));
        translation->setEasingCurve(QEasingCurve::OutBounce);
        translation->setDuration(duration);

        group->addAnimation(translation);
    }

    photos[0]->loadAvatar("generals/small/caocao.png");
    photos[1]->loadAvatar("generals/small/liubei.png");
    photos[2]->loadAvatar("generals/small/sunquan.png");
    photos[3]->loadAvatar("generals/small/simayi.png");
    photos[4]->loadAvatar("generals/small/guojia.png");
    photos[5]->loadAvatar("generals/small/zhugeliang.png");
    photos[6]->loadAvatar("generals/small/zhouyu.png");


    {
        dashboard = new Dashboard;
        dashboard->setGeneral(new General("caocao", "wei", 4, true));

        addItem(dashboard);

        avatar = dashboard->getAvatar();

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
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);

    Card *card1 = new Card("savage_assault", Card::Spade, 1);
    Card *card2 = new Card("slash", Card::Club, 7);
    Card *card3 = new Card("jink", Card::Heart, 2);
    Card *card4 = new Card("peach", Card::Diamond, 10);
    Card *card5 = new Card("archery_attack", Card::Heart, 11);
    Card *card6 = new Card("crossbow", Card::Club, 12);

    dashboard->addCard(card1);
    dashboard->addCard(card2);
    dashboard->addCard(card3);
    dashboard->addCard(card4);
    dashboard->addCard(card5);
    dashboard->addCard(card6);

    card4->setEnabled(false);
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
    QString filename = "generals/bust/" + name + ".png";
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

void RoomScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mousePressEvent(event);
    if(event->button() == Qt::RightButton){
        // use skill
    }
}

void RoomScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mouseMoveEvent(event);

    QGraphicsObject *obj = static_cast<QGraphicsObject*>(focusItem());
    Card *card = qobject_cast<Card*>(obj);
    if(!card || !card->isUnderMouse())
        return;

    foreach(Photo *photo, photos){
        if(photo->isUnderMouse()){
            photo->setSelected(true);
        }
    }

    if(avatar->isUnderMouse()){
        avatar->setSelected(true);
    }
}

