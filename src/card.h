#ifndef CARD_H
#define CARD_H

#include <QGraphicsObject>
#include <QGraphicsColorizeEffect>
#include <QSize>
#include <QPropertyAnimation>

class Card : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(bool red READ isRed STORED false CONSTANT)
    Q_PROPERTY(bool black READ isBlack STORED false CONSTANT)
    Q_PROPERTY(QString suit READ getSuit CONSTANT)
    Q_PROPERTY(int number READ getNumber CONSTANT)
    Q_PROPERTY(QString number_string READ getNumberString CONSTANT)
    Q_PROPERTY(QString type READ getType)
public:
    // enumeration type
    enum Suit {Spade, Club, Heart, Diamond};
    enum Type {Basic, Equip, Trick};

    // constructor
    Card(const QString name, enum Suit suit, int number);

    // property getter
    QString getSuit() const;
    bool isRed() const;
    bool isBlack() const;
    int getNumber() const;
    QString getNumberString() const;
    QString getType() const;

    // others
    void setHomePos(QPointF home_pos);
    void goBack();
    void viewAs(const QString &view_card_name);

    virtual QRectF boundingRect() const;

    // compare static functions
    static bool CompareBySuitNumber(Card *a, Card *b);
    static bool CompareByType(Card *a, Card *b);
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QString name;
    enum Suit suit;
    int number;
    QPixmap pixmap;
    QPixmap suit_pixmap;
    QGraphicsColorizeEffect *monochrome_effect;
    QPointF home_pos;
    QGraphicsPixmapItem *view_card;
    Type type;

private slots:
    void setMonochrome();
};

#endif // CARD_H
