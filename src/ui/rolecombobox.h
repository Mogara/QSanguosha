#ifndef ROLECOMBOBOX_H
#define ROLECOMBOBOX_H

#include <QObject>

#include "pixmap.h"
#include "player.h"

class Photo;

class RoleComboboxItem : public Pixmap{
    Q_OBJECT

public:
    RoleComboboxItem(const QString &role, int number, QSize size);
    QString getRole() const;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    QString role;

signals:
    void clicked();
};

class RoleCombobox : public QObject
{
    Q_OBJECT

public:
    RoleCombobox(Photo *photo);
    void hide();
    void show();
    void setPos(qreal x, qreal y);
    void setPos(QPointF pos);
    static const int S_ROLE_COMBO_BOX_WIDTH = 25;
    static const int S_ROLE_COMBO_BOX_HEIGHT = 26;
    static const int S_ROLE_COMBO_BOX_GAP = 5;
public slots:
    void fix(const QString &role);
protected:
    qreal _m_posX, _m_posY;
    bool _m_expanded;
private:
    QList<RoleComboboxItem *> items;

private slots:
    void onItemClicked();
};

#endif // ROLECOMBOBOX_H
