#include "rolecombobox.h"
#include "photo.h"
#include "engine.h"

#include <QGraphicsScene>

RoleComboboxItem::RoleComboboxItem(const QString &role, int number)
    :role(role)
{
    if(number != 0 )
        changePixmap(QString("image/system/roles/%1-%2.png").arg(role).arg(number));
    else
        changePixmap(QString("image/system/roles/%1.png").arg(role));
}

QString RoleComboboxItem::getRole() const{
    return role;
}

void RoleComboboxItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    emit clicked();
}

RoleCombobox::RoleCombobox(Photo *photo)
    :QObject(photo)
{
    int index = Sanguosha->getRoleIndex();
    items << new RoleComboboxItem("unknown", 0)
            << new RoleComboboxItem("loyalist", index)
            << new RoleComboboxItem("rebel", index)
            << new RoleComboboxItem("renegade", index);

    setupItems();

    foreach(RoleComboboxItem *item, items){
        item->setParentItem(photo);
        item->hide();

        connect(item, SIGNAL(clicked()), this, SLOT(onItemClicked()));
    }

    items.first()->show();
}

void RoleCombobox::setupItems(){
    qreal height = items.first()->boundingRect().height();
    int i;
    for(i=0; i<items.length(); i++){
        qreal x = 85;
        qreal y = 15 + i*height;

        RoleComboboxItem *item = items.at(i);
        item->setPos(x, y);
    }
}

void RoleCombobox::hide(){
    foreach(QGraphicsItem *item, items)
        item->hide();
}

void RoleCombobox::show(){
    foreach(QGraphicsItem *item, items)
        item->show();
}

void RoleCombobox::onItemClicked(){
    if(items.length() < 2)
        return;

    bool expand = items.at(1)->isVisible();
    RoleComboboxItem *item = qobject_cast<RoleComboboxItem *>(sender());

    if(expand){
        QPointF a = items.first()->pos();
        QPointF b = item->pos();

        item->setPos(a);
        items.first()->setPos(b);

        int i = items.indexOf(item);
        items.swap(i, 0);

        for(i=1; i<items.length(); i++)
            items.at(i)->hide();
    }else{
        foreach(RoleComboboxItem *item, items)
            item->show();
    }
}

void RoleCombobox::fix(const QString &role){
    // create the only one
    QPointF first_pos = items.first()->pos();
    RoleComboboxItem *fixed = new RoleComboboxItem(role, Sanguosha->getRoleIndex());
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
