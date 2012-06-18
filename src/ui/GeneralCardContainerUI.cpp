#include "GeneralCardContainerUI.h"
#include <QParallelAnimationGroup>
#include <QGraphicsProxyWidget>
#include <qpushbutton.h>
#include <qtextdocument.h>
#include <qmenu.h>
#include "engine.h"
#include "standard.h"
#include "clientplayer.h"

using namespace QSanProtocol;

QList<CardItem*> GeneralCardContainer::_createCards(QList<int> card_ids)
{
    QList<CardItem*> result;
    foreach (int card_id, card_ids)
    {
        CardItem* item = _createCard(card_id);
        result.append(item);
    }
    return result;
}

CardItem* GeneralCardContainer::_createCard(int card_id)
{
    const Card* card = Sanguosha->getCard(card_id);
    CardItem *item = new CardItem(card);
    item->setOpacity(0.0);
    item->setParentItem(this);
    return item;
}

void GeneralCardContainer::_destroyCard()
{
    CardItem* card = (CardItem*)sender();
    card->setVisible(false);
    card->deleteLater();
}

bool GeneralCardContainer::_horizontalPosLessThan(const CardItem* card1, const CardItem* card2)
{
    return (card1->x() < card2->x());
}

void GeneralCardContainer::_disperseCards(QList<CardItem*> &cards, QRectF fillRegion,
                                            Qt::Alignment align, bool useHomePos, bool keepOrder)
{
    int numCards = cards.size();
    if (numCards == 0) return;
    if (!keepOrder)
        qSort(cards.begin(), cards.end(), GeneralCardContainer::_horizontalPosLessThan);
    double maxWidth = fillRegion.width();
    int cardWidth = G_COMMON_LAYOUT.m_cardNormalWidth;
    double step = qMin((double)cardWidth, maxWidth / numCards);
    align &= Qt::AlignHorizontal_Mask;
    for (int i = 0; i < numCards; i++)
    {
        CardItem* card = cards[i];
        double newX = 0;
        if (align == Qt::AlignHCenter)
            newX = fillRegion.center().x() + step * (i - (numCards - 1) / 2.0);
        else if (align == Qt::AlignLeft)
            newX = fillRegion.left() + step * i + card->boundingRect().width() / 2.0;
        else if (align == Qt::AlignRight)
            newX = fillRegion.right() + step * (i - numCards)  + card->boundingRect().width() / 2.0;
        else continue;
        QPointF newPos = QPointF(newX, fillRegion.center().y());
        if (useHomePos)
            card->setHomePos(newPos);
        else
            card->setPos(newPos);
        card->setZValue(_m_highestZ++);
    }
}

void GeneralCardContainer::onAnimationFinished()
{
}

void GeneralCardContainer::_doUpdate()
{
    update();
}

void GeneralCardContainer::_playMoveCardsAnimation(QList<CardItem*> &cards, bool destroyCards)
{    
    if (destroyCards)    
    {
        _mutex_cardsToBeDestroyed.lock();
        _cardsToBeDestroyed.append(cards);
        _mutex_cardsToBeDestroyed.unlock();
    }
    
    QParallelAnimationGroup* animation = new QParallelAnimationGroup;
    foreach (CardItem* card_item, cards)
    {
        if (destroyCards)        
            connect(card_item, SIGNAL(movement_animation_finished()), this, SLOT(_destroyCard()));
        animation->addAnimation(card_item->getGoBackAnimation(true));
    }
    
    connect(animation, SIGNAL(finished()), this, SLOT(_doUpdate())); 
    connect(animation, SIGNAL(finished()), this, SLOT(onAnimationFinished()));
    animation->start();
}

void GeneralCardContainer::addCardItems(QList<CardItem*> &card_items, Player::Place place)
{
    foreach (CardItem* card_item, card_items)
    {        
        card_item->setPos(mapFromScene(card_item->scenePos()));
        card_item->setParentItem(this);        
    }
    bool destroy = _addCardItems(card_items, place);
    _playMoveCardsAnimation(card_items, destroy);
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem* &item, const QRect &rect, const QString &key)
{
    _paintPixmap(item, rect, _getPixmap(key));
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem* &item,
    const QRect &rect, const QString &key, QGraphicsItem* parent)
{
    _paintPixmap(item, rect, _getPixmap(key), parent);
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem* &item, const QRect &rect, const QPixmap &pixmap)
{
    _paintPixmap(item, rect, pixmap, this);
}

