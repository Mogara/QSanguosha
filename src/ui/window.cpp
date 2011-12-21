#include "window.h"
#include "settings.h"
#include "button.h"

#include <QPainter>
#include <QGraphicsRotation>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

Window::Window(const QString &title, const QSizeF &size)
    :title(title), size(size), keep_when_disappear(false)
{
    setFlags(ItemIsMovable);

    yRotation = new QGraphicsRotation(this);
    yRotation->setAngle(90);
    yRotation->setOrigin(QVector3D(size.width()/2, size.height()/2, 0));
    yRotation->setAxis(Qt::YAxis);

    QList<QGraphicsTransform *> transforms;
    transforms << yRotation;
    setTransformations(transforms);
}

void Window::addContent(const QString &content){
    QGraphicsTextItem *content_item = new QGraphicsTextItem(this);
    content_item->moveBy(10, 40);
    content_item->setHtml(content);
    content_item->setDefaultTextColor(Qt::white);
    content_item->setTextWidth(size.width() - 40);

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

    QPainterPath path;
    path.addRoundedRect(window_rect, 10, 10);

    painter->fillPath(path, QColor(0x00, 0x00, 0x00, 0.80 * 255));

    QPen pen(Qt::white);
    pen.setWidth(3);
    painter->setPen(pen);    
    painter->drawPath(path);

    QPainterPath title_path;
    title_path.addRoundedRect(0, 0, window_rect.width(), QFontMetrics(Config.SmallFont).height() + 10, 10, 10);
    painter->fillPath(title_path, QColor(0xFF, 0xFF, 0x00, 0.43 * 255));

    painter->setFont(Config.SmallFont);

    QRectF title_rect(window_rect);
    title_rect.setY(5);
    painter->drawText(title_rect, Qt::AlignTop | Qt::AlignHCenter, title);
}

void Window::appear(){
    QPropertyAnimation *y = new QPropertyAnimation(yRotation, "angle");
    y->setEndValue(0);
    y->start();
}

void Window::disappear(){
    QPropertyAnimation *y = new QPropertyAnimation(yRotation, "angle");
    y->setEndValue(90);
    y->start();

    if(!keep_when_disappear)
        connect(y, SIGNAL(finished()), this, SLOT(deleteLater()));
}
