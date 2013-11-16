#include "record-analysis.h"
#include "recorder.h"
#include "settings.h"
#include "engine.h"

#include <QFile>
#include <QMessageBox>

using namespace QSanProtocol;

RecAnalysis::RecAnalysis(QString dir): m_recordPlayers(0), m_currentPlayer(NULL) {
    initialize(dir);
}

void RecAnalysis::initialize(QString dir) {
    QStringList records_line;
    if (dir.isEmpty())
        records_line = ClientInstance->getRecords();
    else if (dir.endsWith(".png")) {
        QByteArray data = Replayer::PNG2TXT(dir);
        QString record_data(data);
        records_line = record_data.split("\n");
    }
    else if (dir.endsWith(".txt")) {
        QFile file(dir);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream stream(&file);
            while (!stream.atEnd()) {
                QString line = stream.readLine();
                records_line << line;
            }
        }
    } else {
        QMessageBox::warning(NULL, tr("Warning"), tr("The file is unreadable"));
        return;
    }
    records_line.removeAll(QString());

    QStringList role_list;
    foreach (QString line, records_line) {
        if (line.contains(QString(S_PLAYER_SELF_REFERENCE_ID))) {
            line = line.split("[").last().remove("]");
            line.remove(QRegExp("[^a-zA-Z0-9_,]"));
            QStringList self_info = line.split(",");
            if (self_info.at(1) == "objectName")
                getPlayer(self_info.at(2), QString(S_PLAYER_SELF_REFERENCE_ID))->m_screenName = Config.UserName;
            else if (self_info.at(1) == "role")
                getPlayer(QString(S_PLAYER_SELF_REFERENCE_ID))->m_role = self_info.at(2);
            else if (self_info.at(1) == "general")
                getPlayer(QString(S_PLAYER_SELF_REFERENCE_ID))->m_generalName = self_info.at(2);
            else if (self_info.at(1) == "general2")
                getPlayer(QString(S_PLAYER_SELF_REFERENCE_ID))->m_general2Name = self_info.at(2);

            continue;
        }

        if (line.contains("setup")) {
            QRegExp rx("(.*):(@?\\w+):(\\d+):(\\d+):([+\\w]*):([RCFSTBHAMN123a-r]*)(\\s+)?");
            if (!rx.exactMatch(line))
                continue;

            QStringList texts = rx.capturedTexts();
            m_recordGameMode = texts.at(2);
            if (texts.startsWith("02_1v1"))
                m_recordGameMode = "02_1v1";
            else if (texts.startsWith("06_3v3"))
                m_recordGameMode = "06_3v3";
            m_recordPlayers = texts.at(2).split("_").first().remove(QRegExp("[^0-9]")).toInt();
            QStringList ban_packages = texts.at(5).split("+");
            foreach (Package *package, Sanguosha->findChildren<Package *>()) {
                if (!ban_packages.contains(package->objectName())
                    && Sanguosha->getScenario(package->objectName()) == NULL)
                    m_recordPackages << Sanguosha->translate(package->objectName());
            }

            QString flags = texts.at(6);
            if (flags.contains("R")) m_recordServerOptions << tr("RandomSeats");
            if (flags.contains("C")) m_recordServerOptions << tr("EnableCheat");
            if (flags.contains("F")) m_recordServerOptions << tr("FreeChoose");
            if (flags.contains("S")) m_recordServerOptions << tr("Enable2ndGeneral");
            if (flags.contains("T")) m_recordServerOptions << tr("EnableSame");
            if (flags.contains("N")) m_recordServerOptions << tr("EnableScene");
            if (flags.contains("B")) m_recordServerOptions << tr("EnableBasara");
            if (flags.contains("H")) m_recordServerOptions << tr("EnableHegemony");
            if (flags.contains("A")) m_recordServerOptions << tr("EnableAI");

            continue;
        }

        if (line.contains("arrangeSeats")) {
            QStringList line_struct = line.split(QRegExp("\\s+"));
            line_struct.removeAll(QString());
            role_list = line_struct.last().split("+");

            continue;
        }

        if (line.contains("addPlayer")) {
            QStringList info_assemble = line.split(" ").last().split(":");
            getPlayer(info_assemble.at(0))->m_screenName = QString::fromUtf8(QByteArray::fromBase64(info_assemble.at(1).toAscii()));
            continue;
        }

        if (line.contains("removePlayer")) {
            QString name = line.split(" ").last();
            m_recordMap.remove(name);
            continue;
        }

        if (line.contains("speak")) {
            QString speaker = line.split(":").first();
            speaker.remove(0, speaker.lastIndexOf(" ") + 1);
            QString words = line.split(":").last().remove(" ");
            words = QString::fromUtf8(QByteArray::fromBase64(words.toAscii()));
            m_recordChat += getPlayer(speaker)->m_screenName+": "+words;
            m_recordChat.append("<br/>");

            continue;
        }

        if (line.contains("general")) {
            foreach (QString object, m_recordMap.keys()) {
                if (line.contains(object)) {
                    QString general = line.split(",").last();
                    general.remove(QRegExp("[^a-z_]+"));

                    line.contains("general2") ?
                            getPlayer(object)->m_general2Name = general :
                            getPlayer(object)->m_generalName = general;
                }
            }

            continue;
        }

        if (line.contains("state") && line.contains("robot")) {
            foreach (QString object_name, m_recordMap.keys()) {
                if (line.contains(object_name)) {
                    getPlayer(object_name)->m_statue = "robot";
                    break;
                }
            }

            continue;
        }

        if (line.contains(QString("%1,%2,").arg(int(S_SRC_ROOM | S_TYPE_NOTIFICATION | S_DEST_CLIENT)).arg(int(S_COMMAND_CHANGE_HP)))) { // means hpChange
            QString name = line.split("[").last().split(",").first();
            name = name.mid(1, name.length() - 2);
            QString delta = line.split(",").at(5);
            bool ok = false;
            int hp_change = delta.toInt(&ok);
            if (!ok)
                continue;
            if (hp_change > 0)
                getPlayer(name)->m_recover += hp_change;

            continue;
        }

        if (line.contains("#Damage")) {
            QStringList texts = line.split(",");
            QString object_damage = line.contains("#DamageNoSource") ? QString() : texts.at(5).mid(1, texts.at(5).length() - 2);
            QString object_damaged = texts.at(6).mid(1, texts.at(6).length() - 2);
            int damage = texts.at(8).mid(1, texts.at(8).length() - 2).toInt();

            if (!object_damage.isEmpty())
                getPlayer(object_damage)->m_damage += damage;
            getPlayer(object_damaged)->m_damaged += damage;
            continue;
        }

        if (line.contains("#Murder") || line.contains("#Suicide")) {
            QStringList texts = line.split(",");
            QString object = texts.at(5).mid(1, texts.at(5).length() - 2);
            getPlayer(object)->m_kill++;
            object = texts.at(6).mid(1, texts.at(6).length() - 2);
            getPlayer(object)->m_isAlive = false;

            continue;
        }

        if (line.contains("#Contingency")) {
            QStringList texts = line.split(",");
            QString object = texts.at(6).mid(1, texts.at(6).length() - 2);
            getPlayer(object)->m_isAlive = false;

            continue;
        }

        if (line.contains("Global_TurnCount")) {
            QString object = line.split("\"").at(1);
            PlayerRecordStruct *rec = getPlayer(object);
            rec->m_turnCount++;
            m_currentPlayer = rec;

            continue;
        }
    }

    QString winners = records_line.last();
    winners.remove(winners.lastIndexOf("[") - 1, winners.length());
    winners = winners.split("[").last();
    m_recordWinners = winners.remove("\"").split("+");

    winners = records_line.last().remove("\"");
    winners.remove(0, winners.lastIndexOf("[") + 1);
    QStringList roles_order = winners.remove(QRegExp("[^a-z,]+")).split(",");
    for (int i = 0; i < role_list.length(); i++)
        getPlayer(role_list.at(i))->m_role = roles_order.at(i);

    setDesignation();
}