QPixmap PlayerCardContainer::_getPixmap(const QString &key, const QString &sArg)
{
    QString rKey = key.arg(getResourceKeyName()).arg(sArg);
    if (G_ROOM_SKIN.isImageKeyDefined(rKey))
        return G_ROOM_SKIN.getPixmap(rKey);
    else return G_ROOM_SKIN.getPixmap(key.arg(sArg));
}

QPixmap PlayerCardContainer::_getPixmap(const QString &key)
{
    if (key.contains("%1") && G_ROOM_SKIN.isImageKeyDefined(key.arg(getResourceKeyName())))
        return G_ROOM_SKIN.getPixmap(key.arg(getResourceKeyName()));
    else return G_ROOM_SKIN.getPixmap(key);

}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem* &item,
    const QRect &rect, const QPixmap &pixmap, QGraphicsItem* parent)
{
    if (item == NULL)
    {
        item = new QGraphicsPixmapItem(parent);
    }
    item->setPos(rect.x(), rect.y());
    if (pixmap.size() == rect.size())
        item->setPixmap(pixmap);
    else
        item->setPixmap(pixmap.scaled(rect.size()));
    item->setParentItem(parent);
}

void PlayerCardContainer::_clearPixmap(QGraphicsPixmapItem *pixmap)
{
    QPixmap dummy;
    if (pixmap == NULL) return;   
    pixmap->setPixmap(dummy);
}

void PlayerCardContainer::hideProgressBar()
{
    _m_progressBar->hide();
}

void PlayerCardContainer::showProgressBar(Countdown countdown)
{
    _m_progressBar->setCountdown(countdown);
    _m_progressBar->show();
}


void PlayerCardContainer::updateAvatar()
{
    const General *general = NULL;
    if (m_player) {
        general = m_player->getAvatarGeneral();
        _m_layout->m_screenNameFont.paintText(_m_screenNameItem, 
                                              _m_layout->m_screenNameArea,
                                              Qt::AlignHCenter,
                                              m_player->screenName());
    }
    if (general != NULL) {
        _m_avatarArea->setToolTip(general->getSkillDescription());
        QPixmap avatarIcon = G_ROOM_SKIN.getGeneralPixmap(
                     general->objectName(),
                     (QSanRoomSkin::GeneralIconSize)_m_layout->m_avatarSize);
        _paintPixmap(_m_avatarIcon, _m_layout->m_avatarArea, avatarIcon, _getAvatarParent());
        // this is just avatar general, perhaps game has not started yet.
        if (m_player->getGeneral() != NULL) {
            QString kingdom = m_player->getKingdom();
            _paintPixmap(_m_kingdomIcon, _m_layout->m_kingdomIconArea,
                         G_ROOM_SKIN.getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_KINGDOM_ICON).arg(kingdom)),
                         this->_getAvatarParent());
            _paintPixmap(_m_kingdomColorMaskIcon, _m_layout->m_kingdomMaskArea,
                         G_ROOM_SKIN.getPixmap(QString(QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK)
                                               .arg(kingdom)),
                         this->_getAvatarParent());
            _m_layout->m_avatarNameFont.paintText(_m_avatarNameItem, 
                                                  _m_layout->m_avatarNameArea, Qt::AlignLeft,
                                                  Sanguosha->translate(general->objectName()));
        }        
    } else {
        _paintPixmap(_m_avatarIcon, _m_layout->m_avatarArea,
                     QSanRoomSkin::S_SKIN_KEY_BLANK_GENERAL, _getAvatarParent());
        _clearPixmap(_m_kingdomColorMaskIcon);
        _clearPixmap(_m_kingdomIcon);
        _m_avatarArea->setToolTip(QString());
    }
    _m_avatarIcon->show();
    _adjustComponentZValues();
}

void PlayerCardContainer::updateSmallAvatar()
{
    const General *general = NULL;
    if (m_player) general = m_player->getGeneral2();
    if (general != NULL) {
        QPixmap smallAvatarIcon = G_ROOM_SKIN.getGeneralPixmap(
            general->objectName(),
            QSanRoomSkin::GeneralIconSize(_m_layout->m_smallAvatarSize));
        _paintPixmap(_m_smallAvatarIcon, _m_layout->m_smallAvatarArea,
                     smallAvatarIcon);
        _m_smallAvatarArea->setToolTip(general->getSkillDescription());
        _m_layout->m_smallAvatarNameFont.paintText(
            _m_smallAvatarNameItem, 
            _m_layout->m_smallAvatarNameArea, Qt::AlignLeft, // @todo: possibly also make this customizable
            Sanguosha->translate(general->objectName()));
        _m_smallAvatarIcon->show();
    }
    else {
        _m_smallAvatarArea->setToolTip(QString());
    }
    _adjustComponentZValues();
}

