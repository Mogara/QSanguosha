#ifndef PIXMAP_H
#define PIXMAP_H

#include <QGraphicsObject>
#include <QPixmap>

class Pixmap : public QGraphicsObject{
    Q_OBJECT

public:
    Pixmap(const QString &filename, bool center_as_origin = true);
    Pixmap();
    virtual QRectF boundingRect() const;
    void changePixmap(const QString &name);
    void shift();

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    QPixmap pixmap;
};

#endif // PIXMAP_H
