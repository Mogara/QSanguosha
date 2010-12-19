#include "couple-scenario.h"
#include "skill.h"
#include "engine.h"

class CoupleScenarioRule: public ScenarioRule{
public:
    CoupleScenarioRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();
        const CoupleScenario *scenario = qobject_cast<const CoupleScenario *>(parent());

        switch(event){
        case GameStart:{
                if(player->isLord()){
                    scenario->marryAll(room);
                }else if(player->getGeneralName() == "lubu"){
                    if(player->askForSkillInvoke("reselect"))
                        room->transfigure(player, "dongzhuo", true);
                }else if(player->getGeneralName() == "zhugeliang"){
                    if(player->askForSkillInvoke("reselect"))
                        room->transfigure(player, "wolong", true);
                }

                break;
            }

        case Death:{
                player->throwAllCards();

                if(player->isLord()){
                    scenario->marryAll(room);
                }else if(player->getGeneral()->isMale()){
                    ServerPlayer *widow = scenario->getSpouse(player);
                    if(widow && widow->isAlive() && widow->getGeneral()->isFemale() && room->getLord()->isAlive())
                        scenario->remarry(room->getLord(), widow);
                }else{
                    if(player->getRoleEnum() == Player::Loyalist){
                        QList<ServerPlayer *> players = room->getAllPlayers();
                        QList<ServerPlayer *> widows;
                        foreach(ServerPlayer *player, players){
                            if(scenario->isWidow(player))
                                widows << player;
                        }

                        ServerPlayer *new_wife = room->askForPlayerChosen(room->getLord(), widows);
                        if(new_wife){
                            scenario->remarry(room->getLord(), new_wife);
                        }
                    }
                }

                QList<ServerPlayer *> players = room->getAllPlayers();
                if(players.length() == 1){
                    ServerPlayer *survivor = players.first();
                    ServerPlayer *spouse = scenario->getSpouse(survivor);
                    if(spouse)
                        room->gameOver(QString("%1+%2").arg(survivor->objectName()).arg(spouse->objectName()));
                    else
                        room->gameOver(survivor->objectName());

                    scenario->disposeSpouseMap(room);
                    return true;
                }else if(players.length() == 2){
                    ServerPlayer *first = players.at(0);
                    ServerPlayer *second = players.at(1);
                    if(scenario->getSpouse(first) == second){
                        room->gameOver(QString("%1+%2").arg(first->objectName()).arg(second->objectName()));

                        scenario->disposeSpouseMap(room);
                        return true;
                    }
                }

                // reward and punishment
                QString killer_name = data.toString();
                if(!killer_name.isEmpty()){
                    ServerPlayer *killer = room->findChild<ServerPlayer *>(killer_name);

                    if(scenario->getSpouse(killer) == player || player->isLord())
                        killer->throwAllCards();
                    else
                        killer->drawCards(3);
                }
            }

        default:
            break;
        }

        return false;
    }
};

CoupleScenario::CoupleScenario()
    :Scenario("couple")
{
    lord = "caocao";
    renegades << "lubu" << "diaochan";
    rule = new CoupleScenarioRule(this);

    t["couple"] = tr("couple");

    t["reselect"] = tr("reselect");

    t["#Marry"] = tr("#Marry");
    t["#Divorse"] = tr("#Divorse");

    map["caopi"] = "zhenji";
    map["guojia"] = "simayi";
    map["liubei"] = "sunshangxiang";
    map["zhugeliang"] = "huangyueying";
    map["menghuo"] = "zhurong";
    map["zhouyu"] = "xiaoqiao";
    map["lubu"] = "diaochan";

    full_map = map;
    full_map["dongzhuo"] = "diaochan";
    full_map["wolong"] = "huangyueying";
}

void CoupleScenario::marryAll(Room *room) const{
    SpouseMapStar spouse_map = new SpouseMap;

    foreach(QString husband_name, full_map.keys()){
        ServerPlayer *husband = room->findPlayer(husband_name, true);
        if(husband == NULL)
            continue;

        QString wife_name = map.value(husband_name, QString());
        if(!wife_name.isNull()){
            ServerPlayer *wife = room->findPlayer(wife_name);
            marry(husband, wife, spouse_map);
        }
    }

    QVariant data = QVariant::fromValue(spouse_map);
    room->setTag("SpouseMap", data);
}

void CoupleScenario::marry(ServerPlayer *husband, ServerPlayer *wife, SpouseMapStar map_star) const{
    if(map_star->value(husband, NULL) == wife && map_star->value(wife, NULL) == husband)
        return;

    LogMessage log;
    log.type = "#Marry";
    log.from = husband;
    log.to << wife;
    husband->getRoom()->sendLog(log);

    map_star->insert(husband, wife);
    map_star->insert(wife, husband);
}

void CoupleScenario::remarry(ServerPlayer *enkemann, ServerPlayer *widow) const{
    Room *room = enkemann->getRoom();
    SpouseMapStar map_star = room->getTag("SpouseMap").value<SpouseMapStar>();

    ServerPlayer *ex_husband = map_star->value(widow);
    map_star->remove(ex_husband);
    LogMessage log;
    log.type = "#Divorse";
    log.from = widow;
    log.to << ex_husband;
    room->sendLog(log);

    ServerPlayer *ex_wife = map_star->value(enkemann, NULL);
    if(ex_wife){
        map_star->remove(ex_wife);
        LogMessage log;
        log.type = "#Divorse";
        log.from = enkemann;
        log.to << ex_wife;
        room->sendLog(log);
    }

    marry(enkemann, widow, map_star);
    room->setPlayerProperty(widow, "role", "loyalist");
}

ServerPlayer *CoupleScenario::getSpouse(ServerPlayer *player) const{
    Room *room = player->getRoom();
    SpouseMapStar map_star = room->getTag("SpouseMap").value<SpouseMapStar>();
    return map_star->value(player, NULL);
}

void CoupleScenario::disposeSpouseMap(Room *room) const{
    SpouseMapStar map_star = room->getTag("SpouseMap").value<SpouseMapStar>();
    delete map_star;

    room->setTag("SpouseMap", QVariant());
}

bool CoupleScenario::isWidow(ServerPlayer *player) const{
    if(player->getGeneral()->isMale())
        return false;

    ServerPlayer *spouse = getSpouse(player);
    return spouse && spouse->isDead();
}

void CoupleScenario::assign(QStringList &generals, QStringList &roles) const{
    generals << lord;

    QStringList husbands = map.keys();
    qShuffle(husbands);
    husbands = husbands.mid(0, 4);

    QStringList others;
    foreach(QString husband, husbands)
        others << husband << map.value(husband);

    qShuffle(others);
    generals << others;

    // roles
    roles << "lord";
    int i;
    for(i=0; i<8; i++)
        roles << "renegade";
}

int CoupleScenario::getPlayerCount() const{
    return 9;
}

void CoupleScenario::getRoles(char *roles) const{
    strcpy(roles, "ZNNNNNNNN");
}

void CoupleScenario::onTagSet(Room *room, const QString &key) const{

}

ADD_SCENARIO(Couple);
