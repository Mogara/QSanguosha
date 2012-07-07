#include "dashboard.h"
#include "engine.h"
#include "settings.h"
#include "client.h"
#include "standard.h"
#include "playercarddialog.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPixmapCache>
#include <QParallelAnimationGroup>

using namespace QSanProtocol;

Dashboard::Dashboard(QGraphicsItem *widget)
    : button_widget(widget), selected(NULL), view_as_skill(NULL), filter(NULL)
{
    Q_ASSERT(button_widget);
    _dlayout = &G_DASHBOARD_LAYOUT;
    _m_layout = _dlayout;
    m_player = Self;
    _m_leftFrame = _m_rightFrame = _m_middleFrame = NULL;
    _m_rightFrameBg = NULL;
    animations = new EffectAnimation();
    pending_card = NULL;
    for (int i = 0; i < 4; i++)
    {
        _m_equipSkillBtns[i] = NULL;
        _m_isEquipsAnimOn[i] = false;
    }
    // At this stage, we cannot decide the dashboard size yet, the whole
    // point in creating them here is to allow PlayerCardContainer to 
    // anchor all controls and widgets to the correct frame.
    //
    // Note that 20 is just a random plug-in so that we can proceed with
    // control creation, the actual width is updated when setWidth() is
    // called by its graphics parent.
    //
    _m_width = G_DASHBOARD_LAYOUT.m_leftWidth + G_DASHBOARD_LAYOUT.m_rightWidth + 20; 
    
    _createLeft();
    _createMiddle();
    _createRight();
    
    // only do this after you create all frames.
    _createControls();
}

bool Dashboard::isAvatarUnderMouse()
{
    return _m_avatarArea->isUnderMouse();
}

QGraphicsItem* Dashboard::getMouseClickReceiver()  
{
    return _m_avatarIcon; 
}

void Dashboard::_createLeft(){
    QRect rect = QRect(0, 0, G_DASHBOARD_LAYOUT.m_leftWidth, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(_m_leftFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_LEFTFRAME), this);
    _m_leftFrame->setZValue(-1000); // nobody should be under me.
    _createEquipBorderAnimations();
}

int Dashboard::getButtonWidgetWidth() const{
    Q_ASSERT(button_widget);
    return button_widget->boundingRect().width();
}

void Dashboard::_createMiddle() {    
    // this is just a random rect. see constructor for more details
    QRect rect = QRect(0, 0, 1, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(_m_middleFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_MIDDLEFRAME), this);
    _m_middleFrame->setZValue(-1000); // nobody should be under me.
    button_widget->setParentItem(_m_middleFrame);

    trusting_item = new QGraphicsRectItem(this);
    trusting_text = new QGraphicsSimpleTextItem(tr("Trusting ..."), _m_middleFrame);

    QBrush trusting_brush(QColor(0x26, 0x1A, 0x42));
    trusting_item->setBrush(trusting_brush);
    trusting_item->setOpacity(0.36);
    trusting_item->setZValue(2.0);
    
    trusting_text->setFont(Config.BigFont);
    trusting_text->setBrush(Qt::white);
    trusting_text->setZValue(2.1);

    trusting_item->hide();
    trusting_text->hide();
}

void Dashboard::_adjustComponentZValues()
{
    PlayerCardContainer::_adjustComponentZValues();
    // make sure right frame is on top because we have a lot of stuffs
    // attached to it, such as the rolecomboBox, which should not be under
    // middle frame
    _layUnder(_m_rightFrame);
    _layUnder(_m_leftFrame);
    _layUnder(_m_middleFrame);    
    _layBetween(button_widget, _m_middleFrame, _m_roleComboBox);
    _layBetween(_m_rightFrameBg, _m_faceTurnedIcon, _m_equipRegions[3]);
}

int Dashboard::width()
{
    return this->_m_width;
}

