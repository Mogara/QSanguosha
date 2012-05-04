#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "pixmap.h"
#include "carditem.h"
#include "player.h"
#include "skill.h"
#include "sprite.h"
#include "protocol.h"
#include "TimedProgressBar.h"

#include <QPushButton>
#include <QComboBox>
#include <QGraphicsLinearLayout>
#include <QLineEdit>
#include <QProgressBar>
#include <QMutex>

class Dashboard : public Pixmap
{
    Q_OBJECT

public:
    Dashboard(QGraphicsItem *button_widget);
    virtual QRectF boundingRect() const;
    void setWidth(int width);
    QGraphicsProxyWidget *addWidget(QWidget *widget, int x, bool from_left);
    QPushButton *createButton(const QString &name);
    QPushButton *addButton(const QString &name, int x, bool from_left);
    
    //Progress bar functions
    void hideProgressBar();
    void showProgressBar(QSanProtocol::Countdown countdown);

    void setTrust(bool trust);
    void addCardItem(CardItem *card_item);
    CardItem *takeCardItem(int card_id, Player::Place place);
    void setPlayer(const ClientPlayer *player);
    Pixmap *getAvatar();
    void selectCard(const QString &pattern, bool forward = true);
    void useSelected();
    const Card *getSelected() const;
    void unselectAll();
    void hideAvatar();
    void setFilter(const FilterSkill *filter);
    const FilterSkill *getFilter() const;

    void disableAllCards();
    void enableCards();
    void enableAllCards();

    void installEquip(CardItem *equip);
    void installDelayedTrick(CardItem *card);

    // pending operations
    void startPending(const ViewAsSkill *skill);
    void stopPending();
    void updatePending();
    const ViewAsSkill *currentSkill() const;
    const Card *pendingCard() const;

    void killPlayer();
    void revivePlayer();

    int getRightPosition();
    int getMidPosition();
    int getButtonWidgetWidth() const;
    int getTextureWidth() const;

public slots:
    inline void onProgressBarTimedOut() { emit progressBarTimedOut(); }
    void updateAvatar();
    void updateSmallAvatar();
    void updateReadyItem(bool visible);
    void refresh();
    void doFilter();
    void sortCards(int sort_type);
    void reverseSelection();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    // ui controls
    QSanCommandProgressBar m_progressBar;
    
    // sync objects
    QMutex m_mutex;

private:
    QPixmap left_pixmap, right_pixmap;
    QGraphicsRectItem *left, *middle, *right;
    QGraphicsItem *button_widget;

    QList<CardItem*> card_items;
    CardItem *selected;
    Pixmap *avatar, *small_avatar;
    QGraphicsPixmapItem *kingdom, *ready_item;
    QGraphicsTextItem *mark_item;
    QGraphicsPixmapItem *action_item;

    int sort_type;
    QGraphicsSimpleTextItem *handcard_num;
    QList<CardItem *> judging_area;
    QList<QGraphicsItem *> delayed_tricks;
    QGraphicsPixmapItem *death_item;
    Pixmap *chain_icon, *back_icon;

    QGraphicsRectItem *equip_rects[4];
    CardItem *weapon, *armor, *defensive_horse, *offensive_horse;
    QList<CardItem **> equips;

    QGraphicsRectItem *trusting_item;
    QGraphicsSimpleTextItem *trusting_text;

    //for animated effects
    EffectAnimation *animations;

    // UI control creation
    void _addProgressBar();

    // for parts creation
    void createLeft();
    void createRight();
    void createMiddle();
    void setMiddleWidth(int middle_width);

    // for pendings
    QList<CardItem *> pendings;
    const Card *pending_card;
    const ViewAsSkill *view_as_skill;
    const FilterSkill *filter;

    void adjustCards();
    void adjustCards(const QList<CardItem *> &list, int y);
    void drawEquip(QPainter *painter, const CardItem *equip, int order);
    void setSelectedItem(CardItem *card_item);
    void drawHp(QPainter *painter) const;

private slots:
    void onCardItemClicked();
    void onCardItemThrown();
    void onCardItemHover();
    void onCardItemLeaveHover();
    void onMarkChanged();
    void setActionState();

signals:
    void card_selected(const Card *card);
    void card_to_use();
    void progressBarTimedOut();
};

#endif // DASHBOARD_H
