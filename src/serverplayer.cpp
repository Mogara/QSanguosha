#include "serverplayer.h"
#include "skill.h"
#include "engine.h"

#include <QHostAddress>

ServerPlayer::ServerPlayer(Room *room)
    : Player(room), socket(NULL), room(room)
{
}

void ServerPlayer::drawCard(const Card *card){
    handcards << card;
}

Room *ServerPlayer::getRoom() const{
    return room;
}

int ServerPlayer::aliveCount() const{
    return room->alivePlayerCount();
}

int ServerPlayer::getHandcardNum() const{
    return handcards.length();
}

void ServerPlayer::setSocket(QTcpSocket *socket){
    this->socket = socket;
    if(socket){
        connect(socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
        connect(socket, SIGNAL(readyRead()), this, SLOT(getRequest()));
    }
}

void ServerPlayer::unicast(const QString &message){
    if(socket){
        socket->write(message.toAscii());
        socket->write("\n");

#ifndef QT_NO_DEBUG
        qDebug("%s: %s", qPrintable(objectName()), qPrintable(message));
#endif
    }
}

void ServerPlayer::invoke(const char *method, const QString &arg){
    unicast(QString("! %1 %2").arg(method).arg(arg));
}

QString ServerPlayer::reportHeader() const{
    QString name = objectName();
    if(name.isEmpty())
        name = tr("Anonymous");
    return QString("%1[%2] ").arg(name).arg(socket->peerAddress().toString());
}

void ServerPlayer::sendProperty(const char *property_name){
    QString value = property(property_name).toString();
    unicast(QString(". %1 %2").arg(property_name).arg(value));
}

void ServerPlayer::getRequest(){
    while(socket->canReadLine()){
        QString request = socket->readLine(1024);
        request.chop(1); // remove the ending '\n' character
        emit request_got(request);
    }
}

void ServerPlayer::removeCard(const Card *card, Place place){
    switch(place){
    case Hand: handcards.removeOne(card); break;
    case Equip: removeEquip(card); break;
    default:
        // FIXME
        ;
    }
}

void ServerPlayer::addCard(const Card *card, Place place){
    switch(place){
    case Hand: handcards << card; break;
    case Equip: setEquip(card); break;
    default:
        // FIXME
        ;
    }
}

void ServerPlayer::MoveCard(const CardMoveStruct &move){
    const Card *card = Sanguosha->getCard(move.card_id);
    if(move.from)
        move.from->removeCard(card, move.from_place);
    else{
        if(move.to){
            Room *room = move.to->getRoom();
            room->getDiscardPile()->removeOne(move.card_id);
        }
    }

    if(move.to){
        move.to->addCard(card, move.to_place);        
    }else{
        if(move.from){
            Room *room = move.from->getRoom();
            room->getDiscardPile()->prepend(move.card_id);
        }
    }
}