void Dashboard::_createRight()
{ 
    QRect rect = QRect(_m_width - G_DASHBOARD_LAYOUT.m_rightWidth, 0, 
                       G_DASHBOARD_LAYOUT.m_rightWidth,
                       G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(_m_rightFrame, rect, QPixmap(1, 1), this);
    _paintPixmap(_m_rightFrameBg, QRect(0, 0, rect.width(), rect.height()), 
                 _getPixmap(QSanRoomSkin::S_SKIN_KEY_RIGHTFRAME), _m_rightFrame);
    _m_rightFrame->setZValue(-1000); // nobody should be under me.
    
    _m_skillDock = new QSanInvokeSkillDock(_m_rightFrame);
    QRect avatar = G_DASHBOARD_LAYOUT.m_avatarArea;
    _m_skillDock->setPos(avatar.left(), avatar.bottom() + 
                         G_DASHBOARD_LAYOUT.m_skillButtonsSize[0].height());
    _m_skillDock->setWidth(avatar.width());
}

void Dashboard::_updateFrames()
{
    // Here is where we adjust all frames to actual width
    QRect rect = QRect(G_DASHBOARD_LAYOUT.m_leftWidth, 0, 
        this->width() - G_DASHBOARD_LAYOUT.m_rightWidth - G_DASHBOARD_LAYOUT.m_leftWidth, G_DASHBOARD_LAYOUT.m_normalHeight);
    _paintPixmap(_m_middleFrame, rect, _getPixmap(QSanRoomSkin::S_SKIN_KEY_MIDDLEFRAME), this);
    trusting_item->setRect(rect);
    trusting_item->setPos(G_DASHBOARD_LAYOUT.m_leftWidth, 0);
    _m_rightFrame->setX(_m_width - G_DASHBOARD_LAYOUT.m_rightWidth);
    Q_ASSERT(button_widget);
    button_widget->setX(rect.width() - getButtonWidgetWidth());
    button_widget->setY(0);
}

void Dashboard::setFilter(const FilterSkill *filter){
    this->filter = filter;
    doFilter();
}

const FilterSkill *Dashboard::getFilter() const{
    return filter;
}

void Dashboard::doFilter(){
    foreach(CardItem *card_item, m_handCards)
        card_item->filter(filter);
}

void Dashboard::setTrust(bool trust){
    trusting_item->setVisible(trust);
    trusting_text->setVisible(trust);
}

bool Dashboard::_addCardItems(QList<CardItem*> &card_items, Player::Place place)
{
    if (place == Player::PlaceSpecial)
    {
        foreach(CardItem* card, card_items)
        {
            card->setHomeOpacity(0.0);            
        }
        QPointF center = mapFromItem(_getAvatarParent(), _dlayout->m_avatarArea.center());
        QRectF rect = QRectF(0, 0, _dlayout->m_disperseWidth, 0);
        rect.moveCenter(center);
        _disperseCards(card_items, rect, Qt::AlignCenter, true, false);
        return true;
    }

    if (place == Player::PlaceEquip)        
       addEquips(card_items);
    else if (place == Player::PlaceDelayedTrick)
       addDelayedTricks(card_items);
    else if (place == Player::PlaceHand)
       addHandCards(card_items);
    
    adjustCards(true);
    return false;
}

void Dashboard::addHandCards(QList<CardItem*> &card_items)
{
    foreach (CardItem* card_item, card_items)
        _addHandCard(card_item);
    updateHandcardNum();
}

void Dashboard::_addHandCard(CardItem* card_item)
{
    card_item->filter(filter);

    if(ClientInstance->getStatus() == Client::Playing)
        card_item->setEnabled(card_item->getFilteredCard()->isAvailable(Self));
    else
        card_item->setEnabled(false);
    
    card_item->setHomeOpacity(1.0); // @todo: it's 1.0 even if disabled?
    card_item->setRotation(0.0);
    card_item->setFlags(ItemIsFocusable);
    card_item->setZValue(0.1);
    m_handCards << card_item;

    connect(card_item, SIGNAL(clicked()), this, SLOT(onCardItemClicked()));
    connect(card_item, SIGNAL(thrown()), this, SLOT(onCardItemThrown()));
    connect(card_item, SIGNAL(enter_hover()), this, SLOT(onCardItemHover()));
    connect(card_item, SIGNAL(leave_hover()), this, SLOT(onCardItemLeaveHover()));      
}

void Dashboard::selectCard(const QString &pattern, bool forward){
    if(selected)
        selectCard(selected, true); // adjust the position

    // find all cards that match the card type
    QList<CardItem*> matches;

    foreach(CardItem *card_item, m_handCards){
        if(card_item->isEnabled()){
            if(pattern == "." || card_item->getFilteredCard()->match(pattern))
                matches << card_item;
        }
    }

    if(matches.isEmpty()){
        unselectAll();
        return;
    }

    int index = matches.indexOf(selected);
    int n = matches.length();
    if(forward)
        index = (index + 1) % n;
    else
        index = (index - 1 + n) % n;

    CardItem *to_select = matches[index];

    if(to_select != selected){
        if(selected)
            selectCard(selected, false);
        selectCard(to_select, true);
        selected = to_select;

        emit card_selected(selected->getFilteredCard());
    }
}

const Card *Dashboard::getSelected() const
{
    if (view_as_skill)
        return pending_card;
    else if(selected)
        return selected->getFilteredCard();
    else
        return NULL;
}

void Dashboard::selectCard(CardItem* item, bool isSelected){
    //if(Self && Self->getHandcardNum() > Config.MaxCards)
    //    frame->show();    
    bool oldState = item->isSelected();
    if (oldState == isSelected) return;
    m_mutex.lock();
    item->setSelected(isSelected);
    QPointF oldPos = item->homePos();
    QPointF newPos = oldPos;
    if (isSelected)
        newPos.setY(newPos.y() + S_PENDING_OFFSET_Y);
    else 
        newPos.setY(newPos.y() - S_PENDING_OFFSET_Y);
    item->setHomePos(newPos);  
    selected = item;
    // setY(PendingY);
    // if (!hasFocus()) item->goBack(true);
    m_mutex.unlock();
}

void Dashboard::unselectAll(){
    selected = NULL;

    foreach(CardItem *card_item, m_handCards){
        selectCard(card_item, false);
    }

    adjustCards(true);
}

QRectF Dashboard::boundingRect() const{
    return QRectF(0, 0, _m_width, _m_layout->m_normalHeight);
}

void Dashboard::setWidth(int width){
    prepareGeometryChange();
    adjustCards(true);
    this->_m_width = width;
    _updateFrames();
}

QGraphicsProxyWidget *Dashboard::addWidget(QWidget *widget, int x, bool toLeft){
    QGraphicsProxyWidget *proxy_widget = new QGraphicsProxyWidget(this);
    proxy_widget->setWidget(widget);
    proxy_widget->setParentItem(toLeft ? _m_leftFrame : _m_rightFrame);
    proxy_widget->setPos(x, -32);
    return proxy_widget;
}

QSanSkillButton *Dashboard::addSkillButton(const QString &skillName)
{
    // if it's a equip skill, add it to equip bar
    _mutexEquipAnim.lock();
    for (int i = 0; i < 4; i++)
    {
        if (!_m_equipCards[i]) continue;
        const EquipCard *equip = qobject_cast<const EquipCard *>(_m_equipCards[i]->getCard());
        Q_ASSERT(equip);
        // @todo: we must fix this in the server side - add a skill to the card itself instead
        // of getting it from the engine.
        const Skill* skill = Sanguosha->getSkill(equip->objectName());
        if (skill == NULL) skill = equip->getSkill();
        if (skill == NULL) continue;
        if (skill->objectName() == skillName)
        {
            // If there is already a button there, then we haven't removed the last skill before attaching
            // a new one. The server must have sent the requests out of order. So crash.
            Q_ASSERT(_m_equipSkillBtns[i] == NULL);
            _m_equipSkillBtns[i] = new QSanInvokeSkillButton();
            _m_equipSkillBtns[i]->setSkill(skill);
            connect(_m_equipSkillBtns[i], SIGNAL(clicked()), this, SLOT(_onEquipSelectChanged()));
            connect(_m_equipSkillBtns[i], SIGNAL(enable_changed()), this, SLOT(_onEquipSelectChanged()));
            QSanSkillButton* btn = _m_equipSkillBtns[i];
            _mutexEquipAnim.unlock();
            return btn;
        }
    }
    _mutexEquipAnim.unlock();
    const Skill* skill = Sanguosha->getSkill(skillName);
    Q_ASSERT(skill && !skill->inherits("WeaponSkill") && !skill->inherits("ArmorSkill"));
    if(_m_skillDock->getSkillButtonByName(skillName) != NULL) return NULL;
    return _m_skillDock->addSkillButtonByName(skillName);
}

QSanSkillButton* Dashboard::removeSkillButton(const QString &skillName)
{
    QSanSkillButton* btn = NULL;
    _mutexEquipAnim.lock();
    for (int i = 0; i < 4; i++)
    {
        if (!_m_equipSkillBtns[i]) continue;
        const Skill* skill = _m_equipSkillBtns[i]->getSkill();
        Q_ASSERT(skill != NULL);
        if (skill->objectName() == skillName)
        {
            btn = _m_equipSkillBtns[i];
            _m_equipSkillBtns[i] = NULL;
            continue;
        }
    }
    _mutexEquipAnim.unlock();
    if (btn == NULL) btn  = _m_skillDock->removeSkillButtonByName(skillName);
    //Q_ASSERT(btn != NULL);
    //Be care LordSkill and SPConvertSkill
    if (btn != NULL && getFilter() == btn->getSkill()){
        setFilter(NULL);
    }    
    return btn;
}

void Dashboard::highlightEquip(QString skillName, bool highlight)
{
    QSanSkillButton* btn = NULL;
    int i = 0;
    for (i = 0; i < 4; i++)
    {
        if (!_m_equipSkillBtns[i]) continue;
        const Skill* skill = _m_equipSkillBtns[i]->getSkill();
        Q_ASSERT(skill != NULL);
        if (skill->objectName() == skillName)
        {
            btn = _m_equipSkillBtns[i];
            break;
        }
    }
    if (btn != NULL)
    {
        _setEquipBorderAnimation(i, highlight);
    }
}

QPushButton *Dashboard::createButton(const QString &name){
    QPushButton *button = new QPushButton;
    button->setEnabled(false);

    // @todo: We put half of the skins in qss but half in json file. This is not acceptable
    // and must be moved.
    QPixmap icon_pixmap(QString("image/system/button/%1.png").arg(name));
    QPixmap icon_pixmap_disabled(QString("image/system/button/%1-disabled.png").arg(name));

    QIcon icon(icon_pixmap);
    icon.addPixmap(icon_pixmap_disabled, QIcon::Disabled);

    button->setIcon(icon);
    button->setIconSize(icon_pixmap.size());
    button->setFixedSize(icon_pixmap.size());
    // button->setObjectName(name);
    // button->setProperty("game_control",true);

    button->setAttribute(Qt::WA_TranslucentBackground);

    return button;
}

void Dashboard::skillButtonActivated(){
    QSanSkillButton *button = qobject_cast<QSanSkillButton *>(sender());
    foreach(QSanSkillButton *btn, _m_skillDock->getAllSkillButtons()){
        if(button == btn)
            continue;

        if(btn->getViewAsSkill() != NULL && btn->isDown())
            btn->setState(QSanButton::S_STATE_UP);
    }

    for(int i = 0; i < 4; i++)
    {
        if(button == _m_equipSkillBtns[i])
            continue;

        if(_m_equipSkillBtns[i] != NULL)
            _m_equipSkillBtns[i]->setEnabled(false);
    }
}

void Dashboard::skillButtonDeactivated(){
    foreach(QSanSkillButton *btn, _m_skillDock->getAllSkillButtons()){
        if(btn->getViewAsSkill() != NULL && btn->isDown())
            btn->setState(QSanButton::S_STATE_UP);
    }
    
    for(int i = 0; i < 4; i++)
    {
        if(_m_equipSkillBtns[i] != NULL){
            _m_equipSkillBtns[i]->setEnabled(true);
            if(_m_equipSkillBtns[i]->isDown())
                _m_equipSkillBtns[i]->click();
        }
    }
}

void Dashboard::selectAll(){
    if(view_as_skill){
        unselectAll();

        foreach(CardItem *card_item, m_handCards){
            selectCard(card_item, true);
            pendings << card_item;
        }
        updatePending();
    }
    adjustCards(true);
}

QPushButton *Dashboard::addButton(const QString &name, int x, bool from_left){
    QPushButton *button = createButton(name);
    addWidget(button, x, from_left);
    return button;
}

void Dashboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

void Dashboard::mousePressEvent(QGraphicsSceneMouseEvent *){
}

void Dashboard::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    PlayerCardContainer::mouseReleaseEvent(mouseEvent);

    CardItem *to_select = NULL;
    int i;
    for (i = 0; i < 4; i++) {
        if(_m_equipRegions[i]->isUnderMouse()){
            to_select = _m_equipCards[i];
            break;
        }
    }
    if (!to_select) return;
    if (_m_equipSkillBtns[i] != NULL
        && _m_equipSkillBtns[i]->isEnabled())
    {
        _m_equipSkillBtns[i]->click();        
    }
    else if (to_select->isMarkable()) {
        // According to the game rule, you cannot select a weapon as a card when
        // you are invoking the skill of that equip. So something must be wrong.
        // Crash.
        Q_ASSERT(_m_equipSkillBtns[i] == NULL || !_m_equipSkillBtns[i]->isDown());
        to_select->mark(!to_select->isMarked());
        update();
    }
}

