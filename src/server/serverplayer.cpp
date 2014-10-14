/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "serverplayer.h"
#include "engine.h"
#include "standard.h"
#include "ai.h"
#include "settings.h"
#include "recorder.h"
#include "lua-wrapper.h"
#include "json.h"
#include "gamerule.h"

using namespace QSanProtocol;

const int ServerPlayer::S_NUM_SEMAPHORES = 6;

ServerPlayer::ServerPlayer(Room *room)
    : Player(room), m_isClientResponseReady(false), m_isWaitingReply(false),
      event_received(false), socket(NULL), room(room),
      ai(NULL), trust_ai(new TrustAI(this)), recorder(NULL),
      _m_phases_index(0)
{
    semas = new QSemaphore *[S_NUM_SEMAPHORES];
    for (int i = 0; i < S_NUM_SEMAPHORES; i++)
        semas[i] = new QSemaphore(0);
}

ServerPlayer::~ServerPlayer(){
    for (int i = 0; i < S_NUM_SEMAPHORES; i++)
        delete semas[i];

    delete[] semas;
    delete trust_ai;
}

void ServerPlayer::drawCard(const Card *card) {
    handcards << card;
}

Room *ServerPlayer::getRoom() const{
    return room;
}

void ServerPlayer::broadcastSkillInvoke(const QString &card_name) const{
    room->broadcastSkillInvoke(card_name, isMale(), -1);
}

void ServerPlayer::broadcastSkillInvoke(const Card *card) const{
    if (card->isMute())
        return;

    QString skill_name = card->getSkillName();
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill == NULL) {
        if (card->getCommonEffectName().isNull())
            broadcastSkillInvoke(card->objectName());
        else
            room->broadcastSkillInvoke(card->getCommonEffectName(), "common");
        return;
    } else {
        int index = skill->getEffectIndex(this, card);
        if (index == 0) return;

        if ((index == -1 && skill->getSources().isEmpty()) || index == -2) {
            if (card->getCommonEffectName().isNull())
                broadcastSkillInvoke(card->objectName());
            else
                room->broadcastSkillInvoke(card->getCommonEffectName(), "common");
        } else {
            room->broadcastSkillInvoke(skill_name, index, this);
        }
    }
}

int ServerPlayer::getRandomHandCardId() const{
    return getRandomHandCard()->getEffectiveId();
}

const Card *ServerPlayer::getRandomHandCard() const{
    int index = qrand() % handcards.length();
    return handcards.at(index);
}

void ServerPlayer::obtainCard(const Card *card, bool unhide) {
    CardMoveReason reason(CardMoveReason::S_REASON_GOTCARD, objectName());
    room->obtainCard(this, card, reason, unhide);
}

void ServerPlayer::throwAllEquips() {
    QList<const Card *> equips = getEquips();

    if (equips.isEmpty()) return;

    DummyCard card;
    foreach(const Card *equip, equips) {
        if (!isJilei(&card))
            card.addSubcard(equip);
    }
    if (card.subcardsLength() > 0)
        room->throwCard(&card, this);
}

void ServerPlayer::throwAllHandCards() {
    int card_length = getHandcardNum();
    room->askForDiscard(this, QString(), card_length, card_length);
}

void ServerPlayer::throwAllHandCardsAndEquips() {
    int card_length = getCardCount(true);
    room->askForDiscard(this, QString(), card_length, card_length, false, true);
}

void ServerPlayer::throwAllMarks(bool visible_only) {
    // throw all marks
    foreach(QString mark_name, marks.keys()) {
        if (!mark_name.startsWith("@"))
            continue;

        int n = marks.value(mark_name, 0);
        if (n != 0)
            room->setPlayerMark(this, mark_name, 0);
    }

    if (!visible_only)
        marks.clear();
}

void ServerPlayer::clearOnePrivatePile(const QString &pile_name) {
    if (!piles.contains(pile_name))
        return;
    QList<int> &pile = piles[pile_name];

    DummyCard dummy(pile);
    CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, this->objectName());
    room->throwCard(&dummy, reason, NULL);
    piles.remove(pile_name);
}

void ServerPlayer::clearPrivatePiles() {
    foreach(QString pile_name, piles.keys())
        clearOnePrivatePile(pile_name);
    piles.clear();
}

void ServerPlayer::bury() {
    clearFlags();
    clearHistory();
    throwAllCards();
    throwAllMarks();
    clearPrivatePiles();

    room->clearPlayerCardLimitation(this, false);
}

void ServerPlayer::throwAllCards() {
    DummyCard *card = isKongcheng() ? new DummyCard : wholeHandCards();
    foreach(const Card *equip, getEquips())
        card->addSubcard(equip);
    if (card->subcardsLength() != 0)
        room->throwCard(card, this);
    card->deleteLater();

    QList<const Card *> tricks = getJudgingArea();
    foreach(const Card *trick, tricks) {
        CardMoveReason reason(CardMoveReason::S_REASON_THROW, this->objectName());
        room->throwCard(trick, reason, NULL);
    }
}

int ServerPlayer::getMaxCards(MaxCardsType::MaxCardsCount type) const{
    int origin = Sanguosha->correctMaxCards(this, true, type);
    if (origin == 0)
        origin = qMax(getHp(), 0);

    origin += Sanguosha->correctMaxCards(this, false, type);

    return qMax(origin, 0);
}

void ServerPlayer::drawCards(int n, const QString &reason) {
    room->drawCards(this, n, reason);
}

bool ServerPlayer::askForSkillInvoke(const QString &skill_name, const QVariant &data) {
    return room->askForSkillInvoke(this, skill_name, data);
}

QList<int> ServerPlayer::forceToDiscard(int discard_num, bool include_equip, bool is_discard) {
    QList<int> to_discard;

    QString flags = "h";
    if (include_equip)
        flags.append("e");

    QList<const Card *> all_cards = getCards(flags);
    qShuffle(all_cards);

    for (int i = 0; i < all_cards.length(); i++) {
        if (!is_discard || !isJilei(all_cards.at(i)))
            to_discard << all_cards.at(i)->getId();
        if (to_discard.length() == discard_num)
            break;
    }

    return to_discard;
}

int ServerPlayer::aliveCount(bool includeRemoved) const{
    int n = room->alivePlayerCount();
    if (!includeRemoved)
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->isRemoved())
                n--;
    return n;
}

int ServerPlayer::getHandcardNum() const{
    return handcards.length();
}

int ServerPlayer::getPlayerNumWithSameKingdom(const QString &reason, const QString &_to_calculate, MaxCardsType::MaxCardsCount type) const{
    QString to_calculate = _to_calculate;

    if (to_calculate.isEmpty()) {
        if (getRole() == "careerist")
            to_calculate = "careerist";
        else
            to_calculate = getKingdom();
    }

    ServerPlayer *this_player = room->findPlayer(objectName());
    QList<ServerPlayer *> players = room->getAlivePlayers();

    int num = 0;
    foreach (ServerPlayer *p, players) {
        if (!p->hasShownOneGeneral())
            continue;
        if (to_calculate == "careerist") {
            if (p->getRole() == "careerist") {
                ++num;
                break;    // careerist always alone.
            }
            continue;
        }
        if (p->getKingdom() == to_calculate)
            ++num;
    }

    if (reason != "AI") {
        QVariant data = QVariant::fromValue(PlayerNumStruct(num, to_calculate, type, reason));
        room->getThread()->trigger(ConfirmPlayerNum, room, this_player, data);
        PlayerNumStruct playerNumStruct = data.value<PlayerNumStruct>();
        num = playerNumStruct.m_num;
    }

    return qMax(num, 0);
}