void PlayerCardContainer::updateReadyItem(bool visible)
{
    _m_readyIcon->setVisible(visible);
}

void PlayerCardContainer::updatePhase()
{
    if (!m_player || !m_player->isAlive()) {
        _clearPixmap(_m_phaseIcon);
    }
    else if (m_player->getPhase() != Player::NotActive) {
        int index = static_cast<int>(m_player->getPhase());
        QRect phaseArea = _m_layout->m_phaseArea.getTranslatedRect(
                          _getPhaseParent()->boundingRect().toRect());
        _paintPixmap(_m_phaseIcon, phaseArea,
                     _getPixmap(QSanRoomSkin::S_SKIN_KEY_PHASE, QString::number(index)),
                     _getPhaseParent());
        _m_phaseIcon->show();
    } else {
        if (_m_progressBar) _m_progressBar->hide();
        if (_m_phaseIcon) _m_phaseIcon->hide();
    }
}

void PlayerCardContainer::updateHp()
{
    Q_ASSERT(_m_hpBox && _m_saveMeIcon && m_player);
    _m_hpBox->setHp(m_player->getHp());
    _m_hpBox->setMaxHp(m_player->getMaxHp());
    _m_hpBox->update();
    _m_saveMeIcon->setVisible(m_player->getHp() <= 0 && m_player->getMaxHp() > 0);
}

void PlayerCardContainer::updatePile(const QString &pile_name)
{
        // retrieve menu and create a new pile if necessary
    QMenu* menu;
    QPushButton* button;
    if (!_m_privatePiles.contains(pile_name))
    {
        button = new QPushButton;
        button->setObjectName(pile_name);
        button->setProperty("private_pile","true");
        QGraphicsProxyWidget *button_widget = new QGraphicsProxyWidget(_getPileParent());
        button_widget->setWidget(button);
        _m_privatePiles[pile_name] = button_widget;		
    }
    else
    {		
        button = (QPushButton*)(_m_privatePiles[pile_name]->widget());
        menu = button->menu();
    }
    
        
    ClientPlayer* player = (ClientPlayer*)sender();
    const QList<int> &pile = player->getPile(pile_name);
    if (pile.size() == 0)
    {
        delete _m_privatePiles[pile_name];
        _m_privatePiles.remove(pile_name);
    }
    else
    {
        button->setText(QString("%1(%2)").arg(Sanguosha->translate(pile_name)).arg(pile.length()));
        menu = new QMenu(button);
        int visibleCards = 0;
        foreach (int card_id, pile){
            if (card_id == Card::S_UNKNOWN_CARD_ID) continue;
            const Card *card = Sanguosha->getCard(card_id);
            menu->addAction(G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()), card->getFullName());
            visibleCards++;
        }
        if (visibleCards > 0) button->setMenu(menu);
        else
        {
            delete menu;
            button->setMenu(NULL);
        }
    }        

    QPoint start = _m_layout->m_privatePileStartPos;
    QPoint step = _m_layout->m_privatePileStep;
    QSize size = _m_layout->m_privatePileButtonSize;
    QList<QGraphicsProxyWidget*> widgets = _m_privatePiles.values();
    for (int i = 0; i < widgets.length(); i++)
    {
        QGraphicsProxyWidget* widget = widgets[i];
        widget->setPos(start + i * step);
        widget->resize(size);
    }
}

void PlayerCardContainer::updateDrankState()
{
    if(m_player->hasFlag("drank"))
        _m_avatarArea->setBrush(G_PHOTO_LAYOUT.m_drankMaskColor);
    else
        _m_avatarArea->setBrush(Qt::NoBrush);
}

void PlayerCardContainer::updateHandcardNum()
{
    if (!m_player || !m_player->getGeneral()) return;
    _m_layout->m_handCardFont.paintText(_m_handCardNumText, _m_layout->m_handCardArea, 
        Qt::AlignCenter, QString::number(m_player->getHandcardNum()));
    _m_handCardNumText->setVisible(true);
}

