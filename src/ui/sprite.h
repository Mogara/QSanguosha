#ifndef SPRITE_H
#define SPRITE_H

#include <QObject>
#include <QTimer>
#include <QGraphicsItem>
#include <QGraphicsEffect>
#include <QMap>
#include <QEasingCurve>

#include "pixmap.h"

class Sprite : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    Q_PROPERTY(qreal x READ getX WRITE setX)
    Q_PROPERTY(qreal y READ getY WRITE setY)
public:
    explicit Sprite(QGraphicsItem *parent = 0);

    void addKeyFrame(int time,const QString & property, qreal value,QEasingCurve::Type easing = QEasingCurve::Linear);
    void setResetTime(int time);
    void setPixmapAtMid(const QPixmap &pixmap);

    qreal getX() const;
    qreal getY() const;

signals:
    void finished();
public slots:

    void start(int loops = 1);

private:
    struct AnimationLine
    {
        AnimationLine(){frames[0] = 1;}
        QString name;
        QMap<int,qreal> frames;
        QMap<int,QEasingCurve::Type> easings;
    };

    QMap<QString,AnimationLine*> lines;
    int total_time;
    int resetTime;
};

class QAnimatedEffect : public QGraphicsEffect
{
    Q_OBJECT
    Q_PROPERTY(int index READ getIndex WRITE setIndex)
public:
    void setStay(bool stay);
    void reset(){index =0;}
    int getIndex(){return index;}
    void setIndex(int ind){index = ind;}

protected:
    bool stay;
    int index;
signals:
    void loop_finished();
};

class EffectAnimation : public QObject
{
    Q_OBJECT
public:
    EffectAnimation();

    void fade(QGraphicsItem * map);
    void emphasize(QGraphicsItem *map,bool stay = true);
    void sendBack(QGraphicsItem *map);
    void effectOut(QGraphicsItem *map);
    void deleteEffect(QAnimatedEffect* effect);
public slots:

    void deleteEffect();
private:
    QMap<QGraphicsItem*,QAnimatedEffect*> effects;
    QMap<QGraphicsItem*,QAnimatedEffect*> registered;
};



class EmphasizeEffect : public QAnimatedEffect
{
    Q_OBJECT

public:
    EmphasizeEffect(bool stay = false,QObject *parent = 0);


protected:
    virtual void draw(QPainter *painter);
    virtual QRectF boundingRectFor(const QRectF &sourceRect) const;

};

class SentbackEffect : public QAnimatedEffect
{
    Q_OBJECT
public:
    SentbackEffect(bool stay = false,QObject * parent = 0);



protected:
    virtual void draw(QPainter *painter);
    virtual QRectF boundingRectFor(const QRectF &sourceRect) const;

private:
    QImage *grayed;
};

class FadeEffect : public QAnimatedEffect
{
    Q_OBJECT
public:
    FadeEffect(bool stay = false, QObject * parent = 0);

protected:
    virtual void draw(QPainter *painter);
};

#endif // SPRITE_H
