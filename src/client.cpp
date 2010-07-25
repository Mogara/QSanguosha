#include "client.h"
#include "settings.h"
#include "engine.h"

#include <QMessageBox>

Client::Client(QObject *parent)
    :QTcpSocket(parent), room(new QObject(this)), focus_player(NULL)
{
    self = new ClientPlayer(this);
    self->setObjectName(Config.UserName);
    self->setProperty("avatar", Config.UserAvatar);

    connect(self, SIGNAL(role_changed(QString)), this, SLOT(notifyRoleChange(QString)));

    connectToHost(Config.HostAddress, Config.Port);

    connect(this, SIGNAL(readyRead()), this, SLOT(processReply()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(raiseError(QAbstractSocket::SocketError)));
}

const ClientPlayer *Client::getPlayer() const{
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
            ClientPlayer *player = findChild<ClientPlayer*>(object);
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
    case HostNotFoundError: reason = tr("Host not found"); break;
    case SocketAccessError: reason = tr("Socket access error"); break;
        // FIXME
    default: reason = tr("Unknow error"); break;
    }

    emit error_message(tr("Connection failed, error code = %1\n reason:\n %2").arg(socket_error).arg(reason));
}

void Client::addPlayer(const QString &player_info){
    QStringList words = player_info.split(":");
    if(words.length() >=2){
        ClientPlayer *player = new ClientPlayer(this);
        QString name = words[0];
        QString avatar = words[1];
        player->setObjectName(name);
        player->setProperty("avatar", avatar);

        emit player_added(player);
    }
}

void Client::removePlayer(const QString &player_name){
    ClientPlayer *player = findChild<ClientPlayer*>(player_name);
    if(player){
        player->setParent(NULL);
        emit player_removed(player_name);
    }
}

void Client::drawCards(const QString &cards_str){
    QList<Card*> cards;
    QStringList card_list = cards_str.split("+");
    foreach(QString card_str, card_list){
        int card_id = card_str.toInt();
        Card *card = Sanguosha->getCard(card_id);
        cards << card;
        self->addCard(card, "hand");
    }

    emit cards_drawed(cards);
}

void Client::drawNCards(const QString &draw_str){
    int colon_index = draw_str.indexOf(QChar(':'));
    ClientPlayer *player = findChild<ClientPlayer *>(draw_str.left(colon_index));
    int n = draw_str.right(draw_str.length() - colon_index - 1).toInt();

    if(player && n>0){
        player->drawNCard(n);
        emit n_card_drawed(player, n);
    }
}

void Client::getLords(const QString &lords_str){
    QStringList lord_list = lords_str.split("+");
    QList<const General *> lords;
    foreach(QString lord_name, lord_list){
        const General *general = Sanguosha->getGeneral(lord_name);
        lords << general;
    }

    emit lords_got(lords);
}

void Client::getGenerals(const QString &generals_str){
    QStringList generals_list = generals_str.split("+");
    QList<const General *> generals;
    foreach(QString general_name, generals_list){
        const General *general = Sanguosha->getGeneral(general_name);
        generals << general;
    }

    const General *lord = generals.takeFirst();
    emit generals_got(lord, generals);
}

void Client::itemChosen(const QString &item_name){
    if(!item_name.isEmpty())
        request("choose " + item_name);
}

void Client::useCard(const Card *card, const QList<const ClientPlayer *> &targets){
    if(card){
        QStringList target_names;
        foreach(const ClientPlayer *target, targets)
            target_names << target->objectName();

        QString target_str = target_names.join("+");
        if(card->isVirtualCard())
            request(QString("useCard %1=%2 %3").arg(card->toString()).arg(card->subcardString()).arg(target_str));
        else
            request(QString("useCard %1 %2").arg(card->getID()).arg(target_str));
    }
}

void Client::useCard(const Card *card){
    if(card){
        if(card->isVirtualCard())
            request(QString("useCard %1 .").arg(card->toString()));
        else
            request(QString("useCard %1 .").arg(card->getID()));
    }
}

void Client::startInXs(const QString &left_seconds){
    emit prompt_changed(tr("Game will start in %1 seconds").arg(left_seconds));
}

void Client::duplicationError(const QString &){
    QMessageBox::critical(NULL, tr("Error"), tr("Name %1 duplication, you've to be offline").arg(Config.UserName));
    disconnectFromHost();
    exit(1);
}

void Client::arrangeSeats(const QString &seats_str){    
    QStringList player_names = seats_str.split("+");
    QList<const ClientPlayer*> players, seats;

    int i;
    for(i=0; i<player_names.length(); i++){
        ClientPlayer *player = findChild<ClientPlayer*>(player_names.at(i));

        Q_ASSERT(player != NULL);

        player->setSeat(i+1);
        players << player;        
    }

    int self_index = players.indexOf(self);
    for(i=self_index+1; i<players.length(); i++)
        seats.prepend(players.at(i));
    for(i=0; i<self_index; i++)
        seats.prepend(players.at(i));

    emit seats_arranged(seats);
}

void Client::notifyRoleChange(const QString &new_role){
    if(!new_role.isEmpty()){
        QString prompt_str = tr("Your role is %1").arg(Sanguosha->translate(new_role));
        if(new_role != "lord")
            prompt_str += tr("\n wait for the lord player choosing general, please");
        emit prompt_changed(prompt_str);
    }
}

void Client::activate(const QString &player_name){
    emit activity_set(self->objectName() == player_name);
}

void Client::moveCard(const QString &move_str){
    int colon_index = move_str.indexOf(QChar(':'));
    int card_id = move_str.left(colon_index).toInt();
    const Card *card = Sanguosha->getCard(card_id);
    QString place_str = move_str.right(move_str.length() - colon_index - 1);
    QStringList places = place_str.split("->");
    QString src = places.at(0);
    QString dest = places.at(1);

    QStringList words = src.split("@");
    QString src_name = words.at(0);
    if(src_name != "_"){
        QString location = words.at(1);
        ClientPlayer *player = findChild<ClientPlayer*>(src_name);
        player->removeCard(card, location);
    }

    words = dest.split("@");
    QString dest_name = words.at(0);
    if(dest_name != "_"){
        QString dest_location = words.at(1);
        ClientPlayer *player = findChild<ClientPlayer*>(dest_name);
        player->addCard(card, dest_location);
    }

    emit card_moved(src, dest, card_id);
}

void Client::requestCard(const QString &request_str){
    // FIXME
    emit card_requested(request_str);
}

void Client::startGame(const QString &first_player){
    // attach basic rule
    Sanguosha->attachBasicRule(self);

    // attach all skills
    QList<ClientPlayer *> players = findChildren<ClientPlayer*>();
    foreach(ClientPlayer *player, players){
        const General *general = Sanguosha->getGeneral(player->getGeneral());
        const QList<const Skill *> skills = general->findChildren<const Skill *>();
        foreach(const Skill *skill, skills){
            skill->attachPlayer(self);
        }
    }

    activate(first_player);
}

void Client::triggerSkill(){
    QList<const Skill *> skills = self->getSkills();
    foreach(const Skill *skill, skills){
        skill->trigger(this);
    }

}
