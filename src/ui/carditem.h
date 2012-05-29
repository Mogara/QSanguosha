#ifndef _CARDITEM_H
#define _CARDITEM_H

#include "card.h"
#include "pixmap.h"
#include "settings.h"
#include <QAbstractAnimation>
#include <QMutex>
#include <QSize>
#include "SkinBank.h"

class FilterSkill;
class General;

class CardItem : public Pixmap
{
    Q_OBJECT

public:
    CardItem(const Card *card);
    CardItem(const QString &general_name);
    ~CardItem();

    virtual void setEnabled(bool enabled);
    void filter(const FilterSkill *filter_skill);
    const Card *getFilteredCard() const;    

    const Card *getCard() const;
    void setCard(const Card* card);
    inline int getId() const {
        if (m_card == NULL) return Card::S_UNKNOWN_CARD_ID;
        else return m_card->getId();
    }

    // For move card animation
    void setHomePos(QPointF home_pos);
    QPointF homePos() const;    
    QAbstractAnimation* getGoBackAnimation(bool doFadeEffect);
    void goBack(bool playAnimation, bool doFade = true);
    inline QAbstractAnimation* getCurrentAnimation(bool doFade) { return m_currentAnimation; }
    inline void setHomeOpacity(double opacity) { m_opacityAtHome = opacity; }
    inline double getHomeOpacity() { return m_opacityAtHome; }

    const QPixmap &getSuitPixmap() const;
    const QPixmap &getNumberPixmap() const;
    const QPixmap &getIconPixmap() const;
    void setFrame(const QString &frame);
    void showAvatar(const General *general);
    void hideFrame();
    void setAutoBack(bool auto_back);
    void changeGeneral(const QString &general_name);
    void setFootnote(QString desc);

    bool isSelected() const { return m_isSelected; }
    inline void setSelected(bool selected) { m_isSelected = selected; }
    bool isEquipped() const;

    void setFrozen(bool is_frozen);
    bool isFrozen() const;

    inline void setFootnoteVisible(bool visible) { m_isDescriptionVisible = visible; }
    bool isFootnoteVisible() { return m_isDescriptionVisible; }

    static CardItem *FindItem(const QList<CardItem *> &items, int card_id);
    static const int S_NORMAL_CARD_WIDTH = 93.0;
    static const int S_NORMAL_CARD_HEIGHT = 130.0;

public slots:
    void reduceZ();
    void promoteZ();

protected:
    void _initialize();
    QAbstractAnimation* m_currentAnimation;
    QMutex m_animationMutex;
    double m_opacityAtHome;
    bool m_isSelected;
    bool m_isDescriptionVisible;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);    

private:    
    const Card *m_card, *filtered_card;
    QPixmap suit_pixmap, icon_pixmap, number_pixmap, cardsuit_pixmap;
    QString owner_text;
    QPointF home_pos;
    QGraphicsPixmapItem *frame, *avatar;
    const QSanRoomSkin* _m_roomSkin;
    const QSanRoomSkin::CommonLayout* _m_layout;
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
