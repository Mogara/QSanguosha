#ifndef PLAYER_H
#define PLAYER_H

#include "general.h"
#include "card.h"

#include <QObject>
#include <QTcpSocket>

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int hp READ getHp WRITE setHp)
    Q_PROPERTY(int max_hp READ getMaxHP WRITE setMaxHP)
    Q_PROPERTY(bool wounded READ isWounded STORED false)    
    Q_PROPERTY(QString name READ objectName WRITE setObjectName STORED false)
    Q_PROPERTY(QString role READ getRole WRITE setRole)
    Q_PROPERTY(QString general READ getGeneral WRITE setGeneral)
    Q_PROPERTY(QString state READ getState WRITE setState)
    Q_PROPERTY(int handcard_num READ getHandcardNum)

public:
    explicit Player(QObject *parent = 0);

    // property setters/getters
    int getHp() const;
    void setHp(int hp);
    int getMaxHP() const;
    void setMaxHP(int max_hp);
    bool isWounded() const;
    int getGeneralMaxHP() const;

    void setRole(const QString &role);
    QString getRole() const;
    const General *getAvatarGeneral() const;

    void setGeneral(const QString &general_name);
    QString getGeneral() const;

    void setState(const QString &state);
    QString getState() const;

    int getHandcardNum() const;

    void drawCard(int card_num);
    void drawCard(const Card *card);

private:
    const General *general;
    int hp, max_hp;
    QString role;
    QString state;
    int handcard_num;
    QList<const Card*> handcards;

signals:
    void general_changed();
    void role_changed(const QString &new_role);
    void state_changed(const QString &new_state);
    void handcard_num_changed(int num);

    // just for server side
    //--------------------------------------------------
public:
    void setSocket(QTcpSocket *socket);
    void unicast(const QString &message);
    QString reportHeader() const;
    void sendProperty(const char *property_name);
private:
    QTcpSocket *socket;
private slots:
    void getRequest();
signals:
    void disconnected();
    void request_got(const QString &request);
};

#endif // PLAYER_H
