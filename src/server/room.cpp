#include "room.h"
#include "engine.h"
#include "settings.h"
#include "standard.h"
#include "ai.h"
#include "scenario.h"
#include "gamerule.h"
#include "scenerule.h"
#include "banpair.h"
#include "roomthread3v3.h"
#include "roomthreadxmode.h"
#include "roomthread1v1.h"
#include "server.h"
#include "generalselector.h"
#include "jsonutils.h"
#include "structs.h"
#include "miniscenarios.h"

#include <QStringList>
#include <QMessageBox>
#include <QHostAddress>
#include <QTimer>
#include <QMetaEnum>
#include <QTimerEvent>
#include <QDateTime>
#include <QFile>
#include <QTextStream>

#ifdef QSAN_UI_LIBRARY_AVAILABLE
#pragma message WARN("UI elements detected in server side!!!")
#endif

using namespace QSanProtocol;
using namespace QSanProtocol::Utils;

Room::Room(QObject *parent, const QString &mode)
    : QThread(parent), mode(mode), current(NULL), pile1(Sanguosha->getRandomCards()),
      m_drawPile(&pile1), m_discardPile(&pile2),
      game_started(false), game_finished(false), game_paused(false), L(NULL), thread(NULL),
      thread_3v3(NULL), thread_xmode(NULL), thread_1v1(NULL), _m_semRaceRequest(0), _m_semRoomMutex(1),
      _m_raceStarted(false), provided(NULL), has_provided(false),
      m_surrenderRequestReceived(false), _virtual(false), _m_roomState(false)
{
    static int s_global_room_id = 0;
    _m_Id = s_global_room_id++;
    _m_lastMovementId = 0;
    player_count = Sanguosha->getPlayerCount(mode);
    scenario = Sanguosha->getScenario(mode);

    initCallbacks();

    L = CreateLuaState();
    QStringList scripts;
    scripts << "lua/sanguosha.lua" << "lua/ai/smart-ai.lua";
    DoLuaScripts(L, scripts);
}

void Room::initCallbacks() {
    // init request response pair
    m_requestResponsePair[S_COMMAND_PLAY_CARD] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_NULLIFICATION] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_SHOW_CARD] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_ASK_PEACH] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_PINDIAN] = S_COMMAND_RESPONSE_CARD;
    m_requestResponsePair[S_COMMAND_EXCHANGE_CARD] = S_COMMAND_DISCARD_CARD;
    m_requestResponsePair[S_COMMAND_CHOOSE_DIRECTION] = S_COMMAND_MULTIPLE_CHOICE;

    // client request handlers
    m_callbacks[S_COMMAND_SURRENDER] = &Room::processRequestSurrender;
    m_callbacks[S_COMMAND_CHEAT] = &Room::processRequestCheat;

    // Client notifications
    callbacks["toggleReadyCommand"] = &Room::toggleReadyCommand;
    callbacks["addRobotCommand"] = &Room::addRobotCommand;
    callbacks["fillRobotsCommand"] = &Room::fillRobotsCommand;

    callbacks["speakCommand"] = &Room::speakCommand;
    callbacks["trustCommand"] = &Room::trustCommand;
    callbacks["pauseCommand"] = &Room::pauseCommand;

    //Client request
    callbacks["networkDelayTestCommand"] = &Room::networkDelayTestCommand;
}

ServerPlayer *Room::getCurrent() const{
    return current;
}

void Room::setCurrent(ServerPlayer *current) {
    this->current = current;
}

int Room::alivePlayerCount() const{
    return m_alivePlayers.count();
}

bool Room::notifyUpdateCard(ServerPlayer *player, int cardId, const Card *newCard) {
    Json::Value val(Json::arrayValue);
    Q_ASSERT(newCard);
    QString className = Sanguosha->getWrappedCard(newCard->getId())->getClassName();
    val[0] = cardId;
    val[1] = (int)newCard->getSuit();
    val[2] = newCard->getNumber();
    val[3] = toJsonString(className);
    val[4] = toJsonString(newCard->getSkillName());
    val[5] = toJsonString(newCard->objectName());
    val[6] = toJsonArray(newCard->getFlags());
    doNotify(player, S_COMMAND_UPDATE_CARD, val);
    return true;
}

bool Room::broadcastUpdateCard(const QList<ServerPlayer *> &players, int cardId, const Card *newCard) {
    foreach (ServerPlayer *player, players)
        notifyUpdateCard(player, cardId, newCard);
    return true;
}

bool Room::notifyResetCard(ServerPlayer *player, int cardId) {
    doNotify(player, S_COMMAND_UPDATE_CARD, cardId);
    return true;
}

bool Room::broadcastResetCard(const QList<ServerPlayer *> &players, int cardId) {
    foreach (ServerPlayer *player, players)
        notifyResetCard(player, cardId);
    return true;
}

QList<ServerPlayer *> Room::getPlayers() const{
    return m_players;
}

QList<ServerPlayer *> Room::getAllPlayers(bool include_dead) const{
    QList<ServerPlayer *> count_players = m_players;
    if (current == NULL)
        return count_players;

    ServerPlayer *starter = current;
    if (current->getPhase() == Player::NotActive)
        starter = current->getNextAlive();
    int index = count_players.indexOf(starter);
    if (index == -1)
        return count_players;

    QList<ServerPlayer *> all_players;
    for (int i = index; i < count_players.length(); i++) {
        if (include_dead || count_players[i]->isAlive())
            all_players << count_players[i];
    }

    for (int i = 0; i < index; i++) {
        if (include_dead || count_players[i]->isAlive())
            all_players << count_players[i];
    }

    return all_players;
}

QList<ServerPlayer *> Room::getOtherPlayers(ServerPlayer *except, bool include_dead) const{
    QList<ServerPlayer *> other_players = getAllPlayers(include_dead);
    if (except && (except->isAlive() || include_dead))
        other_players.removeOne(except);
    return other_players;
}

QList<ServerPlayer *> Room::getAlivePlayers() const{
    return m_alivePlayers;
}

void Room::output(const QString &message) {
    emit room_message(message);
}

void Room::outputEventStack() {
    QString msg = "End of Event Stack.";
    foreach (EventTriplet triplet, *thread->getEventStack())
        msg.prepend(triplet.toString());
    msg.prepend("Event Stack:\n");
    output(msg);
}

void Room::enterDying(ServerPlayer *player, DamageStruct *reason) {
    setPlayerFlag(player, "Global_Dying");
    QStringList currentdying = getTag("CurrentDying").toStringList();
    currentdying << player->objectName();
    setTag("CurrentDying", QVariant::fromValue(currentdying));

    Json::Value arg(Json::arrayValue);
    arg[0] = (int)QSanProtocol::S_GAME_EVENT_PLAYER_DYING;
    arg[1] = toJsonString(player->objectName());
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, arg);

    DyingStruct dying;
    dying.who = player;
    dying.damage = reason;

    QVariant dying_data = QVariant::fromValue(dying);
    foreach (ServerPlayer *p, getAllPlayers()) {
        if (thread->trigger(Dying, this, p, dying_data) || player->getHp() > 0 || player->isDead())
            break;
    }

    if (player->isAlive()) {
        if (player->getHp() > 0) {
            setPlayerFlag(player, "-Global_Dying");
        } else {
            LogMessage log;
            log.type = "#AskForPeaches";
            log.from = player;
            log.to = getAllPlayers();
            log.arg = QString::number(1 - player->getHp());
            sendLog(log);

            foreach (ServerPlayer *saver, getAllPlayers()) {
                if (player->getHp() > 0 || player->isDead())
                    break;

                QString cd = saver->property("currentdying").toString();
                setPlayerProperty(saver, "currentdying", player->objectName());
                thread->trigger(AskForPeaches, this, saver, dying_data);
                setPlayerProperty(saver, "currentdying", cd);
            }
            thread->trigger(AskForPeachesDone, this, player, dying_data);

            setPlayerFlag(player, "-Global_Dying");
        }
    }
    currentdying = getTag("CurrentDying").toStringList();
    currentdying.removeOne(player->objectName());
    setTag("CurrentDying", QVariant::fromValue(currentdying));

    if (player->isAlive()) {
        Json::Value arg(Json::arrayValue);
        arg[0] = (int)QSanProtocol::S_GAME_EVENT_PLAYER_QUITDYING;
        arg[1] = toJsonString(player->objectName());
        doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, arg);
    }
    thread->trigger(QuitDying, this, player, dying_data);
}

ServerPlayer *Room::getCurrentDyingPlayer() const{
    QStringList currentdying = getTag("CurrentDying").toStringList();
    if (currentdying.isEmpty()) return NULL;
    QString dyingobj = currentdying.last();
    ServerPlayer *who = NULL;
    foreach (ServerPlayer *p, m_alivePlayers) {
        if (p->objectName() == dyingobj) {
            who = p;
            break;
        }
    }
    return who;
}

void Room::revivePlayer(ServerPlayer *player) {
    player->setAlive(true);
    player->throwAllMarks(false);
    broadcastProperty(player, "alive");
    //setEmotion(player, "revive");

    m_alivePlayers.clear();
    foreach (ServerPlayer *player, m_players) {
        if (player->isAlive())
            m_alivePlayers << player;
    }

    for (int i = 0; i < m_alivePlayers.length(); i++) {
        m_alivePlayers.at(i)->setSeat(i + 1);
        broadcastProperty(m_alivePlayers.at(i), "seat");
    }

    doBroadcastNotify(S_COMMAND_REVIVE_PLAYER, toJsonString(player->objectName()));
    updateStateItem();
}

static bool CompareByRole(ServerPlayer *player1, ServerPlayer *player2) {
    int role1 = player1->getRoleEnum();
    int role2 = player2->getRoleEnum();

    if (role1 != role2)
        return role1 < role2;
    else
        return player1->isAlive();
}

void Room::updateStateItem() {
    QList<ServerPlayer *> players = this->m_players;
    qSort(players.begin(), players.end(), CompareByRole);
    QString roles;
    foreach (ServerPlayer *p, players) {
        QChar c = "ZCFN"[p->getRoleEnum()];
        if (p->isDead())
            c = c.toLower();

        roles.append(c);
    }

    doBroadcastNotify(S_COMMAND_UPDATE_STATE_ITEM, toJsonString(roles));
}

void Room::killPlayer(ServerPlayer *victim, DamageStruct *reason) {
    ServerPlayer *killer = reason ? reason->from : NULL;
    QList<ServerPlayer *> players_with_victim = getAllPlayers();

    victim->setAlive(false);

    int index = m_alivePlayers.indexOf(victim);
    for (int i = index + 1; i < m_alivePlayers.length(); i++) {
        ServerPlayer *p = m_alivePlayers.at(i);
        p->setSeat(p->getSeat() - 1);
        broadcastProperty(p, "seat");
    }

    m_alivePlayers.removeOne(victim);

    DeathStruct death;
    death.who = victim;
    death.damage = reason;
    QVariant data = QVariant::fromValue(death);
    thread->trigger(BeforeGameOverJudge, this, victim, data);

    updateStateItem();

    LogMessage log;
    log.type = killer ? (killer == victim ? "#Suicide" : "#Murder") : "#Contingency";
    log.to << victim;
    log.arg = Config.EnableHegemony ? victim->getKingdom() : victim->getRole();
    log.from = killer;
    sendLog(log);

    broadcastProperty(victim, "alive");
    broadcastProperty(victim, "role");

    doBroadcastNotify(S_COMMAND_KILL_PLAYER, toJsonString(victim->objectName()));

    thread->trigger(GameOverJudge, this, victim, data);

    foreach (ServerPlayer *p, players_with_victim)
        if (p->isAlive() || p == victim)
            thread->trigger(Death, this, p, data);

    victim->detachAllSkills();
    thread->trigger(BuryVictim, this, victim, data);

    if (!victim->isAlive() && Config.EnableAI) {
        bool expose_roles = true;
        foreach (ServerPlayer *player, m_alivePlayers) {
            if (!player->isOffline()) {
                expose_roles = false;
                break;
            }
        }

        if (expose_roles) {
            foreach (ServerPlayer *player, m_alivePlayers) {
                if (Config.EnableHegemony) {
                    QString role = player->getKingdom();
                    if (role == "god")
                        role = Sanguosha->getGeneral(getTag(player->objectName()).toStringList().at(0))->getKingdom();
                    role = BasaraMode::getMappedRole(role);
                    broadcastProperty(player, "role", role);
                } else
                    broadcastProperty(player, "role");
            }

            static QStringList continue_list;
            if (continue_list.isEmpty())
                continue_list << "02_1v1" << "04_1v3" << "06_XMode";
            if (continue_list.contains(Config.GameMode))
                return;

            if (Config.AlterAIDelayAD)
                Config.AIDelay = Config.AIDelayAD;
            if (victim->isOnline() && Config.SurrenderAtDeath && mode != "02_1v1" && mode != "06_XMode"
                && askForSkillInvoke(victim, "surrender", "yes"))
                makeSurrender(victim);
        }
    }
}

void Room::judge(JudgeStruct &judge_struct) {
    Q_ASSERT(judge_struct.who != NULL);

    JudgeStar judge_star = &judge_struct;
    QVariant data = QVariant::fromValue(judge_star);
    thread->trigger(StartJudge, this, judge_star->who, data);

    QList<ServerPlayer *> players = getAllPlayers();
    foreach (ServerPlayer *player, players) {
        if (thread->trigger(AskForRetrial, this, player, data))
            break;
    }

    thread->trigger(FinishRetrial, this, judge_star->who, data);
    thread->trigger(FinishJudge, this, judge_star->who, data);
}

void Room::sendJudgeResult(const JudgeStar judge) {
    Json::Value arg(Json::arrayValue);
    arg[0] = (int)QSanProtocol::S_GAME_EVENT_JUDGE_RESULT;
    arg[1] = judge->card->getEffectiveId();
    arg[2] = judge->isEffected();
    arg[3] = toJsonString(judge->who->objectName());
    arg[4] = toJsonString(judge->reason);
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, arg);
}

QList<int> Room::getNCards(int n, bool update_pile_number) {
    QList<int> card_ids;
    for (int i = 0; i < n; i++)
        card_ids << drawCard();

    if (update_pile_number)
        doBroadcastNotify(S_COMMAND_UPDATE_PILE, Json::Value(m_drawPile->length()));

    return card_ids;
}

QStringList Room::aliveRoles(ServerPlayer *except) const{
    QStringList roles;
    foreach (ServerPlayer *player, m_alivePlayers) {
        if (player != except)
            roles << player->getRole();
    }

    return roles;
}

void Room::gameOver(const QString &winner) {
    QStringList all_roles;
    foreach (ServerPlayer *player, m_players) {
        all_roles << player->getRole();
        if (player->getHandcardNum() > 0) {
            QStringList handcards;
            foreach (const Card *card, player->getHandcards())
                handcards << Sanguosha->getEngineCard(card->getId())->getLogName();
            QString handcard = handcards.join(", ").toUtf8().toBase64();
            setPlayerProperty(player, "last_handcards", handcard);
        }
    }

    game_finished = true;

    emit game_over(winner);

    if (mode.contains("_mini_")) {
        ServerPlayer *playerWinner = NULL;
        QStringList winners =winner.split("+");
        foreach (ServerPlayer *sp, m_players) {
            if (sp->getState() != "robot"
                && (winners.contains(sp->getRole())
                    || winners.contains(sp->objectName()))) {
                playerWinner = sp;
                break;
            }
        }

        if (playerWinner) {
            QString id = Config.GameMode;
            id.replace("_mini_", "");
            int stage = Config.value("MiniSceneStage", 1).toInt();
            int current = id.toInt();
            if (current < Sanguosha->getMiniSceneCounts()) {
                if (current + 1 > stage) Config.setValue("MiniSceneStage", current + 1);
                QString mode = QString(MiniScene::S_KEY_MINISCENE).arg(QString::number(current + 1));
                Config.setValue("GameMode", mode);
                Config.GameMode = mode;
            }
        }
    }
    Config.AIDelay = Config.OriginAIDelay;

    if (!getTag("NextGameMode").toString().isNull()) {
        QString name = getTag("NextGameMode").toString();
        Config.GameMode = name;
        Config.setValue("GameMode", name);
        removeTag("NextGameMode");
    }
    if (!getTag("NextGameSecondGeneral").isNull()) {
        bool enable = getTag("NextGameSecondGeneral").toBool();
        Config.Enable2ndGeneral = enable;
        Config.setValue("Enable2ndGeneral", enable);
        removeTag("NextGameSecondGeneral");
    }

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(winner);
    arg[1] = toJsonArray(all_roles);
    doBroadcastNotify(S_COMMAND_GAME_OVER, arg);
    throw GameFinished;
}

void Room::slashEffect(const SlashEffectStruct &effect) {
    QVariant data = QVariant::fromValue(effect);
    if (thread->trigger(SlashEffected, this, effect.to, data)) {
        if (!effect.to->hasFlag("Global_NonSkillNullify"))
            ;//setEmotion(effect.to, "skill_nullify");
        else
            effect.to->setFlags("-Global_NonSkillNullify");
        if (effect.slash) {
            QStringList qinggang = effect.to->tag["Qinggang"].toStringList();
            if (!qinggang.isEmpty()) {
                qinggang.removeOne(effect.slash->toString());
                effect.to->tag["Qinggang"] = qinggang;
            }
        }
    }
}

void Room::slashResult(const SlashEffectStruct &effect, const Card *jink) {
    SlashEffectStruct result_effect = effect;
    result_effect.jink = jink;
    QVariant data = QVariant::fromValue(result_effect);

    if (jink == NULL) {
        if (effect.to->isAlive())
            thread->trigger(SlashHit, this, effect.from, data);
    } else {
        if (effect.to->isAlive()) {
            if (jink->getSkillName() != "eight_diagram" && jink->getSkillName() != "bazhen")
                setEmotion(effect.to, "jink");
        }
        if (effect.slash) {
            QStringList qinggang = effect.to->tag["Qinggang"].toStringList();
            if (!qinggang.isEmpty()) {
                qinggang.removeOne(effect.slash->toString());
                effect.to->tag["Qinggang"] = qinggang;
            }
        }
        thread->trigger(SlashMissed, this, effect.from, data);
    }
}

void Room::attachSkillToPlayer(ServerPlayer *player, const QString &skill_name) {
    player->acquireSkill(skill_name);
    doNotify(player, S_COMMAND_ATTACH_SKILL, toJsonString(skill_name));
}

void Room::detachSkillFromPlayer(ServerPlayer *player, const QString &skill_name, bool is_equip) {
    if (!player->hasSkill(skill_name, true)) return;

    if (player->getAcquiredSkills().contains(skill_name))
        player->detachSkill(skill_name);
    else
        player->loseSkill(skill_name);

    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill && skill->isVisible() && !skill->inherits("SPConvertSkill")) {
        Json::Value args;
        args[0] = QSanProtocol::S_GAME_EVENT_DETACH_SKILL;
        args[1] = toJsonString(player->objectName());
        args[2] = toJsonString(skill_name);
        doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);

        if (!is_equip) {
            LogMessage log;
            log.type = "#LoseSkill";
            log.from = player;
            log.arg = skill_name;
            sendLog(log);

            QVariant data = skill_name;
            thread->trigger(EventLoseSkill, this, player, data);
        }

        foreach (const Skill *skill, Sanguosha->getRelatedSkills(skill_name)) {
            if (skill->isVisible())
                detachSkillFromPlayer(player, skill->objectName());
        }
    }
}

void Room::handleAcquireDetachSkills(ServerPlayer *player, const QStringList &skill_names) {
    if (skill_names.isEmpty()) return;
    QList<bool> isLost;
    QStringList triggerList;
    foreach (QString skill_name, skill_names) {
        if (skill_name.startsWith("-")) {
            QString actual_skill = skill_name.mid(1);
            if (!player->hasSkill(actual_skill, true)) continue;
            if (player->getAcquiredSkills().contains(actual_skill))
                player->detachSkill(actual_skill);
            else
                player->loseSkill(actual_skill);
            const Skill *skill = Sanguosha->getSkill(actual_skill);
            if (skill && skill->isVisible() && !skill->inherits("SPConvertSkill")) {
                Json::Value args;
                args[0] = QSanProtocol::S_GAME_EVENT_DETACH_SKILL;
                args[1] = toJsonString(player->objectName());
                args[2] = toJsonString(actual_skill);
                doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);

                LogMessage log;
                log.type = "#LoseSkill";
                log.from = player;
                log.arg = actual_skill;
                sendLog(log);

                triggerList << actual_skill;
                isLost << true;

                foreach (const Skill *skill, Sanguosha->getRelatedSkills(skill_name)) {
                    if (!skill->isVisible())
                        detachSkillFromPlayer(player, skill->objectName());
                }
            }
        } else {
            const Skill *skill = Sanguosha->getSkill(skill_name);
            if (!skill) continue;
            if (player->getAcquiredSkills().contains(skill_name)) continue;
            player->acquireSkill(skill_name);

            if (skill->inherits("TriggerSkill")) {
                const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
                thread->addTriggerSkill(trigger_skill);
            }

            if (skill->isVisible()) {
                Json::Value args;
                args[0] = QSanProtocol::S_GAME_EVENT_ACQUIRE_SKILL;
                args[1] = toJsonString(player->objectName());
                args[2] = toJsonString(skill_name);
                doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);

                foreach (const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)) {
                    if (!related_skill->isVisible())
                        acquireSkill(player, related_skill);
                }

                triggerList << skill_name;
                isLost << false;
            }
        }
    }
    if (!triggerList.isEmpty()) {
        for (int i = 0; i < triggerList.length(); i++) {
            QVariant data = triggerList.at(i);
            thread->trigger(isLost.at(i) ? EventLoseSkill : EventAcquireSkill, this, player, data);
        }
    }
}

void Room::handleAcquireDetachSkills(ServerPlayer *player, const QString &skill_names) {
    handleAcquireDetachSkills(player, skill_names.split("|"));
}

bool Room::doRequest(ServerPlayer *player, QSanProtocol::CommandType command, const Json::Value &arg, bool wait) {
    time_t timeOut = ServerInfo.getCommandTimeout(command, S_SERVER_INSTANCE);
    return doRequest(player, command, arg, timeOut, wait);
}

bool Room::doRequest(ServerPlayer *player, QSanProtocol::CommandType command, const Json::Value &arg, time_t timeOut, bool wait) {
    QSanGeneralPacket packet(S_SRC_ROOM | S_TYPE_REQUEST | S_DEST_CLIENT, command);
    packet.setMessageBody(arg);
    player->acquireLock(ServerPlayer::SEMA_MUTEX);
    player->m_isClientResponseReady = false;
    player->drainLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
    player->setClientReply(Json::Value::null);
    player->setClientReplyString(QString());
    player->m_isWaitingReply = true;
    player->m_expectedReplySerial = packet.m_globalSerial;
    if (m_requestResponsePair.contains(command))
        player->m_expectedReplyCommand = m_requestResponsePair[command];
    else
        player->m_expectedReplyCommand = command;

    player->invoke(&packet);
    player->releaseLock(ServerPlayer::SEMA_MUTEX);
    if (wait) return getResult(player, timeOut);
    else return true;
}

bool Room::doBroadcastRequest(QList<ServerPlayer *> &players, QSanProtocol::CommandType command) {
    time_t timeOut = ServerInfo.getCommandTimeout(command, S_SERVER_INSTANCE);
    return doBroadcastRequest(players, command, timeOut);
}

bool Room::doBroadcastRequest(QList<ServerPlayer *> &players, QSanProtocol::CommandType command, time_t timeOut) {
    foreach (ServerPlayer *player, players)
        doRequest(player, command, player->m_commandArgs, timeOut, false);

    QTime timer;
    time_t remainTime = timeOut;
    timer.start();
    foreach (ServerPlayer *player, players) {
        remainTime = timeOut - timer.elapsed();
        if (remainTime < 0) remainTime = 0;
        getResult(player, remainTime);
    }
    return true;
}

