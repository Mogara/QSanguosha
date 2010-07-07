#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"
#include "player.h"

#include <QGraphicsObject>
#include <QPixmap>

class Photo : public Pixmap
{
    Q_OBJECT
public:
    explicit Photo();
    void setPlayer(const Player *player);
    const Player *getPlayer() const;
    void speak(const QString &content);

public slots:
    void updateAvatar();
    void changeHandCardNum(int num);
    void updateStateStr(const QString &new_state);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

private:
    const Player *player;
    QPixmap avatar;
    QPixmap avatar_frame;
    QPixmap kingdom;
    QPixmap handcard;
    int handcard_num;
    QPixmap magatamas[5];
    QString state_str;
};

#endif // PHOTOBACK_H
