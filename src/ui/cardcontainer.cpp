#include "cardcontainer.h"
#include "clientplayer.h"
#include "carditem.h"
#include "engine.h"
#include "client.h"
#include "roomscene.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

CardContainer::CardContainer()
    : _m_background_top(G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CARD_CONTAINER_TOP)),
      _m_background_middle(G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CARD_CONTAINER_MIDDLE)),
      _m_background_bottom(G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CARD_CONTAINER_BOTTOM))
{
    _m_background = NULL;
    _m_background_seat = NULL;
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
    close_button = new CloseButton;
    close_button->setParentItem(this);
    close_button->hide();
    connect(close_button, SIGNAL(clicked()), this, SLOT(clear()));
}

void CardContainer::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    
}

QRectF CardContainer::boundingRect() const{
    return _m_boundingRect;
}

void CardContainer::_repaint() {
    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    const int blank = 3;
    int width = (card_width + blank) * items.length() - blank + 50;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * (items.length() + 1) / 2 - blank + 50;
        one_row = false;
    }
    int height = (one_row ? 1 : 2) * card_height + 70 + (one_row ? 0 : blank);
    int middle_height = qMax(height - _m_background_top.height() - _m_background_bottom.height(), 0);

    _m_boundingRect = QRectF(0, 0, width, _m_background_top.height() + middle_height + _m_background_bottom.height());

    QPixmap pix(width, _m_background_top.height() + middle_height + _m_background_bottom.height());
    pix.fill(QColor(0, 0, 0, 0));
    QPainter painter(&pix);
    painter.drawPixmap(0, 0, width, _m_background_top.height(), _m_background_top);
    painter.drawPixmap(0, _m_background_top.height(), width, middle_height, _m_background_middle);
    painter.drawPixmap(0, _m_background_top.height() + middle_height, width, _m_background_bottom.height(), _m_background_bottom);
    _m_background = new QGraphicsPixmapItem(this);
    _m_background->setTransformationMode(Qt::SmoothTransformation);
    _m_background->setPixmap(pix);
    _m_background->setParentItem(this);

    int first_row = one_row ? items.length() : (items.length() + 1) / 2;

    _m_background_seat = new QGraphicsPixmapItem(this);
    _m_background_seat->setTransformationMode(Qt::SmoothTransformation);
    QPixmap pixmap(width, _m_background_top.height() + middle_height + _m_background_bottom.height());
    pixmap.fill(QColor(0, 0, 0, 0));
    QPainter painter2(&pixmap);

    for (int i = 0; i < items.length(); ++ i) {
        int x, y = 0;
        if (i < first_row) {
            x = 25 + (card_width + blank) * i;
            y = 45;
        } else {
            if (items.length() % 2 == 1)
                x = 25 + card_width / 2 + blank / 2 
                    + (card_width + blank) * (i - first_row);
            else
                x = 25 + (card_width + blank) * (i - first_row);
            y = 45 + card_height + blank;
        }
        painter2.drawPixmap(x, y, card_width, card_height, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CARD_CONTAINER_FRAME));
    }
    _m_background_seat->setPixmap(pixmap);
    _m_background_seat->setParentItem(this);
    _m_background_seat->setZValue(-999);
    _m_background->setZValue(-1000);
    close_button->setPos(width - 30, 0);
    update();
}