void Dashboard::_onEquipSelectChanged()
{
    QSanSkillButton* btn = qobject_cast<QSanSkillButton*>(sender());
    if (btn)
    {
        for (int i = 0; i < 4; i++)
        {
            if (_m_equipSkillBtns[i] == btn)
            {
                _setEquipBorderAnimation(i, btn->isDown());
                break;
            }
        }
    }
    else
    {
        CardItem* equip = qobject_cast<CardItem*>(sender());
        // Do not remove this assertion. If equip is NULL here, some other
        // sources that could select equip has not been considered and must
        // be implemented.
        Q_ASSERT(equip);
        for (int i = 0; i < 4; i++)
        {
            if (_m_equipCards[i] == equip)
            {
                _setEquipBorderAnimation(i, equip->isMarked());
                break;
            }
        }
    }
}

void Dashboard::_createEquipBorderAnimations()
{
    for (int i = 0; i < 4; i++)
    {
        _m_equipBorders[i] = new PixmapAnimation();
        _m_equipBorders[i]->setParentItem(_getEquipParent());
        _m_equipBorders[i]->setPath("image/system/emotion/equipborder/");
        if (!_m_equipBorders[i]->valid())
        {
            delete _m_equipBorders[i];
            _m_equipBorders[i] = NULL;
            continue;
        }
        _m_equipBorders[i]->setPos(_dlayout->m_equipBorderPos +
                              _dlayout->m_equipSelectedOffset +
                            _dlayout->m_equipAreas[i].topLeft());
        _m_equipBorders[i]->hide();
    }
}

