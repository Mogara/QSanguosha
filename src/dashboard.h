#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "pixmap.h"
#include "carditem.h"
#include "player.h"

#include <QPushButton>
#include <QComboBox>
#include <QStack>
#include <MediaObject>

class Dashboard : public Pixmap
{
    Q_OBJECT

public:
    Dashboard();
    void addCardItem(CardItem *card_item);
    CardItem *takeCardItem(int card_id, const QString &location);
    void setPlayer(const Player *player);
    Pixmap *getAvatar();
    void selectCard(const QString &pattern = "", bool forward = true);
    void useSelected();
    CardItem *getSelected() const;
    void unselectAll();
    void sort(int order);
    void disableAllCards();    
    void enableCards(const Client *client);
    void installEquip(CardItem *equip);

public slots:
    void updateAvatar();
    void enableCards(const QString &pattern);

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QList<CardItem*> card_items;
    CardItem *selected;
    const Player *player;
    QPixmap magatamas[5];
    Pixmap *avatar;
    QGraphicsPixmapItem *kingdom;
    bool use_skill;
    QComboBox *sort_combobox;
    CardItem *weapon, *armor, *defensive_horse, *offensive_horse;
    QStack<CardItem *> judging_area;
    Phonon::MediaObject *effect;

    void adjustCards();    
    void installDelayedTrick(CardItem *card);
    void drawEquip(QPainter *painter, CardItem *equip, int order);

private slots:
    void sortCards();
};

#endif // DASHBOARD_H