PlayerRecordStruct *RecAnalysis::getPlayerRecord(const Player *player) const{
    if (m_recordMap.keys().contains(player->objectName()))
        return m_recordMap[player->objectName()];
    else
        return NULL;
}

QMap<QString, PlayerRecordStruct *> RecAnalysis::getRecordMap() const{
    return m_recordMap;
}

QStringList RecAnalysis::getRecordPackages() const{
    return m_recordPackages;
}

QStringList RecAnalysis::getRecordWinners() const{
    return m_recordWinners;
}

QString RecAnalysis::getRecordGameMode() const{
    return m_recordGameMode;
}

QStringList RecAnalysis::getRecordServerOptions() const{
    return m_recordServerOptions;
}

QString RecAnalysis::getRecordChat() const{
    return m_recordChat;
}

PlayerRecordStruct *RecAnalysis::getPlayer(QString object_name, const QString &addition_name) {
    if (m_recordMap.keys().contains(addition_name)) {
        m_recordMap[object_name] = m_recordMap[addition_name];
        m_recordMap[object_name]->m_additionName = addition_name;
        m_recordMap.remove(addition_name);
    } else if (!m_recordMap.keys().contains(addition_name) && !addition_name.isEmpty()) {
        m_recordMap[object_name] = new PlayerRecordStruct;
        m_recordMap[object_name]->m_additionName = addition_name;
    } else if (!m_recordMap.keys().contains(object_name)) {
        bool inQueue = false;
        foreach (QString name, m_recordMap.keys()) {
            if (m_recordMap[name]->m_additionName == object_name) {
                object_name = name;
                inQueue = true;
                break;
            }
        }

        if (!inQueue) m_recordMap[object_name] = new PlayerRecordStruct;
    }

    return m_recordMap[object_name];
}