void PlayerCardContainer::updateMarks()
{
    if (!_m_markItem) return;
    QRect parentRect = _getMarkParent()->boundingRect().toRect();
    QSize markSize = _m_markItem->boundingRect().size().toSize();
    QRect newRect = _m_layout->m_markTextArea.getTranslatedRect(parentRect, markSize);
    _m_markItem->setPos(newRect.topLeft());
}

void PlayerCardContainer::refresh()
{
    if (!m_player || !m_player->getGeneral() || !m_player->isAlive())
    {
        _m_faceTurnedIcon->setVisible(false);
        _m_chainIcon->setVisible(false);
        _m_actionIcon->setVisible(false);
        _m_saveMeIcon->setVisible(false);
    }
    else if (m_player && m_player->isDead())
    {
        _paintPixmap(_m_deathIcon, _m_layout->m_deathIconRegion,
            QPixmap(m_player->getDeathPixmapPath()), this);
    }
    else
    {
        _m_faceTurnedIcon->setVisible(!m_player->faceUp());
        _m_chainIcon->setVisible(m_player->isChained());
        _m_actionIcon->setVisible(m_player->hasFlag("actioned"));
    }
    updateHandcardNum();
    _adjustComponentZValues();
}

void PlayerCardContainer::repaintAll()
{
    updateAvatar();
    updateSmallAvatar();
    updatePhase();
    updateMarks();
    _paintPixmap(_m_faceTurnedIcon, _m_layout->m_avatarArea, QSanRoomSkin::S_SKIN_KEY_FACETURNEDMASK,
                 _getAvatarParent());
    _paintPixmap(_m_readyIcon, _m_layout->m_readyIconRegion, QSanRoomSkin::S_SKIN_KEY_READY_ICON,
                 _getAvatarParent());
    _paintPixmap(_m_chainIcon, _m_layout->m_chainedIconRegion, QSanRoomSkin::S_SKIN_KEY_CHAIN,
                 _getAvatarParent());
    _paintPixmap(_m_saveMeIcon, _m_layout->m_saveMeIconRegion, QSanRoomSkin::S_SKIN_KEY_SAVE_ME_ICON,
                 _getAvatarParent());
    _paintPixmap(_m_actionIcon, _m_layout->m_actionedIconRegion, QSanRoomSkin::S_SKIN_KEY_ACTIONED_ICON,
                 _getAvatarParent());
    _paintPixmap(_m_handCardBg, _m_layout->m_handCardArea, QSanRoomSkin::S_SKIN_KEY_HANDCARDNUM,
                 _getAvatarParent());
    if (_m_roleComboBox != NULL)
        _m_roleComboBox->setPos(_m_layout->m_roleComboBoxPos); 
    _adjustComponentZValues();
    refresh();
}

void PlayerCardContainer::_createRoleComboBox()
{
    _m_roleComboBox = new RoleComboBox(_getRoleComboBoxParent());
}

void PlayerCardContainer::setPlayer(ClientPlayer* player)
{
    this->m_player = player;

    if(player){
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));
        connect(player, SIGNAL(general2_changed()), this, SLOT(updateSmallAvatar()));
        connect(player, SIGNAL(kingdom_changed()), this, SLOT(updateAvatar()));
        connect(player, SIGNAL(ready_changed(bool)), this, SLOT(updateReadyItem(bool)));
        connect(player, SIGNAL(state_changed()), this, SLOT(refresh()));
        connect(player, SIGNAL(phase_changed()), this, SLOT(updatePhase()));
        connect(player, SIGNAL(drank_changed()), this, SLOT(updateDrankState()));
        connect(player, SIGNAL(action_taken()), this, SLOT(refresh()));
        connect(player, SIGNAL(pile_changed(QString)), this, SLOT(updatePile(QString)));
        connect(player, SIGNAL(role_changed(QString)), _m_roleComboBox, SLOT(fix(QString)));
        connect(player, SIGNAL(hp_changed()), this, SLOT(updateHp()));

        QTextDocument* textDoc = m_player->getMarkDoc();
        Q_ASSERT(_m_markItem);
        _m_markItem->setDocument(textDoc);
        connect(textDoc, SIGNAL(contentsChanged()), this, SLOT(updateMarks()));
    }
    refresh();
}

 QList<CardItem*> PlayerCardContainer::removeDelayedTricks(const QList<int> &cardIds)
 {
    QList<CardItem*> result;
    foreach (int card_id, cardIds)
    {
        CardItem* item = CardItem::FindItem(_m_judgeCards, card_id);
        Q_ASSERT(item != NULL);
        int index = _m_judgeCards.indexOf(item);
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * index);
        item->setOpacity(0.0);
        item->setPos(start.center());
        _m_judgeCards.removeAt(index);
        delete _m_judgeIcons.takeAt(index);
        result.append(item);
    }
    updateDelayedTricks();
    return result;
 }

