#ifndef PIXMAP_H
#define PIXMAP_H

#include <QGraphicsObject>
#include <QPixmap>

class Pixmap : public QGraphicsObject {
    Q_OBJECT

public:
    Pixmap(const QString &filename, bool center_as_origin = false);
    Pixmap(bool center_as_origin = false);
    virtual QRectF boundingRect() const;
    bool load(const QString &filename, bool center_as_origin = false);
    bool load(const QString &filename, QSize newSize, bool center_as_origin = false);
    void setPixmap(const QPixmap &pixmap);
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
    int _m_width, _m_height;
    QPixmap pixmap;

private:
    bool _load(const QString &filename, QSize newSize, bool useNewSize, bool center_as_origin);
    bool markable, marked;

signals:
    void mark_changed();
    void selected_changed();
    void enable_changed();
};

#endif // PIXMAP_H
