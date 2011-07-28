#include "serverplayer.h"
#include "skill.h"
#include "engine.h"
#include "standard.h"
#include "ai.h"
#include "settings.h"
#include "recorder.h"

ServerPlayer::ServerPlayer(Room *room)
    : Player(room), socket(NULL), room(room),
    ai(NULL), trust_ai(new TrustAI(this)), recorder(NULL), next(NULL)
{
}

void ServerPlayer::drawCard(const Card *card){
    handcards << card;
}

Room *ServerPlayer::getRoom() const{
    return room;
}

void ServerPlayer::playCardEffect(const Card *card){
    if(card->isVirtualCard() && !card->isMute()){
        QString skill_name = card->getSkillName();
        const Skill *skill = Sanguosha->getSkill(skill_name);
        int index = -1;
        if(skill)
            index = skill->getEffectIndex(this, card);

        room->playSkillEffect(skill_name, index);
    }else
        room->playCardEffect(card->objectName(), getGeneral()->isMale());
}

int ServerPlayer::getRandomHandCardId() const{
    return getRandomHandCard()->getEffectiveId();
}

const Card *ServerPlayer::getRandomHandCard() const{
    int index = qrand() % handcards.length();
    return handcards.at(index);
}

void ServerPlayer::obtainCard(const Card *card){
    room->obtainCard(this, card);
}

void ServerPlayer::throwAllEquips(){
    QList<const Card *> equips = getEquips();

    if(equips.isEmpty())
        return;

    DummyCard *card = new DummyCard;
    foreach(const Card *equip, equips)
        card->addSubcard(equip);
    room->throwCard(card);

    CardStar card_star = card;
    QVariant data = QVariant::fromValue(card_star);
    room->getThread()->trigger(CardDiscarded, this, data);
    card->deleteLater();
}

void ServerPlayer::throwAllHandCards(){
    DummyCard *card = wholeHandCards();
    if(card == NULL)
        return;

    room->throwCard(card);
    CardStar card_star = card;
    QVariant data = QVariant::fromValue(card_star);
    room->getThread()->trigger(CardDiscarded, this, data);
    card->deleteLater();
}

void ServerPlayer::throwAllMarks(){
    // throw all marks
    foreach(QString mark_name, marks.keys()){
        if(!mark_name.startsWith("@"))
            continue;

        int n = marks.value(mark_name, 0);
        if(n != 0){
            room->setPlayerMark(this, mark_name, 0);
        }
    }

    marks.clear();
}

void ServerPlayer::clearPrivatePiles(){
    // throw private piles
    foreach(QString pile_name, piles.keys()){
        QList<int> &pile = piles[pile_name];

        foreach(int card_id, pile){
            room->throwCard(card_id);
            QString pile_command = QString("%1:%2-%3").arg(objectName()).arg(pile_name).arg(card_id);
            room->broadcastInvoke("pile", pile_command);
        }
    }
    piles.clear();
}

void ServerPlayer::bury(){
    throwAllCards();
    throwAllMarks();
    clearPrivatePiles();
}

void ServerPlayer::throwAllCards(){
    throwAllEquips();
    throwAllHandCards();

    QList<const Card *> tricks = getJudgingArea();
    foreach(const Card *trick, tricks)
        room->throwCard(trick);
}

void ServerPlayer::drawCards(int n, bool set_emotion){
    room->drawCards(this, n);

    if(set_emotion)
        room->setEmotion(this, "draw-card");
}

// a convenient way to ask player
bool ServerPlayer::askForSkillInvoke(const QString &skill_name, const QVariant &data){
    return room->askForSkillInvoke(this, skill_name, data);
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
    if(socket){
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(message_got(char*)), this, SLOT(getMessage(char*)));

        connect(this, SIGNAL(message_cast(QString)), this, SLOT(castMessage(QString)));
    }else{
        if(this->socket){
            this->disconnect(this->socket);
            this->socket->disconnect(this);
            this->socket->disconnectFromHost();
            this->socket->deleteLater();
        }

        disconnect(this, SLOT(castMessage(QString)));
    }

    this->socket = socket;
}

