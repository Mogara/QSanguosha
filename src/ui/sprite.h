#ifndef SPRITE_H
#define SPRITE_H

#include <QObject>
#include <QGraphicsItem>
#include <QMap>
#include <QEasingCurve>

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

    qreal getX();
    qreal getY();

signals:
    void finished();
public slots:

    void start(int loops = 1);

private:
    struct AnimationLine
    {
        AnimationLine(){frames[0] = 1;};
        QString name;
        QMap<int,qreal> frames;
        QMap<int,QEasingCurve::Type> easings;
    };

    QMap<QString,AnimationLine*> lines;
    int total_time;
    int resetTime;
};

#endif // SPRITE_H
