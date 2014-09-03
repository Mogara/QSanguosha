#include "jiange-defense-scenario.h"
#include "skill.h"
#include "engine.h"
#include "room.h"
#include "roomthread.h"

class JiangeDefenseScenarioRule: public ScenarioRule {
public:
    JiangeDefenseScenarioRule(Scenario *scenario)
        : ScenarioRule(scenario)
    {
        events << GameStart;
    }

    virtual bool effect(TriggerEvent, Room *room, ServerPlayer *player, QVariant &, ServerPlayer *) const{
        if (player == NULL)
            foreach (ServerPlayer *p, room->getPlayers())
                if (p->getActualGeneral1Name().startsWith("jg_"))
                    p->showGeneral(true, true, false);
        return false;
    }
};

JiangeDefenseScenario::JiangeDefenseScenario()
    : Scenario("jiange_defense")
{
    rule = new JiangeDefenseScenarioRule(this);
}

void JiangeDefenseScenario::assign(QStringList &generals, QStringList &generals2, QStringList &kingdoms, Room *room) const{
    QMap<QString, QStringList> roles;
    QStringList wei_roles, shu_roles;
    wei_roles << "ghost" << "machine" << "human" << "human";
    shu_roles << "ghost" << "machine" << "human" << "human";
    roles.insert("wei", wei_roles);
    roles.insert("shu", shu_roles);
    qShuffle(kingdoms);
    QStringList wei_generals, shu_generals;
    foreach (QString general, Sanguosha->getLimitedGeneralNames()) {
        if (general.startsWith("lord_")) continue;
        QString kingdom = Sanguosha->getGeneral(general)->getKingdom();
        if (kingdom == "wei")
            wei_generals << general;
        else if (kingdom == "shu")
            shu_generals << general;
    }
    qShuffle(wei_generals);
    qShuffle(shu_generals);
    Q_ASSERT(wei_generals.length() >= 10 && shu_generals.length() >= 10);
    QMap<ServerPlayer *, QStringList> human_map; // Rara said, human couldn't get ghost or machine as its general.
    QList<ServerPlayer *> players = room->getPlayers();
    for (int i = 0; i < 8; i++) {
        if (players[i]->getState() == "online") {
            QStringList choices;
            foreach (QString kingdom, roles.keys())
                if (roles[kingdom].contains("human"))
                    choices << kingdom;
            QString choice = choices.at(qrand() % choices.length());
            QStringList role_list = roles[choice];
            role_list.removeOne("human");
            roles[choice] = role_list;
            if (choice == "wei") {
                QStringList weijiangs;
                for (int j = 0; j < 5; j++)
                    weijiangs << wei_generals.takeFirst();
                QStringList answer = room->askForGeneral(players[i], weijiangs, QString(), false).split("+");
                answer.prepend("wei");
                human_map.insert(players[i], answer);
            } else if (choice == "shu") {
                QStringList shujiangs;
                for (int j = 0; j < 5; j++)
                    shujiangs << shu_generals.takeFirst();
                QStringList answer = room->askForGeneral(players[i], shujiangs, QString(), false).split("+");
                answer.prepend("shu");
                human_map.insert(players[i], answer);
            }
        }
    }
                    
    for (int i = 0; i < 8; i++) {
        if (human_map.contains(players[i])) {
            QStringList answer = human_map[players[i]];
            kingdoms << answer.takeFirst();
            generals << answer.takeFirst();
            generals2 << answer.takeFirst();
        } else {
            QStringList kingdom_choices;
            foreach (QString kingdom, roles.keys())
                if (!roles[kingdom].isEmpty())
                    kingdom_choices << kingdom;
            QString kingdom = kingdom_choices.at(qrand() % kingdom_choices.length());
            kingdoms << kingdom;
            QStringList role_list = roles[kingdom];
            QString role = role_list.at(qrand() % role_list.length());
            role_list.removeOne(role);
            roles[kingdom] = role_list;
            if (role == "ghost") {
                QString name = kingdom == "wei" ? getRandomWeiGhost() : getRandomShuGhost();
                generals << name;
                generals2 << name;
            } else if (role == "machine") {
                QString name = kingdom == "wei" ? getRandomWeiMachine() : getRandomShuMachine();
                generals << name;
                generals2 << name;
            } else if (role == "human") {
                int n = kingdom == "wei" ? 4 : 5;
                QStringList choices;
                for (int j = 0; j < n; j++)
                    choices << ((kingdom == "wei") ? wei_generals.takeFirst() : shu_generals.takeFirst());
                QString answer = room->askForGeneral(players[i], choices, QString(), false);
                generals << answer.split("+").first();
                generals2 << answer.split("+").last();
            }
        }
    }
}

int JiangeDefenseScenario::getPlayerCount() const{
    return 8;
}

QString JiangeDefenseScenario::getRoles() const{
    return "ZNNNNNNN";
}

void JiangeDefenseScenario::onTagSet(Room *, const QString &) const{
}

QString JiangeDefenseScenario::getRandomWeiGhost() const{
    QStringList ghosts;
    ghosts << "jg_caozhen" << "jg_xiahou" << "jg_sima" << "jg_zhanghe";
    return ghosts.at(qrand() % ghosts.length());
}

QString JiangeDefenseScenario::getRandomWeiMachine() const{
    QStringList machines;
    machines << "jg_bian_machine" << "jg_suanni_machine" << "jg_chiwen_machine" << "jg_yazi_machine";
    return machines.at(qrand() % machines.length());
}

QString JiangeDefenseScenario::getRandomShuGhost() const{
    QStringList ghosts;
    ghosts << "jg_liubei" << "jg_zhuge" << "jg_yueying" << "jg_pangtong";
    return ghosts.at(qrand() % ghosts.length());
}

QString JiangeDefenseScenario::getRandomShuMachine() const{
    QStringList machines;
    machines << "jg_qinglong_machine" << "jg_baihu_machine" << "jg_zhuque_machine" << "jg_xuanwu_machine";
    return machines.at(qrand() % machines.length());
}