#ifndef WINDOW_H
#define WINDOW_H

class QGraphicsRotation;

#include <QGraphicsObject>

class Window : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Window(const QString &title, const QSizeF &size);
    void addContent(const QString &content);
    void addCloseButton(const QString &label);
    void shift();

    virtual QRectF boundingRect() const;

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public slots:
    void appear();
    void disappear();

private:
    QString title;
    QGraphicsRotation *yRotation;
    QSizeF size;
};

#endif // WINDOW_H
