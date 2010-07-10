#include "photo.h"
#include "clientplayer.h"
#include "settings.h"
#include "carditem.h"
#include "engine.h"

#include <QPainter>
#include <QMimeData>
#include <QDrag>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMessageBox>
#include <QGraphicsProxyWidget>

Photo::Photo()
    :Pixmap(":/images/photo-back.png"),
    player(NULL),
    avatar_frame(":/images/avatar-frame.png"),
    handcard(":/images/handcard.png")
{
    setAcceptHoverEvents(true);
    setFlags(ItemIsSelectable);

    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/images/magatamas/%1.png").arg(i+1));
        magatamas[i] = magatamas[i].scaled(20,20);
    }

    role_combobox = new QComboBox;
    role_combobox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    role_combobox->addItem(tr("unknown"));    

    QGraphicsProxyWidget *widget = new QGraphicsProxyWidget(this);
    widget->setWidget(role_combobox);
    widget->setPos(pixmap.width()/2, pixmap.height()-10);
}

void Photo::setPlayer(const ClientPlayer *player)
{
    this->player = player;

    if(player){        
        role_combobox->addItem(QIcon(":/images/roles/loyalist.png"), tr("loyalist"));
        role_combobox->addItem(QIcon(":/images/roles/rebel.png"), tr("rebel"));
        role_combobox->addItem(QIcon(":/images/roles/renegade.png"), tr("renegade"));

        connect(player, SIGNAL(role_changed(QString)), this, SLOT(updateRoleCombobox(QString)));
        connect(player, SIGNAL(state_changed(QString)), this, SLOT(updateStateStr(QString)));
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));

    }

    updateAvatar();
}

void Photo::updateAvatar(){
    if(player){
        const General *general = player->getAvatarGeneral();
        avatar.load(general->getPixmapPath("small"));
        avatar = avatar.scaled(QSize(128,58));
        kingdom.load(general->getKingdomPath());
    }else{
        avatar.detach();
        kingdom.detach();
    }

    update();
}

void Photo::updateStateStr(const QString &new_state){
    state_str = Sanguosha->translate(new_state);
}

void Photo::updateRoleCombobox(const QString &new_role){
    role_combobox->clear();
    QIcon icon(QString(":/images/roles/%1.png").arg(new_role));
    QString caption = Sanguosha->translate(new_role);
    role_combobox->addItem(icon, caption);
    role_combobox->setEnabled(false);
}

const ClientPlayer *Photo::getPlayer() const{
    return player;
}

void Photo::speak(const QString &content)
{

}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);
    if(player){
        painter->drawPixmap(1, 13, avatar);
        painter->drawPixmap(0, 10, avatar_frame);
        painter->drawPixmap(0,  0, kingdom);

        painter->setPen(Qt::white);
        painter->drawText(QRectF(0,0,132,19), player->objectName(), QTextOption(Qt::AlignHCenter));

        // draw magatamas, similar with dashboard, but magatama is smaller
        int hp = player->getHp();
        if(hp > 0){
            QPixmap *magatama;
            if(player->isWounded())
                magatama = &magatamas[hp-1];
            else
                magatama = &magatamas[4]; // the green magatama which denote full blood state
            int i;
            for(i=0; i<hp; i++)
                painter->drawPixmap(34 + i*(magatama->width()+2), 78, *magatama);
        }
        int n = player->getHandcardNum();
        if(n > 0){
            painter->drawPixmap(0, 72, handcard);
            painter->drawText(8, 95, QString::number(n));
        }

        if(!state_str.isEmpty()){
            painter->drawText(100, 100, state_str);
        }

        if(!player->getPhase().isEmpty()){
            painter->drawRoundRect(boundingRect(), 10, 15);
        }
    }
}

void Photo::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    QGraphicsObject *obj = static_cast<QGraphicsObject*>(scene()->focusItem());
    CardItem *card_item = qobject_cast<CardItem*>(obj);
    if(card_item && card_item->isUnderMouse()){
        QMessageBox::information(NULL, "", card_item->getCard()->objectName());
    }
}


