#ifndef ROLECOMBOBOX_H
#define ROLECOMBOBOX_H

#include <QObject>

#include "pixmap.h"
#include "player.h"

class Photo;

class RoleComboboxItem : public Pixmap{
    Q_OBJECT

public:
    RoleComboboxItem(const QString &role, int number);
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
    void setupItems(Photo *photo);
    void hide();

public slots:
    void fix(const QString &role);

private:
    QList<RoleComboboxItem *> items;

private slots:
    void onItemClicked();
};

#endif // ROLECOMBOBOX_H
