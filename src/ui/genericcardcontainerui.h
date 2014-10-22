/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef _GENERAL_CARD_CONTAINER_UI_H
#define _GENERAL_CARD_CONTAINER_UI_H

#include "carditem.h"
#include "player.h"
#include "qsanselectableitem.h"
#include "skinbank.h"
#include "timedprogressbar.h"
#include "magatamasitem.h"
#include "rolecombobox.h"

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QMutex>
#include <QParallelAnimationGroup>
#include <QGraphicsEffect>
#include <QLabel>

class GraphicsPixmapHoverItem;

class GenericCardContainer : public QGraphicsObject {
    Q_OBJECT

public:
    inline GenericCardContainer() { _m_highestZ = 10000; }
    virtual QList<CardItem *> removeCardItems(const QList<int> &card_ids, Player::Place place) = 0;
    virtual void addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo);
    virtual QList<CardItem *> cloneCardItems(QList<int> card_ids);

protected:
    // @return Whether the card items should be destroyed after animation
    virtual bool _addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo) = 0;
    QList<CardItem *> _createCards(QList<int> card_ids);
    CardItem *_createCard(int card_id);
    void _disperseCards(QList<CardItem *> &cards, QRectF fillRegion, Qt::Alignment align, bool useHomePos, bool keepOrder);
    void _playMoveCardsAnimation(QList<CardItem *> &cards, bool destroyCards);
    int _m_highestZ;

protected slots:
    virtual void onAnimationFinished();

private slots:
    void _doUpdate();
    void _destroyCard();

private:
    static bool _horizontalPosLessThan(const CardItem *card1, const CardItem *card2);

signals:
    void animation_finished();
};

class PlayerCardContainer : public GenericCardContainer {
    Q_OBJECT

public:
    PlayerCardContainer();
    virtual void showProgressBar(QSanProtocol::Countdown countdown);
    void hideProgressBar();
    void hideAvatars();
    const ClientPlayer *getPlayer() const;
    virtual void setPlayer(ClientPlayer *player);
    inline int getVotes() { return _m_votesGot; }
    inline void setMaxVotes(int maxVotes) { _m_maxVotes = maxVotes; }
    // See _m_floatingArea for more information
    inline QRect getFloatingArea() const{ return _m_floatingAreaRect; }
    inline void setSaveMeIcon(bool visible) { _m_saveMeIcon->setVisible(visible); }
    void setFloatingArea(QRect rect);

    // repaintAll is different from refresh in that it recreates all controls and is
    // very costly. Avoid calling this except for changing skins or only once during
    // the initialization. If you just want to update the information displayed, call
    // refresh instead.
    virtual void repaintAll();
    virtual void killPlayer();
    virtual void revivePlayer();
    virtual QGraphicsItem *getMouseClickReceiver() = 0;
    inline virtual QGraphicsItem *getMouseClickReceiver2() { return NULL; }
    virtual void updateAvatarTooltip();

    inline void hookMouseEvents();

    QPixmap paintByMask(QPixmap& source);

    inline RoleComboBox *getRoleComboBox() const { return _m_roleComboBox; }

    bool canBeSelected();

    void stopHeroSkinChangingAnimation();

public slots:
    virtual void updateAvatar();
    virtual void updateSmallAvatar();
    void updatePhase();
    void updateHp();
    void updateHandcardNum();
    void updateDrankState();
    //************************************
    // Method:    updatePile
    // FullName:  PlayerCardContainer::updatePile
    // Access:    public
    // Returns:   void
    // Qualifier:
    // Parameter: const QString & pile_name
    // Description: Update the button of the private pile named pile_name.
    //              The pile button will be destructed if the pile contains no card.
    //
    // Last Updated By Yanguam Siliagim
    // To fix no-response when click "confirm" in pile box
    //
    // QSanguosha-Rara
    // March 14 2014
    //************************************
    void updatePile(const QString &pile_name);
    void updateKingdom(const QString &kingdom);
    void updateMarks();
    void updateVotes(bool need_select = true, bool display_1 = false);
    void updateReformState();
    void showDistance();
    void hideDistance();
    void onRemovedChanged();
    virtual void showSeat();
    virtual void showPile();
    virtual void refresh();

    QPixmap getHeadAvatarIcon(const QString &generalName);
    QPixmap getDeputyAvatarIcon(const QString &generalName);

    inline GraphicsPixmapHoverItem *getHeadAvartarItem() const { return _m_avatarIcon; }
    inline GraphicsPixmapHoverItem *getDeputyAvartarItem() const { return _m_smallAvatarIcon; }

    static void _paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QPixmap &pixmap, QGraphicsItem *parent);

