#include "choosegeneralbox.h"
#include "engine.h"
#include "roomscene.h"
#include "SkinBank.h"
#include "protocol.h"

GeneralCardItem::GeneralCardItem(const QString &general_name)
    : CardItem(general_name), has_companion(false)
{
    setAcceptHoverEvents(true);
}

void GeneralCardItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QRect rect = G_COMMON_LAYOUT.m_cardMainArea;
    if (!isEnabled()) {
        painter->fillRect(rect, QColor(0, 0, 0));
        painter->setOpacity(0.4 * opacity());
    }

    if (!_m_isUnknownGeneral)
        painter->drawPixmap(rect, G_ROOM_SKIN.getCardMainPixmap(objectName()));
    else
        painter->drawPixmap(rect, G_ROOM_SKIN.getPixmap("generalCardBack"));

    if (!has_companion) return;

    QString kingdom = Sanguosha->getGeneral(objectName())->getKingdom();
    QPixmap icon = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_GENERAL_CARD_ITEM_COMPANION_ICON, kingdom);
    painter->drawPixmap(boundingRect().center().x() - icon.width() / 2 + 3, boundingRect().bottom() - icon.height(), icon);

    painter->drawPixmap(G_COMMON_LAYOUT.m_generalCardItemCompanionPromptRegion, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_GENERAL_CARD_ITEM_COMPANION_FONT, kingdom));
}

void GeneralCardItem::showCompanion() {
    if (has_companion) return;
    has_companion = true;
    update();
}

void GeneralCardItem::hideCompanion() {
    if (!has_companion) return;
    has_companion = false;
    update();
}

ChooseGeneralBox::ChooseGeneralBox() 
    : general_number(0), single_result(false)
{
    setFlag(ItemIsFocusable);
    setFlag(ItemIsMovable);
    confirm = new QSanButton("choose-general-box", "confirm", this);
    confirm->setEnabled(ClientInstance->getReplayer());
    connect(confirm, SIGNAL(clicked()), this, SLOT(reply()));
    progress_bar = NULL;
    animations = new EffectAnimation;
}

void ChooseGeneralBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    //============================================================
    //||========================================================||
    //||                   请选择相同势力的武将                  ||
    //||       ______   ______   ______   ______   ______       ||
    //||      |      | |      | |      | |      | |      |      ||
    //||      |  g1  | |  g2  | |  g3  | |  g4  | |  g5  |      ||
    //||      |      | |      | |      | |      | |      |      ||
    //||       ——————   ——————   ——————   ——————   ——————       ||
    //||           ______   ______   ______   ______            ||
    //||          |      | |      | |      | |      |           ||
    //||          |  g6  | |  g7  | |  g8  | |  g9  |           ||
    //||          |      | |      | |      | |      |           ||
    //||           ——————   ——————   ——————   ——————            ||
    //||     ----------------------------------------------     ||                  
    //||                           \/                           ||
    //||                    ______   ______                     ||
    //||                   |      | |      |                    ||
    //||                   |  hg  | |  dg  |                    ||
    //||                   |      | |      |                    ||
    //||                    ——————   ——————                     ||
    //||                       __________                       ||
    //||                      |   确定   |                      ||
    //||                       ——————————                       ||
    //||               =========================                || 
    //||                                                        ||
    //============================================================
    //
    //
    //==================================================
    //||                                          |X| ||
    //||==============================================||
    //||               知己知彼观看主将                ||
    //||             __________________               ||
    //||            |                  |              ||
    //||            |                  |              ||
    //||            |                  |              ||
    //||            |                  |              ||
    //||            |                  |              ||
    //||            |                  |              || 
    //||            |                  |              ||
    //||            |                  |              ||
    //||            |                  |              ||
    //||            |                  |              ||
    //||             ——————————————————               ||
    //||                                              ||
    //==================================================
    painter->save();
    painter->setBrush(QBrush(G_COMMON_LAYOUT.m_chooseGeneralBoxBackgroundColor));
    QRectF rect = boundingRect();
    const int x = rect.x();
    const int y = rect.y();
    const int w = rect.width();
    const int h = rect.height();
    painter->drawRect(QRect(x, y, w, h));
    painter->drawRect(QRect(x, y, w, top_dark_bar));
    G_COMMON_LAYOUT.m_chooseGeneralBoxTitleFont.paintText(painter, QRect(x, y, w, top_dark_bar), Qt::AlignCenter, single_result ? tr("Please select one general") : tr("Please select the same nationality generals"));
    painter->restore();
    painter->setPen(G_COMMON_LAYOUT.m_chooseGeneralBoxBorderColor);
    painter->drawRect(QRect(x + 1, y + 1, w - 2, h - 2));

    if (single_result) return;

    int split_line_y = top_blank_width + G_COMMON_LAYOUT.m_cardNormalHeight + card_bottom_to_split_line;
    if (general_number > 5)
        split_line_y += (card_to_center_line + G_COMMON_LAYOUT.m_cardNormalHeight);
    QPixmap line = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CHOOSE_GENERAL_BOX_SPLIT_LINE);
    const int line_length = boundingRect().width() - 2 * left_blank_width;
    painter->drawPixmap(left_blank_width, split_line_y, line, (line.width() - line_length) / 2, y, line_length, line.height());

    QPixmap seat = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_CHOOSE_GENERAL_BOX_DEST_SEAT);
    QRect seat1_rect(rect.center().x() - G_COMMON_LAYOUT.m_cardNormalWidth - card_to_center_line - 2, split_line_y + split_line_to_card_seat - 2, G_COMMON_LAYOUT.m_cardNormalWidth + 4, G_COMMON_LAYOUT.m_cardNormalHeight + 4);
    painter->drawPixmap(seat1_rect, seat);
    IQSanComponentSkin::QSanSimpleTextFont font = G_COMMON_LAYOUT.m_chooseGeneralBoxDestSeatFont;
    font.paintText(painter, seat1_rect, Qt::AlignCenter, tr("head_general"));
    QRect seat2_rect(rect.center().x() + card_to_center_line - 2, split_line_y + split_line_to_card_seat - 2, G_COMMON_LAYOUT.m_cardNormalWidth + 4, G_COMMON_LAYOUT.m_cardNormalHeight + 4);
    painter->drawPixmap(seat2_rect, seat);
    font.paintText(painter, seat2_rect, Qt::AlignCenter, tr("deputy_general"));
}

