#include "photo.h"
#include "clientplayer.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"
#include "standard.h"
#include "client.h"
#include "magatamawidget.h"
#include "rolecombobox.h"

#include <QPainter>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMessageBox>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QPropertyAnimation>

Photo::Photo(int order)
    :Pixmap(":/photo-back.png"),
    player(NULL),
    handcard(":/handcard.png"),
    chain(":/chain.png"),
    weapon(NULL), armor(NULL), defensive_horse(NULL), offensive_horse(NULL),
    order_item(new QGraphicsPixmapItem(QPixmap(QString(":/number/%1.png").arg(order+1)),this)),
    hide_avatar(false)
{
    setAcceptHoverEvents(true);

    order_item->setVisible(false);

    back_icon = new Pixmap(":/small-back.png");
    back_icon->setParentItem(this);
    back_icon->setPos(105, 67);
    back_icon->hide();
    back_icon->setZValue(1.0);

    progress_bar = new QProgressBar;
    progress_bar->setOrientation(Qt::Vertical);
    progress_bar->setMinimum(0);
    progress_bar->setMaximum(100);
    progress_bar->setValue(0);
    progress_bar->hide();
    progress_bar->setMaximumWidth(10);
    progress_bar->setMaximumHeight(pixmap.height());
    timer_id = 0;

    frame_item = new QGraphicsPixmapItem(this);
    frame_item->setPos(-6, -6);

    QGraphicsProxyWidget *widget = new QGraphicsProxyWidget(this);
    widget->setWidget(progress_bar);
    widget->setPos(pixmap.width() - 15, 0);

    skill_name_item = new QGraphicsSimpleTextItem(this);
    skill_name_item->setBrush(Qt::white);
    skill_name_item->setFont(Config.SmallFont);
    skill_name_item->moveBy(10, 30);

    emotion_item = new QGraphicsPixmapItem(this);
    emotion_item->moveBy(100, 0);

    avatar_area = new QGraphicsRectItem(0, 0, 122, 50, this);
    avatar_area->setPos(5, 15);
    avatar_area->setPen(Qt::NoPen);

    small_avatar_area = new QGraphicsRectItem(0, 0, 42, 36, this);
    small_avatar_area->setPos(86, 30);
    small_avatar_area->setPen(Qt::NoPen);

    equips << &weapon << &armor << &defensive_horse << &offensive_horse;
    int i;
    for(i=0; i<4; i++){
        equip_rects[i] = new QGraphicsRectItem(QRect(1, 118 + 17 * i, 129, 16), this);
        equip_rects[i]->setPen(Qt::NoPen);
    }

    kingdom_item = new QGraphicsPixmapItem(this);
    kingdom_item->setPos(-12, -6);

    mark_item = new QGraphicsTextItem(this);
    mark_item->setPos(2, 99);
    mark_item->setDefaultTextColor(Qt::white);

    role_combobox = NULL;
}

void Photo::createRoleCombobox(){
    role_combobox = new RoleCombobox(this);

    QString role = player->getRole();
    if(!role.isEmpty())
        role_combobox->fix(role);

    connect(player, SIGNAL(role_changed(QString)), role_combobox, SLOT(fix(QString)));
}

void Photo::showProcessBar(){
    progress_bar->setValue(0);
    progress_bar->show();

    if(ServerInfo.OperationTimeout != 0);
        timer_id = startTimer(500);
}

void Photo::hideProcessBar(){
    progress_bar->setValue(0);
    progress_bar->hide();

    if(timer_id != 0){
        killTimer(timer_id);
        timer_id = 0;
    }
}

void Photo::setEmotion(const QString &emotion, bool permanent){
    QString path = QString(":/emotion/%1.png").arg(emotion);
    emotion_item->setPixmap(QPixmap(path));
    emotion_item->show();

    if(!permanent)
        QTimer::singleShot(2000, this, SLOT(hideEmotion()));
}

void Photo::tremble(){
    QPropertyAnimation *vibrate = new QPropertyAnimation(this, "x");
    static qreal offset = 20;

    vibrate->setKeyValueAt(0.5, x() - offset);
    vibrate->setEndValue(x());

    vibrate->setEasingCurve(QEasingCurve::OutInBounce);

    vibrate->start(QAbstractAnimation::DeleteWhenStopped);
}

void Photo::showSkillName(const QString &skill_name){
    skill_name_item->setText(Sanguosha->translate(skill_name));
    skill_name_item->show();

    QTimer::singleShot(1500, this, SLOT(hideSkillName()));
}

void Photo::hideSkillName(){
    skill_name_item->hide();
}

void Photo::hideEmotion(){
    emotion_item->hide();
}

void Photo::timerEvent(QTimerEvent *event){
    int step = 100 / double(ServerInfo.OperationTimeout * 5);
    int new_value = progress_bar->value() + step;
    new_value = qMin(progress_bar->maximum(), new_value);
    progress_bar->setValue(new_value);

    if(new_value == progress_bar->maximum()){
        killTimer(event->timerId());
        timer_id = 0;
    }
}