void ServerPlayer::setSocket(ClientSocket *socket) {
    if (socket) {
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(message_got(QByteArray)), this, SLOT(getMessage(QByteArray)));
        connect(this, SIGNAL(message_ready(QByteArray)), this, SLOT(sendMessage(QByteArray)));
    }
    else {
        if (this->socket) {
            this->disconnect(this->socket);
            this->socket->disconnect(this);
            //this->socket->disconnectFromHost();
            this->socket->deleteLater();
        }

        disconnect(this, SLOT(sendMessage(QByteArray)));
    }

    this->socket = socket;
}

void ServerPlayer::kick(){
    room->notifyProperty(this, this, "flags", "is_kicked");
    if (socket != NULL)
        socket->disconnectFromHost();
    setSocket(NULL);
}

void ServerPlayer::getMessage(QByteArray request) {
    if (request.endsWith('\n'))
        request.chop(1);

    emit request_got(request);

    Packet packet;
    if (packet.parse(request)) {
        switch (packet.getPacketDestination()) {
        case S_DEST_ROOM:
            emit roomPacketReceived(packet);
            break;
        //unused destination. Lobby hasn't been implemented.
        case S_DEST_LOBBY:
            emit lobbyPacketReceived(packet);
            break;
        default:
            emit invalidPacketReceived(request);
        }
    } else {
        emit invalidPacketReceived(request);
    }
}

void ServerPlayer::unicast(const QByteArray &message) {
    emit message_ready(message);

    if (recorder)
        recorder->recordLine(message);
}

void ServerPlayer::startNetworkDelayTest() {
    test_time = QDateTime::currentDateTime();
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, S_COMMAND_NETWORK_DELAY_TEST);
    unicast(&packet);
}

qint64 ServerPlayer::endNetworkDelayTest() {
    return test_time.msecsTo(QDateTime::currentDateTime());
}

void ServerPlayer::startRecord() {
    recorder = new Recorder(this);
}

void ServerPlayer::saveRecord(const QString &filename) {
    if (recorder)
        recorder->save(filename);
}

void ServerPlayer::addToSelected(const QString &general) {
    selected.append(general);
}

QStringList ServerPlayer::getSelected() const{
    return selected;
}

QString ServerPlayer::findReasonable(const QStringList &generals, bool no_unreasonable) {
    foreach(QString name, generals) {
        if (getGeneral() && getGeneral()->getKingdom() != Sanguosha->getGeneral(name)->getKingdom())
            continue;
        return name;
    }

    if (no_unreasonable)
        return QString();

    return generals.first();
}

void ServerPlayer::clearSelected() {
    selected.clear();
}

void ServerPlayer::sendMessage(const QByteArray &message) {
    if (socket) {
#ifndef QT_NO_DEBUG
        printf("%s", qPrintable(objectName()));
#endif
        socket->send(message);
    }
}

void ServerPlayer::unicast(const AbstractPacket *packet) {
    unicast(packet->toJson());
}

void ServerPlayer::notify(CommandType type, const QVariant &arg){
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, type);
    packet.setMessageBody(arg);
    unicast(packet.toJson());
}

QString ServerPlayer::reportHeader() const{
    QString name = objectName();
    return QString("%1 ").arg(name.isEmpty() ? tr("Anonymous") : name);
}

void ServerPlayer::removeCard(const Card *card, Place place) {
    switch (place) {
    case PlaceHand: {
        handcards.removeOne(card);
        break;
    }
    case PlaceEquip: {
        const EquipCard *equip = qobject_cast<const EquipCard *>(card->getRealCard());
        if (equip == NULL)
            equip = qobject_cast<const EquipCard *>(Sanguosha->getEngineCard(card->getEffectiveId()));
        Q_ASSERT(equip != NULL);
        equip->onUninstall(this);

        WrappedCard *wrapped = Sanguosha->getWrappedCard(card->getEffectiveId());
        removeEquip(wrapped);

        bool show_log = true;
        foreach(QString flag, flags)
            if (flag.endsWith("_InTempMoving")) {
            show_log = false;
            break;
            }
        if (show_log) {
            LogMessage log;
            log.type = "$Uninstall";
            log.card_str = wrapped->toString();
            log.from = this;
            room->sendLog(log);
        }
        break;
    }
    case PlaceDelayedTrick: {
        removeDelayedTrick(card);
        break;
    }
    case PlaceSpecial: {
        int card_id = card->getEffectiveId();
        QString pile_name = getPileName(card_id);

        //@todo: sanity check required
        if (!pile_name.isEmpty())
            piles[pile_name].removeOne(card_id);

        break;
    }
    default:
        break;
    }
}

void ServerPlayer::addCard(const Card *card, Place place) {
    switch (place) {
    case PlaceHand: {
        handcards << card;
        break;
    }
    case PlaceEquip: {
        WrappedCard *wrapped = Sanguosha->getWrappedCard(card->getEffectiveId());
        const EquipCard *equip = qobject_cast<const EquipCard *>(wrapped->getRealCard());
        setEquip(wrapped);
        equip->onInstall(this);
        break;
    }
    case PlaceDelayedTrick: {
        addDelayedTrick(card);
        break;
    }
    default:
        break;
    }
}

bool ServerPlayer::isLastHandCard(const Card *card, bool contain) const{
    if (!card->isVirtualCard()) {
        return handcards.length() == 1 && handcards.first()->getEffectiveId() == card->getEffectiveId();
    }
    else if (card->getSubcards().length() > 0) {
        if (!contain) {
            foreach(int card_id, card->getSubcards()) {
                if (!handcards.contains(Sanguosha->getCard(card_id)))
                    return false;
            }
            return handcards.length() == card->getSubcards().length();
        }
        else {
            foreach(const Card *ncard, handcards) {
                if (!card->getSubcards().contains(ncard->getEffectiveId()))
                    return false;
            }
            return true;
        }
    }
    return false;
}

QList<int> ServerPlayer::handCards() const{
    QList<int> cardIds;
    foreach(const Card *card, handcards)
        cardIds << card->getId();
    return cardIds;
}

QList<const Card *> ServerPlayer::getHandcards() const{
    return handcards;
}

QList<const Card *> ServerPlayer::getCards(const QString &flags) const{
    QList<const Card *> cards;
    if (flags.contains("h"))
        cards << handcards;
    if (flags.contains("e"))
        cards << getEquips();
    if (flags.contains("j"))
        cards << getJudgingArea();

    return cards;
}

DummyCard *ServerPlayer::wholeHandCards() const{
    if (isKongcheng()) return NULL;

    DummyCard *dummy_card = new DummyCard;
    foreach(const Card *card, handcards)
        dummy_card->addSubcard(card->getId());

    return dummy_card;
}

bool ServerPlayer::hasNullification() const{
    foreach (const Card *card, handcards) {
        if (card->isKindOf("Nullification"))
            return true;
    }
    foreach (int id, getPile("wooden_ox")) {
        if (Sanguosha->getCard(id)->isKindOf("Nullification"))
            return true;
    }

    foreach(const Skill *skill, getVisibleSkillList(true)) {
        if (hasSkill(skill->objectName())){
            if (skill->inherits("ViewAsSkill")) {
                const ViewAsSkill *vsskill = qobject_cast<const ViewAsSkill *>(skill);
                if (vsskill->isEnabledAtNullification(this)) return true;
            }
            else if (skill->inherits("TriggerSkill")) {
                const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
                if (trigger_skill && trigger_skill->getViewAsSkill()) {
                    const ViewAsSkill *vsskill = qobject_cast<const ViewAsSkill *>(trigger_skill->getViewAsSkill());
                    if (vsskill && vsskill->isEnabledAtNullification(this)) return true;
                }
            }
        }
    }

    return false;
}


