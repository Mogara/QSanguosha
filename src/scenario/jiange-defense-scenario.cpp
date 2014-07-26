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
        events << GameStart << GameOverJudge << BuryVictim;
    }
};

JiangeDefenseScenario::JiangeDefenseScenario()
    : Scenario("jiange_defense")
{
    rule = new JiangeDefenseScenarioRule(this);
}

void JiangeDefenseScenario::assign(QStringList &generals, QStringList &generals2, QStringList &kingdoms, Room *room) const{
    kingdoms << "wei" << "wei" << "wei" << "wei"
             << "shu" << "shu" << "shu" << "shu";
    QStringList wei_roles, shu_roles;
    wei_roles << "ghost" << "machine" << "human" << "human";
    shu_roles << "ghost" << "machine" << "human" << "human";
    qShuffle(kingdoms);
    qShuffle(wei_roles);
    qShuffle(shu_roles);
    QStringList wei_generals, shu_generals;
    foreach (QString general, Sanguosha->getLimitedGeneralNames()) {
        QString kingdom = Sanguosha->getGeneral(general)->getKingdom();
        if (kingdom == "wei")
            wei_generals << general;
        else if (kingdom == "shu")
            shu_generals << general;
    }
    QList<ServerPlayer *> players = room->getPlayers();
    for (int i = 0; i < 8; i++) {
        if (kingdoms[i] == "wei") {
            QString role = wei_roles.takeFirst();
            if (role == "ghost") {
                QString name = getRandomWeiGhost();
                generals << name;
                generals2 << name;
            } else if (role == "machine") {
                QString name = getRandomWeiMachine();
                generals << name;
                generals2 << name;
            } else if (role == "human") {
                QStringList choices;
                for (int j = 0; j < 4; j++)
                    choices << wei_generals.takeFirst();
                QString answer = room->askForGeneral(players[i], choices, QString(), false);
                generals << answer.split("+").first();
                generals2 << answer.split("+").last();
            }
        } else {
            QString role = shu_roles.takeFirst();
            if (role == "ghost") {
                QString name = getRandomShuGhost();
                generals << name;
                generals2 << name;
            } else if (role == "machine") {
                QString name = getRandomShuMachine();
                generals << name;
                generals2 << name;
            } else if (role == "human") {
                QStringList choices;
                for (int j = 0; j < 5; j++)
                    choices << shu_generals.takeFirst();
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
    machines << "jg_bian_machine" << "jg_suanni_machine" << "jg_taotie_machine" << "jg_yazi_machine";
    return machines.at(qrand() % machines.length());
}

QString JiangeDefenseScenario::getRandomShuGhost() const{
    QStringList ghosts;
    ghosts << "jg_liubei" << "jg_zhuge" << "jg_yueying" << "jg_pangtong";
    return ghosts.at(qrand() % ghosts.length());
}

QString JiangeDefenseScenario::getRandomShuMachine() const{
    QStringList machines;
    machines << "jg_qinglong_machine" << "jg_baohu_machine" << "jg_zhuque_machine" << "jg_xuanwu_machine";
    return machines.at(qrand() % machines.length());
}