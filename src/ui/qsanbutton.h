#ifndef _QSAN_BUTTON_H
#define _QSAN_BUTTON_H

#include <QGraphicsObject>
#include <QPixmap>
#include <QRegion>
#include <qrect.h>
#include <qlist.h>
#include "skill.h"

class QSanButton : public QGraphicsObject
{
    Q_OBJECT
public:
    QSanButton(QGraphicsItem* parent);
    QSanButton(const QString &groupName, const QString &buttonName, QGraphicsItem* parent);
    enum ButtonState { S_STATE_UP, S_STATE_HOVER, S_STATE_DOWN, 
                       S_STATE_DISABLED, S_NUM_BUTTON_STATES };
    enum ButtonStyle { S_STYLE_PUSH, S_STYLE_TOGGLE };
    void setSize(QSize size);
    void click();
    void setStyle(ButtonStyle style);
    void setState(ButtonState state);
    inline void setButtonName(QString buttonName) { _m_buttonName = buttonName; }
    inline QString getButtonName() { return _m_buttonName; }
    inline ButtonState getState() const { return _m_state; }
    inline ButtonStyle getStyle() const { return _m_style; }
    void setRect(QRect rect);
    virtual QRectF boundingRect() const;
    bool insideButton(QPointF pos) const;
    void setEnabled(bool enabled);
    bool isDown();

protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void _onMouseClick(bool inside);
    ButtonState _m_state;
    ButtonStyle _m_style;
    QString _m_groupName;
    QString _m_buttonName;
    QRegion _m_mask;
    QSize _m_size;
    // @todo: currently this is an extremely dirty hack. Refactor the button states to
    // get rid of it.
    bool _m_mouseEntered;
    QPixmap _m_bgPixmap[S_NUM_BUTTON_STATES];
signals:
    void clicked();
    void enable_changed();
};

class QSanSkillButton: public QSanButton
{
    Q_OBJECT
public:
    enum SkillType { S_SKILL_PROACTIVE, S_SKILL_FREQUENT, S_SKILL_COMPULSORY,
                     S_SKILL_AWAKEN, S_SKILL_ONEOFF_SPELL, S_NUM_SKILL_TYPES };
    inline static QString getSkillTypeString(SkillType type)
    {
        QString arg1;
        if (type == QSanSkillButton::S_SKILL_AWAKEN) arg1 = "awaken";
        else if (type == QSanSkillButton::S_SKILL_COMPULSORY) arg1 = "compulsory";
        else if (type == QSanSkillButton::S_SKILL_FREQUENT) arg1 = "frequent";
        else if (type == QSanSkillButton::S_SKILL_ONEOFF_SPELL) arg1 = "oneoff";
        else if (type == QSanSkillButton::S_SKILL_PROACTIVE) arg1 = "proactive";
        return arg1;
    }
    virtual void setSkill(const Skill* skill);   
    inline virtual const Skill* getSkill() const{ return _m_skill; }
    inline virtual void setEnabled(bool enabled) {
        if (!_m_canEnable && enabled) return;
        if (!_m_canDisable && !enabled) return;
        QSanButton::setEnabled(enabled);
    }
    QSanSkillButton(QGraphicsItem* parent = NULL);
    inline const ViewAsSkill* getViewAsSkill() const { return _m_viewAsSkill; }
protected:
    virtual void _setSkillType(SkillType type);
    virtual void _repaint() = 0;
    const ViewAsSkill* _parseViewAsSkill() const;
    SkillType _m_skillType;
    bool _m_emitActivateSignal;
    bool _m_emitDeactivateSignal;
    bool _m_canEnable;
    bool _m_canDisable; 
    const Skill* _m_skill;
    const ViewAsSkill* _m_viewAsSkill;
protected slots:
    void onMouseClick();
signals:
    void skill_activated(const Skill*);
    void skill_activated();
    void skill_deactivated(const Skill*);
    void skill_deactivated();
};

class QSanInvokeSkillButton: public QSanSkillButton
{
    Q_OBJECT
public:
    inline QSanInvokeSkillButton(QGraphicsItem* parent = NULL) : QSanSkillButton(parent)
    {
        _m_enumWidth = S_WIDTH_NARROW;
    }
    enum SkillButtonWidth { S_WIDTH_WIDE = 0, S_WIDTH_MED = 1, S_WIDTH_NARROW = 2, S_NUM_BUTTON_WIDTHS};    
    inline void setButtonWidth(SkillButtonWidth width) { _m_enumWidth = width; _repaint(); }
    inline SkillButtonWidth getButtonWidth() { return _m_enumWidth; }
protected:
    // this function does not update the button's bg and is therefore not exposed to outside
    // classes.    
    virtual void _repaint();
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    SkillButtonWidth _m_enumWidth;
    
    // QPixmap _m_skillTextPixmap[S_NUM_BUTTON_STATES];
};

class QSanInvokeSkillDock: public QGraphicsObject
{
    Q_OBJECT
public:
    QSanInvokeSkillDock(QGraphicsItem* parent) : QGraphicsObject(parent) {}
    int width() const;
    int height() const;
    void setWidth(int width);
    inline void addSkillButton(QSanInvokeSkillButton* button)
    {
        _m_buttons.push_back(button);
    }
    inline void removeSkillButton(QSanInvokeSkillButton* button)
    {
        if (button == NULL) return;
        disconnect(button);
        _m_buttons.removeAll(button);
    }
    // Any one who call the following functions are responsible for
    // destroying the buttons returned
    QSanSkillButton* addSkillButtonByName(const QString &skillName);
    inline QSanSkillButton* removeSkillButtonByName(const QString &skillName)
    {
        QSanInvokeSkillButton* button = getSkillButtonByName(skillName);
        if (button != NULL) removeSkillButton(button);
        update();
        return button;
    }
    QSanInvokeSkillButton* getSkillButtonByName(const QString &skillName) const;
    void update();    
    virtual QRectF boundingRect() const {return QRectF(0, -height(), width(), height());}
protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {}    
    QList<QSanInvokeSkillButton*> _m_buttons;
    int _m_width;    
signals:
    void skill_activated(const Skill* skill);
    void skill_deactivated(const Skill* skill);
};

#endif
