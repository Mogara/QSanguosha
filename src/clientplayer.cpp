#include "clientplayer.h"
#include "skill.h"
#include "client.h"
#include "engine.h"
#include "standard.h"

#include <QTextDocument>
#include <QTextOption>

ClientPlayer *Self = NULL;

ClientPlayer::ClientPlayer(Client *client)
    :Player(client), handcard_num(0)
{
    mark_doc = new QTextDocument(this);
    mark_doc->setTextWidth(128);
    mark_doc->setDefaultTextOption(QTextOption(Qt::AlignRight));
}

void ClientPlayer::handCardChange(int delta){
    handcard_num += delta;
}

int ClientPlayer::aliveCount() const{
    return ClientInstance->alivePlayerCount();
}

int ClientPlayer::getHandcardNum() const{
    return handcard_num;
}

void ClientPlayer::addCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            if(card)
                known_cards << card;
            handcard_num++;
            break;
        }
    case Equip: {
            const EquipCard *equip = qobject_cast<const EquipCard*>(card);
            setEquip(equip);
            break;
        }
    case Judging:{
            addDelayedTrick(card);
            break;
        }
    default:
        // FIXME
        ;
    }
}

void ClientPlayer::addKnownHandCard(const Card *card){
    if(!known_cards.contains(card))
        known_cards << card;
}

bool ClientPlayer::isLastHandCard(const Card *card) const{
    if(known_cards.length() != 1)
        return false;

    if(!card->isVirtualCard()){
        return known_cards.first() == card;
    }else{
        QList<int> subcards = card->getSubcards();
        return subcards.length() == 1 && subcards.first() == known_cards.first()->getId();
    }
}

void ClientPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            handcard_num--;
            if(card)
                known_cards.removeOne(card);
            break;
        }
    case Equip:{
            const EquipCard *equip = qobject_cast<const EquipCard*>(card);
            removeEquip(equip);
            break;
        }
    case Judging:{
            removeDelayedTrick(card);
            break;
        }

    default:
        // FIXME
        ;
    }
}

QList<const Card *> ClientPlayer::getCards() const{
    return known_cards;
}

void ClientPlayer::setCards(const QList<int> &card_ids){
    known_cards.clear();

    foreach(int card_id, card_ids){
        known_cards << Sanguosha->getCard(card_id);
    }
}

QTextDocument *ClientPlayer::getMarkDoc() const{
    return mark_doc;
}

void ClientPlayer::changePile(const QString &name, bool add, int card_id){
    QList<int> &pile = getPile(name);
    if(add)
        pile.append(card_id);
    else
        pile.removeOne(card_id);

    emit pile_changed(name);
}

void ClientPlayer::setMark(const QString &mark, int value){
    if(marks[mark] == value)
        return;

    marks[mark] = value;

    if(!mark.startsWith("@"))
        return;

    // set mark doc
    QString text = "";
    QMapIterator<QString, int> itor(marks);
    while(itor.hasNext()){
        itor.next();

        if(itor.key().startsWith("@") && itor.value() > 0){
            QString mark_text = QString("<img src='image/mark/%1.png' />").arg(itor.key());
            if(itor.value() != 1)
                mark_text.append(QString("x%1").arg(itor.value()));
            text.append(mark_text);
        }
    }

    mark_doc->setHtml(text);
}