void PlayerCardContainer::updateDelayedTricks()
 {
     for (int i = 0; i < _m_judgeIcons.size(); i++)
     {
        QGraphicsPixmapItem *item = _m_judgeIcons[i];
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * i);
        item->setPos(start.topLeft());
    }
 }


void PlayerCardContainer::addDelayedTricks(QList<CardItem*> &tricks)
{
    foreach (CardItem* trick, tricks)
    {
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(_getDelayedTrickParent());
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * _m_judgeCards.size());
        _paintPixmap(item, start, G_ROOM_SKIN.getCardJudgeIconPixmap(trick->objectName()));
        trick->setHomeOpacity(0.0);
        trick->setHomePos(start.center());
        QString toolTip;
        if(trick->getCard()->isVirtualCard())
            toolTip = Sanguosha->getCard(trick->getCard()->getSubcards().at(0))->getDescription();
        else
            toolTip = trick->getCard()->getDescription();
        trick->setToolTip(toolTip);
        _m_judgeCards.append(trick);
        _m_judgeIcons.append(item);
    }
}

QPixmap PlayerCardContainer::_getEquipPixmap(const EquipCard* equip)
{
    QPixmap equipIcon(_m_layout->m_equipAreas[0].size());
    equipIcon.fill(Qt::transparent);
    QPainter painter(&equipIcon);
    // icon / background
    painter.drawPixmap(_m_layout->m_equipImageArea,
        _getPixmap(QSanRoomSkin::S_SKIN_KEY_EQUIP_ICON, equip->objectName()));
    // equip name
    _m_layout->m_equipFont.paintText(
        &painter,
        _m_layout->m_equipTextArea,
        Qt::AlignLeft,
        Sanguosha->translate(equip->objectName()));
    // equip suit
    painter.drawPixmap(_m_layout->m_equipSuitArea,
        G_ROOM_SKIN.getCardSuitPixmap(equip->getSuit()));
    // equip point
    _m_layout->m_equipPointFont.paintText(
        &painter,
        _m_layout->m_equipPointArea,
        Qt::AlignLeft,
        equip->getNumberString());
    // distance
    int index = (int)(equip->location());
    QString distance;
    if (index == 0)
    {
        const Weapon *weapon = qobject_cast<const Weapon*>(equip);
        Q_ASSERT(weapon);
        if (weapon)
            distance = Sanguosha->translate(QString("CAPITAL(%1)")
                .arg(QString::number(weapon->getRange())));
        // fall through
    }
    else if (index == 2)
    {
        const DefensiveHorse* horse = qobject_cast<const DefensiveHorse*>(equip);
        Q_ASSERT(horse);
        if (horse) distance = QString("+%1").arg(QString::number(horse->getCorrect()));
    }
    else if (index == 3)
    {
        const OffensiveHorse* horse = qobject_cast<const OffensiveHorse*>(equip);
        Q_ASSERT(horse);
        if (horse) distance = QString::number(horse->getCorrect());
    }
    if (index != 0)
    {
        _m_layout->m_equipFont.paintText(
            &painter,
            _m_layout->m_equipDistanceArea,
            Qt::AlignLeft,
            distance);
    }
    return equipIcon;
}

void PlayerCardContainer::setFloatingArea(QRect rect)
{
    _m_floatingAreaRect = rect;
    QPixmap dummy(rect.size());
    dummy.fill(Qt::transparent);
    _m_floatingArea->setPixmap(dummy);
    _m_floatingArea->setPos(rect.topLeft());
    updatePhase();
    updateMarks();
}