ServerPlayer *Room::doBroadcastRaceRequest(QList<ServerPlayer *> &players, QSanProtocol::CommandType command,
                                           time_t timeOut, ResponseVerifyFunction validateFunc, void *funcArg) {
    _m_semRoomMutex.acquire();
    _m_raceStarted = true;
    _m_raceWinner = NULL;
    while (_m_semRaceRequest.tryAcquire(1)) {} //drain lock
    _m_semRoomMutex.release();
    Countdown countdown;
    countdown.m_max = timeOut;
    countdown.m_type = Countdown::S_COUNTDOWN_USE_SPECIFIED;
    if (command == S_COMMAND_NULLIFICATION)
        notifyMoveFocus(getAllPlayers(), command, countdown);
    else
        notifyMoveFocus(players, command, countdown);
    foreach (ServerPlayer *player, players)
        doRequest(player, command, player->m_commandArgs, timeOut, false);

    ServerPlayer *winner = getRaceResult(players, command, timeOut, validateFunc, funcArg);
    return winner;
}

ServerPlayer *Room::getRaceResult(QList<ServerPlayer *> &players, QSanProtocol::CommandType, time_t timeOut,
                                  ResponseVerifyFunction validateFunc, void *funcArg) {
    QTime timer;
    timer.start();
    bool validResult = false;
    for (int i = 0; i < players.size(); i++) {
        time_t timeRemain = timeOut - timer.elapsed();
        if (timeRemain < 0) timeRemain = 0;
        bool tryAcquireResult = true;
        if (Config.OperationNoLimit)
            _m_semRaceRequest.acquire();
        else
            tryAcquireResult = _m_semRaceRequest.tryAcquire(1, timeRemain);

        if (!tryAcquireResult)
            _m_semRoomMutex.tryAcquire(1);
        // So that processResponse cannot update raceWinner when we are reading it.

        if (_m_raceWinner == NULL) {
            _m_semRoomMutex.release();
            continue;
        }

        if (validateFunc == NULL
            || (_m_raceWinner->m_isClientResponseReady
                && (this->*validateFunc)(_m_raceWinner, _m_raceWinner->getClientReply(), funcArg))) {
            validResult = true;
            break;
        } else {
            // Don't give this player any more chance for this race
            _m_raceWinner->m_isWaitingReply = false;
            _m_raceWinner = NULL;
            _m_semRoomMutex.release();
        }
    }

    if (!validResult) _m_semRoomMutex.acquire();
    _m_raceStarted = false;
    foreach (ServerPlayer *player, players) {
        player->acquireLock(ServerPlayer::SEMA_MUTEX);
        player->m_expectedReplyCommand = S_COMMAND_UNKNOWN;
        player->m_isWaitingReply = false;
        player->m_expectedReplySerial = -1;
        player->releaseLock(ServerPlayer::SEMA_MUTEX);
    }
    _m_semRoomMutex.release();
    return _m_raceWinner;
}

bool Room::doNotify(ServerPlayer *player, QSanProtocol::CommandType command, const Json::Value &arg) {
    QSanGeneralPacket packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, command);
    packet.setMessageBody(arg);
    player->invoke(&packet);
    return true;
}

bool Room::doBroadcastNotify(const QList<ServerPlayer *> &players, QSanProtocol::CommandType command, const Json::Value &arg) {
    foreach (ServerPlayer *player, players)
        doNotify(player, command, arg);
    return true;
}

bool Room::doBroadcastNotify(QSanProtocol::CommandType command, const Json::Value &arg) {
    return doBroadcastNotify(m_players, command, arg);
}

// the following functions for Lua
bool Room::doNotify(ServerPlayer *player, int command, const QString &arg) {
    QSanGeneralPacket packet(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT, (QSanProtocol::CommandType)command);
    Json::Reader reader;
    Json::Value json_arg;
    std::string str = arg.toStdString();
    if (reader.parse(str, json_arg)) {
        packet.setMessageBody(json_arg);
        player->invoke(&packet);
    } else {
        output("Fail to parse the Json Value " + arg);
    }
    return true;
}

bool Room::doBroadcastNotify(const QList<ServerPlayer *> &players, int command, const QString &arg) {
    foreach (ServerPlayer *player, players)
        doNotify(player, command, arg);
    return true;
}

bool Room::doBroadcastNotify(int command, const QString &arg) {
    return doBroadcastNotify(m_players, command, arg);
}

void Room::broadcastInvoke(const char *method, const QString &arg, ServerPlayer *except) {
    // @@Compatibility
    // ================================================
    if (strcmp(method, "clearAG") == 0) {
        clearAG();
        return;
    } else if (strcmp(method, "animate") == 0) {
        AnimateType a_type;
        QString arg1, arg2;
        QStringList list = arg.split(":");
        if (list.length() > 1)
            arg1 = list.at(1);
        if (list.length() > 2)
            arg2 = list.at(2);
        QString type = list.first();
        if (type == "lightbox")
            a_type = S_ANIMATE_LIGHTBOX;
        else if (type == "nullification")
            a_type = S_ANIMATE_NULLIFICATION;
        else if (type == "indicate")
            a_type = S_ANIMATE_INDICATE;
        doAnimate(a_type, arg1, arg2);
        return;
    }
    // ================================================

    broadcast(QString("%1 %2").arg(method).arg(arg), except);
}

void Room::broadcastInvoke(const QSanProtocol::QSanPacket *packet, ServerPlayer *except) {
    broadcast(QString(packet->toString().c_str()), except);
}

bool Room::getResult(ServerPlayer *player, time_t timeOut) {
    Q_ASSERT(player->m_isWaitingReply);
    bool validResult = false;
    player->acquireLock(ServerPlayer::SEMA_MUTEX);

    if (player->isOnline()) {
        player->releaseLock(ServerPlayer::SEMA_MUTEX);

        if (Config.OperationNoLimit)
            player->acquireLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
        else
            player->tryAcquireLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE, timeOut);

        // Note that we rely on processResponse to filter out all unrelevant packet.
        // By the time the lock is released, m_clientResponse must be the right message
        // assuming the client side is not tampered.

        // Also note that lock can be released when a player switch to trust or offline status.
        // It is ensured by trustCommand and reportDisconnection that the player reports these status
        // is the player waiting the lock. In these cases, the serial number and command type doesn't matter.
        player->acquireLock(ServerPlayer::SEMA_MUTEX);
        validResult = player->m_isClientResponseReady;
    }
    player->m_expectedReplyCommand = S_COMMAND_UNKNOWN;
    player->m_isWaitingReply = false;
    player->m_expectedReplySerial = -1;
    player->releaseLock(ServerPlayer::SEMA_MUTEX);
    return validResult;
}

bool Room::notifyMoveFocus(ServerPlayer *player) {
    QList<ServerPlayer *> players;
    players.append(player);
    Countdown countdown;
    countdown.m_type = Countdown::S_COUNTDOWN_NO_LIMIT;
    notifyMoveFocus(players, S_COMMAND_MOVE_FOCUS, countdown);
    return notifyMoveFocus(players, S_COMMAND_MOVE_FOCUS, countdown);
}

bool Room::notifyMoveFocus(ServerPlayer *player, CommandType command) {
    QList<ServerPlayer *> players;
    players.append(player);
    Countdown countdown;
    countdown.m_max = ServerInfo.getCommandTimeout(command, S_CLIENT_INSTANCE);
    countdown.m_type = Countdown::S_COUNTDOWN_USE_SPECIFIED;
    return notifyMoveFocus(players, S_COMMAND_MOVE_FOCUS, countdown);
}

bool Room::notifyMoveFocus(const QList<ServerPlayer *> &players, CommandType command, Countdown countdown) {
    Json::Value arg(Json::arrayValue);
    int n = players.size();
    for (int i = 0; i < n; i++)
        arg[0][i] = toJsonString(players[i]->objectName());
    arg[1] = (int)command;
    arg[2] = countdown.toJsonValue();
    return doBroadcastNotify(S_COMMAND_MOVE_FOCUS, arg);
}

bool Room::askForSkillInvoke(ServerPlayer *player, const QString &skill_name, const QVariant &data) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_INVOKE_SKILL);

    bool invoked = false;
    AI *ai = player->getAI();
    if (ai) {
        invoked = ai->askForSkillInvoke(skill_name, data);
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (invoked && !(skill && skill->getFrequency() == Skill::Frequent))
            thread->delay();
    } else {
        Json::Value skillCommand;
        if (data.type() == QVariant::String)
            skillCommand = toJsonArray(skill_name, data.toString());
        else
            skillCommand = toJsonArray(skill_name, QString());

        if (!doRequest(player, S_COMMAND_INVOKE_SKILL, skillCommand, true)) {
            invoked = false;
        } else {
            Json::Value clientReply = player->getClientReply();
            if (clientReply.isBool())
                invoked = clientReply.asBool();
        }
    }

    if (invoked) {
        Json::Value msg = toJsonArray(skill_name, player->objectName());
        doBroadcastNotify(S_COMMAND_INVOKE_SKILL, msg);
        notifySkillInvoked(player, skill_name);
    }

    QVariant decisionData = QVariant::fromValue("skillInvoke:" + skill_name + ":" + (invoked ? "yes" : "no"));
    thread->trigger(ChoiceMade, this, player, decisionData);
    return invoked;
}

QString Room::askForChoice(ServerPlayer *player, const QString &skill_name, const QString &choices, const QVariant &data) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_MULTIPLE_CHOICE);

    QStringList validChoices = choices.split("+");
    Q_ASSERT(!validChoices.isEmpty());

    AI *ai = player->getAI();
    QString answer;
    if (validChoices.size() == 1)
        answer = validChoices.first();
    else {
        if (ai) {
            answer = ai->askForChoice(skill_name, choices, data);
            thread->delay();
        } else {
            bool success = doRequest(player, S_COMMAND_MULTIPLE_CHOICE, toJsonArray(skill_name, choices), true);
            Json::Value clientReply = player->getClientReply();
            if (!success || !clientReply.isString()) {
                answer = ".";
                const Skill *skill = Sanguosha->getSkill(skill_name);
                if (skill)
                    return skill->getDefaultChoice(player);
            } else
                answer = toQString(clientReply);
        }
    }

    if (!validChoices.contains(answer))
        answer = validChoices[0];

    QVariant decisionData = QVariant::fromValue("skillChoice:" + skill_name + ":" + answer);
    thread->trigger(ChoiceMade, this, player, decisionData);
    return answer;
}

void Room::obtainCard(ServerPlayer *target, const Card *card, const CardMoveReason &reason, bool unhide) {
    if (card == NULL) return;
    moveCardTo(card, NULL, target, Player::PlaceHand, reason, unhide);
}

void Room::obtainCard(ServerPlayer *target, const Card *card, bool unhide) {
    if (card == NULL) return;
    CardMoveReason reason(CardMoveReason::S_REASON_GOTBACK, target->objectName());
    obtainCard(target, card, reason, unhide);
}

void Room::obtainCard(ServerPlayer *target, int card_id, bool unhide) {
    obtainCard(target, Sanguosha->getCard(card_id), unhide);
}

bool Room::isCanceled(const CardEffectStruct &effect) {
    if (!effect.card->isCancelable(effect))
        return false;

    QVariant decisionData = QVariant::fromValue(effect.to);
    setTag("NullifyingTarget", decisionData);
    decisionData = QVariant::fromValue(effect.from);
    setTag("NullifyingSource", decisionData);
    decisionData = QVariant::fromValue(effect.card);
    setTag("NullifyingCard", decisionData);
    setTag("NullifyingTimes", 0);
    return askForNullification(effect.card, effect.from, effect.to, true);
}

bool Room::verifyNullificationResponse(ServerPlayer *player, const Json::Value &response, void *) {
    const Card *card = NULL;
    if (player != NULL && response.isArray() && response[0].isString())
        card = Card::Parse(toQString(response[0]));
    return card != NULL;
}

bool Room::askForNullification(const Card *trick, ServerPlayer *from, ServerPlayer *to, bool positive) {
    _NullificationAiHelper aiHelper;
    aiHelper.m_from = from;
    aiHelper.m_to = to;
    aiHelper.m_trick = trick;
    return _askForNullification(trick, from, to, positive, aiHelper);
}

bool Room::_askForNullification(const Card *trick, ServerPlayer *from, ServerPlayer *to,
                                bool positive, _NullificationAiHelper aiHelper) {
    while (isPaused()) {}

    _m_roomState.setCurrentCardUseReason(CardUseStruct::CARD_USE_REASON_RESPONSE_USE);
    QString trick_name = trick->objectName();
    QList<ServerPlayer *> validHumanPlayers;
    QList<ServerPlayer *> validAiPlayers;

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(trick_name);
    arg[1] = from ? toJsonString(from->objectName()) : Json::Value::null;
    arg[2] = to ? toJsonString(to->objectName()) : Json::Value::null;

    CardEffectStruct trickEffect;
    trickEffect.card = trick;
    trickEffect.from = from;
    trickEffect.to = to;
    QVariant data = QVariant::fromValue(trickEffect);
    foreach (ServerPlayer *player, m_alivePlayers) {
        if (player->hasNullification()) {
            if (!thread->trigger(TrickCardCanceling, this, player, data)) {
                if (player->isOnline()) {
                    player->m_commandArgs = arg;
                    validHumanPlayers << player;
                } else
                    validAiPlayers << player;
            }
        }
    }

    ServerPlayer *repliedPlayer = NULL;
    time_t timeOut = ServerInfo.getCommandTimeout(S_COMMAND_NULLIFICATION, S_SERVER_INSTANCE);
    if (!validHumanPlayers.empty()) {
        if (trick->isKindOf("AOE") || trick->isKindOf("GlobalEffect")) {
            foreach (ServerPlayer *p, validHumanPlayers)
                doNotify(p, S_COMMAND_NULLIFICATION_ASKED, toJsonString(trick->objectName()));
        }
        repliedPlayer = doBroadcastRaceRequest(validHumanPlayers, S_COMMAND_NULLIFICATION,
                                               timeOut, &Room::verifyNullificationResponse);
    }
    const Card *card = NULL;
    if (repliedPlayer != NULL) {
        Json::Value clientReply = repliedPlayer->getClientReply();
        if (clientReply[0].isString())
            card = Card::Parse(toQString(clientReply[0]));
    }
    if (card == NULL) {
        foreach (ServerPlayer *player, validAiPlayers) {
            AI *ai = player->getAI();
            if (ai == NULL) continue;
            card = ai->askForNullification(aiHelper.m_trick, aiHelper.m_from, aiHelper.m_to, positive);
            if (card && player->isCardLimited(card, Card::MethodUse))
                card = NULL;
            if (card != NULL) {
                thread->delay();
                repliedPlayer = player;
                break;
            }
        }
    }

    if (card == NULL) return false;

    card = card->validateInResponse(repliedPlayer);

    if (card == NULL)
        return _askForNullification(trick, from, to, positive, aiHelper);

    doAnimate(S_ANIMATE_NULLIFICATION, repliedPlayer->objectName(), to->objectName());
    useCard(CardUseStruct(card, repliedPlayer, QList<ServerPlayer *>()));

    LogMessage log;
    log.type = "#NullificationDetails";
    log.from = from;
    log.to << to;
    log.arg = trick_name;
    sendLog(log);
    thread->delay(500);

    QVariant decisionData = QVariant::fromValue("Nullification:" + QString(trick->getClassName())
                                                + ":" + to->objectName() + ":" + (positive ? "true" : "false"));
    thread->trigger(ChoiceMade, this, repliedPlayer, decisionData);
    setTag("NullifyingTimes", getTag("NullifyingTimes").toInt() + 1);

    bool result = true;
    CardEffectStruct effect;
    effect.card = card;
    effect.to = repliedPlayer;
    if (card->isCancelable(effect))
        result = !_askForNullification(card, repliedPlayer, to, !positive, aiHelper);
    return result;
}

int Room::askForCardChosen(ServerPlayer *player, ServerPlayer *who, const QString &flags, const QString &reason,
                           bool handcard_visible, Card::HandlingMethod method) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_CHOOSE_CARD);

    if (handcard_visible && !who->isKongcheng()) {
        QList<int> handcards = who->handCards();
        Json::Value arg(Json::arrayValue);
        arg[0] = toJsonString(who->objectName());
        arg[1] = toJsonArray(handcards);
        doNotify(player, S_COMMAND_SET_KNOWN_CARDS, arg);
    }
    int card_id = Card::S_UNKNOWN_CARD_ID;
    if (who != player && !handcard_visible
        && (flags == "h"
            || (flags == "he" && !who->hasEquip())
            || (flags == "hej" && !who->hasEquip() && who->getJudgingArea().isEmpty())))
        card_id = who->getRandomHandCardId();
    else {
        AI *ai = player->getAI();
        if (ai) {
            thread->delay();
            card_id = ai->askForCardChosen(who, flags, reason, method);
            if (card_id == -1) {
                QList<const Card *> cards = who->getCards(flags);
                if (method == Card::MethodDiscard) {
                    foreach (const Card *card, cards) {
                        if (!player->canDiscard(who, card->getEffectiveId()))
                            cards.removeOne(card);
                    }
                }
                Q_ASSERT(!cards.isEmpty());
                card_id = cards.at(qrand() % cards.length())->getId();
            }
        } else {
            Json::Value arg(Json::arrayValue);
            arg[0] = toJsonString(who->objectName());
            arg[1] = toJsonString(flags);
            arg[2] = toJsonString(reason);
            arg[3] = handcard_visible;
            arg[4] = (int)method;
            bool success = doRequest(player, S_COMMAND_CHOOSE_CARD, arg, true);
            //@todo: check if the card returned is valid
            Json::Value clientReply = player->getClientReply();
            if (!success || !clientReply.isInt()) {
                // randomly choose a card
                QList<const Card *> cards = who->getCards(flags);
                do {
                    card_id = cards.at(qrand() % cards.length())->getId();
                } while (method == Card::MethodDiscard && !player->canDiscard(who, card_id));
            } else
                card_id = clientReply.asInt();

            if (card_id == Card::S_UNKNOWN_CARD_ID)
                card_id = who->getRandomHandCardId();
        }
    }

    Q_ASSERT(card_id != Card::S_UNKNOWN_CARD_ID);

    QVariant decisionData = QVariant::fromValue(QString("cardChosen:%1:%2:%3:%4").arg(reason).arg(card_id)
                                                                                 .arg(player->objectName()).arg(who->objectName()));
    thread->trigger(ChoiceMade, this, player, decisionData);
    return card_id;
}

const Card *Room::askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt,
                             const QVariant &data, const QString &skill_name) {
    return askForCard(player, pattern, prompt, data, Card::MethodDiscard, NULL, false, skill_name);
}

const Card *Room::askForCard(ServerPlayer *player, const QString &pattern, const QString &prompt,
                             const QVariant &data, Card::HandlingMethod method, ServerPlayer *to,
                             bool isRetrial, const QString &skill_name) {
    // @@Compatibility.
    // ===================================================
    TriggerEvent triggerEvent = (TriggerEvent)int(method);
    switch (triggerEvent) {
    case CardUsed: method = Card::MethodUse; break;
    case CardResponded: method = Card::MethodResponse; break;
    case AskForRetrial: method = Card::MethodResponse; isRetrial = true; break;
    case NonTrigger: method = Card::MethodNone; break;
    default: ;
    }
    // ===================================================

    Q_ASSERT(pattern != "slash" || method != Card::MethodUse); // use askForUseSlashTo instead
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_RESPONSE_CARD);

    _m_roomState.setCurrentCardUsePattern(pattern);

    const Card *card = NULL;

    QStringList asked;
    asked << pattern << prompt;
    QVariant asked_data = QVariant::fromValue(asked);
    if ((method == Card::MethodUse || method == Card::MethodResponse) && !isRetrial && !player->hasFlag("continuing"))
        thread->trigger(CardAsked, this, player, asked_data);

    CardUseStruct::CardUseReason reason = CardUseStruct::CARD_USE_REASON_UNKNOWN;
    if (method == Card::MethodResponse)
        reason = CardUseStruct::CARD_USE_REASON_RESPONSE;
    else if (method == Card::MethodUse)
        reason = CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
    _m_roomState.setCurrentCardUseReason(reason);

    if (player->hasFlag("continuing"))
        setPlayerFlag(player, "-continuing");
    if (has_provided || !player->isAlive()) {
        card = provided;
        provided = NULL;
        has_provided = false;
    } else {
        AI *ai = player->getAI();
        if (ai) {
            card = ai->askForCard(pattern, prompt, data);
            if (card && card->isKindOf("DummyCard") && card->subcardsLength() == 1)
                card = Sanguosha->getCard(card->getEffectiveId());
            if (card && player->isCardLimited(card, method)) card = NULL;
            if (card) thread->delay();
        } else {
            Json::Value arg(Json::arrayValue);
            arg[0] = toJsonString(pattern);
            arg[1] = toJsonString(prompt);
            arg[2] = int(method);
            bool success = doRequest(player, S_COMMAND_RESPONSE_CARD, arg, true);
            Json::Value clientReply = player->getClientReply();
            if (success && !clientReply.isNull())
                card = Card::Parse(toQString(clientReply[0]));
        }
    }

    if (card == NULL) {
        QVariant decisionData = QVariant::fromValue(QString("cardResponded:%1:%2:_nil_").arg(pattern).arg(prompt));
        thread->trigger(ChoiceMade, this, player, decisionData);
        return NULL;
    }

    card = card->validateInResponse(player);
    const Card *result = NULL;

    if (card) {
        if (!card->isVirtualCard()) {
            WrappedCard *wrapped = Sanguosha->getWrappedCard(card->getEffectiveId());
            if (wrapped->isModified())
                broadcastUpdateCard(getPlayers(), card->getEffectiveId(), wrapped);
            else
                broadcastResetCard(getPlayers(), card->getEffectiveId());
        }

        if ((method == Card::MethodUse || method == Card::MethodResponse) && !isRetrial) {
            LogMessage log;
            log.card_str = card->toString();
            log.from = player;
            log.type = QString("#%1").arg(card->getClassName());
            if (method == Card::MethodResponse)
                log.type += "_Resp";
            sendLog(log);
            player->broadcastSkillInvoke(card);
        } else if (method == Card::MethodDiscard) {
            LogMessage log;
            log.type = skill_name.isEmpty()? "$DiscardCard" : "$DiscardCardWithSkill";
            log.from = player;
            QList<int> to_discard;
            if (card->isVirtualCard())
                to_discard.append(card->getSubcards());
            else
                to_discard << card->getEffectiveId();
            log.card_str = IntList2StringList(to_discard).join("+");
            if (!skill_name.isEmpty())
                log.arg = skill_name;
            sendLog(log);
            if (!skill_name.isEmpty())
                notifySkillInvoked(player, skill_name);
        }
    }

    if (card) {
        QVariant decisionData = QVariant::fromValue(QString("cardResponded:%1:%2:_%3_").arg(pattern).arg(prompt).arg(card->toString()));
        thread->trigger(ChoiceMade, this, player, decisionData);

        if (method == Card::MethodUse) {
            CardMoveReason reason(CardMoveReason::S_REASON_LETUSE, player->objectName(), QString(), card->getSkillName(), QString());
            moveCardTo(card, player, NULL, Player::PlaceTable, reason, true);
        } else if (method == Card::MethodDiscard) {
            CardMoveReason reason(CardMoveReason::S_REASON_THROW, player->objectName());
            moveCardTo(card, player, NULL, Player::DiscardPile, reason, pattern != "." && pattern != "..");
        } else if (method != Card::MethodNone && !isRetrial) {
            CardMoveReason reason(CardMoveReason::S_REASON_RESPONSE, player->objectName());
            reason.m_skillName = card->getSkillName();
            moveCardTo(card, player, NULL, Player::DiscardPile, reason);
        }

        if ((method == Card::MethodUse || method == Card::MethodResponse) && !isRetrial) {
            if (!card->getSkillName().isNull() && card->getSkillName(true) == card->getSkillName(false)
                && player->hasSkill(card->getSkillName()))
                notifySkillInvoked(player, card->getSkillName());
            CardResponseStruct resp(card, to, method == Card::MethodUse);
            QVariant data = QVariant::fromValue(resp);
            thread->trigger(CardResponded, this, player, data);
            if (method == Card::MethodUse) {
                if (getCardPlace(card->getEffectiveId()) == Player::PlaceTable) {
                    CardMoveReason reason(CardMoveReason::S_REASON_LETUSE, player->objectName(),
                                          QString(), card->getSkillName(), QString());
                    moveCardTo(card, player, NULL, Player::DiscardPile, reason, true);
                }
                CardUseStruct card_use;
                card_use.card = card;
                card_use.from = player;
                if (to) card_use.to << to;
                QVariant data2 = QVariant::fromValue(card_use);
                thread->trigger(CardFinished, this, player, data2);
            }
        }
        result = card;
    } else {
        setPlayerFlag(player, "continuing");
        result = askForCard(player, pattern, prompt, data, method, to, isRetrial);
    }
    return result;
}

