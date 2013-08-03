#include "button.h"
#include "audio.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRotation>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include "engine.h"

static QRectF ButtonRect(0, 0, 189, 46);

Button::Button(const QString &label, qreal scale)
    :label(label), size(ButtonRect.size() * scale),
    mute(true), font(Config.SmallFont)
{

    init();
}

Button::Button(const QString &label, const QSizeF &size)
    :label(label), size(size), mute(true), font(Config.SmallFont)
{
    init();
}

void Button::init()
{
    setFlags(ItemIsFocusable);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void Button::setMute(bool mute){
    this->mute = mute;
}

void Button::setFont(const QFont &font){
    this->font = font;
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *){
    setFocus(Qt::MouseFocusReason);

#ifdef AUDIO_SUPPORT

    if(!mute)
        Sanguosha->playAudio("button-hover");

#endif

}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event){
    event->accept();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *){
#ifdef AUDIO_SUPPORT

    if(!mute)
        Sanguosha->playAudio("button-down");

#endif

    emit clicked();
}

QRectF Button::boundingRect() const{
    return QRectF(QPointF(), size);
}

static QColor ReverseColor(const QColor &color){
    int r = 0xFF - color.red();
    int g = 0xFF - color.green();
    int b = 0xFF - color.blue();
    return QColor::fromRgb(r, g, b);
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    QRectF rect = boundingRect();

    QColor textColor, edgeColor, boxColor;
    textColor = edgeColor = Qt::white;
    boxColor = Qt::black;

    if(hasFocus()){
        textColor = ReverseColor(textColor);
        boxColor = ReverseColor(boxColor);
    }

    boxColor.setAlphaF(0.8);

    painter->fillRect(rect, boxColor);

    QPen pen(edgeColor);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->drawRect(rect);

    pen.setColor(textColor);
    painter->setPen(pen);
    painter->setFont(font);
    painter->drawText(rect, Qt::AlignCenter, label);
}