PindianStruct *ServerPlayer::pindianSelect(ServerPlayer *target, const QString &reason, const Card *card1) {
    LogMessage log;
    log.type = "#Pindian";
    log.from = this;
    log.to << target;
    room->sendLog(log);

    room->tryPause();

    const Card *card2;

    if (card1 == NULL) {
        QList<const Card *> cards = room->askForPindianRace(this, target, reason);
        card1 = cards.first();
        card2 = cards.last();
    }
    else {
        if (card1->isVirtualCard()) {
            int card_id = card1->getEffectiveId();
            card1 = Sanguosha->getCard(card_id);
        }
        card2 = room->askForPindian(target, this, target, reason);
    }

    if (card1 == NULL || card2 == NULL) return NULL;

    PindianStruct *pindian = new PindianStruct;
    pindian->from = this;
    pindian->to = target;
    pindian->from_card = card1;
    pindian->to_card = card2;
    pindian->from_number = card1->getNumber();
    pindian->to_number = card2->getNumber();
    pindian->reason = reason;

    QList<CardsMoveStruct> pd_move;
    CardsMoveStruct move1;
    move1.card_ids << pindian->from_card->getEffectiveId();
    move1.from = pindian->from;
    move1.to = NULL;
    move1.to_place = Player::PlaceTable;
    CardMoveReason reason1(CardMoveReason::S_REASON_PINDIAN, pindian->from->objectName(), pindian->to->objectName(), pindian->reason, QString());
    move1.reason = reason1;

    CardsMoveStruct move2;
    move2.card_ids << pindian->to_card->getEffectiveId();
    move2.from = pindian->to;
    move2.to = NULL;
    move2.to_place = Player::PlaceTable;
    CardMoveReason reason2(CardMoveReason::S_REASON_PINDIAN, pindian->to->objectName());
    move2.reason = reason2;

    pd_move << move1 << move2;

    LogMessage log2;
    log2.type = "$PindianResult";
    log2.from = pindian->from;
    log2.card_str = QString::number(pindian->from_card->getEffectiveId());
    room->sendLog(log2);

    log2.type = "$PindianResult";
    log2.from = pindian->to;
    log2.card_str = QString::number(pindian->to_card->getEffectiveId());
    room->sendLog(log2);

    room->moveCardsAtomic(pd_move, true);

    return pindian;
}

bool ServerPlayer::pindian(PindianStruct *pd){
    Q_ASSERT(pd != NULL);

    room->tryPause();

    PindianStruct &pindian_struct = *pd;
    RoomThread *thread = room->getThread();
    PindianStruct *pindian_star = pd;
    QVariant data = QVariant::fromValue(pindian_star);
    Q_ASSERT(thread != NULL);
    thread->trigger(PindianVerifying, room, this, data);

    PindianStruct *new_star = data.value<PindianStruct *>();
    pindian_struct.from_number = new_star->from_number;
    pindian_struct.to_number = new_star->to_number;
    pindian_struct.success = (new_star->from_number > new_star->to_number);

    LogMessage log;
    log.type = pindian_struct.success ? "#PindianSuccess" : "#PindianFailure";
    log.from = this;
    log.to.clear();
    log.to << pd->to;
    log.card_str.clear();
    room->sendLog(log);

    JsonArray arg;
    arg << (int)S_GAME_EVENT_REVEAL_PINDIAN;
    arg << objectName();
    arg << pindian_struct.from_card->getEffectiveId();
    arg << pd->to->objectName();
    arg << pindian_struct.to_card->getEffectiveId();
    arg << pindian_struct.success;
    arg << pd->reason;
    room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);

    pindian_star = &pindian_struct;
    data = QVariant::fromValue(pindian_star);
    thread->trigger(Pindian, room, this, data);

    QList<CardsMoveStruct> pd_move;

    if (room->getCardPlace(pindian_struct.from_card->getEffectiveId()) == Player::PlaceTable) {
        CardsMoveStruct move1;
        move1.card_ids << pindian_struct.from_card->getEffectiveId();
        move1.from = pindian_struct.from;
        move1.to = NULL;
        move1.to_place = Player::DiscardPile;
        CardMoveReason reason1(CardMoveReason::S_REASON_PINDIAN, pindian_struct.from->objectName(), pindian_struct.to->objectName(),
            pindian_struct.reason, QString());
        move1.reason = reason1;
        pd_move << move1;
    }

    if (room->getCardPlace(pindian_struct.to_card->getEffectiveId()) == Player::PlaceTable) {
        CardsMoveStruct move2;
        move2.card_ids << pindian_struct.to_card->getEffectiveId();
        move2.from = pindian_struct.to;
        move2.to = NULL;
        move2.to_place = Player::DiscardPile;
        CardMoveReason reason2(CardMoveReason::S_REASON_PINDIAN, pindian_struct.to->objectName());
        move2.reason = reason2;
        pd_move << move2;
    }

    if (!pd_move.isEmpty())
        room->moveCardsAtomic(pd_move, true);

    QVariant decisionData = QVariant::fromValue(QString("pindian:%1:%2:%3:%4:%5")
        .arg(pd->reason)
        .arg(this->objectName())
        .arg(pindian_struct.from_card->getEffectiveId())
        .arg(pd->to->objectName())
        .arg(pindian_struct.to_card->getEffectiveId()));
    thread->trigger(ChoiceMade, room, this, decisionData);


    bool r = pindian_struct.success;
    delete pd;
    return r;
}

void ServerPlayer::turnOver() {
    setFaceUp(!faceUp());
    room->broadcastProperty(this, "faceup");

    LogMessage log;
    log.type = "#TurnOver";
    log.from = this;
    log.arg = faceUp() ? "face_up" : "face_down";
    room->sendLog(log);

    Q_ASSERT(room->getThread() != NULL);
    room->getThread()->trigger(TurnedOver, room, this);
}

bool ServerPlayer::changePhase(Player::Phase from, Player::Phase to) {
    RoomThread *thread = room->getThread();
    Q_ASSERT(room->getThread() != NULL);

    setPhase(PhaseNone);

    PhaseChangeStruct phase_change;
    phase_change.from = from;
    phase_change.to = to;
    QVariant data = QVariant::fromValue(phase_change);

    bool skip = thread->trigger(EventPhaseChanging, room, this, data);
    if (skip && to != NotActive) {
        setPhase(from);
        return true;
    }

    setPhase(to);
    room->broadcastProperty(this, "phase");

    if (!phases.isEmpty())
        phases.removeFirst();

    if (!thread->trigger(EventPhaseStart, room, this)) {
        if (getPhase() != NotActive)
            thread->trigger(EventPhaseProceeding, room, this);
    }
    if (getPhase() != NotActive)
        thread->trigger(EventPhaseEnd, room, this);

    return false;
}

void ServerPlayer::play(QList<Player::Phase> set_phases) {
    if (!set_phases.isEmpty()) {
        if (!set_phases.contains(NotActive))
            set_phases << NotActive;
    }
    else
        set_phases << RoundStart << Start << Judge << Draw << Play
        << Discard << Finish << NotActive;

    phases = set_phases;
    _m_phases_state.clear();
    for (int i = 0; i < phases.size(); i++) {
        PhaseStruct _phase;
        _phase.phase = phases[i];
        _m_phases_state << _phase;
    }

    for (int i = 0; i < _m_phases_state.size(); i++) {
        if (isDead()) {
            changePhase(getPhase(), NotActive);
            break;
        }

        _m_phases_index = i;
        PhaseChangeStruct phase_change;
        phase_change.from = getPhase();
        phase_change.to = phases[i];

        RoomThread *thread = room->getThread();
        setPhase(PhaseNone);
        QVariant data = QVariant::fromValue(phase_change);

        bool skip = thread->trigger(EventPhaseChanging, room, this, data);
        phase_change = data.value<PhaseChangeStruct>();
        _m_phases_state[i].phase = phases[i] = phase_change.to;

        setPhase(phases[i]);
        room->broadcastProperty(this, "phase");

        if ((skip || _m_phases_state[i].finished)
            && !thread->trigger(EventPhaseSkipping, room, this, data)
            && phases[i] != NotActive)
            continue;

        if (!thread->trigger(EventPhaseStart, room, this)) {
            if (getPhase() != NotActive)
                thread->trigger(EventPhaseProceeding, room, this);
        }
        if (getPhase() != NotActive)
            thread->trigger(EventPhaseEnd, room, this);
        else
            break;
    }
}

