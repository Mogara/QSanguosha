#include "window.h"
#include "settings.h"
#include "button.h"

#include <QPainter>
#include <QGraphicsRotation>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

Window::Window(const QString &title, const QSizeF &size)
    :title(title), size(size), keep_when_disappear(false)
{
    setFlags(ItemIsMovable);

    QPixmap *bg;
    bg = size.width()>size.height() ?
                new QPixmap("image/system/tip.png")
              : new QPixmap("image/system/about.png");
    QImage bgimg = bg->toImage();
    outimg = new QImage(size.toSize(),QImage::Format_ARGB32);

    qreal pad = 10;

    int w = bgimg.width();
    int h = bgimg.height();

    int tw = outimg->width();
    int th  =outimg->height();

    qreal xc = (w - 2*pad)/(tw - 2*pad);
    qreal yc = (h - 2*pad)/(th - 2*pad);

    for(int i=0;i<tw;i++)
        for(int j=0;j<th;j++)
        {
            int x = i;
            int y = j;

            if( x>=pad && x<=(tw - pad) ) x = pad + (x - pad)*xc;
            else if(x>=(tw-pad))x = w - (tw - x);

            if( y>=pad && y<=(th - pad) ) y = pad + (y - pad)*yc;
            else if(y>=(th-pad))y = h - (th - y);


            QRgb rgb = bgimg.pixel(x,y);
            outimg->setPixel(i,j,rgb);
        }


    scaleTransform = new QGraphicsScale(this);
    scaleTransform->setXScale(1.05);
    scaleTransform->setYScale(0.95);
    scaleTransform->setOrigin(QVector3D(
                                  this->boundingRect().width()/2,
                                  this->boundingRect().height()/2,
                                  0
                                  ));

    QList<QGraphicsTransform *> transforms;
    transforms << scaleTransform;
    setTransformations(transforms);

    this->setOpacity(0.0);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(10);

    shadow->setColor(QColor(0,0,0,80));
    this->setGraphicsEffect(shadow);

    QGraphicsTextItem * titleItem = new QGraphicsTextItem(this);

    QString style;
    style.append("font-size:18pt; ");
    style.append("color:#77c379; ");
    style.append(QString("font-family: %1").arg(Config.SmallFont.family()));

    QString content;
    content.append(QString("<h style=\"%1\">%2</h>")
                   .arg(style).arg(title));

    titleItem->setHtml(content);
    titleItem->moveBy(size.width()/2 - titleItem->boundingRect().width()/2,10);
}

void Window::addContent(const QString &content){
    QGraphicsTextItem *content_item = new QGraphicsTextItem(this);
    content_item->moveBy(15, 40);
    content_item->setHtml(content);
    content_item->setDefaultTextColor(Qt::white);
    content_item->setTextWidth(size.width() - 30);

    QFont *font = new QFont();
    font->setBold(true);
    font->setPointSize(12);
    content_item->setFont(*font);
}

void Window::addCloseButton(const QString &label)
{
    Button *ok_button = new Button(label, 0.6);
    ok_button->setFont(Config.TinyFont);
    ok_button->setParentItem(this);

    qreal x = size.width() - ok_button->boundingRect().width() - 15;
    qreal y = size.height() - ok_button->boundingRect().height() - 15;
    ok_button->setPos(x, y);

    connect(ok_button, SIGNAL(clicked()), this, SLOT(disappear()));
}

void Window::shift(){
    moveBy(-size.width()/2, -size.height()/2);
}

void Window::keepWhenDisappear(){
    keep_when_disappear = true;
}

QRectF Window::boundingRect() const{
    return QRectF(QPointF(), size);
}

void Window::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QRectF window_rect = boundingRect();

    //QPainterPath path;
    //path.addRoundedRect(window_rect, 10, 10);

    //painter->fillPath(path, QColor(0x00, 0x00, 0x00, 0.80 * 255));

//    QPen pen(Qt::white);
//    pen.setWidth(3);
//    painter->setPen(pen);
//    painter->drawPath(path);

    painter->drawImage(window_rect,*outimg);

//    QPainterPath title_path;
//    title_path.addRoundedRect(0, 0, window_rect.width(), QFontMetrics(Config.SmallFont).height() + 10, 10, 10);
//    painter->fillPath(title_path, QColor(0xFF, 0xFF, 0x00, 0.43 * 255));

    painter->setFont(Config.SmallFont);
    painter->setPen(Qt::yellow);

    QRectF title_rect(window_rect);
    title_rect.setY(5);
    //painter->drawText(title_rect, Qt::AlignTop | Qt::AlignHCenter, title);



}

void Window::appear(){
    QPropertyAnimation *scale_x = new QPropertyAnimation(scaleTransform, "xScale");
    QPropertyAnimation *scale_y = new QPropertyAnimation(scaleTransform, "yScale");
    QPropertyAnimation *opacity = new QPropertyAnimation(this,"opacity");

    QParallelAnimationGroup *group = new QParallelAnimationGroup();


    scale_x->setEndValue(1);
    scale_y->setEndValue(1);
    opacity->setEndValue(1.0);
    group->addAnimation(scale_x);
    group->addAnimation(scale_y);
    group->addAnimation(opacity);

    group->start(QAbstractAnimation::DeleteWhenStopped);

}

void Window::disappear(){
    QPropertyAnimation *scale_x = new QPropertyAnimation(scaleTransform, "xScale");
    QPropertyAnimation *scale_y = new QPropertyAnimation(scaleTransform, "yScale");
    QPropertyAnimation *opacity = new QPropertyAnimation(this,"opacity");
    QParallelAnimationGroup *group = new QParallelAnimationGroup();

    scale_x->setEndValue(1.05);
    scale_y->setEndValue(0.95);
    opacity->setEndValue(0.0);
    group->addAnimation(scale_x);
    group->addAnimation(scale_y);
    group->addAnimation(opacity);

    group->start();

    if(!keep_when_disappear)
        connect(group, SIGNAL(finished()), this, SLOT(deleteLater()));
}