void Photo::setPlayer(const ClientPlayer *player)
{
    this->player = player;

    if(player){
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));
        connect(player, SIGNAL(general2_changed()), this, SLOT(updateSmallAvatar()));
        connect(player, SIGNAL(kingdom_changed()), this, SLOT(updateAvatar()));
        connect(player, SIGNAL(state_changed()), this, SLOT(refresh()));
        connect(player, SIGNAL(phase_changed()), this, SLOT(updatePhase()));

        mark_item->setDocument(player->getMarkDoc());
    }

    updateAvatar();
}

void Photo::hideAvatar(){
    hide_avatar = true;
    kingdom_item->hide();

    update();
}

void Photo::showCard(int card_id){
    const Card *card = Sanguosha->getCard(card_id);

    CardItem *card_item = new CardItem(card);
    scene()->addItem(card_item);

    QPointF card_pos(pos() + QPointF(0, 20));
    card_item->setPos(card_pos);
    card_item->setHomePos(card_pos);
    card_item->setEnabled(false);

    QTimer::singleShot(2000, card_item, SLOT(deleteLater()));
}

void Photo::updateAvatar(){
    if(player){
        const General *general = player->getAvatarGeneral();
        avatar_area->setToolTip(general->getSkillDescription());
        avatar.load(general->getPixmapPath("small"));
        QPixmap kingdom_icon(player->getKingdomIcon());
        kingdom_item->setPixmap(kingdom_icon);
        kingdom_frame.load(player->getKingdomFrame());
    }else{
        avatar = QPixmap();
        kingdom_frame = QPixmap();

        avatar_area->setToolTip(QString());
        small_avatar_area->setToolTip(QString());
    }

    hide_avatar = false;
    kingdom_item->show();

    update();
}

void Photo::updateSmallAvatar(){
    const General *general2 = player->getGeneral2();
    if(general2){
        small_avatar.load(general2->getPixmapPath("tiny"));
        small_avatar_area->setToolTip(general2->getSkillDescription());
    }

    hide_avatar = false;
    update();
}

void Photo::refresh(){
    if(player && player->getHp() == 0)
        setFrame(SOS);
    else
        updatePhase();

    update();
}

const ClientPlayer *Photo::getPlayer() const{
    return player;
}

void Photo::speak(const QString &content)
{

}

CardItem *Photo::takeCardItem(int card_id, Player::Place place){
    CardItem *card_item = NULL;

    if(place == Player::Hand || place == Player::Special){
        card_item = new CardItem(Sanguosha->getCard(card_id));
        card_item->setPos(pos());
        card_item->shift();
    }else if(place == Player::Equip){
        foreach(CardItem **equip_ptr, equips){
            CardItem *equip = *equip_ptr;
            if(equip && equip->getCard()->getId() == card_id){
                card_item = equip;
                *equip_ptr = NULL;

                int index = equips.indexOf(equip_ptr);
                equip_rects[index]->setToolTip(QString());
                break;
            }
        }
    }else if(place == Player::Judging){
        QMutableVectorIterator<CardItem *> itor(judging_area);
        while(itor.hasNext()){
            CardItem *item = itor.next();
            if(item->getCard()->getId() == card_id){
                card_item = item;

                int index = judging_area.indexOf(item);
                QGraphicsPixmapItem *pixmap_item = judging_pixmaps.at(index);
                judging_pixmaps.remove(index);
                delete pixmap_item;
                itor.remove();

                break;
            }
        }
    }

    update();
    return card_item;
}

void Photo::installEquip(CardItem *equip){
    const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard());
    int index = -1;
    switch(equip_card->location()){
    case EquipCard::WeaponLocation: weapon = equip; index = 0; break;
    case EquipCard::ArmorLocation: armor = equip; index = 1; break;
    case EquipCard::DefensiveHorseLocation: defensive_horse = equip; index = 2; break;
    case EquipCard::OffensiveHorseLocation: offensive_horse = equip; index = 3; break;
    }

    if(index >= 0)
        equip_rects[index]->setToolTip(equip_card->getDescription());

    equip->setHomePos(pos());
    equip->goBack(true);

    update();
}

void Photo::installDelayedTrick(CardItem *trick){
    trick->setHomePos(pos());
    trick->goBack(true);

    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(this);
    item->setPixmap(QPixmap(player->topDelayedTrick()->getIconPath()));

    item->setPos(-10, 16 + judging_area.count() * 19);
    judging_area.push(trick);
    judging_pixmaps.push(item);
}

void Photo::addCardItem(CardItem *card_item){
    card_item->setHomePos(pos());
    card_item->goBack(true);

    update();
}

