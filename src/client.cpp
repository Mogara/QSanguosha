#include "client.h"
#include "settings.h"
#include "engine.h"
#include "nullificationdialog.h"
#include "playercarddialog.h"

#include <QMessageBox>
#include <QCheckBox>
#include <QCommandLinkButton>
#include <QTimer>

bool CardMoveStructForClient::parse(const QString &str){
    static QMap<QString, Player::Place> place_map;
    if(place_map.isEmpty()){
        place_map["hand"] = Player::Hand;
        place_map["equip"] = Player::Equip;
        place_map["delayed_trick"] = Player::DelayedTrick;
        place_map["special"] = Player::Special;
        place_map["_"] = Player::DiscardedPile;
    }

    // example: 12:tenshi@equip->moligaloo@hand
    QRegExp pattern("(\\d+):(.+)@(\\w+)->(.+)@(\\w+)");
    if(!pattern.exactMatch(str)){
        return false;
    }

    QStringList words = pattern.capturedTexts();

    card_id = words.at(1).toInt();

    if(words.at(2) == "_")
        from = NULL;
    else
        from = ClientInstance->findChild<ClientPlayer *>(words.at(2));
    from_place = place_map.value(words.at(3), Player::DiscardedPile);

    if(words.at(4) == "_")
        to = NULL;
    else
        to = ClientInstance->findChild<ClientPlayer *>(words.at(4));
    to_place = place_map.value(words.at(5), Player::DiscardedPile);

    return true;
}

Client *ClientInstance = NULL;

Client::Client(QObject *parent)
    :QTcpSocket(parent), room(new QObject(this)), status(NotActive), alive_count(1)
{
    ClientInstance = this;

    self = new ClientPlayer(this);
    self->setObjectName(Config.UserName);
    self->setProperty("avatar", Config.UserAvatar);    

    connectToHost(Config.HostAddress, Config.Port);

    connect(self, SIGNAL(role_changed(QString)), this, SLOT(notifyRoleChange(QString)));
    connect(this, SIGNAL(readyRead()), this, SLOT(processReply()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(raiseError(QAbstractSocket::SocketError)));

    callbacks["addPlayer"] = &Client::addPlayer;
    callbacks["removePlayer"] = &Client::removePlayer;
    callbacks["drawCards"] = &Client::drawCards;
    callbacks["drawNCards"] = &Client::drawNCards;
    callbacks["getLords"] = &Client::getLords;
    callbacks["getGenerals"] = &Client::getGenerals;
    callbacks["startInXs"] = &Client::startInXs;
    callbacks["duplicationError"] = &Client::duplicationError;
    callbacks["arrangeSeats"] = &Client::arrangeSeats;
    callbacks["moveCard"] = &Client::moveCard;
    callbacks["activate"] = &Client::activate;
    callbacks["startGame"] = &Client::startGame;
    callbacks["hpDamage"] = &Client::hpDamage;
    callbacks["hpFlow"] = &Client::hpFlow;
    callbacks["hpRecover"] = &Client::hpRecover;
    callbacks["judge"] = &Client::judge;
    callbacks["requestForCard"] = &Client::requestForCard;
    callbacks["askForSkillInvoke"] = &Client::askForSkillInvoke;
    callbacks["playSkillEffect"] = &Client::playSkillEffect;
    callbacks["askForNullification"] = &Client::askForNullification;
    callbacks["askForCardChosen"] = &Client::askForCardChosen;    
}

const ClientPlayer *Client::getPlayer() const{
    return self;
}

void Client::request(const QString &message){
    write(message.toAscii());
    write("\n");
}

void Client::signup(){
    request(QString("signup %1:%2").arg(Config.UserName).arg(Config.UserAvatar));
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
            Callback callback = callbacks.value(words[1], NULL);
            if(callback)
                (this->*callback)(value);
            else
                QMessageBox::information(NULL, tr("Warning"), tr("No such invokable method named %1").arg(words[1]));

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

        alive_count ++;

        emit player_added(player);
    }
}

void Client::removePlayer(const QString &player_name){
    ClientPlayer *player = findChild<ClientPlayer*>(player_name);
    if(player){
        player->setParent(NULL);

        alive_count--;

        emit player_removed(player_name);
    }
}

void Client::drawCards(const QString &cards_str){
    QList<const Card*> cards;
    QStringList card_list = cards_str.split("+");
    foreach(QString card_str, card_list){
        int card_id = card_str.toInt();
        const Card *card = Sanguosha->getCard(card_id);
        cards << card;
        self->addCard(card, Player::Hand);
    }

    emit cards_drawed(cards);
}

void Client::drawNCards(const QString &draw_str){    
    QRegExp pattern("(\\w+):(\\d+)");
    pattern.indexIn(draw_str);
    QStringList texts = pattern.capturedTexts();
    ClientPlayer *player = findChild<ClientPlayer*>(texts.at(1));
    int n = texts.at(2).toInt();

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
    if(!card)
        return;

    QStringList target_names;
    foreach(const ClientPlayer *target, targets)
        target_names << target->objectName();

    if(target_names.isEmpty())
        request(QString("useCard %1->.").arg(card->toString()));
    else
        request(QString("useCard %1->%2").arg(card->toString()).arg(target_names.join("+")));

    card->use(targets);

    setStatus(NotActive);
}

void Client::useCard(const Card *card){
    if(!card)
        return;

    request(QString("useCard %1->.").arg(card->toString()));
    card->use(QList<const ClientPlayer *>());

    setStatus(NotActive);
}

