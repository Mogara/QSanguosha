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

    return known_cards.first()->getEffectiveId() == card->getEffectiveId();
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
    if(add)
        piles[name] << card_id;
    else
        piles[name].removeOne(card_id);

    emit pile_changed(name);
}

QString ClientPlayer::getDeathPixmapPath() const{
    QString basename;
    if(ServerInfo.GameMode == "06_3v3"){
        if(getRole() == "lord" || getRole() == "renegade")
            basename = "marshal";
        else
            basename = "guard";
    }else
        basename = getRole();

    if(basename.isEmpty()){
        basename = "unknown";
    }

    return QString("image/system/death/%1.png").arg(basename);
}

bool ClientPlayer::hasLordSkill(const QString &skill_name) const{
    if(ServerInfo.GameMode == "06_3v3")
        return false;
    else if(acquired_skills.contains(skill_name))
        return true;
    else
        return getRole() == "lord" && hasSkill(skill_name);
}

void ClientPlayer::setHandcardNum(int n){
    handcard_num = n;
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
