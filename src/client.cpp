#include "client.h"
#include "settings.h"
#include "engine.h"
#include "nullificationdialog.h"
#include "playercarddialog.h"
#include "standard.h"
#include "optionbutton.h"

#include <QMessageBox>
#include <QCheckBox>
#include <QCommandLinkButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QLineEdit>

bool CardMoveStructForClient::parse(const QString &str){
    static QMap<QString, Player::Place> place_map;
    if(place_map.isEmpty()){
        place_map["hand"] = Player::Hand;
        place_map["equip"] = Player::Equip;
        place_map["judging"] = Player::Judging;
        place_map["special"] = Player::Special;
        place_map["_"] = Player::DiscardedPile;
        place_map["="] = Player::DrawPile;
    }

    // example: 12:tenshi@equip->moligaloo@hand
    QRegExp pattern("(-?\\d+):(.+)@(.+)->(.+)@(.+)");
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
    :QTcpSocket(parent), refusable(true), status(NotActive), alive_count(1),
    nullification_dialog(NULL)
{
    ClientInstance = this;

    Self = new ClientPlayer(this);
    Self->setObjectName(Config.UserName);
    Self->setProperty("avatar", Config.UserAvatar);

    connect(Self, SIGNAL(turn_started()), this, SLOT(clearTurnTag()));

    connectToHost(Config.HostAddress, Config.Port);

    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(notifyRoleChange(QString)));
    connect(this, SIGNAL(readyRead()), this, SLOT(processReply()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(raiseError(QAbstractSocket::SocketError)));

    callbacks["checkVersion"] = &Client::checkVersion;
    callbacks["setPlayerCount"] = &Client::setPlayerCount;
    callbacks["addPlayer"] = &Client::addPlayer;
    callbacks["removePlayer"] = &Client::removePlayer;
    callbacks["drawCards"] = &Client::drawCards;
    callbacks["drawNCards"] = &Client::drawNCards;
    callbacks["getGenerals"] = &Client::getGenerals;
    callbacks["startInXs"] = &Client::startInXs;
    callbacks["duplicationError"] = &Client::duplicationError;
    callbacks["arrangeSeats"] = &Client::arrangeSeats;    
    callbacks["activate"] = &Client::activate;
    callbacks["startGame"] = &Client::startGame;
    callbacks["hpChange"] = &Client::hpChange;    
    callbacks["playSkillEffect"] = &Client::playSkillEffect;    
    callbacks["closeNullification"] = &Client::closeNullification;    
    callbacks["playCardEffect"] = &Client::playCardEffect;
    callbacks["prompt"] = &Client::prompt;
    callbacks["clearPile"] = &Client::clearPile;
    callbacks["setPileNumber"] = &Client::setPileNumber;    
    callbacks["gameOver"] = &Client::gameOver;
    callbacks["killPlayer"] = &Client::killPlayer;
    callbacks["gameOverWarn"] = &Client::gameOverWarn;
    callbacks["showCard"] = &Client::showCard;
    callbacks["setMark"] = &Client::setMark;
    callbacks["doGuanxing"] = &Client::doGuanxing;
    callbacks["doGongxin"] = &Client::doGongxin;
    callbacks["log"] = &Client::log;
    callbacks["speak"] = &Client::speak;
    callbacks["increaseSlashCount"] = &Client::increaseSlashCount;

    callbacks["moveNCards"] = &Client::moveNCards;
    callbacks["moveCard"] = &Client::moveCard;
    callbacks["moveCardToDrawPile"] = &Client::moveCardToDrawPile;

    callbacks["askForDiscard"] = &Client::askForDiscard;
    callbacks["askForSuit"] = &Client::askForSuit;
    callbacks["askForSinglePeach"] = &Client::askForSinglePeach;
    callbacks["askForCardChosen"] = &Client::askForCardChosen;
    callbacks["askForCard"] = &Client::askForCard;
    callbacks["askForSkillInvoke"] = &Client::askForSkillInvoke;
    callbacks["askForChoice"] = &Client::askForChoice;
    callbacks["askForNullification"] = &Client::askForNullification;
    callbacks["askForCardShow"] = &Client::askForCardShow;
    callbacks["askForPindian"] = &Client::askForPindian;
    callbacks["askForYiji"] = &Client::askForYiji;
    callbacks["askForPlayerChosen"] = &Client::askForPlayerChosen;

    callbacks["fillAG"] = &Client::fillAG;
    callbacks["askForAG"] = &Client::askForAG;
    callbacks["takeAG"] = &Client::takeAG;
    callbacks["clearAG"] = &Client::clearAG;

    callbacks["attachSkill"] = &Client::attachSkill;
    callbacks["detachSkill"] = &Client::detachSkill;

    request(QString("signup %1:%2").arg(Config.UserName).arg(Config.UserAvatar));
}

void Client::checkVersion(const QString &server_version){
    QString client_version = Sanguosha->getVersion();

    if(server_version == client_version)
        return;

    static QString link = "http://code.google.com/p/q-sanguosha/downloads/list";
    QString text = tr("Server version is %1, client version is %2 <br/>").arg(server_version).arg(client_version);
    if(server_version > client_version)
        text.append(tr("Your client version is older than the server's, please update it <br/>"));
    else
        text.append(tr("The server version is older than your client version, please ask the server to update<br/>"));

    text.append(tr("Download link : <a href='%1'>%1</a> <br/>").arg(link));
    QMessageBox::warning(NULL, tr("Warning"), text);

    disconnectFromHost();
}

void Client::setPlayerCount(const QString &count_str){
    if(state() != ConnectedState)
        return;

    int count = count_str.toInt();

    Config.PlayerCount = count;
    Config.setValue("PlayerCount", count);

    emit server_connected();
}

void Client::request(const QString &message){
    write(message.toAscii());
    write("\n");
}

void Client::processReply(){
    static char self_prefix = '.';
    static char other_prefix = '#';

    typedef char buffer_t[1024];

    while(canReadLine()){
        buffer_t buffer;
        readLine(buffer, sizeof(buffer));

        if(buffer[0] == self_prefix){
            // client it Self
            if(Self){
                buffer_t property, value;
                sscanf(buffer, ".%s %s", property, value);
                Self->setProperty(property, value);
            }
        }else if(buffer[0] == other_prefix){
            // others
            buffer_t object_name, property, value;
            sscanf(buffer, "#%s %s %s", object_name, property, value);
            ClientPlayer *player = findChild<ClientPlayer*>(object_name);
            if(player){
                bool declared = player->setProperty(property, value);
                if(!declared){
                    QMessageBox::warning(NULL, tr("Warning"), tr("There is no such property named %1").arg(property));
                }
            }else
                QMessageBox::warning(NULL, tr("Warning"), tr("There is no player named %1").arg(object_name));

        }else{
            // invoke methods
            buffer_t method_name, arg;
            sscanf(buffer, "%s %s", method_name, arg);
            Callback callback = callbacks.value(method_name, NULL);
            if(callback)
                (this->*callback)(arg);
            else
                QMessageBox::information(NULL, tr("Warning"), tr("No such invokable method named %1").arg(method_name));
        }
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
    case NetworkError: reason = tr("Server's' firewall blocked the connection or the network cable was plugged out"); break;
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
        Self->addCard(card, Player::Hand);
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
        player->handCardChange(n);
        emit n_cards_drawed(player, n);
    }
}

void Client::getGenerals(const QString &generals_str){
    QStringList generals_list = generals_str.split("+");
    QList<const General *> generals;
    foreach(QString general_name, generals_list){
        const General *general = Sanguosha->getGeneral(general_name);
        generals << general;
    }

    emit generals_got(generals);
}

void Client::itemChosen(const QString &item_name){
    if(!item_name.isEmpty())
        request("choose " + item_name);
}

#ifndef QT_NO_DEBUG

void Client::cheatChoose(){
    QLineEdit *cheat_edit = qobject_cast<QLineEdit*>(sender());
    if(cheat_edit){
        itemChosen(cheat_edit->text());
    }
}

void Client::requestCard(int card_id){
    request(QString("useCard @CheatCard=%1->.").arg(card_id));
}

#endif

void Client::useCard(const Card *card, const QList<const ClientPlayer *> &targets){
    if(!card){
        request("useCard .");
        setStatus(NotActive);
        return;
    }

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
    if(!card){
        request("useCard .");
        setStatus(NotActive);
        return;
    }

    request(QString("useCard %1->.").arg(card->toString()));
    card->use(QList<const ClientPlayer *>());

    setStatus(NotActive);
}

void Client::startInXs(const QString &left_seconds){
    int seconds = left_seconds.toInt();
    emit message_changed(tr("Game will start in %1 seconds").arg(left_seconds));

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
    players.clear();

    int i;
    for(i=0; i<player_names.length(); i++){
        ClientPlayer *player = findChild<ClientPlayer*>(player_names.at(i));

        Q_ASSERT(player != NULL);

        player->setSeat(i+1);
        players << player;        
    }

    QList<const ClientPlayer*> seats;
    int self_index = players.indexOf(Self);

    Q_ASSERT(self_index != -1);

    for(i=self_index+1; i<players.length(); i++)
        seats.prepend(players.at(i));
    for(i=0; i<self_index; i++)
        seats.prepend(players.at(i));

    Q_ASSERT(seats.length() == players.length()-1);

    emit seats_arranged(seats);
}

void Client::notifyRoleChange(const QString &new_role){
    if(!new_role.isEmpty()){
        QString prompt_str = tr("Your role is %1").arg(Sanguosha->translate(new_role));
        if(new_role != "lord")
            prompt_str += tr("\n wait for the lord player choosing general, please");
        emit message_changed(prompt_str);
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
        const Card *card = Sanguosha->getCard(move.card_id);
        if(move.from)
            move.from->removeCard(card, move.from_place);
        else{
            if(move.from_place == Player::DiscardedPile)
                ClientInstance->discarded_list.removeOne(card);
        }

        if(move.to)
            move.to->addCard(card, move.to_place);
        else{
            if(move.to_place == Player::DiscardedPile)
                ClientInstance->discarded_list.prepend(card);
        }
        emit card_moved(move);
    }else{
        QMessageBox::warning(NULL, tr("Warning"), tr("Card moving response string is not well formatted"));
    }
}

void Client::moveNCards(const QString &move_str){
    QRegExp rx("(\\d+):(\\w+)->(\\w+)");
    if(rx.exactMatch(move_str)){
        QStringList texts = rx.capturedTexts();
        int n = texts.at(1).toInt();
        QString from = texts.at(2);
        QString to = texts.at(3);

        ClientPlayer *src = findChild<ClientPlayer *>(from);
        ClientPlayer *dest = findChild<ClientPlayer *>(to);

        src->handCardChange(-n);
        dest->handCardChange(n);

        emit n_cards_moved(n, from, to);
    }else{
        QMessageBox::warning(NULL, tr("Warning"), tr("moveNCards string is not well formatted!"));
    }
}

void Client::moveCardToDrawPile(const QString &from){
    ClientPlayer *src = findChild<ClientPlayer *>(from);

    Q_ASSERT(src);

    src->handCardChange(-1);

    emit card_moved_to_draw_pile(from);
}

void Client::startGame(const QString &){
    QList<ClientPlayer *> players = findChildren<ClientPlayer *>();
    alive_count = players.count();
    emit game_started();
}

void Client::hpChange(const QString &change_str){
    QRegExp rx("(.+):(-?\\d+)");

    if(!rx.exactMatch(change_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString who = texts.at(1);
    int delta = texts.at(2).toInt();

    emit hp_changed(who, delta);
}

void Client::setStatus(Status status){
    if(this->status != status){
        this->status = status;
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

void Client::askForCard(const QString &request_str){
    QStringList texts = request_str.split(":");
    QString pattern = texts.first();

    card_pattern = pattern;
    QString prompt = Sanguosha->translate(texts.at(1));
    if(texts.length() >= 3){
        QString src = Sanguosha->translate(texts.at(2));
        prompt.replace("%src", src);
    }

    if(texts.length() >= 4){
        QString dest = Sanguosha->translate(texts.at(3));
        prompt.replace("%dest", dest);
    }

    if(texts.length() >= 5){
        QString arg = Sanguosha->translate(texts.at(4));
        prompt.replace("%arg", arg);
    }

    emit prompt_changed(prompt);
    if(pattern.endsWith("!"))
        refusable = false;
    else
        refusable = true;
    setStatus(Responsing);
}

void Client::askForSkillInvoke(const QString &skill_name){
    bool auto_invoke = frequent_flags.contains(skill_name);
    if(auto_invoke)
        request("invokeSkill yes");
    else{
        QString name = Sanguosha->translate(skill_name);
        QString title = tr("Ask for skill invoke");
        QString description = Sanguosha->translate(QString("%1:yes").arg(skill_name));
        QString text = tr("Do you want to invoke skill [%1] ?, if you choose yes, then %2").arg(name).arg(description);

        QMessageBox::StandardButton button = QMessageBox::question(NULL, title, text, QMessageBox::Ok | QMessageBox::No, QMessageBox::Ok);
        if(button == QMessageBox::Ok)
            request("invokeSkill yes");
        else
            request("invokeSkill no");
    }
}

void Client::askForChoice(const QString &ask_str){
    QRegExp pattern("(\\w+):(.+)");
    pattern.indexIn(ask_str);
    QStringList words = pattern.capturedTexts();
    QString skill_name = words.at(1);    

    QMessageBox *box = new QMessageBox;
    box->setIcon(QMessageBox::Question);
    box->setWindowTitle(Sanguosha->translate(skill_name));
    box->setText(Sanguosha->translate(QString(":%1:").arg(skill_name)));

    QStringList choices = words.at(2).split("+");
    foreach(QString choice, choices){
        QCommandLinkButton *button = new QCommandLinkButton(box);
        button->setObjectName(choice);
        QString choice_str = QString("%1:%2").arg(skill_name).arg(choice);
        button->setText(Sanguosha->translate(choice_str));

        box->addButton(button, QMessageBox::AcceptRole);
    }

    box->exec();

    QString choice = box->clickedButton()->objectName();
    request("selectChoice " + choice);
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
    delete nullification_dialog;
    nullification_dialog = NULL;
}

void Client::askForNullification(const QString &ask_str){
    QList<int> card_ids = Self->nullifications();
    if(card_ids.isEmpty()){
        replyNullification(-1);
        return;
    }

    QRegExp pattern("(\\w+):(.+)->(.+)");
    pattern.indexIn(ask_str);
    QStringList texts = pattern.capturedTexts();
    QString trick_name = texts.at(1);
    QString source_name = texts.at(2);
    ClientPlayer *source = NULL;
    if(source_name != ".")
        source = findChild<ClientPlayer *>(source_name);
    ClientPlayer *target = findChild<ClientPlayer *>(texts.at(3));

    if(Config.NeverNullifyMyTrick && source == Self){
        const TrickCard *trick_card = Sanguosha->findChild<const TrickCard *>(trick_name);
        if(trick_card->inherits("SingleTargetTrick")){
            replyNullification(-1);
            return;
        }
    }

    nullification_dialog = new NullificationDialog(trick_name, source, target, card_ids);
    nullification_dialog->exec();
}

void Client::closeNullification(const QString &){
    if(nullification_dialog)
        nullification_dialog->accept();
}

void Client::askForCardChosen(const QString &ask_str){
    QRegExp pattern("(.+):([hej]+:(\\w+))");
    pattern.indexIn(ask_str);
    QStringList texts = pattern.capturedTexts();
    QString player_name = texts.at(1);
    ClientPlayer *player = findChild<ClientPlayer *>(player_name);
    QString flags = texts.at(2);
    QString reason = texts.at(3);

    PlayerCardDialog *dialog = new PlayerCardDialog(player, flags);
    dialog->setWindowTitle(Sanguosha->translate(reason));

    connect(dialog, SIGNAL(card_id_chosen(int)), this, SLOT(chooseCard(int)));
    connect(dialog, SIGNAL(rejected()), this, SLOT(chooseCard()));

    dialog->exec();
}

void Client::playCardEffect(const QString &play_str){
    static QRegExp rx1("(\\w+):([MF])");
    static QRegExp rx2("(\\w+)@(\\w+):([MF])");

    if(rx1.exactMatch(play_str)){
        QStringList texts = rx1.capturedTexts();
        QString card_name = texts.at(1);
        bool is_male = texts.at(2) == "M";

        Sanguosha->playCardEffect(card_name, is_male);
    }else if(rx2.exactMatch(play_str)){
        QStringList texts = rx2.capturedTexts();
        QString card_name = texts.at(1);
        QString package_name = texts.at(2);
        bool is_male = texts.at(3) == "M";

        Sanguosha->playCardEffect(card_name, package_name, is_male);
    }
}

void Client::chooseCard(int card_id){
    QDialog *dialog = qobject_cast<QDialog *>(sender());

    if(!dialog)
        return;

    if(card_id == -2){
        dialog->show();
    }else{
        dialog->accept();
        delete dialog;

        request(QString("chooseCard %1").arg(card_id));
    }
}

void Client::choosePlayer(){
    if(sender()->inherits("QDialog")){
        QDialog *dialog = qobject_cast<QDialog *>(sender());
        dialog->show();
    }else if(sender()->inherits("OptionButton")){
        QString player_name = sender()->objectName();
        request("choosePlayer " + player_name);
    }
}

void Client::trust(){
    request("trust .");

    setStatus(NotActive);
}

void Client::speakToServer(const QString &text){
    QByteArray data = text.toUtf8().toBase64();
    request(QString("speak %1").arg(QString(data)));
}

void Client::increaseSlashCount(const QString &){
    // increase slash count
    int slash_count = turn_tag.value("slash_count", 0).toInt();
    turn_tag.insert("slash_count", slash_count + 1);
}

int Client::alivePlayerCount() const{
    return alive_count;
}

void Client::responseCard(const Card *card){
    if(card)
        request(QString("responseCard %1").arg(card->toString()));
    else
        request("responseCard .");

    card_pattern.clear();
    setStatus(NotActive);
}

void Client::responseCard(const Card *card, const QList<const ClientPlayer *> &targets){
    if(card == NULL){
        request("useCard .");
    }else{
        QStringList target_names;
        foreach(const ClientPlayer *target, targets)
            target_names << target->objectName();
        request(QString("useCard %1->%2").arg(card->toString()).arg(target_names.join("+")));
    }

    card_pattern.clear();
    setStatus(NotActive);
}

bool Client::noTargetResponsing() const{
    return status == Responsing && !card_pattern.startsWith("@@");
}

void Client::prompt(const QString &prompt_str){
    // translate the prompt string

    emit prompt_changed(prompt_str);
}

void Client::clearPile(const QString &){
    discarded_list.clear();

    emit pile_cleared();
}

void Client::setPileNumber(const QString &pile_num){
    emit pile_num_set(pile_num.toInt());
}

void Client::askForDiscard(const QString &discard_str){
    QRegExp rx("(\\d+)([oe]*)([SCHD]?)");
    if(!rx.exactMatch(discard_str)){
        QMessageBox::warning(NULL, tr("Warning"), tr("Discarding string is not well formatted!"));
        return;
    }

    QStringList texts = rx.capturedTexts();
    discard_num = texts.at(1).toInt();

    QString flag_str = texts.at(2);
    refusable = flag_str.contains("o");
    include_equip = flag_str.contains("e");

    QString suit = texts.at(3);    
    if(suit.isEmpty())
        discard_suit = Card::NoSuit;
    else if(suit == "S")
        discard_suit = Card::Spade;
    else if(suit == "C")
        discard_suit = Card::Club;
    else if(suit == "H")
        discard_suit = Card::Heart;
    else if(suit == "D")
        discard_suit = Card::Diamond;

    QString prompt;
    if(discard_suit != Card::NoSuit){
        QString suit_string = Sanguosha->translate(Card::Suit2String(discard_suit));
        prompt = tr("Please discard a handcard with the same suit of %1").arg(suit_string);
    }else if(include_equip)
        prompt = tr("Please discard %1 card(s), include equip").arg(discard_num);
    else
        prompt = tr("Please discard %1 card(s), only hand cards is allowed").arg(discard_num);

    emit prompt_changed(prompt);

    setStatus(Discarding);    
}

void Client::gameOver(const QString &result_str){
    QRegExp rx("(\\w+):(.+)");
    if(!rx.exactMatch(result_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString winner = texts.at(1);
    QStringList roles = texts.at(2).split("+");

    Q_ASSERT(roles.length() == players.length());

    int i;
    for(i=0; i<roles.length(); i++){
        players.at(i)->setRole(roles.at(i));
    }

    bool victory = false;
    QList<bool> result_list;
    foreach(ClientPlayer *player, players){
        QString role = player->getRole();
        bool result = (role == winner);
        if(winner == "lord" && role == "loyalist")
            result = true;

        result_list << result;

        if(player == Self)
            victory = result;
    }

    emit game_over(victory, result_list);
}

void Client::killPlayer(const QString &player_name){
    alive_count --;

    emit player_killed(player_name);
}

void Client::gameOverWarn(const QString &){
    QMessageBox::warning(NULL, tr("Warning"), tr("Game is over now"));
}

void Client::askForSuit(const QString &){
    QDialog *dialog = new QDialog;
    QVBoxLayout *layout = new QVBoxLayout;

    QStringList suits;
    suits << "spade" << "club" << "heart" << "diamond";

    foreach(QString suit, suits){
        QCommandLinkButton *button = new QCommandLinkButton;
        button->setIcon(QIcon(QString(":/suit/%1.png").arg(suit)));
        button->setText(Sanguosha->translate(suit));
        button->setObjectName(suit);

        layout->addWidget(button);

        connect(button, SIGNAL(clicked()), this, SLOT(chooseSuit()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
    }

    connect(dialog, SIGNAL(rejected()), this, SLOT(chooseSuit()));

    dialog->setObjectName("no_suit");
    dialog->setWindowTitle(tr("Please choose a suit"));
    dialog->setLayout(layout);
    dialog->exec();
}

void Client::setMark(const QString &mark_str){
    QRegExp rx("(\\w+)\\.(\\w+)=(\\d+)");

    if(!rx.exactMatch(mark_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString who = texts.at(1);
    QString mark = texts.at(2);
    int value = texts.at(3).toInt();

    ClientPlayer *player = findChild<ClientPlayer*>(who);
    player->setMark(mark, value);
}

void Client::chooseSuit(){
    request("chooseSuit " + sender()->objectName());
}

void Client::discardCards(const Card *card){
    if(card)
        request(QString("discardCards %1").arg(card->subcardString()));
    else
        request("discardCards .");

    setStatus(NotActive);
}

void Client::fillAG(const QString &cards_str){
    QStringList cards = cards_str.split("+");
    QList<int> card_ids;
    foreach(QString card, cards){
        card_ids << card.toInt();
    }

    emit ag_filled(card_ids);
}

void Client::takeAG(const QString &take_str){
    QRegExp rx("(.+):(\\d+)");
    rx.exactMatch(take_str);

    QStringList words = rx.capturedTexts();
    QString taker_name = words.at(1);
    int card_id = words.at(2).toInt();

    ClientPlayer *taker = NULL;
    if(taker_name != ".")
        taker = findChild<ClientPlayer *>(taker_name);

    const Card *card = Sanguosha->getCard(card_id);
    if(taker)
        taker->addCard(card, Player::Hand);
    else
        discarded_list.prepend(card);

    emit ag_taken(taker, card_id);
}

void Client::clearAG(const QString &){
    emit ag_cleared();
}

void Client::askForSinglePeach(const QString &ask_str){
    QRegExp rx("(.+):(\\d+)");
    rx.exactMatch(ask_str);

    QStringList texts = rx.capturedTexts();
    ClientPlayer *dying = findChild<ClientPlayer *>(texts.at(1));
    int peaches = texts.at(2).toInt();

    if(dying == Self){
        emit prompt_changed(tr("You are dying, please provide %1 peach(es)(or analeptic) to save yourself").arg(peaches));
        card_pattern = "peach+analeptic";
    }else{
        QString dying_general = Sanguosha->translate(dying->getGeneralName());
        emit prompt_changed(tr("%1 is dying, please provide %2 peach(es) to save him").arg(dying_general).arg(peaches));
        card_pattern = "peach";
    }

    refusable = true;
    setStatus(Responsing);
}

void Client::askForCardShow(const QString &requestor){
    QString name = Sanguosha->translate(requestor);
    emit prompt_changed(tr("%1 request you to show one hand card").arg(name));

    card_pattern = "."; // any card can be matched
    refusable = false;
    setStatus(Responsing);    
}

void Client::askForAG(const QString &){
    setStatus(AskForAG);
}

void Client::chooseAG(int card_id){
    request(QString("chooseAG %1").arg(card_id));

    setStatus(NotActive);
}

QList<ClientPlayer*> Client::getPlayers() const{
    return players;
}

void Client::clearTurnTag(){
    turn_tag.clear();
}

void Client::showCard(const QString &show_str){
    QRegExp rx("(.+):(\\d+)");
    if(!rx.exactMatch(show_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString player_name = texts.at(1);
    int card_id = texts.at(2).toInt();

    if(player_name == Config.UserName)
        return;

    ClientPlayer *player = findChild<ClientPlayer *>(player_name);
    if(player){
        player->addKnownHandCard(Sanguosha->getCard(card_id));
        emit card_shown(player_name, card_id);
    }
}

void Client::attachSkill(const QString &skill_name){
    emit skill_attached(skill_name);
}

void Client::detachSkill(const QString &skill_name){
    emit skill_detached(skill_name);
}

void Client::doGuanxing(const QString &guanxing_str){
    QStringList cards = guanxing_str.split("+");
    QList<int> card_ids;
    foreach(QString card, cards)
        card_ids << card.toInt();

    emit guanxing(card_ids);
    setStatus(AskForGuanxing);
}

void Client::doGongxin(const QString &gongxin_str){
    QRegExp rx("(\\w+):(.+)");
    if(!rx.exactMatch(gongxin_str))
        return;

    QStringList texts = rx.capturedTexts();
    ClientPlayer *who = findChild<ClientPlayer *>(texts.at(1));

    QStringList cards = texts.at(2).split("+");
    QList<int> card_ids;
    foreach(QString card, cards)
        card_ids << card.toInt();

    who->setCards(card_ids);

    emit gongxin(card_ids);
    setStatus(AskForGongxin);
}

void Client::replyGongxin(int card_id){
    if(card_id == -1)
        request("replyGongxin .");
    else
        request(QString("replyGongxin %1").arg(card_id));

    setStatus(NotActive);
}

void Client::askForPindian(const QString &ask_str){
    QStringList words = ask_str.split("->");
    QString from = words.at(0);
    // QString to = words.at(1);

    if(from == Self->getGeneralName())
        emit prompt_changed(tr("Please play a card for pindian"));
    else
        emit prompt_changed(tr("%1 ask for you to play a card to pindian").arg(Sanguosha->translate(from)));

    card_pattern = ".";    
    refusable = false;

    setStatus(Responsing);
}

void Client::askForYiji(const QString &card_list){
    card_pattern = card_list;
    setStatus(AskForYiji);
}

void Client::askForPlayerChosen(const QString &ask_str){
    QStringList player_names = ask_str.split("+");

    QList<const ClientPlayer *> players;
    foreach(QString player_name, player_names)
        players << findChild<const ClientPlayer *>(player_name);

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Please choose a player"));

    QHBoxLayout *layout = new QHBoxLayout;
    foreach(const ClientPlayer *player, players){
        QString icon_path = player->getGeneral()->getPixmapPath("big");
        QString caption = Sanguosha->translate(player->getGeneralName());
        OptionButton *button = new OptionButton(icon_path, caption);
        button->setObjectName(player->objectName());

        connect(button, SIGNAL(double_clicked()), this, SLOT(choosePlayer()));
        connect(button, SIGNAL(double_clicked()), dialog, SLOT(accept()));
        layout->addWidget(button);
    }

    connect(dialog, SIGNAL(rejected()), this, SLOT(choosePlayer()));

    dialog->setLayout(layout);
    dialog->exec();
}

void Client::replyYiji(const Card *card, const ClientPlayer *to){
    if(card)
        request(QString("replyYiji %1->%2").arg(card->subcardString()).arg(to->objectName()));
    else
        request("replyYiji .");

    setStatus(NotActive);
}

void Client::replyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards){
    QStringList up_items, down_items;
    foreach(int card_id, up_cards)
        up_items << QString::number(card_id);

    foreach(int card_id, down_cards)
        down_items << QString::number(card_id);

    request(QString("replyGuanxing %1:%2").arg(up_items.join("+")).arg(down_items.join("+")));

    setStatus(NotActive);
}

void Client::log(const QString &log_str){
    emit log_received(log_str);
}

void Client::speak(const QString &speak_data){
    QStringList words = speak_data.split(":");
    QString who = words.at(0);
    QString base64 = words.at(1);

    QByteArray data = QByteArray::fromBase64(base64.toAscii());
    QString text = QString::fromUtf8(data);

    emit words_spoken(who, text);
}
