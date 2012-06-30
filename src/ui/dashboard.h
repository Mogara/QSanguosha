#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "QSanSelectableItem.h"
#include "qsanbutton.h"
#include "carditem.h"
#include "player.h"
#include "skill.h"
#include "sprite.h"
#include "protocol.h"
#include "TimedProgressBar.h"
#include "GeneralCardContainerUI.h"

#include <QPushButton>
#include <QComboBox>
#include <QGraphicsLinearLayout>
#include <QLineEdit>
#include <QProgressBar>
#include <QMutex>

class Dashboard : public PlayerCardContainer
{
    Q_OBJECT
public:
    Dashboard(QGraphicsItem *button_widget);
    virtual QRectF boundingRect() const;
    void setWidth(int width);
    int getMiddleWidth();
    QGraphicsProxyWidget *addWidget(QWidget *widget, int x, bool from_left);
    // @TODO: the following function needs to be removed
    QPushButton *createButton(const QString &name);
    QPushButton *addButton(const QString &name, int x, bool from_left);
    
    void removeSkillButton(const QString &skillName);
    QSanSkillButton *addSkillButton(const QString &skillName);
    bool isAvatarUnderMouse();

    void setTrust(bool trust);    
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

    void adjustCards(bool playAnimation = true);
    
    virtual QGraphicsItem* getMouseClickReceiver();

    QList<CardItem*> removeCardItems(const QList<int> &card_ids, Player::Place place);

    // pending operations
    void startPending(const ViewAsSkill *skill);
    void stopPending();
    void updatePending();
    const ViewAsSkill *currentSkill() const;
    const Card *pendingCard() const;

    void selectCard(CardItem* item, bool isSelected);

    int getButtonWidgetWidth() const;
    int getTextureWidth() const;

    int width();
    int height();

    static const int S_PENDING_OFFSET_Y = -40;
public slots:
    void doFilter();
    void sortCards(bool doAnmiation = true);
    void reverseSelection();

protected:
    virtual void _adjustComponentZValues();
    virtual void addHandCards(QList<CardItem*> &cards);
    virtual QList<CardItem*> removeHandCards(const QList<int> &cardIds);

    // initialization of _m_layout is compulsory for children classes.
    inline virtual QGraphicsItem* _getEquipParent() { return _m_leftFrame; }
    inline virtual QGraphicsItem* _getDelayedTrickParent() { return _m_leftFrame; }
    inline virtual QGraphicsItem* _getAvatarParent() { return _m_rightFrame; }
    inline virtual QGraphicsItem* _getMarkParent() { return _m_floatingArea; }
    inline virtual QGraphicsItem* _getPhaseParent() { return _m_floatingArea; }
    inline virtual QGraphicsItem* _getRoleComboBoxParent() { return _m_rightFrame; }
    inline virtual QGraphicsItem* _getPileParent() { return _m_rightFrame; }
    inline virtual QGraphicsItem* _getProgressBarParent() { return _m_floatingArea; }
    inline virtual QGraphicsItem* _getFocusFrameParent() { return _m_rightFrame; }
    inline virtual QString getResourceKeyName() { return QSanRoomSkin::S_SKIN_KEY_DASHBOARD; }
    
    bool _addCardItems(QList<CardItem*> &card_items, Player::Place place);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void _addHandCard(CardItem* card_item);    
    void _adjustCards();
    void _adjustCards(const QList<CardItem *> &list, int y);
    // ui controls
    const static QRect S_EQUIP_CARD_MOVE_REGION;
    const static QRect S_JUDGE_CARD_MOVE_REGION;
    QRectF m_cardTakeOffRegion;
    QRectF m_cardSpecialRegion;

    int _m_width;

    // sync objects
    QMutex m_mutex;
    QMutex m_mutexEnableCards;

    QGraphicsPixmapItem *_m_leftFrame, *_m_middleFrame, *_m_rightFrame;    
    // we can not draw bg directly _m_rightFrame because then it will always be
    // under avatar (since it's avatar's parent).
    QGraphicsPixmapItem *_m_rightFrameBg;
    QGraphicsItem *button_widget;
        
    CardItem *selected;
    QList<CardItem*> m_handCards;

    QGraphicsRectItem *trusting_item;
    QGraphicsSimpleTextItem *trusting_text;

    QSanInvokeSkillDock* _m_skillDock;

    //for animated effects
    EffectAnimation *animations;

    // for parts creation
    void _createLeft();
    void _createRight();
    void _createMiddle();
    void _updateFrames();

    // for pendings
    QList<CardItem *> pendings;
    const Card *pending_card;
    const ViewAsSkill *view_as_skill;
    const FilterSkill *filter;
       
    void drawEquip(QPainter *painter, const CardItem *equip, int order);
    void setSelectedItem(CardItem *card_item);

private slots:
    void onCardItemClicked();
    void onCardItemThrown();
    void onCardItemHover();
    void onCardItemLeaveHover();
    void onMarkChanged();

signals:
    void card_selected(const Card *card);
    void card_to_use();
    void progressBarTimedOut();
};

#endif // DASHBOARD_H