const unsigned int RecAnalysis::findPlayerOfDamage(int n) const{
    int result = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_damage >= n) result++;
        result *= 2;
    }
    return result / 2;
}

const unsigned int RecAnalysis::findPlayerOfDamaged(int n) const{
    int result = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_damaged >= n) result++;
        result *= 2;
    }
    return result / 2;
}

const unsigned int RecAnalysis::findPlayerOfKills(int n) const{
    int result = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_kill >= n) result++;
        result *= 2;
    }
    return result / 2;
}

const unsigned int RecAnalysis::findPlayerOfRecover(int n) const{
    int result = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_recover >= n) result++;
        result *= 2;
    }
    return result / 2;
}

const unsigned int RecAnalysis::findPlayerOfDamage(int upper, int lower) const{
    int result = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_damage >= upper && s->m_damage <= lower) result++;
        result *= 2;
    }
    return result / 2;
}

const unsigned int RecAnalysis::findPlayerOfDamaged(int upper, int lower) const{
    int result = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_damaged >= upper && s->m_damaged <= lower) result++;
        result *= 2;
    }
    return result / 2;
}

const unsigned int RecAnalysis::findPlayerOfRecover(int upper, int lower) const{
    int result = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_recover >= upper && s->m_recover <= lower) result++;
        result *= 2;
    }
    return result / 2;
}

const unsigned int RecAnalysis::findPlayerOfKills(int upper, int lower) const{
    int result = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_kill >= upper && s->m_kill <= lower) result++;
        result *= 2;
    }
    return result / 2;
}

