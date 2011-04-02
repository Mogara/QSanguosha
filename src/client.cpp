#include "client.h"
#include "settings.h"
#include "engine.h"
#include "nullificationdialog.h"
#include "playercarddialog.h"
#include "standard.h"
#include "optionbutton.h"
#include "nativesocket.h"
#include "recorder.h"

#include <QCryptographicHash>
#include <QMessageBox>
#include <QCheckBox>
#include <QCommandLinkButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QTextDocument>
#include <QTextCursor>

Client *ClientInstance = NULL;

Client::Client(QObject *parent, const QString &filename)
    :QObject(parent), refusable(true), status(NotActive), alive_count(1),
    slash_count(0)
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
    callbacks["clearPile"] = &Client::clearPile;
    callbacks["setPileNumber"] = &Client::setPileNumber;    
    callbacks["gameOver"] = &Client::gameOver;
    callbacks["killPlayer"] = &Client::killPlayer;
    callbacks["warn"] = &Client::warn;
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
    callbacks["setPrompt"] = &Client::setPrompt;
    callbacks["jilei"] = &Client::jilei;
    callbacks["judgeResult"] = &Client::judgeResult;
    callbacks["setScreenName"] = &Client::setScreenName;
    callbacks["pile"] = &Client::pile;

    callbacks["playSkillEffect"] = &Client::playSkillEffect;
    callbacks["playCardEffect"] = &Client::playCardEffect;
    callbacks["playAudio"] = &Client::playAudio;

    callbacks["moveNCards"] = &Client::moveNCards;
    callbacks["moveCard"] = &Client::moveCard;
    callbacks["drawCards"] = &Client::drawCards;
    callbacks["drawNCards"] = &Client::drawNCards;

    // interactive methods
    callbacks["activate"] = &Client::activate;
    callbacks["doChooseGeneral"] = &Client::doChooseGeneral;
    callbacks["doChooseGeneral2"] = &Client::doChooseGeneral2;
    callbacks["doGuanxing"] = &Client::doGuanxing;
    callbacks["doGongxin"] = &Client::doGongxin;
    callbacks["askForDiscard"] = &Client::askForDiscard;
    callbacks["askForExchange"] = &Client::askForExchange;
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
    callbacks["askForGeneral"] = &Client::askForGeneral;

    callbacks["fillAG"] = &Client::fillAG;
    callbacks["askForAG"] = &Client::askForAG;
    callbacks["takeAG"] = &Client::takeAG;
    callbacks["clearAG"] = &Client::clearAG;

    ask_dialog = NULL;
    use_card = false;

    Self = new ClientPlayer(this);
    Self->setScreenName(Config.UserName);
    Self->setProperty("avatar", Config.UserAvatar);
    connect(Self, SIGNAL(phase_changed()), this, SLOT(clearTurnTag()));
    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(notifyRoleChange(QString)));

    if(!filename.isEmpty()){
        socket = NULL;
        recorder = NULL;

        replayer = new Replayer(this, filename);
        connect(replayer, SIGNAL(command_parsed(QString)), this, SLOT(processCommand(QString)));
    }else{
        socket = new NativeClientSocket;
        socket->setParent(this);

        recorder = new Recorder(this);

        connect(socket, SIGNAL(message_got(char*)), recorder, SLOT(record(char*)));
        connect(socket, SIGNAL(message_got(char*)), this, SLOT(processReply(char*)));
        connect(socket, SIGNAL(error_message(QString)), this, SIGNAL(error_message(QString)));
        connect(socket, SIGNAL(connected()), this, SLOT(signup()));
        socket->connectToHost();

        replayer = NULL;
    }

    lines_doc = new QTextDocument(this);

    prompt_doc = new QTextDocument(this);
    prompt_doc->setTextWidth(350);
    prompt_doc->setDefaultFont(QFont("SimHei"));
}

