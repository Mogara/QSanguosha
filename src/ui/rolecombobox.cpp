#include "rolecombobox.h"
#include "photo.h"
#include "engine.h"

#include <QGraphicsScene>

RoleComboboxItem::RoleComboboxItem(const QString &role, int number, QSize size)
    :role(role)
{
    if(number != 0 )
        load(QString("image/system/roles/%1-%2.png").arg(role).arg(number), size, false);
    else
        load(QString("image/system/roles/%1.png").arg(role), size, false);
    this->setFlag(QGraphicsItem::ItemIgnoresParentOpacity);
}

QString RoleComboboxItem::getRole() const{
    return role;
}

void RoleComboboxItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    emit clicked();
}

RoleCombobox::RoleCombobox(Photo *photo):QObject(photo)
{
    int index = Sanguosha->getRoleIndex();
    QSize size(S_ROLE_COMBO_BOX_WIDTH, S_ROLE_COMBO_BOX_HEIGHT);
    items << new RoleComboboxItem("unknown", 0, size)
          << new RoleComboboxItem("loyalist", index, size)
          << new RoleComboboxItem("rebel", index, size)
          << new RoleComboboxItem("renegade", index, size);

    setPos(0, 0);
    _m_expanded = false;
    foreach(RoleComboboxItem *item, items){
        item->setParentItem(photo);
        item->hide();
        connect(item, SIGNAL(clicked()), this, SLOT(onItemClicked()));
    }

    items.first()->show();
}

void RoleCombobox::setPos(QPointF point)
{
    setPos(point.x(), point.y());
}

void RoleCombobox::setPos(qreal x, qreal y)
{
    _m_posX = x; _m_posY = y;
    for(int i = 0; i<items.length(); i++){
        RoleComboboxItem *item = items.at(i);
        item->setPos(x, y + i * (S_ROLE_COMBO_BOX_HEIGHT + S_ROLE_COMBO_BOX_GAP));
        item->setZValue(1.0);
    }
}

void RoleCombobox::hide(){
    foreach (QGraphicsItem *item, items)
        item->hide();
}

void RoleCombobox::show(){
    foreach(QGraphicsItem *item, items)
        item->show();
}

void RoleCombobox::onItemClicked(){
    if (items.length() < 2) return;

    RoleComboboxItem *clicked_item = qobject_cast<RoleComboboxItem *>(sender());

    if(_m_expanded){
        int i = 0;
        foreach (RoleComboboxItem *item, items){
            if  (item == clicked_item) continue;
            else i++;
            item->setPos(_m_posX, _m_posY + i * (S_ROLE_COMBO_BOX_HEIGHT + S_ROLE_COMBO_BOX_GAP));
            item->setZValue(1.0);
            item->hide();
        }
        clicked_item->setPos(_m_posX, _m_posY);
        clicked_item->show();
    }
    else{
        foreach(RoleComboboxItem *item, items)
            item->show();
    }
    _m_expanded = !_m_expanded;
}

void RoleCombobox::fix(const QString &role){
    // create the only one
    QPointF first_pos = items.first()->pos();
    QSize size(S_ROLE_COMBO_BOX_WIDTH, S_ROLE_COMBO_BOX_HEIGHT);
    RoleComboboxItem *fixed = new RoleComboboxItem(role, Sanguosha->getRoleIndex(), size);
    fixed->setPos(first_pos);
    fixed->show();
    fixed->setEnabled(false);
    fixed->setParentItem(qobject_cast<QGraphicsObject *>(parent()));

    // delete all
    foreach(RoleComboboxItem *item, items)
        delete item;
    items.clear();
    items << fixed;
}
