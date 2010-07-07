#include "player.h"
#include "engine.h"

#include <QHostAddress>

Player::Player(QObject *parent)
    :QObject(parent), general(NULL), hp(-1), max_hp(-1), state("online"), handcard_num(0), socket(NULL)
{
}

void Player::setHp(int hp){
    if(hp >= 0 && hp <= max_hp)
        this->hp = hp;
}

int Player::getHp() const{
    return hp;
}

int Player::getMaxHP() const{
    return max_hp;
}

void Player::setMaxHP(int max_hp){
    this->max_hp = max_hp;
}

bool Player::isWounded() const{
    if(hp < 0)
        return true;
    else
        return hp < general->getMaxHp();
}

int Player::getGeneralMaxHP() const{
    Q_ASSERT(general != NULL);
    return general->getMaxHp();
}

void Player::setGeneral(const QString &general_name){
    const General *new_general = Sanguosha->getGeneral(general_name);

    if(this->general != new_general){
        this->general = new_general;
        if(new_general)
            setHp(getMaxHP());        

        emit general_changed();
    }
}

QString Player::getGeneral() const{
    if(general)
        return general->objectName();
    else
        return "";
}

QString Player::getState() const{
    return state;
}

void Player::setState(const QString &state){
    if(this->state != state){
        this->state = state;
        emit state_changed(state);
    }
}

int Player::getHandcardNum() const{
    if(handcards.isEmpty())
        return handcard_num;
    else
        return handcards.length();
}

void Player::drawCard(const Card *card){
    handcards << card;
}

void Player::drawCard(int card_num){
    handcard_num += card_num;
    emit handcard_num_changed(handcard_num);
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

        qDebug("%s: %s", objectName().toAscii().data(), message.toAscii().data());
    }
}

QString Player::reportHeader() const{
    QString name = objectName();
    if(name.isEmpty())
        name = tr("Anonymous");
    return QString("%1[%2] ").arg(name).arg(socket->peerAddress().toString());
}

void Player::sendProperty(const char *property_name){
    QString value = property(property_name).toString();
    unicast(QString("#%1 %2 %3").arg(objectName()).arg(property_name).arg(value));
}

void Player::getRequest(){
    while(socket->canReadLine()){
        QString request = socket->readLine(1024);
        request.chop(1); // remove the ending '\n' character
        emit request_got(request);
    }
}
