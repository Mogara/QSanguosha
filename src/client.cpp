#include "client.h"
#include "settings.h"
#include "engine.h"

#include <QMessageBox>

Client::Client(QObject *parent)
    :QTcpSocket(parent), room(new QObject(this))
{
    self = new Player(this);
    self->setObjectName(Config.UserName);
    self->setProperty("avatar", Config.UserAvatar);

    connectToHost(Config.HostAddress, Config.Port);

    connect(this, SIGNAL(readyRead()), this, SLOT(processReply()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(raiseError(QAbstractSocket::SocketError)));
}

const Player *Client::getPlayer() const{
    return self;
}

void Client::request(const QString &message){
    write(message.toAscii());
    write("\n");
}

void Client::signup(){
    request(QString("signup %1 %2").arg(Config.UserName).arg(Config.UserAvatar));
}

void Client::processReply(){
    static QChar self_prefix('.');
    static QChar room_prefix('$');
    static QChar other_prefix('#');
    static QChar method_prefix('!');

    while(canReadLine()){
        QString reply = readLine(1024);
        reply.chop(1);
        QStringList words = reply.split(QChar(' '));

        QString object = words[0];        
        const char *field = words[1].toAscii();
        QString value = words[2];

        if(object.startsWith(self_prefix)){
            // client it self
            if(self)
                self->setProperty(field, value);
        }else if(object.startsWith(room_prefix)){
            // room
            room->setProperty(field, value);
        }else if(object.startsWith(other_prefix)){
            // others
            object.remove(other_prefix);
            Player *player = findChild<Player*>(object);
            player->setProperty(field, value);
        }else if(object.startsWith(method_prefix)){
            // invoke methods            
            QMetaObject::invokeMethod(this, field, Qt::DirectConnection, Q_ARG(QString, value));
        }else
            QMessageBox::information(NULL, tr("Reply format error!"), reply);
    }
}

void Client::raiseError(QAbstractSocket::SocketError socket_error){
    // translate error message
    QString reason;
    switch(socket_error){
    case ConnectionRefusedError: reason = tr("Connection was refused or timeout"); break;
    case RemoteHostClosedError: reason = tr("Remote host close this connection"); break;
        // FIXME
    default: reason = tr("Unknow error"); break;
    }

    emit error_message(tr("Connection failed, error code = %1\n reason:\n %2").arg(socket_error).arg(reason));
}

void Client::addPlayer(const QString &player_info){
    QStringList words = player_info.split(":");
    if(words.length() >=2){
        Player *player = new Player(this);
        QString name = words[0];
        QString avatar = words[1];
        player->setObjectName(name);
        player->setProperty("avatar", avatar);

        emit player_added(player);
    }
}

void Client::removePlayer(const QString &player_name){
    Player *player = findChild<Player*>(player_name);
    if(player){
        player->setParent(NULL);
        emit player_removed(player_name);
    }
}

void Client::drawCards(const QString &card_str){
    QList<Card*> cards;
    QStringList card_list = card_str.split("+");
    foreach(QString card_str, card_list){
        int card_id = card_str.toInt();
        Card *card = Sanguosha->getCard(card_id);
        cards << card;        
    }

    emit cards_drawed(cards);
}
