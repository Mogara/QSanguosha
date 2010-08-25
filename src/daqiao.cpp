#include "daqiao.h"
#include "settings.h"

#include <QPainter>

Daqiao::Daqiao()
    :Pixmap(":/daqiao.png", true)
{
    setOpacity(0.8);
    setFlag(ItemIsMovable);
}

void Daqiao::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    static QRectF ContentRect(59, 100, 234, 77);

    Pixmap::paint(painter, option, widget);

    painter->setFont(Config.TinyFont);
    painter->drawText(ContentRect, content);
}

void Daqiao::setContent(const QString &content){
    if(this->content != content){
        this->content = content;
        if(content.isEmpty())
            hide();
        else
            show();

        update();
    }
}
