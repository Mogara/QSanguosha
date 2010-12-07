#include "client.h"
#include "settings.h"
#include "engine.h"
#include "nullificationdialog.h"
#include "playercarddialog.h"
#include "standard.h"
#include "optionbutton.h"
#include "nativesocket.h"
#include "recorder.h"

#include <QMessageBox>
#include <QCheckBox>
#include <QCommandLinkButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTextDocument>

Client *ClientInstance = NULL;

Client::Client(QObject *parent, const QString &filename)
    :QObject(parent), refusable(true), status(NotActive), alive_count(1),
    nullification_dialog(NULL)
{
    ClientInstance = this;    

    callbacks["checkVersion"] = &Client::checkVersion;
    callbacks["setup"] = &Client::setup;
    callbacks["addPlayer"] = &Client::addPlayer;
    callbacks["removePlayer"] = &Client::removePlayer;
    callbacks["startInXs"] = &Client::startInXs;
    callbacks["arrangeSeats"] = &Client::arrangeSeats;
    callbacks["startGame"] = &Client::startGame;
    callbacks["hpChange"] = &Client::hpChange;    
    callbacks["playSkillEffect"] = &Client::playSkillEffect;    
    callbacks["closeNullification"] = &Client::closeNullification;    
    callbacks["playCardEffect"] = &Client::playCardEffect;
    callbacks["clearPile"] = &Client::clearPile;
    callbacks["setPileNumber"] = &Client::setPileNumber;    
    callbacks["gameOver"] = &Client::gameOver;
    callbacks["killPlayer"] = &Client::killPlayer;
    callbacks["gameOverWarn"] = &Client::gameOverWarn;
    callbacks["showCard"] = &Client::showCard;
    callbacks["setMark"] = &Client::setMark;
    callbacks["log"] = &Client::log;
    callbacks["speak"] = &Client::speak;
    callbacks["increaseSlashCount"] = &Client::increaseSlashCount;
    callbacks["attachSkill"] = &Client::attachSkill;
    callbacks["detachSkill"] = &Client::detachSkill;
    callbacks["moveFocus"] = &Client::moveFocus;
    callbacks["setEmotion"] = &Client::setEmotion;
    callbacks["skillInvoked"] = &Client::skillInvoked;
    callbacks["acquireSkill"] = &Client::acquireSkill;
    callbacks["addProhibitSkill"] = &Client::addProhibitSkill;
    callbacks["animate"] = &Client::animate;

    callbacks["moveNCards"] = &Client::moveNCards;
    callbacks["moveCard"] = &Client::moveCard;
    callbacks["drawCards"] = &Client::drawCards;
    callbacks["drawNCards"] = &Client::drawNCards;

    // interactive methods
    callbacks["activate"] = &Client::activate;
    callbacks["doChooseGeneral"] = &Client::doChooseGeneral;
    callbacks["doGuanxing"] = &Client::doGuanxing;
    callbacks["doGongxin"] = &Client::doGongxin;
    callbacks["askForDiscard"] = &Client::askForDiscard;
    callbacks["askForSuit"] = &Client::askForSuit;
    callbacks["askForKingdom"] = &Client::askForKingdom;
    callbacks["askForSinglePeach"] = &Client::askForSinglePeach;
    callbacks["askForCardChosen"] = &Client::askForCardChosen;
    callbacks["askForCard"] = &Client::askForCard;
    callbacks["askForUseCard"] = &Client::askForUseCard;
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

    ask_dialog = NULL;
    use_card = false;

    Self = new ClientPlayer(this);
    Self->setScreenName(Config.UserName);
    Self->setProperty("avatar", Config.UserAvatar);
    connect(Self, SIGNAL(turn_started()), this, SLOT(clearTurnTag()));
    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(notifyRoleChange(QString)));

    if(!filename.isEmpty()){
        socket = NULL;
        recorder = NULL;

        replayer = new Replayer(this, filename);
        connect(replayer, SIGNAL(command_parsed(QString)), this, SLOT(processCommand(QString)));
    }else{
        socket = new NativeClientSocket;

        socket->setParent(this);
        connect(socket, SIGNAL(message_got(char*)), this, SLOT(processReply(char*)));
        connect(socket, SIGNAL(error_message(QString)), this, SIGNAL(error_message(QString)));
        connect(socket, SIGNAL(connected()), this, SLOT(signup()));
        socket->connectToHost();

        recorder = new Recorder(this);
        connect(socket, SIGNAL(message_got(char*)), recorder, SLOT(record(char*)));

        replayer = NULL;
    }

    lines_doc = new QTextDocument(this);
}

