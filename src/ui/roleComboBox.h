#ifndef ROLEComboBox_H
#define ROLEComboBox_H

#include <QObject>

#include "QSanSelectableItem.h"
#include "player.h"

class Photo;

class RoleComboBoxItem : public QSanSelectableItem{
    Q_OBJECT

public:
    RoleComboBoxItem(const QString &role, int number, QSize size);
    QString getRole() const;
    void setRole(const QString &role);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    QString m_role;
    int m_number;
    QSize m_size;

signals:
    void clicked();
};

class RoleComboBox : public QGraphicsObject
{
    Q_OBJECT

public:
    RoleComboBox(QGraphicsItem *photo);
    static const int S_ROLE_COMBO_BOX_WIDTH = 25;
    static const int S_ROLE_COMBO_BOX_HEIGHT = 26;
    static const int S_ROLE_COMBO_BOX_GAP = 5;
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
public slots:
    void fix(const QString &role);
protected:
    qreal _m_posX, _m_posY;
private:
    QList<RoleComboBoxItem *> items;
    RoleComboBoxItem* m_currentRole;
private slots:
    void collapse();
    void expand();
};

#endif // ROLEComboBox_H
