#ifndef CONTESTDB_H
#define CONTESTDB_H

#include <QObject>
#include <QHash>

class ServerPlayer;

class ContestDB : public QObject
{
    Q_OBJECT

public:
    static ContestDB *GetInstance();
    bool loadMembers();
    bool checkPassword(const QString &username, const QString &password);
    void saveResult(const QList<ServerPlayer *> &players, const QString &winner);

private:
    explicit ContestDB(QObject *parent);
    int getScore(ServerPlayer *player, const QString &winner);

    struct Member{
        QString password;
        QString salt;
    };

    QHash<QString, Member> members;
};

#endif // CONTESTDB_H
