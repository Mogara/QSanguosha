#include "window.h"
#include "settings.h"
#include "button.h"

#include <QPainter>
#include <QGraphicsRotation>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>

static const qreal TitleBarHeight = 35;
static const qreal ContentMargin = 15;

Window::Window(const QString &title, const QSizeF &size)
    :title(title), size(size), keep_when_disappear(false)
{
    setFlags(ItemIsMovable);
}

void Window::addCloseButton()
{
    Button *closeButton = new Button(tr("OK"));

    const qreal scale = 0.6;
    closeButton->setScale(scale);

    QSizeF closeButtonSize = closeButton->boundingRect().size();
    closeButtonSize *= scale;

    closeButton->setParentItem(this);
    closeButton->setPos(size.width() - closeButtonSize.width()  - ContentMargin,
                        size.height() - closeButtonSize.height() - ContentMargin);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(disappear()));
}

void Window::setContent(const QString &content){
    QGraphicsTextItem *content_item = new QGraphicsTextItem(this);
    content_item->moveBy(ContentMargin, ContentMargin + TitleBarHeight);
    content_item->setHtml(content);
    content_item->setDefaultTextColor(Qt::white);
    content_item->setTextWidth(size.width() - ContentMargin * 2);
}

void Window::moveToCenter(){
    moveBy(-size.width()/2, -size.height()/2);
}

void Window::keepWhenDisappear(){
    keep_when_disappear = true;
}

QRectF Window::boundingRect() const{
    return QRectF(QPointF(), size);
}

void Window::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    QRectF rect = boundingRect();

    QColor fillColor(Qt::black);
    fillColor.setAlphaF(0.8);
    painter->fillRect(rect, fillColor);

    QColor penColor(Qt::white);
    penColor.setAlphaF(0.8);
    QPen pen(penColor);
    pen.setWidth(3);

    painter->setPen(pen);
    painter->drawRect(rect);

    painter->drawLine(0, TitleBarHeight, size.width(), TitleBarHeight);

    painter->setFont(QFont(Config.SmallFont.family(), 15));

    painter->drawText(QRectF(0, 0, size.width(), TitleBarHeight), Qt::AlignCenter, title);
}

void Window::appear(){
    show();
}

void Window::disappear(){

    hide();

    if(!keep_when_disappear)
        deleteLater();
}
