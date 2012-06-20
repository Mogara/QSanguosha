#ifndef _QSAN_BUTTON_H
#define _QSAN_BUTTON_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QRegion>
#include <qrect.h>

class QSanButton : public QGraphicsObject
{
    Q_OBJECT
public:
    QSanButton(const QString &buttonName, QGraphicsItem* parent);
    enum ButtonState { S_STATE_UP, S_STATE_HOVER, S_STATE_DOWN, S_STATE_DISABLED };
    enum ButtonStyle { S_STYLE_PUSH, S_STYLE_TOGGLE};
    void setSize(QSize size);
    void click();
    void setStyle(ButtonStyle style);
    void setState(ButtonState state);
    void setRect(QRect rect);
    virtual QRectF boundingRect() const;
    bool insideButton(QPointF pos) const;
    void setEnabled(bool enabled);
protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    ButtonState _m_state;
    ButtonStyle _m_style;
    QString _m_buttonName;
    QRegion _m_mask;
    QSize _m_size;
    bool _m_mouseEntered;
signals:
    void clicked();
};
#endif // IRREGULARBUTTON_H
