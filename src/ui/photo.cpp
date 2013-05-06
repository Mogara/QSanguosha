#include "photo.h"
#include "clientplayer.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "client.h"
#include "playercarddialog.h"
#include "rolecombobox.h"
#include "SkinBank.h"

#include <QPainter>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMessageBox>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QMenu>
#include <QFile>

#include "pixmapanimation.h"

using namespace QSanProtocol;

// skins that remain to be extracted:
// equips
// mark
// emotions
// hp
// seatNumber
// death logo
// kingdom mask and kingdom icon (decouple from player)
// make layers (drawing order) configurable

Photo::Photo(): PlayerCardContainer() {
    _m_mainFrame = NULL;
    m_player = NULL;
    _m_focusFrame = NULL;
    _m_onlineStatusItem = NULL;
    _m_layout = &G_PHOTO_LAYOUT;
    _m_frameType = S_FRAME_NO_FRAME;
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setTransform(QTransform::fromTranslate(-G_PHOTO_LAYOUT.m_normalWidth / 2, -G_PHOTO_LAYOUT.m_normalHeight / 2), true);
    _m_skillNameItem = new QGraphicsPixmapItem(_m_groupMain);
        
    emotion_item = new QGraphicsPixmapItem(_m_groupMain);
    emotion_item->moveBy(10, 0);

    _m_duanchangMask = new QGraphicsRectItem(_m_groupMain);
    _m_duanchangMask->setRect(boundingRect());
    _m_duanchangMask->setZValue(32767.0);
    _m_duanchangMask->setOpacity(0.4);
    _m_duanchangMask->hide();
    QBrush duanchang_brush(G_PHOTO_LAYOUT.m_duanchangMaskColor);
    _m_duanchangMask->setBrush(duanchang_brush);

    _createControls();
}