void Client::signup(){
    if(replayer)
        replayer->start();
    else{
        QString base64 = Config.UserName.toUtf8().toBase64();
        request(QString("signup %1:%2").arg(base64).arg(Config.UserAvatar));
    }
}

void Client::request(const QString &message){
    if(socket)
        socket->send(message);
}

void Client::checkVersion(const QString &server_version){
    QString client_version = Sanguosha->getVersion();

    if(server_version == client_version)
        return;

    static QString link = "http://github.com/Moligaloo/QSanguosha/downloads";
    QString text = tr("Server version is %1, client version is %2 <br/>").arg(server_version).arg(client_version);
    if(server_version > client_version)
        text.append(tr("Your client version is older than the server's, please update it <br/>"));
    else
        text.append(tr("The server version is older than your client version, please ask the server to update<br/>"));

    text.append(tr("Download link : <a href='%1'>%1</a> <br/>").arg(link));
    QMessageBox::warning(NULL, tr("Warning"), text);

    disconnectFromHost();
}

void Client::setup(const QString &setup_str){
    if(socket && !socket->isConnected())
        return;

    ServerInfo.parse(setup_str);
    emit server_connected();
}

void Client::disconnectFromHost(){
    if(socket){
        socket->disconnectFromHost();
        socket = NULL;
    }
}

typedef char buffer_t[1024];

void Client::processCommand(const QString &cmd){
    processReply(cmd.toAscii().data());
}

void Client::processReply(char *reply){
    if(strlen(reply) <= 2)
        return;

    static char self_prefix = '.';
    static char other_prefix = '#';

    if(reply[0] == self_prefix){
        // client it Self
        if(Self){
            buffer_t property, value;
            sscanf(reply, ".%s %s", property, value);
            Self->setProperty(property, value);
        }
    }else if(reply[0] == other_prefix){
        // others
        buffer_t object_name, property, value;
        sscanf(reply, "#%s %s %s", object_name, property, value);
        ClientPlayer *player = getPlayer(object_name);
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
        sscanf(reply, "%s %s", method_name, arg);
        Callback callback = callbacks.value(method_name, NULL);

        QString method = method_name;
        if(replayer && (method.startsWith("askFor") || method.startsWith("do") || method == "activate"))
            return;

        if(callback){
            QString arg_str = arg;
            (this->*callback)(arg_str);
        }else
            QMessageBox::information(NULL, tr("Warning"), tr("No such invokable method named \"%1\"").arg(method_name));
    }
}

void Client::addPlayer(const QString &player_info){
    QStringList texts = player_info.split(":");
    QString name = texts.at(0);
    QString base64 = texts.at(1);
    QByteArray data = QByteArray::fromBase64(base64.toAscii());
    QString screen_name = QString::fromUtf8(data);
    QString avatar = texts.at(2);

    ClientPlayer *player = new ClientPlayer(this);
    player->setObjectName(name);
    player->setScreenName(screen_name);
    player->setProperty("avatar", avatar);

    alive_count ++;

    emit player_added(player);
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

    pile_num -= cards.length();
    updatePileNum();

    emit cards_drawed(cards);
}

void Client::drawNCards(const QString &draw_str){    
    QRegExp pattern("(\\w+):(\\d+)");
    pattern.indexIn(draw_str);
    QStringList texts = pattern.capturedTexts();
    ClientPlayer *player = findChild<ClientPlayer*>(texts.at(1));
    int n = texts.at(2).toInt();

    if(player && n>0){
        pile_num -= n;
        updatePileNum();

        player->handCardChange(n);
        emit n_cards_drawed(player, n);
    }
}

void Client::doChooseGeneral(const QString &generals_str){
    QStringList generals_list = generals_str.split("+");
    QList<const General *> generals;
    foreach(QString general_name, generals_list){
        const General *general = Sanguosha->getGeneral(general_name);
        generals << general;
    }

    emit generals_got(generals);
}

