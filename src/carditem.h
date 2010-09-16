#ifndef CARDITEM_H
#define CARDITEM_H

#include "card.h"
#include "pixmap.h"

#include <QSize>
#include <QPropertyAnimation>

class CardItem : public Pixmap
{
    Q_OBJECT
public:
    CardItem(const Card *card);

    const Card *getCard() const;
    void setHomePos(QPointF home_pos);
    void goBack(bool kieru = false);
    const QPixmap &getSuitPixmap() const;
    const QPixmap &getIconPixmap() const;

    void select();
    void unselect();
    bool isPending() const;

    bool isEquipped() const;

    bool isMarked() const;
    bool isMarkable() const;
    void mark(bool marked = true);
    void setMarkable(bool markable);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    const Card *card;
    QPixmap suit_pixmap, icon_pixmap;
    QPointF home_pos;

    bool markable, marked;

signals:
    void show_discards();
    void hide_discards();
    void clicked();
    void double_clicked();
    void thrown();
    void mark_changed();
};

#endif // CARDITEM_H
