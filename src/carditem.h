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
    bool isEquipped() const;

    bool isMarked() const;
    bool isMarkable() const;
    void mark(bool marked = true);
    void setMarkable(bool markable);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    const Card *card;
    QPixmap suit_pixmap, icon_pixmap;
    QPointF home_pos;

    bool markable, marked;

signals:
    void show_discards();
    void hide_discards();
    void card_selected(CardItem *card_item);
    void pending(CardItem *item, bool add_to_pendings);
};

#endif // CARDITEM_H