void ServerPlayer::getMessage(char *message){
    QString request = message;
    if(request.endsWith("\n"))
        request.chop(1);

    emit request_got(request);
}

void ServerPlayer::unicast(const QString &message) const{
    emit message_cast(message);

    if(recorder)
        recorder->recordLine(message);
}

void ServerPlayer::startRecord(){
    recorder = new Recorder(this);
}

void ServerPlayer::saveRecord(const QString &filename){
    if(recorder)
        recorder->save(filename);
}

void ServerPlayer::addToSelected(const QString &general){
    selected.append(general);
}

QStringList ServerPlayer::getSelected() const{
    return selected;
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

void ServerPlayer::sendProperty(const char *property_name, const Player *player) const{
    if(player == NULL)
        player = this;

    QString value = player->property(property_name).toString();
    if(player == this)
        unicast(QString(".%1 %2").arg(property_name).arg(value));
    else
        unicast(QString("#%1 %2 %3").arg(player->objectName()).arg(property_name).arg(value));
}

void ServerPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            handcards.removeOne(card);
            break;
        }

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

    case Special:{
            int card_id = card->getEffectiveId();
            QString pile_name = getPileName(card_id);
            Q_ASSERT(!pile_name.isEmpty());

            piles[pile_name].removeOne(card_id);

            break;
        }

    default:
        // FIXME
        ;
    }
}