QRectF ChooseGeneralBox::boundingRect() const {
    //确定第一行和第二行容纳的武将数
    int first_row, second_row = 0;

    //6将及以上分成两行，第二行牌数不超过第一行，两行牌差不超过1
    if (general_number < 6)
        first_row = general_number;
    else {
        second_row = general_number / 2;
        first_row = general_number - second_row;
    }

    const int width = first_row * G_COMMON_LAYOUT.m_cardNormalWidth + (first_row - 1) * card_to_center_line + left_blank_width * 2;

    int height = top_blank_width + G_COMMON_LAYOUT.m_cardNormalHeight + bottom_blank_width;

    if (second_row)
        height += (card_to_center_line + G_COMMON_LAYOUT.m_cardNormalHeight);

    if (single_result) return QRectF(0, 0, width, height);
    
    height += G_COMMON_LAYOUT.m_cardNormalHeight + card_bottom_to_split_line + split_line_to_card_seat;

    return QRectF(0, 0, width, height);
}

void ChooseGeneralBox::chooseGeneral(QStringList generals) {
    //重新绘制背景
    if (generals.contains("anjiang(lord)")) generals.removeAll("anjiang(lord)");
    general_number = generals.length();
    update();

    items.clear();
    selected.clear();
    foreach(QString general, generals) {
        if (general.endsWith("(lord)")) continue;
        GeneralCardItem *general_item = new GeneralCardItem(general);
        general_item->setFlag(QGraphicsItem::ItemIsFocusable);
        general_item->setAcceptedMouseButtons(Qt::LeftButton);

        if (single_result)
            general_item->setFlag(QGraphicsItem::ItemIsMovable, false);
        else {
            general_item->setAutoBack(true);
            connect(general_item, SIGNAL(released()), this, SLOT(_adjust()));
        }

        connect(general_item, SIGNAL(clicked()), this, SLOT(_onItemClicked()));
        connect(general_item, SIGNAL(enter_hover()), this, SLOT(_onCardItemHover()));
        connect(general_item, SIGNAL(leave_hover()), this, SLOT(_onCardItemLeaveHover()));

        if (!single_result) {
            const General *hero = Sanguosha->getGeneral(general);
            foreach (QString other, generals) {
                if (other.endsWith("(lord)")) continue;
                if (general != other && hero->isCompanionWith(other)) {
                    general_item->showCompanion();
                    break;
                }
            }
        }

        items << general_item;
        general_item->setParentItem(this);
    }

    setPos(RoomSceneInstance->tableCenterPos() - QPointF(boundingRect().width() / 2, boundingRect().height() / 2));
    show();

    int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    int first_row = (general_number < 6) ? general_number : ((general_number + 1) / 2);

    for (int i = 0; i < items.length(); ++ i) {
        GeneralCardItem *card_item = items.at(i);

        QPointF pos;
        if (i < first_row) {
            pos.setX(left_blank_width + (card_width + card_to_center_line) * i + card_width / 2);
            pos.setY(top_blank_width + card_height / 2);
        } else {
            if (items.length() % 2 == 1)
                pos.setX(left_blank_width + card_width / 2 + card_to_center_line / 2 
                    + (card_width + card_to_center_line) * (i - first_row) + card_width / 2);
            else
                pos.setX(left_blank_width + (card_width + card_to_center_line) * (i - first_row) + card_width / 2);
            pos.setY(top_blank_width + card_height + card_to_center_line + card_height / 2);
        }

        card_item->setPos(25, 45);
        if (!single_result)
            //把我家庭住址存下来，防止回不来
            card_item->setData(S_DATA_INITIAL_HOME_POS, pos);
        card_item->setHomePos(pos);
        card_item->goBack(true);
    }

    if (single_result)
        confirm->hide();
    else {
        confirm->setPos(boundingRect().center().x() - confirm->boundingRect().width() / 2, boundingRect().height() - 60);
        confirm->show();
    }
    _initializeItems();

    if (ServerInfo.OperationTimeout != 0) {
        if (!progress_bar) {
	        progress_bar = new QSanCommandProgressBar();
	        progress_bar->setMinimumWidth(200);
	        progress_bar->setMaximumHeight(12);
	        progress_bar->setTimerEnabled(true);
	        progress_bar_item = new QGraphicsProxyWidget(this);
	        progress_bar_item->setWidget(progress_bar);
	        progress_bar_item->setPos(boundingRect().center().x() - progress_bar_item->boundingRect().width() / 2, boundingRect().height() - 30);
	        connect(progress_bar, SIGNAL(timedOut()), this, SLOT(reply()));
        }
        progress_bar->setCountdown(QSanProtocol::S_COMMAND_CHOOSE_GENERAL);
        progress_bar->show();
    }
}

