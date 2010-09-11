#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "pixmap.h"
#include "carditem.h"
#include "player.h"
#include "skill.h"

#include <QPushButton>
#include <QComboBox>
#include <QStack>
#include <QGraphicsLinearLayout>

class Dashboard : public Pixmap
{
    Q_OBJECT

public:
    Dashboard();
    void addCardItem(CardItem *card_item);
    CardItem *takeCardItem(int card_id, Player::Place place);
    void setPlayer(const Player *player);
    Pixmap *getAvatar();
    void selectCard(const QString &pattern = "", bool forward = true);
    void useSelected();
    const Card *getSelected() const;
    void unselectAll();
    void sort(int order);
    void hideAvatar();

    void disableAllCards();
    void enableCards();
    void enableCards(const QString &pattern);

    void installEquip(CardItem *equip);
    void installDelayedTrick(CardItem *card);    

    // pending operations
    void startPending(const ViewAsSkill *skill);
    void stopPending();
    const ViewAsSkill *currentSkill() const;    
    const Card *pendingCard() const;

    void addSkillButton(QPushButton *button);
    void removeSkillButton(QPushButton *button);

public slots:
    void updateAvatar();
    void refresh();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    QList<CardItem*> card_items;
    CardItem *selected;
    const Player *player;
    QPixmap magatamas[6];
    Pixmap *avatar;
    QGraphicsPixmapItem *kingdom;
    bool use_skill;

    QComboBox *sort_combobox;
    CardItem *weapon, *armor, *defensive_horse, *offensive_horse;
    QStack<CardItem *> judging_area;
    QGraphicsLinearLayout *button_layout;
    QPixmap death_pixmap;
    Pixmap *chain_icon, *back_icon;
    QMap<QPushButton *, QGraphicsWidget *> button2widget;

    // for pendings
    QList<CardItem *> pendings;
    const Card *pending_card;
    const ViewAsSkill *view_as_skill;

    void adjustCards();
    void adjustCards(const QList<CardItem *> &list, int y);    
    void drawEquip(QPainter *painter, const CardItem *equip, int order);    
    void setSelectedItem(CardItem *card_item);

private slots:
    void sortCards();
    void onCardItemClicked();
    void onCardItemThrown();
    void updateEnablity(CardItem *card_item);

signals:
    void card_selected(const Card *card);
    void card_to_use();
};

#endif // DASHBOARD_H
