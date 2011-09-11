#include "indicatoritem.h"
#include "engine.h"

#include <QPainter>
#include <QGraphicsBlurEffect>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QPauseAnimation>

IndicatorItem::IndicatorItem(const QPointF &start, const QPointF &real_finish, Player *player)
    :start(start), finish(start), real_finish(real_finish)
{
    setGraphicsEffect(new QGraphicsBlurEffect);
    color = Sanguosha->getKingdomColor(player->getKingdom());
    width = player->isLord() ? 12 : 8;
}

void IndicatorItem::doAnimation(){
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup(this);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "finish");
    animation->setEndValue(real_finish);

    QPauseAnimation *pause = new QPauseAnimation(1000);

    group->addAnimation(animation);
    group->addAnimation(pause);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    connect(group, SIGNAL(finished()), this, SLOT(deleteLater()));
}

QPointF IndicatorItem::getFinish() const{
    return finish;
}

void IndicatorItem::setFinish(const QPointF &finish){
    this->finish = finish;
    update();
}

void IndicatorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QPen pen(color);
    pen.setWidthF(width);

    painter->setPen(pen);
    painter->drawLine(mapFromScene(start), mapFromScene(finish));
}

QRectF IndicatorItem::boundingRect() const{
    qreal width = qAbs(start.x() - real_finish.x());
    qreal height = qAbs(start.y() - real_finish.y());

    return QRectF(0, 0, width, height);
}
