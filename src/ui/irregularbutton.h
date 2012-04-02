#ifndef IRREGULARBUTTON_H
#define IRREGULARBUTTON_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QRegion>

class IrregularButton : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit IrregularButton(const QString &name);

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    enum State { Normal, Hover, Down };
    State state;

    QPixmap normal, hover, down, disabled;
    QRegion mask;

    void changeState(State state);

signals:
    void clicked();
};

#endif // IRREGULARBUTTON_H