void ChooseGeneralBox::_adjust() {
    GeneralCardItem *item = qobject_cast<GeneralCardItem *>(sender());
    if (item == NULL) return;

    const int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;

    const int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;
    int middle_y = top_blank_width + G_COMMON_LAYOUT.m_cardNormalHeight + card_bottom_to_split_line;
    if (general_number > 5)
        middle_y += (card_to_center_line + G_COMMON_LAYOUT.m_cardNormalHeight);

    if (selected.contains(item) && item->y() <= middle_y) {
        selected.removeOne(item);
        items << item;
        item->setHomePos(item->data(S_DATA_INITIAL_HOME_POS).toPointF());
        item->goBack(true);
        //孩纸，回去吧
    } else if (selected.length() == 2 && !Sanguosha->getGeneral(selected.first()->objectName())->isLord() && (selected.first() == item && item->x() > boundingRect().center().x() || selected.last() == item && item->x() < boundingRect().center().x()))
        qSwap(selected[0], selected[1]);
    else if (items.contains(item) && item->y() > middle_y) {
        if (selected.length() > 1) return;
        items.removeOne(item);
        selected << item;
    }

    if (!selected.isEmpty()) {
        int dest_seat_y = top_blank_width + G_COMMON_LAYOUT.m_cardNormalHeight + card_bottom_to_split_line + split_line_to_card_seat + card_height / 2;
        if (general_number > 5)
            dest_seat_y += (card_to_center_line + card_height);
        selected.first()->setHomePos(QPointF(boundingRect().center().x() - card_to_center_line - card_width / 2, dest_seat_y));
        selected.first()->goBack(true);
        if (selected.length() == 2) {
            selected.last()->setHomePos(QPointF(boundingRect().center().x() + card_to_center_line + card_width / 2, dest_seat_y));
            selected.last()->goBack(true);
        }
    }

    adjustItems();
}