bool Room::askForUseCard(ServerPlayer *player, const QString &pattern, const QString &prompt, int notice_index,
                         Card::HandlingMethod method, bool addHistory) {
    Q_ASSERT(method != Card::MethodResponse);
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_RESPONSE_CARD);

    _m_roomState.setCurrentCardUsePattern(pattern);
    _m_roomState.setCurrentCardUseReason(CardUseStruct::CARD_USE_REASON_RESPONSE_USE);
    CardUseStruct card_use;

    bool isCardUsed = false;
    AI *ai = player->getAI();
    if (ai) {
        QString answer = ai->askForUseCard(pattern, prompt, method);
        if (answer != ".") {
            isCardUsed = true;
            card_use.from = player;
            card_use.parse(answer, this);
            thread->delay();
        }
    } else {
        Json::Value ask_str(Json::arrayValue);
        ask_str[0] = toJsonString(pattern);
        ask_str[1] = toJsonString(prompt);
        ask_str[2] = int(method);
        ask_str[3] = notice_index;
        bool success = doRequest(player, S_COMMAND_RESPONSE_CARD, ask_str, true);
        if (success) {
            Json::Value clientReply = player->getClientReply();
            isCardUsed = !clientReply.isNull();
            if (isCardUsed && card_use.tryParse(clientReply, this))
                card_use.from = player;
        }
    }
    card_use.m_reason = CardUseStruct::CARD_USE_REASON_RESPONSE_USE;
    if (isCardUsed && card_use.isValid(pattern)) {
        QVariant decisionData = QVariant::fromValue(card_use);
        thread->trigger(ChoiceMade, this, player, decisionData);
        if (!useCard(card_use, addHistory))
            return askForUseCard(player, pattern, prompt, notice_index, method, addHistory);
        return true;
    } else {
        QVariant decisionData = QVariant::fromValue("cardUsed:" + pattern + ":" + prompt + ":nil");
        thread->trigger(ChoiceMade, this, player, decisionData);
    }

    return false;
}

bool Room::askForUseSlashTo(ServerPlayer *slasher, QList<ServerPlayer *> victims, const QString &prompt,
                            bool distance_limit, bool disable_extra, bool addHistory, const QString &pattern) {
    Q_ASSERT(!victims.isEmpty());

    // The realization of this function in the Slash::onUse and Slash::targetFilter.
    setPlayerFlag(slasher, "slashTargetFix");
    if (!distance_limit)
        setPlayerFlag(slasher, "slashNoDistanceLimit");
    if (disable_extra)
        setPlayerFlag(slasher, "slashDisableExtraTarget");
    if (victims.length() == 1)
        setPlayerFlag(slasher, "slashTargetFixToOne");
    foreach (ServerPlayer *victim, victims)
        setPlayerFlag(victim, "SlashAssignee");

    bool use = askForUseCard(slasher, pattern, prompt, -1, Card::MethodUse, addHistory);
    if (!use) {
        setPlayerFlag(slasher, "-slashTargetFix");
        setPlayerFlag(slasher, "-slashTargetFixToOne");
        foreach (ServerPlayer *victim, victims)
            setPlayerFlag(victim, "-SlashAssignee");
        if (slasher->hasFlag("slashNoDistanceLimit"))
            setPlayerFlag(slasher, "-slashNoDistanceLimit");
        if (slasher->hasFlag("slashDisableExtraTarget"))
            setPlayerFlag(slasher, "-slashDisableExtraTarget");
    }

    return use;
}

bool Room::askForUseSlashTo(ServerPlayer *slasher, ServerPlayer *victim, const QString &prompt,
                            bool distance_limit, bool disable_extra, bool addHistory, const QString &pattern) {
    Q_ASSERT(victim != NULL);
    QList<ServerPlayer *> victims;
    victims << victim;
    return askForUseSlashTo(slasher, victims, prompt, distance_limit, disable_extra, addHistory, pattern);
}

int Room::askForAG(ServerPlayer *player, const QList<int> &card_ids, bool refusable, const QString &reason) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_AMAZING_GRACE);
    Q_ASSERT(card_ids.length() > 0);

    int card_id = -1;
    if (card_ids.length() == 1 && !refusable)
        card_id = card_ids.first();
    else {
        AI *ai = player->getAI();
        if (ai) {
            thread->delay();
            card_id = ai->askForAG(card_ids, refusable, reason);
        } else {
            bool success = doRequest(player, S_COMMAND_AMAZING_GRACE, refusable, true);
            Json::Value clientReply = player->getClientReply();
            if (success && clientReply.isInt())
                card_id = clientReply.asInt();
        }

        if (!card_ids.contains(card_id))
            card_id = refusable ? -1 : card_ids.first();
    }

    QVariant decisionData = QVariant::fromValue("AGChosen:" + reason + ":" + QString::number(card_id));
    thread->trigger(ChoiceMade, this, player, decisionData);

    return card_id;
}

const Card *Room::askForCardShow(ServerPlayer *player, ServerPlayer *requestor, const QString &reason) {
    Q_ASSERT(!player->isKongcheng());
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_SHOW_CARD);
    const Card *card = NULL;

    AI *ai = player->getAI();
    if (ai)
        card = ai->askForCardShow(requestor, reason);
    else {
        if (player->getHandcardNum() == 1)
            card = player->getHandcards().first();
        else {
            bool success = doRequest(player, S_COMMAND_SHOW_CARD, toJsonString(requestor->getGeneralName()), true);
            Json::Value clientReply = player->getClientReply();
            if (success && clientReply[0].isString())
                card = Card::Parse(toQString(clientReply[0]));
            if (card == NULL)
                card = player->getRandomHandCard();
        }
    }

    QVariant decisionData = QVariant::fromValue("cardShow:" + reason + ":_" + card->toString() + "_");
    thread->trigger(ChoiceMade, this, player, decisionData);
    return card;
}

const Card *Room::askForSinglePeach(ServerPlayer *player, ServerPlayer *dying) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_ASK_PEACH);
    _m_roomState.setCurrentCardUseReason(CardUseStruct::CARD_USE_REASON_RESPONSE_USE);

    const Card *card = NULL;

    AI *ai = player->getAI();
    if (ai)
        card = ai->askForSinglePeach(dying);
    else {
        int peaches = 1 - dying->getHp();
        Json::Value arg(Json::arrayValue);
        arg[0] = toJsonString(dying->objectName());
        arg[1] = peaches;
        bool success = doRequest(player, S_COMMAND_ASK_PEACH, arg, true);
        Json::Value clientReply = player->getClientReply();
        if (!success || !clientReply[0].isString())
            return NULL;

        card = Card::Parse(toQString(clientReply[0]));
    }

    if (card && player->isCardLimited(card, Card::MethodUse))
        card = NULL;
    if (card != NULL)
        card = card->validateInResponse(player);
    else
        return NULL;

    const Card *result = NULL;
    if (card) {
        QVariant decisionData = QVariant::fromValue(QString("peach:%1:%2:%3")
                                                            .arg(dying->objectName())
                                                            .arg(1 - dying->getHp())
                                                            .arg(card->toString()));
        thread->trigger(ChoiceMade, this, player, decisionData);
        result = card;
    } else
        result = askForSinglePeach(player, dying);
    return result;
}

void Room::addPlayerHistory(ServerPlayer *player, const QString &key, int times) {
    if (player) {
        if (key == ".")
            player->clearHistory();
        else
            player->addHistory(key, times);
    }

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(key);
    arg[1] = times;

    if (player)
        doNotify(player, S_COMMAND_ADD_HISTORY, arg);
    else
        doBroadcastNotify(S_COMMAND_ADD_HISTORY, arg);
}

void Room::setPlayerFlag(ServerPlayer *player, const QString &flag) {
    if (flag.startsWith("-")) {
        QString set_flag = flag.mid(1);
        if (!player->hasFlag(set_flag)) return;
    }
    player->setFlags(flag);
    broadcastProperty(player, "flags", flag);
}

void Room::setPlayerProperty(ServerPlayer *player, const char *property_name, const QVariant &value) {
    player->setProperty(property_name, value);
    broadcastProperty(player, property_name);

    if (strcmp(property_name, "hp") == 0)
        thread->trigger(HpChanged, this, player);

    if (strcmp(property_name, "maxhp") == 0)
        thread->trigger(MaxHpChanged, this, player);

    if (strcmp(property_name, "chained") == 0)
        thread->trigger(ChainStateChanged, this, player);
}

void Room::setPlayerMark(ServerPlayer *player, const QString &mark, int value) {
    player->setMark(mark, value);

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(player->objectName());
    arg[1] = toJsonString(mark);
    arg[2] = value;
    doBroadcastNotify(S_COMMAND_SET_MARK, arg);
}

void Room::addPlayerMark(ServerPlayer *player, const QString &mark, int add_num) {
    int value = player->getMark(mark);
    value += add_num;
    setPlayerMark(player, mark, value);
}

void Room::removePlayerMark(ServerPlayer *player, const QString &mark, int remove_num) {
    int value = player->getMark(mark);
    if (value == 0) return;
    value -= remove_num;
    value = qMax(0, value);
    setPlayerMark(player, mark, value);
}

void Room::setPlayerCardLimitation(ServerPlayer *player, const QString &limit_list,
                                   const QString &pattern, bool single_turn) {
    player->setCardLimitation(limit_list, pattern, single_turn);

    Json::Value arg(Json::arrayValue);
    arg[0] = true;
    arg[1] = toJsonString(limit_list);
    arg[2] = toJsonString(pattern);
    arg[3] = single_turn;
    doNotify(player, S_COMMAND_CARD_LIMITATION, arg);
}

void Room::removePlayerCardLimitation(ServerPlayer *player, const QString &limit_list,
                                   const QString &pattern) {
    player->removeCardLimitation(limit_list, pattern);

    Json::Value arg(Json::arrayValue);
    arg[0] = false;
    arg[1] = toJsonString(limit_list);
    arg[2] = toJsonString(pattern);
    arg[3] = false;
    doNotify(player, S_COMMAND_CARD_LIMITATION, arg);
}

void Room::clearPlayerCardLimitation(ServerPlayer *player, bool single_turn) {
    player->clearCardLimitation(single_turn);

    Json::Value arg(Json::arrayValue);
    arg[0] = true;
    arg[1] = Json::Value::null;
    arg[2] = Json::Value::null;
    arg[3] = single_turn;
    doNotify(player, S_COMMAND_CARD_LIMITATION, arg);
}

void Room::setCardFlag(const Card *card, const QString &flag, ServerPlayer *who) {
    if (flag.isEmpty()) return;

    card->setFlags(flag);

    if (!card->isVirtualCard())
        setCardFlag(card->getEffectiveId(), flag, who);
}

void Room::setCardFlag(int card_id, const QString &flag, ServerPlayer *who) {
    if (flag.isEmpty()) return;

    Q_ASSERT(Sanguosha->getCard(card_id) != NULL);
    Sanguosha->getCard(card_id)->setFlags(flag);

    Json::Value arg(Json::arrayValue);
    arg[0] = card_id;
    arg[1] = toJsonString(flag);
    if (who)
        doNotify(who, S_COMMAND_CARD_FLAG, arg);
    else
        doBroadcastNotify(S_COMMAND_CARD_FLAG, arg);
}

void Room::clearCardFlag(const Card *card, ServerPlayer *who) {
    card->clearFlags();

    if (!card->isVirtualCard())
        clearCardFlag(card->getEffectiveId(), who);
}

void Room::clearCardFlag(int card_id, ServerPlayer *who) {
    Q_ASSERT(Sanguosha->getCard(card_id) != NULL);
    Sanguosha->getCard(card_id)->clearFlags();

    Json::Value arg(Json::arrayValue);
    arg[0] = card_id;
    arg[1] = toJsonString(".");
    if (who)
        doNotify(who, S_COMMAND_CARD_FLAG, arg);
    else
        doBroadcastNotify(S_COMMAND_CARD_FLAG, arg);
}

ServerPlayer *Room::addSocket(ClientSocket *socket) {
    ServerPlayer *player = new ServerPlayer(this);
    player->setSocket(socket);
    m_players << player;

    connect(player, SIGNAL(disconnected()), this, SLOT(reportDisconnection()));
    connect(player, SIGNAL(request_got(QString)), this, SLOT(processClientPacket(QString)));

    return player;
}

bool Room::isFull() const{
    return m_players.length() == player_count;
}

bool Room::isFinished() const{
    return game_finished;
}

bool Room::canPause(ServerPlayer *player) const{
    if (!isFull()) return false;
    if (!player || !player->isOwner()) return false;
    foreach (ServerPlayer *p, m_players) {
        if (!p->isAlive() || p->isOwner()) continue;
        if (p->getState() != "robot")
            return false;
    }
    return true;
}

bool Room::isPaused() const{
    if (!canPause(getOwner())) return false;
    return game_paused;
}

int Room::getLack() const{
    return player_count - m_players.length();
}

QString Room::getMode() const{
    return mode;
}

const Scenario *Room::getScenario() const{
    return scenario;
}

void Room::broadcast(const QString &message, ServerPlayer *except) {
    foreach (ServerPlayer *player, m_players) {
        if (player != except)
            player->unicast(message);
    }
}

void Room::swapPile() {
    if (m_discardPile->isEmpty()) {
        // the standoff
        gameOver(".");
    }

    int times = tag.value("SwapPile", 0).toInt();
    tag.insert("SwapPile", ++times);

    int limit = Config.value("PileSwappingLimitation", 5).toInt() + 1;
    if (mode == "04_1v3")
        limit = qMin(limit, Config.BanPackages.contains("maneuvering") ? 3 : 2);
    if (limit > 0 && times == limit)
        gameOver(".");

    qSwap(m_drawPile, m_discardPile);

    doBroadcastNotify(S_COMMAND_RESET_PILE, Json::Value::null);
    doBroadcastNotify(S_COMMAND_UPDATE_PILE, Json::Value(m_drawPile->length()));

    qShuffle(*m_drawPile);
    foreach (int card_id, *m_drawPile)
        setCardMapping(card_id, NULL, Player::DrawPile);
}

QList<int> Room::getDiscardPile() {
    return *m_discardPile;
}

ServerPlayer *Room::findPlayer(const QString &general_name, bool include_dead) const{
    const QList<ServerPlayer *> &list = include_dead ? m_players : m_alivePlayers;

    if (general_name.contains("+")) {
        QStringList names = general_name.split("+");
        foreach (ServerPlayer *player, list) {
            if (names.contains(player->getGeneralName()))
                return player;
        }
        return NULL;
    }

    foreach (ServerPlayer *player, list) {
        if (player->getGeneralName() == general_name)
            return player;
    }

    return NULL;
}

QList<ServerPlayer *>Room::findPlayersBySkillName(const QString &skill_name) const{
    QList<ServerPlayer *> list;
    foreach (ServerPlayer *player, getAllPlayers()) {
        if (player->hasSkill(skill_name))
            list << player;
    }
    return list;
}

ServerPlayer *Room::findPlayerBySkillName(const QString &skill_name) const{
    foreach (ServerPlayer *player, getAllPlayers()) {
        if (player->hasSkill(skill_name))
            return player;
    }
    return NULL;
}

void Room::installEquip(ServerPlayer *player, const QString &equip_name) {
    if (player == NULL) return;

    int card_id = getCardFromPile(equip_name);
    if (card_id == -1) return;

    moveCardTo(Sanguosha->getCard(card_id), player, Player::PlaceEquip, true);
}

void Room::resetAI(ServerPlayer *player) {
    AI *smart_ai = player->getSmartAI();
    int index = -1;
    if (smart_ai) {
        index = ais.indexOf(smart_ai);
        ais.removeOne(smart_ai);
        smart_ai->deleteLater();
    }
    AI *new_ai = cloneAI(player);
    player->setAI(new_ai);
    if (index == -1)
        ais.append(new_ai);
    else
        ais.insert(index, new_ai);
}

void Room::changeHero(ServerPlayer *player, const QString &new_general, bool full_state, bool invokeStart,
                      bool isSecondaryHero, bool sendLog) {
    Json::Value arg(Json::arrayValue);
    arg[0] = S_GAME_EVENT_CHANGE_HERO;
    arg[1] = toJsonString(player->objectName());
    arg[2] = toJsonString(new_general);
    arg[3] = isSecondaryHero;
    arg[4] = sendLog;
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, arg);

    if (isSecondaryHero)
        changePlayerGeneral2(player, new_general);
    else
        changePlayerGeneral(player, new_general);
    player->setMaxHp(player->getGeneralMaxHp());

    if (full_state)
        player->setHp(player->getMaxHp());
    broadcastProperty(player, "hp");
    broadcastProperty(player, "maxhp");

    QVariant void_data;
    QList<const TriggerSkill *> game_start;
    const General *gen = isSecondaryHero ? player->getGeneral2() : player->getGeneral();
    if (gen) {
        foreach (const Skill *skill, gen->getSkillList()) {
            if (skill->inherits("TriggerSkill")) {
                 const TriggerSkill *trigger = qobject_cast<const TriggerSkill *>(skill);
                 thread->addTriggerSkill(trigger);
                 if (invokeStart && trigger->getTriggerEvents().contains(GameStart) && trigger->triggerable(player))
                     game_start << trigger;
            }
        }
    }
    if (invokeStart) {
        foreach (const TriggerSkill *skill, game_start)
            skill->trigger(GameStart, this, player, void_data);
    }
    resetAI(player);
}

lua_State *Room::getLuaState() const{
    return L;
}

void Room::setFixedDistance(Player *from, const Player *to, int distance) {
    from->setFixedDistance(to, distance);

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(from->objectName());
    arg[1] = toJsonString(to->objectName());
    arg[2] = distance;
    doBroadcastNotify(S_COMMAND_FIXED_DISTANCE, arg);
}

void Room::reverseFor3v3(const Card *card, ServerPlayer *player, QList<ServerPlayer *> &list) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_CHOOSE_DIRECTION);

    bool isClockwise = false;
    if (player->isOnline()) {
        bool success = doRequest(player, S_COMMAND_CHOOSE_DIRECTION, Json::Value::null, true);
        Json::Value clientReply = player->getClientReply();
        if (success && clientReply.isString())
            isClockwise = (clientReply.asString() == "cw");
    } else {
        QVariant data = QVariant::fromValue((CardStar)card);
        isClockwise = (askForChoice(player, "3v3_direction", "cw+ccw", data) == "cw");
    }

    LogMessage log;
    log.type = "#TrickDirection";
    log.from = player;
    log.arg = isClockwise ? "cw" : "ccw";
    log.arg2 = card->objectName();
    sendLog(log);

    if (isClockwise) {
        QList<ServerPlayer *> new_list;

        while (!list.isEmpty())
            new_list << list.takeLast();

        if (card->isKindOf("GlobalEffect")) {
            new_list.removeLast();
            new_list.prepend(player);
        }

        list = new_list;
    }
}

const ProhibitSkill *Room::isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others) const{
    return Sanguosha->isProhibited(from, to, card, others);
}

int Room::drawCard() {
    thread->trigger(FetchDrawPileCard, this, NULL);
    if (m_drawPile->isEmpty())
        swapPile();
    return m_drawPile->takeFirst();
}

void Room::prepareForStart() {
    if (scenario) {
        QStringList generals, roles;
        scenario->assign(generals, roles);

        bool expose_roles = scenario->exposeRoles();
        for (int i = 0; i < m_players.length(); i++) {
            ServerPlayer *player = m_players[i];
            if (generals.size() > i && !generals[i].isNull()) {
                player->setGeneralName(generals[i]);
                broadcastProperty(player, "general");
            }

            player->setRole(roles.at(i));
            if (player->isLord())
                broadcastProperty(player, "role");

            if (expose_roles)
                broadcastProperty(player, "role");
            else
                notifyProperty(player, player, "role");
        }
    } else if (mode == "06_3v3") {
        return;
    } else if (mode == "06_XMode") {
        return;
    } else if (mode == "02_1v1") {
        if (qrand() % 2 == 0)
            m_players.swap(0, 1);

        QString order = askForOrder(m_players.at(0));
        if (order == "warm") {
            m_players.at(0)->setRole("lord");
            m_players.at(1)->setRole("renegade");
        } else {
            m_players.at(0)->setRole("renegade");
            m_players.at(1)->setRole("lord");
        }

        for (int i = 0; i < 2; i++)
            broadcastProperty(m_players.at(i), "role");
    } else if (!Config.EnableHegemony && Config.EnableCheat && Config.value("FreeAssign", false).toBool()) {
        ServerPlayer *owner = getOwner();
        notifyMoveFocus(owner, S_COMMAND_CHOOSE_ROLE);
        if (owner && owner->isOnline()) {
            bool success = doRequest(owner, S_COMMAND_CHOOSE_ROLE, Json::Value::null, true);
            Json::Value clientReply = owner->getClientReply();
            if (!success || !clientReply.isArray() || clientReply.size() != 2) {
                if (Config.RandomSeat)
                    qShuffle(m_players);
                assignRoles();
            } else if (Config.FreeAssignSelf) {
                QString name = toQString(clientReply[0][0]);
                QString role = toQString(clientReply[1][0]);
                ServerPlayer *player_self = findChild<ServerPlayer *>(name);
                setPlayerProperty(player_self, "role", role);

                QList<ServerPlayer *> all_players = m_players;
                all_players.removeOne(player_self);
                int n = all_players.count();
                QStringList roles = Sanguosha->getRoleList(mode);
                roles.removeOne(role);
                qShuffle(roles);

                for (int i = 0; i < n; i++) {
                    ServerPlayer *player = all_players[i];
                    QString role = roles.at(i);

                    player->setRole(role);
                    if (role == "lord" && !ServerInfo.EnableHegemony)
                        broadcastProperty(player, "role", "lord");
                    else {
                        if (mode == "04_1v3")
                            broadcastProperty(player, "role", role);
                        else
                            notifyProperty(player, player, "role");
                    }
                }
            } else {
                for (unsigned int i = 0; i < clientReply[0].size(); i++) {
                    QString name = toQString(clientReply[0][i]);
                    QString role = toQString(clientReply[1][i]);

                    ServerPlayer *player = findChild<ServerPlayer *>(name);
                    setPlayerProperty(player, "role", role);

                    m_players.swap(i, m_players.indexOf(player));
                }
            }
        } else if (mode == "04_1v3") {
            if (Config.RandomSeat)
                qShuffle(m_players);
            ServerPlayer *lord = m_players.at(qrand() % 4);
            for (int i = 0; i < 4; i++) {
                ServerPlayer *player = m_players.at(i);
                if (player == lord)
                    player->setRole("lord");
                else
                    player->setRole("rebel");
                broadcastProperty(player, "role");
            }
        } else {
            if (Config.RandomSeat)
                qShuffle(m_players);
            assignRoles();
        }
    } else {
        if (Config.RandomSeat)
            qShuffle(m_players);
        assignRoles();
    }

    adjustSeats();
}

