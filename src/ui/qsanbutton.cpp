#include "qsanbutton.h"
#include "clientplayer.h"
#include "SkinBank.h"

#include <QPixmap>
#include <qbitmap.h>
#include <QPainter>
#include <QGraphicsSceneHoverEvent>


QSanButton::QSanButton(const QString &buttonName, QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    _m_state = S_STATE_UP;
    _m_style = S_STYLE_PUSH;
    _m_buttonName = buttonName;

    QPixmap pixmap = G_ROOM_SKIN.getButtonPixmap(buttonName, _m_state);
    setSize(pixmap.size());    

    setAcceptsHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void QSanButton::click(){
    if (isEnabled())
        emit clicked();
}

QRectF QSanButton::boundingRect() const{
    return QRectF(0, 0, _m_size.width(), _m_size.height());
}

void QSanButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QPixmap& pixmap = G_ROOM_SKIN.getButtonPixmap(_m_buttonName, _m_state);
    painter->drawPixmap(0, 0, _m_size.width(), _m_size.height(), pixmap);
}

void QSanButton::setSize(QSize newSize)
{
    _m_size = newSize;
    QPixmap pixmap = G_ROOM_SKIN.getButtonPixmap(_m_buttonName, _m_state);
    _m_mask = QRegion(pixmap.mask().scaled(newSize));
}

void QSanButton::setRect(QRect rect)
{
    setSize(rect.size());
    setPos(rect.topLeft());
}

void QSanButton::setStyle(ButtonStyle style)
{
    _m_style = style;
}

void QSanButton::setEnabled(bool enabled)
{
    if (enabled) setState(S_STATE_UP);
    QGraphicsObject::setEnabled(enabled);
    if (!enabled) setState(S_STATE_DISABLED);
    update();
}

void QSanButton::setState(QSanButton::ButtonState state)
{
    if (this->_m_state != state){
        this->_m_state = state;
        update();
    }
}

bool QSanButton::insideButton(QPointF pos) const
{
    return _m_mask.contains(QPoint(pos.x(), pos.y()));
}

void QSanButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_ASSERT(_m_state != S_STATE_DISABLED);        
    QPointF point = event->pos();
    if (insideButton(point)){
        setState(S_STATE_HOVER);
    }
}

void QSanButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    Q_ASSERT(_m_state != S_STATE_DISABLED);
    setState(S_STATE_UP);
}

void QSanButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    Q_ASSERT(_m_state != S_STATE_DISABLED);
    QPointF point = event->pos();
    if (insideButton(point)) {
        setState(S_STATE_HOVER);
    } else {
        setState(S_STATE_UP);
    }
}

void QSanButton::mousePressEvent(QGraphicsSceneMouseEvent *event){
    Q_ASSERT(_m_state != S_STATE_DISABLED);
    QPointF point = event->pos();
    if (insideButton(point)) {
        setState(S_STATE_DOWN);
    }
}

void QSanButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    Q_ASSERT(_m_state != S_STATE_DISABLED);
    QPointF point = event->pos();
    if (insideButton(point)) {
        if (_m_style == S_STYLE_PUSH)
            setState(S_STATE_UP);
        else
        {
            if (_m_state == S_STATE_DOWN)
                _m_state = S_STATE_UP;
            else
                _m_state = S_STATE_DOWN;
        }
        emit clicked();
    }    
}
