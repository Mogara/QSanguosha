#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"
#include "player.h"

#include <QGraphicsObject>
#include <QPixmap>
#include <QComboBox>

class ClientPlayer;

class Photo : public Pixmap
{
    Q_OBJECT
public:
    explicit Photo();
    void setPlayer(const ClientPlayer *player);
    const ClientPlayer *getPlayer() const;
    void speak(const QString &content);

public slots:
    void updateAvatar();
    void updateStateStr(const QString &new_state);
    void updateRoleCombobox(const QString &new_role);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

private:
    const ClientPlayer *player;
    QPixmap avatar;
    QPixmap avatar_frame;
    QPixmap kingdom;
    QPixmap handcard;
    QPixmap magatamas[5];
    QString state_str;
    QComboBox *role_combobox;
};

#endif // PHOTOBACK_H