void Dashboard::_setEquipBorderAnimation(int index, bool turnOn)
{
    _mutexEquipAnim.lock();
    if (_m_isEquipsAnimOn[index] == turnOn) {
        _mutexEquipAnim.unlock();
        return;
    }
    
    QPoint newPos;

    if (turnOn)
    {
        newPos = _dlayout->m_equipSelectedOffset + _dlayout->m_equipAreas[index].topLeft();        
    }
    else
    {
        newPos = _dlayout->m_equipAreas[index].topLeft();
    }
    
    _m_equipAnim[index]->stop();
    _m_equipAnim[index]->clear();
    QPropertyAnimation* anim = new QPropertyAnimation(_m_equipRegions[index], "pos");
    anim->setEndValue(newPos);
    anim->setDuration(200);
    _m_equipAnim[index]->addAnimation(anim);
    anim = new QPropertyAnimation(_m_equipRegions[index], "opacity");
    anim->setEndValue(255);
    anim->setDuration(200);
    _m_equipAnim[index]->addAnimation(anim);
    _m_equipAnim[index]->start();
    
    Q_ASSERT(_m_equipBorders[index]);
    if (turnOn)
    {
        _m_equipBorders[index]->show();
        _m_equipBorders[index]->start();
    }
    else
    {
        _m_equipBorders[index]->hide();
        _m_equipBorders[index]->stop();
    }

    _m_isEquipsAnimOn[index] = turnOn;
    _mutexEquipAnim.unlock();
}