void CardContainer::fillCards(const QList<int> &card_ids, const QList<int> &disabled_ids) {
    QList<CardItem *> card_items;
    if (card_ids.isEmpty() && items.isEmpty())
        return;
    else if (card_ids.isEmpty() && !items.isEmpty()) {
        card_items = items;
        items.clear();
    } else if (!items.isEmpty()) {
        retained_stack.push(retained());
        items_stack.push(items);
        foreach (CardItem *item, items)
            item->hide();
        items.clear();
    }

    close_button->hide();
    if (card_items.isEmpty())
        card_items = _createCards(card_ids);

    items.append(card_items);
    _repaint();
    int n = items.length();

    const int blank = 3;
    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    int width = (card_width + blank) * items.length() - blank + 50;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * (items.length() + 1) / 2 - blank + 50;
        one_row = false;
    }
    int first_row = one_row ? items.length() : (items.length() + 1) / 2;

    for (int i = 0; i < n; i++) {
        QPointF pos;
        if (i < first_row) {
            pos.setX(25 + (card_width + blank) * i);
            pos.setY(45);
        } else {
            if (n % 2 == 1)
                pos.setX(25 + card_width / 2 + blank / 2 
                    + (card_width + blank) * (i - first_row));
            else
                pos.setX(25 + (card_width + blank) * (i - first_row));
            pos.setY(45 + card_height + blank);
        }
        CardItem *item = items[i];
        item->resetTransform();
        item->setPos(pos);
        item->setHomePos(pos);
        item->setOpacity(1.0);
        item->setHomeOpacity(1.0);
        item->setFlag(QGraphicsItem::ItemIsFocusable);
        if (disabled_ids.contains(item->getCard()->getEffectiveId())) item->setEnabled(false);
        item->show();
    }
}

bool CardContainer::_addCardItems(QList<CardItem *> &, const CardsMoveStruct &) {
    return true;
}

bool CardContainer::retained() {
    return close_button != NULL && close_button->isVisible();
}

void CardContainer::clear() {
    foreach (CardItem *item, items) {
        item->hide();
        delete item;
        item = NULL;
    }

    items.clear();
    if (!items_stack.isEmpty()) {
        items = items_stack.pop();
        bool retained = retained_stack.pop();
        fillCards();
        if (retained && close_button)
            close_button->show();
    } else {
        //reset pixmap
        if (_m_background)
            _m_background->setPixmap(QPixmap());
        if (_m_background_seat)
            _m_background_seat->setPixmap(QPixmap());
        close_button->hide();
        hide();
    }
}

void CardContainer::freezeCards(bool is_frozen) {
    foreach (CardItem *item, items)
        item->setFrozen(is_frozen);
}

QList<CardItem *> CardContainer::removeCardItems(const QList<int> &card_ids, Player::Place place) {
    QList<CardItem *> result;
    foreach (int card_id, card_ids) {
        CardItem *to_take = NULL;
        foreach (CardItem *item, items) {
            if (item->getCard()->getId() == card_id) {
                to_take = item;
                break;
            }
        }
        if (to_take == NULL) continue;

        to_take->setEnabled(false);

        CardItem *copy = new CardItem(to_take->getCard());
        copy->setPos(mapToScene(to_take->pos()));
        copy->setEnabled(false);
        result.append(copy);

        if (m_currentPlayer)
            to_take->showAvatar(m_currentPlayer->getGeneral());
    }
    return result;
}

int CardContainer::getFirstEnabled() const{
    foreach (CardItem *card, items) {
        if (card->isEnabled())
            return card->getCard()->getId();
    }
    return -1;
}

void CardContainer::startChoose() {
    close_button->hide();
    foreach (CardItem *item, items) {
        connect(item, SIGNAL(leave_hover()), this, SLOT(grabItem()));
        connect(item, SIGNAL(clicked()), this, SLOT(chooseItem()));
    }
}

void CardContainer::startGongxin(const QList<int> &enabled_ids) {
    if (enabled_ids.isEmpty()) return;
    foreach (CardItem *item, items) {
        const Card *card = item->getCard();
        if (card && enabled_ids.contains(card->getEffectiveId()))
            connect(item, SIGNAL(double_clicked()), this, SLOT(gongxinItem()));
        else
            item->setEnabled(false);
    }
}

void CardContainer::addCloseButton() {
    close_button->show();
}

void CardContainer::grabItem() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item && !collidesWithItem(card_item)) {
        card_item->disconnect(this);
        emit item_chosen(card_item->getCard()->getId());
    }
}

void CardContainer::chooseItem() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        card_item->disconnect(this);
        emit item_chosen(card_item->getCard()->getId());
    }
}

void CardContainer::gongxinItem() {
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if (card_item) {
        emit item_gongxined(card_item->getCard()->getId());
        clear();
    }
}

CloseButton::CloseButton()
    : QSanSelectableItem("image/system/close.png", false)
{
    setFlag(ItemIsFocusable);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void CloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    event->accept();
}

void CloseButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *) {
    emit clicked();
}

void CardContainer::view(const ClientPlayer *player) {
    QList<int> card_ids;
    QList<const Card *> cards = player->getHandcards();
    foreach (const Card *card, cards)
        card_ids << card->getEffectiveId();

    fillCards(card_ids);
}

GuanxingBox::GuanxingBox()
    : CardContainer()
{
}

void GuanxingBox::doGuanxing (const QList<int> &card_ids, bool up_only) {
    if (card_ids.isEmpty()) {
        clear();
        return;
    }

    this->up_only = up_only;
    up_items.clear();

    foreach (int card_id, card_ids) {
        CardItem *card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setAutoBack(false);
        card_item->setFlag(QGraphicsItem::ItemIsFocusable);
        connect(card_item, SIGNAL(released()), this, SLOT(adjust()));

        up_items << card_item;
        card_item->setParentItem(this);
    }

    _repaint();
    setPos(RoomSceneInstance->tableCenterPos() - QPointF(boundingRect().width() / 2, boundingRect().height() / 2));
    show();

    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    const int blank = 3;
    int width = (card_width + blank) * up_items.length() - blank + 50;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * (up_items.length() + 1) / 2 - blank + 50;
        one_row = false;
    }
    int first_row = one_row ? up_items.length() : (up_items.length() + 1) / 2;

    for (int i = 0; i < up_items.length(); i++) {
        CardItem *card_item = up_items.at(i);

        QPointF pos;
        if (i < first_row) {
            pos.setX(25 + (card_width + blank) * i);
            pos.setY(45);
        } else {
            if (up_items.length() % 2 == 1)
                pos.setX(25 + card_width / 2 + blank / 2 
                    + (card_width + blank) * (i - first_row));
            else
                pos.setX(25 + (card_width + blank) * (i - first_row));
            pos.setY(45 + card_height + blank);
        }

        card_item->resetTransform();
        card_item->setPos(25, 45);
        card_item->setHomePos(pos);
        card_item->goBack(true);
    }
}

void GuanxingBox::adjust() {
    CardItem *item = qobject_cast<CardItem *>(sender());
    if (item == NULL) return;

    const int item_count = up_items.length() + down_items.length();

    const int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int blank = 3;
    int width = (card_width + blank) * item_count - blank + 50;
    bool one_row = true;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * (item_count + 1) / 2 - blank + 50;
        one_row = false;
    }
    const int first_row = one_row ? item_count : (item_count + 1) / 2;

    up_items.removeOne(item);
    down_items.removeOne(item);

    const int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    const int middle_y = 45 + (one_row ? card_height : (card_height * 2 + blank));

    QList<CardItem *> *items = (up_only || item->y() + card_height / 2 <= middle_y) ? &up_items : items = &down_items;
    bool odd_row = true;
    if (!one_row && item_count % 2) {
        const qreal y = item->y() + card_height / 2;
        if (y >= 45 + card_height && y <= 45 + card_height * 2 + blank || y >= 45 + card_height * 3 + blank * 3) odd_row = false;
    }
    const int start_x = 25 + (odd_row ? 0 : (card_width / 2 + blank / 2));
    int c = (item->x() + item->boundingRect().width() / 2 - start_x) / card_width;
    c = qBound(0, c, items->length());
    items->insert(c, item);

    for (int i = 0; i < up_items.length(); i++) {
        QPointF pos;
        if (i < first_row) {
            pos.setX(25 + (card_width + blank) * i);
            pos.setY(45);
        } else {
            if (item_count % 2 == 1)
                pos.setX(25 + card_width / 2 + blank / 2 
                    + (card_width + blank) * (i - first_row));
            else
                pos.setX(25 + (card_width + blank) * (i - first_row));
            pos.setY(45 + card_height + blank);
        }
        up_items.at(i)->setHomePos(pos);
        up_items.at(i)->goBack(true);
    }

    for (int i = 0; i < down_items.length(); i++) {
        QPointF pos;
        if (i < first_row) {
            pos.setX(25 + (card_width + blank) * i);
            pos.setY(45 + (card_height + blank) * (one_row ? 1 : 2));
        } else {
            if (item_count % 2 == 1)
                pos.setX(25 + card_width / 2 + blank / 2 
                    + (card_width + blank) * (i - first_row));
            else
                pos.setX(25 + (card_width + blank) * (i - first_row));
            pos.setY(45 + card_height * 3 + blank * 3);
        }
        down_items.at(i)->setHomePos(pos);
        down_items.at(i)->goBack(true);
    }
}