void Room::reportDisconnection() {
    ServerPlayer *player = qobject_cast<ServerPlayer *>(sender());
    if (player == NULL) return;

    // send disconnection message to server log
    emit room_message(player->reportHeader() + tr("disconnected"));

    // the 4 kinds of circumstances
    // 1. Just connected, with no object name : just remove it from player list
    // 2. Connected, with an object name : remove it, tell other clients and decrease signup_count
    // 3. Game is not started, but role is assigned, give it the default general(general2) and others same with fourth case
    // 4. Game is started, do not remove it just set its state as offline
    // all above should set its socket to NULL

    player->setSocket(NULL);

    if (player->objectName().isEmpty()) {
        // first case
        player->setParent(NULL);
        m_players.removeOne(player);
    } else if (player->getRole().isEmpty()) {
        // second case
        if (m_players.length() < player_count) {
            player->setParent(NULL);
            m_players.removeOne(player);

            if (player->getState() != "robot") {
                QString screen_name = player->screenName();
                QString leaveStr = tr("<font color=#000000>Player <b>%1</b> left the game</font>").arg(screen_name);
                speakCommand(player, leaveStr.toUtf8().toBase64());
            }

            broadcastInvoke("removePlayer", player->objectName());
        }
    } else {
        // fourth case
        if (player->m_isWaitingReply)
            player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
        setPlayerProperty(player, "state", "offline");

        bool someone_is_online = false;
        foreach (ServerPlayer *player, m_players) {
            if (player->getState() == "online" || player->getState() == "trust") {
                someone_is_online = true;
                break;
            }
        }

        if (!someone_is_online) {
            game_finished = true;
            return;
        }
    }

    if (player->isOwner()) {
        player->setOwner(false);
        broadcastProperty(player, "owner");
        foreach (ServerPlayer *p, m_players) {
            if (p->getState() == "online") {
                p->setOwner(true);
                broadcastProperty(p, "owner");
                break;
            }
        }
    }
}

void Room::trustCommand(ServerPlayer *player, const QString &) {
    player->acquireLock(ServerPlayer::SEMA_MUTEX);
    if (player->isOnline()) {
        player->setState("trust");
        if (player->m_isWaitingReply) {
            player->releaseLock(ServerPlayer::SEMA_MUTEX);
            player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
        }
    } else
        player->setState("online");

    player->releaseLock(ServerPlayer::SEMA_MUTEX);
    broadcastProperty(player, "state");
}

void Room::pauseCommand(ServerPlayer *player, const QString &arg) {
    if (!canPause(player)) return;
    bool pause = (arg == "true");
    if (game_paused != pause) {
        Json::Value arg(Json::arrayValue);
        arg[0] = (int)S_GAME_EVENT_PAUSE;
        arg[1] = pause;
        doNotify(player, S_COMMAND_LOG_EVENT, arg);

        game_paused = pause;
    }
}

bool Room::processRequestCheat(ServerPlayer *player, const QSanProtocol::QSanGeneralPacket *packet) {
    if (!Config.EnableCheat) return false;
    Json::Value arg = packet->getMessageBody();
    if (!arg.isArray() || !arg[0].isInt()) return false;
    //@todo: synchronize this
    player->m_cheatArgs = arg;
    player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
    return true;
}

bool Room::makeSurrender(ServerPlayer *initiator) {
    bool loyalGiveup = true;
    int loyalAlive = 0;
    bool renegadeGiveup = true;
    int renegadeAlive = 0;
    bool rebelGiveup = true;
    int rebelAlive = 0;

    // broadcast polling request
    QList<ServerPlayer *> playersAlive;
    foreach (ServerPlayer *player, m_players) {
        QString playerRole = player->getRole();
        if ((playerRole == "loyalist" || playerRole == "lord") && player->isAlive()) loyalAlive++;
        else if (playerRole == "rebel" && player->isAlive()) rebelAlive++;
        else if (playerRole == "renegade" && player->isAlive()) renegadeAlive++;

        if (player != initiator && player->isAlive() && player->isOnline()) {
            player->m_commandArgs = toJsonString(initiator->getGeneral()->objectName());
            playersAlive << player;
        }
    }
    doBroadcastRequest(playersAlive, S_COMMAND_SURRENDER);

    // collect polls
    foreach (ServerPlayer *player, playersAlive) {
        bool result = false;
        if (!player->m_isClientResponseReady || !player->getClientReply().isBool())
            result = !player->isOnline();
        else
            result = player->getClientReply().asBool();

        QString playerRole = player->getRole();
        if (playerRole == "loyalist" || playerRole == "lord") {
            loyalGiveup &= result;
            if (player->isAlive()) loyalAlive++;
        } else if (playerRole == "rebel") {
            rebelGiveup &= result;
            if (player->isAlive()) rebelAlive++;
        } else if (playerRole == "renegade") {
            renegadeGiveup &= result;
            if (player->isAlive()) renegadeAlive++;
        }
    }

    // vote counting
    if (loyalGiveup && renegadeGiveup && !rebelGiveup)
        gameOver("rebel");
    else if (loyalGiveup && !renegadeGiveup && rebelGiveup)
        gameOver("renegade");
    else if (!loyalGiveup && renegadeGiveup && rebelGiveup)
        gameOver("lord+loyalist");
    else if (loyalGiveup && renegadeGiveup && rebelGiveup) {
        // if everyone give up, then ensure that the initiator doesn't win.
        QString playerRole = initiator->getRole();
        if (playerRole == "lord" || playerRole == "loyalist")
            gameOver(renegadeAlive >= rebelAlive ? "renegade" : "rebel");
        else if (playerRole == "renegade")
            gameOver(loyalAlive >= rebelAlive ? "loyalist+lord" : "rebel");
        else if (playerRole == "rebel")
            gameOver(renegadeAlive >= loyalAlive ? "renegade" : "loyalist+lord");
    }

    m_surrenderRequestReceived = false;

    initiator->setFlags("Global_ForbidSurrender");
    doNotify(initiator, S_COMMAND_ENABLE_SURRENDER, Json::Value(false));
    return true;
}

bool Room::processRequestSurrender(ServerPlayer *player, const QSanProtocol::QSanGeneralPacket *) {
    //@todo: Strictly speaking, the client must be in the PLAY phase
    //@todo: return false for 3v3 and 1v1!!!
    if (player == NULL || !player->m_isWaitingReply)
        return false;
    if (!_m_isFirstSurrenderRequest
        && _m_timeSinceLastSurrenderRequest.elapsed() <= Config.S_SURRENDER_REQUEST_MIN_INTERVAL)
        return false; //@todo: warn client here after new protocol has been enacted on the warn request

    _m_isFirstSurrenderRequest = false;
    _m_timeSinceLastSurrenderRequest.restart();
    m_surrenderRequestReceived = true;
    player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
    return true;
}

void Room::processClientPacket(const QString &request) {
    QSanGeneralPacket packet;
    //@todo: remove this thing after the new protocol is fully deployed
    if (packet.parse(request.toAscii().constData())) {
        ServerPlayer *player = qobject_cast<ServerPlayer *>(sender());
        if (packet.getPacketType() == S_TYPE_REPLY) {
            if (player == NULL) return;
            player->setClientReplyString(request);
            processResponse(player, &packet);
        } else if (packet.getPacketType() == S_TYPE_REQUEST) {
            CallBack callback = m_callbacks[packet.getCommandType()];
            if (!callback) return;
            (this->*callback)(player, &packet);
        }
    } else {
        QStringList args = request.split(" ");
        QString command = args.first();
        ServerPlayer *player = qobject_cast<ServerPlayer *>(sender());
        if (player == NULL) return;

        if (game_finished) {
            if (player->isOnline())
                player->invoke("warn", "GAME_OVER");
            return;
        }

        command.append("Command");
        Callback callback = callbacks.value(command, NULL);
        if (callback) {
            (this->*callback)(player, args.at(1));
#ifndef QT_NO_DEBUG
            // output client command only in debug version
            emit room_message(player->reportHeader() + request);
#endif
        } else
            emit room_message(tr("%1: %2 is not invokable").arg(player->reportHeader()).arg(command));
    }
}

void Room::addRobotCommand(ServerPlayer *player, const QString &) {
    if (player && !player->isOwner()) return;
    if (isFull()) return;

    int n = 0;
    foreach (ServerPlayer *player, m_players) {
        if (player->getState() == "robot")
            n++;
    }

    ServerPlayer *robot = new ServerPlayer(this);
    robot->setState("robot");

    m_players << robot;

    const QString robot_name = tr("Computer %1").arg(QChar('A' + n));
    const QString robot_avatar = Sanguosha->getRandomGeneralName();
    signup(robot, robot_name, robot_avatar, true);

    QString greeting = tr("Hello, I'm a robot").toUtf8().toBase64();
    speakCommand(robot, greeting);

    broadcastProperty(robot, "state");
}

void Room::fillRobotsCommand(ServerPlayer *player, const QString &) {
    int left = player_count - m_players.length();
    for (int i = 0; i < left; i++) {
        addRobotCommand(player, QString());
    }
}

ServerPlayer *Room::getOwner() const{
    foreach (ServerPlayer *player, m_players) {
        if (player->isOwner())
            return player;
    }

    return NULL;
}

void Room::toggleReadyCommand(ServerPlayer *player, const QString &) {
    if (game_started)
        return;

    setPlayerProperty(player, "ready", !player->isReady());

    if (player->isReady() && isFull()) {
        bool allReady = true;
        foreach (ServerPlayer *player, m_players) {
            if (!player->isReady()) {
                allReady = false;
                break;
            }
        }

        if (allReady) {
            foreach (ServerPlayer *player, m_players)
                setPlayerProperty(player, "ready", false);
            start();
        }
    }
}

void Room::signup(ServerPlayer *player, const QString &screen_name, const QString &avatar, bool is_robot) {
    player->setObjectName(generatePlayerName());
    player->setProperty("avatar", avatar);
    player->setScreenName(screen_name);

    if (!is_robot) {
        notifyProperty(player, player, "objectName");

        ServerPlayer *owner = getOwner();
        if (owner == NULL) {
            player->setOwner(true);
            notifyProperty(player, player, "owner");
        }
    }

    // introduce the new joined player to existing players except himself
    player->introduceTo(NULL);

    if (!is_robot) {
        QString greetingStr = tr("<font color=#EEB422>Player <b>%1</b> joined the game</font>").arg(screen_name);
        speakCommand(player, greetingStr.toUtf8().toBase64());
        player->startNetworkDelayTest();

        // introduce all existing player to the new joined
        foreach (ServerPlayer *p, m_players) {
            if (p != player)
                p->introduceTo(player);
        }
    } else
        toggleReadyCommand(player, QString());
}

void Room::assignGeneralsForPlayers(const QList<ServerPlayer *> &to_assign) {
    QSet<QString> existed;
    foreach (ServerPlayer *player, m_players) {
        if (player->getGeneral())
            existed << player->getGeneralName();
        if (player->getGeneral2())
            existed << player->getGeneral2Name();
    }
    if (Config.Enable2ndGeneral) {
        foreach (QString name, BanPair::getAllBanSet())
            existed << name;
        if (to_assign.first()->getGeneral()) {
            foreach (QString name, BanPair::getSecondBanSet())
                existed << name;
        }
    }

    const int max_choice = (Config.EnableHegemony && Config.Enable2ndGeneral) ?
                               Config.value("HegemonyMaxChoice", 7).toInt() :
                               Config.value("MaxChoice", 5).toInt();
    const int total = Sanguosha->getGeneralCount();
    const int max_available = (total - existed.size()) / to_assign.length();
    const int choice_count = qMin(max_choice, max_available);

    QStringList choices = Sanguosha->getRandomGenerals(total - existed.size(), existed);

    if (Config.EnableHegemony) {
        if (to_assign.first()->getGeneral()) {
            foreach (ServerPlayer *sp, m_players) {
                QStringList old_list = sp->getSelected();
                sp->clearSelected();
                QString choice;

                //keep legal generals
                foreach (QString name, old_list) {
                    Q_ASSERT(sp->getGeneral() != NULL);
                    if (Sanguosha->getGeneral(name)->getKingdom() != sp->getGeneral()->getKingdom()
                        || sp->findReasonable(old_list, true) == name) {
                        sp->addToSelected(name);
                        old_list.removeOne(name);
                    }
                }

                //drop the rest and add new generals
                while (old_list.length()) {
                    choice = sp->findReasonable(choices);
                    sp->addToSelected(choice);
                    old_list.pop_front();
                    choices.removeOne(choice);
                }
            }
            return;
        }
    }

    foreach (ServerPlayer *player, to_assign) {
        player->clearSelected();

        for (int i = 0; i < choice_count; i++) {
            QString choice = player->findReasonable(choices, true);
            if (choice.isEmpty()) break;
            player->addToSelected(choice);
            choices.removeOne(choice);
        }
    }
}

void Room::chooseGenerals() {
    // for lord.
    int lord_num = Config.value("LordMaxChoice", -1).toInt();
    int nonlord_num = Config.value("NonLordMaxChoice", 2).toInt();
    if (lord_num == 0 && nonlord_num == 0)
        nonlord_num = 1;
    int nonlord_prob = (lord_num == -1) ? 5 : 55 - qMin(lord_num, 10);
    if (!Config.EnableHegemony) {
        QStringList lord_list;
        ServerPlayer *the_lord = getLord();
        if (Config.EnableSame)
            lord_list = Sanguosha->getRandomGenerals(Config.value("MaxChoice", 5).toInt());
        else if (the_lord->getState() == "robot")
            if (((qrand() % 100 < nonlord_prob || lord_num == 0) && nonlord_num > 0)
                || Sanguosha->getLords().length() == 0)
                lord_list = Sanguosha->getRandomGenerals(1);
            else
                lord_list = Sanguosha->getLords();
        else
            lord_list = Sanguosha->getRandomLords();
        QString general = askForGeneral(the_lord, lord_list);
        the_lord->setGeneralName(general);
        if (!Config.EnableBasara)
            broadcastProperty(the_lord, "general", general);

        if (Config.EnableSame) {
            foreach (ServerPlayer *p, m_players) {
                if (!p->isLord())
                    p->setGeneralName(general);
            }

            Config.Enable2ndGeneral = false;
            return;
        }
    }
    QList<ServerPlayer *> to_assign = m_players;
    if (!Config.EnableHegemony) to_assign.removeOne(getLord());

    assignGeneralsForPlayers(to_assign);
    foreach (ServerPlayer *player, to_assign)
        _setupChooseGeneralRequestArgs(player);

    doBroadcastRequest(to_assign, S_COMMAND_CHOOSE_GENERAL);
    foreach (ServerPlayer *player, to_assign) {
        if (player->getGeneral() != NULL) continue;
        Json::Value generalName = player->getClientReply();
        if (!player->m_isClientResponseReady || !generalName.isString()
            || !_setPlayerGeneral(player, toQString(generalName), true))
            _setPlayerGeneral(player, _chooseDefaultGeneral(player), true);
    }

    if (Config.Enable2ndGeneral) {
        QList<ServerPlayer *> to_assign = m_players;
        assignGeneralsForPlayers(to_assign);
        foreach (ServerPlayer *player, to_assign)
            _setupChooseGeneralRequestArgs(player);

        doBroadcastRequest(to_assign, S_COMMAND_CHOOSE_GENERAL);
        foreach (ServerPlayer *player, to_assign) {
            if (player->getGeneral2() != NULL) continue;
            Json::Value generalName = player->getClientReply();
            if (!player->m_isClientResponseReady || !generalName.isString()
                || !_setPlayerGeneral(player, toQString(generalName), false))
                _setPlayerGeneral(player, _chooseDefaultGeneral(player), false);
        }
    }

    if (Config.EnableBasara) {
        foreach (ServerPlayer *player, m_players) {
            QStringList names;
            if (player->getGeneral()) {
                QString name = player->getGeneralName();
                names.append(name);
                player->setGeneralName("anjiang");
                notifyProperty(player, player, "general");
            }
            if (player->getGeneral2() && Config.Enable2ndGeneral) {
                QString name = player->getGeneral2Name();
                names.append(name);
                player->setGeneral2Name("anjiang");
                notifyProperty(player, player, "general2");
            }
            this->setTag(player->objectName(), QVariant::fromValue(names));
        }
    }
}

void Room::run() {
    // initialize random seed for later use
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
    Config.AIDelay = Config.OriginAIDelay;

    foreach (ServerPlayer *player, m_players) {
        //Ensure that the game starts with all player's mutex locked
        player->drainAllLocks();
        player->releaseLock(ServerPlayer::SEMA_MUTEX);
    }

    prepareForStart();

    bool using_countdown = true;
    if (_virtual || !property("to_test").toString().isEmpty())
        using_countdown = false;

#ifndef QT_NO_DEBUG
    using_countdown = false;
#endif

    if (using_countdown) {
        for (int i = Config.CountDownSeconds; i >= 0; i--) {
            broadcastInvoke("startInXs", QString::number(i));
            sleep(1);
        }
    } else
        broadcastInvoke("startInXs", "0");

    if (scenario && !scenario->generalSelection())
        startGame();
    else if (mode == "06_3v3") {
        thread_3v3 = new RoomThread3v3(this);
        thread_3v3->start();

        connect(thread_3v3, SIGNAL(finished()), this, SLOT(startGame()));
        connect(thread_3v3, SIGNAL(finished()), thread_3v3, SLOT(deleteLater()));
    } else if (mode == "06_XMode") {
        thread_xmode = new RoomThreadXMode(this);
        thread_xmode->start();

        connect(thread_xmode, SIGNAL(finished()), this, SLOT(startGame()));
        connect(thread_xmode, SIGNAL(finished()), thread_xmode, SLOT(deleteLater()));
    } else if (mode == "02_1v1") {
        thread_1v1 = new RoomThread1v1(this);
        thread_1v1->start();

        connect(thread_1v1, SIGNAL(finished()), this, SLOT(startGame()));
        connect(thread_1v1, SIGNAL(finished()), thread_1v1, SLOT(deleteLater()));
    } else if (mode == "04_1v3") {
        ServerPlayer *lord = m_players.first();
        setPlayerProperty(lord, "general", "shenlvbu1");

        QList<const General *> generals = QList<const General *>();
        foreach (QString pack_name, GetConfigFromLuaState(Sanguosha->getLuaState(), "hulao_packages").toStringList()) {
             const Package *pack = Sanguosha->findChild<const Package *>(pack_name);
             if (pack) generals << pack->findChildren<const General *>();
        }

        QStringList names;
        foreach (const General *general, generals) {
            if (general->isTotallyHidden())
                continue;
            names << general->objectName();
        }

        foreach (QString name, Config.value("Banlist/HulaoPass").toStringList())
            if (names.contains(name)) names.removeOne(name);

        foreach (ServerPlayer *player, m_players) {
            if (player == lord)
                continue;

            qShuffle(names);
            QStringList choices = names.mid(0, 3);
            QString name = askForGeneral(player, choices);

            setPlayerProperty(player, "general", name);
            names.removeOne(name);
        }

        startGame();
    } else {
        chooseGenerals();
        startGame();
    }
}

void Room::assignRoles() {
    int n = m_players.count();

    QStringList roles = Sanguosha->getRoleList(mode);
    qShuffle(roles);

    for (int i = 0; i < n; i++) {
        ServerPlayer *player = m_players[i];
        QString role = roles.at(i);

        player->setRole(role);
        if (role == "lord" && !ServerInfo.EnableHegemony)
            broadcastProperty(player, "role", "lord");
        else
            notifyProperty(player, player, "role");
    }
}

void Room::swapSeat(ServerPlayer *a, ServerPlayer *b) {
    int seat1 = m_players.indexOf(a);
    int seat2 = m_players.indexOf(b);

    m_players.swap(seat1, seat2);

    QStringList player_circle;
    foreach (ServerPlayer *player, m_players)
        player_circle << player->objectName();
    broadcastInvoke("arrangeSeats", player_circle.join("+"));

    m_alivePlayers.clear();
    for (int i = 0; i < m_players.length(); i++) {
        ServerPlayer *player = m_players.at(i);
        if (player->isAlive()) {
            m_alivePlayers << player;
            player->setSeat(m_alivePlayers.length());
        } else {
            player->setSeat(0);
        }

        broadcastProperty(player, "seat");

        player->setNext(m_players.at((i + 1) % m_players.length()));
    }
}

void Room::adjustSeats() {
    QList<ServerPlayer *> players;
    int i = 0;
    for (i = 0; i < m_players.length(); i++) {
        if (m_players.at(i)->getRoleEnum() == Player::Lord)
            break;
    }
    for (int j = i; j < m_players.length(); j++)
        players << m_players.at(j);
    for (int j = 0; j < i; j++)
        players << m_players.at(j);

    m_players = players;

    for (int i = 0; i < m_players.length(); i++)
        m_players.at(i)->setSeat(i + 1);

    // tell the players about the seat, and the first is always the lord
    QStringList player_circle;
    foreach (ServerPlayer *player, m_players)
        player_circle << player->objectName();

    broadcastInvoke("arrangeSeats", player_circle.join("+"));
}

int Room::getCardFromPile(const QString &card_pattern) {
    if (m_drawPile->isEmpty())
        swapPile();

    if (card_pattern.startsWith("@")) {
        if (card_pattern == "@duanliang") {
            foreach (int card_id, *m_drawPile) {
                const Card *card = Sanguosha->getCard(card_id);
                if (card->isBlack() && (card->isKindOf("BasicCard") || card->isKindOf("EquipCard")))
                    return card_id;
            }
        }
    } else {
        QString card_name = card_pattern;
        foreach (int card_id, *m_drawPile) {
            const Card *card = Sanguosha->getCard(card_id);
            if (card->objectName() == card_name)
                return card_id;
        }
    }

    return -1;
}

QString Room::_chooseDefaultGeneral(ServerPlayer *player) const{
    Q_ASSERT(!player->getSelected().isEmpty());
    if (Config.EnableHegemony && Config.Enable2ndGeneral) {
        foreach (QString name, player->getSelected()) {
            Q_ASSERT(!name.isEmpty());
            if (player->getGeneral() != NULL) { // choosing first general
                if (name == player->getGeneralName()) continue;
                Q_ASSERT(Sanguosha->getGeneral(name) != NULL);
                Q_ASSERT(player->getGeneral() != NULL);
                if (Sanguosha->getGeneral(name)->getKingdom() == player->getGeneral()->getKingdom())
                    return name;
            } else {
                foreach (QString other, player->getSelected()) { // choosing second general
                    if (name == other) continue;
                    Q_ASSERT(Sanguosha->getGeneral(other) != NULL);
                    Q_ASSERT(Sanguosha->getGeneral(name) != NULL);
                    if (Sanguosha->getGeneral(name)->getKingdom() == Sanguosha->getGeneral(other)->getKingdom())
                        return name;
                }
            }
        }
        Q_ASSERT(false);
        return QString();
    } else {
        GeneralSelector *selector = GeneralSelector::getInstance();
        QString choice = selector->selectFirst(player, player->getSelected());
        return choice;
    }
}

bool Room::_setPlayerGeneral(ServerPlayer *player, const QString &generalName, bool isFirst) {
    const General *general = Sanguosha->getGeneral(generalName);
    if (general == NULL)
        return false;
    else if (!Config.FreeChoose && !player->getSelected().contains(generalName))
        return false;

    if (isFirst) {
        player->setGeneralName(general->objectName());
        notifyProperty(player, player, "general");
    } else {
        player->setGeneral2Name(general->objectName());
        notifyProperty(player, player, "general2");
    }
    return true;
}