protected:
    // overrider parent functions
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    // initialization of _m_layout is compulsory for children classes.
    virtual QGraphicsItem *_getEquipParent() = 0;
    virtual QGraphicsItem *_getDelayedTrickParent() = 0;
    virtual QGraphicsItem *_getAvatarParent() = 0;
    virtual QGraphicsItem *_getMarkParent() = 0;
    virtual QGraphicsItem *_getPhaseParent() = 0;
    virtual QGraphicsItem *_getRoleComboBoxParent() = 0;
    virtual QGraphicsItem *_getPileParent() = 0;
    virtual QGraphicsItem *_getFocusFrameParent() = 0;
    virtual QGraphicsItem *_getProgressBarParent() = 0;
    virtual QGraphicsItem *_getDeathIconParent() = 0;
    virtual QString getResourceKeyName() = 0;

    virtual QAbstractAnimation *_getPlayerRemovedEffect() = 0;
    virtual void _initializeRemovedEffect() = 0;

    virtual void _createRoleComboBox();
    void _updateProgressBar(); // a dirty function used by the class itself only.
    void _updateDeathIcon();
    void _updateEquips();
    void _paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QString &key);
    void _paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QString &key, QGraphicsItem *parent);
    void _paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QPixmap &pixmap);

    void _clearPixmap(QGraphicsPixmapItem *item);
    QPixmap _getPixmap(const QString &key);
    QPixmap _getPixmap(const QString &key, const QString &arg);
    QPixmap _getEquipPixmap(const EquipCard *equip);
    virtual void _adjustComponentZValues();
    void _updateFloatingArea();
    // We use QList of cards instead of a single card as parameter here, just in case
    // we need to do group animation in the future.
    virtual void addEquips(QList<CardItem *> &equips);
    virtual QList<CardItem *> removeEquips(const QList<int> &cardIds);
    virtual void addDelayedTricks(QList<CardItem *> &judges);
    virtual QList<CardItem *> removeDelayedTricks(const QList<int> &cardIds);
    virtual void updateDelayedTricks();

    // This is a dirty but easy design, we require children class to call create controls after
    // everything specific to the children has been setup (such as the frames that we attach
    // the controls. Consider revise this in the future.
    void _createControls();
    void _layBetween(QGraphicsItem *middle, QGraphicsItem *item1, QGraphicsItem *item2);
    void _layUnder(QGraphicsItem *item);

    bool _isSelected(QGraphicsItem *item) const;

    // layout
    const QSanRoomSkin::PlayerCardContainerLayout *_m_layout;
    QGraphicsRectItem *_m_avatarArea, *_m_secondaryAvatarArea;

    // icons;
    // painting large shadowtext every frame is very costly, so we use a
    // graphicsitem to cache the result
    QGraphicsPixmapItem *_m_avatarNameItem, *_m_secondaryAvatarNameItem;
    GraphicsPixmapHoverItem *_m_avatarIcon, *_m_smallAvatarIcon;
    QGraphicsPixmapItem *_m_circleItem;
    QGraphicsPixmapItem *_m_screenNameItem;
    QGraphicsPixmapItem *_m_chainIcon, *_m_chainIcon2;
    QGraphicsPixmapItem *_m_duanchangMask, *_m_duanchangMask2;
    QGraphicsPixmapItem *_m_faceTurnedIcon, *_m_faceTurnedIcon2;
    QGraphicsPixmapItem *_m_handCardBg, *_m_handCardNumText;
    QGraphicsPixmapItem *_m_kingdomColorMaskIcon, *_m_kingdomColorMaskIcon2;
    QGraphicsPixmapItem *_m_deathIcon;
    QGraphicsPixmapItem *_m_actionIcon;
    QGraphicsPixmapItem *_m_kingdomIcon;
    QGraphicsPixmapItem *_m_saveMeIcon;
    QGraphicsPixmapItem *_m_phaseIcon;
    QGraphicsPixmapItem *_m_extraSkillBg;
    QGraphicsPixmapItem *_m_extraSkillText;
    QGraphicsPixmapItem *leftDisableShowLock;
    QGraphicsPixmapItem *rightDisableShowLock;
    QGraphicsTextItem *_m_markItem;
    QGraphicsPixmapItem *_m_selectedFrame, *_m_selectedFrame2;
    QGraphicsProxyWidget *_m_privatePileArea;
    QMap<QString, QGraphicsProxyWidget *> _m_privatePiles;

    // The frame that is maintained by roomscene. Items in this area has positions
    // or contents that cannot be decided based on the information of PlayerCardContainer
    // alone. It is relative to other components in the roomscene. One use case is
    // phase area of dashboard;
    QRect _m_floatingAreaRect;
    QGraphicsPixmapItem *_m_floatingArea;

    QList<QGraphicsPixmapItem *> _m_judgeIcons;
    QList<CardItem *> _m_judgeCards;

    QGraphicsProxyWidget *_m_equipRegions[S_EQUIP_AREA_LENGTH];
    CardItem *_m_equipCards[S_EQUIP_AREA_LENGTH];
    QLabel *_m_equipLabel[S_EQUIP_AREA_LENGTH];
    QParallelAnimationGroup *_m_equipAnim[S_EQUIP_AREA_LENGTH];
    QMutex _mutexEquipAnim;

    // controls
    MagatamasBoxItem *_m_hpBox;
    RoleComboBox *_m_roleComboBox;
    QSanCommandProgressBar *_m_progressBar;
    QGraphicsProxyWidget *_m_progressBarItem;

    // in order to apply different graphics effect;
    QGraphicsPixmapItem *_m_groupMain;
    QGraphicsPixmapItem *_m_groupDeath;

    // now, logic
    ClientPlayer *m_player;

    // The following stuffs for mulitple votes required for yeyan
    int _m_votesGot, _m_maxVotes;
    QGraphicsPixmapItem *_m_votesItem;

    // The following stuffs for showing distance
    QGraphicsPixmapItem *_m_distanceItem;

    // The following stuffs for showing seat
    QGraphicsPixmapItem *_m_seatItem;

protected slots:
    virtual void _onEquipSelectChanged();

private:
    bool _startLaying();
    void clearVotes();
    int _lastZ;
    bool _allZAdjusted;
signals:
    void selected_changed();
    void enable_changed();
};

#endif