void Dashboard::adjustCards(bool playAnimation){
    _adjustCards();
    foreach (CardItem* card, m_handCards)
    {
        card->goBack(playAnimation);
    }        
}

void Dashboard::_adjustCards(){
    int maxCards = Config.MaxCards;

    int n = m_handCards.length();

    if (n == 0) return;

    if (maxCards >= n) maxCards = n;
    else if (maxCards <= (n - 1) / 2 + 1) maxCards = (n - 1) / 2 + 1;
    QList<CardItem*> row;
    QSanRoomSkin::DashboardLayout* layout = (QSanRoomSkin::DashboardLayout*)_m_layout;
    int leftWidth = layout->m_leftWidth;
    int cardHeight = G_COMMON_LAYOUT.m_cardNormalHeight;
    int middleWidth = _m_width - layout->m_leftWidth - layout->m_rightWidth - this->getButtonWidgetWidth();
    QRect rowRect = QRect(leftWidth, layout->m_normalHeight - cardHeight, middleWidth, cardHeight);
    for (int i = 0; i < maxCards; i++)
    {
        row.push_back(m_handCards[i]);
    }
    _m_highestZ = n;
    _disperseCards(row, rowRect, Qt::AlignLeft, true, true);
    
    row.clear();
    rowRect.translate(0, 1.5 * S_PENDING_OFFSET_Y);
    for (int i = maxCards; i < n; i++)
    {
        row.push_back(m_handCards[i]);
    }
    _m_highestZ = 0;
    _disperseCards(row, rowRect, Qt::AlignLeft, true, true); 

    for (int i = 0; i < n; i++)
    {
        CardItem* card = m_handCards[i];

        if (card->isSelected())
        {
            QPointF newPos = card->homePos();
            newPos.setY(newPos.y() + S_PENDING_OFFSET_Y);
            card->setHomePos(newPos);
        }
    }
}