QList<Player::Phase> &ServerPlayer::getPhases() {
    return phases;
}

void ServerPlayer::skip(bool sendLog) {
    for (int i = 0; i < _m_phases_state.size(); i++)
        _m_phases_state[i].finished = true;

    if (sendLog) {
        LogMessage log;
        log.type = "#SkipAllPhase";
        log.from = this;
        room->sendLog(log);
    }
}

void ServerPlayer::skip(Player::Phase phase, bool sendLog) {
    for (int i = _m_phases_index; i < _m_phases_state.size(); i++) {
        if (_m_phases_state[i].phase == phase) {
            if (_m_phases_state[i].finished) return;
            _m_phases_state[i].finished = true;
            break;
        }
    }

    static QStringList phase_strings;
    if (phase_strings.isEmpty())
        phase_strings << "round_start" << "start" << "judge" << "draw"
        << "play" << "discard" << "finish" << "not_active";
    int index = static_cast<int>(phase);

    if (sendLog){
        LogMessage log;
        log.type = "#SkipPhase";
        log.from = this;
        log.arg = phase_strings.at(index);
        room->sendLog(log);
    }
}

void ServerPlayer::insertPhase(Player::Phase phase) {
    PhaseStruct _phase;
    _phase.phase = phase;
    phases.insert(_m_phases_index, phase);
    _m_phases_state.insert(_m_phases_index, _phase);
}

bool ServerPlayer::isSkipped(Player::Phase phase) {
    for (int i = _m_phases_index; i < _m_phases_state.size(); i++) {
        if (_m_phases_state[i].phase == phase)
            return _m_phases_state[i].finished;
    }
    return false;
}

void ServerPlayer::gainMark(const QString &mark, int n) {
    int value = getMark(mark) + n;

    LogMessage log;
    log.type = "#GetMark";
    log.from = this;
    log.arg = mark;
    log.arg2 = QString::number(n);

    room->sendLog(log);
    room->setPlayerMark(this, mark, value);
}

void ServerPlayer::loseMark(const QString &mark, int n) {
    if (getMark(mark) == 0) return;
    int value = getMark(mark) - n;
    if (value < 0) { value = 0; n = getMark(mark); }

    LogMessage log;
    log.type = "#LoseMark";
    log.from = this;
    log.arg = mark;
    log.arg2 = QString::number(n);

    room->sendLog(log);
    room->setPlayerMark(this, mark, value);
}

void ServerPlayer::loseAllMarks(const QString &mark_name) {
    loseMark(mark_name, getMark(mark_name));
}

