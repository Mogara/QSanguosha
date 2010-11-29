#include "serverplayer.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "ai.h"
#include "settings.h"

ServerPlayer::ServerPlayer(Room *room)
    : Player(room), socket(NULL), room(room), ai(NULL)
{
}

void ServerPlayer::drawCard(const Card *card){
    handcards << card;
}

Room *ServerPlayer::getRoom() const{
    return room;
}

void ServerPlayer::playCardEffect(const Card *card){
    if(card->isVirtualCard() && !card->getSkillName().isEmpty())
        room->playSkillEffect(card->getSkillName());
    else
        room->playCardEffect(card->objectName(), getGeneral()->isMale());
}

int ServerPlayer::getRandomHandCard() const{
    int index = qrand() % handcards.length();
    return handcards.at(index)->getId();
}

void ServerPlayer::obtainCard(const Card *card){
    room->obtainCard(this, card);
}

void ServerPlayer::throwAllEquips(){
    room->throwCard(getWeapon());
    room->throwCard(getArmor());
    room->throwCard(getDefensiveHorse());
    room->throwCard(getOffensiveHorse());
}

void ServerPlayer::throwAllHandCards(){
    foreach(const Card *card, handcards)
        room->throwCard(card);
}

void ServerPlayer::throwAllCards(){
    throwAllEquips();
    throwAllHandCards();

    QStack<const Card *> tricks = getJudgingArea();
    foreach(const Card *trick, tricks)
        room->throwCard(trick);
}

void ServerPlayer::drawCards(int n, bool set_emotion){
    room->drawCards(this, n);

    if(set_emotion)
        room->setEmotion(this, Room::DrawCard);
}

QList<int> ServerPlayer::forceToDiscard(int discard_num, bool include_equip){
    QList<int> to_discard;

    QString flags = "h";
    if(include_equip)
        flags.append("e");

    QList<const Card *> all_cards = getCards(flags);
    qShuffle(all_cards);

    int i;
    for(i=0; i<discard_num; i++)
        to_discard << all_cards.at(i)->getId();

    return to_discard;
}

int ServerPlayer::aliveCount() const{
    return room->alivePlayerCount();
}

int ServerPlayer::getHandcardNum() const{
    return handcards.length();
}

void ServerPlayer::setSocket(ClientSocket *socket){
    this->socket = socket;

    if(socket){
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(message_got(char*)), this, SLOT(getMessage(char*)));

        connect(this, SIGNAL(message_cast(QString)), this, SLOT(castMessage(QString)));
    }
}

void ServerPlayer::getMessage(char *message){
    QString request = message;
    if(request.endsWith("\n"))
        request.chop(1);

    emit request_got(request);
}

void ServerPlayer::unicast(const QString &message){
    emit message_cast(message);
}

void ServerPlayer::castMessage(const QString &message){
    if(socket){
        socket->send(message);

#ifndef QT_NO_DEBUG
        qDebug("%s: %s", qPrintable(objectName()), qPrintable(message));
#endif
    }
}

void ServerPlayer::invoke(const char *method, const QString &arg){
    unicast(QString("%1 %2").arg(method).arg(arg));
}

QString ServerPlayer::reportHeader() const{
    QString name = objectName();
    return QString("%1 ").arg(name.isEmpty() ? tr("Anonymous") : name);
}

void ServerPlayer::sendProperty(const char *property_name){
    QString value = property(property_name).toString();
    unicast(QString(".%1 %2").arg(property_name).arg(value));
}

void ServerPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case Hand: handcards.removeOne(card); break;
    case Equip: {
            const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            removeEquip(equip);

            LogMessage log;
            log.type = "$Uninstall";
            log.card_str = card->toString();
            log.from = this;
            room->sendLog(log);

            equip->onUninstall(this);
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

void ServerPlayer::addCard(const Card *card, Place place){
    switch(place){
    case Hand: handcards << card; break;
    case Equip: {
            const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            setEquip(equip);
            equip->onInstall(this);
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

QList<int> ServerPlayer::handCards() const{
    QList<int> card_ids;
    foreach(const Card *card, handcards)
        card_ids << card->getId();

    return card_ids;
}

QList<const Card *> ServerPlayer::getHandcards() const{
    return handcards;
}

QList<const Card *> ServerPlayer::getCards(const QString &flags) const{
    QList<const Card *> cards;
    if(flags.contains("h"))
        cards << handcards;

    if(flags.contains("e"))
        cards << getEquips();

    if(flags.contains("j")){
        QStack<const Card *> tricks = getJudgingArea();
        foreach(const Card *trick, tricks)
            cards << trick;
    }

    return cards;
}

DummyCard *ServerPlayer::wholeHandCards() const{
    if(isKongcheng())
        return NULL;

    DummyCard *dummy_card = new DummyCard;
    foreach(const Card *card, handcards)
        dummy_card->addSubcard(card->getId());

    return dummy_card;
}

bool ServerPlayer::isLord() const{
    return room->getLord() == this;
}

bool ServerPlayer::hasNullification() const{
    if(hasSkill("kanpo")){
        foreach(const Card *card, handcards){
            if(card->isBlack() || card->objectName() == "nullification")
                return true;
        }
    }else{
        foreach(const Card *card, handcards){
            if(card->objectName() == "nullification")
                return true;
        }
    }

    return false;
}

void ServerPlayer::kick(){
    if(socket){
        socket->disconnectFromHost();
    }
}

void ServerPlayer::setAIByGeneral(){
    if(getGeneral()){
        delete ai;
        ai = AI::CreateAI(this);
    }
}

void ServerPlayer::setAI(AI *ai) {
    this->ai = ai;
}

AI *ServerPlayer::getAI() const{
    if(getState() == "online")
        return NULL;
    else
        return ai;
}
