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

#ifndef _DASHBOARD_H
#define _DASHBOARD_H

#include "qsanselectableitem.h"
#include "qsanbutton.h"
#include "carditem.h"
#include "player.h"
#include "skill.h"
#include "timedprogressbar.h"
#include "genericcardcontainerui.h"
#include "pixmapanimation.h"

#include <QPushButton>
#include <QComboBox>
#include <QGraphicsLinearLayout>
#include <QLineEdit>
#include <QMutex>
#include <QPropertyAnimation>

class HeroSkinContainer;
class GraphicsPixmapHoverItem;

class Dashboard : public PlayerCardContainer {
    Q_OBJECT
    Q_ENUMS(SortType)

public:
    enum SortType { ByType, BySuit, ByNumber };

    Dashboard(QGraphicsItem *buttonWidget);

    virtual QRectF boundingRect() const;
    void refresh();
    void repaintAll();
    void setWidth(int width);
    int getMiddleWidth();
    inline QRectF getRightAvatarArea() {
        QRectF rect;
        rect.setSize(layout->m_avatarArea.size());
        QPointF topLeft = mapFromItem(_getAvatarParent(), layout->m_avatarArea.topLeft());
        rect.moveTopLeft(topLeft);
        return rect;
    }
    inline QRectF getLeftAvatarArea() {
        QRectF rect;
        rect.setSize(layout->m_secondaryAvatarArea.size());
        QPointF topLeft = mapFromItem(_getAvatarParent(), layout->m_secondaryAvatarArea.topLeft());
        rect.moveTopLeft(topLeft);
        return rect;
    }

    void hideControlButtons();
    void showControlButtons();
    virtual void showProgressBar(QSanProtocol::Countdown countdown);

    QSanSkillButton *removeSkillButton(const QString &skillName);
    QSanSkillButton *addSkillButton(const QString &skillName, const bool &head = true);
    bool isAvatarUnderMouse();

    void highlightEquip(QString skillName, bool hightlight);

    void setTrust(bool trust);
    virtual void killPlayer();
    virtual void revivePlayer();
    virtual void setDeathColor();
    void selectCard(const QString &pattern, bool forward = true, bool multiple = false);
    void selectEquip(int position);
    void selectOnlyCard(bool need_only = false);
    void useSelected();
    const Card *getSelected() const;
    void unselectAll(const CardItem *except = NULL);
    void hideAvatar();

    void disableAllCards();
    void enableCards();
    void enableAllCards();

    void adjustCards(bool playAnimation = true);

    virtual QGraphicsItem *getMouseClickReceiver();
    virtual QGraphicsItem *getMouseClickReceiver2();

    QList<CardItem *> removeCardItems(const QList<int> &card_ids, Player::Place place);
    virtual QList<CardItem *> cloneCardItems(QList<int> card_ids);

    // pending operations
    void startPending(const ViewAsSkill *skill);
    void stopPending();
    void updatePending();
    const ViewAsSkill *currentSkill() const;
    const Card *getPendingCard() const;

    void expandPileCards(const QString &pile_name);
    void retractPileCards(const QString &pile_name);

    void selectCard(CardItem *item, bool isSelected);

    int getButtonWidgetWidth() const;
    int getTextureWidth() const;

    int getWidth();
    int height();

    void showNullificationButton();
    void hideNullificationButton();

    static const int S_PENDING_OFFSET_Y = -25;

    inline void updateSkillButton() {
        if (rightSkillDock)
            rightSkillDock->update();
        if (leftSkillDock)
            leftSkillDock->update();
    }

    void setPlayer(ClientPlayer *player);

    void showSeat();

    inline QRectF getAvatarAreaSceneBoundingRect() const {
        return rightFrame->sceneBoundingRect();
    }

    inline void addPending(CardItem *item)
    {
        pendings << item;
    }

    inline QList<CardItem *> getPendings() const {
        return pendings;
    }

    void clearPendings();

    inline bool hasHandCard(CardItem *item) const {
        return m_handCards.contains(item);
    }

    void addTransferButton(TransferButton *button);
    QList<TransferButton *> getTransferButtons() const;

public slots:
    void sortCards();
    void beginSorting();
    void reverseSelection();
    void cancelNullification();
    void skillButtonActivated();
    void skillButtonDeactivated();
    void selectAll();
    void selectCards(const QString &pattern);
    void controlNullificationButton();

    virtual void updateAvatar();
    virtual void updateSmallAvatar();

protected:
    void _createExtraButtons();
    virtual void _adjustComponentZValues();
    virtual void addHandCards(QList<CardItem *> &cards);
    virtual QList<CardItem *> removeHandCards(const QList<int> &cardIds);