void Room::speakCommand(ServerPlayer *player, const QString &arg) {
#define _NO_BROADCAST_SPEAKING {\
                                   broadcast = false;\
                                   player->invoke("speak", QString("%1:%2").arg(player->objectName()).arg(arg));\
                               }
    bool broadcast = true;
    if (player && Config.EnableCheat) {
        QString sentence = QString::fromUtf8(QByteArray::fromBase64(arg.toAscii()));
        if (sentence == ".BroadcastRoles") {
            _NO_BROADCAST_SPEAKING
            foreach (ServerPlayer *p, m_alivePlayers)
                broadcastProperty(p, "role", p->getRole());
        } else if (sentence.startsWith(".BroadcastRoles=")) {
            _NO_BROADCAST_SPEAKING
            QString name = sentence.mid(12);
            foreach (ServerPlayer *p, m_alivePlayers) {
                if (p->objectName() == name || p->getGeneralName() == name) {
                    broadcastProperty(p, "role", p->getRole());
                    break;
                }
            }
        } else if (sentence == ".ShowHandCards") {
            _NO_BROADCAST_SPEAKING
            QString split("----------");
            split = split.toUtf8().toBase64();
            player->invoke("speak", QString("%1:%2").arg(player->objectName()).arg(split));
            foreach (ServerPlayer *p, m_alivePlayers) {
                if (!p->isKongcheng()) {
                    QStringList handcards;
                    foreach (const Card *card, p->getHandcards())
                        handcards << QString("<b>%1</b>")
                                             .arg(Sanguosha->getEngineCard(card->getId())->getLogName());
                    QString hand = handcards.join(", ");
                    hand = hand.toUtf8().toBase64();
                    player->invoke("speak", QString("%1:%2").arg(p->objectName()).arg(hand));
                }
            }
            player->invoke("speak", QString("%1:%2").arg(player->objectName()).arg(split));
        } else if (sentence.startsWith(".ShowHandCards=")) {
            _NO_BROADCAST_SPEAKING
            QString name = sentence.mid(15);
            foreach (ServerPlayer *p, m_alivePlayers) {
                if (p->objectName() == name || p->getGeneralName() == name) {
                    if (!p->isKongcheng()) {
                        QStringList handcards;
                        foreach (const Card *card, p->getHandcards())
                            handcards << QString("<b>%1</b>")
                                                 .arg(Sanguosha->getEngineCard(card->getId())->getLogName());
                        QString hand = handcards.join(", ");
                        hand = hand.toUtf8().toBase64();
                        player->invoke("speak", QString("%1:%2").arg(p->objectName()).arg(hand));
                    }
                    break;
                }
            }
        } else if (sentence.startsWith(".ShowPrivatePile=")) {
            _NO_BROADCAST_SPEAKING
            QStringList arg = sentence.mid(17).split(":");
            if (arg.length() == 2) {
                QString name = arg.first();
                QString pile_name = arg.last();
                foreach (ServerPlayer *p, m_alivePlayers) {
                    if (p->objectName() == name || p->getGeneralName() == name) {
                        if (!p->getPile(pile_name).isEmpty()) {
                            QStringList pile_cards;
                            foreach (int id, p->getPile(pile_name))
                                pile_cards << QString("<b>%1</b>").arg(Sanguosha->getEngineCard(id)->getLogName());
                            QString pile = pile_cards.join(", ");
                            pile = pile.toUtf8().toBase64();
                            player->invoke("speak", QString("%1:%2").arg(p->objectName()).arg(pile));
                        }
                        break;
                    }
                }
            }
        } else if (sentence == ".ShowHuashen") {
            _NO_BROADCAST_SPEAKING
            QList<ServerPlayer *> zuocis = findPlayersBySkillName("huashen");
            QStringList huashen_name;
            foreach (ServerPlayer *zuoci, zuocis) {
                QVariantList huashens = zuoci->tag["Huashens"].toList();
                huashen_name.clear();
                foreach (QVariant name, huashens)
                    huashen_name << QString("<b>%1</b>").arg(Sanguosha->translate(name.toString()));
                QString huashen = huashen_name.join(", ");
                huashen = huashen.toUtf8().toBase64();
                player->invoke("speak", QString("%1:%2").arg(zuoci->objectName()).arg(huashen));
            }
        } else if (sentence.startsWith(".SetAIDelay=")) {
            _NO_BROADCAST_SPEAKING
            bool ok = false;
            int delay = sentence.mid(12).toInt(&ok);
            if (ok) {
                Config.AIDelay = Config.OriginAIDelay = delay;
                Config.setValue("OriginAIDelay", delay);
            }
        } else if (sentence.startsWith(".SetGameMode=")) {
            _NO_BROADCAST_SPEAKING
            QString name = sentence.mid(13);
            setTag("NextGameMode", name);
        } else if (sentence.startsWith(".SecondGeneral=")) {
            _NO_BROADCAST_SPEAKING
            QString prop = sentence.mid(15);
            setTag("NextGameSecondGeneral", !prop.isEmpty() && prop != "0" && prop != "false");
        } else if (sentence == ".Pause") {
            _NO_BROADCAST_SPEAKING
            pauseCommand(player, "true");
        } else if (sentence == ".Resume") {
            _NO_BROADCAST_SPEAKING
            pauseCommand(player, "false");
        }
    }
    if (broadcast)
        broadcastInvoke("speak", QString("%1:%2").arg(player->objectName()).arg(arg));
#undef _NO_BROADCAST_SPEAKING
}

void Room::processResponse(ServerPlayer *player, const QSanGeneralPacket *packet) {
    player->acquireLock(ServerPlayer::SEMA_MUTEX);
    bool success = false;
    if (player == NULL)
        emit room_message(tr("Unable to parse player"));
    else if (!player->m_isWaitingReply || player->m_isClientResponseReady)
        emit room_message(tr("Server is not waiting for reply from %1").arg(player->objectName()));
    else if (packet->getCommandType() != player->m_expectedReplyCommand)
        emit room_message(tr("Reply command should be %1 instead of %2")
                             .arg(player->m_expectedReplyCommand).arg(packet->getCommandType()));
    else if (packet->m_localSerial != player->m_expectedReplySerial)
        emit room_message(tr("Reply serial should be %1 instead of %2")
                             .arg(player->m_expectedReplySerial).arg(packet->m_localSerial));
    else
        success = true;

    if (!success) {
        player->releaseLock(ServerPlayer::SEMA_MUTEX);
        return;
    } else {
        _m_semRoomMutex.acquire();
        if (_m_raceStarted) {
            player->setClientReply(packet->getMessageBody());
            player->m_isClientResponseReady = true;
            // Warning: the statement below must be the last one before releasing the lock!!!
            // Any statement after this statement will totally compromise the synchronization
            // because getRaceResult will then be able to acquire the lock, reading a non-null
            // raceWinner and proceed with partial data. The current implementation is based on
            // the assumption that the following line is ATOMIC!!!
            // @todo: Find a Qt atomic semantic or use _asm to ensure the following line is atomic
            // on a multi-core machine. This is the core to the whole synchornization mechanism for
            // broadcastRaceRequest.
            _m_raceWinner = player;
            // the _m_semRoomMutex.release() signal is in getRaceResult();
            _m_semRaceRequest.release();
        } else {
            _m_semRoomMutex.release();
            player->setClientReply(packet->getMessageBody());
            player->m_isClientResponseReady = true;
            player->releaseLock(ServerPlayer::SEMA_COMMAND_INTERACTIVE);
        }

        player->releaseLock(ServerPlayer::SEMA_MUTEX);
    }
}

bool Room::useCard(const CardUseStruct &use, bool add_history) {
    CardUseStruct card_use = use;
    card_use.m_addHistory = false;
    const Card *card = card_use.card;

    if (card_use.from->isCardLimited(card, card->getHandlingMethod())
        && (!card->canRecast() || card_use.from->isCardLimited(card, Card::MethodRecast)))
        return true;

    QString key;
    if (card->inherits("LuaSkillCard"))
        key = "#" + card->objectName();
    else
        key = card->getClassName();
    int slash_count = card_use.from->getSlashCount();
    bool slash_not_record = key.contains("Slash")
                            && slash_count > 0
                            && (card_use.from->hasWeapon("Crossbow")
                                || Sanguosha->correctCardTarget(TargetModSkill::Residue, card_use.from, card) > 500);

    card = card_use.card->validate(card_use);
    if (card == NULL)
        return false;

    if (card_use.from->getPhase() == Player::Play && add_history) {
        if (!slash_not_record) {
            card_use.m_addHistory = true;
            addPlayerHistory(card_use.from, key);
        }
        addPlayerHistory(NULL, "pushPile");
    }

    try {
        if (card_use.card->getRealCard() == card) {
            if (card->isKindOf("DelayedTrick") && card->isVirtualCard() && card->subcardsLength() == 1) {
                Card *trick = Sanguosha->cloneCard(card);
                Q_ASSERT(trick != NULL);
                WrappedCard *wrapped = Sanguosha->getWrappedCard(card->getSubcards().first());
                wrapped->takeOver(trick);
                broadcastUpdateCard(getPlayers(), wrapped->getId(), wrapped);
                card_use.card = wrapped;
                wrapped->onUse(this, card_use);
                return true;
            }
            if (card_use.card->isKindOf("Slash") && add_history && slash_count > 0)
                card_use.from->setFlags("Global_MoreSlashInOneTurn");
            if (!card_use.card->isVirtualCard()) {
                WrappedCard *wrapped = Sanguosha->getWrappedCard(card_use.card->getEffectiveId());
                if (wrapped->isModified())
                    broadcastUpdateCard(getPlayers(), card_use.card->getEffectiveId(), wrapped);
                else
                    broadcastResetCard(getPlayers(), card_use.card->getEffectiveId());
            }
            card_use.card->onUse(this, card_use);
        } else if (card) {
            CardUseStruct new_use = card_use;
            new_use.card = card;
            useCard(new_use);
        }
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == StageChange || triggerEvent == TurnBroken) {
            if (getCardPlace(card_use.card->getEffectiveId()) == Player::PlaceTable) {
                CardMoveReason reason(CardMoveReason::S_REASON_UNKNOWN, card_use.from->objectName(), QString(), card_use.card->getSkillName(), QString());
                if (card_use.to.size() == 1) reason.m_targetId = card_use.to.first()->objectName();
                moveCardTo(card_use.card, card_use.from, NULL, Player::DiscardPile, reason, true);
            }
            QVariant data = QVariant::fromValue(card_use);
            thread->trigger(CardFinished, this, card_use.from, data);

            foreach (ServerPlayer *p, m_alivePlayers) {
                p->tag.remove("Qinggang");

                foreach (QString flag, p->getFlagList()) {
                    if (flag == "Global_GongxinOperator")
                        p->setFlags("-" + flag);
                    else if (flag.endsWith("_InTempMoving"))
                        setPlayerFlag(p, "-" + flag);
                }
            }

            foreach (int id, Sanguosha->getRandomCards()) {
                if (getCardPlace(id) == Player::PlaceTable || getCardPlace(id) == Player::PlaceJudge)
                    moveCardTo(Sanguosha->getCard(id), NULL, Player::DiscardPile, true);
                if (Sanguosha->getCard(id)->hasFlag("using"))
                    setCardFlag(id, "-using");
            }
        }
        throw triggerEvent;
    }
    return true;
}

void Room::loseHp(ServerPlayer *victim, int lose) {
    if (victim->isDead())
        return;
    QVariant data = lose;
    if (thread->trigger(PreHpLost, this, victim, data))
        return;

    LogMessage log;
    log.type = "#LoseHp";
    log.from = victim;
    log.arg = QString::number(lose);
    sendLog(log);

    setPlayerProperty(victim, "hp", victim->getHp() - lose);

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(victim->objectName());
    arg[1] = -lose;
    arg[2] = -1;
    doBroadcastNotify(S_COMMAND_CHANGE_HP, arg);

    thread->trigger(PostHpReduced, this, victim, data);
}

void Room::loseMaxHp(ServerPlayer *victim, int lose) {
    int hp_1 = victim->getHp();
    victim->setMaxHp(qMax(victim->getMaxHp() - lose, 0));
    int hp_2 = victim->getHp();

    broadcastProperty(victim, "maxhp");
    broadcastProperty(victim, "hp");

    LogMessage log;
    log.type = "#LoseMaxHp";
    log.from = victim;
    log.arg = QString::number(lose);
    sendLog(log);

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(victim->objectName());
    arg[1] = -lose;
    doBroadcastNotify(S_COMMAND_CHANGE_MAXHP, arg);

    LogMessage log2;
    log2.type = "#GetHp";
    log2.from = victim;
    log2.arg = QString::number(victim->getHp());
    log2.arg2 = QString::number(victim->getMaxHp());
    sendLog(log2);

    if (victim->getMaxHp() == 0)
        killPlayer(victim);
    else {
        thread->trigger(MaxHpChanged, this, victim);
        if (hp_1 > hp_2) {
            QVariant data = QVariant::fromValue(hp_1 - hp_2);
            thread->trigger(PostHpReduced, this, victim, data);
        }
    }
}

bool Room::changeMaxHpForAwakenSkill(ServerPlayer *player, int magnitude) {
    player->gainMark("@waked");
    int n = player->getMark("@waked");
    if (magnitude < 0) {
        if (Config.Enable2ndGeneral && player->getGeneral() && player->getGeneral2()
            && Config.MaxHpScheme > 0 && Config.PreventAwakenBelow3
            && player->getMaxHp() <= 3) {
            setPlayerMark(player, "AwakenLostMaxHp", 1);
        } else {
            loseMaxHp(player, -magnitude);
        }
    } else {
        setPlayerProperty(player, "maxhp", player->getMaxHp() + magnitude);
    }
    return (player->getMark("@waked") >= n);
}

void Room::applyDamage(ServerPlayer *victim, const DamageStruct &damage) {
    int new_hp = victim->getHp() - damage.damage;

    setPlayerProperty(victim, "hp", new_hp);
    QString change_str = QString("%1:%2").arg(victim->objectName()).arg(-damage.damage);
    switch (damage.nature) {
    case DamageStruct::Fire: change_str.append("F"); break;
    case DamageStruct::Thunder: change_str.append("T"); break;
    default: break;
    }

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(victim->objectName());
    arg[1] = -damage.damage;
    arg[2] = int(damage.nature);
    doBroadcastNotify(S_COMMAND_CHANGE_HP, arg);
}

void Room::recover(ServerPlayer *player, const RecoverStruct &recover, bool set_emotion) {
    if (player->getLostHp() == 0 || player->isDead())
        return;
    RecoverStruct recover_struct = recover;

    QVariant data = QVariant::fromValue(recover_struct);
    if (thread->trigger(PreHpRecover, this, player, data))
        return;

    recover_struct = data.value<RecoverStruct>();
    int recover_num = recover_struct.recover;

    int new_hp = qMin(player->getHp() + recover_num, player->getMaxHp());
    setPlayerProperty(player, "hp", new_hp);

    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(player->objectName());
    arg[1] = recover_num;
    arg[2] = 0;
    doBroadcastNotify(S_COMMAND_CHANGE_HP, arg);

    if (set_emotion) setEmotion(player, "recover");

    thread->trigger(HpRecover, this, player, data);
}

bool Room::cardEffect(const Card *card, ServerPlayer *from, ServerPlayer *to) {
    CardEffectStruct effect;
    effect.card = card;
    effect.from = from;
    effect.to = to;

    return cardEffect(effect);
}

bool Room::cardEffect(const CardEffectStruct &effect) {
    QVariant data = QVariant::fromValue(effect);
    bool cancel = false;
    if (effect.to->isAlive() || effect.card->isKindOf("Slash")) { // Be care!!!
        // No skills should be triggered here!
        thread->trigger(CardEffect, this, effect.to, data);
        // Make sure that effectiveness of Slash isn't judged here!
        if (!thread->trigger(CardEffected, this, effect.to, data)) {
            cancel = true;
        } else {
            if (!effect.to->hasFlag("Global_NonSkillNullify"))
                ;//setEmotion(effect.to, "skill_nullify");
            else
                effect.to->setFlags("-Global_NonSkillNullify");
        }
    }
    thread->trigger(PostCardEffected, this, effect.to, data);
    return cancel;
}

bool Room::isJinkEffected(ServerPlayer *user, const Card *jink) {
    if (jink == NULL || user == NULL)
        return false;
    Q_ASSERT(jink->isKindOf("Jink"));
    QVariant jink_data = QVariant::fromValue((CardStar)jink);
    return !thread->trigger(JinkEffect, this, user, jink_data);
}

void Room::damage(const DamageStruct &data) {
    DamageStruct damage_data = data;
    if (damage_data.to == NULL || damage_data.to->isDead())
        return;

    QVariant qdata = QVariant::fromValue(damage_data);

    if (!damage_data.chain && !damage_data.transfer) {
        thread->trigger(ConfirmDamage, this, damage_data.from, qdata);
        damage_data = qdata.value<DamageStruct>();
    }

    // Predamage
    if (thread->trigger(Predamage, this, damage_data.from, qdata)) {
        if (damage_data.card && damage_data.card->isKindOf("Slash")) {
            QStringList qinggang = damage_data.to->tag["Qinggang"].toStringList();
            if (!qinggang.isEmpty()) {
                qinggang.removeOne(damage_data.card->toString());
                damage_data.to->tag["Qinggang"] = qinggang;
            }
        }
        return;
    }

    try {
        bool enter_stack = false;
        do {
            bool prevent = thread->trigger(DamageForseen, this, damage_data.to, qdata);
            if (prevent)
                break;

            if (damage_data.from) {
                if (thread->trigger(DamageCaused, this, damage_data.from, qdata))
                    break;
            }

            damage_data = qdata.value<DamageStruct>();

            bool broken = thread->trigger(DamageInflicted, this, damage_data.to, qdata);
            if (broken)
                break;

            enter_stack = true;
            m_damageStack.push_back(damage_data);
            setTag("CurrentDamageStruct", qdata);

            thread->trigger(PreDamageDone, this, damage_data.to, qdata);

            if (damage_data.card && damage_data.card->isKindOf("Slash")) {
                QStringList qinggang = damage_data.to->tag["Qinggang"].toStringList();
                if (!qinggang.isEmpty()) {
                    qinggang.removeOne(damage_data.card->toString());
                    damage_data.to->tag["Qinggang"] = qinggang;
                }
            }
            thread->trigger(DamageDone, this, damage_data.to, qdata);

            if (damage_data.from)
                thread->trigger(Damage, this, damage_data.from, qdata);

            thread->trigger(Damaged, this, damage_data.to, qdata);
        } while (false);

        damage_data = qdata.value<DamageStruct>();
        thread->trigger(DamageComplete, this, damage_data.to, qdata);

        if (enter_stack) {
            m_damageStack.pop();
            if (m_damageStack.isEmpty())
                removeTag("CurrentDamageStruct");
            else
                setTag("CurrentDamageStruct", QVariant::fromValue(m_damageStack.first()));
        }
    }
    catch (TriggerEvent triggerEvent) {
        if (triggerEvent == StageChange || triggerEvent == TurnBroken) {
            removeTag("is_chained");
            removeTag("CurrentDamageStruct");
            m_damageStack.clear();
        }
        throw triggerEvent;
    }
}

void Room::sendDamageLog(const DamageStruct &data) {
    LogMessage log;

    if (data.from) {
        log.type = "#Damage";
        log.from = data.from;
    } else {
        log.type = "#DamageNoSource";
    }

    log.to << data.to;
    log.arg = QString::number(data.damage);

    switch (data.nature) {
    case DamageStruct::Normal: log.arg2 = "normal_nature"; break;
    case DamageStruct::Fire: log.arg2 = "fire_nature"; break;
    case DamageStruct::Thunder: log.arg2 = "thunder_nature"; break;
    }

    sendLog(log);
}

bool Room::hasWelfare(const ServerPlayer *player) const{
    if (mode == "06_3v3")
        return player->isLord() || player->getRole() == "renegade";
    else if (mode == "04_1v3")
        return false;
    else if (Config.EnableHegemony || mode == "06_XMode")
        return false;
    else
        return player->isLord() && player_count > 4;
}

ServerPlayer *Room::getFront(ServerPlayer *a, ServerPlayer *b) const{
    ServerPlayer *p;
    for (p = current; ; p = p->getNext()) {
        if (p == a)
            return a;
        else if (p == b)
            return b;
    }

    return a;
}

void Room::reconnect(ServerPlayer *player, ClientSocket *socket) {
    /*
    player->setSocket(socket);
    player->setState("online");

    marshal(player);

    broadcastProperty(player, "state"); */
}

void Room::marshal(ServerPlayer *player) {
    notifyProperty(player, player, "objectName");
    notifyProperty(player, player, "role");
    player->unicast(".flags marshalling");

    foreach (ServerPlayer *p, m_players) {
        if (p != player)
            p->introduceTo(player);
    }

    QStringList player_circle;
    foreach (ServerPlayer *player, m_players)
        player_circle << player->objectName();

    player->invoke("arrangeSeats", player_circle.join("+"));
    player->invoke("startInXs", "0");

    foreach (ServerPlayer *p, m_players) {
        notifyProperty(player, p, "general");

        if (p->getGeneral2())
            notifyProperty(player, p, "general2");
    }

    player->invoke("startGame");

    foreach (ServerPlayer *p, m_players)
        p->marshal(player);

    player->unicast(".flags -marshalling");
    player->invoke("setPileNumber", QString::number(m_drawPile->length()));
}

void Room::startGame() {
    m_alivePlayers = m_players;
    for (int i = 0; i < player_count - 1; i++)
        m_players.at(i)->setNext(m_players.at(i + 1));
    m_players.last()->setNext(m_players.first());

    foreach (ServerPlayer *player, m_players) {
        Q_ASSERT(player->getGeneral());
        player->setMaxHp(player->getGeneralMaxHp());
        player->setHp(player->getMaxHp());
        // setup AI
        AI *ai = cloneAI(player);
        ais << ai;
        player->setAI(ai);
    }

    foreach (ServerPlayer *player, m_players) {
        if (!Config.EnableBasara
            && (mode == "06_3v3" || mode == "02_1v1" || mode == "06_XMode" || !player->isLord()))
            broadcastProperty(player, "general");

        if (mode == "02_1v1")
            doBroadcastNotify(getOtherPlayers(player, true), S_COMMAND_REVEAL_GENERAL, toJsonArray(player->objectName(), player->getGeneralName()));

        if (Config.Enable2ndGeneral
            && mode != "02_1v1" && mode != "06_3v3" && mode != "06_XMode" && mode != "04_1v3"
            && !Config.EnableBasara)
            broadcastProperty(player, "general2");

        broadcastProperty(player, "hp");
        broadcastProperty(player, "maxhp");

        if (mode == "06_3v3" || mode == "06_XMode")
            broadcastProperty(player, "role");
    }

    preparePlayers();

    QList<int> drawPile = *m_drawPile;
    qShuffle(drawPile);
    doBroadcastNotify(S_COMMAND_AVAILABLE_CARDS, toJsonArray(drawPile));

    doBroadcastNotify(S_COMMAND_GAME_START, Json::Value::null);
    game_started = true;

    Server *server = qobject_cast<Server *>(parent());
    foreach (ServerPlayer *player, m_players) {
        if (player->getState() == "online")
            server->signupPlayer(player);
    }

    current = m_players.first();

    // initialize the place_map and owner_map;
    foreach (int card_id, *m_drawPile)
        setCardMapping(card_id, NULL, Player::DrawPile);
    doBroadcastNotify(S_COMMAND_UPDATE_PILE, Json::Value(m_drawPile->length()));

    thread = new RoomThread(this);
    if (mode != "02_1v1" && mode != "06_3v3" && mode != "06_XMode")
        _m_roomState.reset();
    connect(thread, SIGNAL(started()), this, SIGNAL(game_start()));

    if (!_virtual) thread->start();
}

