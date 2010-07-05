#include "player.h"
#include "engine.h"

#include <QHostAddress>

Player::Player(QObject *parent)
    :QObject(parent), general(NULL), hp(-1), socket(NULL)
{
}

void Player::setHp(int hp){
    if(hp >= 0 && hp <= general->getMaxHp())
        this->hp = hp;
}

int Player::getHp() const{
    return hp;
}

bool Player::isWounded() const{
    return hp < general->getMaxHp();
}

void Player::setGeneral(const General *general){
    if(this->general != general){
        this->general = general;
        emit general_changed(general);
    }
}

const General *Player::getGeneral() const{
    return general;
}

void Player::setRole(const QString &role){
    if(this->role != role){
        this->role = role;
        emit role_changed(role);
    }
}

QString Player::getRole() const{
    return role;
}

const General *Player::getAvatarGeneral() const{
    if(general)
        return general;

    QString general_name = property("avatar").toString();
    if(general_name.isEmpty())
        return NULL;
    return Sanguosha->getGeneral(general_name);
}

void Player::setSocket(QTcpSocket *socket){
    this->socket = socket;
    if(socket){
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(getRequest()));
    }
}

void Player::unicast(const QString &message){
    if(socket){
        socket->write(message.toAscii());
        socket->write("\n");
    }
}

QString Player::reportHeader() const{
    QString name = objectName();
    if(name.isEmpty())
        name = tr("Anonymous");
    return QString("%1[%2] ").arg(name).arg(socket->peerAddress().toString());
}

void Player::getRequest(){
    while(socket->canReadLine()){
        QString request = socket->readLine(1024);
        request.chop(1); // remove the ending '\n' character
        emit request_got(request);
    }
}
