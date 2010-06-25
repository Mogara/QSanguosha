#include "dashboard.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QComboBox>

Dashboard::Dashboard()
    :Pixmap(":/images/dashboard.png"), general(NULL), avatar(NULL), use_skill(false)
{
    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/images/magatamas/%1.png").arg(i+1));
    }

    QComboBox *sort_type = new QComboBox;
    sort_type->addItem(tr("No sort"));
    sort_type->addItem(tr("Sort by suit"));
    sort_type->addItem(tr("Sort by type"));
    sort_type->move(0, 32);
    QGraphicsProxyWidget *sort_widget = new QGraphicsProxyWidget(this);
    sort_widget->setWidget(sort_type);
    connect(sort_type, SIGNAL(currentIndexChanged(int)), this, SLOT(sortCards(int)));
}

void Dashboard::addCard(Card *card){
    card->setParentItem(this);
    card->setParent(this);
    cards << card;

    adjustCards();
}

void Dashboard::setGeneral(General *general){
    this->general = general;
    avatar = new Pixmap("generals/big/" + general->objectName() + ".png");
    avatar->setPos(837, 35);
    avatar->setFlag(ItemIsSelectable);
    avatar->setParent(this);
    avatar->setParentItem(this);
}

Pixmap *Dashboard::getAvatar(){
    return avatar;
}

void Dashboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    // draw general's hp
    if(general){
        int hp = general->getHp();
        QPixmap *magatama = &magatamas[hp-1];
        int i;
        for(i=0; i<hp; i++)
            painter->drawPixmap(985, 24 + i*(magatama->height()+4), *magatama);
    }
}

void Dashboard::adjustCards(){
    int n = cards.size();
    if(n == 0)
        return;

    int card_width = cards.front()->boundingRect().width();
    int card_skip;
    if(n > 5)
        card_skip = (530 - n * card_width)/n + card_width;
    else
        card_skip = card_width;

    int i;
    for(i=0; i<n; i++){
        cards[i]->setZValue(0.1 * i);
        cards[i]->setHomePos(QPointF(180 + i*card_skip, 45));
        cards[i]->goBack();
    }
}

void Dashboard::sortCards(int sort_type){
    if(sort_type == 0)
        return;

    if(sort_type == 1)
        qSort(cards.begin(), cards.end(), Card::CompareBySuitNumber);
    else
        qSort(cards.begin(), cards.end(), Card::CompareByType);

    adjustCards();
}
