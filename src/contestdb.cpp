#include "contestdb.h"
#include "engine.h"
#include "settings.h"
#include "serverplayer.h"
#include "lua.hpp"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDateTime>

const QString ContestDB::TimeFormat = "MMdd-hhmmss";

ContestDB::ContestDB(QObject *parent) :
    QObject(parent)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QString filename = Config.value("Contest/Database", "users.db").toString();
    db.setDatabaseName(filename);

    bool ok = db.open();
    if(!ok){
        QSqlError error = db.lastError();
        QMessageBox::warning(NULL, tr("Database open error"), tr("The database can not be opened:\n %1").arg(error.text()));
    }


    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS results"
               "(start_time TEXT PRIMARY KEY,"
               "username TEXT,"
               "general TEXT,"
               "role TEXT,"
               "score INTEGER,"
               "victims TEXT,"
               "alive INTEGER)");

    QSqlQuery query2;
    query2.exec("CREATE TABLE IF NOT EXISTS rooms"
               "(start_time TEXT PROMARY KEY"
               "end_time TEXT,"
               "winner TEXT)");
}

ContestDB *ContestDB::GetInstance(){
    static ContestDB *db;
    if(db == NULL)
        db = new ContestDB(Sanguosha);

    return db;
}

bool ContestDB::loadMembers(){
    QSqlQuery query("SELECT username, password, salt FROM users");
    QSqlError error = query.lastError();
    if(error.isValid()){
        QMessageBox::warning(NULL, tr("Database query error"), tr("Load member table error:\n %1").arg(error.text()));
        return false;
    }

    while(query.next()){
        QString username = query.value(0).toString();

        Member member;
        member.password = query.value(1).toString();
        member.salt = query.value(2).toString();

        members.insert(username, member);
    }

    return true;
}

bool ContestDB::checkPassword(const QString &username, const QString &password){
    if(members.contains(username)){
        Member member = members.value(username);
        QString salted = password + member.salt;
        QString digest = QCryptographicHash::hash(salted.toAscii(), QCryptographicHash::Md5).toHex();
        return digest == member.password;
    }else
        return false;
}

void ContestDB::saveResult(const QList<ServerPlayer *> &players, const QString &winner){
    Room *room = players.first()->getRoom();
    QString start_time = room->getTag("StartTime").toDateTime().toString(TimeFormat);

    QSqlQuery query;
    query.prepare("INSERT INTO results (start_time, username, general, role, score, victims, alive)"
                  "VALUES (?, ?, ?, ?, ?, ?, ?)");

    QVariantList start_times, usernames, generals, roles, scores, victims, alives;
    foreach(ServerPlayer *player, players){
        start_times << start_time;
        usernames << player->screenName();
        generals << player->getGeneralName();
        roles << player->getRole();
        scores << getScore(player, winner);

        QStringList victim_list;
        foreach(ServerPlayer *victim, player->getVictims()){
            victim_list << victim->getGeneralName();
        }

        victims << victim_list.join("+");

        alives << player->isAlive();
    }

    query.addBindValue(start_times);
    query.addBindValue(usernames);
    query.addBindValue(generals);
    query.addBindValue(roles);
    query.addBindValue(scores);
    query.addBindValue(victims);
    query.addBindValue(alives);

    if(!query.execBatch())
        room->output(query.lastError().text());

    QSqlQuery query2;
    query2.prepare("INSERT INTO rooms (start_time, end_time, winner)"
                   "VALUES (?, ?, ?)");
    query2.addBindValue(start_time);
    query2.addBindValue(QDateTime::currentDateTime().toString(TimeFormat));
    query2.addBindValue(winner);

    if(!query2.exec())
        room->output(query2.lastError().text());
}

int ContestDB::getScore(ServerPlayer *player, const QString &winner){
    QString role = player->getRole();
    Room *room = player->getRoom();

    if(winner == "lord+loyalist"){
        if(role == "rebel")
            return 0;
        else if(role == "renegade"){
            if(room->getTag("RenegadeInFinalPK").toBool())
                return 8;
            else
                return 0;
        }else{
            int loyalists = 0, victims = 0;
            QList<ServerPlayer *> survivors = room->getAlivePlayers();
            foreach(ServerPlayer *survivor, survivors){
                if(survivor->getRole() == "loyalist")
                    loyalists ++;
            }

            foreach(ServerPlayer *victim, player->getVictims()){
                if(victim->getRole() == "rebel" || victim->getRole() == "renegade")
                    victims ++;
            }

            if(role == "lord")
                return 4 + loyalists * 2 + victims;
            else
                return 5 + loyalists + victims;

        }
    }else if(winner == "rebel"){
        if(role == "lord" || role == "loyalist")
            return 0;
        else if(role == "renegade")
            return player->isAlive() ? 1 : 0;

        QList<ServerPlayer *> survivors = room->getAlivePlayers();
        int rebels = 0;
        foreach(ServerPlayer *survivor, survivors){
            if(survivor->getRole() == "rebel")
                rebels ++;
        }

        int bonus = 0;
        foreach(ServerPlayer *victim, player->getVictims()){
            if(victim->getRole() == "loyalist" || victim->getRole() == "renegade")
                bonus ++;
            else if(victim->isLord())
                bonus += 2;
        }

        return rebels * 3 + bonus;
    }

    // renegade win
    if(role == "renegade")
        return 20;
    else if(role == "lord")
        return 1;
    else
        return 0;
}

void ContestDB::sendResult(Room *room){
    QString start_time = room->getTag("StartTime").toDateTime().toString(TimeFormat);

    lua_State *L = room->getLuaState();

    int error = luaL_loadfile(L, "lua/tools/send-result.lua");
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
        return;
    }

    lua_pushstring(L, start_time.toAscii());

    error = lua_pcall(L, 1, 1, 0);
    if(error){
        const char *error_msg = lua_tostring(L, -1);
        lua_pop(L, 1);
        room->output(error_msg);
    }
}
