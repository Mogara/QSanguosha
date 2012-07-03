#include "pixmapanimation.h"
#include "SkinBank.h"

#include <QPainter>
#include <QPixmapCache>
#include <QDir>

PixmapAnimation::PixmapAnimation(QGraphicsScene *scene) :
    QGraphicsItem(0,scene)
{
}

void PixmapAnimation::advance(int phase)
{
    if (phase) current++;
    if (current >= frames.size())
    {
        current = 0;
        emit finished();
    }
    update();
}

void PixmapAnimation::setPath(const QString &path)
{
    frames.clear();
    current = 0;

    int i = 0;
    QString pic_path = QString("%1%2%3").arg(path).arg(i++).arg(".png");
    do{
        frames << G_ROOM_SKIN.getPixmapFromFileName(pic_path);
        pic_path = QString("%1%2%3").arg(path).arg(i++).arg(".png");
    }while(G_ROOM_SKIN.getPixmapFromFileName(pic_path).size().height() > 1);
}

void PixmapAnimation::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap(0, 0, frames.at(current));
}

QRectF PixmapAnimation::boundingRect() const
{
    return frames.at(current).rect();
}

bool PixmapAnimation::valid()
{
    return !frames.isEmpty();
}

void PixmapAnimation::timerEvent(QTimerEvent *)
{
    advance(1);
}

void PixmapAnimation::start(bool permanent,int interval)
{
    _m_timerId = startTimer(interval);
    if (!permanent) connect(this,SIGNAL(finished()),this,SLOT(deleteLater()));
}

void PixmapAnimation::stop()
{
    killTimer(_m_timerId);
}

PixmapAnimation* PixmapAnimation::GetPixmapAnimation(QGraphicsItem *parent, const QString &emotion)
{
    PixmapAnimation *pma = new PixmapAnimation();
    pma->setPath(QString("image/system/emotion/%1/").arg(emotion));
    if(pma->valid())
    {
        if (emotion == "slash_red" ||
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
            if (emotion == "fire_slash") pma->moveBy(40,0);
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

QPixmap PixmapAnimation::GetFrameFromCache(const QString &filename){
    QPixmap pixmap;
    if(!QPixmapCache::find(filename, &pixmap)){
        pixmap.load(filename);
        if(!pixmap.isNull())
            QPixmapCache::insert(filename, pixmap);
    }

    return pixmap;
}

int PixmapAnimation::GetFrameCount(const QString &emotion){
    QString path = QString("image/system/emotion/%1/").arg(emotion);
    QDir dir(path);
    return dir.entryList(QDir::Files | QDir::NoDotAndDotDot).count();
}