void ServerPlayer::addSkill(const QString &skill_name, bool head_skill) {
    Player::addSkill(skill_name, head_skill);
    JsonArray args;
    args << (int) QSanProtocol::S_GAME_EVENT_ADD_SKILL;
    args << objectName();
    args << skill_name;
    args << head_skill;
    room->doNotify(this, QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void ServerPlayer::loseSkill(const QString &skill_name) {
    Player::loseSkill(skill_name);
    JsonArray args;
    args << (int) QSanProtocol::S_GAME_EVENT_LOSE_SKILL;
    args << objectName();
    args << skill_name;
    room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void ServerPlayer::setGender(General::Gender gender) {
    if (gender == getGender())
        return;
    Player::setGender(gender);
    JsonArray args;
    args << (int) QSanProtocol::S_GAME_EVENT_CHANGE_GENDER;
    args << objectName();
    args << (int)gender;
    room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

bool ServerPlayer::isOnline() const{
    return getState() == "online";
}

void ServerPlayer::setAI(AI *ai) {
    this->ai = ai;
}

AI *ServerPlayer::getAI() const{
    if (getState() == "online")
        return NULL;
    else if (getState() == "robot" || Config.EnableCheat)
        return ai;
    else
        return trust_ai;
}

AI *ServerPlayer::getSmartAI() const{
    return ai;
}

void ServerPlayer::addVictim(ServerPlayer *victim) {
    victims.append(victim);
}

QList<ServerPlayer *> ServerPlayer::getVictims() const{
    return victims;
}

int ServerPlayer::getGeneralMaxHp() const{
    int max_hp = 0;

    if (getGeneral2() == NULL)
        max_hp = getGeneral()->getDoubleMaxHp();
    else {
        int first = getGeneral()->getMaxHpHead();
        int second = getGeneral2()->getMaxHpDeputy();

        max_hp = (first + second) / 2;
    }

    return max_hp;
}

QString ServerPlayer::getGameMode() const{
    return room->getMode();
}

QString ServerPlayer::getIp() const{
    if (socket)
        return socket->peerAddress();
    else
        return QString();
}

void ServerPlayer::introduceTo(ServerPlayer *player) {
    QString screen_name = screenName();
    QString avatar = property("avatar").toString();

    JsonArray introduce_str;
    introduce_str << objectName();
    introduce_str << screen_name;
    introduce_str << avatar;

    if (player) {
        player->notify(S_COMMAND_ADD_PLAYER, introduce_str);
        room->notifyProperty(player, this, "state");
    } else {
        room->doBroadcastNotify(S_COMMAND_ADD_PLAYER, introduce_str, this);
        room->broadcastProperty(this, "state");
    }

    if (hasShownGeneral1()) {
        foreach(const QString skill_name, head_skills.keys()) {
            if (Sanguosha->getSkill(skill_name)->isVisible()) {
                JsonArray args1;
                args1 << (int) S_GAME_EVENT_ADD_SKILL;
                args1 << objectName();
                args1 << skill_name;
                args1 << true;
                room->doNotify(player, S_COMMAND_LOG_EVENT, args1);
            }

            foreach(const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)) {
                if (!related_skill->isVisible()) {
                    JsonArray args2;
                    args2 << (int) S_GAME_EVENT_ADD_SKILL;
                    args2 << objectName();
                    args2 << related_skill->objectName();
                    args2 << true;
                    room->doNotify(player, S_COMMAND_LOG_EVENT, args2);
                }
            }
        }
    }

    if (hasShownGeneral2()) {
        foreach(const QString skill_name, deputy_skills.keys()) {
            if (Sanguosha->getSkill(skill_name)->isVisible()) {
                JsonArray args1;
                args1 << S_GAME_EVENT_ADD_SKILL;
                args1 << objectName();
                args1 << skill_name;
                args1 << false;
                room->doNotify(player, S_COMMAND_LOG_EVENT, args1);
            }

            foreach(const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)) {
                if (!related_skill->isVisible()) {
                    JsonArray args2;
                    args2 << (int) S_GAME_EVENT_ADD_SKILL;
                    args2 << objectName();
                    args2 << related_skill->objectName();
                    args2 << false;
                    room->doNotify(player, S_COMMAND_LOG_EVENT, args2);
                }
            }
        }
    }
}

void ServerPlayer::marshal(ServerPlayer *player) const
{
    room->notifyProperty(player, this, "maxhp");
    room->notifyProperty(player, this, "hp");
    room->notifyProperty(player, this, "general1_showed");
    room->notifyProperty(player, this, "general2_showed");

    if (this == player || hasShownGeneral1())
        room->notifyProperty(player, this, "head_skin_id");
    if (this == player || hasShownGeneral2())
        room->notifyProperty(player, this, "deputy_skin_id");

    if (isAlive()) {
        room->notifyProperty(player, this, "seat");
        if (getPhase() != Player::NotActive)
            room->notifyProperty(player, this, "phase");
    } else {
        room->notifyProperty(player, this, "alive");
        room->notifyProperty(player, this, "role");
        room->doNotify(player, S_COMMAND_KILL_PLAYER, objectName());
    }

    if (!faceUp())
        room->notifyProperty(player, this, "faceup");

    if (isChained())
        room->notifyProperty(player, this, "chained");

    room->notifyProperty(player, this, "gender");

    QList<ServerPlayer*> players;
    players << player;

    QList<CardsMoveStruct> moves;

    if (!isKongcheng()) {
        CardsMoveStruct move;
        foreach(const Card *card, handcards) {
            move.card_ids << card->getId();
            if (player == this) {
                WrappedCard *wrapped = qobject_cast<WrappedCard *>(room->getCard(card->getId()));
                if (wrapped->isModified())
                    room->notifyUpdateCard(player, card->getId(), wrapped);
            }
        }
        move.from_place = DrawPile;
        move.to_player_name = objectName();
        move.to_place = PlaceHand;

        if (player == this)
            move.to = player;

        moves << move;
    }

    if (hasEquip()) {
        CardsMoveStruct move;
        foreach(const Card *card, getEquips()) {
            move.card_ids << card->getId();
            WrappedCard *wrapped = qobject_cast<WrappedCard *>(room->getCard(card->getId()));
            if (wrapped->isModified())
                room->notifyUpdateCard(player, card->getId(), wrapped);
        }
        move.from_place = DrawPile;
        move.to_player_name = objectName();
        move.to_place = PlaceEquip;

        moves << move;
    }

    if (!getJudgingAreaID().isEmpty()) {
        CardsMoveStruct move;
        foreach(int card_id, getJudgingAreaID())
            move.card_ids << card_id;
        move.from_place = DrawPile;
        move.to_player_name = objectName();
        move.to_place = PlaceDelayedTrick;

        moves << move;
    }

    if (!moves.isEmpty()) {
        room->notifyMoveCards(true, moves, false, players);
        room->notifyMoveCards(false, moves, false, players);
    }

    if (!getPileNames().isEmpty()) {
        CardsMoveStruct move;
        move.from_place = DrawPile;
        move.to_player_name = objectName();
        move.to_place = PlaceSpecial;
        foreach(QString pile, piles.keys()) {
            move.card_ids.clear();
            move.card_ids.append(piles[pile]);
            move.to_pile_name = pile;

            QList<CardsMoveStruct> moves2;
            moves2 << move;

            bool open = pileOpen(pile, player->objectName());

            room->notifyMoveCards(true, moves2, open, players);
            room->notifyMoveCards(false, moves2, open, players);
        }
    }

    if (player == this || hasShownOneGeneral()) {
        foreach(QString mark_name, marks.keys()) {
            if (mark_name.startsWith("@")) {
                int value = getMark(mark_name);
                if (value > 0) {
                    JsonArray arg;
                    arg << objectName();
                    arg << mark_name;
                    arg << value;
                    room->doNotify(player, S_COMMAND_SET_MARK, arg);
                }
            }
        }
        room->notifyProperty(player, this, "kingdom");
        room->notifyProperty(player, this, "role");
    }
    else {
        room->notifyProperty(player, this, "kingdom", "god");
    }

    foreach(const QString &flag, flags)
        room->notifyProperty(player, this, "flags", flag);

    foreach(const QString &item, history.keys()) {
        int value = history.value(item);
        if (value > 0) {

            JsonArray arg;
            arg << item;
            arg << value;

            room->doNotify(player, S_COMMAND_ADD_HISTORY, arg);
        }
    }
}

void ServerPlayer::addToPile(const QString &pile_name, const Card *card, bool open, QList<ServerPlayer *> open_players) {
    QList<int> card_ids;
    if (card->isVirtualCard())
        card_ids = card->getSubcards();
    else
        card_ids << card->getEffectiveId();
    return addToPile(pile_name, card_ids, open, open_players);
}

void ServerPlayer::addToPile(const QString &pile_name, int card_id, bool open, QList<ServerPlayer *> open_players) {
    QList<int> card_ids;
    card_ids << card_id;
    return addToPile(pile_name, card_ids, open, open_players);
}

void ServerPlayer::addToPile(const QString &pile_name, QList<int> card_ids, bool open, QList<ServerPlayer *> open_players) {
    return addToPile(pile_name, card_ids, open, open_players, CardMoveReason());
}

void ServerPlayer::addToPile(const QString &pile_name, QList<int> card_ids,
                             bool open, QList<ServerPlayer *> open_players, CardMoveReason reason) {
    if (!open) {
        if (open_players.isEmpty()) {
            foreach(int id, card_ids) {
                ServerPlayer *owner = room->getCardOwner(id);
                if (owner && !open_players.contains(owner))
                    open_players << owner;
            }
        }
    } else {
        open_players = room->getAllPlayers();
    }
    foreach(ServerPlayer *p, open_players)
        setPileOpen(pile_name, p->objectName());
    piles[pile_name].append(card_ids);

    CardsMoveStruct move;
    move.card_ids = card_ids;
    move.to = this;
    move.to_place = Player::PlaceSpecial;
    move.reason = reason;
    room->moveCardsAtomic(move, open);
}

void ServerPlayer::gainAnExtraTurn() {
    QStringList extraTurnList;
    if (!room->getTag("ExtraTurnList").isNull())
        extraTurnList = room->getTag("ExtraTurnList").toStringList();
    extraTurnList.prepend(objectName());
    room->setTag("ExtraTurnList", QVariant::fromValue(extraTurnList));
}

void ServerPlayer::copyFrom(ServerPlayer *sp) {
    ServerPlayer *b = this;
    ServerPlayer *a = sp;

    b->handcards = QList<const Card *>(a->handcards);
    b->phases = QList<ServerPlayer::Phase>(a->phases);
    b->selected = QStringList(a->selected);

    Player *c = b;
    c->copyFrom(a);
}

bool ServerPlayer::CompareByActionOrder(ServerPlayer *a, ServerPlayer *b) {
    Room *room = a->getRoom();
    return room->getFront(a, b) == a;
}

void ServerPlayer::showGeneral(bool head_general, bool trigger_event, bool sendLog) {
    QStringList names = room->getTag(objectName()).toStringList();
    if (names.isEmpty()) return;
    QString general_name;

    room->tryPause();

    if (head_general) {
        if (getGeneralName() != "anjiang") return;

        setSkillsPreshowed("h");
        notifyPreshow();
        room->setPlayerProperty(this, "general1_showed", true);

        general_name = names.first();

        JsonArray arg;
        arg << (int) S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << general_name;
        arg << false;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral(this, general_name);

        sendSkillsToOthers();

        if (property("Duanchang").toString() != "head")
            foreach(const Skill *skill, getHeadSkillList()) {
            if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()
                && (!skill->isLordSkill() || hasLordSkill(skill->objectName()))
                && hasShownSkill(skill)) {
                JsonArray arg;
                arg << objectName();
                arg << skill->getLimitMark();
                arg << getMark(skill->getLimitMark());
                room->doBroadcastNotify(QSanProtocol::S_COMMAND_SET_MARK, arg);
            }
        }

        foreach (ServerPlayer *p, room->getOtherPlayers(this, true))
            room->notifyProperty(p, this, "head_skin_id");

        if (!hasShownGeneral2()) {
            QString kingdom = room->getMode() == "custom_scenario" ? getKingdom() : getGeneral()->getKingdom();
            room->setPlayerProperty(this, "kingdom", kingdom);

            QString role = HegemonyMode::GetMappedRole(kingdom);
            int i = 1;
            bool has_lord = isAlive() && getGeneral()->isLord();
            if (!has_lord) {
                foreach(ServerPlayer *p, room->getOtherPlayers(this, true)) {
                    if (p->getKingdom() == kingdom) {
                        if (p->getGeneral()->isLord()) {
                            has_lord = true;
                            break;
                        }
                        if (p->hasShownOneGeneral() && p->getRole() != "careerist")
                            ++i;
                    }
                }
            }

            if ((!has_lord && i > (room->getPlayers().length() / 2)) || (has_lord && getLord(true)->isDead()))
                role = "careerist";

            room->setPlayerProperty(this, "role", role);
        }

        if (isLord()) {
            QString kingdom = getKingdom();
            foreach(ServerPlayer *p, room->getPlayers()) {
                if (p->getKingdom() == kingdom && p->getRole() == "careerist"){
                    room->setPlayerProperty(p, "role", HegemonyMode::GetMappedRole(kingdom));
                    room->broadcastProperty(p, "kingdom");
                }
            }
        }
    } else {
        if (getGeneral2Name() != "anjiang") return;

        setSkillsPreshowed("d");
        notifyPreshow();
        room->setPlayerProperty(this, "general2_showed", true);

        general_name = names.at(1);
        JsonArray arg;
        arg << S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << general_name;
        arg << true;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral2(this, general_name);

        sendSkillsToOthers(false);

        if (property("Duanchang").toString() != "deputy"){
            foreach(const Skill *skill, getDeputySkillList()) {
                if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()
                    && (!skill->isLordSkill() || hasLordSkill(skill->objectName()))
                    && hasShownSkill(skill)) {
                    JsonArray arg;
                    arg << objectName();
                    arg << skill->getLimitMark();
                    arg << getMark(skill->getLimitMark());
                    room->doBroadcastNotify(QSanProtocol::S_COMMAND_SET_MARK, arg);
                }
            }
        }

        foreach (ServerPlayer *p, room->getOtherPlayers(this, true))
            room->notifyProperty(p, this, "deputy_skin_id");

        if (!hasShownGeneral1()) {
            QString kingdom = getGeneral2()->getKingdom();
            room->setPlayerProperty(this, "kingdom", kingdom);

            QString role = HegemonyMode::GetMappedRole(kingdom);
            int i = 1;
            bool has_lord = isAlive() && getGeneral()->isLord();
            if (!has_lord) {
                foreach(ServerPlayer *p, room->getOtherPlayers(this, true)) {
                    if (p->getKingdom() == kingdom) {
                        if (p->getGeneral()->isLord()) {
                            has_lord = true;
                            break;
                        }
                        if (p->hasShownOneGeneral() && p->getRole() != "careerist")
                            ++i;
                    }
                }
            }

            if ((!has_lord && i > (room->getPlayers().length() / 2))
                || (has_lord && getLord(true)->isDead()))
                role = "careerist";

            room->setPlayerProperty(this, "role", role);
        }
    }

    if (sendLog) {
        LogMessage log;
        log.type = "#BasaraReveal";
        log.from = this;
        log.arg = getGeneralName();
        log.arg2 = getGeneral2Name();
        room->sendLog(log);
    }

    if (trigger_event) {
        Q_ASSERT(room->getThread() != NULL);
        QVariant _head = head_general;
        room->getThread()->trigger(GeneralShown, room, this, _head);
    }

    room->filterCards(this, getCards("he"), true);
}

