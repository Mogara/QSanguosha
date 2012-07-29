#include "GenericCardContainerUI.h"
#include <QParallelAnimationGroup>
#include <qpropertyanimation.h>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsProxyWidget>
#include <QGraphicsColorizeEffect>
#include <qpushbutton.h>
#include <qtextdocument.h>
#include <qmenu.h>
#include <qlabel.h>
#include "engine.h"
#include "standard.h"
#include "clientplayer.h"

using namespace QSanProtocol;

QList<CardItem*> GenericCardContainer::cloneCardItems(QList<int> card_ids){
    return _createCards(card_ids);
}

QList<CardItem*> GenericCardContainer::_createCards(QList<int> card_ids)
{
    QList<CardItem*> result;
    foreach (int card_id, card_ids)
    {
        CardItem* item = _createCard(card_id);
        result.append(item);
    }
    return result;
}

CardItem* GenericCardContainer::_createCard(int card_id)
{
    const Card* card = Sanguosha->getCard(card_id);
    CardItem *item = new CardItem(card);
    item->setOpacity(0.0);
    item->setParentItem(this);
    return item;
}

void GenericCardContainer::_destroyCard()
{
    CardItem* card = (CardItem*)sender();
    card->setVisible(false);
    card->deleteLater();
}

bool GenericCardContainer::_horizontalPosLessThan(const CardItem* card1, const CardItem* card2)
{
    return (card1->x() < card2->x());
}

