#ifndef WINDOW_H
#define WINDOW_H

#include <QGraphicsScale>

#include <QGraphicsObject>

class Window : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Window(const QString &title, const QSizeF &size);
    void addContent(const QString &content);
    void addCloseButton(const QString &label);
    void shift();
    void keepWhenDisappear();

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
    void appear();
    void disappear();

private:
    QString title;
    QGraphicsScale *scaleTransform;
    QSizeF size;
    bool keep_when_disappear;
    QImage *outimg;
};

#endif // WINDOW_H
