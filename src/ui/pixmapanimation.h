#ifndef PIXMAPANIMATION_H
#define PIXMAPANIMATION_H

#include <QGraphicsPixmapItem>

class PixmapAnimation : public QObject,public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    PixmapAnimation(QGraphicsScene *scene = 0);

    QRectF boundingRect() const;
    void advance(int phase);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void timerEvent(QTimerEvent *e);

    void setPath(const QString &path);
    bool valid();

    void start(bool permanent = true,int interval = 50);

signals:
    void finished();

private:
    QString path;
    QList<QPixmap> frames;
    int current;
};

#endif // PIXMAPANIMATION_H