void Photo::refresh() {
    PlayerCardContainer::refresh();
    if (!m_player) return;
    QString state_str = m_player->getState();
    if (!state_str.isEmpty() && state_str != "online") {
        QRect rect = G_PHOTO_LAYOUT.m_onlineStatusArea;
        QImage image(rect.size(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.fillRect(QRect(0, 0, rect.width(), rect.height()), G_PHOTO_LAYOUT.m_onlineStatusBgColor);
        G_PHOTO_LAYOUT.m_onlineStatusFont.paintText(&painter, QRect(QPoint(0, 0), rect.size()),
                                                    Qt::AlignCenter,
                                                    Sanguosha->translate(state_str));
        QPixmap pixmap = QPixmap::fromImage(image);
        _paintPixmap(_m_onlineStatusItem, rect, pixmap, _m_groupMain);
        _layBetween(_m_onlineStatusItem, _m_mainFrame, _m_chainIcon);
        if (!_m_onlineStatusItem->isVisible()) _m_onlineStatusItem->show();
    } else if (_m_onlineStatusItem != NULL && state_str == "online")
        _m_onlineStatusItem->hide();

}

QRectF Photo::boundingRect() const{
    return QRect(0, 0, G_PHOTO_LAYOUT.m_normalWidth, G_PHOTO_LAYOUT.m_normalHeight);
}

void Photo::repaintAll() {
    resetTransform();
    setTransform(QTransform::fromTranslate(-G_PHOTO_LAYOUT.m_normalWidth / 2, -G_PHOTO_LAYOUT.m_normalHeight / 2), true);
    _paintPixmap(_m_mainFrame, G_PHOTO_LAYOUT.m_mainFrameArea, QSanRoomSkin::S_SKIN_KEY_MAINFRAME);
    setFrame(_m_frameType);
    hideSkillName(); // @todo: currently we don't adjust skillName's position for simplicity,
                     // consider repainting it instead of hiding it in the future.
    PlayerCardContainer::repaintAll();    
    refresh();
}

void Photo::_adjustComponentZValues() {
    PlayerCardContainer::_adjustComponentZValues();
    _layBetween(_m_mainFrame, _m_faceTurnedIcon, _m_equipRegions[3]);    
    _layBetween(emotion_item, _m_chainIcon, _m_roleComboBox);
    _layBetween(_m_skillNameItem, _m_chainIcon, _m_roleComboBox);
    _m_progressBarItem->setZValue(_m_groupMain->zValue() + 1);
}

void Photo::setEmotion(const QString &emotion, bool permanent) {
    if (emotion == ".") {
        emotion_item->hide();
        return;
    }

    QString path = QString("image/system/emotion/%1.png").arg(emotion);
    if (QFile::exists(path)) {
        emotion_item->setPixmap(QPixmap(path));
        _layBetween(emotion_item, _m_chainIcon, _m_roleComboBox);
        emotion_item->show();

        if (!permanent)
            QTimer::singleShot(2000, this, SLOT(hideEmotion()));
    } else {
        PixmapAnimation::GetPixmapAnimation(this, emotion);
    }
}

void Photo::tremble() {
    QPropertyAnimation *vibrate = new QPropertyAnimation(this, "x");
    static qreal offset = 20;

    vibrate->setKeyValueAt(0.5, x() - offset);
    vibrate->setEndValue(x());

    vibrate->setEasingCurve(QEasingCurve::OutInBounce);

    vibrate->start(QAbstractAnimation::DeleteWhenStopped);
}

void Photo::showSkillName(const QString &skill_name) {
    G_PHOTO_LAYOUT.m_skillNameFont.paintText(_m_skillNameItem,
                                             G_PHOTO_LAYOUT.m_skillNameArea,
                                             Qt::AlignLeft,
                                             Sanguosha->translate(skill_name));
    _m_skillNameItem->show();
    QTimer::singleShot(1000, this, SLOT(hideSkillName()));
}

void Photo::hideSkillName() {
    _m_skillNameItem->hide();
}

void Photo::hideEmotion() {
    emotion_item->hide();
}

void Photo::updateDuanchang() {
    if (!m_player) return;
    _m_duanchangMask->setVisible(m_player->getMark("@duanchang") > 0);
}

const ClientPlayer *Photo::getPlayer() const{
    return m_player;
}

void Photo::speak(const QString &content) {
}

QList<CardItem *> Photo::removeCardItems(const QList<int> &card_ids, Player::Place place) {
    QList<CardItem *> result;    
    if (place == Player::PlaceHand || place == Player::PlaceSpecial) {
        result = _createCards(card_ids);
        updateHandcardNum();
    } else if (place == Player::PlaceEquip) {
        result = removeEquips(card_ids);
    } else if (place == Player::PlaceDelayedTrick) {
        result = removeDelayedTricks(card_ids);
    }

    // if it is just one card from equip or judge area, we'd like to keep them
    // to start from the equip/trick icon.
    if (result.size() > 1 || (place != Player::PlaceEquip && place != Player::PlaceDelayedTrick))
        _disperseCards(result, G_PHOTO_LAYOUT.m_cardMoveRegion, Qt::AlignCenter, false, false);
    
    update();
    return result;
}

bool Photo::_addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo) {
    _disperseCards(card_items, G_PHOTO_LAYOUT.m_cardMoveRegion, Qt::AlignCenter, true, false);
    double homeOpacity = 0.0;
    bool destroy = true;

    Player::Place place = moveInfo.to_place;

    foreach (CardItem *card_item, card_items)
        card_item->setHomeOpacity(homeOpacity);
    if (place == Player::PlaceEquip) {
        addEquips(card_items);
        destroy = false;
    } else if (place == Player::PlaceDelayedTrick) {
        addDelayedTricks(card_items);
        destroy = false;
    } else if (place == Player::PlaceHand) {
        updateHandcardNum();
    }
    return destroy;
}

void Photo::setFrame(FrameType type) {
    _m_frameType = type;
    if (type == S_FRAME_NO_FRAME) {
        if (_m_focusFrame) {
            if (_m_saveMeIcon && _m_saveMeIcon->isVisible())
                setFrame(S_FRAME_SOS);
            else if (m_player->getPhase() != Player::NotActive)
                setFrame(S_FRAME_PLAYING);
            else
                _m_focusFrame->hide();
        }
    } else {
        _paintPixmap(_m_focusFrame, G_PHOTO_LAYOUT.m_focusFrameArea,
                     _getPixmap(QSanRoomSkin::S_SKIN_KEY_FOCUS_FRAME, QString::number(type)),
                     _m_groupMain);
        _layBetween(_m_focusFrame, _m_avatarArea, _m_mainFrame);
        _m_focusFrame->show();
    }
    update();
}

void Photo::updatePhase() {
    PlayerCardContainer::updatePhase();
    if (m_player->getPhase() != Player::NotActive)
        setFrame(S_FRAME_PLAYING);
    else
        setFrame(S_FRAME_NO_FRAME);
}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

QGraphicsItem *Photo::getMouseClickReceiver() {
    return this; 
}

