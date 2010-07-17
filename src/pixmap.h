#ifndef PIXMAP_H
#define PIXMAP_H

#include <QGraphicsObject>
#include <QPixmap>

class Pixmap : public QGraphicsObject
{
public:
    Pixmap(const QString &filename, bool center_as_origin = true);
    virtual QRectF boundingRect() const;
    void changePixmap(const QString &name);
    void shift();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QPixmap pixmap;
};

#endif // PIXMAP_H