void ServerPlayer::addCard(const Card *card, Place place){
    switch(place){
    case Hand: {
            handcards << card;
            break;
        }

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

bool ServerPlayer::isLastHandCard(const Card *card) const{
    if(handcards.length() != 1)
        return false;

    return card->getEffectiveId() == handcards.first()->getEffectiveId();
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

    if(flags.contains("j"))
        cards << getJudgingArea();

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

bool ServerPlayer::hasNullification() const{
    if(hasSkill("kanpo")){
        foreach(const Card *card, handcards){
            if(card->isBlack() || card->objectName() == "nullification")
                return true;
        }
    }else if(hasSkill("wushen")){
        foreach(const Card *card, handcards){
            if(card->objectName() == "nullification" && card->getSuit() != Card::Heart)
                return true;
        }
    }else if(hasSkill("guhuo")){
        return !isKongcheng();
    }else if(hasFlag("lexue")){
        int card_id = getMark("lexue");
        const Card *card = Sanguosha->getCard(card_id);
        if(card->objectName() == "nullification"){
            foreach(const Card *c, handcards + getEquips()){
                if(c->objectName() == "nullification" || c->getSuit() == card->getSuit())
                    return true;
            }
        }
    }else if(hasSkill("longhun")){
        int n = qMax(1, getHp());
        int count = 0;
        foreach(const Card *card, handcards + getEquips()){
            if(card->objectName() == "nullification")
                return true;

            if(card->getSuit() == Card::Spade)
                count ++;
        }

        return count >= n;
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

bool ServerPlayer::pindian(ServerPlayer *target, const QString &reason, const Card *card1){
    LogMessage log;
    log.type = "#Pindian";
    log.from = this;
    log.to << target;
    room->sendLog(log);

    if(card1 == NULL)
        card1 = room->askForPindian(this, this, target, reason);
    else if(card1->isVirtualCard()){
        int card_id = card1->getEffectiveId();
        card1 = Sanguosha->getCard(card_id);
    }

    const Card *card2 = room->askForPindian(target, this, target, reason);

    PindianStruct pindian_struct;
    pindian_struct.from = this;
    pindian_struct.to = target;
    pindian_struct.from_card = card1;
    pindian_struct.to_card = card2;
    pindian_struct.reason = reason;

    PindianStar pindian_star = &pindian_struct;
    QVariant data = QVariant::fromValue(pindian_star);
    room->getThread()->trigger(Pindian, this, data);

    bool success = pindian_star->from_card->getNumber() > pindian_star->to_card->getNumber();
    log.type = success ? "#PindianSuccess" : "#PindianFailure";
    log.from = this;
    log.to.clear();
    log.to << target;
    log.card_str.clear();
    room->sendLog(log);

    if(success)
        room->setEmotion(this, "success");
    else
        room->setEmotion(this, "no-success");

    return success;
}

void ServerPlayer::turnOver(){
    setFaceUp(!faceUp());
    room->broadcastProperty(this, "faceup");

    LogMessage log;
    log.type = "#TurnOver";
    log.from = this;
    log.arg = faceUp() ? "face_up" : "face_down";
    room->sendLog(log);
}

void ServerPlayer::play(){
    static QList<Phase> all_phases;
    if(all_phases.isEmpty()){
        all_phases << Start << Judge << Draw << Play
                << Discard << Finish << NotActive;
    }

    phases = all_phases;
    while(!phases.isEmpty()){
        Phase phase = phases.takeFirst();
        setPhase(phase);
        room->broadcastProperty(this, "phase");
        room->getThread()->trigger(PhaseChange, this);

        if(isDead() && phase != NotActive){
            phases.clear();
            phases << NotActive;
        }
    }
}

QList<Player::Phase> &ServerPlayer::getPhases(){
    return phases;
}

void ServerPlayer::skip(Player::Phase phase){
    phases.removeOne(phase);

    static QStringList phase_strings;
    if(phase_strings.isEmpty()){
        phase_strings << "start" << "judge" << "draw"
                << "play" << "discard" << "finish" << "not_active";
    }

    int index = static_cast<int>(phase);

    LogMessage log;
    log.type = "#SkipPhase";
    log.from = this;
    log.arg = phase_strings.at(index);
    room->sendLog(log);
}


void ServerPlayer::gainMark(const QString &mark, int n){
    int value = getMark(mark) + n;

    LogMessage log;
    log.type = "#GetMark";
    log.from = this;
    log.arg = mark;
    log.arg2 = QString::number(n);

    room->sendLog(log);

    room->setPlayerMark(this, mark, value);
}

void ServerPlayer::loseMark(const QString &mark, int n){
    int value = getMark(mark) - n;

    LogMessage log;
    log.type = "#LoseMark";
    log.from = this;
    log.arg = mark;
    log.arg2 = QString::number(n);

    room->sendLog(log);

    room->setPlayerMark(this, mark, value);
}

void ServerPlayer::loseAllMarks(const QString &mark_name){
    int n = getMark(mark_name);
    if(n > 0){
        loseMark(mark_name, n);
    }
}

void ServerPlayer::setAI(AI *ai) {
    this->ai = ai;
}

AI *ServerPlayer::getAI() const{
    if(getState() == "online"){
        return NULL;
    }
    else if(getState() == "trust" && !Config.FreeChoose)
        return trust_ai;
    else
        return ai;
}

AI *ServerPlayer::getSmartAI() const{
    return ai;
}

void ServerPlayer::addVictim(ServerPlayer *victim){
    victims.append(victim);
}

QList<ServerPlayer *> ServerPlayer::getVictims() const{
    return victims;
}

void ServerPlayer::setNext(ServerPlayer *next){
    this->next = next;
}

ServerPlayer *ServerPlayer::getNext() const{
    return next;
}

ServerPlayer *ServerPlayer::getNextAlive() const{
    ServerPlayer *next = this->next;
    while(next->isDead())
        next = next->getNext();

    return next;
}

int ServerPlayer::getGeneralMaxHP() const{
    int max_hp = 0;

    if(getGeneral2() == NULL)
        max_hp = getGeneral()->getMaxHp();
    else{
        int first = getGeneral()->getMaxHp();
        int second = getGeneral2()->getMaxHp();

        switch(Config.MaxHpScheme){
        case 2: max_hp = (first + second)/2; break;
        case 1: max_hp = qMin(first, second); break;
        case 0:
        default:
            max_hp = first + second - 3; break;
        }

        max_hp = qMin(max_hp, 8);
    }

    if(room->hasWelfare(this))
        max_hp++;

    return max_hp;
}

bool ServerPlayer::hasLordSkill(const QString &skill_name) const{
    if(room->getMode() == "06_3v3" || room->getMode() == "02_1v1")
        return false;
    else if(acquired_skills.contains(skill_name))
        return true;
    else
        return isLord() && hasSkill(skill_name);
}

QString ServerPlayer::getIp() const{
    if(socket)
        return socket->peerAddress();
    else
        return QString();
}

void ServerPlayer::introduceTo(ServerPlayer *player){
    QString screen_name = Config.ContestMode ? tr("Contestant") : screenName();
    QString avatar = property("avatar").toString();

    QString introduce_str = QString("%1:%2:%3")
                            .arg(objectName())
                            .arg(QString(screen_name.toUtf8().toBase64()))
                            .arg(avatar);

    if(player)
        player->invoke("addPlayer", introduce_str);
    else
        room->broadcastInvoke("addPlayer", introduce_str, this);
}

void ServerPlayer::marshal(ServerPlayer *player) const{
    player->sendProperty("maxhp", this);
    player->sendProperty("hp", this);

    if(getKingdom() != getGeneral()->getKingdom())
        player->sendProperty("kingdom", this);

    if(isAlive()){
        player->sendProperty("seat", this);
        if(getXueyi() > 0)
            player->sendProperty("xueyi", this);
        if(getPhase() != Player::NotActive)
            player->sendProperty("phase", this);
    }else{
        player->sendProperty("alive", this);
        player->sendProperty("role", this);
        player->invoke("killPlayer", objectName());
    }

    if(!faceUp())
        player->sendProperty("faceup", this);

    if(isChained())
        player->sendProperty("chained", this);

    if(getAttackRange() != 1)
        player->sendProperty("atk", this);

    if(!isKongcheng()){
        if(player != this){
            player->invoke("drawNCards",
                           QString("%1:%2")
                           .arg(objectName())
                           .arg(getHandcardNum()));
        }else{
            QStringList card_str;
            foreach(const Card *card, handcards){
                card_str << QString::number(card->getId());
            }

            player->invoke("drawCards", card_str.join("+"));
        }
    }


    foreach(const Card *equip, getEquips()){
        player->invoke("moveCard",
                       QString("%1:_@=->%2@equip")
                       .arg(equip->getId())
                       .arg(objectName()));
    }

    foreach(const Card *card, getJudgingArea()){
        player->invoke("moveCard",
                       QString("%1:_@=->%2@judging")
                       .arg(card->getId())
                       .arg(objectName()));
    }

    foreach(QString mark_name, marks.keys()){
        if(mark_name.startsWith("@")){
            int value = getMark(mark_name);
            if(value != 0){
                QString mark_str = QString("%1.%2=%3")
                                   .arg(objectName())
                                   .arg(mark_name)
                                   .arg(value);

                player->invoke("setMark", mark_str);
            }
        }
    }

    foreach(QString skill_name, acquired_skills){
        player->invoke("acquireSkill", QString("%1:%2").arg(objectName()).arg(skill_name));
    }

    foreach(QString flag, flags){
        player->unicast(QString("#%1 flags %2").arg(objectName()).arg(flag));
    }

    foreach(QString item, history.keys()){
        int value = history.value(item);
        if(value > 0)
            player->invoke("addHistory", QString("%1#%2").arg(item).arg(value));
    }
}

void ServerPlayer::addToPile(const QString &pile_name, int card_id, bool open){
    piles[pile_name] << card_id;

    room->moveCardTo(Sanguosha->getCard(card_id), this, Player::Special, open);
}

void ServerPlayer::copyFrom(ServerPlayer* sp)
{
    ServerPlayer *b = this;
    ServerPlayer *a = sp;

    b->handcards    = QList<const Card *> (a->handcards);
    b->phases       = QList<ServerPlayer::Phase> (a->phases);
    b->selected     = QStringList (a->selected);

    Player* c = b;
    c->copyFrom(a);
}