int Dashboard::getMiddleWidth()
{
    return _m_width - G_DASHBOARD_LAYOUT.m_leftWidth - G_DASHBOARD_LAYOUT.m_rightWidth;
}

QList<CardItem*> Dashboard::cloneCardItems(QList<int> card_ids){
    QList<CardItem*> result;
    CardItem* card_item;
    CardItem* new_card;

    foreach (int card_id, card_ids)
    {
        card_item = CardItem::FindItem(m_handCards, card_id);
        new_card = _createCard(card_id);
        Q_ASSERT(card_item);
        if(card_item)
        {
            new_card->setPos(card_item->pos());
            new_card->setHomePos(card_item->homePos());
        }
        result.append(new_card);
    }
    return result;
}

QList<CardItem*> Dashboard::removeHandCards(const QList<int> &card_ids)
{
    QList<CardItem*> result;
    CardItem* card_item;
    foreach (int card_id, card_ids)
    {

        card_item = CardItem::FindItem(m_handCards, card_id);
        if (card_item == selected) selected = NULL;
        Q_ASSERT(card_item);
        if(card_item)
        {
            m_handCards.removeOne(card_item);
            card_item->hideFrame();
            card_item->disconnect(this);
            result.append(card_item);            
        }
    }
    updateHandcardNum();
    return result;
}

