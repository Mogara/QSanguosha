#include "pixmapanimation.h"
#include <QPainter>

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
        QString pic_path = QString("%1%2%3").arg(path).arg(i).arg(".png");
        QPixmap *frame;
        if(Loaded_animation_pixmaps[pic_path] != NULL)frame = Loaded_animation_pixmaps[pic_path];
        else    frame = new QPixmap(pic_path);
        if(frame->isNull())break;

        frames << frame;
        Loaded_animation_pixmaps[pic_path] = frame;
        i++;
    }

    current = 0;
}

void PixmapAnimation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->drawPixmap(0,0,*frames.at(current));
}

QRectF PixmapAnimation::boundingRect() const
{
    return frames.at(current)->rect();
}

bool PixmapAnimation::valid()
{
    return frames.size();
}

void PixmapAnimation::timerEvent(QTimerEvent *e)
{
    advance(1);
}

void PixmapAnimation::start(bool permanent,int interval)
{
    startTimer(interval);
    if(!permanent)connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

PixmapAnimation* PixmapAnimation::GetPixmapAnimation(QGraphicsObject *parent, const QString &emotion)
{
    PixmapAnimation *pma = new PixmapAnimation();
    pma->setPath(QString("image/system/emotion/%1/").arg(emotion));
    if(pma->valid())
    {
        if(emotion == "slash_red" ||
                emotion == "slash_black" ||
                emotion == "thunder_slash" ||
                emotion == "peach" ||
                emotion == "analeptic")
        {
            pma->moveBy(pma->boundingRect().width()*0.15,
                        pma->boundingRect().height()*0.15);
            pma->setScale(0.7);
        }
        else if(emotion == "no-success")
        {
            pma->moveBy(pma->boundingRect().width()*0.15,
                        pma->boundingRect().height()*0.15);
            pma->setScale(0.7);
        }

        pma->moveBy((parent->boundingRect().width() - pma->boundingRect().width())/2,
                (parent->boundingRect().height() - pma->boundingRect().height())/2);

        {
            if(emotion == "fire_slash")pma->moveBy(40,0);
        }
        pma->setParentItem(parent);
        pma->startTimer(50);
        connect(pma,SIGNAL(finished()),pma,SLOT(deleteLater()));
        return pma;
    }
    else
    {
        delete pma;
        return NULL;
    }
}

void PixmapAnimation::LoadEmotion(const QString &emotion)
{
    int i = 0;
    QString path = QString("image/system/emotion/%1/").arg(emotion);
    while(true)
    {
        QString pic_path = QString("%1%2%3").arg(path).arg(i).arg(".png");
        QPixmap *frame;
        if(Loaded_animation_pixmaps[pic_path] != NULL)frame = Loaded_animation_pixmaps[pic_path];
        else    frame = new QPixmap(pic_path);
        if(frame->isNull())break;

        Loaded_animation_pixmaps[pic_path] = frame;
        i++;
    }
}
