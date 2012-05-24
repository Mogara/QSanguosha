#ifndef PHOTO_H
#define PHOTO_H

#include "pixmap.h"
#include "player.h"
#include "carditem.h"
#include "protocol.h"
#include "TimedProgressBar.h"
#include "GeneralCardContainerUI.h"

#include <QGraphicsObject>
#include <QPixmap>
#include <QComboBox>
#include <QProgressBar>

class ClientPlayer;
class RoleCombobox;
class QPushButton;

class Photo : public PlayerCardContainer
{
    Q_OBJECT

public:
    explicit Photo();
    void setPlayer(const ClientPlayer *player);
    const ClientPlayer *getPlayer() const;
    void speak(const QString &content);
    QList<CardItem*> removeCardItems(const QList<int> &card_id, Player::Place place);    
    void installEquip(CardItem *equip);
    void installDelayedTrick(CardItem *trick);    
    void hideAvatar();
    void showCard(int card_id);
    void showProgressBar(QSanProtocol::Countdown countdown);
    void hideProgressBar();
    void setEmotion(const QString &emotion, bool permanent = false);
    void tremble();
    void showSkillName(const QString &skill_name);
    void createRoleCombobox();
    void setOrder(int order);
    void revivePlayer();

    enum FrameType{
        Playing,
        Responsing,
        SOS,
        NoFrame
    };

    void setFrame(FrameType type);
    virtual QRectF boundingRect() const;
    static const int S_NORMAL_PHOTO_WIDTH = 130;
    static const int S_NORMAL_PHOTO_HEIGHT = 150;
    static const int S_SHADOW_INCLUSIVE_PHOTO_WIDTH = 139;
    static const int S_SHADOW_INCLUSIVE_PHOTO_HEIGHT = 155;
public slots:
    void updateAvatar();    
    void updateSmallAvatar();
    void updateReadyItem(bool visible);
    void updatePhase();
    void updatePile(const QString &pile_name);
    void refresh();
    void hideEmotion();
    void hideSkillName();
    void setDrankState();
    void setActionState();
    void killPlayer();

protected:
    bool _addCardItems(QList<CardItem*> &card_items, Player::Place place);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    static const QRect S_CARD_MOVE_REGION;
    QList<CardItem*> m_takenOffCards;
private:
    const ClientPlayer *player;
    QPixmap avatar, small_avatar;
    QGraphicsPixmapItem *ready_item;    
    QPixmap _m_mainFrame;
    QPixmap _m_handCardIcon;
    QPixmap _m_kingdomIcon;
    QPixmap _m_kindomColorMaskIcon;
    RoleCombobox *role_combobox;
    QGraphicsProxyWidget  *pile_button;
    QGraphicsPixmapItem *action_item, *save_me_item;    
    bool permanent;

    QGraphicsTextItem *mark_item;

    CardItem *weapon, *armor, *defensive_horse, *offensive_horse;
    QList<CardItem **> equips;
    QGraphicsRectItem *equip_rects[4];

    QList<QGraphicsPixmapItem *> judging_pixmaps;    
    QList<CardItem *> judging_area;

    QMap<QString, QGraphicsPixmapItem *> mark_items;
    QMap<QString, QGraphicsSimpleTextItem *> mark_texts;

    QGraphicsPixmapItem *order_item;
    bool hide_avatar;
    QPixmap death_pixmap;
    QPixmap back_icon, chain_icon;
    QSanCommandProgressBar *progress_bar;
    QGraphicsPixmapItem *emotion_item, *frame_item;
    QGraphicsSimpleTextItem *skill_name_item;
    QGraphicsRectItem *avatar_area, *small_avatar_area;

    void drawEquip(QPainter *painter, CardItem *equip, int order);
    void drawHp(QPainter *painter);
    void drawMagatama(QPainter *painter, int index, const QPixmap &pixmap);
};

#endif // PHOTOBACK_H