QList<CardItem*> Dashboard::removeCardItems(const QList<int> &card_ids, Player::Place place){
    CardItem *card_item = NULL;
    QList<CardItem*> result;
    if (place == Player::PlaceHand)
        result = removeHandCards(card_ids);
    else if(place == Player::PlaceEquip)
        result = removeEquips(card_ids);
    else if(place == Player::PlaceDelayedTrick)
        result = removeDelayedTricks(card_ids);
    else if (place == Player::PlaceSpecial)
    {
        foreach (int card_id, card_ids)
        {
            card_item = _createCard(card_id);
            card_item->setOpacity(0.0);
            result.push_back(card_item);
        }
    }
    else Q_ASSERT(false);

    Q_ASSERT(result.size() == card_ids.size());
    if (place == Player::PlaceHand)    
        adjustCards();   
    else if (result.size() > 1 || place == Player::PlaceSpecial)
    {
        QRect rect(0, 0, _dlayout->m_disperseWidth, 0);
        QPointF center(0, 0);
        if (place == Player::PlaceEquip || place == Player::PlaceDelayedTrick)
        {
            for (int i = 0; i < result.size(); i++)
            {
                center += result[i]->pos();
            }
            center = 1.0 / result.length() * center;
        }
        else if (place == Player::PlaceSpecial)
            center = mapFromItem(_getAvatarParent(), _dlayout->m_avatarArea.center());
        else
            Q_ASSERT(false);
        rect.moveCenter(center.toPoint());
        _disperseCards(result, rect, Qt::AlignCenter, false, false);
    }
    update();
    return result;
}

static bool CompareByType(const CardItem *a, const CardItem *b) 
{
    return Card::CompareByType(a->getCard(), b->getCard());
}

void Dashboard::sortCards(bool doAnimation){
    qSort(m_handCards.begin(), m_handCards.end(), CompareByType);    
    if (doAnimation)
        adjustCards();
}

void Dashboard::reverseSelection(){
    if(view_as_skill == NULL)
        return;

     m_mutexEnableCards.lock();
    QSet<CardItem *> selected_set = pendings.toSet();
    unselectAll();

    foreach(CardItem *item, m_handCards)
        item->setEnabled(false);

    pendings.clear();

    foreach(CardItem *item, m_handCards){
        if(view_as_skill->viewFilter(pendings, item) && !selected_set.contains(item)){
            selectCard(item, false);
            pendings << item;

            item->setEnabled(true);
        }
    }

    if(pending_card && pending_card->isVirtualCard() && pending_card->parent() == NULL)
        delete pending_card;
    pending_card = view_as_skill->viewAs(pendings);
    m_mutexEnableCards.unlock();
    emit card_selected(pending_card);
}

void Dashboard::disableAllCards(){
    m_mutexEnableCards.lock();
    foreach(CardItem *card_item, m_handCards){
        card_item->setEnabled(false);
    }
    m_mutexEnableCards.unlock();
}