void GenericCardContainer::_disperseCards(QList<CardItem*> &cards, QRectF fillRegion,
                                            Qt::Alignment align, bool useHomePos, bool keepOrder)
{
    int numCards = cards.size();
    if (numCards == 0) return;
    if (!keepOrder)
        qSort(cards.begin(), cards.end(), GenericCardContainer::_horizontalPosLessThan);
    double maxWidth = fillRegion.width();
    int cardWidth = G_COMMON_LAYOUT.m_cardNormalWidth;
    double step = qMin((double)cardWidth, (maxWidth - cardWidth) / (numCards - 1));
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

void GenericCardContainer::onAnimationFinished()
{
}

void GenericCardContainer::_doUpdate()
{
    update();
}

void GenericCardContainer::_playMoveCardsAnimation(QList<CardItem*> &cards, bool destroyCards)
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

void GenericCardContainer::addCardItems(QList<CardItem*> &card_items, const CardsMoveStruct &moveInfo)
{
    foreach (CardItem* card_item, card_items)
    {        
        card_item->setPos(mapFromScene(card_item->scenePos()));
        card_item->setParentItem(this);        
    }
    bool destroy = _addCardItems(card_items, moveInfo);
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
    _paintPixmap(item, rect, pixmap, _m_groupMain);
}

QPixmap PlayerCardContainer::_getPixmap(const QString &key, const QString &sArg)
{
    Q_ASSERT(key.contains("%1"));
    if (key.contains("%2"))
    {
        QString rKey = key.arg(getResourceKeyName()).arg(sArg);
 
        if (G_ROOM_SKIN.isImageKeyDefined(rKey))
            return G_ROOM_SKIN.getPixmap(rKey); // first try "%1key%2 = ...", %1 = "photo", %2 = sArg
    
        rKey = key.arg(getResourceKeyName()).arg(QSanRoomSkin::S_SKIN_KEY_DEFAULT);
        Q_ASSERT(G_ROOM_SKIN.isImageKeyDefined(rKey));
        return G_ROOM_SKIN.getPixmap(rKey, sArg); // then try "%1key = ..."
    }
    else 
    {
        return G_ROOM_SKIN.getPixmap(key, sArg); // finally, try "key = ..."
    }
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
        item->setTransformationMode(Qt::SmoothTransformation);
    }
    item->setPos(rect.x(), rect.y());
    if (pixmap.size() == rect.size())
        item->setPixmap(pixmap);
    else
        item->setPixmap(pixmap.scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    item->setParentItem(parent);
}

void PlayerCardContainer::_clearPixmap(QGraphicsPixmapItem *pixmap)
{
    QPixmap dummy;
    if (pixmap == NULL) return;   
    pixmap->setPixmap(dummy);
    pixmap->hide();
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


QPixmap PlayerCardContainer::_getAvatarIcon(QString heroName)
{
    int avatarSize = m_player->getGeneral2() ? _m_layout->m_primaryAvatarSize : _m_layout->m_avatarSize;
    return G_ROOM_SKIN.getGeneralPixmap(heroName, (QSanRoomSkin::GeneralIconSize)avatarSize);
}

void PlayerCardContainer::updateAvatar()
{
    const General *general = NULL;
    if (m_player) {
        general = m_player->getAvatarGeneral();
        _m_layout->m_screenNameFont.paintText(_m_screenNameItem, 
                                              _m_layout->m_screenNameArea,
                                              Qt::AlignCenter,
                                              m_player->screenName());
    }
    if (general != NULL) {
        _m_avatarArea->setToolTip(general->getSkillDescription());
        QPixmap avatarIcon = _getAvatarIcon(general->objectName());
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
            QString name = Sanguosha->translate("&" + general->objectName());
            if (name.startsWith("&"))
                name = Sanguosha->translate(general->objectName());
            _m_layout->m_avatarNameFont.paintText(_m_avatarNameItem, 
                                                  _m_layout->m_avatarNameArea,
                                                  Qt::AlignLeft | Qt::AlignJustify, name);
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

QPixmap PlayerCardContainer::paintByMask(QPixmap &source){
    QPixmap tmp = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_GENERAL_CIRCLE_MASK, QString::number(_m_layout->m_circleImageSize));
    QPainter p(&tmp);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.drawPixmap(0, 0, _m_layout->m_smallAvatarArea.width(), _m_layout->m_smallAvatarArea.height(), source);
    return tmp;
}

void PlayerCardContainer::updateSmallAvatar()
{
    updateAvatar();
    const General *general = NULL;
    if (m_player) general = m_player->getGeneral2();
    if (general != NULL) {
        QPixmap smallAvatarIcon = G_ROOM_SKIN.getGeneralPixmap(
            general->objectName(),
            QSanRoomSkin::GeneralIconSize(_m_layout->m_smallAvatarSize));
        smallAvatarIcon = paintByMask(smallAvatarIcon);
        _paintPixmap(_m_smallAvatarIcon, _m_layout->m_smallAvatarArea,
                     smallAvatarIcon, _getAvatarParent());
        _paintPixmap(_m_circleItem, _m_layout->m_circleArea,
                     QString(QSanRoomSkin::S_SKIN_KEY_GENERAL_CIRCLE_IMAGE).arg(_m_layout->m_circleImageSize),
                     _getAvatarParent());
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
    else if (m_player->getPhase() != Player::NotActive ) {
        if(m_player->getPhase() == Player::PhaseNone)
            return;
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

static bool CompareByNumber(const Card *card1, const Card *card2){
    return card1->getNumber() < card2->getNumber();
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
        button->setProperty("private_pile", "true");
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
        menu->setProperty("private_pile", "true");

        //Sort the cards in pile by number can let players know what is in this pile more clear.
        //If someone has "buqu", we can got which card he need or which he hate easier.
        QList<const Card *> cards;
        foreach(int card_id, pile){
            const Card *card = Sanguosha->getCard(card_id);
            if (card != NULL) cards << card;
        }
        qSort(cards.begin(), cards.end(), CompareByNumber);

        foreach (const Card *card, cards)
            menu->addAction(G_ROOM_SKIN.getCardSuitPixmap(card->getSuit()), card->getFullName());

        if (cards.count() > 0) button->setMenu(menu);
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
    int num = 0;    
    if (m_player && m_player->getGeneral()) num = m_player->getHandcardNum();
    Q_ASSERT(num >= 0);
    _m_layout->m_handCardFont.paintText(_m_handCardNumText, _m_layout->m_handCardArea, 
        Qt::AlignCenter, QString::number(num));
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

void PlayerCardContainer::_updateEquips()
{
    for (int i = 0; i < 4; i++) {
        CardItem* equip = _m_equipCards[i];
        if (equip == NULL) continue;
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard()->getRealCard());
        QPixmap pixmap = _getEquipPixmap(equip_card);
        _m_equipLabel[i]->setPixmap(pixmap);
        _m_equipRegions[i]->setPos(_m_layout->m_equipAreas[i].topLeft());
    }
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
    else if(m_player)
    {
        if (_m_faceTurnedIcon) _m_faceTurnedIcon->setVisible(!m_player->faceUp());
        if (_m_chainIcon) _m_chainIcon->setVisible(m_player->isChained());
        if (_m_actionIcon) _m_actionIcon->setVisible(m_player->hasFlag("actioned"));
        if (_m_deathIcon) _m_deathIcon->setVisible(m_player->isDead());
    }
    updateHandcardNum();
    _adjustComponentZValues();
}

void PlayerCardContainer::repaintAll()
{
    _m_avatarArea->setRect(_m_layout->m_avatarArea);
    _m_smallAvatarArea->setRect(_m_layout->m_smallAvatarArea);

    updateAvatar();
    updateSmallAvatar();
    updatePhase();
    updateMarks();
    _updateProgressBar();
    _updateDeathIcon();
    _updateEquips();
    updateDelayedTricks();

    if (_m_huashenAnimation != NULL)
    {
        startHuaShen(_m_huashenGeneralName, _m_huashenSkillName);
    }

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

    _m_hpBox->setIconSize(_m_layout->m_magatamaSize);
    _m_hpBox->setOrientation(_m_layout->m_magatamasHorizontal ?  Qt::Horizontal : Qt::Vertical);
    _m_hpBox->setBackgroundVisible(_m_layout->m_magatamasBgVisible);
    _m_hpBox->setAnchorEnable(true);
    _m_hpBox->setAnchor(_m_layout->m_magatamasAnchor, _m_layout->m_magatamasAlign);
    _m_hpBox->setImageArea(_m_layout->m_magatamaImageArea);
    _m_hpBox->update();

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
        _paintPixmap(item, start, G_ROOM_SKIN.getCardJudgeIconPixmap(trick->getCard()->objectName()));
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
    const Card *realCard = Sanguosha->getEngineCard(equip->getEffectiveId());
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
        Qt::AlignLeft | Qt::AlignCenter,
        Sanguosha->translate(equip->objectName()));
    // equip suit
    painter.drawPixmap(_m_layout->m_equipSuitArea,
        G_ROOM_SKIN.getCardSuitPixmap(realCard->getSuit()));
    // equip point
    _m_layout->m_equipPointFont.paintText(
        &painter,
        _m_layout->m_equipPointArea,
        Qt::AlignLeft | Qt::AlignVCenter,
        realCard->getNumberString());
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
    if (index != 1)
    {
        _m_layout->m_equipFont.paintText(
            &painter,
            _m_layout->m_equipDistanceArea,
            Qt::AlignLeft | Qt::AlignVCenter,
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
    if (_getPhaseParent() == _m_floatingArea) updatePhase();
    if (_getMarkParent() == _m_floatingArea) updateMarks();
    if (_getProgressBarParent() == _m_floatingArea) _updateProgressBar();
}

void PlayerCardContainer::addEquips(QList<CardItem*> &equips)
{   

    foreach (CardItem* equip, equips)
    {
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard()->getRealCard());
        int index = (int)(equip_card->location());
        Q_ASSERT(_m_equipCards[index] == NULL);
        _m_equipCards[index] = equip;
        connect(equip, SIGNAL(mark_changed()), this, SLOT(_onEquipSelectChanged()));
        equip->setHomeOpacity(0.0);
        equip->setHomePos(_m_layout->m_equipAreas[index].center());
        _m_equipRegions[index]->setToolTip(equip_card->getDescription());
        QPixmap pixmap = _getEquipPixmap(equip_card);
        _m_equipLabel[index]->setPixmap(pixmap);

        _mutexEquipAnim.lock();
        _m_equipRegions[index]->setPos(_m_layout->m_equipAreas[index].topLeft() + 
                                       QPoint(_m_layout->m_equipAreas[index].width() / 2, 0));
        _m_equipRegions[index]->setOpacity(0);
        _m_equipRegions[index]->show();
        _m_equipAnim[index]->stop();
        _m_equipAnim[index]->clear();
        QPropertyAnimation* anim = new QPropertyAnimation(_m_equipRegions[index], "pos");
        anim->setEndValue(_m_layout->m_equipAreas[index].topLeft());
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        anim = new QPropertyAnimation(_m_equipRegions[index], "opacity");
        anim->setEndValue(255);
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        _m_equipAnim[index]->start();
        _mutexEquipAnim.unlock();
        // _paintPixmap(_m_equipRegions[index], _m_layout->m_equipAreas[index], 
        //             _getEquipPixmap(equip_card), _getEquipParent());
        const Skill *skill = equip_card->getSkill();
        if(skill == NULL) skill = Sanguosha->getSkill(equip_card->objectName());
        if(skill == NULL) continue;
        emit add_equip_skill(skill, true);
    }
}

 QList<CardItem*> PlayerCardContainer::removeEquips(const QList<int> &cardIds)
 {
    QList<CardItem*> result;
    foreach (int card_id, cardIds)
    {
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(Sanguosha->getEngineCard(card_id));
        int index = (int)(equip_card->location());
        Q_ASSERT(_m_equipCards[index] != NULL);
        CardItem* equip = _m_equipCards[index];
        equip->setHomeOpacity(0.0);
        equip->setPos(_m_layout->m_equipAreas[index].center());
        result.append(equip);
        _m_equipCards[index] = NULL;
        _mutexEquipAnim.lock();        
        _m_equipAnim[index]->stop();
        _m_equipAnim[index]->clear();
        QPropertyAnimation* anim = new QPropertyAnimation(_m_equipRegions[index], "pos");
        anim->setEndValue(_m_layout->m_equipAreas[index].topLeft() + 
                          QPoint(_m_layout->m_equipAreas[index].width() / 2, 0));
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        anim = new QPropertyAnimation(_m_equipRegions[index], "opacity");
        anim->setEndValue(0);
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        _m_equipAnim[index]->start();
        _mutexEquipAnim.unlock();

        const Skill *skill = equip_card->getSkill();
        if(skill == NULL) skill = Sanguosha->getSkill(equip_card->objectName());
        if(skill != NULL) emit remove_equip_skill(skill->objectName());
    }
    return result;
 }

 void PlayerCardContainer::startHuaShen(QString generalName, QString skillName)
 {
     _m_huashenGeneralName = generalName;
     _m_huashenSkillName = skillName;
     Q_ASSERT(m_player->hasSkill("huashen"));
     QPixmap pixmap = _getAvatarIcon(generalName);
     if (pixmap.size() != _m_layout->m_avatarArea.size())
         pixmap = pixmap.scaled(_m_layout->m_avatarArea.size());
     stopHuaShen();
     _m_huashenAnimation = G_ROOM_SKIN.createHuaShenAnimation(
         pixmap,
         _m_layout->m_avatarArea.topLeft(),
         _getAvatarParent(),
         _m_huashenItem);
     _m_huashenAnimation->start();
     _paintPixmap(_m_extraSkillBg, _m_layout->m_extraSkillArea, QSanRoomSkin::S_SKIN_KEY_EXTRA_SKILL_BG, _getAvatarParent());
     _m_extraSkillBg->show();
     _m_layout->m_extraSkillFont.paintText(_m_extraSkillText, _m_layout->m_extraSkillTextArea, Qt::AlignCenter,
                                           Sanguosha->translate(skillName).left(2));
     _m_extraSkillText->show();
     _m_extraSkillBg->setToolTip(Sanguosha->getSkill(skillName)->getDescription());
     _adjustComponentZValues();
 }

 void PlayerCardContainer::stopHuaShen()
 {
     if (_m_huashenAnimation != NULL)
     {
         _m_huashenAnimation->stop();
         _m_huashenAnimation->deleteLater();
         delete _m_huashenItem;
         _m_huashenAnimation = NULL;
         _m_huashenItem = NULL;
         _clearPixmap(_m_extraSkillBg);
         _clearPixmap(_m_extraSkillText);
     }
 }

PlayerCardContainer::PlayerCardContainer()
{
    _m_layout = NULL;
    _m_avatarArea = _m_smallAvatarArea = NULL;
    _m_avatarNameItem = _m_smallAvatarNameItem = NULL;
    _m_avatarIcon = _m_smallAvatarIcon = _m_circleItem = NULL;
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
        _m_equipAnim[i] = NULL;
        _m_equipLabel[i] = NULL;
    }
    _m_huashenItem = NULL;
    _m_huashenAnimation = NULL;
    _m_extraSkillBg = NULL;
    _m_extraSkillText = NULL;

    _m_floatingArea = NULL;
    _m_votesGot = 0;
    _m_maxVotes = 1;
    _m_votesItem = NULL;
    _m_groupMain = new QGraphicsPixmapItem(this);
    _m_groupMain->setFlag(ItemHasNoContents);
    _m_groupMain->setPos(0, 0);
    _m_groupDeath = new QGraphicsPixmapItem(this);
    _m_groupDeath->setFlag(ItemHasNoContents);
    _m_groupDeath->setPos(0, 0);
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
    Q_ASSERT((unsigned long)item != 0xcdcdcdcd);
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
    _layUnder(_m_votesItem);
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
    _layUnder(_m_extraSkillText);
    _layUnder(_m_extraSkillBg);
    _layUnder(_m_faceTurnedIcon);  
    _layUnder(_m_smallAvatarArea);
    _layUnder(_m_avatarArea);
    _layUnder(_m_circleItem);
    _layUnder(_m_smallAvatarIcon);
    _layUnder(_m_huashenItem);
    _layUnder(_m_avatarIcon);    
}

void PlayerCardContainer::updateRole(const QString &role)
{
    _m_roleComboBox->fix(role);
}

void PlayerCardContainer::_updateProgressBar()
{
    QGraphicsItem* parent = _getProgressBarParent();
    if (parent == NULL) return;
    _m_progressBar->setOrientation(_m_layout->m_isProgressBarHorizontal ? Qt::Horizontal : Qt::Vertical);  
    QRectF newRect = _m_layout->m_progressBarArea
                     .getTranslatedRect(parent->boundingRect().toRect());
    _m_progressBar->setFixedHeight(newRect.height());
    _m_progressBar->setFixedWidth(newRect.width());
    _m_progressBarItem->setParentItem(parent);
    _m_progressBarItem->setPos(newRect.left(), newRect.top());
}

void PlayerCardContainer::_createControls()
{
    _m_floatingArea = new QGraphicsPixmapItem(_m_groupMain);
    
    _m_screenNameItem = new QGraphicsPixmapItem(_getAvatarParent());

    _m_avatarArea = new QGraphicsRectItem(_m_layout->m_avatarArea, _getAvatarParent());
    _m_avatarArea->setPen(Qt::NoPen);
    _m_avatarNameItem = new QGraphicsPixmapItem(_getAvatarParent());
    
    _m_smallAvatarArea = new QGraphicsRectItem(_m_layout->m_smallAvatarArea, _getAvatarParent());
    _m_smallAvatarArea->setPen(Qt::NoPen);
    _m_smallAvatarNameItem = new QGraphicsPixmapItem(_getAvatarParent());
    
    _m_extraSkillText = new QGraphicsPixmapItem(_getAvatarParent());
    _m_extraSkillText->hide();
    
    _m_handCardNumText = new QGraphicsPixmapItem(_getAvatarParent());

    _m_hpBox = new MagatamasBoxItem(_getAvatarParent());
    
     // Now set up progress bar
    _m_progressBar = new QSanCommandProgressBar;
    _m_progressBar->setAutoHide(true);
    _m_progressBar->hide();  
    _m_progressBarItem = new QGraphicsProxyWidget(_getProgressBarParent());
    _m_progressBarItem->setWidget(_m_progressBar);
    _updateProgressBar();

    for (int i = 0; i < 4; i++)
    {
        _m_equipLabel[i] = new QLabel;
        _m_equipLabel[i]->setStyleSheet("QLabel{ background-color: transparent;}");
        _m_equipLabel[i]->setPixmap(QPixmap(_m_layout->m_equipAreas[i].size()));
        _m_equipRegions[i] = new QGraphicsProxyWidget();
        _m_equipRegions[i]->setWidget(_m_equipLabel[i]);
        _m_equipRegions[i]->setPos(_m_layout->m_equipAreas[i].topLeft());
        _m_equipRegions[i]->setParentItem(_getEquipParent());
        _m_equipRegions[i]->hide();
        _m_equipAnim[i] = new QParallelAnimationGroup;
    }

    _m_markItem = new QGraphicsTextItem(_getMarkParent());
    _m_markItem->setDefaultTextColor(Qt::white);

    _createRoleComboBox();
    repaintAll();
}

void PlayerCardContainer::_updateDeathIcon()
{
    if (!m_player || !m_player->isDead()) return;
    QRect deathArea = _m_layout->m_deathIconRegion.getTranslatedRect(
                      _getDeathIconParent()->boundingRect().toRect());
    _paintPixmap(_m_deathIcon, deathArea,
                 QPixmap(m_player->getDeathPixmapPath()), _getDeathIconParent());
    _m_deathIcon->setZValue(_m_groupMain->zValue() + 10);
}
    
void PlayerCardContainer::killPlayer()
{
    _m_roleComboBox->fix(m_player->getRole());
    _m_roleComboBox->setEnabled(false);
    _updateDeathIcon();
    _m_saveMeIcon->hide();
    QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect();
    effect->setColor(_m_layout->m_deathEffectColor);
    effect->setStrength(1.0);
    _m_groupMain->setGraphicsEffect(effect);
    refresh();
    _m_deathIcon->show();
}

void PlayerCardContainer::revivePlayer()
{
    _m_groupMain->setGraphicsEffect(NULL);
    Q_ASSERT(_m_deathIcon);
    _m_deathIcon->hide();
    refresh();    
}

void PlayerCardContainer::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // we need to override QGraphicsItem's selecting behaviours.
}

void PlayerCardContainer::updateVotes(){
    if (!isSelected() || _m_votesGot <= 1)
        _clearPixmap(_m_votesItem);
    else 
    {
        _paintPixmap(_m_votesItem, _m_layout->m_votesIconRegion,
                     _getPixmap(QSanRoomSkin::S_SKIN_KEY_VOTES_NUMBER, QString::number(_m_votesGot)),
                     _getAvatarParent());
        _m_votesItem->setZValue(1);
    }
}

void PlayerCardContainer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{    
    QGraphicsItem* item = getMouseClickReceiver();
    if (item != NULL && item->isUnderMouse() && isEnabled() &&
        (flags() & QGraphicsItem::ItemIsSelectable))
    {
        if (event->button() == Qt::RightButton)
        {
            setSelected(false);
        }
        else if(event->button() == Qt::LeftButton)
        {
            _m_votesGot++;
            if (_m_votesGot > _m_maxVotes) setSelected(false);
            else setSelected(true);
            if (_m_votesGot > 1) emit selected_changed();
        }
        updateVotes();
    }
}

QVariant PlayerCardContainer::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == ItemSelectedHasChanged) {
        if (!value.toBool())
        {
            _m_votesGot = 0;
            _clearPixmap(_m_selectedFrame);
            _m_selectedFrame->hide();
        }
        else
        {
             _paintPixmap(_m_selectedFrame, _m_layout->m_focusFrameArea,
                          _getPixmap(QSanRoomSkin::S_SKIN_KEY_SELECTED_FRAME),
                          _getFocusFrameParent());
             _m_selectedFrame->show();
        }
        updateVotes();
        emit selected_changed();
    } else if(change == ItemEnabledHasChanged) {
        _m_votesGot = 0;
        emit enable_changed();
    } 

    return QGraphicsObject::itemChange(change, value);
}

void PlayerCardContainer::_onEquipSelectChanged()
{
}