void Client::chooseItem(const QString &item_name){
    if(!item_name.isEmpty()){
        if(!Self->getGeneral())
            request("choose " + item_name);
        else
            request("choose2 " + item_name);

        Sanguosha->playAudio("choose-item");
    }
}

#ifndef QT_NO_DEBUG

void Client::requestCard(int card_id){
    request(QString("useCard @CheatCard=%1->.").arg(card_id));
}

#endif

void Client::useCard(const Card *card, const QList<const ClientPlayer *> &targets){
    if(card == NULL){
        request("useCard .");
    }else{
        QStringList target_names;
        foreach(const ClientPlayer *target, targets)
            target_names << target->objectName();

        if(target_names.isEmpty())
            request(QString("useCard %1->.").arg(card->toString()));
        else
            request(QString("useCard %1->%2").arg(card->toString()).arg(target_names.join("+")));
    }

    if(status == Playing)
        card->use(targets);
    else if(status == Responsing)
        card_pattern.clear();

    setStatus(NotActive);
}

void Client::useCard(const Card *card){
    if(card){
        request(QString("useCard %1->.").arg(card->toString()));
        card->use(QList<const ClientPlayer *>());
    }else{
        request("useCard .");
    }

    setStatus(NotActive);
}

void Client::startInXs(const QString &left_seconds){
    int seconds = left_seconds.toInt();
    emit message_changed(tr("Game will start in %1 seconds").arg(left_seconds));

    if(seconds == 0){
        emit avatars_hiden();
    }
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
    if(focus_player == Self->objectName())
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
            if(move.from_place == Player::DrawPile)
                pile_num --;
            else if(move.from_place == Player::DiscardedPile)
                ClientInstance->discarded_list.removeOne(card);

            updatePileNum();
        }

        if(move.to)
            move.to->addCard(card, move.to_place);
        else{
            if(move.to_place == Player::DrawPile)
                pile_num ++;
            else if(move.to_place == Player::DiscardedPile)
                ClientInstance->discarded_list.prepend(card);

            updatePileNum();
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

        ClientPlayer *src = getPlayer(from);
        ClientPlayer *dest = getPlayer(to);

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

void Client::askForCardOrUseCard(const QString &request_str){
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

void Client::askForCard(const QString &request_str){
    use_card = request_str.startsWith("@@");
    askForCardOrUseCard(request_str);
}

void Client::askForUseCard(const QString &request_str){
    use_card = true;
    askForCardOrUseCard(request_str);
}

void Client::askForSkillInvoke(const QString &skill_name){
    bool auto_invoke = frequent_flags.contains(skill_name);
    if(auto_invoke){
        invokeSkill(QDialog::Accepted);
        return;
    }


    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(Sanguosha->translate(skill_name));

    QString text = tr("Do you want to invoke skill [%1] ?").arg(dialog->windowTitle());

    QLabel *label = new QLabel(text);

    QCommandLinkButton *ok_button = new QCommandLinkButton(tr("OK"));
    QString description = Sanguosha->translate(QString("%1:yes").arg(skill_name));
    ok_button->setToolTip(description);

    QCommandLinkButton *cancel_button = new QCommandLinkButton(tr("Cancel"));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch();
    hlayout->addWidget(ok_button);
    hlayout->addWidget(cancel_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addLayout(hlayout);
    dialog->setLayout(layout);

    ask_dialog = dialog;

    connect(ok_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cancel_button, SIGNAL(clicked()), dialog, SLOT(reject()));
    connect(dialog, SIGNAL(finished(int)), this, SLOT(invokeSkill(int)));

    setStatus(ExecDialog);
    Sanguosha->playAudio("pop-up");
    dialog->exec();
}

void Client::selectChoice(){
    QString option = sender()->objectName();
    request("selectChoice " + option);
    setStatus(NotActive);
}

void Client::askForChoice(const QString &ask_str){
    QRegExp rx("(\\w+):(.+)");

    if(!rx.exactMatch(ask_str))
        return;

    QStringList words = rx.capturedTexts();
    QString skill_name = words.at(1);
    QStringList options = words.at(2).split("+");

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(Sanguosha->translate(skill_name));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Please choose:")));

    foreach(QString option, options){
        QCommandLinkButton *button = new QCommandLinkButton;
        QString text = QString("%1:%2").arg(skill_name).arg(option);

        button->setObjectName(option);
        button->setText(Sanguosha->translate(text));

        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(button, SIGNAL(clicked()), this, SLOT(selectChoice()));

        layout->addWidget(button);
    }

    dialog->setObjectName(options.first());
    connect(dialog, SIGNAL(rejected()), this, SLOT(selectChoice()));

    dialog->setLayout(layout);

    ask_dialog = dialog;
    setStatus(ExecDialog);

    Sanguosha->playAudio("pop-up");
    dialog->exec();
}

void Client::playSkillEffect(const QString &play_str){
    QRegExp rx("(#?\\w+):([-\\w]+)");
    if(!rx.exactMatch(play_str))
        return;

    QStringList words = rx.capturedTexts();
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
        source = getPlayer(source_name);
    ClientPlayer *target = getPlayer(texts.at(3));

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
    QRegExp rx("(.+):([hej]+):(\\w+)");
    if(!rx.exactMatch(ask_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString player_name = texts.at(1);
    ClientPlayer *player = getPlayer(player_name);
    QString flags = texts.at(2);
    QString reason = texts.at(3);

    PlayerCardDialog *dialog = new PlayerCardDialog(player, flags);
    dialog->setWindowTitle(Sanguosha->translate(reason));

    connect(dialog, SIGNAL(card_id_chosen(int)), this, SLOT(chooseCard(int)));
    connect(dialog, SIGNAL(rejected()), this, SLOT(chooseCard()));

    ask_dialog = dialog;
    setStatus(ExecDialog);

    dialog->exec();
}

void Client::playCardEffect(const QString &play_str){
    static QRegExp rx1("(@?\\w+):([MF])");
    static QRegExp rx2("(\\w+)@(\\w+):([MF])"); // old version

    if(rx1.exactMatch(play_str)){
        QStringList texts = rx1.capturedTexts();
        QString card_name = texts.at(1);
        bool is_male = texts.at(2) == "M";

        Sanguosha->playCardEffect(card_name, is_male);
    }else if(rx2.exactMatch(play_str)){
        QStringList texts = rx2.capturedTexts();
        QString card_name = texts.at(1);        
        bool is_male = texts.at(3) == "M";

        Sanguosha->playCardEffect("@" + card_name, is_male);
    }
}

void Client::chooseCard(int card_id){
    Q_ASSERT(ask_dialog->inherits("PlayerCardDialog"));

    if(card_id == -2){
        request("chooseCard .");
    }else{
        delete ask_dialog;
        ask_dialog = NULL;

        request(QString("chooseCard %1").arg(card_id));
    }

    setStatus(NotActive);
}

void Client::choosePlayer(const ClientPlayer *player){
    if(player == NULL)
        player = findChild<const ClientPlayer *>(players_to_choose.first());

    request("choosePlayer " + player->objectName());
    setStatus(NotActive);
}

void Client::trust(){
    request("trust .");

    if(Self->getState() == "trust")
        Sanguosha->playAudio("untrust");
    else
        Sanguosha->playAudio("trust");

    setStatus(NotActive);
}

void Client::surrender(){
    request("surrender .");

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

void Client::invokeSkill(int result){
    bool invoke = result == QDialog::Accepted;

    if(invoke)
        request("invokeSkill yes");
    else
        request("invokeSkill no");

    setStatus(NotActive);
}

void Client::responseCard(const Card *card){
    if(card)
        request(QString("responseCard %1").arg(card->toString()));
    else
        request("responseCard .");

    card_pattern.clear();
    setStatus(NotActive);
}

bool Client::noTargetResponsing() const{
    return status == Responsing && !use_card;
}

void Client::prompt(const QString &prompt_str){
    // translate the prompt string

    emit prompt_changed(prompt_str);
}

ClientPlayer *Client::getPlayer(const QString &name){
    return findChild<ClientPlayer *>(name);
}

void Client::kick(const QString &to_kick){
    request("kick " + to_kick);
}

bool Client::save(const QString &filename) const{
    if(recorder)
        return recorder->save(filename);
    else
        return false;
}

bool Client::isProhibited(const Player *to, const Card *card) const{
    foreach(const ProhibitSkill *skill, prohibit_skills){
        if(to->hasSkill(skill->objectName()) && skill->isProhibited(Self, to, card))
            return true;
    }

    return false;
}

void Client::setLines(const QString &filename){
    QRegExp rx(".+/(\\w+\\d?).ogg");
    if(rx.exactMatch(filename)){
        QString skill_name = rx.capturedTexts().at(1);
        QString lines = Sanguosha->translate("$" + skill_name);
        QChar last_char = skill_name[skill_name.length()-1];
        if(last_char.isDigit())
            skill_name.chop(1);
        lines = QString("<b>%1</b>: %2").arg(Sanguosha->translate(skill_name)).arg(lines);

        skill_line = lines;

        updatePileNum();
    }
}

QTextDocument *Client::getLinesDoc() const{
    return lines_doc;
}

void Client::clearPile(const QString &){
    discarded_list.clear();

    emit pile_cleared();
}

void Client::setPileNumber(const QString &pile_str){
    pile_num = pile_str.toInt();

    updatePileNum();
}

void Client::updatePileNum(){
    lines_doc->setHtml(tr("Draw pile: %1, discard pile: %2 <br/> %3")
                       .arg(pile_num).arg(discarded_list.length()).arg(skill_line));
}

void Client::askForDiscard(const QString &discard_str){
    QRegExp rx("(\\d+)([oe]*)");
    if(!rx.exactMatch(discard_str)){
        QMessageBox::warning(NULL, tr("Warning"), tr("Discarding string is not well formatted!"));
        return;
    }

    QStringList texts = rx.capturedTexts();
    discard_num = texts.at(1).toInt();

    QString flag_str = texts.at(2);
    refusable = flag_str.contains("o");
    include_equip = flag_str.contains("e");

    QString prompt;
    if(include_equip)
        prompt = tr("Please discard %1 card(s), include equip").arg(discard_num);
    else
        prompt = tr("Please discard %1 card(s), only hand cards is allowed").arg(discard_num);

    emit prompt_changed(prompt);

    setStatus(Discarding);    
}

void Client::gameOver(const QString &result_str){    
    QStringList texts = result_str.split(":");
    QString winner = texts.at(0);
    QStringList roles = texts.at(1).split("+");

    Q_ASSERT(roles.length() == players.length());

    int i;
    for(i=0; i<roles.length(); i++){
        players.at(i)->setRole(roles.at(i));
    }

    if(winner == "."){
        emit standoff();
        return;
    }

    bool victory = false;
    QList<bool> result_list;
    foreach(ClientPlayer *player, players){
        QString role = player->getRole();
        bool result = winner.contains(role);
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
    delete ask_dialog;

    QDialog *dialog = new QDialog;
    ask_dialog = dialog;

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

    dialog->setObjectName(".");
    dialog->setWindowTitle(tr("Please choose a suit"));
    dialog->setLayout(layout);

    setStatus(ExecDialog);
    dialog->exec();
}

void Client::askForKingdom(const QString &){
    delete ask_dialog;

    QDialog *dialog = new QDialog;
    ask_dialog = dialog;

    QVBoxLayout *layout = new QVBoxLayout;

    QStringList kingdoms = Sanguosha->getKingdoms();

    foreach(QString kingdom, kingdoms){
        QCommandLinkButton *button = new QCommandLinkButton;
        QPixmap kingdom_pixmap(QString("image/kingdom/icon/%1.png").arg(kingdom));
        QIcon kingdom_icon(kingdom_pixmap);

        button->setIcon(kingdom_icon);
        button->setIconSize(kingdom_pixmap.size());
        button->setText(Sanguosha->translate(kingdom));
        button->setObjectName(kingdom);

        layout->addWidget(button);

        connect(button, SIGNAL(clicked()), this, SLOT(chooseKingdom()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
    }

    dialog->setObjectName(".");
    connect(dialog, SIGNAL(rejected()), this, SLOT(chooseKingdom()));

    dialog->setObjectName(".");
    dialog->setWindowTitle(tr("Please choose a kingdom"));
    dialog->setLayout(layout);

    setStatus(ExecDialog);
    dialog->exec();
}

void Client::setMark(const QString &mark_str){
    QRegExp rx("(\\w+)\\.(@?[\\w-]+)=(\\d+)");

    if(!rx.exactMatch(mark_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString who = texts.at(1);
    QString mark = texts.at(2);
    int value = texts.at(3).toInt();

    ClientPlayer *player = getPlayer(who);
    player->setMark(mark, value);
}

void Client::chooseSuit(){
    request("chooseSuit " + sender()->objectName());

    setStatus(NotActive);
}

void Client::chooseKingdom(){
    request("chooseKingdom " + sender()->objectName());

    setStatus(NotActive);
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
    if(!rx.exactMatch(take_str))
        return;

    QStringList words = rx.capturedTexts();
    QString taker_name = words.at(1);
    int card_id = words.at(2).toInt();

    const Card *card = Sanguosha->getCard(card_id);
    if(taker_name != "."){
        ClientPlayer *taker = getPlayer(taker_name);
        taker->addCard(card, Player::Hand);
        emit ag_taken(taker, card_id);
    }else{
        discarded_list.prepend(card);
        emit ag_taken(NULL, card_id);
    }
}

void Client::clearAG(const QString &){
    emit ag_cleared();
}

void Client::askForSinglePeach(const QString &ask_str){
    QRegExp rx("(.+):(\\d+)");
    rx.exactMatch(ask_str);

    QStringList texts = rx.capturedTexts();
    ClientPlayer *dying = getPlayer(texts.at(1));
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
    use_card = false;
    setStatus(Responsing);
}

void Client::askForCardShow(const QString &requestor){
    QString name = Sanguosha->translate(requestor);
    emit prompt_changed(tr("%1 request you to show one hand card").arg(name));

    card_pattern = "."; // any card can be matched
    refusable = false;
    use_card = false;
    setStatus(Responsing);
}

void Client::askForAG(const QString &ask_str){
    refusable = ask_str == "?";
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
    Sanguosha->playAudio("your-turn");

    turn_tag.clear();
}

void Client::showCard(const QString &show_str){
    QRegExp rx("(.+):(\\d+)");
    if(!rx.exactMatch(show_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString player_name = texts.at(1);
    int card_id = texts.at(2).toInt();

    ClientPlayer *player = getPlayer(player_name);
    if(player != Self)
        player->addKnownHandCard(Sanguosha->getCard(card_id));

    emit card_shown(player_name, card_id);
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
    ClientPlayer *who = getPlayer(texts.at(1));

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
        prompt(tr("Please play a card for pindian"));
    else
        prompt(tr("%1 ask for you to play a card to pindian").arg(Sanguosha->translate(from)));

    use_card = false;
    card_pattern = ".";    
    refusable = false;

    setStatus(Responsing);
}

void Client::askForYiji(const QString &card_list){
    card_pattern = card_list;
    setStatus(AskForYiji);
}

void Client::askForPlayerChosen(const QString &players){
    players_to_choose = players.split("+");

    Q_ASSERT(!players_to_choose.isEmpty());

    setStatus(AskForPlayerChoose);
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

void Client::moveFocus(const QString &focus){
    emit focus_moved(focus);
}

void Client::setEmotion(const QString &set_str){
    QStringList words = set_str.split(":");
    QString target_name = words.at(0);
    QString emotion = words.at(1);

    emit emotion_set(target_name, emotion);
}

void Client::skillInvoked(const QString &invoke_str){
    QRegExp rx("(\\w+):(\\w+)");

    if(!rx.exactMatch(invoke_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString who = texts.at(1);
    QString skill_name = texts.at(2);

    emit skill_invoked(who, skill_name);
}

void Client::acquireSkill(const QString &acquire_str){
    QRegExp rx("(\\w+):(\\w+)");

    if(!rx.exactMatch(acquire_str))
        return;

    QStringList texts = rx.capturedTexts();
    ClientPlayer *who = getPlayer(texts.at(1));
    QString skill_name = texts.at(2);

    who->acquireSkill(skill_name);

    emit skill_acquired(who, skill_name);
}

void Client::addProhibitSkill(const QString &skill_name){
    const Skill *skill = Sanguosha->getSkill(skill_name);
    const ProhibitSkill *prohibit_skill = qobject_cast<const ProhibitSkill *>(skill);
    if(prohibit_skill){
        prohibit_skills << prohibit_skill;
    }
}

void Client::animate(const QString &animate_str){
    QStringList args = animate_str.split(":");
    QString name = args.takeFirst();

    emit animated(name, args);
}
