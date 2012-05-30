#include "rolecombobox.h"
#include "photo.h"
#include "engine.h"

#include <QGraphicsScene>

RoleComboboxItem::RoleComboboxItem(const QString &role, int number, QSize size)
    :m_role(role), m_number(number), m_size(size)
{
    setRole(role);
    this->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
}

QString RoleComboboxItem::getRole() const{
    return m_role;
}

void RoleComboboxItem::setRole(const QString& role){
    m_role = role;
    if(m_number != 0 && role != "unknown")
        load(QString("image/system/roles/%1-%2.png").arg(m_role).arg(m_number), m_size, false);
    else
        load(QString("image/system/roles/%1.png").arg(m_role), m_size, false);
}

void RoleComboboxItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    emit clicked();
}

RoleCombobox::RoleCombobox(Photo *photo):QObject(photo)
{
    int index = Sanguosha->getRoleIndex();
    QSize size(S_ROLE_COMBO_BOX_WIDTH, S_ROLE_COMBO_BOX_HEIGHT);
    m_currentRole = new RoleComboboxItem("unknown", index, size);
    m_currentRole->setParentItem(photo);
    connect(m_currentRole, SIGNAL(clicked()), this, SLOT(expand()));
    items << new RoleComboboxItem("loyalist", index, size)
          << new RoleComboboxItem("rebel", index, size)
          << new RoleComboboxItem("renegade", index, size);
    foreach(RoleComboboxItem *item, items){
        item->setParentItem(photo);
        item->hide();
        connect(item, SIGNAL(clicked()), this, SLOT(collapse()));
    }
}

void RoleCombobox::setPos(QPointF point)
{
    setPos(point.x(), point.y());
}

void RoleCombobox::setPos(qreal x, qreal y)
{
    _m_posX = x; _m_posY = y;
    m_currentRole->setPos(x, y);
    for(int i = 0; i<items.length(); i++){
        RoleComboboxItem *item = items.at(i);
        item->setPos(x, y + (i + 1) * (S_ROLE_COMBO_BOX_HEIGHT + S_ROLE_COMBO_BOX_GAP));
        item->setZValue(1.0);
    }
    
    m_currentRole->setZValue(0.5);
}

void RoleCombobox::hide(){
    foreach (QGraphicsItem *item, items)
        item->hide();
    m_currentRole->hide();
}

void RoleCombobox::show(){
    foreach(QGraphicsItem *item, items)
        item->show();
    m_currentRole->show();
}

void RoleCombobox::collapse(){
    RoleComboboxItem *clicked_item = qobject_cast<RoleComboboxItem *>(sender());
    foreach (RoleComboboxItem *item, items) item->hide();
    m_currentRole->setRole(clicked_item->getRole());
}

void RoleCombobox::expand(){
    foreach(RoleComboboxItem *item, items)
        item->show();
    m_currentRole->setRole("unknown");
}

void RoleCombobox::fix(const QString &role){
    m_currentRole->setRole(role);
    disconnect(m_currentRole);
    // delete all
    foreach(RoleComboboxItem *item, items)
        delete item;
    items.clear();
}
