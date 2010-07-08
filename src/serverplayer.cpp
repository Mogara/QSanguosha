#include "serverplayer.h"

#include <QHostAddress>

ServerPlayer::ServerPlayer(QObject *parent)
    : Player(parent), socket(NULL)
{
}

void ServerPlayer::drawCard(const Card *card){
    handcards << card;
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

        qDebug("%s: %s", objectName().toAscii().data(), message.toAscii().data());
    }
}

QString ServerPlayer::reportHeader() const{
    QString name = objectName();
    if(name.isEmpty())
        name = tr("Anonymous");
    return QString("%1[%2] ").arg(name).arg(socket->peerAddress().toString());
}

void ServerPlayer::sendProperty(const char *property_name){
    QString value = property(property_name).toString();
    unicast(QString("#%1 %2 %3").arg(objectName()).arg(property_name).arg(value));
}

void ServerPlayer::getRequest(){
    while(socket->canReadLine()){
        QString request = socket->readLine(1024);
        request.chop(1); // remove the ending '\n' character
        emit request_got(request);
    }
}
