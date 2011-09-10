#ifndef BUTTON_H
#define BUTTON_H

#include "settings.h"

#include <QGraphicsObject>
#include <QFont>
#include <QFontMetrics>

class Button : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Button(const QString &label, qreal scale = 1.0);
    explicit Button(const QString &label, const QSizeF &size);
    void setMute(bool mute);
    void setFont(const QFont &font);

    virtual QRectF boundingRect() const;    

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QString label;
    QSizeF size;
    bool mute;
    QFont font;

    void init();

signals:
    void clicked();
};

#endif // BUTTON_H