bool Room::notifyProperty(ServerPlayer *playerToNotify, const ServerPlayer *propertyOwner, const char *propertyName, const QString &value) {
    if (propertyOwner == NULL) return false;
    QString real_value = value;
    if (real_value.isNull()) real_value = propertyOwner->property(propertyName).toString();
    Json::Value arg(Json::arrayValue);
    if (propertyOwner == playerToNotify)
        arg[0] = toJsonString(QString(QSanProtocol::S_PLAYER_SELF_REFERENCE_ID));
    else
        arg[0] = toJsonString(propertyOwner->objectName());
    arg[1] = propertyName;
    arg[2] = toJsonString(real_value);
    return doNotify(playerToNotify, S_COMMAND_SET_PROPERTY, arg);
}

bool Room::broadcastProperty(ServerPlayer *player, const char *property_name, const QString &value) {
    if (player == NULL) return false;
    QString real_value = value;
    if (real_value.isNull()) real_value = player->property(property_name).toString();
    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(player->objectName());
    arg[1] = property_name;
    arg[2] = toJsonString(real_value);
    return doBroadcastNotify(S_COMMAND_SET_PROPERTY, arg);
}

void Room::drawCards(ServerPlayer *player, int n, const QString &reason) {
    QList<ServerPlayer *> players;
    players.append(player);
    drawCards(players, n, reason);
}

void Room::drawCards(QList<ServerPlayer *> players, int n, const QString &reason) {
    if (n <= 0) return;
    QList<CardsMoveStruct> moves;
    foreach (ServerPlayer *player, players) {
        if (!player->isAlive() && reason != "reform") continue;
        QList<int> card_ids = getNCards(n, false);

        CardsMoveStruct move;
        move.card_ids = card_ids;
        move.from = NULL;
        move.to = player;
        move.to_place = Player::PlaceHand;
        moves.append(move);
    }
    moveCardsAtomic(moves, false);
}

void Room::throwCard(const Card *card, ServerPlayer *who, ServerPlayer *thrower) {
    CardMoveReason reason;
    if (thrower == NULL) {
        reason.m_reason = CardMoveReason::S_REASON_THROW;
        reason.m_playerId = who ? who->objectName() : QString();
    } else {
        reason.m_reason = CardMoveReason::S_REASON_DISMANTLE;
        reason.m_targetId = who ? who->objectName() : QString();
        reason.m_playerId = thrower->objectName();
    }
    reason.m_skillName = card->getSkillName();
    throwCard(card, reason, who, thrower);
}

void Room::throwCard(const Card *card, const CardMoveReason &reason, ServerPlayer *who, ServerPlayer *thrower) {
    if (card == NULL)
        return;

    QList<int> to_discard;
    if (card->isVirtualCard())
        to_discard.append(card->getSubcards());
    else
        to_discard << card->getEffectiveId();

    LogMessage log;
    if (who) {
        if (thrower == NULL) {
            log.type = "$DiscardCard";
            log.from = who;
        } else {
            log.type = "$DiscardCardByOther";
            log.from = thrower;
            log.to << who;
        }
    } else {
        log.type = "$EnterDiscardPile";
    }
    log.card_str = IntList2StringList(to_discard).join("+");
    sendLog(log);

    QList<CardsMoveStruct> moves;
    if (who) {
        CardsMoveStruct move(to_discard, who, NULL, Player::DiscardPile, reason);
        moves.append(move);
        moveCardsAtomic(moves, true);
    } else {
        CardsMoveStruct move(to_discard, NULL, Player::DiscardPile, reason);
        moves.append(move);
        moveCardsAtomic(moves, true);
    }
}

void Room::throwCard(int card_id, ServerPlayer *who, ServerPlayer *thrower) {
    throwCard(Sanguosha->getCard(card_id), who, thrower);
}

RoomThread *Room::getThread() const{
    return thread;
}

void Room::moveCardTo(const Card *card, ServerPlayer *dstPlayer, Player::Place dstPlace, bool forceMoveVisible) {
    moveCardTo(card, dstPlayer, dstPlace,
               CardMoveReason(CardMoveReason::S_REASON_UNKNOWN, QString()), forceMoveVisible);
}

void Room::moveCardTo(const Card *card, ServerPlayer *dstPlayer, Player::Place dstPlace,
                      const CardMoveReason &reason, bool forceMoveVisible) {
    moveCardTo(card, NULL, dstPlayer, dstPlace, QString(), reason, forceMoveVisible);
}

void Room::moveCardTo(const Card *card, ServerPlayer *srcPlayer, ServerPlayer *dstPlayer, Player::Place dstPlace,
                      const CardMoveReason &reason, bool forceMoveVisible) {
    moveCardTo(card, srcPlayer, dstPlayer, dstPlace, QString(), reason, forceMoveVisible);
}

void Room::moveCardTo(const Card *card, ServerPlayer *srcPlayer, ServerPlayer *dstPlayer, Player::Place dstPlace,
                      const QString &pileName, const CardMoveReason &reason, bool forceMoveVisible) {
    CardsMoveStruct move;
    if (card->isVirtualCard()) {
        move.card_ids = card->getSubcards();
        if (move.card_ids.size() == 0) return;
    } else
        move.card_ids.append(card->getId());
    move.to = dstPlayer;
    move.to_place = dstPlace;
    move.to_pile_name = pileName;
    move.from = srcPlayer;
    move.reason = reason;
    QList<CardsMoveStruct> moves;
    moves.append(move);
    moveCardsAtomic(moves, forceMoveVisible);
}

void Room::moveCards(CardsMoveStruct cards_move, bool forceMoveVisible, bool ignoreChanges) {
    QList<CardsMoveStruct> cards_moves;
    cards_moves.append(cards_move);
    moveCards(cards_moves, forceMoveVisible, ignoreChanges);
}

void Room::_fillMoveInfo(CardsMoveStruct &moves, int card_index) const{
    int card_id = moves.card_ids[card_index];
    if (!moves.from)
        moves.from = getCardOwner(card_id);
    moves.from_place = getCardPlace(card_id);
    if (moves.from) { // Hand/Equip/Judge
        if (moves.from_place == Player::PlaceSpecial || moves.from_place == Player::PlaceTable)
            moves.from_pile_name = moves.from->getPileName(card_id);
        moves.from_player_name = moves.from->objectName();
    }
    if (moves.to) {
        if (moves.to->isAlive()) {
            moves.to_player_name = moves.to->objectName();
            int card_id = moves.card_ids[card_index];
            if (moves.to_place == Player::PlaceSpecial || moves.to_place == Player::PlaceTable)
                moves.to_pile_name = moves.to->getPileName(card_id);
        } else {
            moves.to = NULL;
            moves.to_place = Player::DiscardPile;
            return;
        }
    }
}

static bool CompareByActionOrder_OneTime(CardsMoveOneTimeStruct move1, CardsMoveOneTimeStruct move2) {
    ServerPlayer *a = (ServerPlayer *)move1.from;
    if (a == NULL) a = (ServerPlayer *)move1.to;
    ServerPlayer *b = (ServerPlayer *)move2.from;
    if (b == NULL) b = (ServerPlayer *)move2.to;

    if (a == NULL || b == NULL)
        return a != NULL;

    Room *room = a->getRoom();
    return room->getFront(a, b) == a;
}

static bool CompareByActionOrder(CardsMoveStruct move1, CardsMoveStruct move2) {
    ServerPlayer *a = (ServerPlayer *)move1.from;
    if (a == NULL) a = (ServerPlayer *)move1.to;
    ServerPlayer *b = (ServerPlayer *)move2.from;
    if (b == NULL) b = (ServerPlayer *)move2.to;

    if (a == NULL || b == NULL)
        return a != NULL;

    Room *room = a->getRoom();
    return room->getFront(a, b) == a;
}

QList<CardsMoveOneTimeStruct> Room::_mergeMoves(QList<CardsMoveStruct> cards_moves) {
    QMap<_MoveMergeClassifier, QList<CardsMoveStruct> > moveMap;

    foreach (CardsMoveStruct cards_move, cards_moves) {
        _MoveMergeClassifier classifier(cards_move);
        moveMap[classifier].append(cards_move);
    }

    QList<CardsMoveOneTimeStruct> result;
    foreach (_MoveMergeClassifier cls, moveMap.keys()) {
        CardsMoveOneTimeStruct moveOneTime;
        moveOneTime.from = cls.m_from;
        moveOneTime.reason = moveMap[cls].first().reason;
        moveOneTime.to = cls.m_to;
        moveOneTime.to_place = cls.m_to_place;
        moveOneTime.to_pile_name = cls.m_to_pile_name;
        foreach (CardsMoveStruct move, moveMap[cls]) {
            moveOneTime.card_ids.append(move.card_ids);
            for (int i = 0; i < move.card_ids.size(); i++) {
                moveOneTime.from_places.append(move.from_place);
                moveOneTime.from_pile_names.append(move.from_pile_name);
                moveOneTime.open.append(move.open);
            }
        }
        result.append(moveOneTime);
    }

    if (result.size() > 1)
        qSort(result.begin(), result.end(), CompareByActionOrder_OneTime);

    return result;
}

QList<CardsMoveStruct> Room::_separateMoves(QList<CardsMoveOneTimeStruct> moveOneTimes) {
    QList<_MoveSeparateClassifier> classifiers;
    QList<QList<int> > ids;
    foreach (CardsMoveOneTimeStruct moveOneTime, moveOneTimes) {
        for (int i = 0; i < moveOneTime.card_ids.size(); i++) {
            _MoveSeparateClassifier classifier(moveOneTime, i);
            if (classifiers.contains(classifier)) {
                int pos = classifiers.indexOf(classifier);
                ids[pos].append(moveOneTime.card_ids[i]);
            } else {
                classifiers << classifier;
                QList<int> new_ids;
                new_ids << moveOneTime.card_ids[i];
                ids << new_ids;
            }
        }
    }

    QList<CardsMoveStruct> card_moves;
    int i = 0;
    foreach (_MoveSeparateClassifier cls, classifiers) {
        CardsMoveStruct card_move;
        card_move.from = cls.m_from;
        card_move.to = cls.m_to;
        if (card_move.from)
            card_move.from_player_name = card_move.from->objectName();
        if (card_move.to)
            card_move.to_player_name = card_move.to->objectName();
        card_move.from_place = cls.m_from_place;
        card_move.to_place = cls.m_to_place;
        card_move.from_pile_name = cls.m_from_pile_name;
        card_move.to_pile_name = cls.m_to_pile_name;
        card_move.open = cls.m_open;
        card_move.card_ids = ids.at(i);
        card_move.reason = cls.m_reason;

        card_moves.append(card_move);
        i++;
    }
    if (card_moves.size() > 1)
        qSort(card_moves.begin(), card_moves.end(), CompareByActionOrder);
    return card_moves;
}

void Room::moveCardsAtomic(CardsMoveStruct cards_move, bool forceMoveVisible) {
    QList<CardsMoveStruct> cards_moves;
    cards_moves.append(cards_move);
    moveCardsAtomic(cards_moves, forceMoveVisible);
}

void Room::moveCardsAtomic(QList<CardsMoveStruct> cards_moves, bool forceMoveVisible) {
    cards_moves = _breakDownCardMoves(cards_moves);

    QList<CardsMoveOneTimeStruct> moveOneTimes = _mergeMoves(cards_moves);
    int i = 0;
    foreach (CardsMoveOneTimeStruct moveOneTime, moveOneTimes) {
        if (moveOneTime.card_ids.size() == 0) continue;
        QVariant data = QVariant::fromValue(moveOneTime);
        foreach (ServerPlayer *player, getAllPlayers())
            thread->trigger(BeforeCardsMove, this, player, data);
        moveOneTime = data.value<CardsMoveOneTimeStruct>();
        moveOneTimes[i] = moveOneTime;
        i++;
    }
    cards_moves = _separateMoves(moveOneTimes);

    notifyMoveCards(true, cards_moves, forceMoveVisible);
    // First, process remove card
    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &cards_move = cards_moves[i];
        for (int j = 0; j < cards_move.card_ids.size(); j++) {
            int card_id = cards_move.card_ids[j];
            const Card *card = Sanguosha->getCard(card_id);

            if (cards_move.from) // Hand/Equip/Judge
                cards_move.from->removeCard(card, cards_move.from_place);

            switch (cards_move.from_place) {
            case Player::DiscardPile: m_discardPile->removeOne(card_id); break;
            case Player::DrawPile: m_drawPile->removeOne(card_id); break;
            case Player::PlaceSpecial: table_cards.removeOne(card_id); break;
            default:
                    break;
            }
        }
        if (cards_move.from_place == Player::DrawPile)
            doBroadcastNotify(S_COMMAND_UPDATE_PILE, Json::Value(m_drawPile->length()));
    }

    foreach (CardsMoveStruct move, cards_moves)
        updateCardsOnLose(move);

    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &cards_move = cards_moves[i];
        for (int j = 0; j < cards_move.card_ids.size(); j++) {
            setCardMapping(cards_move.card_ids[j], (ServerPlayer *)cards_move.to, cards_move.to_place);
        }
    }
    foreach (CardsMoveStruct move, cards_moves)
        updateCardsOnGet(move);
    notifyMoveCards(false, cards_moves, forceMoveVisible);

    // Now, process add cards
    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &cards_move = cards_moves[i];
        for (int j = 0; j < cards_move.card_ids.size(); j++) {
            int card_id = cards_move.card_ids[j];
            const Card *card = Sanguosha->getCard(card_id);
            if (forceMoveVisible && cards_move.to_place == Player::PlaceHand)
                card->setFlags("visible");
            else
                card->setFlags("-visible");
            if (cards_move.to) // Hand/Equip/Judge
                cards_move.to->addCard(card, cards_move.to_place);

            switch (cards_move.to_place) {
            case Player::DiscardPile: m_discardPile->prepend(card_id); break;
            case Player::DrawPile: m_drawPile->prepend(card_id); break;
            case Player::PlaceSpecial: table_cards.append(card_id); break;
            default:
                    break;
            }
        }
    }

    //trigger event
    moveOneTimes = _mergeMoves(cards_moves);
    foreach (CardsMoveOneTimeStruct moveOneTime, moveOneTimes) {
        if (moveOneTime.card_ids.size() == 0) continue;
        QVariant data = QVariant::fromValue(moveOneTime);
        foreach (ServerPlayer *player, getAllPlayers())
            thread->trigger(CardsMoveOneTime, this, player, data);
    }
}

QList<CardsMoveStruct> Room::_breakDownCardMoves(QList<CardsMoveStruct> &cards_moves) {
    QList<CardsMoveStruct> all_sub_moves;
    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &move = cards_moves[i];
        if (move.card_ids.size() == 0) continue;

        QMap<_MoveSourceClassifier, QList<int> > moveMap;
        // reassemble move sources
        for (int j = 0; j < move.card_ids.size(); j++) {
            _fillMoveInfo(move, j);
            _MoveSourceClassifier classifier(move);
            moveMap[classifier].append(move.card_ids[j]);
        }
        foreach (_MoveSourceClassifier cls, moveMap.keys()) {
            CardsMoveStruct sub_move = move;
            cls.copyTo(sub_move);
            if ((sub_move.from == sub_move.to && sub_move.from_place == sub_move.to_place)
                || sub_move.card_ids.size() == 0)
                continue;
            sub_move.card_ids = moveMap[cls];
            all_sub_moves.append(sub_move);
        }
    }
    return all_sub_moves;
}

void Room::moveCards(QList<CardsMoveStruct> cards_moves, bool forceMoveVisible, bool enforceOrigin) {
    QList<CardsMoveStruct> all_sub_moves = _breakDownCardMoves(cards_moves);
    _moveCards(all_sub_moves, forceMoveVisible, enforceOrigin);
}

void Room::_moveCards(QList<CardsMoveStruct> cards_moves, bool forceMoveVisible, bool enforceOrigin) {   
    // First, process remove card
    QList<CardsMoveStruct> origin = cards_moves;
    notifyMoveCards(true, cards_moves, forceMoveVisible);

    QList<CardsMoveOneTimeStruct> moveOneTimes = _mergeMoves(cards_moves);
    int i = 0;
    foreach (CardsMoveOneTimeStruct moveOneTime, moveOneTimes) {
        if (moveOneTime.card_ids.size() == 0) continue;
        Player *origin_to = moveOneTime.to;
        Player::Place origin_place = moveOneTime.to_place;
        moveOneTime.to = NULL;
        moveOneTime.to_place = Player::PlaceTable;
        QVariant data = QVariant::fromValue(moveOneTime);
        foreach (ServerPlayer *player, getAllPlayers())
            thread->trigger(BeforeCardsMove, this, player, data);
        moveOneTime = data.value<CardsMoveOneTimeStruct>();
        moveOneTime.to = origin_to;
        moveOneTime.to_place = origin_place;
        moveOneTimes[i] = moveOneTime;
        i++;
    }
    cards_moves = _separateMoves(moveOneTimes);

    QList<Player::Place> final_places;
    QList<Player *> move_tos;
    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &cards_move = cards_moves[i];
        final_places.append(cards_move.to_place);
        move_tos.append(cards_move.to);

        cards_move.to_place = Player::PlaceTable;
        cards_move.to = NULL;
    }

    for (int i = 0; i < cards_moves.size(); i++) {
		CardsMoveStruct &cards_move = cards_moves[i];
        for (int j = 0; j < cards_move.card_ids.size(); j++) {
            int card_id = cards_move.card_ids[j];
            const Card *card = Sanguosha->getCard(card_id);

            if (cards_move.from) // Hand/Equip/Judge
                cards_move.from->removeCard(card, cards_move.from_place);

            switch (cards_move.from_place) {
            case Player::DiscardPile: m_discardPile->removeOne(card_id); break;
            case Player::DrawPile: m_drawPile->removeOne(card_id); break;
            case Player::PlaceSpecial: table_cards.removeOne(card_id); break;
            default:
                    break;
            }

            setCardMapping(card_id, NULL, Player::PlaceTable);
        }
        if (cards_move.from_place == Player::DrawPile)
            doBroadcastNotify(S_COMMAND_UPDATE_PILE, Json::Value(m_drawPile->length()));
    }

    foreach (CardsMoveStruct move, cards_moves)
        updateCardsOnLose(move);

    //trigger event
    moveOneTimes = _mergeMoves(cards_moves);
    foreach (CardsMoveOneTimeStruct moveOneTime, moveOneTimes) {
        if (moveOneTime.card_ids.size() == 0) continue;
        QVariant data = QVariant::fromValue(moveOneTime);
        foreach (ServerPlayer *player, getAllPlayers())
            thread->trigger(CardsMoveOneTime, this, player, data);
    }

    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &cards_move = cards_moves[i];
        cards_move.to = move_tos[i];
        cards_move.to_place = final_places[i];
    }

    if (enforceOrigin) {
        for (int i = 0; i < cards_moves.size(); i++) {
            CardsMoveStruct &cards_move = cards_moves[i];
            if (cards_move.to && !cards_move.to->isAlive()) {
                cards_move.to = NULL;
                cards_move.to_place = Player::DiscardPile;
            }
        }
    }

    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &cards_move = cards_moves[i];
        cards_move.from_place = Player::PlaceTable;
    }

    moveOneTimes = _mergeMoves(cards_moves);
    i = 0;
    foreach (CardsMoveOneTimeStruct moveOneTime, moveOneTimes) {
        if (moveOneTime.card_ids.size() == 0) continue;
        QVariant data = QVariant::fromValue(moveOneTime);
        foreach (ServerPlayer *player, getAllPlayers())
            thread->trigger(BeforeCardsMove, this, player, data);
        moveOneTime = data.value<CardsMoveOneTimeStruct>();
        moveOneTimes[i] = moveOneTime;
        i++;
    }
    cards_moves = _separateMoves(moveOneTimes);

    // Now, process add cards
    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &cards_move = cards_moves[i];
        for (int j = 0; j < cards_move.card_ids.size(); j++) {
            setCardMapping(cards_move.card_ids[j], (ServerPlayer *)cards_move.to, cards_move.to_place);
        }
    }
    foreach (CardsMoveStruct move, cards_moves)
        updateCardsOnGet(move);

    QList<CardsMoveStruct> origin_x;
    foreach (CardsMoveStruct m, origin) {
        CardsMoveStruct m_x = m;
        m_x.card_ids.clear();
        m_x.to = NULL;
        m_x.to_place = Player::DiscardPile;
        foreach (int id, m.card_ids) {
            bool sure = false;
            foreach (CardsMoveStruct cards_move, cards_moves) {
                if (cards_move.card_ids.contains(id)) {
                    m_x.card_ids << id;
                    if (!sure) {
                        m_x.to = cards_move.to;
                        m_x.to_place = cards_move.to_place;
                        sure = true;
                    }
                }
            }
        }
        origin_x.append(m_x);
    }
    notifyMoveCards(false, origin_x, forceMoveVisible);

    for (int i = 0; i < cards_moves.size(); i++) {
        CardsMoveStruct &cards_move = cards_moves[i];
        for (int j = 0; j < cards_move.card_ids.size(); j++) {
            int card_id = cards_move.card_ids[j];
            const Card *card = Sanguosha->getCard(card_id);
            if (forceMoveVisible && cards_move.to_place == Player::PlaceHand)
                card->setFlags("visible");
            else
                card->setFlags("-visible");
            if (cards_move.to) // Hand/Equip/Judge
                cards_move.to->addCard(card, cards_move.to_place);

            switch (cards_move.to_place) {
            case Player::DiscardPile: m_discardPile->prepend(card_id); break;
            case Player::DrawPile: m_drawPile->prepend(card_id); break;
            case Player::PlaceSpecial: table_cards.append(card_id); break;
            default:
                    break;
            }
        }
    }

    //trigger event
    moveOneTimes = _mergeMoves(cards_moves);
    foreach (CardsMoveOneTimeStruct moveOneTime, moveOneTimes) {
        if (moveOneTime.card_ids.size() == 0) continue;
        QVariant data = QVariant::fromValue(moveOneTime);
        foreach (ServerPlayer *player, getAllPlayers())
            thread->trigger(CardsMoveOneTime, this, player, data);
    }
}

void Room::updateCardsOnLose(const CardsMoveStruct &move) {
    for (int i = 0; i < move.card_ids.size(); i++) {
        WrappedCard *card = qobject_cast<WrappedCard *>(getCard(move.card_ids[i]));
        if (card->isModified()) {
            if (move.to_place == Player::DiscardPile) {
                resetCard(move.card_ids[i]);
                broadcastResetCard(getPlayers(), move.card_ids[i]);
            }
        }
    }
}

void Room::updateCardsOnGet(const CardsMoveStruct &move) {
    if (move.card_ids.isEmpty()) return;
    ServerPlayer *player = (ServerPlayer *)move.from;
    if (player != NULL && move.to_place == Player::PlaceDelayedTrick) {
        for (int i = 0; i < move.card_ids.size(); i++) {
            WrappedCard *card = qobject_cast<WrappedCard *>(getCard(move.card_ids[i]));
            const Card *engine_card = Sanguosha->getEngineCard(move.card_ids[i]);
            if (card->getSuit() != engine_card->getSuit() || card->getNumber() != engine_card->getNumber()) {
                Card *trick = Sanguosha->cloneCard(card->getRealCard());
                trick->setSuit(engine_card->getSuit());
                trick->setNumber(engine_card->getNumber());
                card->takeOver(trick);
                broadcastUpdateCard(getPlayers(), move.card_ids[i], card);
            }
        }
        return;
    }

    player = (ServerPlayer *)move.to;
    if (player != NULL && (move.to_place == Player::PlaceHand
                           || move.to_place == Player::PlaceEquip
                           || move.to_place == Player::PlaceJudge
                           || move.to_place == Player::PlaceSpecial)) {
        QList<const Card *> cards;
        foreach (int cardId, move.card_ids)
            cards.append(getCard(cardId));
        filterCards(player, cards, true);
    }
}

