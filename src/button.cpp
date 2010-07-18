#include "button.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

static Phonon::MediaSource HoverSource("audio/button-hover.wav");
static Phonon::MediaSource DownSource("audio/button-down.wav");

Button::Button(const QString &label)
    :label(label){

    QFontMetrics metrics(Config.BigFont);
    width = metrics.width(label);
    height = metrics.height();

    setFlags(QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

    effect =  Phonon::createPlayer(Phonon::MusicCategory);
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    setFocus(Qt::MouseFocusReason);

    effect->setCurrentSource(HoverSource);
    effect->play();
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event){
    event->accept();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    effect->setCurrentSource(DownSource);
    effect->play();

    emit clicked();
}

QRectF Button::boundingRect() const{
    return QRectF(-width/2 -2, -height/2 -8, width + 10, height + 10);
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setFont(Config.BigFont);
    qreal font_size = Config.BigFont.pixelSize();

    if(hasFocus()){
        painter->setPen(QPen(Qt::black));
        painter->drawText(QPointF(8 - width/2, 8 + font_size/2), label);

        painter->setPen(QPen(Qt::white));
        painter->drawText(QPointF(-2 - width/2,-2 + font_size/2),label);
    }else{
        painter->setPen(QPen(Qt::black));
        painter->drawText(QPointF(5 - width/2, 5 + font_size/2), label);

        painter->setPen(QPen(Qt::white));
        painter->drawText(QPointF(-width/2, font_size/2),label);
    }
}
