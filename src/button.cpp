#include "button.h"
#include "irrKlang.h"

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRotation>
#include <QPropertyAnimation>

extern irrklang::ISoundEngine *SoundEngine;

static QRectF ButtonRect(0, 0, 189, 46);

Button::Button(const QString &label)
    :label(label){

    QFontMetrics metrics(Config.BigFont);
    width = metrics.width(label);
    height = metrics.height();

    setFlags(ItemIsFocusable);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);

//    QGraphicsRotation *yRotation = new QGraphicsRotation(this);
//    yRotation->setAxis(Qt::YAxis);
//    yRotation->setOrigin(QVector3D(ButtonRect.width()/2, ButtonRect.height()/2, 0));
//    //yRotation->setAngle(45);
//    setTransformations(QList<QGraphicsTransform *>() << yRotation);

//    QPropertyAnimation *animation = new QPropertyAnimation(yRotation, "angle");
//    animation->setStartValue(0.0);
//    animation->setKeyValueAt(0.5, 90);
//    animation->setEndValue(0.0);

//    animation->setLoopCount(5);


//    animation->start();
}

void Button::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    setFocus(Qt::MouseFocusReason);

    if(SoundEngine)
        SoundEngine->play2D("audio/system/button-hover.ogg");
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event){
    event->accept();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(SoundEngine)
        SoundEngine->play2D("audio/system/button-down.ogg");

    emit clicked();
}



QRectF Button::boundingRect() const{
    return ButtonRect;
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QPainterPath path;
    path.addRoundedRect(ButtonRect, 5, 5);

    QColor rect_color(Qt::black);
    if(hasFocus())
        rect_color = QColor(0xFF, 0xFF, 0x00);
    rect_color.setAlpha(0.43 * 255);
    painter->fillPath(path, rect_color);

    QPen pen(Qt::white);
    pen.setWidth(3);
    painter->setPen(pen);
    painter->drawPath(path);

    painter->setFont(Config.SmallFont);
    painter->setPen(Qt::white);
    painter->drawText(ButtonRect, Qt::AlignCenter, label);
}