void ChooseGeneralBox::adjustItems() {
    if (selected.length() == 2){
        foreach(GeneralCardItem *card, items)
            card->setEnabled(false);
        confirm->setEnabled(true);
    } else if (selected.length() == 1) {
        selected.first()->hideCompanion();
        foreach(GeneralCardItem *card, items) {
            const General *seleted_general = Sanguosha->getGeneral(selected.first()->objectName());
            const General *general = Sanguosha->getGeneral(card->objectName());
            if (general->getKingdom() != seleted_general->getKingdom() || general->isLord()) {
                if (card->isEnabled())
                    card->setEnabled(false);
                card->hideCompanion();
            } else {
                if (!card->isEnabled())
                    card->setEnabled(true);
                if (general->isCompanionWith(selected.first()->objectName())) {
                    selected.first()->showCompanion();
                    card->showCompanion();
                } else card->hideCompanion();
            }
        }
        if (confirm->isEnabled()) confirm->setEnabled(false);
    } else {
        _initializeItems();
        foreach (GeneralCardItem *card, items) {
            card->hideCompanion();
            foreach (GeneralCardItem *other, items) {
                if (other->objectName().endsWith("(lord)")) continue;
                const General *hero = Sanguosha->getGeneral(card->objectName());
                if (card != other && hero->isCompanionWith(other->objectName())) {
                    card->showCompanion();
                    break;
                }
            }
        }
        if (confirm->isEnabled()) confirm->setEnabled(false);
    }
}

void ChooseGeneralBox::_initializeItems() {
    QList<const General *> generals;
    foreach(GeneralCardItem *item, items)
        generals << Sanguosha->getGeneral(item->objectName());

    int index = 0;
    foreach (const General *general, generals) {
        int party = 0;
        bool has_lord = false;
        foreach (const General *other, generals) {
            if (other->getKingdom() == general->getKingdom()) {
                party++;
                if (other != general && other->isLord())
                    has_lord = true;
            }
        }
        GeneralCardItem *item = items.at(index);
        if ((party < 2 || (selected.isEmpty() && has_lord && party == 2))) {
            if (item->isEnabled())
                item->setEnabled(false);
        } else if (!item->isEnabled())
            item->setEnabled(true);

        if (Self->isDead() && !item->isEnabled())
            item->setEnabled(true);
        ++ index;
    }
}

void ChooseGeneralBox::reply() {
    QString generals;
    if (!selected.isEmpty()) {
        generals = selected.first()->objectName();
        if (selected.length() == 2)
            generals += ("+" + selected.last()->objectName());
    }
    if (progress_bar != NULL){
        progress_bar->hide();
        progress_bar->deleteLater();
    }
    ClientInstance->onPlayerChooseGeneral(generals);
    clear();
}

void ChooseGeneralBox::clear() {
    foreach (GeneralCardItem *card_item, items)
        card_item->deleteLater();

    foreach (GeneralCardItem *card_item, selected)
        card_item->deleteLater();

    items.clear();
    selected.clear();

    update();

    hide();
}

void ChooseGeneralBox::_onItemClicked() {
    GeneralCardItem *item = qobject_cast<GeneralCardItem *>(sender());
    if (item == NULL) return;

    if (single_result) {
        selected << item;
        reply();
        return;
    }

    const int card_width = G_COMMON_LAYOUT.m_cardNormalWidth;
    const int card_height = G_COMMON_LAYOUT.m_cardNormalHeight;

    if (selected.contains(item)) {
        selected.removeOne(item);
        items << item;
        item->setHomePos(item->data(S_DATA_INITIAL_HOME_POS).toPointF());
        item->goBack(true);
    } else if (items.contains(item)) {
        if (selected.length() > 1) return;
        items.removeOne(item);
        selected << item;
    }

    if (!selected.isEmpty()) {
        int dest_seat_y = top_blank_width + G_COMMON_LAYOUT.m_cardNormalHeight + card_bottom_to_split_line + split_line_to_card_seat + card_height / 2;
        if (general_number > 5)
            dest_seat_y += (card_to_center_line + card_height);
        selected.first()->setHomePos(QPointF(boundingRect().center().x() - card_to_center_line - card_width / 2, dest_seat_y));
        selected.first()->goBack(true);
        if (selected.length() == 2) {
            selected.last()->setHomePos(QPointF(boundingRect().center().x() + card_to_center_line + card_width / 2, dest_seat_y));
            selected.last()->goBack(true);
        }
    }

    adjustItems();
}

void ChooseGeneralBox::_onCardItemHover() {
    QGraphicsItem *card_item = qobject_cast<QGraphicsItem *>(sender());
    if (!card_item) return;

    animations->emphasize(card_item);
}

void ChooseGeneralBox::_onCardItemLeaveHover() {
    QGraphicsItem *card_item = qobject_cast<QGraphicsItem *>(sender());
    if (!card_item) return;

    animations->effectOut(card_item);
}
