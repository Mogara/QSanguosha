#include "dashboard.h"
#include "engine.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

Dashboard::Dashboard()
    :Pixmap(":/images/dashboard.png"), player(NULL), avatar(NULL), kingdom(NULL), use_skill(false)
{
    int i;
    for(i=0; i<5; i++){
        magatamas[i].load(QString(":/images/magatamas/%1.png").arg(i+1));
    }

    sort_combobox = new QComboBox;
    sort_combobox->addItem(tr("No sort"));
    sort_combobox->addItem(tr("Sort by suit"));
    sort_combobox->addItem(tr("Sort by type"));
    sort_combobox->move(0, 32);
    QGraphicsProxyWidget *sort_widget = new QGraphicsProxyWidget(this);
    sort_widget->setWidget(sort_combobox);
    connect(sort_combobox, SIGNAL(currentIndexChanged(int)), this, SLOT(sortCards()));
}

void Dashboard::addCardItem(CardItem *card_item){
    card_item->setParentItem(this);
    card_item->setParent(this);
    card_items << card_item;

    adjustCards();
}

void Dashboard::setPlayer(Player *player){
    this->player = player;
    const General *general = player->getGeneral();
    if(general == NULL)
        general = Sanguosha->getGeneral(player->property("avatar").toString());

    QString filename = general->getPixmapPath("big");
    if(avatar)
        avatar->changePixmap(filename);
    else
        avatar = new Pixmap(filename);

    avatar->setPos(837, 35);
    avatar->setFlag(ItemIsSelectable);
    avatar->setParent(this);
    avatar->setParentItem(this);

    if(kingdom)
        kingdom->changePixmap(general->getKingdomPath());
    else{
        kingdom = new Pixmap(general->getKingdomPath());
        kingdom->setParent(this);
        kingdom->setParentItem(this);
        kingdom->setPos(avatar->pos());
    }
}

Pixmap *Dashboard::getAvatar(){
    return avatar;
}

void Dashboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    // draw general's hp
    if(player){
        int hp = player->getHp();
        QPixmap *magatama = &magatamas[hp-1];
        int i;
        for(i=0; i<hp; i++)
            painter->drawPixmap(985, 24 + i*(magatama->height()+4), *magatama);
    }
}

void Dashboard::adjustCards(){
    int n = card_items.size();
    if(n == 0)
        return;

    int card_width = card_items.front()->boundingRect().width();
    int card_skip;
    if(n > 5)
        card_skip = (530 - n * card_width)/n + card_width;
    else
        card_skip = card_width;

    int i;
    for(i=0; i<n; i++){
        card_items[i]->setZValue(0.1 * i);
        card_items[i]->setHomePos(QPointF(180 + i*card_skip, 45));
        card_items[i]->goBack();
    }
}

static bool CompareBySuitNumber(const CardItem *a, const CardItem *b){
    return Card::CompareBySuitNumber(a->getCard(), b->getCard());
}

static bool CompareByType(const CardItem *a, const CardItem *b){
    return Card::CompareByType(a->getCard(), b->getCard());
}

void Dashboard::sortCards(){
    int sort_type = sort_combobox->currentIndex();
    switch(sort_type){
    case 0: return;
    case 1: qSort(card_items.begin(), card_items.end(), CompareBySuitNumber); break;
    case 2: qSort(card_items.begin(), card_items.end(), CompareByType); break;
    }

    adjustCards();
}
