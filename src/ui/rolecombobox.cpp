#include "rolecombobox.h"
#include "SkinBank.h"
#include "roomscene.h"

void RoleComboBox::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (!fixed_role.isEmpty()) return;
    QPoint point = QPoint(event->pos().x(), event->pos().y());;
    if (expanding && !boundingRect().contains(point)) {
        expanding = false;
        update();
        return;
    } else if (!expanding) {
        expanding = true;
        update();
        return;
    }
    if (G_COMMON_LAYOUT.ROLE_WEI_RECT.contains(point))
        wei_excluded = !wei_excluded;
    else if (G_COMMON_LAYOUT.ROLE_QUN_RECT.contains(point))
        qun_excluded = !qun_excluded;
    else if (G_COMMON_LAYOUT.ROLE_SHU_RECT.contains(point))
        shu_excluded = !shu_excluded;
    else if (G_COMMON_LAYOUT.ROLE_WU_RECT.contains(point))
        wu_excluded = !wu_excluded;
    update();
}

void RoleComboBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    /*
      --------------------
      --------------------
      ||       ||       ||
      ||  WEI  ||  QUN  ||
      ||       ||       ||
      --------------------
      --------------------
      ||       ||       ||
      ||  SHU  ||  WU   ||
      ||       ||       ||
      --------------------
      --------------------
    */
    if (!fixed_role.isEmpty()) {
        QPixmap pix;
        pix.load(QString("image/system/roles/%1.png").arg(fixed_role));
        painter->drawPixmap(0, 0, pix);
        update();
        return;
    }
    if (!expanding) {
        if (circle) {
            QPixmap pix;
            pix.load("image/system/roles/unknown.png");
            painter->drawPixmap(0, 0, pix);
        } else {
            QColor grey = G_COMMON_LAYOUT.ROLE_DARK_COLOR;
            QPen pen(Qt::black);
            pen.setWidth(1);
            painter->setPen(pen);
            //paint wei
            painter->setBrush(QBrush(wei_excluded ? grey : G_COMMON_LAYOUT.ROLE_WEI_COLOR));
            painter->drawRect(COMPACT_BORDER_WIDTH, COMPACT_BORDER_WIDTH, COMPACT_ITEM_LENGTH, COMPACT_ITEM_LENGTH);
            //paint qun
            painter->setBrush(QBrush(qun_excluded ? grey : G_COMMON_LAYOUT.ROLE_QUN_COLOR));
            painter->drawRect(COMPACT_BORDER_WIDTH * 2 + COMPACT_ITEM_LENGTH, COMPACT_BORDER_WIDTH, COMPACT_ITEM_LENGTH, COMPACT_ITEM_LENGTH);
            //paint shu
            painter->setBrush(QBrush(shu_excluded ? grey : G_COMMON_LAYOUT.ROLE_SHU_COLOR));
            painter->drawRect(COMPACT_BORDER_WIDTH, COMPACT_BORDER_WIDTH * 2 + COMPACT_ITEM_LENGTH, COMPACT_ITEM_LENGTH, COMPACT_ITEM_LENGTH);
            //paint wu
            painter->setBrush(QBrush(wu_excluded ? grey : G_COMMON_LAYOUT.ROLE_WU_COLOR));
            painter->drawRect(COMPACT_BORDER_WIDTH * 2 + COMPACT_ITEM_LENGTH, COMPACT_BORDER_WIDTH * 2 + COMPACT_ITEM_LENGTH, COMPACT_ITEM_LENGTH, COMPACT_ITEM_LENGTH);
        }
    } else {
        QPixmap pix = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_EXPANDING_ROLE_BOX);
        painter->drawPixmap(0, 0, pix);

        if (wei_excluded)
            painter->drawPixmap(G_COMMON_LAYOUT.ROLE_WEI_RECT, G_ROOM_SKIN.getPixmap (QSanRoomSkin::S_SKIN_KEY_ROLE_BOX_KINGDOM_MASK, "wei"));
        if (qun_excluded)
            painter->drawPixmap(G_COMMON_LAYOUT.ROLE_QUN_RECT, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_ROLE_BOX_KINGDOM_MASK, "qun"));
        if (shu_excluded)
            painter->drawPixmap(G_COMMON_LAYOUT.ROLE_SHU_RECT, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_ROLE_BOX_KINGDOM_MASK, "shu"));
        if (wu_excluded)
            painter->drawPixmap(G_COMMON_LAYOUT.ROLE_WU_RECT, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_ROLE_BOX_KINGDOM_MASK, "wu"));
    }
}

QRectF RoleComboBox::boundingRect() const {
    QRect rect = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_EXPANDING_ROLE_BOX).rect();
    return QRectF(rect.x(), rect.y(), rect.width(), rect.height());
}

RoleComboBox::RoleComboBox(QGraphicsItem *photo, bool circle)
    : QGraphicsObject(photo), circle(circle), expanding(false), wei_excluded(false), qun_excluded(false), shu_excluded(false), wu_excluded(false)
{
    connect(RoomSceneInstance, SIGNAL(cancel_role_box_expanding()), this, SLOT(mouseClickedOutside()));
    setAcceptedMouseButtons(Qt::LeftButton);
}

void RoleComboBox::fix(const QString &role) {
    if (role == "god") return;
    fixed_role = role;
    update();
}

void RoleComboBox::mouseClickedOutside() {
    if (!expanding) return;
    expanding = false;
    update();
}