void PlayerCardContainer::addEquips(QList<CardItem*> &equips)
{    
    foreach (CardItem* equip, equips)
    {
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard());
        int index = (int)(equip_card->location());
        Q_ASSERT(_m_equipCards[index] == NULL);
        _m_equipCards[index] = equip;
        equip->setHomeOpacity(0.0);
        equip->setHomePos(_m_layout->m_equipAreas[index].center());
        _m_equipRegions[index]->setToolTip(equip_card->getDescription());
        _paintPixmap(_m_equipRegions[index], _m_layout->m_equipAreas[index], 
                     _getEquipPixmap(equip_card), _getEquipParent());
    }
}

 QList<CardItem*> PlayerCardContainer::removeEquips(const QList<int> &cardIds)
 {
    QList<CardItem*> result;
    foreach (int card_id, cardIds)
    {
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(Sanguosha->getCard(card_id));
        int index = (int)(equip_card->location());
        Q_ASSERT(_m_equipCards[index] != NULL);
        CardItem* equip = _m_equipCards[index];
        equip->setHomeOpacity(0.0);
        equip->setPos(_m_layout->m_equipAreas[index].center());
        result.append(equip);
        _m_equipCards[index] = NULL;
        _clearPixmap(_m_equipRegions[index]);
    }
    return result;
 }

PlayerCardContainer::PlayerCardContainer()
{
    _m_layout = NULL;
    _m_avatarArea = _m_smallAvatarArea = NULL;
    _m_avatarNameItem = _m_smallAvatarNameItem = NULL;
    _m_avatarIcon = _m_smallAvatarIcon = NULL;
    _m_screenNameItem = NULL;
    _m_chainIcon = _m_faceTurnedIcon = NULL;
    _m_handCardBg = _m_handCardNumText = NULL;
    _m_kingdomColorMaskIcon = _m_deathIcon = NULL;
    _m_readyIcon = _m_actionIcon = NULL;
    _m_kingdomIcon = NULL;
    _m_saveMeIcon = NULL;
    _m_phaseIcon = NULL;
    _m_markItem = NULL;
    _m_roleComboBox = NULL;
    m_player = NULL;
    _m_selectedFrame = NULL;
    
    for (int i = 0; i < 4; i++) {
        _m_equipCards[i] = NULL;
        _m_equipRegions[i] = NULL;
    }
    _m_floatingArea = NULL;
    _allZAdjusted = false;
}

void PlayerCardContainer::hideAvatars()
{
    if (_m_avatarIcon) _m_avatarIcon->hide();
    if (_m_smallAvatarIcon) _m_smallAvatarIcon->hide();
}

void PlayerCardContainer::_layUnder(QGraphicsItem* item)
{
    _lastZ--;
    Q_ASSERT((long)item != 0xcdcdcdcd);
    if (item)
        item->setZValue(_lastZ--);
    else
        _allZAdjusted = false;
}

bool PlayerCardContainer::_startLaying()
{
    if (_allZAdjusted) 
        return false;
    _allZAdjusted = true;
    _lastZ = -1;
    return true;
}

void PlayerCardContainer::_layBetween(QGraphicsItem* middle, QGraphicsItem* item1,
                                       QGraphicsItem* item2)
{
    if (middle && item1 && item2)
        middle->setZValue((item1->zValue() + item2->zValue()) / 2.0);
    else
        _allZAdjusted = false;
}

void PlayerCardContainer::_adjustComponentZValues()
{
    // @todo: consider make this configurable as well.
    // all components use negative zvalues to ensure that no other generated
    // cards can be under us.

    // layout
    if (!_startLaying()) return;
    _layUnder(_m_floatingArea);
    foreach (QGraphicsItem* pile, _m_privatePiles.values())
        _layUnder(pile);    
    foreach (QGraphicsItem* judge, _m_judgeIcons)
        _layUnder(judge);
    _layUnder(_m_markItem);    
    _layUnder(_m_progressBarItem);
    _layUnder(_m_roleComboBox);
    _layUnder(_m_chainIcon);        
    _layUnder(_m_hpBox);
    _layUnder(_m_handCardNumText);
    _layUnder(_m_handCardBg);
    _layUnder(_m_readyIcon);
    _layUnder(_m_actionIcon);
    _layUnder(_m_deathIcon);
    _layUnder(_m_saveMeIcon);
    _layUnder(_m_phaseIcon);
    _layUnder(_m_smallAvatarNameItem);
    _layUnder(_m_avatarNameItem);
    _layUnder(_m_kingdomIcon);
    _layUnder(_m_kingdomColorMaskIcon);
    _layUnder(_m_screenNameItem);
    for (int i = 0; i < 4; i++)
        _layUnder(_m_equipRegions[i]);
    _layUnder(_m_selectedFrame);
    _layUnder(_m_faceTurnedIcon);   
    _layUnder(_m_smallAvatarArea);
    _layUnder(_m_avatarArea);
    _layUnder(_m_smallAvatarIcon);
    _layUnder(_m_avatarIcon);    
}