void Client::signup(){
    if(replayer)
        replayer->start();
    else{
        QString base64 = Config.UserName.toUtf8().toBase64();       
        QString signup_str = QString("signup %1:%2").arg(base64).arg(Config.UserAvatar);
        QString password = Config.Password;
        if(!password.isEmpty()){
            password = QCryptographicHash::hash(password.toAscii(), QCryptographicHash::Md5).toHex();
            signup_str.append(":" + password);
        }
        request(signup_str);
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
            player->setProperty(property, value);
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
    choose_command = "choose";
    emit generals_got(generals_str.split("+"));
}

void Client::doChooseGeneral2(const QString &generals_str){
    choose_command = "choose2";
    emit generals_got(generals_str.split("+"));
}

void Client::chooseItem(const QString &item_name){
    if(!item_name.isEmpty()){        
        request(QString("%1 %2").arg(choose_command).arg(item_name));
        Sanguosha->playAudio("choose-item");
    }
}

void Client::requestCard(int card_id){
    request(QString("useCard @CheatCard=%1->.").arg(card_id));
}

void Client::addRobot(){
    request("addRobot .");
}

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

        if(status == Playing){
            ++ used[card->metaObject()->className()];

            if(card->inherits("Slash"))
                increaseSlashCount();

        }else if(status == Responsing)
            card_pattern.clear();
    }

    setStatus(NotActive);
}

