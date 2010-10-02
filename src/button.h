#ifndef BUTTON_H
#define BUTTON_H

#include "settings.h"
#include "audiere.h"

#include <QGraphicsObject>
#include <QFont>
#include <QFontMetrics>


class Button : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Button(const QString &label);

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString label;
    qreal width, height;
    audiere::SoundEffectPtr down_effect, hover_effect;

signals:
    void clicked();
};

#endif // BUTTON_H