void PlayerCardContainer::updateRole(const QString &role)
{
    _m_roleComboBox->fix(role);
}

void PlayerCardContainer::_createControls()
{
    _m_floatingArea = new QGraphicsPixmapItem(this);
    
    _m_screenNameItem = new QGraphicsPixmapItem(_getAvatarParent());

    _m_avatarArea = new QGraphicsRectItem(_m_layout->m_avatarArea, _getAvatarParent());
    _m_avatarArea->setPen(Qt::NoPen);
    _m_avatarNameItem = new QGraphicsPixmapItem(_getAvatarParent());

    _m_smallAvatarArea = new QGraphicsRectItem(_m_layout->m_smallAvatarArea, _getAvatarParent());
    _m_smallAvatarArea->setPen(Qt::NoPen);
    _m_smallAvatarNameItem = new QGraphicsPixmapItem(_getAvatarParent());
    
    _m_handCardNumText = new QGraphicsPixmapItem(_getAvatarParent());

    _m_hpBox = new MagatamasBoxItem(_getAvatarParent());
    _m_hpBox->setIconSize(_m_layout->m_magatamaSize);
    _m_hpBox->setOrientation(_m_layout->m_magatamasHorizontal ?  Qt::Horizontal : Qt::Vertical);
    _m_hpBox->setBackgroundVisible(_m_layout->m_magatamasBgVisible);
    _m_hpBox->setAnchorEnable(true);
    _m_hpBox->setAnchor(_m_layout->m_magatamasAnchor, _m_layout->m_magatamasAlign); 
    
     // Now set up progress bar
    _m_progressBar = new QSanCommandProgressBar;
    _m_progressBar->setAutoHide(true);
    _m_progressBar->hide();
    _m_progressBar->setOrientation(_m_layout->m_isProgressBarHorizontal ? Qt::Horizontal : Qt::Vertical);
    _m_progressBar->setFixedHeight(_m_layout->m_progressBarArea.height());
    _m_progressBar->setFixedWidth(_m_layout->m_progressBarArea.width());
    _m_progressBarItem = new QGraphicsProxyWidget(this);
    _m_progressBarItem->setWidget(_m_progressBar);
    _m_progressBarItem->setPos(_m_layout->m_progressBarArea.left(), _m_layout->m_progressBarArea.top());

    for (int i = 0; i < 4; i++)
    {
        QPixmap emptyPixmap(_m_layout->m_equipAreas[i].size());
        _m_equipRegions[i] = new QGraphicsPixmapItem(emptyPixmap);
    }

    _m_markItem = new QGraphicsTextItem(_getMarkParent());
    _m_markItem->setDefaultTextColor(Qt::white);

    _createRoleComboBox();
    repaintAll();
}
    
void PlayerCardContainer::killPlayer()
{
    _m_roleComboBox->fix(m_player->getRole());
    updateAvatar();
    updateSmallAvatar();
    _m_deathIcon->show();
}

void PlayerCardContainer::revivePlayer()
{
    updateAvatar();
    updateSmallAvatar();
    _m_deathIcon->hide();
}

void PlayerCardContainer::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // we need to override QGraphicsItem's selecting behaviours.
}

void PlayerCardContainer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem* item = getMouseClickReceiver();
    if (item != NULL && item->isUnderMouse() && isEnabled() &&
        (flags() & QGraphicsItem::ItemIsSelectable))
    {
        setSelected(!isSelected());
    }
}

QVariant PlayerCardContainer::itemChange(GraphicsItemChange change, const QVariant &value) {
    if(change == ItemSelectedHasChanged){
        if(value.toBool())
        {
             _paintPixmap(_m_selectedFrame, _m_layout->m_focusFrameArea,
                          _getPixmap(QSanRoomSkin::S_SKIN_KEY_SELECTED_FRAME),
                          _getFocusFrameParent());
        }
        else
        {
            _clearPixmap(_m_selectedFrame);
        }

        emit selected_changed();
    }else if(change == ItemEnabledHasChanged){
        emit enable_changed();
    }

    return QGraphicsObject::itemChange(change, value);
}
