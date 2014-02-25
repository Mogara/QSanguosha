#ifndef _ROLE_COMBO_BOX_H
#define _ROLE_COMBO_BOX_H

#include <QGraphicsObject>
#include <QPainter>
#include <QGraphicsSceneEvent>

class RoleComboBox: public QGraphicsObject {
    Q_OBJECT

public:
    RoleComboBox(QGraphicsItem *photo, bool circle = false);
    static const int COMPACT_BORDER_WIDTH = 1;
    static const int COMPACT_ITEM_LENGTH = 10;

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    inline bool isExpanding() const {    return expanding;    };

private:

    bool circle;
    bool expanding;
    QMap<QString, bool> kingdoms_excluded;
    QString fixed_role;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

public slots:
    void fix(const QString &role);
    void mouseClickedOutside();
};

#endif
