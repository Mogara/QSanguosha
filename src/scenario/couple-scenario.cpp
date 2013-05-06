#include "couple-scenario.h"
#include "skill.h"
#include "engine.h"
#include "room.h"

class CoupleScenarioRule: public ScenarioRule {
public:
    CoupleScenarioRule(Scenario *scenario)
        : ScenarioRule(scenario)
    {
        events << GameStart << GameOverJudge << BuryVictim;
    }

    virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
        const CoupleScenario *scenario = qobject_cast<const CoupleScenario *>(parent());

        switch (triggerEvent) {
        case GameStart: {
                if (player != NULL) return false;
                foreach (ServerPlayer *player, room->getPlayers()) {
                    if (player->isLord()) {
                        continue;
                    } else {
                        QMap<QString, QString> OH_map = scenario->getOriginalMap(true);
                        QMap<QString, QString> OW_map = scenario->getOriginalMap(false);
                        QMap<QString, QStringList> H_map = scenario->getMap(true);
                        QMap<QString, QStringList> W_map = scenario->getMap(false);
                        if (OH_map.contains(player->getGeneralName())) {
                            QStringList h_list = W_map.value(OH_map.value(player->getGeneralName()));
                            if (h_list.length() > 1) {
                                if (player->askForSkillInvoke("reselect")) {
                                    h_list.removeOne(player->getGeneralName());
                                    QString general_name = room->askForGeneral(player, h_list);
                                    room->changeHero(player, general_name, true, false);
                                }
                            }
                        } else if (OW_map.contains(player->getGeneralName())) {
                            QStringList w_list = H_map.value(OW_map.value(player->getGeneralName()));
                            if (w_list.length() > 1) {
                                if (player->askForSkillInvoke("reselect")) {
                                    w_list.removeOne(player->getGeneralName());
                                    QString general_name = room->askForGeneral(player, w_list);
                                    room->changeHero(player, general_name, true, false);
                                }
                            }
                        }
                    }
                }
                scenario->marryAll(room);
                room->setTag("SkipNormalDeathProcess", true);
                break;
            }
        case GameOverJudge: {
                if (player->isLord()) {
                    scenario->marryAll(room);
                } else if (player->isMale()) {
                    ServerPlayer *loyalist = NULL;
                    foreach (ServerPlayer *player, room->getAlivePlayers()) {
                        if (player->getRoleEnum() == Player::Loyalist) {
                            loyalist = player;
                            break;
                        }
                    }
                    ServerPlayer *widow = scenario->getSpouse(player);
                    if (widow && widow->isAlive() && widow->isFemale() && room->getLord()->isAlive() && loyalist == NULL)
                        scenario->remarry(room->getLord(), widow);
                } else if (player->getRoleEnum() == Player::Loyalist) {
                    room->setPlayerProperty(player, "role", "renegade");

                    QList<ServerPlayer *> players = room->getAllPlayers();
                    QList<ServerPlayer *> widows;
                    foreach (ServerPlayer *player, players) {
                        if (scenario->isWidow(player))
                            widows << player;
                    }

                    ServerPlayer *new_wife = room->askForPlayerChosen(room->getLord(), widows, "remarry");
                    if (new_wife) {
                        scenario->remarry(room->getLord(), new_wife);
                    }
                }

                QList<ServerPlayer *> players = room->getAlivePlayers();
                if (players.length() == 1) {
                    ServerPlayer *survivor = players.first();
                    ServerPlayer *spouse = scenario->getSpouse(survivor);
                    if (spouse)
                        room->gameOver(QString("%1+%2").arg(survivor->objectName()).arg(spouse->objectName()));
                    else
                        room->gameOver(survivor->objectName());

                    return true;
                } else if (players.length() == 2) {
                    ServerPlayer *first = players.at(0);
                    ServerPlayer *second = players.at(1);
                    if (scenario->getSpouse(first) == second) {
                        room->gameOver(QString("%1+%2").arg(first->objectName()).arg(second->objectName()));
                        return true;
                    }
                }

                return true;
            }

        case BuryVictim: {
                DeathStruct death = data.value<DeathStruct>();
                player->bury();
                // reward and punishment
                if (death.damage && death.damage->from) {
                    ServerPlayer *killer = death.damage->from;
                    if (killer == player)
                        return false;

                    if (scenario->getSpouse(killer) == player)
                        killer->throwAllHandCardsAndEquips();
                    else
                        killer->drawCards(3);
                }

                break;
            }
        default:
            break;
        }

        return false;
    }
};

