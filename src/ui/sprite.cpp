#include "sprite.h"
#include "QAnimationGroup"
#include "QPropertyAnimation"
#include "QParallelAnimationGroup"
#include "QSequentialAnimationGroup"

Sprite::Sprite(QGraphicsItem *parent) :
    QGraphicsPixmapItem(parent),total_time(0)
{
}

void Sprite::addKeyFrame(int time, const QString &property, qreal value, QEasingCurve::Type easing)
{
    AnimationLine *line = lines[property];
    if(!line)line = lines[property] = new AnimationLine();

    line->frames[time] = value;
    line->easings[time]= easing;
    if(time>total_time)total_time=time;
}

void Sprite::start(int loops)
{
    if(loops<0)loops=0;
    QMapIterator<QString, AnimationLine*> i(lines);
    QParallelAnimationGroup *pgroup = new QParallelAnimationGroup;

    while (i.hasNext())  {
        i.next();
        AnimationLine * line = i.value();
        const QByteArray &name = i.key().toLocal8Bit();
        QSequentialAnimationGroup *sgroup = new QSequentialAnimationGroup;

        QMapIterator<int,qreal> j(line->frames);

        while(j.hasNext())
        {
            j.next();

            QPropertyAnimation *trans = new QPropertyAnimation(this,name);
            trans->setStartValue(j.value());
            trans->setEasingCurve(line->easings[j.key()]);
            if(j.hasNext())
            {
                trans->setEndValue(j.peekNext().value());
                trans->setDuration(j.peekNext().key()-j.key());
            }
            else
            {
                trans->setEndValue(j.value());
                trans->setDuration(total_time - j.key());
            }
            sgroup->addAnimation(trans);
        }
        QPropertyAnimation *trans = new QPropertyAnimation(this,name);
        trans->setEndValue(line->frames[0]);
        trans->setDuration(resetTime);
        sgroup->addAnimation(trans);

        pgroup->addAnimation(sgroup);
    }

    if(--loops)connect(pgroup,SIGNAL(finished()),this,SLOT(start(loops)));
    else connect(pgroup,SIGNAL(finished()),this,SLOT(deleteLater()));
    pgroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void Sprite::setResetTime(int time)
{
    resetTime = time;
}

void Sprite::setPixmapAtMid(const QPixmap &pixmap)
{
    QGraphicsPixmapItem::setPixmap(pixmap);

    this->setOffset( - pixmap.width()/2, - pixmap.height()/2);
}

qreal Sprite::getX()
{
    return pos().x();
}

qreal Sprite::getY()
{
    return pos().y();
}
