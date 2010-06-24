#include "roomscene.h"
#include "settings.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsSceneMouseEvent>

RoomScene::RoomScene()
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
        bottom = new Bottom;
        addItem(bottom);

        QPointF start_pos(Config.Rect.topLeft());
        QPointF end_pos(Config.Rect.x(), Config.Rect.bottom() - bottom->boundingRect().height());
        int duration = 1500;

        QPropertyAnimation *translation = new QPropertyAnimation(bottom, "pos");
        translation->setStartValue(start_pos);
        translation->setEndValue(end_pos);
        translation->setEasingCurve(QEasingCurve::OutBounce);
        translation->setDuration(duration);

        QPropertyAnimation *enlarge = new QPropertyAnimation(bottom, "scale");
        enlarge->setStartValue(0.2);
        enlarge->setEndValue(1.0);
        enlarge->setEasingCurve(QEasingCurve::OutBounce);
        enlarge->setDuration(duration);

        group->addAnimation(translation);
        group->addAnimation(enlarge);
    }

    group->start(QAbstractAnimation::DeleteWhenStopped);
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

void RoomScene::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QGraphicsScene::mousePressEvent(event);
    if(event->button() == Qt::RightButton){
        // use skill
    }
}

