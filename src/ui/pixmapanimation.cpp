#include "pixmapanimation.h"
#include <fstream>
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

    if(QFile(path + "prop").exists())
    {
        QPixmap atlas = GetFrameFromCache(path + "atlas.png");

        int width,height,num = 0;
        std::ifstream fin(QString(path + "prop").toStdString().data());
        fin >> width >> height >> num;

        for(int i = 0; i < num; i++)
        {
            QPixmap pic(width,height);
            pic.fill(Qt::transparent);
            QPainter pt(&pic);
            pt.setClipRect(0,0,width,height);
            pt.drawPixmap(-width * i, 0, atlas);

            frames << pic;
        }
        return;
    }

    int i = 0;
    QString pic_path = QString("%1%2%3").arg(path).arg(i++).arg(".png");
    do{
        frames << GetFrameFromCache(pic_path);
        pic_path = QString("%1%2%3").arg(path).arg(i++).arg(".png");
    }while(!GetFrameFromCache(pic_path).isNull());

    QPixmap atlas(frames.last().size().width() * frames.count(),frames.last().size().height());
    atlas.fill(Qt::transparent);
    QPainter pt(&atlas);
    i = 0;

    foreach(QPixmap map,frames)
    {
        pt.drawPixmap((i++)*map.size().width(),0,map);
    }
    atlas.save(path + "atlas.png");

    using namespace std;
    ofstream cfg(QString("%1prop").arg(path).toStdString().data());

    cfg << frames.last().size().width();
    cfg << ' ' << frames.last().size().height() << ' '
        << frames.count();

    cfg.close();
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
