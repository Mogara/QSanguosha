#ifndef PLAYER_H
#define PLAYER_H

#include "general.h"

#include <QObject>
#include <QTcpSocket>

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hp READ getHp WRITE setHp)
    Q_PROPERTY(bool wounded READ isWounded STORED false)    
    Q_PROPERTY(QString name READ objectName WRITE setObjectName STORED false)
    Q_PROPERTY(QString role READ getRole WRITE setRole)
    Q_PROPERTY(QString general READ getGeneral WRITE setGeneral)
    Q_PROPERTY(QString state READ getState WRITE setState)

public:
    explicit Player(QObject *parent = 0);

    // property setters/getters
    int getHp() const;
    void setHp(int hp);
    bool isWounded() const;

    void setGeneral(const QString &general_name);
    QString getGeneral() const;

    void setState(const QString &state);
    QString getState() const;

    void setRole(const QString &role);
    QString getRole() const;
    const General *getAvatarGeneral() const;

private:
    const General *general;
    int hp;
    QString role;
    QString state;

signals:
    void general_changed();
    void role_changed(const QString &new_role);
    void state_changed(const QString &new_state);

    // just for server side
    //--------------------------------------------------
public:
    void setSocket(QTcpSocket *socket);
    void unicast(const QString &message);
    QString reportHeader() const;
private:
    QTcpSocket *socket;
private slots:
    void getRequest();
signals:
    void disconnected();
    void request_got(const QString &request);
};

#endif // PLAYER_H