void ServerPlayer::hideGeneral(bool head_general) {
    room->tryPause();

    if (head_general) {
        if (getGeneralName() == "anjiang") return;

        setSkillsPreshowed("h", false);
        // dirty hack for temporary convenience.
        room->setPlayerProperty(this, "flags", "hiding");
        notifyPreshow();
        room->setPlayerProperty(this, "general1_showed", false);
        room->setPlayerProperty(this, "flags", "-hiding");

        JsonArray arg;
        arg << (int) S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << "anjiang";
        arg << false;
        arg << false;
        foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral(this, "anjiang");

        disconnectSkillsFromOthers();

        foreach(const Skill *skill, getVisibleSkillList()) {
            if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()
                && (!skill->isLordSkill() || hasLordSkill(skill->objectName()))
                && !hasShownSkill(skill) && getMark(skill->getLimitMark()) > 0) {
                JsonArray arg;
                arg << objectName();
                arg << skill->getLimitMark();
                arg << 0;
                foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
                    room->doNotify(p, QSanProtocol::S_COMMAND_SET_MARK, arg);
            }
        }

        if (!hasShownGeneral2()) {
            room->setPlayerProperty(this, "kingdom", "god");
            room->setPlayerProperty(this, "role", HegemonyMode::GetMappedRole("god"));
        }
    } else {
        if (getGeneral2Name() == "anjiang") return;

        setSkillsPreshowed("d", false);
        // dirty hack for temporary convenience
        room->setPlayerProperty(this, "flags", "hiding");
        notifyPreshow();
        room->setPlayerProperty(this, "general2_showed", false);
        room->setPlayerProperty(this, "flags", "-hiding");

        JsonArray arg;
        arg << (int) S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << "anjiang";
        arg << true;
        arg << false;
        foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral2(this, "anjiang");

        disconnectSkillsFromOthers(false);

        foreach(const Skill *skill, getVisibleSkillList()) {
            if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()
                && (!skill->isLordSkill() || hasLordSkill(skill->objectName()))
                && !hasShownSkill(skill) && getMark(skill->getLimitMark()) > 0) {
                JsonArray arg;
                arg << objectName();
                arg << skill->getLimitMark();
                arg << 0;
                foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
                    room->doNotify(p, QSanProtocol::S_COMMAND_SET_MARK, arg);
            }
        }

        if (!hasShownGeneral1()) {
            room->setPlayerProperty(this, "kingdom", "god");
            room->setPlayerProperty(this, "role", HegemonyMode::GetMappedRole("god"));
        }
    }

    LogMessage log;
    log.type = "#BasaraConceal";
    log.from = this;
    log.arg = getGeneralName();
    log.arg2 = getGeneral2Name();
    room->sendLog(log);

    Q_ASSERT(room->getThread() != NULL);
    QVariant _head = head_general;
    room->getThread()->trigger(GeneralHidden, room, this, _head);

    room->filterCards(this, getCards("he"), true);
}