void Photo::drawMagatama(QPainter *painter, int index, const QPixmap &pixmap){
    static const QPoint first_row(42, 69);
    static const QPoint second_row(26, 86);
    static const int skip =  16;

    // index is count from 0
    if(index >= 5){
        // draw magatama at first row
        QPoint pos = first_row;
        pos.rx() += (index - 5) * skip;
        painter->drawPixmap(pos, pixmap);
    }else{
        // draw magatama at second row
        QPoint pos = second_row;
        pos.rx() += index * skip;
        painter->drawPixmap(pos, pixmap);
    }
}



void Photo::drawHp(QPainter *painter){
    int hp = player->getHp();
    if(hp <= 0)
        return;

    int index = 5;
    if(player->isWounded())
        index = qMin(hp, 5);

    QPixmap *magatama = MagatamaWidget::GetSmallMagatama(index);
    QPixmap *zero_magatama = MagatamaWidget::GetSmallMagatama(0);

    int max_hp = player->getMaxHP();
    int i;
    for(i=0; i< hp; i++)
        drawMagatama(painter, i, *magatama);
    for(i=hp; i< max_hp; i++)
        drawMagatama(painter, i, *zero_magatama);

    QString text = QString("%1/%2").arg(hp).arg(max_hp);
    painter->drawText(25, 80, text);
}

void Photo::setFrame(FrameType type){
    static QPixmap playing_frame(":/frame/playing.png");
    static QPixmap responsing_frame(":/frame/responsing.png");
    static QPixmap sos_frame(":/frame/sos.png");

    QPixmap *to_draw = NULL;
    switch(type){
    case Playing: to_draw = &playing_frame; break;
    case Responsing: to_draw = &responsing_frame; break;
    case SOS: to_draw = &sos_frame; break;
    default:
        break;
    }

    if(to_draw){
        frame_item->setPixmap(*to_draw);
        frame_item->show();
    }else{
        frame_item->hide();
    }

    update();
}

void Photo::updatePhase(){
    if(player->getPhase() != Player::NotActive)
        setFrame(Playing);
    else
        setFrame(NoFrame);
}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    if(!player)
        return;

    painter->setPen(Qt::white);
    QString title = player->screenName();
    painter->drawText(QRectF(0,0,132,19), title, QTextOption(Qt::AlignHCenter));

    static QPixmap wait_frame(":/wait-frame.png");
    if(kingdom_frame.isNull())
        painter->drawPixmap(3, 13, wait_frame);

    if(hide_avatar)
        return;

    // avatar related
    painter->drawPixmap(5, 15, avatar);
    painter->drawPixmap(86, 30, small_avatar);

    // kingdom related
    painter->drawPixmap(3, 13, kingdom_frame);

    if(!player->isAlive()){
        if(death_pixmap.isNull()){
            death_pixmap.load(QString(":/death/%1.png").arg(player->getRole()));
            death_pixmap = death_pixmap.scaled(death_pixmap.size() / (1.5));
        }

        painter->drawPixmap(5, 30, death_pixmap);
        return;
    }

    int n = player->getHandcardNum();
    if(n > 0){
        painter->drawPixmap(2, 68, handcard);
        painter->drawText(8, 86, QString::number(n));
    }

    QString state_str = player->getState();
    if(!state_str.isEmpty() && state_str != "online"){
        painter->drawText(1, 100, Sanguosha->translate(state_str));
    }

    drawHp(painter);

    if(player->getPhase() != Player::NotActive){
        static QList<QPixmap> phase_pixmaps;
        if(phase_pixmaps.isEmpty()){
            QStringList names;
            names << "start" << "judge" << "draw"
                    << "play" << "discard" << "finish";

            foreach(QString name, names)
                phase_pixmaps << QPixmap(QString(":/phase/%1.png").arg(name));
        }

        int index = static_cast<int>(player->getPhase());
        QPixmap phase_pixmap = phase_pixmaps.at(index);
        painter->drawPixmap(115, 120, phase_pixmap);
    }

    drawEquip(painter, weapon, 0);
    drawEquip(painter, armor, 1);
    drawEquip(painter, defensive_horse, 2);
    drawEquip(painter, offensive_horse, 3);

    // draw iron chain
    if(player->isChained())
        painter->drawPixmap(28, 16, chain);

    back_icon->setVisible(! player->faceUp());
}

void Photo::drawEquip(QPainter *painter, CardItem *equip, int order){
    if(!equip)
        return;

    QRect suit_rect(2, 104 + 15 + order * 17, 13, 13);
    painter->drawPixmap(suit_rect, equip->getSuitPixmap());

    const EquipCard *card = qobject_cast<const EquipCard *>(equip->getCard());
    painter->setPen(Qt::black);
    QFont bold_font;
    bold_font.setBold(true);
    painter->setFont(bold_font);
    painter->drawText(20, 115 + 15 + order * 17, card->getNumberString());
    painter->drawText(35, 115 + 15 + order * 17, card->label());
}

QVariant Photo::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change == ItemFlagsHaveChanged)
        order_item->setVisible(flags() & ItemIsSelectable);

    return Pixmap::itemChange(change, value);
}
