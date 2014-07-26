#include "jiange-defense.h"
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
                generals << "simayi";
                generals2 << "simayi";
            } else if (role == "machine") {
                generals << "zhenji";
                generals2 << "zhenji";
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
                generals << "pangtong";
                generals2 << "pangtong";
            } else if (role == "machine") {
                generals << "huangyueying";
                generals2 << "huangyueying";
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

void JiangeDefenseScenario::onTagSet(Room *room, const QString &key) const{
}
