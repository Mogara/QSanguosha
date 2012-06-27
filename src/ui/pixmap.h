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
    bool changePixmap(const QString &name);
    void setPixmap(const QPixmap &pixmap);
    void shift();
    void makeGray();
    void scaleSmoothly(qreal ratio);

    bool isMarked() const;
    bool isMarkable() const;
    void mark(bool marked = true);
    void setMarkable(bool markable);

    static void MakeGray(QPixmap &pixmap);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    QPixmap pixmap;

private:
    bool markable, marked;

signals:
    void mark_changed();
    void selected_changed();
    void enable_changed();
};

#endif // PIXMAP_H
