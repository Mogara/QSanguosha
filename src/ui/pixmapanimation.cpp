#include "pixmapanimation.h"
#include "QPainter.h"

PixmapAnimation::PixmapAnimation(QGraphicsScene *scene) :
    QGraphicsItem(0,scene)
{
}

void PixmapAnimation::advance(int phase)
{
    if(phase)current++;
    if(current>=frames.size())
    {
        current = 0;
        emit finished();
    }
    update();
}

void PixmapAnimation::setPath(const QString &path)
{
    frames.clear();

    int i = 0;
    while(true)
    {
        QPixmap frame(QString("%1%2%3").arg(path).arg(i).arg(".png"));
        if(frame.isNull())break;
        frames << frame;
        i++;
    }

    current = 0;
}

void PixmapAnimation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawPixmap(0,0,frames.at(current));
}

QRectF PixmapAnimation::boundingRect() const
{
    return frames.at(current).rect();
}

bool PixmapAnimation::valid()
{
    return frames.size();
}

void PixmapAnimation::timerEvent(QTimerEvent *e)
{
    advance(1);
}
