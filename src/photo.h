#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"
#include "player.h"
#include "carditem.h"

#include <QGraphicsObject>
#include <QPixmap>
#include <QComboBox>

class ClientPlayer;

class Photo : public Pixmap
{
    Q_OBJECT
public:
    explicit Photo(int order);
    void setPlayer(const ClientPlayer *player);
    const ClientPlayer *getPlayer() const;
    void speak(const QString &content);
    CardItem *takeCardItem(int card_id, Player::Place place);
    void installEquip(CardItem *equip);
    void installDelayedTrick(CardItem *trick);
    void addCardItem(CardItem *card_item);
    void hideAvatar();
    void showCard(int card_id);

public slots:
    void updateAvatar();
    void updateRoleCombobox(const QString &new_role);
    void refresh();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    const ClientPlayer *player;
    QPixmap avatar;
    QPixmap avatar_frame;
    QPixmap kingdom;
    QPixmap handcard;
    QPixmap chain;
    QPixmap magatamas[6];
    QComboBox *role_combobox;
    CardItem *weapon, *armor, *defensive_horse, *offensive_horse;
    QStack<QGraphicsPixmapItem *> judging_pixmaps;
    QStack<CardItem *> judging_area;
    QGraphicsPixmapItem *order_item;
    bool hide_avatar;
    QPixmap death_pixmap;
    Pixmap *back_icon;

    void drawEquip(QPainter *painter, CardItem *equip, int order);
};

#endif // PHOTOBACK_H