void RecAnalysis::setDesignation() {
    if (m_recordPlayers < 5)
        return;

    initialDesignation();

    unsigned int rec_data = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_turnCount == 0) rec_data++;
        rec_data *= 2;
    }
    addDesignation(tr("Soy"), NoOption, rec_data / 2, true, QString(), false, true);

    rec_data = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_turnCount == 1 && m_currentPlayer == s) rec_data++;
        rec_data *= 2;
    }
    addDesignation(tr("Rapid Victory"), NoOption, rec_data / 2, true, QString(), true, false, true);

    addDesignation(tr("Burning Soul"), MostDamage | MostDamaged);
    addDesignation(tr("Regretful Lose"), MostDamage | ZeroKill, M_ALL_PLAYER, true, QString(), false, false, false, true);
    addDesignation(tr("Fall Short"), NoOption, findPlayerOfKills(m_recordPlayers - 2), m_recordWinners.contains("lord"),
                   "renegade", false, false, false, true);
    addDesignation(tr("Wicked Kill"), LeastDamage | MostKill);
    addDesignation(tr("Peaceful Watcher"), ZeroDamage | LeastDamaged, findPlayerOfRecover(1));
    addDesignation(tr("MVP"), MostKill | MostDamage | MostRecover, findPlayerOfDamage(10) & findPlayerOfRecover(10),
                   true, QString(), true, false, true);
    addDesignation(tr("Useless alive"), ZeroDamage | ZeroDamaged | ZeroRecover|ZeroKill);
    addDesignation(tr("Awe Prestige"), MostKill | MostDamage, findPlayerOfKills(3), true, "lord", true);

    addDesignation(tr("Wisely Loyalist"), ZeroDamaged, M_ALL_PLAYER, true, "loyalist", true, false, true);
    addDesignation(tr("Conspiracy"), ZeroDamaged, M_ALL_PLAYER, true, "renegade", true, false, true);

    addDesignation(tr("Vanguard"), MostKill, findPlayerOfKills(2), true, "~lord", true);
    addDesignation(tr("Fierce Lord"), MostKill, findPlayerOfKills(2), true, "lord", true);
    addDesignation(tr("Blood Judgement"), MostKill, findPlayerOfKills((int)(m_recordPlayers / 2.0 + 0.5)));
    addDesignation(tr("Rampage"), MostKill, findPlayerOfKills(m_recordPlayers - 1));
    addDesignation(tr("Unrealized Aspiration"), MostKill | MostDamage, M_ALL_PLAYER, true, QString(), false, false, false, true);

    bool only = true;
    int count_kill = 0, count_dead = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_kill == 1) count_kill++;
        if (!s->m_isAlive) count_dead++;
        if (s->m_kill > 1 || count_kill > 1 || count_dead > 1) {
            only = false;
            break;
        }
        if (s->m_role == "lord" && s->m_isAlive) {
            only = false;
            break;
        }
    }
    addDesignation(tr("Break Point"), MostKill, findPlayerOfKills(1), only, "rebel", false, false, true);

    addDesignation(tr("Legatus"), MostDamage, M_ALL_PLAYER, true, "~lord", true, false, true);
    addDesignation(tr("Frightful Lord"), MostDamage, M_ALL_PLAYER, true, "lord", true);
    addDesignation(tr("Bloody Warrior"), MostDamage, findPlayerOfDamage(10, 14));
    addDesignation(tr("Warrior Soul"), MostDamage, findPlayerOfDamage(15, 19));
    addDesignation(tr("Wrath Warlord"), MostDamage, findPlayerOfDamage(20));

    addDesignation(tr("Peaceful"), MostRecover, findPlayerOfRecover(10));
    addDesignation(tr("Recovery"), MostRecover, findPlayerOfRecover(5, 9));

    addDesignation(tr("Fodder"), MostDamaged | ZeroDamage, M_ALL_PLAYER, true, "~lord", false, true);
    addDesignation(tr("Fire Target"), MostDamaged, findPlayerOfDamaged(15));
    addDesignation(tr("Master Tank"), MostDamaged, findPlayerOfDamaged(10), true, QString(), true, false, true);
    addDesignation(tr("War Spirit"), MostDamaged, findPlayerOfDamaged(10), true, QString(), true, false, false, true);

    int loyal_num = 0, rebel_num = 0;
    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        if (s->m_role == "loyalist" && s->m_isAlive) loyal_num++;
        if (s->m_role == "rebel" && s->m_isAlive) rebel_num++;
    }
    addDesignation(tr("Priority Honor"), NoOption, M_ALL_PLAYER, loyal_num == 1, "loyalist", true, false, true);
    addDesignation(tr("Impasse Strike"), NoOption, M_ALL_PLAYER, rebel_num == 1, "rebel", true, false, true);
}

void RecAnalysis::addDesignation(const QString &designation,
                                 unsigned long designation_union,
                                 unsigned int data_requirement,
                                 bool custom_condition,
                                 const QString &addition_option_role,
                                 bool need_alive,
                                 bool need_dead,
                                 bool need_win,
                                 bool need_lose) {
    if (!custom_condition) return;

    QList<DesignationType> des_union;
    for (long i = ZeroDamaged; i > 0 && designation_union > 0; i /= 2) {
        if ((unsigned long)i <= designation_union) {
            des_union << static_cast<DesignationType>(i);
            designation_union -= (unsigned long)i;
        }
    }

    unsigned int player_test_mask = 1;
    for (int i = 0; i < m_recordMap.size(); i++)
        player_test_mask *= 2;

    foreach (QString objectName, m_recordMap.keys()) {
        player_test_mask /= 2;
        bool has_player = custom_condition;
        foreach (DesignationType type, des_union)
            if (!m_recordMap[objectName]->m_designEnum.contains(type)) { has_player = false; break; }

        if (need_win
            && !m_recordWinners.contains(m_recordMap[objectName]->m_role)
            && !m_recordWinners.contains(objectName)) {
            has_player = false;
        }

        if (need_lose
            && (m_recordWinners.contains(m_recordMap[objectName]->m_role)
                || m_recordWinners.contains(objectName))) {
            has_player = false;
        }

        if (!has_player) continue;
        if ((data_requirement & player_test_mask) == 0) continue;

        if (!addition_option_role.isEmpty()) {
            if (!isNormalGameMode(m_recordGameMode))
                continue;
            if (addition_option_role.startsWith("~")) {
                QString role = addition_option_role;
                role.remove("~");
                has_player &= m_recordMap[objectName]->m_role != role;
            } else
                has_player &= m_recordMap[objectName]->m_role == addition_option_role;
        }
        if (need_alive)
            has_player &= m_recordMap[objectName]->m_isAlive;
        if (need_dead)
            has_player &= !m_recordMap[objectName]->m_isAlive;

        if (has_player) {
            m_recordMap[objectName]->m_designation << designation;
            //break;
        }
    }
}

