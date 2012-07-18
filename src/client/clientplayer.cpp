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
    case PlaceHand: {
            if(card)
                known_cards << card;
            handcard_num++;
            break;
        }
    case PlaceEquip: {
            WrappedCard *equip = Sanguosha->getWrappedCard(card->getEffectiveId());
            setEquip(equip);
            break;
        }
    case PlaceDelayedTrick:{
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
    if (!card->isVirtualCard()){
        if(known_cards.length() != 1)
            return false;
        return known_cards.first()->getEffectiveId() == card->getEffectiveId();
    }
    else if (card->getSubcards().length() > 0){
        foreach (int card_id, card->getSubcards()){
            if (!known_cards.contains(Sanguosha->getCard(card_id)))
                    return false;
        }
        return known_cards.length() == card->getSubcards().length();
    }
    return false;
}

void ClientPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case PlaceHand: {
            handcard_num--;
            if(card)
                known_cards.removeOne(card);
            break;
        }
    case PlaceEquip:{
            WrappedCard *equip = Sanguosha->getWrappedCard(card->getEffectiveId());
            removeEquip(equip);
            break;
        }
    case PlaceDelayedTrick:{
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

void ClientPlayer::changePile(const QString &name, bool add, QList<int> card_ids){
    if(add)
        piles[name].append(card_ids);
    else
        foreach (int card_id, card_ids){
            //todo: Fix it !!!
            if(piles[name].contains(Card::S_UNKNOWN_CARD_ID) && !piles[name].contains(card_id))
                piles[name].removeOne(Card::S_UNKNOWN_CARD_ID);
            else piles[name].removeOne(card_id);
        }

    if(!name.startsWith("#"))
        emit pile_changed(name);
}

QString ClientPlayer::getDeathPixmapPath() const{
    QString basename;
    if(ServerInfo.GameMode == "06_3v3"){
        if(getRole() == "lord" || getRole() == "renegade")
            basename = "marshal";
        else
            basename = "guard";
    }else if(ServerInfo.EnableHegemony){
        basename.clear();
    }else
        basename = getRole();

    if(basename.isEmpty()){
        basename = "unknown";
    }

    return QString("image/system/death/%1.png").arg(basename);
}

void ClientPlayer::setHandcardNum(int n){
    handcard_num = n;
}

QString ClientPlayer::getGameMode() const{
    return ServerInfo.GameMode;
}

void ClientPlayer::setFlags(const QString &flag){
    Player::setFlags(flag);

    if(flag.endsWith("drank"))
        emit drank_changed();
    else if(flag.endsWith("actioned"))
        emit action_taken();
}

void ClientPlayer::setMark(const QString &mark, int value){
    if(marks[mark] == value)
        return;

    marks[mark] = value;

    if(!mark.startsWith("@"))
        return;

    // @todo: consider move all the codes below to PlayerCardContainerUI.cpp
    // set mark doc
    QString text = "";
    QMapIterator<QString, int> itor(marks);
    while(itor.hasNext()){
        itor.next();

        if(itor.key().startsWith("@") && itor.value() > 0){
            QString mark_text = QString("<img src='image/mark/%1.png' />").arg(itor.key());
            if(itor.value() != 1)
                mark_text.append(QString("%1").arg(itor.value()));
            // @todo: add an option so that mark can be placed horizontally.
            mark_text.append("<br>");
            text.append(mark_text);
        }
    }

    mark_doc->setHtml(text);
}
