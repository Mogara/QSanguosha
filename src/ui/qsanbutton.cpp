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
    _m_mouseEntered = false;

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
    QPixmap pixmap = G_ROOM_SKIN.getButtonPixmap(_m_buttonName, _m_state);
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
    if (enabled)
    {
        setState(S_STATE_UP);
        _m_mouseEntered = false;
    }
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
    QPointF point = event->pos();
    if (_m_mouseEntered || !insideButton(point)) return; // fake event;

    Q_ASSERT(_m_state != S_STATE_DISABLED &&
             _m_state != S_STATE_HOVER);
    _m_mouseEntered = true;    
    if (_m_state == S_STATE_UP){
        setState(S_STATE_HOVER);
    }
}

void QSanButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if (!_m_mouseEntered) return;

    Q_ASSERT(_m_state != S_STATE_DISABLED);
    if (_m_state == S_STATE_HOVER)
        setState(S_STATE_UP);
}

void QSanButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    QPointF point = event->pos();
    if (insideButton(point)) {
        if (!_m_mouseEntered) hoverEnterEvent(event);
    } else {
        if (_m_mouseEntered) hoverLeaveEvent(event);
    }
}

void QSanButton::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QPointF point = event->pos();
    if (!insideButton(point)) return;

    Q_ASSERT(_m_state != S_STATE_DISABLED);
    if (_m_style == S_STYLE_TOGGLE)
        return;    
    setState(S_STATE_DOWN);    
}

void QSanButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    Q_ASSERT(_m_state != S_STATE_DISABLED);    
    QPointF point = event->pos();
    bool inside = insideButton(point);    

    if (_m_style == S_STYLE_PUSH)
    {
        // Q_ASSERT(_m_state == S_STATE_DOWN);
        setState(S_STATE_UP);        
    }

    else if (_m_style == S_STYLE_TOGGLE) {
        if (_m_state == S_STATE_HOVER)
            _m_state = S_STATE_UP; // temporarily set, do not use setState!
        
        if (_m_state == S_STATE_DOWN && inside)
            setState(S_STATE_UP);
        else if (_m_state == S_STATE_UP && inside)
            setState(S_STATE_DOWN);
    }

    if (inside)
        emit clicked();
}
