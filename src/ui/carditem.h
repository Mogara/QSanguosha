#ifndef _CARDITEM_H
#define _CARDITEM_H

#include "card.h"
#include "QSanSelectableItem.h"
#include "settings.h"
#include <QAbstractAnimation>
#include <QMutex>
#include <QSize>
#include "SkinBank.h"

class FilterSkill;
class General;

class CardItem : public QSanSelectableItem
{
    Q_OBJECT

public:

    CardItem(const Card *card);
    CardItem(const QString &general_name);
    ~CardItem();

    virtual QRectF boundingRect() const;
    virtual void setEnabled(bool enabled);   

    const Card *getCard() const;
    void setCard(const Card* card);
    inline int getId() const {
        return m_cardId;
    }

    // For move card animation
    void setHomePos(QPointF home_pos);
    QPointF homePos() const;    
    QAbstractAnimation* getGoBackAnimation(bool doFadeEffect, bool smoothTransition = false);
    void goBack(bool playAnimation, bool doFade = true);
    inline QAbstractAnimation* getCurrentAnimation(bool doFade) { return m_currentAnimation; }
    inline void setHomeOpacity(double opacity) { m_opacityAtHome = opacity; }
    inline double getHomeOpacity() { return m_opacityAtHome; }

    void showFrame(const QString &frame);
    void hideFrame();
    void showAvatar(const General *general);
    void hideAvatar();
    void setAutoBack(bool auto_back);
    void changeGeneral(const QString &general_name);
    void setFootnote(const QString &desc);

    bool isSelected() const { return m_isSelected; }
    inline void setSelected(bool selected) { m_isSelected = selected; }
    bool isEquipped() const;

    void setFrozen(bool is_frozen);
    bool isFrozen() const;

    inline void showFootnote() { _m_showFootnote = false; }
    inline void hideFootnote() { _m_showFootnote = true; }

    static CardItem *FindItem(const QList<CardItem *> &items, int card_id);
    
protected:
    void _initialize();
    QAbstractAnimation* m_currentAnimation;
    QImage _m_footnoteImage;
    bool _m_showFootnote;
    // QGraphicsPixmapItem* _m_footnoteItem;
    QMutex m_animationMutex;
    double m_opacityAtHome;
    bool m_isSelected;
    bool _m_isUnknownGeneral;
    static const int _S_CLICK_JITTER_TOLERANCE;
    static const int _S_MOVE_JITTER_TOLERANCE;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);    
    
private:    
    int m_cardId;
    QString _m_frameType, _m_avatarName;
    QPointF home_pos;
    QPointF _m_lastMousePressScenePos;
    bool auto_back, frozen;
signals:
    void toggle_discards();
    void clicked();
    void double_clicked();
    void thrown();
    void released();
    void enter_hover();
    void leave_hover();
    void movement_animation_finished();
};

#endif // CARDITEM_H