void Dashboard::enableCards(){
    m_mutexEnableCards.lock();
    foreach(CardItem *card_item, m_handCards){
        card_item->setEnabled(card_item->getFilteredCard()->isAvailable(Self));
    }
    m_mutexEnableCards.unlock();
}

void Dashboard::enableAllCards(){
    m_mutexEnableCards.lock();
    foreach(CardItem *card_item, m_handCards)
        card_item->setEnabled(true);
    m_mutexEnableCards.unlock();
}

void Dashboard::startPending(const ViewAsSkill *skill){
    m_mutexEnableCards.lock();
    view_as_skill = skill;
    pendings.clear();
    unselectAll();

    for (int i = 0; i < 4; i++) {
        if (_m_equipCards[i] != NULL)
            connect(_m_equipCards[i], SIGNAL(mark_changed()), this, SLOT(onMarkChanged()));
    }

    updatePending();
    // adjustCards(false);
    m_mutexEnableCards.unlock();
}

void Dashboard::stopPending(){
    m_mutexEnableCards.lock();
    view_as_skill = NULL;
    pending_card = NULL;
    emit card_selected(NULL);

    foreach (CardItem* item, m_handCards)
    {
        item->setEnabled(false);
    }

    for (int i = 0; i < 4; i++) {
        CardItem* equip = _m_equipCards[i];
        if (equip != NULL) {
            equip->mark(false);
            equip->setMarkable(false);
            equip->setEnabled(false);
            disconnect(equip, SIGNAL(mark_changed()));
        }
    }
    pendings.clear();
    adjustCards(true);
    m_mutexEnableCards.unlock();
}

void Dashboard::onCardItemClicked(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(!card_item)
        return;

    if(view_as_skill){
        if(card_item->isSelected()){
            selectCard(card_item, false);
            pendings.removeOne(card_item);
        }else{
            selectCard(card_item, true);
            pendings << card_item;
        }

        updatePending();

    }else{
        if(card_item->isSelected()){
            unselectAll();
            emit card_selected(NULL);
        }else{
            unselectAll();
            selectCard(card_item, true);          
            selected = card_item;

            emit card_selected(selected->getFilteredCard());
        }
    }
}

void Dashboard::updatePending(){
    if (!view_as_skill) return;
    foreach(CardItem *c, m_handCards){
        if(!c->isSelected() || pendings.isEmpty()){
            c->setEnabled(view_as_skill->viewFilter(pendings, c));
        }
    }

    for (int i = 0; i < 4; i++){
        CardItem *equip = _m_equipCards[i];
        if(equip && !equip->isMarked())
            equip->setMarkable(view_as_skill->viewFilter(pendings, equip));
    }

    const Card *new_pending_card = view_as_skill->viewAs(pendings);
    if(pending_card != new_pending_card){
        pending_card = new_pending_card;
        emit card_selected(pending_card);
    }
}


void Dashboard::onCardItemThrown(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());
    if(card_item){
        if(!view_as_skill)
            selected = card_item;
        emit card_to_use();
    }
}

void Dashboard::onCardItemHover()
{
    QGraphicsItem *card_item = qobject_cast<QGraphicsItem *>(sender());
    if (!card_item) return;

    animations->emphasize(card_item);
}

void Dashboard::onCardItemLeaveHover()
{
    QGraphicsItem *card_item = qobject_cast<QGraphicsItem *>(sender());
    if (!card_item) return;

    animations->effectOut(card_item);
}

void Dashboard::onMarkChanged(){
    CardItem *card_item = qobject_cast<CardItem *>(sender());

    Q_ASSERT(card_item->isEquipped());

    if(card_item){
        if(card_item->isMarked()){
            if(!pendings.contains(card_item))
                pendings.append(card_item);
        }else
            pendings.removeOne(card_item);

        updatePending();
    }
}

const ViewAsSkill *Dashboard::currentSkill() const{
    return view_as_skill;
}

const Card *Dashboard::pendingCard() const{
    return pending_card;
}