CoupleScenario::CoupleScenario()
    : Scenario("couple")
{
    lua_State *lua = Sanguosha->getLuaState();
    lord = GetConfigFromLuaState(lua, "couple_lord").toString();
    loadCoupleMap();

    rule = new CoupleScenarioRule(this);
}

void CoupleScenario::loadCoupleMap() {
    QStringList couple_list = GetConfigFromLuaState(Sanguosha->getLuaState(), "couple_couples").toStringList();
    foreach (QString couple, couple_list) {
        QStringList husbands = couple.split("+").first().split("|");
        QStringList wifes = couple.split("+").last().split("|");
        foreach (QString husband, husbands)
            husband_map[husband] = wifes;
        original_husband_map[husbands.first()] = wifes.first();
        foreach (QString wife, wifes)
            wife_map[wife] = husbands;
        original_wife_map[wifes.first()] = husbands.first();
    }
}

void CoupleScenario::marryAll(Room *room) const{
    foreach (QString husband_name, husband_map.keys()) {
        ServerPlayer *husband = room->findPlayer(husband_name, true);
        if (husband == NULL)
            continue;

        QStringList wife_names = husband_map.value(husband_name, QStringList());
        if (!wife_names.isEmpty()) {
            foreach (QString wife_name, wife_names) {
                ServerPlayer *wife = room->findPlayer(wife_name, true);
                if (wife != NULL) {
                    marry(husband, wife);
                    break;
                }
            }
        }
    }
}

void CoupleScenario::setSpouse(ServerPlayer *player, ServerPlayer *spouse) const{
    if (spouse)
        player->tag["spouse"] = QVariant::fromValue(spouse);
    else
        player->tag.remove("spouse");
}

void CoupleScenario::marry(ServerPlayer *husband, ServerPlayer *wife) const{
    if (getSpouse(husband) == wife)
        return;

    LogMessage log;
    log.type = "#Marry";
    log.from = husband;
    log.to << wife;
    husband->getRoom()->sendLog(log);

    setSpouse(husband, wife);
    setSpouse(wife, husband);
}

void CoupleScenario::remarry(ServerPlayer *enkemann, ServerPlayer *widow) const{
    Room *room = enkemann->getRoom();

    ServerPlayer *ex_husband = getSpouse(widow);
    setSpouse(ex_husband, NULL);
    LogMessage log;
    log.type = "#Divorse";
    log.from = widow;
    log.to << ex_husband;
    room->sendLog(log);

    ServerPlayer *ex_wife = getSpouse(enkemann);
    if (ex_wife) {
        setSpouse(ex_wife, NULL);
        LogMessage log;
        log.type = "#Divorse";
        log.from = enkemann;
        log.to << ex_wife;
        room->sendLog(log);
    }

    marry(enkemann, widow);
    room->setPlayerProperty(widow, "role", "loyalist");
    room->resetAI(widow);
}

ServerPlayer *CoupleScenario::getSpouse(const ServerPlayer *player) const{
    return player->tag["spouse"].value<PlayerStar>();
}

bool CoupleScenario::isWidow(ServerPlayer *player) const{
    if (player->isMale())
        return false;

    ServerPlayer *spouse = getSpouse(player);
    return spouse && spouse->isDead();
}

void CoupleScenario::assign(QStringList &generals, QStringList &roles) const{
    generals << lord;

    QStringList husbands = original_husband_map.keys();
    qShuffle(husbands);
    husbands = husbands.mid(0, 4);

    QStringList others;
    foreach (QString husband, husbands)
        others << husband << original_husband_map.value(husband);

    generals << others;
    qShuffle(generals);

    // roles
    for (int i = 0; i < 9; i++) {
        if (generals.at(i) == lord)
            roles << "lord";
        else
            roles << "renegade";
    }
}

int CoupleScenario::getPlayerCount() const{
    return 9;
}

QString CoupleScenario::getRoles() const{
    return "ZNNNNNNNN";
}

void CoupleScenario::onTagSet(Room *room, const QString &key) const{
}

AI::Relation CoupleScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    if (getSpouse(a) == b)
        return AI::Friend;

    if ((a->isLord() || b->isLord()) && a->isMale() != b->isMale())
        return AI::Neutrality;

    return AI::Enemy;
}

QMap<QString, QStringList> CoupleScenario::getMap(bool isHusband) const{
    return isHusband ? husband_map : wife_map;
}

QMap<QString, QString> CoupleScenario::getOriginalMap(bool isHusband) const{
    return isHusband ? original_husband_map : original_wife_map;
}