void ServerPlayer::removeGeneral(bool head_general) {
    QString general_name, from_general;

    room->tryPause();

    room->setEmotion(this, "remove");

    if (head_general) {
        if (!hasShownGeneral1())
            showGeneral();   //zoushi?

        from_general = getActualGeneral1Name();
        if (from_general.contains("sujiang")) return;
        General::Gender gender = getActualGeneral1()->getGender();
        QString kingdom = getActualGeneral1()->getKingdom();
        general_name = gender == General::Male ? "sujiang" : "sujiangf";

        room->setPlayerProperty(this, "actual_general1", general_name);
        room->setPlayerProperty(this, "general1_showed", true);

        JsonArray arg;
        arg << (int) S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << general_name;
        arg << false;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral(this, general_name);

        setSkillsPreshowed("h", false);
        disconnectSkillsFromOthers();

        foreach(const Skill *skill, getHeadSkillList()){
            if (skill)
                room->detachSkillFromPlayer(this, skill->objectName(), true);
        }
        if (!hasShownGeneral2()) {
            room->setPlayerProperty(this, "kingdom", kingdom);

            QString role = HegemonyMode::GetMappedRole(kingdom);
            int i = 1;
            bool has_lord = isAlive() && getGeneral()->isLord();
            if (!has_lord) {
                foreach(ServerPlayer *p, room->getOtherPlayers(this, true)) {
                    if (p->getKingdom() == kingdom) {
                        if (p->isAlive() && p->getGeneral()->isLord()) {
                            has_lord = true;
                            break;
                        }
                        if (p->hasShownOneGeneral() && p->getRole() != "careerist")
                            ++i;
                    }
                }
            }

            if (!has_lord && i > (room->getPlayers().length() / 2))
                role = "careerist";

            room->setPlayerProperty(this, "role", role);
        }
    } else {
        if (!hasShownGeneral2())
            showGeneral(false); //zoushi?

        from_general = getActualGeneral2Name();
        if (from_general.contains("sujiang")) return;
        General::Gender gender = getActualGeneral2()->getGender();
        QString kingdom = getActualGeneral2()->getKingdom();
        general_name = gender == General::Male ? "sujiang" : "sujiangf";

        room->setPlayerProperty(this, "actual_general2", general_name);
        room->setPlayerProperty(this, "general2_showed", true);

        JsonArray arg;
        arg << (int) S_GAME_EVENT_CHANGE_HERO;
        arg << objectName();
        arg << general_name;
        arg << true;
        arg << false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral2(this, general_name);

        setSkillsPreshowed("d", false);
        disconnectSkillsFromOthers(false);

        foreach(const Skill *skill, getDeputySkillList()){
            if (skill)
                room->detachSkillFromPlayer(this, skill->objectName());
        }
        if (!hasShownGeneral1()) {
            room->setPlayerProperty(this, "kingdom", kingdom);

            QString role = HegemonyMode::GetMappedRole(kingdom);
            int i = 1;
            bool has_lord = isAlive() && getGeneral()->isLord();
            if (!has_lord) {
                foreach(ServerPlayer *p, room->getOtherPlayers(this, true)) {
                    if (p->getKingdom() == kingdom) {
                        if (p->isAlive() && p->getGeneral()->isLord()) {
                            has_lord = true;
                            break;
                        }
                        if (p->hasShownOneGeneral() && p->getRole() != "careerist")
                            ++i;
                    }
                }
            }

            if (!has_lord && i > (room->getPlayers().length() / 2))
                role = "careerist";

            room->setPlayerProperty(this, "role", role);
        }
    }

    LogMessage log;
    log.type = "#BasaraRemove";
    log.from = this;
    log.arg = head_general ? "head_general" : "deputy_general";
    log.arg2 = from_general;
    room->sendLog(log);

    Q_ASSERT(room->getThread() != NULL);
    QVariant _from = from_general;
    room->getThread()->trigger(GeneralRemoved, room, this, _from);

    room->filterCards(this, getCards("he"), true);
}

void ServerPlayer::sendSkillsToOthers(bool head_skill /* = true */) {
    QStringList names = room->getTag(objectName()).toStringList();
    if (names.isEmpty()) return;

    QString general = head_skill ? names.first() : names.last();
    foreach(const Skill *skill, Sanguosha->getGeneral(general)->getSkillList(true, head_skill)) {
        JsonArray args;
        args << QSanProtocol::S_GAME_EVENT_ADD_SKILL;
        args << objectName();
        args << skill->objectName();
        args << head_skill;
        foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, QSanProtocol::S_COMMAND_LOG_EVENT, args);
    }
}

void ServerPlayer::disconnectSkillsFromOthers(bool head_skill /* = true */) {
    foreach(QString skill, head_skill ? head_skills.keys() : deputy_skills.keys()) {
        QVariant _skill = skill;
        room->getThread()->trigger(EventLoseSkill, room, this, _skill);
        JsonArray args;
        args << (int) QSanProtocol::S_GAME_EVENT_DETACH_SKILL;
        args << objectName();
        args << skill;
        foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, QSanProtocol::S_COMMAND_LOG_EVENT, args);
    }

}

bool ServerPlayer::askForGeneralShow(bool one, bool refusable) {
    if (hasShownAllGenerals())
        return false;

    QStringList choices;

    if (!hasShownGeneral1() && disableShow(true).isEmpty())
        choices << "show_head_general";
    if (!hasShownGeneral2() && disableShow(false).isEmpty())
        choices << "show_deputy_general";
    if (choices.isEmpty())
        return false;
    if (!one && choices.length() == 2)
        choices << "show_both_generals";
    if (refusable)
        choices.append("cancel");

    QString choice = room->askForChoice(this, "GameRule_AskForGeneralShow", choices.join("+"));

    if (choice == "show_head_general" || choice == "show_both_generals")
        showGeneral();
    if (choice == "show_deputy_general" || choice == "show_both_generals")
        showGeneral(false);

    return choice.startsWith("s");
}

void ServerPlayer::notifyPreshow() {
    JsonArray args;
    args << (int) S_GAME_EVENT_UPDATE_PRESHOW;
    JsonObject args1;
    foreach(const QString skill, head_skills.keys() + deputy_skills.keys()) {
        args1.insert(skill, head_skills.value(skill, false)
            || deputy_skills.value(skill, false));
    }
    args << args1;
    room->doNotify(this, S_COMMAND_LOG_EVENT, args);

    JsonArray args2;
    args2 << (int) QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
    room->doNotify(this, QSanProtocol::S_COMMAND_LOG_EVENT, args2);
}

bool ServerPlayer::inSiegeRelation(const ServerPlayer *skill_owner, const ServerPlayer *victim) const {
    if (isFriendWith(victim) || !isFriendWith(skill_owner) || !victim->hasShownOneGeneral()) return false;
    if (this == skill_owner)
        return (getNextAlive() == victim && getNextAlive(2)->isFriendWith(this))
        || (getLastAlive() == victim && getLastAlive(2)->isFriendWith(this));
    else
        return (getNextAlive() == victim && getNextAlive(2) == skill_owner)
        || (getLastAlive() == victim && getLastAlive(2) == skill_owner);
}

bool ServerPlayer::inFormationRalation(ServerPlayer *teammate) const {
    QList<const Player *> teammates = getFormation();
    return teammates.length() > 1 && teammates.contains(teammate);
}

using namespace HegemonyMode;