    // initialization of _m_layout is compulsory for children classes.
    inline virtual QGraphicsItem *_getEquipParent() { return leftFrame; }
    inline virtual QGraphicsItem *_getDelayedTrickParent() { return leftFrame; }
    inline virtual QGraphicsItem *_getAvatarParent() { return rightFrame; }
    inline virtual QGraphicsItem *_getMarkParent() { return _m_floatingArea; }
    inline virtual QGraphicsItem *_getPhaseParent() { return _m_floatingArea; }
    inline virtual QGraphicsItem *_getRoleComboBoxParent() { return rightFrame; }
    inline virtual QGraphicsItem *_getPileParent() { return rightFrame; }
    inline virtual QGraphicsItem *_getProgressBarParent() { return _m_floatingArea; }
    inline virtual QGraphicsItem *_getFocusFrameParent() { return rightFrame; }
    inline virtual QGraphicsItem *_getDeathIconParent() { return middleFrame; }
    inline virtual QString getResourceKeyName() { return QSanRoomSkin::S_SKIN_KEY_DASHBOARD; }
    inline virtual QAbstractAnimation *_getPlayerRemovedEffect() { return _removedEffect; }

    void _createRoleComboBox();

    bool _addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void _addHandCard(CardItem *card_item, bool prepend = false, const QString &footnote = QString());
    void _adjustCards();
    void _adjustCards(const QList<CardItem *> &list, int y);

    int width;
    // sync objects
    QMutex m_mutex;
    QMutex m_mutexEnableCards;

    QSanButton *m_trustButton;
    QSanButton *m_btnReverseSelection;
    QSanButton *m_btnSortHandcard;
    QSanButton *m_btnNoNullification;
    QGraphicsPixmapItem *leftFrame, *middleFrame, *rightFrame;
    // we can not draw bg directly _m_rightFrame because then it will always be
    // under avatar (since it's avatar's parent).
    QGraphicsPixmapItem *rightFrameBase, *rightFrameBg, *magatamasBase,
        *headGeneralFrame, *deputyGeneralFrame;
    QGraphicsItem *buttonWidget;

    CardItem *selected;
    QList<CardItem *> m_handCards;

    QGraphicsRectItem *trusting_item;
    QGraphicsSimpleTextItem *trusting_text;

    QSanInvokeSkillDock *rightSkillDock, *leftSkillDock;
    const QSanRoomSkin::DashboardLayout *layout;

    //for avatar shadow layer
    QGraphicsRectItem *_m_shadow_layer1, *_m_shadow_layer2;

    QGraphicsPixmapItem *leftHiddenMark, *rightHiddenMark;

    QGraphicsPixmapItem *headIcon, *deputyIcon;

    // for parts creation
    void _createLeft();
    void _createRight();
    void _createMiddle();
    void _updateFrames();

    // for pendings
    QList<CardItem *> pendings;
    const Card *pendingCard;
    const ViewAsSkill *viewAsSkill;
    const FilterSkill *filter;
    QStringList _m_pile_expanded;

    // for transfer
    QList<TransferButton *> _transferButtons;

    // for equip skill/selections
    PixmapAnimation *_m_equipBorders[5];
    QSanSkillButton *_m_equipSkillBtns[5];
    bool _m_isEquipsAnimOn[5];
    //QList<QSanSkillButton *> _m_button_recycle;

    void _createEquipBorderAnimations();
    void _setEquipBorderAnimation(int index, bool turnOn);

    void drawEquip(QPainter *painter, const CardItem *equip, int order);
    void setSelectedItem(CardItem *card_item);

    QMenu *_m_sort_menu;

    virtual void _initializeRemovedEffect();
    QPropertyAnimation *_removedEffect;

    QSanButton *m_changeHeadHeroSkinButton;
    QSanButton *m_changeDeputyHeroSkinButton;
    HeroSkinContainer *m_headHeroSkinContainer;
    HeroSkinContainer *m_deputyHeroSkinContainer;

private:
    static const int CARDITEM_Z_DATA_KEY = 0413;

    void showHeroSkinListHelper(const General *general, HeroSkinContainer * &heroSkinContainer);

    QPointF getHeroSkinContainerPosition() const;

protected slots:
    virtual void _onEquipSelectChanged();

private slots:
    void onCardItemClicked();
    void onCardItemDoubleClicked();
    void onCardItemThrown();
    void onMarkChanged();
    void onHeadStateChanged();
    void onDeputyStateChanged();
    void onHeadSkillPreshowed();
    void onDeputySkillPreshowed();
    void updateTrustButton();
    void bringSenderToTop();
    void resetSenderZValue();

    void showHeroSkinList();
    void heroSkinButtonMouseOutsideClicked();

    void onAvatarHoverEnter();
    void onAvatarHoverLeave();
    void onSkinChangingStart();
    void onSkinChangingFinished();

signals:
    void card_selected(const Card *card);
    void card_to_use();
    void progressBarTimedOut();
};

#endif

