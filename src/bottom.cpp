#include "bottom.h"

#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QComboBox>

Bottom::Bottom():Pixmap(":/images/bottom.png"), use_skill(false)
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

    Card *card1 = new Card("savage_assault", Card::Spade, 1);
    Card *card2 = new Card("slash", Card::Club, 7);
    Card *card3 = new Card("jink", Card::Heart, 2);
    Card *card4 = new Card("peach", Card::Diamond, 10);
    Card *card5 = new Card("archery_attack", Card::Heart, 11);
    Card *card6 = new Card("crossbow", Card::Club, 12);

    addCard(card1);
    addCard(card2);
    addCard(card3);
    addCard(card4);
    addCard(card5);
    addCard(card6);

    card4->setEnabled(false);

    avatar.load("generals/big/zhangliao.png");

    general = new General("caocao", "wei", 4, true);
}

void Bottom::addCard(Card *card){
    card->setParentItem(this);
    card->setParent(this);
    cards << card;

    adjustCards();
}

void Bottom::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    Pixmap::paint(painter, option, widget);

    // draw hp
    int hp = general->getHp();
    QPixmap *magatama = &magatamas[hp-1];
    int i;
    for(i=0; i<hp; i++)
        painter->drawPixmap(985, 24 + i*(magatama->height()+4), *magatama);

    // draw general's avatar
    painter->drawPixmap(837, 35, avatar);
}

void Bottom::adjustCards(){
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

void Bottom::sortCards(int sort_type){
    if(sort_type == 0)
        return;

    if(sort_type == 1)
        qSort(cards.begin(), cards.end(), Card::CompareBySuitNumber);
    else
        qSort(cards.begin(), cards.end(), Card::CompareByType);

    adjustCards();
}