void Client::startInXs(const QString &left_seconds){
    int seconds = left_seconds.toInt();
    emit prompt_changed(tr("Game will start in %1 seconds").arg(left_seconds));

    if(seconds == 0){
        emit avatars_hiden();
    }
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

void Client::activate(const QString &focus_player){
    if(focus_player == Config.UserName)
        setStatus(Playing);
    else
        setStatus(NotActive);
}

void Client::moveCard(const QString &move_str){
    CardMoveStructForClient move;

    if(move.parse(move_str)){
        ClientPlayer::MoveCard(move);
        emit card_moved(move);
    }else{
        QMessageBox::warning(NULL, tr("Warning"), tr("Card moving response string is not well formatted"));
    }
}

void Client::startGame(const QString &){
    QList<ClientPlayer *> players = findChildren<ClientPlayer *>();
    alive_count = players.count();
    emit status_changed(NotActive);
}

void Client::hpDamage(const QString &who){
    emit damaged(who);
}

void Client::hpFlow(const QString &flow_str){
    // FIXME
}

void Client::hpRecover(const QString &recover_str){
    // FIXME
}

void Client::ackForHpChange(int delta){
    request(QString("ackForHpChange %1").arg(delta));
}

void Client::askForJudge(const QString &player_name){
    if(player_name.isNull())
        request("judge " + self->objectName());
    else
        request("judge " + player_name);
}

void Client::judge(const QString &judge_str){
    QStringList words = judge_str.split(":");
    // QString target = words.at(0);
    // int card_id = words.at(1).toInt();


}

void Client::setStatus(Status status){
    if(this->status != status){
        this->status = status;

        if(status == Playing)
            turn_tag.clear();

        emit status_changed(status);
    }
}

Client::Status Client::getStatus() const{
    return status;
}

void Client::updateFrequentFlags(int state){    
    QString flag = sender()->objectName();
    if(state == Qt::Checked)
        frequent_flags.insert(flag);
    else
        frequent_flags.remove(flag);
}

void Client::requestForCard(const QString &request_str){
    static QSet<QString> patterns;
    if(patterns.isEmpty()){
        patterns << "jink" << "slash";
    }

    if(patterns.contains(request_str)){
        card_pattern = request_str;
        setStatus(Responsing);
    }else{
        QMessageBox::warning(NULL, "", tr("Unknown request card pattern: %1").arg(request_str));
    }
}

void Client::askForSkillInvoke(const QString &ask_str){
    QRegExp pattern("(\\w+):(.+)");
    pattern.indexIn(ask_str);
    QStringList words = pattern.capturedTexts();
    QString skill_name = words.at(1);

    bool auto_invoke = frequent_flags.contains(skill_name);
    if(auto_invoke)
        request("invokeSkill yes");
    else{
        QMessageBox *box = new QMessageBox;
        box->setIcon(QMessageBox::Question);
        QString name = Sanguosha->translate(skill_name);
        box->setWindowTitle(tr("Ask for skill invoke"));
        box->setText(tr("Do you want to invoke skill [%1] ?").arg(name));

        QStringList options = words.at(2).split("+");
        foreach(QString option, options){
            QCommandLinkButton *button = new QCommandLinkButton(box);
            button->setObjectName(option);
            button->setText(Sanguosha->translate(option));
            button->setDescription(Sanguosha->translate(QString("%1:%2").arg(skill_name).arg(option)));

            box->addButton(button, QMessageBox::AcceptRole);
        }

        box->exec();

        QString result = box->clickedButton()->objectName();
        request("invokeSkill " + result);
    }
}

void Client::playSkillEffect(const QString &play_str){
    QRegExp pattern("(\\w+):([-\\w]+)");
    pattern.indexIn(play_str);
    QStringList words = pattern.capturedTexts();
    QString skill_name = words.at(1);
    int index = words.at(2).toInt();

    Sanguosha->playSkillEffect(skill_name, index);
}

void Client::replyNullification(int card_id){
    request(QString("replyNullification %1").arg(card_id));
}

void Client::askForNullification(const QString &ask_str){
    QList<int> card_ids = self->nullifications();
    if(card_ids.isEmpty()){
        int msec = qrand() % 1000 + 1000;
        QTimer::singleShot(msec, this, SLOT(replyNullification()));
        return;
    }

    QRegExp pattern("(\\w+):(.+)->(.+)");
    pattern.indexIn(ask_str);
    QStringList texts = pattern.capturedTexts();
    QString trick_name = texts.at(1);
    ClientPlayer *source = ClientInstance->findChild<ClientPlayer *>(texts.at(2));
    ClientPlayer *target = ClientInstance->findChild<ClientPlayer *>(texts.at(3));

    if(Config.NeverNullifyMyTrick && source == target && source == self)
        replyNullification(-1);
    else{
        NullificationDialog *dialog = new NullificationDialog(trick_name, source, target, card_ids);
        dialog->exec();
    }
}

void Client::askForCardChosen(const QString &ask_str){
    QRegExp pattern("(.+):([hej]+:(\\w+))");
    pattern.indexIn(ask_str);
    QStringList texts = pattern.capturedTexts();
    QString player_name = texts.at(1);
    ClientPlayer *player = findChild<ClientPlayer *>(player_name);
    QString flags = texts.at(2);
    QString trick_name = texts.at(3);

    PlayerCardDialog *dialog = new PlayerCardDialog(player, flags);
    dialog->setWindowTitle(Sanguosha->translate(trick_name));

    connect(dialog, SIGNAL(card_id_chosen(int)), this, SLOT(chooseCard(int)));

    dialog->exec();
}

void Client::chooseCard(int card_id){
    QDialog *dialog = qobject_cast<QDialog *>(sender());
    if(dialog)
        dialog->accept();

    request(QString("chooseCard %1").arg(card_id));
}

int Client::alivePlayerCount() const{
    return alive_count;
}