bool Room::notifyMoveCards(bool isLostPhase, QList<CardsMoveStruct> cards_moves, bool forceVisible, QList<ServerPlayer *> players) {
    if (players.isEmpty()) players = m_players;
    // process dongcha    
    ServerPlayer *dongchaee = findChild<ServerPlayer *>(tag.value("Dongchaee").toString());    
    ServerPlayer *dongchaer = findChild<ServerPlayer *>(tag.value("Dongchaer").toString());   
    // Notify clients
    int moveId;
    if (isLostPhase)
        moveId = _m_lastMovementId++;
    else
        moveId = --_m_lastMovementId;
    Q_ASSERT(_m_lastMovementId >= 0);
    foreach (ServerPlayer *player, players) {
        if (player->isOffline()) continue;
        Json::Value arg(Json::arrayValue);
        arg[0] = moveId;
        for (int i = 0; i < cards_moves.size(); i++) {
            cards_moves[i].open = forceVisible || cards_moves[i].isRelevant(player)
                                  // forceVisible will override cards to be visible
                                  || cards_moves[i].to_place == Player::PlaceEquip
                                  || cards_moves[i].from_place == Player::PlaceEquip
                                  || cards_moves[i].to_place == Player::PlaceDelayedTrick
                                  || cards_moves[i].from_place == Player::PlaceDelayedTrick
                                  // only cards moved to hand/special can be invisible
                                  || cards_moves[i].from_place == Player::DiscardPile
                                  || cards_moves[i].to_place == Player::DiscardPile
                                  // any card from/to discard pile should be visible
                                  || cards_moves[i].from_place == Player::PlaceTable
                                  // any card from/to place table should be visible
                                  || player->hasFlag("Global_GongxinOperator")
                                  // the player put someone's cards to the drawpile
                                  || (player != NULL && player == dongchaer && (cards_moves[i].isRelevant(dongchaee)));
                                  // card from/to dongchaee is also visible to dongchaer
            arg[i + 1] = cards_moves[i].toJsonValue();
        }
        doNotify(player, isLostPhase ? S_COMMAND_LOSE_CARD : S_COMMAND_GET_CARD, arg);
    }
    return true;
}

void Room::notifySkillInvoked(ServerPlayer *player, const QString &skill_name) {
    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_SKILL_INVOKED;
    args[1] = toJsonString(player->objectName());
    args[2] = toJsonString(skill_name);
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void Room::broadcastSkillInvoke(const QString &skill_name, const QString &category) {
    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_PLAY_EFFECT;
    args[1] = toJsonString(skill_name);
    args[2] = toJsonString(category);
    args[3] = -1;
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void Room::broadcastSkillInvoke(const QString &skill_name) {
    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_PLAY_EFFECT;
    args[1] = toJsonString(skill_name);
    args[2] = true;
    args[3] = -1;
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void Room::broadcastSkillInvoke(const QString &skill_name, int type) {
    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_PLAY_EFFECT;
    args[1] = toJsonString(skill_name);
    args[2] = true;
    args[3] = type;
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void Room::broadcastSkillInvoke(const QString &skill_name, bool isMale, int type) {
    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_PLAY_EFFECT;
    args[1] = toJsonString(skill_name);
    args[2] = isMale;
    args[3] = type;
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void Room::doLightbox(const QString &lightboxName, int duration) {
    doAnimate(S_ANIMATE_LIGHTBOX, lightboxName, QString::number(duration));
    thread->delay(duration / 1.2);
}

void Room::doAnimate(QSanProtocol::AnimateType type, const QString &arg1, const QString &arg2,
                     QList<ServerPlayer *> players) {
    if (players.isEmpty())
        players = m_players;
    Json::Value arg(Json::arrayValue);
    arg[0] = (int)type;
    arg[1] = toJsonString(arg1);
    arg[2] = toJsonString(arg2);
    doBroadcastNotify(players, S_COMMAND_ANIMATE, arg);
}

void Room::preparePlayers() {
    foreach (ServerPlayer *player, m_players) {
        QList<const Skill *> skills = player->getGeneral()->getSkillList();
        foreach (const Skill *skill, skills)
            player->addSkill(skill->objectName());
        if (player->getGeneral2()) {
            skills = player->getGeneral2()->getSkillList();
            foreach (const Skill *skill, skills)
                player->addSkill(skill->objectName());
        }
        player->setGender(player->getGeneral()->getGender());
    }

    Json::Value args;
    args[0] = QSanProtocol::S_GAME_EVENT_UPDATE_SKILL;
    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
}

void Room::changePlayerGeneral(ServerPlayer *player, const QString &new_general) {
    if (player->getGeneral() != NULL) {
        foreach (const Skill *skill, player->getGeneral()->getSkillList())
            player->loseSkill(skill->objectName());
    }
    setPlayerProperty(player, "general", new_general);
    Q_ASSERT(player->getGeneral() != NULL);
    player->setGender(player->getGeneral()->getGender());
    foreach (const Skill *skill, player->getGeneral()->getSkillList())
        player->addSkill(skill->objectName());
    filterCards(player, player->getCards("he"), true);
}

void Room::changePlayerGeneral2(ServerPlayer *player, const QString &new_general) {
    if (player->getGeneral2() != NULL) {
        foreach (const Skill *skill, player->getGeneral2()->getSkillList())
            player->loseSkill(skill->objectName());
    }
    setPlayerProperty(player, "general2", new_general);
    Q_ASSERT(player->getGeneral2() != NULL);
    foreach (const Skill *skill, player->getGeneral2()->getSkillList())
        player->addSkill(skill->objectName());
    filterCards(player, player->getCards("he"), true);
}

void Room::filterCards(ServerPlayer *player, QList<const Card *> cards, bool refilter) {
    if (refilter) {
        for (int i = 0; i < cards.size(); i++) {
            WrappedCard *card = qobject_cast<WrappedCard *>(getCard(cards[i]->getId()));
            if (card->isModified()) {
                int cardId = card->getId();
                resetCard(cardId);
                if (getCardPlace(cardId) != Player::PlaceHand)
                    broadcastResetCard(m_players, cardId);
                else
                    notifyResetCard(player, cardId);
            }
        }
    }

    bool *cardChanged = new bool[cards.size()];
    for (int i = 0; i < cards.size(); i++)
        cardChanged[i] = false;

    QSet<const Skill *> skills = player->getSkills(false, false);
    QList<const FilterSkill *> filterSkills;

    foreach (const Skill *skill, skills) {
        if (player->hasSkill(skill->objectName()) && skill->inherits("FilterSkill")) {
            const FilterSkill *filter = qobject_cast<const FilterSkill *>(skill);
            Q_ASSERT(filter);
            filterSkills.append(filter);
        }
    }
    if (filterSkills.size() == 0) return;

    for (int i = 0; i < cards.size(); i++) {
        const Card *card = cards[i];
        for (int fTime = 0; fTime < filterSkills.size(); fTime++) {
            bool converged = true;
            foreach (const FilterSkill *skill, filterSkills) {
                Q_ASSERT(skill);
                if (skill->viewFilter(cards[i])) {
                    cards[i] = skill->viewAs(card);
                    Q_ASSERT(cards[i] != NULL);
                    converged = false;
                    cardChanged[i] = true;
                }
            }
            if (converged) break;
        }
    }

    for (int i = 0; i < cards.size(); i++) {
        int cardId = cards[i]->getId();
        Player::Place place = getCardPlace(cardId);
        if (!cardChanged[i]) continue;
        if (place == Player::PlaceHand)
            notifyUpdateCard(player, cardId, cards[i]);
        else {
            broadcastUpdateCard(getPlayers(), cardId, cards[i]);
            if (place == Player::PlaceJudge) {
                LogMessage log;
                log.type = "#FilterJudge";
                log.arg = cards[i]->getSkillName();
                log.from = player;

                sendLog(log);
                broadcastSkillInvoke(cards[i]->getSkillName());
            }
        }
    }

    delete cardChanged;
}

void Room::acquireSkill(ServerPlayer *player, const Skill *skill, bool open) {
    QString skill_name = skill->objectName();
    if (player->getAcquiredSkills().contains(skill_name))
        return;
    player->acquireSkill(skill_name);

    if (skill->inherits("TriggerSkill")) {
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        thread->addTriggerSkill(trigger_skill);
    }

    if (skill->isVisible()) {
        if (open) {
            Json::Value args;
            args[0] = QSanProtocol::S_GAME_EVENT_ACQUIRE_SKILL;
            args[1] = toJsonString(player->objectName());
            args[2] = toJsonString(skill_name);
            doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_EVENT, args);
        }

        foreach (const Skill *related_skill, Sanguosha->getRelatedSkills(skill_name)) {
            if (!related_skill->isVisible())
                acquireSkill(player, related_skill);
        }

        QVariant data = skill_name;
        thread->trigger(EventAcquireSkill, this, player, data);
    }
}

void Room::acquireSkill(ServerPlayer *player, const QString &skill_name, bool open) {
    const Skill *skill = Sanguosha->getSkill(skill_name);
    if (skill) acquireSkill(player, skill, open);
}

void Room::setTag(const QString &key, const QVariant &value) {
    tag.insert(key, value);
    if (scenario) scenario->onTagSet(this, key);
}

QVariant Room::getTag(const QString &key) const{
    return tag.value(key);
}

void Room::removeTag(const QString &key) {
    tag.remove(key);
}

void Room::setEmotion(ServerPlayer *target, const QString &emotion) {
    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(target->objectName());
    arg[1] = toJsonString(emotion.isEmpty() ? "." : emotion);
    doBroadcastNotify(S_COMMAND_SET_EMOTION, arg);
}

#include <QElapsedTimer>

void Room::activate(ServerPlayer *player, CardUseStruct &card_use) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_PLAY_CARD);

    _m_roomState.setCurrentCardUsePattern(QString());
    _m_roomState.setCurrentCardUseReason(CardUseStruct::CARD_USE_REASON_PLAY);

    AI *ai = player->getAI();
    if (ai) {
        QElapsedTimer timer;
        timer.start();

        card_use.from = player;
        ai->activate(card_use);

        qint64 diff = Config.AIDelay - timer.elapsed();
        if (diff > 0) thread->delay(diff);
    } else if (player->getPhase() != Player::Play) {
        return;
    } else {
        bool success = doRequest(player, S_COMMAND_PLAY_CARD, toJsonString(player->objectName()), true);
        Json::Value clientReply = player->getClientReply();

        if (m_surrenderRequestReceived) {
            makeSurrender(player);
            if (!game_finished)
                return activate(player, card_use);
        } else {
            if (Config.EnableCheat) {
                if (makeCheat(player)) {
                    if (player->isAlive())
                        return activate(player, card_use);
                    return;
                }
            }
        }

        if (!success || clientReply.isNull()) return;

        card_use.from = player;
        if (!card_use.tryParse(clientReply, this)) {
            emit room_message(tr("Card cannot be parsed:\n %1").arg(toQString(clientReply[0])));
            return;
        }
    }
    card_use.m_reason = CardUseStruct::CARD_USE_REASON_PLAY;
    if (!card_use.isValid(QString()))
        return;
    QVariant data = QVariant::fromValue(card_use);
    thread->trigger(ChoiceMade, this, player, data);
}

Card::Suit Room::askForSuit(ServerPlayer *player, const QString &reason) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_CHOOSE_SUIT);

    AI *ai = player->getAI();
    if (ai)
        return ai->askForSuit(reason);

    bool success = doRequest(player, S_COMMAND_CHOOSE_SUIT, Json::Value::null, true);

    Card::Suit suit = Card::AllSuits[qrand() % 4];
    if (success) {
        Json::Value clientReply = player->getClientReply();
        QString suitStr = toQString(clientReply);
        if (suitStr == "spade")
            suit = Card::Spade;
        else if (suitStr == "club")
            suit = Card::Club;
        else if (suitStr == "heart")
            suit = Card::Heart;
        else if (suitStr == "diamond")
            suit = Card::Diamond;
    }

    return suit;
}

QString Room::askForKingdom(ServerPlayer *player) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_CHOOSE_KINGDOM);

    AI *ai = player->getAI();
    if (ai)
        return ai->askForKingdom();

    bool success = doRequest(player, S_COMMAND_CHOOSE_KINGDOM, Json::Value::null, true);
    Json::Value clientReply = player->getClientReply();
    if (success && clientReply.isString()) {
        QString kingdom = toQString(clientReply.asCString());
        if (Sanguosha->getKingdoms().contains(kingdom))
            return kingdom;
    }
    return "wei";
}

bool Room::askForDiscard(ServerPlayer *player, const QString &reason, int discard_num, int min_num,
                         bool optional, bool include_equip, const QString &prompt) {
    if (!player->isAlive())
        return false;
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_DISCARD_CARD);

    if (!optional) {
        DummyCard *dummy = new DummyCard;
        dummy->deleteLater();
        QList<int> jilei_list;
        QList<const Card *> handcards = player->getHandcards();
        foreach (const Card *card, handcards) {
            if (!player->isJilei(card))
                dummy->addSubcard(card);
            else
                jilei_list << card->getId();
        }
        if (include_equip) {
            QList<const Card *> equips = player->getEquips();
            foreach (const Card *card, equips) {
                if (!player->isJilei(card))
                    dummy->addSubcard(card);
            }
        }

        int card_num = dummy->subcardsLength();
        if (card_num <= min_num) {
            if (card_num > 0) {
                CardMoveReason movereason;
                movereason.m_playerId = player->objectName();
                movereason.m_skillName = dummy->getSkillName();
                if (reason == "gamerule")
                    movereason.m_reason = CardMoveReason::S_REASON_RULEDISCARD;
                else
                    movereason.m_reason = CardMoveReason::S_REASON_THROW;

                throwCard(dummy, movereason, player);

                QVariant data;
                data = QString("%1:%2").arg("cardDiscard").arg(dummy->toString());
                thread->trigger(ChoiceMade, this, player, data);
            }

            if (card_num < min_num && !jilei_list.isEmpty()) {
                Json::Value gongxinArgs(Json::arrayValue);
                gongxinArgs[0] = toJsonString(player->objectName());
                gongxinArgs[1] = false;
                gongxinArgs[2] = toJsonArray(jilei_list);

                foreach (int cardId, jilei_list) {
                    WrappedCard *card = Sanguosha->getWrappedCard(cardId);
                    if (card->isModified())
                        broadcastUpdateCard(getOtherPlayers(player), cardId, card);
                    else
                        broadcastResetCard(getOtherPlayers(player), cardId);
                }

                LogMessage log;
                log.type = "$JileiShowAllCards";
                log.from = player;

                foreach (int card_id, jilei_list)
                    Sanguosha->getCard(card_id)->setFlags("visible");
                log.card_str = IntList2StringList(jilei_list).join("+");
                sendLog(log);

                doBroadcastNotify(S_COMMAND_SHOW_ALL_CARDS, gongxinArgs);
                return false;
            }
            return true;
        }
    }

    AI *ai = player->getAI();
    QList<int> to_discard;
    if (ai) {
        to_discard = ai->askForDiscard(reason, discard_num, min_num, optional, include_equip);
    } else {
        Json::Value ask_str(Json::arrayValue);
        ask_str[0] = discard_num;
        ask_str[1] = min_num;
        ask_str[2] = optional;
        ask_str[3] = include_equip;
        ask_str[4] = toJsonString(prompt);
        bool success = doRequest(player, S_COMMAND_DISCARD_CARD, ask_str, true);

        //@todo: also check if the player does have that card!!!
        Json::Value clientReply = player->getClientReply();
        if (!success || !clientReply.isArray() || ((int)clientReply.size() > discard_num || (int)clientReply.size() < min_num)
           || !tryParse(clientReply, to_discard)) {
            if (optional) return false;
            // time is up, and the server choose the cards to discard
            to_discard = player->forceToDiscard(discard_num, include_equip);
        }
    }

    if (to_discard.isEmpty()) return false;

    DummyCard *dummy_card = new DummyCard;
    foreach (int card_id, to_discard)
        dummy_card->addSubcard(card_id);
    if (reason == "gamerule") {
        CardMoveReason reason(CardMoveReason::S_REASON_RULEDISCARD, player->objectName(), QString(), dummy_card->getSkillName(), QString());
        throwCard(dummy_card, reason, player);
    } else {
        CardMoveReason reason(CardMoveReason::S_REASON_THROW, player->objectName(), QString(), dummy_card->getSkillName(), QString());
        throwCard(dummy_card, reason, player);
    }

    QVariant data;
    data = QString("%1:%2").arg("cardDiscard").arg(dummy_card->toString());
    thread->trigger(ChoiceMade, this, player, data);

    dummy_card->deleteLater();

    return true;
}

const Card *Room::askForExchange(ServerPlayer *player, const QString &reason, int discard_num, bool include_equip,
                                 const QString &prompt, bool optional) {
    if (!player->isAlive())
        return NULL;
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_EXCHANGE_CARD);

    AI *ai = player->getAI();
    QList<int> to_exchange;
    if (ai) {
        // share the same callback interface
        player->setFlags("Global_AIDiscardExchanging");
        try {
            to_exchange = ai->askForDiscard(reason, discard_num, discard_num, optional, include_equip);
            player->setFlags("-Global_AIDiscardExchanging");
        }
        catch (TriggerEvent triggerEvent) {
            if (triggerEvent == TurnBroken || triggerEvent == StageChange)
                player->setFlags("-Global_AIDiscardExchanging");
            throw triggerEvent;
        }
    } else {
        Json::Value exchange_str(Json::arrayValue);
        exchange_str[0] = discard_num;
        exchange_str[1] = include_equip;
        exchange_str[2] = toJsonString(prompt);
        exchange_str[3] = optional;

        bool success = doRequest(player, S_COMMAND_EXCHANGE_CARD, exchange_str, true);
        //@todo: also check if the player does have that card!!!
        Json::Value clientReply = player->getClientReply();
        if (!success || !clientReply.isArray() || (int)clientReply.size() != discard_num
            || !tryParse(clientReply, to_exchange)) {
            if (optional) return NULL;
            to_exchange = player->forceToDiscard(discard_num, include_equip, false);
        }
    }

    if (to_exchange.isEmpty()) return NULL;

    DummyCard *card = new DummyCard;
    foreach (int card_id, to_exchange)
        card->addSubcard(card_id);

    return card;
}

void Room::setCardMapping(int card_id, ServerPlayer *owner, Player::Place place) {
    owner_map.insert(card_id, owner);
    place_map.insert(card_id, place);
}

ServerPlayer *Room::getCardOwner(int card_id) const{
    return owner_map.value(card_id);
}

Player::Place Room::getCardPlace(int card_id) const{
    if (card_id < 0) return Player::PlaceUnknown;
    return place_map.value(card_id);
}

ServerPlayer *Room::getLord() const{
    ServerPlayer *the_lord = m_players.first();
    if (the_lord->getRole() == "lord")
        return the_lord;

    foreach (ServerPlayer *player, m_players) {
        if (player->getRole() == "lord")
            return player;
    }

    return NULL;
}