void GuanxingBox::clear() {
    foreach (CardItem *card_item, up_items)
        card_item->deleteLater();
    foreach (CardItem *card_item, down_items)
        card_item->deleteLater();

    up_items.clear();
    down_items.clear();

    //reset pixmap
    if (_m_background)
        _m_background->setPixmap(QPixmap());
    if (_m_background_seat)
        _m_background_seat->setPixmap(QPixmap());

    hide();
}

void GuanxingBox::reply() {
    QList<int> up_cards, down_cards;
    foreach (CardItem *card_item, up_items)
        up_cards << card_item->getCard()->getId();

    foreach (CardItem *card_item, down_items)
        down_cards << card_item->getCard()->getId();

    ClientInstance->onPlayerReplyGuanxing(up_cards, down_cards);
    clear();
}

void GuanxingBox::_repaint() {
    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    bool one_row = true;
    const int blank = 3;
    int width = (card_width + blank) * up_items.length() - blank + 50;
    if (width * 1.5 > RoomSceneInstance->sceneRect().width()) {
        width = (card_width + blank) * (up_items.length() + 1) / 2 - blank + 50;
        one_row = false;
    }
    int height = (one_row ? 1 : 2) * card_height + (one_row ? 0 : blank);
    if (!up_only) height = height * 2 + blank;
    height += 70;
    int middle_height = qMax(height - _m_background_top.height() - _m_background_bottom.height(), 0);

    _m_boundingRect = QRectF(0, 0, width, _m_background_top.height() + middle_height + _m_background_bottom.height());

    QPixmap pix(width, _m_background_top.height() + middle_height + _m_background_bottom.height());
    pix.fill(QColor(0, 0, 0, 0));
    QPainter painter(&pix);
    painter.drawPixmap(0, 0, width, _m_background_top.height(), _m_background_top);
    painter.drawPixmap(0, _m_background_top.height(), width, middle_height, _m_background_middle);
    painter.drawPixmap(0, _m_background_top.height() + middle_height, width, _m_background_bottom.height(), _m_background_bottom);
    _m_background = new QGraphicsPixmapItem(this);
    _m_background->setTransformationMode(Qt::SmoothTransformation);
    _m_background->setPixmap(pix);
    _m_background->setParentItem(this);

    int first_row = one_row ? up_items.length() : (up_items.length() + 1) / 2;

    _m_background_seat = new QGraphicsPixmapItem(this);
    _m_background_seat->setTransformationMode(Qt::SmoothTransformation);
    QPixmap pixmap(width, _m_background_top.height() + middle_height + _m_background_bottom.height());
    pixmap.fill(QColor(0, 0, 0, 0));
    QPainter painter2(&pixmap);

    for (int i = 0; i < up_items.length(); ++ i) {
        int x, y = 0;
        if (i < first_row) {
            x = 25 + (card_width + blank) * i;
            y = 45;
        } else {
            if (up_items.length() % 2 == 1)
                x = 25 + card_width / 2 + blank / 2 
                    + (card_width + blank) * (i - first_row);
            else
                x = 25 + (card_width + blank) * (i - first_row);
            y = 45 + card_height + blank;
        }
        painter2.drawPixmap(x, y, card_width, card_height, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CARD_CONTAINER_FRAME));
        if (!up_only)
            painter2.drawPixmap(x, y + (card_height + blank) * (one_row ? 1 : 2), card_width, card_height, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CARD_CONTAINER_FRAME));
    }
    _m_background_seat->setPixmap(pixmap);
    _m_background_seat->setParentItem(this);
    _m_background_seat->setZValue(-999);
    _m_background->setZValue(-1000);
    close_button->setPos(width - 30, 0);
    update();
}