void Client::startInXs(const QString &left_seconds){
    int seconds = left_seconds.toInt();
    lines_doc->setHtml(tr("Game will start in <b>%1</b> seconds").arg(left_seconds));

    if(seconds == 0 && Sanguosha->getScenario(ServerInfo.GameMode) == NULL){
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
        lines_doc->setHtml(prompt_str);
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
    QRegExp rx("(.+):(-?\\d+)([FT]*)");

    if(!rx.exactMatch(change_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString who = texts.at(1);
    int delta = texts.at(2).toInt();
    QString nature_str = texts.at(3);

    DamageStruct::Nature nature;
    if(nature_str == "F")
        nature = DamageStruct::Fire;
    else if(nature_str == "T")
        nature = DamageStruct::Thunder;
    else
        nature = DamageStruct::Normal;

    emit hp_changed(who, delta, nature);
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

void Client::setPrompt(const QString &prompt_str){
    QStringList texts = prompt_str.split(":");
    setPromptList(texts);
}

void Client::jilei(const QString &jilei_str){
    if(jilei_str == ".")
        jilei_flags.clear();
    else
        jilei_flags.append(jilei_str);
}

void Client::judgeResult(const QString &result_str){
    QStringList texts = result_str.split(":");
    QString who = texts.at(0);
    QString result = texts.at(1);

    emit judge_result(who, result);
}

bool Client::isJilei(const Card *card) const{
    if(card->inherits("BasicCard"))
        return jilei_flags.contains("B");
    else if(card->inherits("EquipCard"))
        return jilei_flags.contains("E");
    else if(card->inherits("TrickCard"))
        return jilei_flags.contains("T");
    else if(card->inherits("SkillCard")){
        const SkillCard *skill_card = qobject_cast<const SkillCard *>(card);
        if(!skill_card->willThrow())
            return false;

        QList<int> card_ids = card->getSubcards();
        foreach(int card_id, card_ids){
            const Card *subcard = Sanguosha->getCard(card_id);
            if(isJilei(subcard))
                return true;
        }
    }

    return false;
}

bool Client::canSlashWithCrossbow() const{
    if(Self->hasSkill("paoxiao"))
        return true;
    else{
        if(Self->hasFlag("tianyi_success"))
            return slash_count < 2;
        else
            return slash_count < 1;
    }
}

QString Client::getSkillLine() const{
    return skill_line;
}

void Client::setPromptList(const QStringList &texts){
    QString prompt = Sanguosha->translate(texts.at(0));
    if(texts.length() >= 2){
        QString src = Sanguosha->translate(texts.at(1));
        prompt.replace("%src", src);
    }

    if(texts.length() >= 3){
        QString dest = Sanguosha->translate(texts.at(2));
        prompt.replace("%dest", dest);
    }

    if(texts.length() >= 4){
        QString arg = Sanguosha->translate(texts.at(3));
        prompt.replace("%arg", arg);
    }

    prompt_doc->setHtml(prompt);
}

void Client::askForCardOrUseCard(const QString &request_str){
    QStringList texts = request_str.split(":");
    QString pattern = texts.takeFirst();

    card_pattern = pattern;

    if(texts.isEmpty()){
        return;
    }else
        setPromptList(texts);

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

    Sanguosha->playAudio("pop-up");
    setStatus(ExecDialog);
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
    Sanguosha->playAudio("pop-up");
    setStatus(ExecDialog);
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

void Client::askForNullification(const QString &ask_str){
    QRegExp rx("(\\w+):(.+)->(.+)");
    if(!rx.exactMatch(ask_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString trick_name = texts.at(1);
    const Card *trick_card = Sanguosha->findChild<const Card *>(trick_name);

    QString source_name = texts.at(2);
    ClientPlayer *source = NULL;
    if(source_name != ".")
        source = getPlayer(source_name);

    if(Config.NeverNullifyMyTrick && source == Self){
        if(trick_card->inherits("SingleTargetTrick")){
            responseCard(NULL);
            return;
        }
    }

    QString trick_path = trick_card->getPixmapPath();
    QString to = getPlayer(texts.at(3))->getGeneral()->getPixmapPath("big");
    if(source == NULL){
        prompt_doc->setHtml(QString("<img src='%1' /> ==&gt; <img src='%2' />").arg(trick_path).arg(to));
    }else{
        QString from = source->getGeneral()->getPixmapPath("big");
        prompt_doc->setHtml(QString("<img src='%1' /> <img src='%2'/> ==&gt; <img src='%3' />").arg(trick_path).arg(from).arg(to));
    }

    card_pattern = "nullification";
    refusable = true;
    use_card = false;

    setStatus(Responsing);
}

void Client::playAudio(const QString &name){
    Sanguosha->playAudio(name);
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
    slash_count ++;
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
        skill_line = Sanguosha->translate("$" + skill_name);

        QChar last_char = skill_name[skill_name.length()-1];
        if(last_char.isDigit())
            skill_name.chop(1);

        skill_title = Sanguosha->translate(skill_name);

        updatePileNum();
    }
}

QTextDocument *Client::getLinesDoc() const{
    return lines_doc;
}

QTextDocument *Client::getPromptDoc() const{
    return prompt_doc;
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
    QString pile_str = tr("Draw pile: <b>%1</b>, discard pile: <b>%2</b>")
                       .arg(pile_num).arg(discarded_list.length());

    if(skill_title.isEmpty())
        lines_doc->setHtml(pile_str);
    else
        lines_doc->setHtml(QString("%1 <br/> <b>%2</b>: %3").arg(pile_str).arg(skill_title).arg(skill_line));
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

    prompt_doc->setHtml(prompt);

    setStatus(Discarding);    
}

void Client::askForExchange(const QString &exchange_str){
    bool ok;
    discard_num = exchange_str.toInt(&ok);
    if(!ok){
        QMessageBox::warning(NULL, tr("Warning"), tr("Exchange string is not well formatted!"));
        return;
    }

    refusable = false;
    include_equip = false;

    prompt_doc->setHtml(tr("Please give %1 cards to exchange").arg(discard_num));

    setStatus(Discarding);
}

void Client::gameOver(const QString &result_str){    
    QStringList texts = result_str.split(":");
    QString winner = texts.at(0);
    QStringList roles = texts.at(1).split("+");

    Q_ASSERT(roles.length() == players.length());

    int i;
    for(i=0; i<roles.length(); i++){
        QString name = players.at(i)->objectName();
        getPlayer(name)->setRole(roles.at(i));
    }

    if(winner == "."){
        emit standoff();
        return;
    }

    bool victory = false;
    QList<bool> result_list;
    foreach(const ClientPlayer *player, players){
        QString role = player->getRole();
        bool result = winner.contains(player->objectName()) || winner.contains(role);
        result_list << result;

        if(player == Self)
            victory = result;
    }

    emit game_over(victory, result_list);
}

void Client::killPlayer(const QString &player_name){
    alive_count --;

    QString general_name = getPlayer(player_name)->getGeneralName();
    QString last_word = Sanguosha->translate(QString("~%1").arg(general_name));

    skill_title = tr("%1[dead]").arg(Sanguosha->translate(general_name));
    skill_line = last_word;

    updatePileNum();

    emit player_killed(player_name);
}

void Client::warn(const QString &reason){
    QString msg;
    if(reason == "GAME_OVER")
        msg = tr("Game is over now");
    else if(reason == "REQUIRE_PASSWORD")
        msg = tr("The server require password to signup");
    else if(reason == "WRONG_PASSWORD")
        msg = tr("Your password is wrong");
    else
        msg = tr("Unknown warning: %1").arg(reason);

    disconnectFromHost();
    QMessageBox::warning(NULL, tr("Warning"), msg);
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
        button->setIcon(QIcon(QString("image/system/suit/%1.png").arg(suit)));
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
}

void Client::askForKingdom(const QString &){
    delete ask_dialog;

    QDialog *dialog = new QDialog;
    ask_dialog = dialog;

    QVBoxLayout *layout = new QVBoxLayout;

    QStringList kingdoms = Sanguosha->getKingdoms();
    kingdoms.removeOne("god"); // god kingdom does not really exist

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
        prompt_doc->setHtml(tr("You are dying, please provide %1 peach(es)(or analeptic) to save yourself").arg(peaches));
        card_pattern = "peach+analeptic";
    }else{
        QString dying_general = Sanguosha->translate(dying->getGeneralName());
        prompt_doc->setHtml(tr("%1 is dying, please provide %2 peach(es) to save him").arg(dying_general).arg(peaches));
        card_pattern = "peach";
    }

    refusable = true;
    use_card = false;
    setStatus(Responsing);
}

void Client::askForCardShow(const QString &requestor){
    QString name = Sanguosha->translate(requestor);
    prompt_doc->setHtml(tr("%1 request you to show one hand card").arg(name));

    //card_pattern = "."; // any card can be matched
    //refusable = false;
    use_card = false;
    //setStatus(Responsing);

    setStatus(AskForCardShow);
}

void Client::askForAG(const QString &ask_str){
    refusable = ask_str == "?";
    setStatus(AskForAG);
}

void Client::chooseAG(int card_id){
    request(QString("chooseAG %1").arg(card_id));

    setStatus(NotActive);
}

QList<const ClientPlayer*> Client::getPlayers() const{
    return players;
}

void Client::clearTurnTag(){
    switch(Self->getPhase()){
    case Player::Start:{
            Sanguosha->playAudio("your-turn");
            used.clear();
            slash_count = 0;
            break;
    }

    case Player::Finish:{
            Self->clearFlags();
            break;
        }

    default:
        break;
    }
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
    QList<int> card_ids;
    if(guanxing_str != "."){
        QStringList cards = guanxing_str.split("+");

        foreach(QString card, cards)
            card_ids << card.toInt();
    }

    emit guanxing(card_ids);
    setStatus(AskForGuanxing);
}

void Client::doGongxin(const QString &gongxin_str){
    QRegExp rx("(\\w+)(!?):(.+)");
    if(!rx.exactMatch(gongxin_str))
        return;

    QStringList texts = rx.capturedTexts();
    ClientPlayer *who = getPlayer(texts.at(1));
    bool enable_heart = texts.at(2).isEmpty();
    QStringList cards = texts.at(3).split("+");
    QList<int> card_ids;
    foreach(QString card, cards)
        card_ids << card.toInt();

    who->setCards(card_ids);

    emit gongxin(card_ids, enable_heart);
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

    if(from == Self->getGeneralName())
        prompt_doc->setHtml(tr("Please play a card for pindian"));
    else
        prompt_doc->setHtml(tr("%1 ask for you to play a card to pindian").arg(Sanguosha->translate(from)));

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

void Client::askForGeneral(const QString &generals){
    choose_command = "chooseGeneral";
    emit generals_got(generals.split("+"));
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

    if(who == "."){
        QString line = tr("<font color='red'>System: %1</font>").arg(text);
        emit words_spoken(line);
        return;
    }

    const ClientPlayer *from = getPlayer(who);

    QString title;
    if(from){
        title = from->getGeneralName();
        title = Sanguosha->translate(title);
        title.append(QString("(%1)").arg(from->screenName()));
    }

    title = QString("<b>%1</b>").arg(title);

    QString line = tr("<font color='%1'>[%2] said: %3 </font>")
                   .arg(Config.TextEditColor.name()).arg(title).arg(text);

    emit words_spoken(line);
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

bool Client::hasUsed(const QString &card_class) const{
    return used.contains(card_class);
}

int Client::usedTimes(const QString &card_class) const{
    return used.value(card_class, 0);
}

void Client::setScreenName(const QString &set_str){
    QStringList words = set_str.split(":");
    ClientPlayer *player = getPlayer(words.first());
    QString base64 = words.at(1);
    QString screen_name = QString::fromUtf8(QByteArray::fromBase64(base64.toAscii()));
    player->setScreenName(screen_name);
}

void Client::pile(const QString &pile_str){
    QRegExp rx("(\\w+):(\\w+)([+-])(\\d+)");
    if(!rx.exactMatch(pile_str)){
        return;
    }

    QStringList texts = rx.capturedTexts();
    ClientPlayer *player = getPlayer(texts.at(1));
    QString name = texts.at(2);
    bool add = texts.at(3) == "+";
    int card_id = texts.at(4).toInt();

    if(player)
        player->changePile(name, add, card_id);
}