void Room::askForGuanxing(ServerPlayer *zhuge, const QList<int> &cards, bool up_only) {
    QList<int> top_cards, bottom_cards;
    while (isPaused()) {}
    notifyMoveFocus(zhuge, S_COMMAND_SKILL_GUANXING);

    AI *ai = zhuge->getAI();
    if (ai) {
        ai->askForGuanxing(cards, top_cards, bottom_cards, up_only);
    } else if (up_only && cards.length() == 1) {
        top_cards = cards;
    } else {
        Json::Value guanxingArgs(Json::arrayValue);
        guanxingArgs[0] = toJsonArray(cards);
        guanxingArgs[1] = up_only;
        bool success = doRequest(zhuge, S_COMMAND_SKILL_GUANXING, guanxingArgs, true);
        if (!success) {
            foreach (int card_id, cards)
                m_drawPile->prepend(card_id);
            return;
        }
        Json::Value clientReply = zhuge->getClientReply();
        if (clientReply.isArray() && clientReply.size() == 2) {
            success &= tryParse(clientReply[0], top_cards);
            success &= tryParse(clientReply[1], bottom_cards);
        }
    }

    bool length_equal = top_cards.length() + bottom_cards.length() == cards.length();
    bool result_equal = top_cards.toSet() + bottom_cards.toSet() == cards.toSet();
    if (!length_equal || !result_equal) {
        top_cards = cards;
        bottom_cards.clear();
    }

    if (!up_only) {
        LogMessage log;
        log.type = "#GuanxingResult";
        log.from = zhuge;
        log.arg = QString::number(top_cards.length());
        log.arg2 = QString::number(bottom_cards.length());
        sendLog(log);
    }

    if (!top_cards.isEmpty()) {
        LogMessage log;
        log.type = "$GuanxingTop";
        log.from = zhuge;
        log.card_str = IntList2StringList(top_cards).join("+");
        doNotify(zhuge, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
    }
    if (!bottom_cards.isEmpty()) {
        LogMessage log;
        log.type = "$GuanxingBottom";
        log.from = zhuge;
        log.card_str = IntList2StringList(bottom_cards).join("+");
        doNotify(zhuge, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
    }

    QListIterator<int> i(top_cards);
    i.toBack();
    while (i.hasPrevious())
        m_drawPile->prepend(i.previous());

    i = bottom_cards;
    while (i.hasNext())
        m_drawPile->append(i.next());

    doBroadcastNotify(S_COMMAND_UPDATE_PILE, Json::Value(m_drawPile->length()));
}

void Room::doGongxin(ServerPlayer *shenlvmeng, ServerPlayer *target) {
    Q_ASSERT(!target->isKongcheng());
    while (isPaused()) {}
    notifyMoveFocus(shenlvmeng, S_COMMAND_SKILL_GONGXIN);

    LogMessage log;
    log.type = "$ViewAllCards";
    log.from = shenlvmeng;
    log.to << target;
    log.card_str = IntList2StringList(target->handCards()).join("+");
    doNotify(shenlvmeng, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());

    QVariant decisionData = QVariant::fromValue("viewCards:" + shenlvmeng->objectName() + ":" + target->objectName());
    thread->trigger(ChoiceMade, this, shenlvmeng, decisionData);

    shenlvmeng->tag["GongxinTarget"] = QVariant::fromValue((PlayerStar)target);
    int card_id;
    AI *ai = shenlvmeng->getAI();
    if (ai) {
        QList<int> hearts;
        foreach (int id, target->handCards()) {
            if (Sanguosha->getCard(id)->getSuit() == Card::Heart)
                hearts << id;
        }
        if (hearts.isEmpty()) {
            shenlvmeng->tag.remove("GongxinTarget");
            return;
        }
        card_id = ai->askForAG(hearts, true, "gongxin");
        if (card_id == -1) {
            shenlvmeng->tag.remove("GongxinTarget");
            return;
        }
    } else {
        foreach (int cardId, target->handCards()) {
            WrappedCard *card = Sanguosha->getWrappedCard(cardId);
            if (card->isModified())
                notifyUpdateCard(shenlvmeng, cardId, card);
            else
                notifyResetCard(shenlvmeng, cardId);
        }

        Json::Value gongxinArgs(Json::arrayValue);
        gongxinArgs[0] = toJsonString(target->objectName());
        gongxinArgs[1] = true;
        gongxinArgs[2] = toJsonArray(target->handCards());
        bool success = doRequest(shenlvmeng, S_COMMAND_SKILL_GONGXIN, gongxinArgs, true);
        Json::Value clientReply = shenlvmeng->getClientReply();
        if (!success || !clientReply.isInt() || !target->handCards().contains(clientReply.asInt())) {
            shenlvmeng->tag.remove("GongxinTarget");
            return;
        }

        card_id = clientReply.asInt();
    }

    QString result = askForChoice(shenlvmeng, "gongxin", "discard+put");
    shenlvmeng->tag.remove("GongxinTarget");
    if (result == "discard") {
        CardMoveReason reason(CardMoveReason::S_REASON_THROW, shenlvmeng->objectName(), QString(), "gongxin", QString());
        throwCard(Sanguosha->getCard(card_id), reason, target, shenlvmeng);
    } else {
        shenlvmeng->setFlags("Global_GongxinOperator");
        CardMoveReason reason(CardMoveReason::S_REASON_PUT, shenlvmeng->objectName(), QString(), "gongxin", QString());
        moveCardTo(Sanguosha->getCard(card_id), target, NULL, Player::DrawPile, reason, true);
        shenlvmeng->setFlags("-Global_GongxinOperator");
    }
}

const Card *Room::askForPindian(ServerPlayer *player, ServerPlayer *from, ServerPlayer *to, const QString &reason) {
    if (!from->isAlive() || !to->isAlive())
        return NULL;
    Q_ASSERT(!player->isKongcheng());
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_PINDIAN);

    if (player->getHandcardNum() == 1)
        return player->getHandcards().first();

    AI *ai = player->getAI();
    if (ai) {
        thread->delay();
        return ai->askForPindian(from, reason);
    }

    bool success = doRequest(player, S_COMMAND_PINDIAN, toJsonArray(from->objectName(), to->objectName()), true);

    Json::Value clientReply = player->getClientReply();
    if (!success || !clientReply[0].isString()) {
        int card_id = player->getRandomHandCardId();
        return Sanguosha->getCard(card_id);
    } else {
        const Card *card = Card::Parse(toQString(clientReply[0]));
        if (card->isVirtualCard()) {
            const Card *real_card = Sanguosha->getCard(card->getEffectiveId());
            delete card;
            return real_card;
        } else
            return card;
    }
}

QList<const Card *> Room::askForPindianRace(ServerPlayer *from, ServerPlayer *to, const QString &reason) {
    if (!from->isAlive() || !to->isAlive())
        return QList<const Card *>() << NULL << NULL;
    Q_ASSERT(!from->isKongcheng() && !to->isKongcheng());
    while (isPaused()) {}
    Countdown countdown;
    countdown.m_max = ServerInfo.getCommandTimeout(S_COMMAND_PINDIAN, S_CLIENT_INSTANCE);
    countdown.m_type = Countdown::S_COUNTDOWN_USE_SPECIFIED;
    notifyMoveFocus(QList<ServerPlayer *>() << from << to, S_COMMAND_PINDIAN, countdown);

    const Card *from_card = NULL, *to_card = NULL;

    if (from->getHandcardNum() == 1)
        from_card = from->getHandcards().first();
    if (to->getHandcardNum() == 1)
        to_card = to->getHandcards().first();

    AI *ai;
    if (!from_card) {
        ai = from->getAI();
        if (ai)
            from_card = ai->askForPindian(from, reason);
    }
    if (!to_card) {
        ai = to->getAI();
        if (ai)
            to_card = ai->askForPindian(from, reason);
    }
    if (from_card && to_card) {
        thread->delay();
        return QList<const Card *>() << from_card << to_card;
    }

    QList<ServerPlayer *> players;
    if (!from_card) {
        from->m_commandArgs = toJsonArray(from->objectName(), to->objectName());
        players << from;
    }
    if (!to_card) {
        to->m_commandArgs = toJsonArray(from->objectName(), to->objectName());
        players << to;
    }

    doBroadcastRequest(players, S_COMMAND_PINDIAN);

    foreach (ServerPlayer *player, players) {
        const Card *c = NULL;
        Json::Value clientReply = player->getClientReply();
        if (!player->m_isClientResponseReady || !clientReply[0].isString()) {
            int card_id = player->getRandomHandCardId();
            c = Sanguosha->getCard(card_id);
        } else {
            const Card *card = Card::Parse(toQString(clientReply[0]));
            if (card->isVirtualCard()) {
                const Card *real_card = Sanguosha->getCard(card->getEffectiveId());
                delete card;
                c = real_card;
            } else
                c = card;
        }
        if (player == from)
            from_card = c;
        else
            to_card = c;
    }
    return QList<const Card *>() << from_card << to_card;
}

ServerPlayer *Room::askForPlayerChosen(ServerPlayer *player, const QList<ServerPlayer *> &targets, const QString &skillName,
                                       const QString &prompt, bool optional, bool notify_skill) {
    if (targets.isEmpty()) {
        Q_ASSERT(optional);
        return NULL;
    }
    else if (targets.length() == 1 && !optional) {
        QVariant data = QString("%1:%2:%3").arg("playerChosen").arg(skillName).arg(targets.first()->objectName());
        thread->trigger(ChoiceMade, this, player, data);
        return targets.first();
    }

    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_CHOOSE_PLAYER);
    AI *ai = player->getAI();
    ServerPlayer *choice = NULL;
    if (ai)
        choice = ai->askForPlayerChosen(targets, skillName);
    else {
        Json::Value req;
        req[0] = Json::Value(Json::arrayValue);
        req[1] = toJsonString(skillName);
        req[2] = toJsonString(prompt);
        req[3] = optional;
        foreach (ServerPlayer *target, targets)
            req[0].append(toJsonString(target->objectName()));
        bool success = doRequest(player, S_COMMAND_CHOOSE_PLAYER, req, true);

        Json::Value clientReply = player->getClientReply();
        if (success && clientReply.isString())
            choice = findChild<ServerPlayer *>(clientReply.asCString());
    }
    if (choice && !targets.contains(choice))
        choice = NULL;
    if (choice == NULL && !optional)
        choice = targets.at(qrand() % targets.length());
    if (choice) {
        if (notify_skill) {
            notifySkillInvoked(player, skillName);
            QVariant decisionData = QVariant::fromValue("skillInvoke:" + skillName + ":yes");
            thread->trigger(ChoiceMade, this, player, decisionData);

            doAnimate(S_ANIMATE_INDICATE, player->objectName(), choice->objectName());
            LogMessage log;
            log.type = "#ChoosePlayerWithSkill";
            log.from = player;
            log.to << choice;
            log.arg = skillName;
            sendLog(log);
        }
        QVariant data = QString("%1:%2:%3").arg("playerChosen").arg(skillName).arg(choice->objectName());
        thread->trigger(ChoiceMade, this, player, data);
    }
    return choice;
}

void Room::_setupChooseGeneralRequestArgs(ServerPlayer *player) {
    Json::Value options = toJsonArray(player->getSelected());
    if (!Config.EnableBasara)
        options.append(toJsonString(QString("%1(lord)").arg(getLord()->getGeneralName())));
    else
        options.append("anjiang(lord)");
    player->m_commandArgs = options;
}

QString Room::askForGeneral(ServerPlayer *player, const QStringList &generals, QString default_choice) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_CHOOSE_GENERAL);

    if (generals.length() == 1)
        return generals.first();

    if (default_choice.isEmpty())
        default_choice = generals.at(qrand() % generals.length());

    if (player->isOnline()) {
        Json::Value options = toJsonArray(generals);
        bool success = doRequest(player, S_COMMAND_CHOOSE_GENERAL, options, true);

        Json::Value clientResponse = player->getClientReply();
        bool free = Config.FreeChoose || mode.startsWith("_mini_") || mode == "custom_scenario";
        if (!success || !clientResponse.isString() || (!free && !generals.contains(clientResponse.asCString())))
            return default_choice;
        else
            return toQString(clientResponse);
    }

    return default_choice;
}

QString Room::askForGeneral(ServerPlayer *player, const QString &generals, QString default_choice) {
    return askForGeneral(player, generals.split("+"), default_choice); // For Lua only!!!
}

bool Room::makeCheat(ServerPlayer *player) {
    Json::Value arg = player->m_cheatArgs;
    player->m_cheatArgs = Json::Value::null;
    if (!arg.isArray() || !arg[0].isInt()) return false;
    CheatCode code = (CheatCode)arg[0].asInt();
    if (code == S_CHEAT_KILL_PLAYER) {
        if (!isStringArray(arg[1], 0, 1)) return false;
        makeKilling(toQString(arg[1][0]), toQString(arg[1][1]));
    } else if (code == S_CHEAT_MAKE_DAMAGE) {
        if (arg[1].size() != 4 || !isStringArray(arg[1], 0, 1)
            || !arg[1][2].isInt() || !arg[1][3].isInt())
            return false;
        makeDamage(toQString(arg[1][0]), toQString(arg[1][1]),
                   (QSanProtocol::CheatCategory)arg[1][2].asInt(), arg[1][3].asInt());
    } else if (code == S_CHEAT_REVIVE_PLAYER) {
        if (!arg[1].isString()) return false;
        makeReviving(toQString(arg[1]));
    } else if (code == S_CHEAT_RUN_SCRIPT) {
        if (!arg[1].isString()) return false;
        QByteArray data = QByteArray::fromBase64(arg[1].asCString());
        data = qUncompress(data);
        doScript(data);
    } else if (code == S_CHEAT_GET_ONE_CARD) {
        if (!arg[1].isInt()) return false;
        int card_id = arg[1].asInt();

        LogMessage log;
        log.type = "$CheatCard";
        log.from = player;
        log.card_str = QString::number(card_id);
        sendLog(log);

        obtainCard(player, card_id);
    } else if (code == S_CHEAT_CHANGE_GENERAL) {
        if (!arg[1].isString() || !arg[2].isBool()) return false;
        QString generalName = toQString(arg[1]);
        bool isSecondaryHero = arg[2].asBool();
        changeHero(player, generalName, false, true, isSecondaryHero);
    }
    return true;
}

void Room::makeDamage(const QString &source, const QString &target, QSanProtocol::CheatCategory nature, int point) {
    ServerPlayer *sourcePlayer = findChild<ServerPlayer *>(source);
    ServerPlayer *targetPlayer = findChild<ServerPlayer *>(target);
    if (targetPlayer == NULL) return;
    // damage
    if (nature == S_CHEAT_HP_LOSE) {
        loseHp(targetPlayer, point);
        return;
    } else if (nature == S_CHEAT_MAX_HP_LOSE) {
        loseMaxHp(targetPlayer, point);
        return;
    } else if (nature == S_CHEAT_HP_RECOVER) {
        RecoverStruct recover;
        recover.who = sourcePlayer;
        recover.recover = point;
        this->recover(targetPlayer, recover);
        return;
    } else if (nature == S_CHEAT_MAX_HP_RESET) {
        setPlayerProperty(targetPlayer, "maxhp", point);
        return;
    }

    static QMap<QSanProtocol::CheatCategory, DamageStruct::Nature> nature_map;
    if (nature_map.isEmpty()) {
        nature_map[S_CHEAT_NORMAL_DAMAGE] = DamageStruct::Normal;
        nature_map[S_CHEAT_THUNDER_DAMAGE] = DamageStruct::Thunder;
        nature_map[S_CHEAT_FIRE_DAMAGE] = DamageStruct::Fire;
    }

    if (targetPlayer == NULL) return;
    this->damage(DamageStruct("cheat", sourcePlayer, targetPlayer, point, nature_map[nature]));
}

void Room::makeKilling(const QString &killerName, const QString &victimName) {
    ServerPlayer *killer = NULL, *victim = NULL;

    killer = findChild<ServerPlayer *>(killerName);
    victim = findChild<ServerPlayer *>(victimName);

    if (victim == NULL) return;
    if (killer == NULL) return killPlayer(victim);

    DamageStruct damage("cheat", killer, victim);
    killPlayer(victim, &damage);
}

void Room::makeReviving(const QString &name) {
    ServerPlayer *player = findChild<ServerPlayer *>(name);
    Q_ASSERT(player);
    revivePlayer(player);
    setPlayerProperty(player, "maxhp", player->getGeneralMaxHp());
    setPlayerProperty(player, "hp", player->getMaxHp());
}

void Room::fillAG(const QList<int> &card_ids, ServerPlayer *who, const QList<int> &disabled_ids) {
    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonArray(card_ids);
    arg[1] = toJsonArray(disabled_ids);

    if (who)
        doNotify(who, S_COMMAND_FILL_AMAZING_GRACE, arg);
    else
        doBroadcastNotify(S_COMMAND_FILL_AMAZING_GRACE, arg);
}

void Room::takeAG(ServerPlayer *player, int card_id, bool move_cards) {
    Json::Value arg(Json::arrayValue);
    arg[0] = player ? toJsonString(player->objectName()) : Json::Value::null;
    arg[1] = card_id;
    arg[2] = move_cards;

    if (player) {
        if (move_cards) {
            CardsMoveOneTimeStruct move;
            move.from = NULL;
            move.from_places << Player::DrawPile;
            move.to = player;
            move.to_place = Player::PlaceHand;
            move.card_ids << card_id;
            QVariant data = QVariant::fromValue(move);
            foreach (ServerPlayer *p, getAllPlayers())
                thread->trigger(BeforeCardsMove, this, p, data);
            move = data.value<CardsMoveOneTimeStruct>();

            if (move.card_ids.length() > 0) {
                player->addCard(Sanguosha->getCard(card_id), Player::PlaceHand);
                setCardMapping(card_id, player, Player::PlaceHand);
                Sanguosha->getCard(card_id)->setFlags("visible");
                QList<const Card *> cards;
                cards << Sanguosha->getCard(card_id);
                filterCards(player, cards, false);

                data = QVariant::fromValue(move);
                foreach (ServerPlayer *p, getAllPlayers())
                    thread->trigger(CardsMoveOneTime, this, p, data);
            } else {
                arg[2] = false;
            }
        }
        doBroadcastNotify(S_COMMAND_TAKE_AMAZING_GRACE, arg);
    } else {
        doBroadcastNotify(S_COMMAND_TAKE_AMAZING_GRACE, arg);
        if (!move_cards) return;
        LogMessage log;
        log.type = "$EnterDiscardPile";
        log.card_str = QString::number(card_id);
        sendLog(log);

        m_discardPile->prepend(card_id);
        setCardMapping(card_id, NULL, Player::DiscardPile);
    }
}

void Room::clearAG(ServerPlayer *player) {
    if (player)
        doNotify(player, S_COMMAND_CLEAR_AMAZING_GRACE, Json::Value::null);
    else
        doBroadcastNotify(S_COMMAND_CLEAR_AMAZING_GRACE, Json::Value::null);
}

void Room::provide(const Card *card) {
    Q_ASSERT(provided == NULL);
    Q_ASSERT(!has_provided);

    provided = card;
    has_provided = true;
}

QList<ServerPlayer *> Room::getLieges(const QString &kingdom, ServerPlayer *lord) const{
    QList<ServerPlayer *> lieges;
    foreach (ServerPlayer *player, getAllPlayers()) {
        if (player != lord && player->getKingdom() == kingdom)
            lieges << player;
    }

    return lieges;
}

void Room::sendLog(const LogMessage &log) {
    if (log.type.isEmpty())
        return;

    doBroadcastNotify(QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
}

void Room::showCard(ServerPlayer *player, int card_id, ServerPlayer *only_viewer) {
    if (getCardOwner(card_id) != player) return;

    while (isPaused()) {}
    notifyMoveFocus(player);
    Json::Value show_arg(Json::arrayValue);
    show_arg[0] = toJsonString(player->objectName());
    show_arg[1] = card_id;

    WrappedCard *card = Sanguosha->getWrappedCard(card_id);
    bool modified = card->isModified();
    if (only_viewer) {
        QList<ServerPlayer *>players;
        players << only_viewer << player;
        if (modified)
            notifyUpdateCard(only_viewer, card_id, card);
        else
            notifyResetCard(only_viewer, card_id);
        doBroadcastNotify(players, S_COMMAND_SHOW_CARD, show_arg);
    } else {
        if (card_id > 0)
            Sanguosha->getCard(card_id)->setFlags("visible");
        if (modified)
            broadcastUpdateCard(getOtherPlayers(player), card_id, card);
        else
            broadcastResetCard(getOtherPlayers(player), card_id);
        doBroadcastNotify(S_COMMAND_SHOW_CARD, show_arg);
    }
}

void Room::showAllCards(ServerPlayer *player, ServerPlayer *to) {
    if (player->isKongcheng())
        return;
    while (isPaused()) {}

    Json::Value gongxinArgs(Json::arrayValue);
    gongxinArgs[0] = toJsonString(player->objectName());
    gongxinArgs[1] = false;
    gongxinArgs[2] = toJsonArray(player->handCards());

    bool isUnicast = (to != NULL);

    foreach (int cardId, player->handCards()) {
        WrappedCard *card = Sanguosha->getWrappedCard(cardId);
        if (card->isModified()) {
            if (isUnicast)
                notifyUpdateCard(to, cardId, card);
            else
                broadcastUpdateCard(getOtherPlayers(player), cardId, card);
        } else {
            if (isUnicast)
                notifyResetCard(to, cardId);
            else
                broadcastResetCard(getOtherPlayers(player), cardId);
        }
    }

    if (isUnicast) {
        LogMessage log;
        log.type = "$ViewAllCards";
        log.from = to;
        log.to << player;
        log.card_str = IntList2StringList(player->handCards()).join("+");
        doNotify(to, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());

        QVariant decisionData = QVariant::fromValue("viewCards:" + to->objectName() + ":" + player->objectName());
        thread->trigger(ChoiceMade, this, to, decisionData);

        notifyMoveFocus(to, S_COMMAND_SKILL_GONGXIN);
        doRequest(to, S_COMMAND_SKILL_GONGXIN, gongxinArgs, true);
    } else {
        LogMessage log;
        log.type = "$ShowAllCards";
        log.from = player;
        foreach (int card_id, player->handCards())
            Sanguosha->getCard(card_id)->setFlags("visible");
        log.card_str = IntList2StringList(player->handCards()).join("+");
        sendLog(log);

        doBroadcastNotify(S_COMMAND_SHOW_ALL_CARDS, gongxinArgs);
    }
}

void Room::retrial(const Card *card, ServerPlayer *player, JudgeStar judge, const QString &skill_name, bool exchange) {
    if (card == NULL) return;

    bool triggerResponded = getCardOwner(card->getEffectiveId()) == player;

    const Card *oldJudge = judge->card;
    judge->card = Sanguosha->getCard(card->getEffectiveId());

    CardsMoveStruct move1(QList<int>(),
                          judge->who,
                          Player::PlaceJudge,
                          CardMoveReason(CardMoveReason::S_REASON_RETRIAL,
                          player->objectName(),
                          skill_name,
                          QString()));

    move1.card_ids.append(card->getEffectiveId());
    int reasonType = exchange ? CardMoveReason::S_REASON_OVERRIDE : CardMoveReason::S_REASON_JUDGEDONE;
    CardMoveReason reason(reasonType,
                          player->objectName(),
                          exchange ? skill_name : QString(),
                          QString());
    CardsMoveStruct move2(QList<int>(),
                          judge->who,
                          exchange ? player : NULL,
                          exchange ? Player::PlaceHand : Player::DiscardPile,
                          reason);

    move2.card_ids.append(oldJudge->getEffectiveId());

    LogMessage log;
    log.type = "$ChangedJudge";
    log.arg = skill_name;
    log.from = player;
    log.to << judge->who;
    log.card_str = QString::number(card->getEffectiveId());
    sendLog(log);

    QList<CardsMoveStruct> moves;
    moves.append(move1);
    moves.append(move2);
    moveCardsAtomic(moves, true);
    judge->updateResult();

    if (triggerResponded) {
        CardResponseStruct resp(card, judge->who);
        QVariant data = QVariant::fromValue(resp);
        thread->trigger(CardResponded, this, player, data);
    }
}

bool Room::askForYiji(ServerPlayer *guojia, QList<int> &cards, const QString &skill_name,
                      bool is_preview, bool visible, int optional, int max_num,
                      QList<ServerPlayer *> players, CardMoveReason reason) {
    if (max_num == -1)
        max_num = cards.length();
    if (players.isEmpty())
        players = getOtherPlayers(guojia);
    if (cards.isEmpty() || max_num == 0)
        return false;
    if (reason.m_reason == CardMoveReason::S_REASON_UNKNOWN) {
        reason.m_playerId = guojia->objectName();
        // when we use ? : here, compiling error occurs under debug mode...
        if (is_preview)
            reason.m_reason = CardMoveReason::S_REASON_PREVIEWGIVE;
        else
            reason.m_reason = CardMoveReason::S_REASON_GIVE;
    }
    while (isPaused()) {}
    notifyMoveFocus(guojia, S_COMMAND_SKILL_YIJI);

    ServerPlayer *target = NULL;

    QList<int> ids;
    AI *ai = guojia->getAI();
    if (ai) {
        int card_id;
        ServerPlayer *who = ai->askForYiji(cards, skill_name, card_id);
        if (!who)
            return false;
        else {
            target = who;
            ids << card_id;
        }
    } else {
        Json::Value arg(Json::arrayValue);
        arg[0] = toJsonArray(cards);
        arg[1] = optional;
        arg[2] = max_num;
        QStringList player_names;
        foreach (ServerPlayer *player, players)
            player_names << player->objectName();
        arg[3] = toJsonArray(player_names);
        bool success = doRequest(guojia, S_COMMAND_SKILL_YIJI, arg, true);

        //Validate client response
        Json::Value clientReply = guojia->getClientReply();
        if (!success || !clientReply.isArray() || clientReply.size() != 2)
            return false;

        if (!tryParse(clientReply[0], ids) || !clientReply[1].isString())
            return false;

        foreach (int id, ids)
            if (!cards.contains(id)) return false;

        ServerPlayer *who = findChild<ServerPlayer *>(toQString(clientReply[1]));
        if (!who)
            return false;
        else
            target = who;
    }
    Q_ASSERT(target != NULL);

    DummyCard *dummy_card = new DummyCard;
    foreach (int card_id, ids) {
        cards.removeOne(card_id);
        dummy_card->addSubcard(card_id);
    }

    QVariant decisionData = QVariant::fromValue(QString("Yiji:%1:%2:%3:%4")
                                                .arg(skill_name).arg(guojia->objectName()).arg(target->objectName())
                                                .arg(IntList2StringList(ids).join("+")));
    thread->trigger(ChoiceMade, this, guojia, decisionData);

    guojia->setFlags("Global_GongxinOperator");
    moveCardTo(dummy_card, target, Player::PlaceHand, reason, visible);
    guojia->setFlags("-Global_GongxinOperator");
    delete dummy_card;

    return true;
}

QString Room::generatePlayerName() {
    static unsigned int id = 0;
    id++;
    return QString("sgs%1").arg(id);
}

QString Room::askForOrder(ServerPlayer *player) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_CHOOSE_ORDER);

    bool success = doRequest(player, S_COMMAND_CHOOSE_ORDER, (int)S_REASON_CHOOSE_ORDER_TURN, true);

    Game3v3Camp result = qrand() % 2 == 0 ? S_CAMP_WARM : S_CAMP_COOL;
    Json::Value clientReply = player->getClientReply();
    if (success && clientReply.isInt())
        result = (Game3v3Camp)clientReply.asInt();
    return (result == S_CAMP_WARM) ? "warm" : "cool";
}

QString Room::askForRole(ServerPlayer *player, const QStringList &roles, const QString &scheme) {
    while (isPaused()) {}
    notifyMoveFocus(player, S_COMMAND_CHOOSE_ROLE_3V3);

    QStringList squeezed = roles.toSet().toList();
    Json::Value arg(Json::arrayValue);
    arg[0] = toJsonString(scheme);
    arg[1] = toJsonArray(squeezed);
    bool success = doRequest(player, S_COMMAND_CHOOSE_ROLE_3V3, arg, true);
    Json::Value clientReply = player->getClientReply();
    QString result = "abstain";
    if (success && clientReply.isString())
        result = clientReply.asCString();
    return result;
}

void Room::networkDelayTestCommand(ServerPlayer *player, const QString &) {
    qint64 delay = player->endNetworkDelayTest();
    QString reportStr = tr("<font color=#EEB422>The network delay of player <b>%1</b> is %2 milliseconds.</font>")
                           .arg(player->screenName()).arg(QString::number(delay));
    speakCommand(player, reportStr.toUtf8().toBase64());
}

void Room::sortByActionOrder(QList<ServerPlayer *> &players) {
    if (players.length() > 1)
        qSort(players.begin(), players.end(), ServerPlayer::CompareByActionOrder);
}