void RecAnalysis::initialDesignation() {
    int max_damage = 0, max_damaged = 0, max_recover = 0, max_kill = 0;
    int least_damage = 999, least_damaged = 999, least_recover = 999, least_kill = 999;
    QStringList maxDamagePlayer, maxDamagedPlayer, maxRecoverPlayer, maxKillPlayer;
    QStringList leastDamagePlayer, leastDamagedPlayer, leastRecoverPlayer, leastKillPlayer;

    foreach (PlayerRecordStruct *s, m_recordMap.values()) {
        QString objectName =  m_recordMap.key(s);
        if (s->m_damage >= max_damage && s->m_damage > 0) {
            if (s->m_damage > max_damage) {
                max_damage = s->m_damage;
                maxDamagePlayer.clear();
            }
            maxDamagePlayer << objectName;
        }
        if (s->m_damaged >= max_damaged && s->m_damaged > 0) {
            if (s->m_damaged > max_damaged) {
                max_damaged = s->m_damaged;
                maxDamagedPlayer.clear();
            }
            maxDamagedPlayer << objectName;
        }
        if (s->m_recover >= max_recover && s->m_recover > 0) {
            if (s->m_recover > max_recover) {
                max_recover = s->m_recover;
                maxRecoverPlayer.clear();
            }
            maxRecoverPlayer << objectName;
        }
        if (s->m_kill >= max_kill && s->m_kill > 0) {
            if (s->m_kill > max_kill) {
                max_kill = s->m_kill;
                maxKillPlayer.clear();
            }
            maxKillPlayer << objectName;
        }

        if (s->m_damage <= least_damage) {
            if (s->m_damage < least_damage) {
                least_damage = s->m_damage;
                leastDamagePlayer.clear();
            }
            leastDamagePlayer << objectName;
        }
        if (s->m_damaged <= least_damaged) {
            if (s->m_damaged < least_damaged) {
                least_damaged = s->m_damaged;
                leastDamagedPlayer.clear();
            }
            leastDamagedPlayer << objectName;
        }
        if (s->m_recover <= least_recover) {
            if (s->m_recover < least_recover) {
                least_recover = s->m_recover;
                leastRecoverPlayer.clear();
            }
            maxRecoverPlayer << objectName;
        }
        if (s->m_kill <= least_kill) {
            if (s->m_kill < least_kill) {
                least_kill = s->m_kill;
                leastKillPlayer.clear();
            }
            leastKillPlayer << objectName;
        }

        if (s->m_damage == 0) s->m_designEnum << ZeroDamage;
        if (s->m_damaged == 0) s->m_designEnum << ZeroDamaged;
        if (s->m_recover == 0) s->m_designEnum << ZeroRecover;
        if (s->m_kill == 0) s->m_designEnum << ZeroKill;
    }

    foreach (QString player, maxDamagedPlayer) m_recordMap[player]->m_designEnum << MostDamaged;
    foreach (QString player, maxDamagePlayer) m_recordMap[player]->m_designEnum << MostDamage;
    foreach (QString player, maxRecoverPlayer) m_recordMap[player]->m_designEnum << MostRecover;
    foreach (QString player, maxKillPlayer) m_recordMap[player]->m_designEnum << MostKill;
    foreach (QString player, leastDamagedPlayer) m_recordMap[player]->m_designEnum << LeastDamaged;
    foreach (QString player, leastDamagePlayer) m_recordMap[player]->m_designEnum << LeastDamage;
    foreach (QString player, leastRecoverPlayer) m_recordMap[player]->m_designEnum << LeastRecover;
    foreach (QString player, leastKillPlayer) m_recordMap[player]->m_designEnum << LeastKill;
}

PlayerRecordStruct::PlayerRecordStruct()
    : m_statue("online"), m_turnCount(0), m_recover(0), m_damage(0),
      m_damaged(0), m_kill(0), m_isAlive(true)
{
}

bool PlayerRecordStruct::isNull() {
    return m_screenName.isEmpty() || m_generalName.isEmpty();
}

