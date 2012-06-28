#include "sprite.h"

#include <QAnimationGroup>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QtCore/qmath.h>
#include <QPainter>

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

    if(--loops)connect(pgroup,SIGNAL(finished()),this,SLOT(start(int)));
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

qreal Sprite::getX() const
{
    return pos().x();
}

qreal Sprite::getY() const
{
    return pos().y();
}

EffectAnimation::EffectAnimation()
    :QObject()
{
    effects.clear();
    registered.clear();
}

void EffectAnimation::fade(QGraphicsItem *map)
{
    QAnimatedEffect * effect = qobject_cast<QAnimatedEffect *>(map->graphicsEffect());

    if(effect)
    {
        effectOut(map);

        effect = registered.value(map);
        if(effect)effect->deleteLater();

        registered.insert(map,new FadeEffect(true));
        return;
    }


    map->show();
    FadeEffect *fade = new FadeEffect(true);
    map->setGraphicsEffect(fade);
    effects.insert(map,fade);
}

void EffectAnimation::emphasize(QGraphicsItem *map,bool stay)
{
    QAnimatedEffect * effect = qobject_cast<QAnimatedEffect *>(map->graphicsEffect());

    if(effect)
    {
        effectOut(map);

        effect = registered.value(map);
        if(effect)effect->deleteLater();

        registered.insert(map,new EmphasizeEffect(stay));
        return;
    }


    EmphasizeEffect *emphasize = new EmphasizeEffect(stay);
    map->setGraphicsEffect(emphasize);
    effects.insert(map,emphasize);
}

void EffectAnimation::sendBack(QGraphicsItem *map)
{
    QAnimatedEffect * effect = qobject_cast<QAnimatedEffect *>(map->graphicsEffect());

    if(effect)
    {
        effectOut(map);

        effect = registered.value(map);
        if(effect)effect->deleteLater();

        registered.insert(map,new SentbackEffect(true));
        return;
    }


    SentbackEffect *sendBack = new SentbackEffect(true);
    map->setGraphicsEffect(sendBack);
    effects.insert(map,sendBack);
}

void EffectAnimation::effectOut(QGraphicsItem *map)
{
    QAnimatedEffect * effect = qobject_cast<QAnimatedEffect *>(map->graphicsEffect());

    if(effect)
    {
        //deleteEffect(effect);
        effect->setStay(false);
        connect(effect,SIGNAL(loop_finished()),this,SLOT(deleteEffect()));
        //QTimer::singleShot(1000,this,SLOT(deleteEffect()));
    }

    effect = registered.value(map);
    if(effect)effect->deleteLater();
    registered.insert(map,NULL);
}

void EffectAnimation::deleteEffect()
{
    QAnimatedEffect * effect = qobject_cast<QAnimatedEffect * >(sender());
    deleteEffect(effect);
}

void EffectAnimation::deleteEffect(QAnimatedEffect *effect)
{
    if(!effect)return;

    effect->deleteLater();

    QGraphicsItem *pix = effects.key(effect);
    if(pix)
    {
        QAnimatedEffect * effect = registered.value(pix);
        if(effect)effect->reset();
        pix->setGraphicsEffect(registered.value(pix));
        effects.insert(pix,registered.value(pix));
        registered.insert(pix,NULL);
    }
}

EmphasizeEffect::EmphasizeEffect(bool stay,QObject *parent)
{
    this->setObjectName("emphasizer");
    index = 0;
    this->stay = stay;
    QPropertyAnimation * anim = new QPropertyAnimation(this,"index");
    connect(anim,SIGNAL(valueChanged(QVariant)),this,SLOT(update()));
    anim->setEndValue(40);
    anim->setDuration((40 - index)* 5);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void EmphasizeEffect::draw(QPainter *painter){

    QSizeF s = this->sourceBoundingRect().size();

    qreal scale = ( - qAbs(index - 50) + 50 )/1000.0;
    scale = 0.1 - scale;

    QPoint offset;

    QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);

    const QRectF target = boundingRect().adjusted(
                  s.width()*scale-1,
                  s.height()*scale,
                  -s.width()*scale,
                  -s.height()*scale
                );

    const QRectF source(s.width()*0.1,
                        s.height()*0.1,
                        s.width(),
                        s.height());


    painter->setRenderHint(QPainter::SmoothPixmapTransform);
    painter->drawPixmap(target, pixmap, source);
}

QRectF EmphasizeEffect::boundingRectFor(const QRectF &sourceRect) const
{
    qreal scale = 0.1;
    QRectF rect(sourceRect);
    rect.adjust(  -   sourceRect.width() * scale,
                  -   sourceRect.height() * scale,
                      sourceRect.width() * scale,
                      sourceRect.height() * scale);
    return rect;
}

void QAnimatedEffect::setStay(bool stay)
{
    this->stay=stay;

    if(!stay)
    {
        QPropertyAnimation * anim = new QPropertyAnimation(this,"index");
        anim->setEndValue(0);
        anim->setDuration(index * 5);

        connect(anim,SIGNAL(finished()),this,SLOT(deleteLater()));
        connect(anim,SIGNAL(valueChanged(QVariant)),this,SLOT(update()));
        anim->start(QAbstractAnimation::DeleteWhenStopped);


    }
}

SentbackEffect::SentbackEffect(bool stay, QObject *parent)
{
    grayed = 0;
    this->setObjectName("backsender");
    index = 0;
    this->stay = stay;

    QPropertyAnimation * anim = new QPropertyAnimation(this,"index");
    connect(anim,SIGNAL(valueChanged(QVariant)),this,SLOT(update()));
    anim->setEndValue(40);
    anim->setDuration((40 - index)* 5);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

QRectF SentbackEffect::boundingRectFor(const QRectF &sourceRect) const
{
    qreal scale = 0.05;
    QRectF rect(sourceRect);
    rect.adjust(-sourceRect.width() * scale,
                -sourceRect.height() * scale,
                 sourceRect.width() * scale,
                 sourceRect.height() * scale);
    return rect;
}

void SentbackEffect::draw(QPainter *painter)
{
    QPoint offset;

    QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);

    if(!grayed)
    {
        grayed = new QImage(pixmap.size(),QImage::Format_ARGB32);

        QImage image = pixmap.toImage();
        int width = image.width();
        int height = image.height();
        int gray;

        QRgb col;

        for (int i = 0; i < width; ++i)
        {
            for (int j = 0; j < height; ++j)
            {
                col = image.pixel(i, j);
                gray = qGray(col) >> 1;
                grayed->setPixel(i, j, qRgba(gray,gray,gray,qAlpha(col)));
            }
        }
    }

    painter->drawPixmap(offset,pixmap);
    painter->setOpacity((40 - qAbs(index - 40))/80.0);
    painter->drawImage(offset,*grayed);

    return;
}

FadeEffect::FadeEffect(bool stay, QObject *parent)
{
    this->setObjectName("fader");
    index = 0;
    this->stay = stay;

    QPropertyAnimation * anim = new QPropertyAnimation(this,"index");
    connect(anim,SIGNAL(valueChanged(QVariant)),this,SLOT(update()));
    anim->setEndValue(40);
    anim->setDuration((40 - index)* 5);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FadeEffect::draw(QPainter *painter)
{
    QPoint offset;

    QPixmap pixmap = sourcePixmap(Qt::LogicalCoordinates, &offset);


    painter->setOpacity(index/40.0);
    painter->drawPixmap(offset,pixmap);
}
