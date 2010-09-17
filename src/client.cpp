#include "client.h"
#include "settings.h"
#include "engine.h"
#include "nullificationdialog.h"
#include "playercarddialog.h"
#include "standard.h"

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
    :QTcpSocket(parent), refusable(true), room(new QObject(this)), status(NotActive), alive_count(1),
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

    callbacks["moveNCards"] = &Client::moveNCards;
    callbacks["moveCard"] = &Client::moveCard;

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

    callbacks["fillAG"] = &Client::fillAG;
    callbacks["askForAG"] = &Client::askForAG;
    callbacks["takeAG"] = &Client::takeAG;
    callbacks["clearAG"] = &Client::clearAG;

    callbacks["attachSkill"] = &Client::attachSkill;
    callbacks["detachSkill"] = &Client::detachSkill;
}

void Client::request(const QString &message){
    write(message.toAscii());
    write("\n");
}

void Client::signup(){
    request(QString("signup %1:%2").arg(Config.UserName).arg(Config.UserAvatar));
}

void Client::processReply(){
    static QChar Self_prefix('.');
    static QChar room_prefix('$');
    static QChar other_prefix('#');
    static QChar method_prefix('!');

    while(canReadLine()){
        QString reply = readLine(1024);
        reply.chop(1);
        QStringList words = reply.split(QChar(' '));

        QString object = words[0];        
        const char *field = words[1].toStdString().c_str();
        QString value = words[2];

        if(object.startsWith(Self_prefix)){
            // client it Self
            if(Self)
                Self->setProperty(field, value);
        }else if(object.startsWith(room_prefix)){
            // room
            room->setProperty(field, value);
        }else if(object.startsWith(other_prefix)){
            // others
            object.remove(other_prefix);
            ClientPlayer *player = findChild<ClientPlayer*>(object);
            if(player){
                player->setProperty(field, value);
            }else
                QMessageBox::warning(NULL, tr("Warning"), tr("There is no player named %1").arg(object));

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
        ClientPlayer::MoveCard(move);
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

void Client::startGame(const QString &){
    QList<ClientPlayer *> players = findChildren<ClientPlayer *>();
    alive_count = players.count();
    emit status_changed(NotActive);
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
        QMessageBox *box = new QMessageBox;
        box->setIcon(QMessageBox::Question);
        QString name = Sanguosha->translate(skill_name);
        box->setWindowTitle(tr("Ask for skill invoke"));
        box->setText(tr("Do you want to invoke skill [%1] ?").arg(name));

        QCommandLinkButton *yes_button = new QCommandLinkButton(box);
        yes_button->setObjectName("yes");
        yes_button->setText(tr("Yes"));
        yes_button->setDescription(Sanguosha->translate(QString("%1:yes").arg(skill_name)));
        box->addButton(yes_button, QMessageBox::AcceptRole);

        QCommandLinkButton *no_button = new QCommandLinkButton(box);
        no_button->setObjectName("no");
        no_button->setText(tr("No"));
        no_button->setDescription(tr("Nothing"));
        box->addButton(no_button, QMessageBox::AcceptRole);

        box->exec();

        QString result = box->clickedButton()->objectName();
        request("invokeSkill " + result);
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
        // int msec = qrand() % 1000 + 1000;
        // QTimer::singleShot(msec, this, SLOT(replyNullification()));
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
    QRegExp pattern("(\\w+):([MF])");
    if(!pattern.exactMatch(play_str))
        return;

    QStringList texts = pattern.capturedTexts();
    QString card_name = texts.at(1);
    bool is_male = texts.at(2) == "M";

    Sanguosha->playCardEffect(card_name, is_male);
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
        request("responseCardWithTargets .");
    }else{
        QStringList target_names;
        foreach(const ClientPlayer *target, targets)
            target_names << target->objectName();
        request(QString("responseCardWithTargets %1->%2").arg(card->toString()).arg(target_names.join("+")));
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
    static QChar option_symbol('?');
    if(discard_str.contains(option_symbol)){
        QString copy = discard_str;
        copy.remove(option_symbol);
        discard_num = copy.toInt();
        refusable = true;
    }else{
        discard_num = discard_str.toInt();
        refusable = false;
    }

    emit prompt_changed(tr("Please discard %1 card(s)").arg(discard_num));

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

void Client::replyYiji(const Card *card, const ClientPlayer *to){
    if(card)
        request(QString("replyYiji %1->%2").arg(card->subcardString()).arg(to->objectName()));
    else
        request("replyYiji .");

    setStatus(NotActive);
}