void ServerPlayer::summonFriends(const ArrayType type) {
    room->tryPause();

    if (aliveCount() < 4) return;
    LogMessage log;
    log.type = "#InvokeSkill";
    log.from = this;
    log.arg = "GameRule_AskForArraySummon";
    room->sendLog(log);
    LogMessage log2;
    log2.type = "#SummonType";
    log2.arg = type == Siege ? "summon_type_siege" : "summon_type_formation";
    room->sendLog(log2);
    switch (type) {
    case Siege: {
        if (isFriendWith(getNextAlive()) && isFriendWith(getLastAlive())) return;
        QString prompt = "SiegeSummon";
        bool failed = true;
        if (!isFriendWith(getNextAlive())) {
            ServerPlayer *target = qobject_cast<ServerPlayer *>(getNextAlive(2));
            if (!target->hasShownOneGeneral()) {
                if (!target->willBeFriendWith(this))
                    prompt += "!";
                bool success = room->askForSkillInvoke(target, prompt);
                LogMessage log;
                log.type = "#SummonResult";
                log.from = target;
                log.arg = success ? "summon_success" : "summon_failed";
                room->sendLog(log);
                if (success) {
                    target->askForGeneralShow();
                    failed = false;
                }
            }
        }
        if (!isFriendWith(getLastAlive())) {
            ServerPlayer *target = qobject_cast<ServerPlayer *>(getLastAlive(2));
            if (!target->hasShownOneGeneral()) {
                if (!target->willBeFriendWith(this))
                    prompt += "!";
                bool success = room->askForSkillInvoke(target, prompt);
                LogMessage log;
                log.type = "#SummonResult";
                log.from = target;
                log.arg = success ? "summon_success" : "summon_failed";
                room->sendLog(log);
                if (success) {
                    target->askForGeneralShow();
                    failed = false;
                }
            }
        }
        if (failed)
            room->setPlayerFlag(this, "Global_SummonFailed");
        break;
    } case Formation: {
        int n = aliveCount(false);
        int asked = n;
        bool failed = true;
        for (int i = 1; i < n; ++i) {
            ServerPlayer *target = qobject_cast<ServerPlayer *>(getNextAlive(i));
            if (isFriendWith(target))
                continue;
            else if (!target->hasShownOneGeneral()) {
                QString prompt = "FormationSummon";
                if (!target->willBeFriendWith(this))
                    prompt += "!";

                bool success = room->askForSkillInvoke(target, prompt);
                LogMessage log;
                log.type = "#SummonResult";
                log.from = target;
                log.arg = success ? "summon_success" : "summon_failed";
                room->sendLog(log);

                if (success) {
                    target->askForGeneralShow();
                    failed = false;
                }
                else {
                    asked = i;
                    break;
                }
            }
            else {
                asked = i;
                break;
            }
        }

        n -= asked;
        for (int i = 1; i < n; ++i) {
            ServerPlayer *target = qobject_cast<ServerPlayer *>(getLastAlive(i));
            if (isFriendWith(target))
                continue;
            else {
                if (!target->hasShownOneGeneral()) {
                    QString prompt = "FormationSummon";
                    if (!target->willBeFriendWith(this))
                        prompt += "!";

                    bool success = room->askForSkillInvoke(target, prompt);
                    LogMessage log;
                    log.type = "#SummonResult";
                    log.from = target;
                    log.arg = success ? "summon_success" : "summon_failed";
                    room->sendLog(log);

                    if (success) {
                        target->askForGeneralShow();
                        failed = false;
                    }
                }
                break;
            }
        }
        if (failed)
            room->setPlayerFlag(this, "Global_SummonFailed");
        break;
    }
    }
}

QHash<QString, QStringList> ServerPlayer::getBigAndSmallKingdoms(const QString &reason, MaxCardsType::MaxCardsCount _type) const
{
    ServerPlayer *jade_seal_owner = NULL;
    foreach (ServerPlayer *p, room->getAlivePlayers()) {
        if (p->hasTreasure("JadeSeal") && p->hasShownOneGeneral()) {
            jade_seal_owner = p;
            break;
        }
    }
    MaxCardsType::MaxCardsCount type = jade_seal_owner ? MaxCardsType::Max : _type;
    // if there is someone has JadeSeal, needn't trigger event because of the fucking effect of JadeSeal
    QMap<QString, int> kingdom_map;
    QStringList kingdoms = Sanguosha->getKingdoms();
    kingdoms << "careerist";
    foreach (QString kingdom, kingdoms) {
        if (kingdom == "god") continue;
        kingdom_map.insert(kingdom, getPlayerNumWithSameKingdom(reason, kingdom, type));
    }
    foreach (ServerPlayer *p, room->getAlivePlayers()) {
        if (!p->hasShownOneGeneral()) {
            kingdom_map.insert("anjiang", 1);
            break;
        }
    }
    QHash<QString, QStringList> big_n_small;
    big_n_small.insert("big", QStringList());
    big_n_small.insert("small", QStringList());
    foreach (QString key, kingdom_map.keys()) {
        if (kingdom_map[key] == 0)
            continue;
        if (big_n_small["big"].isEmpty()) {
            big_n_small["big"] << key;
            continue;
        }
        if (kingdom_map[key] == kingdom_map[big_n_small["big"].first()]) {
            big_n_small["big"] << key;
        } else if (kingdom_map[key] > kingdom_map[big_n_small["big"].first()]) {
            big_n_small["small"] << big_n_small["big"];
            big_n_small["big"].clear();
            big_n_small["big"] << key;
        } else if (kingdom_map[key] < kingdom_map[big_n_small["big"].first()]) {
            big_n_small["small"] << key;
        }
    }
    if (jade_seal_owner != NULL) {
        if (jade_seal_owner->getRole() == "careerist") {
            big_n_small["small"] << big_n_small["big"];
            big_n_small["big"].clear();
            big_n_small["big"] << jade_seal_owner->objectName(); // record player's objectName who has JadeSeal.
        } else { // has shown one general but isn't careerist
            QString kingdom = jade_seal_owner->getKingdom();
            big_n_small["small"] << big_n_small["big"];
            big_n_small["big"].clear();
            big_n_small["small"].removeOne(kingdom);
            big_n_small["big"] << kingdom;
        }
    }
    return big_n_small;
}

void ServerPlayer::changeToLord() {
    foreach(QString skill_name, head_skills.keys()) {
        Player::loseSkill(skill_name);
        JsonArray arg_loseskill;
        arg_loseskill << (int)QSanProtocol::S_GAME_EVENT_LOSE_SKILL;
        arg_loseskill << objectName();
        arg_loseskill << skill_name;
        room->doNotify(this, QSanProtocol::S_COMMAND_LOG_EVENT, arg_loseskill);

        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill != NULL) {
            if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty())
                room->setPlayerMark(this, skill->getLimitMark(), 0);
        }
    }


    QStringList real_generals = room->getTag(objectName()).toStringList();
    QString name = real_generals.takeFirst();
    name.prepend("lord_");
    real_generals.prepend(name);
    room->setTag(objectName(), real_generals);

    room->setPlayerMark(this, "CompanionEffect", 1);

    const General *lord = Sanguosha->getGeneral(name);
    const General *deputy = Sanguosha->getGeneral(real_generals.last());
    Q_ASSERT(lord != NULL && deputy != NULL);
    int doubleMaxHp = lord->getMaxHpHead() + deputy->getMaxHpDeputy();
    room->setPlayerMark(this, "HalfMaxHpLeft", doubleMaxHp % 2);

    setMaxHp(doubleMaxHp / 2);
    setHp(doubleMaxHp / 2);

    room->broadcastProperty(this, "maxhp");
    room->broadcastProperty(this, "hp");

    JsonArray arg_changehero;
    arg_changehero << (int)S_GAME_EVENT_CHANGE_HERO;
    arg_changehero << objectName();
    arg_changehero << name;
    arg_changehero << false;
    arg_changehero << false;
    room->doNotify(this, QSanProtocol::S_COMMAND_LOG_EVENT, arg_changehero);

    foreach(const Skill *skill, lord->getVisibleSkillList(true)) {
        addSkill(skill->objectName());

        if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()) {
            setMark(skill->getLimitMark(), 1);
            JsonArray arg;
            arg << objectName();
            arg << skill->getLimitMark();
            arg << 1;
            room->doNotify(this, S_COMMAND_SET_MARK, arg);
        }
    }

}

#ifndef QT_NO_DEBUG
bool ServerPlayer::event(QEvent *event) {
#define SET_MY_PROPERTY {\
    ServerPlayerEvent *SPEvent = static_cast<ServerPlayerEvent *>(event); \
    setProperty(SPEvent->property_name, SPEvent->value); \
    room->broadcastProperty(this, SPEvent->property_name); \
    event_received = true; \
}
    if (event->type() == QEvent::User) {
        if (semas[SEMA_MUTEX]) {
            semas[SEMA_MUTEX]->acquire();
            SET_MY_PROPERTY;
            semas[SEMA_MUTEX]->release();
        }
        else
            SET_MY_PROPERTY;
    }
    return Player::event(event);
}

ServerPlayerEvent::ServerPlayerEvent(char *property_name, QVariant &value)
    : QEvent(QEvent::User), property_name(property_name), value(value)
{

}
#endif

