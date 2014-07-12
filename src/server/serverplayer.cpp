/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#include "serverplayer.h"
#include "engine.h"
#include "standard.h"
#include "ai.h"
#include "settings.h"
#include "recorder.h"
#include "lua-wrapper.h"
#include "jsonutils.h"
#include "skill.h"

using namespace QSanProtocol;
using namespace QSanProtocol::Utils;

const int ServerPlayer::S_NUM_SEMAPHORES = 6;

ServerPlayer::ServerPlayer(Room *room)
    : Player(room), m_isClientResponseReady(false), m_isWaitingReply(false),
    event_received(false), socket(NULL), room(room),
    ai(NULL), trust_ai(new TrustAI(this)), recorder(NULL),
    _m_phases_index(0), _m_clientResponse(Json::nullValue)
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
    QString name = card_name;
    if (name.startsWith("heg_"))
        name.remove("heg_");
    room->broadcastSkillInvoke(name, isMale(), -1);
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
    }
    else {
        int index = skill->getEffectIndex(this, card);
        if (index == 0) return;

        if ((index == -1 && skill->getSources().isEmpty()) || index == -2) {
            if (card->getCommonEffectName().isNull())
                broadcastSkillInvoke(card->objectName());
            else
                room->broadcastSkillInvoke(card->getCommonEffectName(), "common");
        }
        else
            room->broadcastSkillInvoke(skill_name, index);
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

void ServerPlayer::clearOnePrivatePile(QString pile_name) {
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

int ServerPlayer::aliveCount() const{
    return room->alivePlayerCount();
}

int ServerPlayer::getHandcardNum() const{
    return handcards.length();
}

void ServerPlayer::setSocket(ClientSocket *socket) {
    if (socket) {
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(message_got(const char *)), this, SLOT(getMessage(const char *)));
        connect(this, SIGNAL(message_ready(QString)), this, SLOT(sendMessage(QString)));
    }
    else {
        if (this->socket) {
            this->disconnect(this->socket);
            this->socket->disconnect(this);
            //this->socket->disconnectFromHost();
            this->socket->deleteLater();
        }

        disconnect(this, SLOT(sendMessage(QString)));
    }

    this->socket = socket;
}

void ServerPlayer::kick(){
    room->notifyProperty(this, this, "flags", "is_kicked");
    if (socket != NULL)
        socket->disconnectFromHost();
    setSocket(NULL);
}

void ServerPlayer::getMessage(const char *message) {
    QString request = QString::fromUtf8(message);
    if (request.endsWith("\n"))
        request.chop(1);

    emit request_got(request);
}

void ServerPlayer::unicast(const QString &message) {
    emit message_ready(message);

    if (recorder)
        recorder->recordLine(message);
}

void ServerPlayer::startNetworkDelayTest() {
    test_time = QDateTime::currentDateTime();
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, S_COMMAND_NETWORK_DELAY_TEST);
    invoke(&packet);
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

void ServerPlayer::sendMessage(const QString &message) {
    if (socket) {
#ifndef QT_NO_DEBUG
        printf("%s", qPrintable(objectName()));
#endif
        socket->send(message.toUtf8());
    }
}

void ServerPlayer::invoke(const AbstractPacket *packet) {
    unicast(packet->toString());
}

void ServerPlayer::invoke(const char *method, const QString &arg) {
    unicast(QString("%1 %2").arg(method).arg(arg));
}

void ServerPlayer::notify(CommandType type, const Json::Value &arg){
    Packet packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, type);
    packet.setMessageBody(arg);
    unicast(packet.toString());
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
    foreach(const Card *card, handcards) {
        if (card->isKindOf("Nullification"))
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

bool ServerPlayer::pindian(ServerPlayer *target, const QString &reason, const Card *card1) {
    LogMessage log;
    log.type = "#Pindian";
    log.from = this;
    log.to << target;
    room->sendLog(log);

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

    if (card1 == NULL || card2 == NULL) return false;

    PindianStruct pindian_struct;
    pindian_struct.from = this;
    pindian_struct.to = target;
    pindian_struct.from_card = card1;
    pindian_struct.to_card = card2;
    pindian_struct.from_number = card1->getNumber();
    pindian_struct.to_number = card2->getNumber();
    pindian_struct.reason = reason;

    QList<CardsMoveStruct> pd_move;
    CardsMoveStruct move1;
    move1.card_ids << pindian_struct.from_card->getEffectiveId();
    move1.from = pindian_struct.from;
    move1.to = NULL;
    move1.to_place = Player::PlaceTable;
    CardMoveReason reason1(CardMoveReason::S_REASON_PINDIAN, pindian_struct.from->objectName(), pindian_struct.to->objectName(),
        pindian_struct.reason, QString());
    move1.reason = reason1;

    CardsMoveStruct move2;
    move2.card_ids << pindian_struct.to_card->getEffectiveId();
    move2.from = pindian_struct.to;
    move2.to = NULL;
    move2.to_place = Player::PlaceTable;
    CardMoveReason reason2(CardMoveReason::S_REASON_PINDIAN, pindian_struct.to->objectName());
    move2.reason = reason2;

    pd_move << move1 << move2;

    room->moveCardsAtomic(pd_move, true);

    LogMessage log2;
    log2.type = "$PindianResult";
    log2.from = pindian_struct.from;
    log2.card_str = QString::number(pindian_struct.from_card->getEffectiveId());
    room->sendLog(log2);

    log2.type = "$PindianResult";
    log2.from = pindian_struct.to;
    log2.card_str = QString::number(pindian_struct.to_card->getEffectiveId());
    room->sendLog(log2);

    RoomThread *thread = room->getThread();
    PindianStruct *pindian_star = &pindian_struct;
    QVariant data = QVariant::fromValue(pindian_star);
    Q_ASSERT(thread != NULL);
    thread->trigger(PindianVerifying, room, this, data);

    PindianStruct *new_star = data.value<PindianStruct *>();
    pindian_struct.from_number = new_star->from_number;
    pindian_struct.to_number = new_star->to_number;
    pindian_struct.success = (new_star->from_number > new_star->to_number);

    log.type = pindian_struct.success ? "#PindianSuccess" : "#PindianFailure";
    log.from = this;
    log.to.clear();
    log.to << target;
    log.card_str.clear();
    room->sendLog(log);

    Json::Value arg(Json::arrayValue);
    arg[0] = (int)S_GAME_EVENT_REVEAL_PINDIAN;
    arg[1] = toJsonString(objectName());
    arg[2] = pindian_struct.from_card->getEffectiveId();
    arg[3] = toJsonString(target->objectName());
    arg[4] = pindian_struct.to_card->getEffectiveId();
    arg[5] = pindian_struct.success;
    arg[6] = toJsonString(reason);
    room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);

    pindian_star = &pindian_struct;
    data = QVariant::fromValue(pindian_star);
    thread->trigger(Pindian, room, this, data);

    pd_move.clear();

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
        .arg(reason)
        .arg(this->objectName())
        .arg(pindian_struct.from_card->getEffectiveId())
        .arg(target->objectName())
        .arg(pindian_struct.to_card->getEffectiveId()));
    thread->trigger(ChoiceMade, room, this, decisionData);

    return pindian_struct.success;
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
    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_ADD_SKILL;
    args[1] = toJsonString(objectName());
    args[2] = toJsonString(skill_name);
    args[3] = head_skill;
    room->doNotify(this, QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void ServerPlayer::loseSkill(const QString &skill_name) {
    Player::loseSkill(skill_name);
    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_LOSE_SKILL;
    args[1] = toJsonString(objectName());
    args[2] = toJsonString(skill_name);
    room->doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void ServerPlayer::setGender(General::Gender gender) {
    if (gender == getGender())
        return;
    Player::setGender(gender);
    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_CHANGE_GENDER;
    args[1] = toJsonString(objectName());
    args[2] = (int)gender;
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

void ServerPlayer::setNext(ServerPlayer *next) {
    Player::setNext(next);
    room->broadcastProperty(this, "next");
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

    Json::Value introduce_str(Json::arrayValue);
    introduce_str.append(toJsonString(objectName()));
    introduce_str.append(toJsonString(screen_name));
    introduce_str.append(toJsonString(avatar));

    if (player) {
        player->notify(S_COMMAND_ADD_PLAYER, introduce_str);
        room->notifyProperty(player, this, "state");
    }
    else {
        room->doBroadcastNotify(S_COMMAND_ADD_PLAYER, introduce_str, this);
        room->broadcastProperty(this, "state");
    }

    if (hasShownGeneral1()) {
        foreach(const QString skill_name, head_skills.keys()) {
            if (Sanguosha->getSkill(skill_name)->isVisible()) {
                Json::Value args1;
                args1[0] = S_GAME_EVENT_ADD_SKILL;
                args1[1] = toJsonString(objectName());
                args1[2] = toJsonString(skill_name);
                args1[3] = true;
                room->doNotify(player, S_COMMAND_LOG_EVENT, args1);
            }

            foreach(const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)) {
                if (!related_skill->isVisible()) {
                    Json::Value args2;
                    args2[0] = S_GAME_EVENT_ADD_SKILL;
                    args2[1] = toJsonString(objectName());
                    args2[2] = toJsonString(related_skill->objectName());
                    args2[3] = true;
                    room->doNotify(player, S_COMMAND_LOG_EVENT, args2);
                }
            }
        }
    }

    if (hasShownGeneral2()) {
        foreach(const QString skill_name, deputy_skills.keys()) {
            if (Sanguosha->getSkill(skill_name)->isVisible()) {
                Json::Value args1;
                args1[0] = S_GAME_EVENT_ADD_SKILL;
                args1[1] = toJsonString(objectName());
                args1[2] = toJsonString(skill_name);
                args1[3] = false;
                room->doNotify(player, S_COMMAND_LOG_EVENT, args1);
            }

            foreach(const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)) {
                if (!related_skill->isVisible()) {
                    Json::Value args2;
                    args2[0] = S_GAME_EVENT_ADD_SKILL;
                    args2[1] = toJsonString(objectName());
                    args2[2] = toJsonString(related_skill->objectName());
                    args2[3] = false;
                    room->doNotify(player, S_COMMAND_LOG_EVENT, args2);
                }
            }
        }
    }
}

#include "gamerule.h"

void ServerPlayer::marshal(ServerPlayer *player) const{
    room->notifyProperty(player, this, "maxhp");
    room->notifyProperty(player, this, "hp");
    room->notifyProperty(player, this, "general1_showed");
    room->notifyProperty(player, this, "general2_showed");

    if (isAlive()) {
        room->notifyProperty(player, this, "seat");
        if (getPhase() != Player::NotActive)
            room->notifyProperty(player, this, "phase");
    }
    else {
        room->notifyProperty(player, this, "alive");
        room->notifyProperty(player, this, "role");
        room->doNotify(player, S_COMMAND_KILL_PLAYER, toJsonString(objectName()));
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
                    Json::Value arg(Json::arrayValue);
                    arg[0] = toJsonString(objectName());
                    arg[1] = toJsonString(mark_name);
                    arg[2] = value;
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

    foreach(QString flag, flags)
        room->notifyProperty(player, this, "flags", flag);

    foreach(QString item, history.keys()) {
        int value = history.value(item);
        if (value > 0) {

            Json::Value arg(Json::arrayValue);
            arg[0] = toJsonString(item);
            arg[1] = value;

            room->doNotify(player, S_COMMAND_ADD_HISTORY, arg);
        }
    }
}

void ServerPlayer::addToPile(const QString &pile_name, const Card *card, bool open) {
    QList<int> card_ids;
    if (card->isVirtualCard())
        card_ids = card->getSubcards();
    else
        card_ids << card->getEffectiveId();
    return addToPile(pile_name, card_ids, open);
}

void ServerPlayer::addToPile(const QString &pile_name, int card_id, bool open) {
    QList<int> card_ids;
    card_ids << card_id;
    return addToPile(pile_name, card_ids, open);
}

void ServerPlayer::addToPile(const QString &pile_name, QList<int> card_ids, bool open) {
    return addToPile(pile_name, card_ids, open, CardMoveReason());
}

void ServerPlayer::addToPile(const QString &pile_name, QList<int> card_ids, bool open, CardMoveReason reason) {
    QList<ServerPlayer *> open_players;
    if (!open) {
        foreach(int id, card_ids) {
            ServerPlayer *owner = room->getCardOwner(id);
            if (owner && !open_players.contains(owner))
                open_players << owner;
        }
    }
    else {
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

void ServerPlayer::exchangeFreelyFromPrivatePile(const QString &skill_name, const QString &pile_name, int upperlimit, bool include_equip) {
    QList<int> pile = getPile(pile_name);
    if (pile.isEmpty()) return;

    QString tempMovingFlag = QString("%1_InTempMoving").arg(skill_name);
    room->setPlayerFlag(this, tempMovingFlag);

    int ai_delay = Config.AIDelay;
    Config.AIDelay = 0;

    QList<int> will_to_pile, will_to_handcard;
    while (!pile.isEmpty()) {
        room->fillAG(pile, this);
        int card_id = room->askForAG(this, pile, true, skill_name);
        room->clearAG(this);
        if (card_id == -1) break;

        pile.removeOne(card_id);
        will_to_handcard << card_id;
        if (pile.length() >= upperlimit) break;

        CardMoveReason reason(CardMoveReason::S_REASON_EXCHANGE_FROM_PILE, this->objectName());
        room->obtainCard(this, Sanguosha->getCard(card_id), reason, false);
    }

    Config.AIDelay = ai_delay;

    int n = will_to_handcard.length();
    if (n == 0) return;
    const Card *exchange_card = room->askForExchange(this, skill_name, n, include_equip);
    will_to_pile = exchange_card->getSubcards();
    delete exchange_card;

    QList<int> will_to_handcard_x = will_to_handcard, will_to_pile_x = will_to_pile;
    QList<int> duplicate;
    foreach(int id, will_to_pile) {
        if (will_to_handcard_x.contains(id)) {
            duplicate << id;
            will_to_pile_x.removeOne(id);
            will_to_handcard_x.removeOne(id);
            n--;
        }
    }

    if (n == 0) {
        addToPile(pile_name, will_to_pile, false);
        room->setPlayerFlag(this, "-" + tempMovingFlag);
        return;
    }

    LogMessage log;
    log.type = "#QixingExchange";
    log.from = this;
    log.arg = QString::number(n);
    log.arg2 = skill_name;
    room->sendLog(log);

    addToPile(pile_name, duplicate, false);
    room->setPlayerFlag(this, "-" + tempMovingFlag);
    addToPile(pile_name, will_to_pile_x, false);

    room->setPlayerFlag(this, tempMovingFlag);
    addToPile(pile_name, will_to_handcard_x, false);
    room->setPlayerFlag(this, "-" + tempMovingFlag);

    DummyCard dummy(will_to_handcard_x);
    CardMoveReason reason(CardMoveReason::S_REASON_EXCHANGE_FROM_PILE, this->objectName());
    room->obtainCard(this, &dummy, reason, false);
}

void ServerPlayer::gainAnExtraTurn() {
    ServerPlayer *current = room->getCurrent();
    Player::Phase orig_phase = Player::NotActive;
    if (current != NULL && current->isAlive())
        orig_phase = current->getPhase();

    try {
        current->setPhase(Player::NotActive);
        room->broadcastProperty(current, "phase");

        room->setCurrent(this);
        room->getThread()->trigger(TurnStart, room, this);

        current->setPhase(orig_phase);
        room->broadcastProperty(current, "phase");
        room->setCurrent(current);
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == TurnBroken) {
            if (getPhase() != Player::NotActive) {
                const GameRule *game_rule = NULL;
                if (room->getMode() == "04_1v3")
                    game_rule = qobject_cast<const GameRule *>(Sanguosha->getTriggerSkill("hulaopass_mode"));
                else
                    game_rule = qobject_cast<const GameRule *>(Sanguosha->getTriggerSkill("game_rule"));
                if (game_rule){
                    QVariant _variant;
                    game_rule->effect(EventPhaseEnd, room, this, _variant, this);
                }
                changePhase(getPhase(), Player::NotActive);
            }
            current->setPhase(orig_phase);
            room->broadcastProperty(current, "phase");
            room->setCurrent(current);
        }
        throw triggerEvent;
    }
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

void ServerPlayer::showGeneral(bool head_general, bool trigger_event) {
    QStringList names = room->getTag(objectName()).toStringList();
    if (names.isEmpty()) return;
    QString general_name;

    if (head_general) {
        if (getGeneralName() != "anjiang") return;

        setSkillsPreshowed("h");
        notifyPreshow();
        room->setPlayerProperty(this, "general1_showed", true);

        general_name = names.first();

        Json::Value arg(Json::arrayValue);
        arg[0] = S_GAME_EVENT_CHANGE_HERO;
        arg[1] = toJsonString(objectName());
        arg[2] = toJsonString(general_name);
        arg[3] = false;
        arg[4] = false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral(this, general_name);

        sendSkillsToOthers();

        if (property("Duanchang").toString() != "head")
            foreach(const Skill *skill, getHeadSkillList()) {
            if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()
                && (!skill->isLordSkill() || hasLordSkill(skill->objectName()))
                && hasShownSkill(skill)) {
                Json::Value arg(Json::arrayValue);
                arg[0] = toJsonString(objectName());
                arg[1] = toJsonString(skill->getLimitMark());
                arg[2] = 1;
                room->doBroadcastNotify(QSanProtocol::S_COMMAND_SET_MARK, arg);
            }
        }

        if (!hasShownGeneral2()) {
            QString kingdom = getGeneral()->getKingdom();
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
    }
    else {
        if (getGeneral2Name() != "anjiang") return;

        setSkillsPreshowed("d");
        notifyPreshow();
        room->setPlayerProperty(this, "general2_showed", true);

        general_name = names.at(1);
        Json::Value arg(Json::arrayValue);
        arg[0] = S_GAME_EVENT_CHANGE_HERO;
        arg[1] = toJsonString(objectName());
        arg[2] = toJsonString(general_name);
        arg[3] = true;
        arg[4] = false;
        room->doBroadcastNotify(S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral2(this, general_name);

        sendSkillsToOthers(false);

        if (property("Duanchang").toString() != "deputy"){
            foreach(const Skill *skill, getDeputySkillList()) {
                if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()
                    && (!skill->isLordSkill() || hasLordSkill(skill->objectName()))
                    && hasShownSkill(skill)) {
                    Json::Value arg(Json::arrayValue);
                    arg[0] = toJsonString(objectName());
                    arg[1] = toJsonString(skill->getLimitMark());
                    arg[2] = 1;
                    room->doBroadcastNotify(QSanProtocol::S_COMMAND_SET_MARK, arg);
                }
            }
        }

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

    LogMessage log;
    log.type = "#BasaraReveal";
    log.from = this;
    log.arg = getGeneralName();
    log.arg2 = getGeneral2Name();
    room->sendLog(log);

    if (trigger_event) {
        Q_ASSERT(room->getThread() != NULL);
        QVariant _head = head_general;
        room->getThread()->trigger(GeneralShown, room, this, _head);
    }
    room->filterCards(this, getCards("he"), true);
}

void ServerPlayer::hideGeneral(bool head_general) {
    if (head_general) {
        if (getGeneralName() == "anjiang") return;

        setSkillsPreshowed("h", false);
        notifyPreshow();
        room->setPlayerProperty(this, "general1_showed", false);

        Json::Value arg(Json::arrayValue);
        arg[0] = S_GAME_EVENT_CHANGE_HERO;
        arg[1] = toJsonString(objectName());
        arg[2] = toJsonString("anjiang");
        arg[3] = false;
        arg[4] = false;
        foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral(this, "anjiang");

        disconnectSkillsFromOthers();

        foreach(const Skill *skill, getVisibleSkillList()) {
            if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()
                && (!skill->isLordSkill() || hasLordSkill(skill->objectName()))
                && !hasShownSkill(skill) && getMark(skill->getLimitMark()) > 0) {
                Json::Value arg(Json::arrayValue);
                arg[0] = toJsonString(objectName());
                arg[1] = toJsonString(skill->getLimitMark());
                arg[2] = 0;
                foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
                    room->doNotify(p, QSanProtocol::S_COMMAND_SET_MARK, arg);
            }
        }

        if (!hasShownGeneral2()) {
            room->setPlayerProperty(this, "kingdom", "god");
            room->setPlayerProperty(this, "role", HegemonyMode::GetMappedRole("god"));
        }
    }
    else {
        if (getGeneral2Name() == "anjiang") return;

        setSkillsPreshowed("d", false);
        notifyPreshow();
        room->setPlayerProperty(this, "general2_showed", false);

        Json::Value arg(Json::arrayValue);
        arg[0] = S_GAME_EVENT_CHANGE_HERO;
        arg[1] = toJsonString(objectName());
        arg[2] = toJsonString("anjiang");
        arg[3] = true;
        arg[4] = false;
        foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, S_COMMAND_LOG_EVENT, arg);
        room->changePlayerGeneral2(this, "anjiang");

        disconnectSkillsFromOthers(false);

        foreach(const Skill *skill, getVisibleSkillList()) {
            if (skill->getFrequency() == Skill::Limited && !skill->getLimitMark().isEmpty()
                && (!skill->isLordSkill() || hasLordSkill(skill->objectName()))
                && !hasShownSkill(skill) && getMark(skill->getLimitMark()) > 0) {
                Json::Value arg(Json::arrayValue);
                arg[0] = toJsonString(objectName());
                arg[1] = toJsonString(skill->getLimitMark());
                arg[2] = 0;
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

        Json::Value arg(Json::arrayValue);
        arg[0] = S_GAME_EVENT_CHANGE_HERO;
        arg[1] = toJsonString(objectName());
        arg[2] = toJsonString(general_name);
        arg[3] = false;
        arg[4] = false;
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
    }
    else {
        if (!hasShownGeneral2())
            showGeneral(false); //zoushi?

        from_general = getActualGeneral2Name();
        if (from_general.contains("sujiang")) return;
        General::Gender gender = getActualGeneral2()->getGender();
        QString kingdom = getActualGeneral2()->getKingdom();
        general_name = gender == General::Male ? "sujiang" : "sujiangf";

        room->setPlayerProperty(this, "actual_general2", general_name);
        room->setPlayerProperty(this, "general2_showed", true);

        Json::Value arg(Json::arrayValue);
        arg[0] = S_GAME_EVENT_CHANGE_HERO;
        arg[1] = toJsonString(objectName());
        arg[2] = toJsonString(general_name);
        arg[3] = true;
        arg[4] = false;
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
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_ADD_SKILL;
        args[1] = toJsonString(objectName());
        args[2] = toJsonString(skill->objectName());
        args[3] = head_skill;
        foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, QSanProtocol::S_COMMAND_LOG_EVENT, args);
    }
}

void ServerPlayer::disconnectSkillsFromOthers(bool head_skill /* = true */) {
    foreach(QString skill, head_skill ? head_skills.keys() : deputy_skills.keys()) {
        QVariant _skill = skill;
        room->getThread()->trigger(EventLoseSkill, room, this, _skill);
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_DETACH_SKILL;
        args[1] = toJsonString(objectName());
        args[2] = toJsonString(skill);
        foreach(ServerPlayer *p, room->getOtherPlayers(this, true))
            room->doNotify(p, QSanProtocol::S_COMMAND_LOG_EVENT, args);
    }

}

bool ServerPlayer::askForGeneralShow(bool one, bool refusable) {
    if (hasShownAllGenerals())
        return false;

    QStringList choices;

    if (!hasShownGeneral1())
        choices << "show_head_general";
    if (!hasShownGeneral2())
        choices << "show_deputy_general";

    if (!one && choices.length() == 2)
        choices << "show_both_generals";
    if (refusable)
        choices.prepend("cancel"); // default choice should do nothing

    QString choice = room->askForChoice(this, "TurnStartShowGeneral", choices.join("+"));

    if (choice == "show_head_general" || choice == "show_both_generals")
        showGeneral();
    if (choice == "show_deputy_general" || choice == "show_both_generals")
        showGeneral(false);

    return choice.startsWith("s");
}

void ServerPlayer::notifyPreshow() {
    Json::Value args;
    args[0] = S_GAME_EVENT_UPDATE_PRESHOW;
    Json::Value args1;
    foreach(const QString skill, head_skills.keys() + deputy_skills.keys()) {
        args1[skill.toLatin1().constData()] = head_skills.value(skill, false)
            || deputy_skills.value(skill, false);
    }
    args[1] = args1;
    room->doNotify(this, S_COMMAND_LOG_EVENT, args);

    Json::Value args2;
    args2[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
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

QList<ServerPlayer *> ServerPlayer::getFormation() const {
    QList<ServerPlayer *> teammates;
    teammates << room->findPlayer(objectName()); //avoid unsafe const_cast
    int n = aliveCount();
    int num = n;
    for (int i = 1; i < n; ++i) {
        ServerPlayer *target = qobject_cast<ServerPlayer *>(getNextAlive(i));
        if (isFriendWith(target))
            teammates << target;
        else {
            num = i;
            break;
        }
    }

    n -= num;
    for (int i = 1; i < n; ++i) {
        ServerPlayer *target = qobject_cast<ServerPlayer *>(getLastAlive(i));
        if (isFriendWith(target))
            teammates << target;
        else break;
    }

    return teammates;
}

bool ServerPlayer::inFormationRalation(ServerPlayer *teammate) const {
    QList<ServerPlayer *> teammates = getFormation();
    return teammates.length() > 1 && teammates.contains(teammate);
}

using namespace HegemonyMode;

void ServerPlayer::summonFriends(const ArrayType type) {
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
        int n = aliveCount();
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

