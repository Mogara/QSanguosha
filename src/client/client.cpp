#include "client.h"
#include "settings.h"
#include "engine.h"
#include "playercarddialog.h"
#include "standard.h"
#include "choosegeneraldialog.h"
#include "nativesocket.h"
#include "recorder.h"
#include "jsonutils.h"

#include <QApplication>
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

using namespace std;
using namespace QSanProtocol;
using namespace QSanProtocol::Utils;

Client *ClientInstance = NULL;

Client::Client(QObject *parent, const QString &filename)
    :QObject(parent), m_isDiscardActionRefusable(true),
    status(NotActive), alive_count(1), swap_pile(0)
{

    ClientInstance = this;

    callbacks["checkVersion"] = &Client::checkVersion;

    callbacks["roomBegin"] = &Client::roomBegin;
    callbacks["room"] = &Client::room;
    callbacks["roomEnd"] = &Client::roomEnd;
    callbacks["roomCreated"] = &Client::roomCreated;
    callbacks["roomError"] = &Client::roomError;
    callbacks["hallEntered"] = &Client::hallEntered;

    callbacks["setup"] = &Client::setup;
    callbacks["networkDelayTest"] = &Client::networkDelayTest;
    callbacks["addPlayer"] = &Client::addPlayer;
    callbacks["removePlayer"] = &Client::removePlayer;
    callbacks["startInXs"] = &Client::startInXs;
    callbacks["arrangeSeats"] = &Client::arrangeSeats;
    callbacks["warn"] = &Client::warn;

    callbacks["startGame"] = &Client::startGame;
    callbacks["gameOver"] = &Client::gameOver;

    callbacks["hpChange"] = &Client::hpChange;
    callbacks["killPlayer"] = &Client::killPlayer;
    callbacks["revivePlayer"] = &Client::revivePlayer;
    m_callbacks[S_COMMAND_SHOW_CARD] = &Client::showCard;
    //callbacks["showCard"] = &Client::showCard;
    callbacks["setMark"] = &Client::setMark;
    callbacks["doFilter"] = &Client::doFilter;
    callbacks["log"] = &Client::log;
    callbacks["speak"] = &Client::speak;
    callbacks["acquireSkill"] = &Client::acquireSkill;
    callbacks["attachSkill"] = &Client::attachSkill;
    callbacks["detachSkill"] = &Client::detachSkill;
    m_callbacks[S_COMMAND_MOVE_FOCUS] = &Client::moveFocus; 
    //callbacks["moveFocus"] = &Client::moveFocus;
    callbacks["setEmotion"] = &Client::setEmotion;
    m_callbacks[S_COMMAND_INVOKE_SKILL] = &Client::skillInvoked; 
    m_callbacks[S_COMMAND_SKILL_GONGXIN] = &Client::askForGongxin; 
    //callbacks["skillInvoked"] = &Client::skillInvoked;
    callbacks["addHistory"] = &Client::addHistory;
    callbacks["animate"] = &Client::animate;
    callbacks["judgeResult"] = &Client::judgeResult;
    callbacks["setScreenName"] = &Client::setScreenName;
    callbacks["setFixedDistance"] = &Client::setFixedDistance;
    callbacks["transfigure"] = &Client::transfigure;
    callbacks["jilei"] = &Client::jilei;
    callbacks["cardLock"] = &Client::cardLock;
    callbacks["pile"] = &Client::pile;

    callbacks["updateStateItem"] = &Client::updateStateItem;

    callbacks["playSkillEffect"] = &Client::playSkillEffect;
    callbacks["playCardEffect"] = &Client::playCardEffect;
    callbacks["playAudio"] = &Client::playAudio;

    callbacks["moveNCards"] = &Client::moveNCards;
    callbacks["moveCard"] = &Client::moveCard;
    callbacks["drawNCards"] = &Client::drawNCards;
    callbacks["drawCards"] = &Client::drawCards;
    callbacks["clearPile"] = &Client::clearPile;
    callbacks["setPileNumber"] = &Client::setPileNumber;
    callbacks["setStatistics"] = &Client::setStatistics;

    // interactive methods    
    m_interactions[S_COMMAND_CHOOSE_GENERAL] = &Client::askForGeneral;
    //callbacks["askForGeneral"] = &Client::askForGeneral;
    m_interactions[S_COMMAND_CHOOSE_PLAYER] = &Client::askForPlayerChosen;
    //callbacks["askForPlayerChosen"] = &Client::askForPlayerChosen;
    m_interactions[S_COMMAND_CHOOSE_ROLE] = &Client::askForAssign;
    //callbacks["askForAssign"] = &Client::askForAssign;
    m_interactions[S_COMMAND_CHOOSE_DIRECTION] = &Client::askForDirection;
    //callbacks["askForDirection"] = &Client::askForDirection;
    m_interactions[S_COMMAND_EXCHANGE_CARD] = &Client::askForExchange;
    //callbacks["askForExchange"] = &Client::askForExchange;
    m_interactions[S_COMMAND_ASK_PEACH] = &Client::askForSinglePeach;
    //callbacks["askForSinglePeach"] = &Client::askForSinglePeach;
    m_interactions[S_COMMAND_SKILL_GUANXING] = &Client::askForGuanxing;
    //callbacks["doGuanxing"] = &Client::doGuanxing;
    m_interactions[S_COMMAND_SKILL_GONGXIN] = &Client::askForGongxin;
    //callbacks["doGongxin"] = &Client::doGongxin;
    m_interactions[S_COMMAND_SKILL_YIJI] = &Client::askForYiji;
    //callbacks["askForYiji"] = &Client::askForYiji;
    m_interactions[S_COMMAND_PLAY_CARD] = &Client::activate;
    // callbacks["activate"] = &Client::activate;
    m_interactions[S_COMMAND_DISCARD_CARD] = &Client::askForDiscard;
    //callbacks["askForDiscard"] = &Client::askForDiscard;
    m_interactions[S_COMMAND_CHOOSE_SUIT] = &Client::askForSuit;
    //callbacks["askForSuit"] = &Client::askForSuit;
    m_interactions[S_COMMAND_CHOOSE_KINGDOM] = &Client::askForKingdom;
    //callbacks["askForKingdom"] = &Client::askForKingdom;    
    m_interactions[S_COMMAND_RESPONSE_CARD] = &Client::askForCard;
    //callbacks["askForCard"] = &Client::askForCard;
    m_interactions[S_COMMAND_USE_CARD] = &Client::askForUseCard;
    //callbacks["askForUseCard"] = &Client::askForUseCard;
    m_interactions[S_COMMAND_INVOKE_SKILL] = &Client::askForSkillInvoke;
    //callbacks["askForSkillInvoke"] = &Client::askForSkillInvoke;
    m_interactions[S_COMMAND_MULTIPLE_CHOICE] = &Client::askForChoice;
    //callbacks["askForChoice"] = &Client::askForChoice;
    m_interactions[S_COMMAND_NULLIFICATION] = &Client::askForNullification;
    //callbacks["askForNullification"] = &Client::askForNullification;
    m_interactions[S_COMMAND_SHOW_CARD] = &Client::askForCardShow;
    //callbacks["askForCardShow"] = &Client::askForCardShow;
    m_interactions[S_COMMAND_AMAZING_GRACE] = &Client::askForAG;
    //callbacks["askForAG"] = &Client::askForAG;        
    m_interactions[S_COMMAND_PINDIAN] = &Client::askForPindian;
    //callbacks["askForPindian"] = &Client::askForPindian;
    m_interactions[S_COMMAND_CHOOSE_CARD] = &Client::askForCardChosen;
    //callbacks["askForCardChosen"] = &Client::askForCardChosen;
    m_interactions[S_COMMAND_CHOOSE_ORDER] = &Client::askForOrder;
    m_interactions[S_COMMAND_CHOOSE_ROLE_3V3] = &Client::askForRole3v3;
    m_interactions[S_COMMAND_SURRENDER] = &Client::askForSurrender;

    callbacks["fillAG"] = &Client::fillAG;    
    callbacks["takeAG"] = &Client::takeAG;
    callbacks["clearAG"] = &Client::clearAG;
    callbacks["disableAG"] = &Client::disableAG;

    // 3v3 mode & 1v1 mode
    callbacks["fillGenerals"] = &Client::fillGenerals;
    callbacks["askForGeneral3v3"] = &Client::askForGeneral3v3;
    callbacks["askForGeneral1v1"] = &Client::askForGeneral3v3;
    callbacks["takeGeneral"] = &Client::takeGeneral;
    callbacks["startArrange"] = &Client::startArrange;    
    callbacks["recoverGeneral"] = &Client::recoverGeneral;
    callbacks["revealGeneral"] = &Client::revealGeneral;   

    ask_dialog = NULL;
    m_isUseCard = false;

    Self = new ClientPlayer(this);
    Self->setScreenName(Config.UserName);
    Self->setProperty("avatar", Config.UserAvatar);
    connect(Self, SIGNAL(phase_changed()), this, SLOT(clearTurnTag()));
    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(notifyRoleChange(QString)));

    players << Self;

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
        QString command = Config.value("EnableReconnection", false).toBool() ? "signupr" : "signup";
        QString signup_str = QString("%1 %2:%3").arg(command).arg(base64).arg(Config.UserAvatar);
        QString password = Config.Password;
        if(!password.isEmpty()){
            password = QCryptographicHash::hash(password.toAscii(), QCryptographicHash::Md5).toHex();
            signup_str.append(":" + password);
        }
        request(signup_str);
    }
}

void Client::networkDelayTest(const QString &){
    request("networkDelayTest .");
}

void Client::replyToServer(CommandType command, const Json::Value &arg){    
    if(socket)
    {
        QSanGeneralPacket packet(S_CLIENT_REPLY, command);
        packet.m_localSerial = _m_lastServerSerial;
        packet.setMessageBody(arg);
        socket->send(toQString(packet.toString()));
    }
}

void Client::requestToServer(CommandType command, const Json::Value &arg){    
    if(socket)
    {
        QSanGeneralPacket packet(S_CLIENT_REQUEST, command);        
        packet.setMessageBody(arg);
        socket->send(toQString(packet.toString()));
    }
}

void Client::request(const QString &message){
    if(socket)
        socket->send(message);
}

void Client::checkVersion(const QString &server_version){
    QString version_number, mod_name;
    if(server_version.contains(QChar(':'))){
        QStringList texts = server_version.split(QChar(':'));
        version_number = texts.value(0);
        mod_name = texts.value(1);
    }else{
        version_number = server_version;
        mod_name = "official";
    }

    emit version_checked(version_number, mod_name);
}

void Client::setup(const QString &setup_str){
    if(socket && !socket->isConnected())
        return;

    if(ServerInfo.parse(setup_str)){
        emit server_connected();
        request("toggleReady .");
    }else{
        QMessageBox::warning(NULL, tr("Warning"), tr("Setup string can not be parsed: %1").arg(setup_str));
    }
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

    QSanGeneralPacket packet;
    if (packet.parse(reply))
    {
        if (packet.getPacketType() == S_SERVER_NOTIFICATION)
        {
            CallBack callback = m_callbacks[packet.getCommandType()];
            if (callback) {            
                (this->*callback)(packet.getMessageBody());
            }
        }
        else if (packet.getPacketType() == S_SERVER_REQUEST)
        {
            _m_lastServerSerial = packet.m_globalSerial;
            CallBack callback = m_interactions[packet.getCommandType()];
            if (callback) {    
                (this->*callback)(packet.getMessageBody());
            }
        }
        return;
    }

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
        QString method = method_name;

        if(replayer && (method.startsWith("askFor") || method.startsWith("do") || method == "activate"))
            return;

        static QSet<QString> deprecated;
        if(deprecated.isEmpty()){
            deprecated << "increaseSlashCount" // replaced by addHistory
                    << "addProhibitSkill"; // add all prohibit skill at game start
        }

        Callback callback = callbacks.value(method, NULL);
        if(callback){
            QString arg_str = arg;
            (this->*callback)(arg_str);
        }else if(!deprecated.contains(method))
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

    players << player;

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
    if(!pattern.exactMatch(draw_str))
        return;

    QStringList texts = pattern.capturedTexts();
    ClientPlayer *player = findChild<ClientPlayer*>(texts.at(1));
    int n = texts.at(2).toInt();

    if(player && n>0){
        if(!Self->hasFlag("marshalling")){
            pile_num -= n;
            updatePileNum();
        }

        player->handCardChange(n);
        emit n_cards_drawed(player, n);
    }
}

void Client::onPlayerChooseGeneral(const QString &item_name){
    if(!item_name.isEmpty()){
        replyToServer(S_COMMAND_CHOOSE_GENERAL, toJsonString(item_name));        
        Sanguosha->playAudio("choose-item");
    }
}

void Client::requestCheatRunScript(const QString& script)
{
    Json::Value cheatReq(Json::arrayValue), cheatArg(Json::arrayValue);
    cheatReq[0] = (int)S_CHEAT_RUN_SCRIPT;    
    cheatReq[1] = toJsonString(script);
    requestToServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatRevive(const QString& name)
{
    Json::Value cheatReq(Json::arrayValue), cheatArg(Json::arrayValue);
    cheatReq[0] = (int)S_CHEAT_REVIVE_PLAYER;    
    cheatReq[1] = toJsonString(name);
    requestToServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatDamage(const QString& source, const QString& target, DamageStruct::Nature nature, int points)
{
    Json::Value cheatReq(Json::arrayValue), cheatArg(Json::arrayValue);
    cheatArg[0] = toJsonString(source);
    cheatArg[1] = toJsonString(target);
    cheatArg[2] = (int)nature;
    cheatArg[3] = points;    

    cheatReq[0] = (int)S_CHEAT_MAKE_DAMAGE;    
    cheatReq[1] = cheatArg;
    requestToServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatKill(const QString& killer, const QString& victim)
{
    Json::Value cheatArg;
    cheatArg[0] = (int)S_CHEAT_KILL_PLAYER;
    cheatArg[1] = toJsonArray(killer, victim);
    requestToServer(S_COMMAND_CHEAT, cheatArg);
}

void Client::requestCheatGetOneCard(int card_id){
    Json::Value cheatArg;
    cheatArg[0] = (int)S_CHEAT_GET_ONE_CARD;
    cheatArg[1] = card_id;
    requestToServer(S_COMMAND_CHEAT, cheatArg);
}

void Client::requestCheatChangeGeneral(QString name){
    Json::Value cheatArg;
    cheatArg[0] = (int)S_CHEAT_CHANGE_GENERAL;
    cheatArg[1] = toJsonString(name);
    requestToServer(S_COMMAND_CHEAT, cheatArg);
}

void Client::addRobot(){
    request("addRobot .");
}

void Client::fillRobots(){
    request("fillRobots .");
}

void Client::arrange(const QStringList &order){
    Q_ASSERT(order.length() == 3);

    request(QString("arrange %1").arg(order.join("+")));
}

void Client::onPlayerUseCard(const Card *card, const QList<const Player *> &targets){

    if(card == NULL){
        replyToServer(S_COMMAND_USE_CARD, Json::Value::null);
        // request("useCard .");
    }else{
        Json::Value targetNames(Json::arrayValue);
        foreach(const Player *target, targets)
            targetNames.append(toJsonString(target->objectName()));

        replyToServer(S_COMMAND_USE_CARD, toJsonArray(card->toString(), targetNames));

        //if(target_names.isEmpty())
        //    request(QString("useCard %1->.").arg(card->toString()));
        //else
        //    request(QString("useCard %1->%2").arg(card->toString()).arg(target_names.join("+")));

        if(status == Responsing)
            card_pattern.clear();
    }

    setStatus(NotActive);
}

void Client::startInXs(const QString &left_seconds){
    int seconds = left_seconds.toInt();
    lines_doc->setHtml(tr("Game will start in <b>%1</b> seconds").arg(left_seconds));

    emit start_in_xs();
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

void Client::activate(const Json::Value& playerId){    
    if(toQString(playerId) == Self->objectName())
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

            if(!Self->hasFlag("marshalling")){
                if(move.from_place == Player::DrawPile)
                    pile_num --;

                updatePileNum();
            }
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
    QRegExp rx("(\\d+):(\\w+)(@special)?->(\\w+)(@special)?");
    if(rx.exactMatch(move_str)){
        QStringList texts = rx.capturedTexts();
        int n = texts.at(1).toInt();

        QString from = texts.at(2);
        QString to = texts.at(4);

        ClientPlayer *src = getPlayer(from);
        ClientPlayer *dest = getPlayer(to);

        if(texts.at(3).isEmpty())
            src->handCardChange(-n);

        if(texts.at(5).isEmpty())
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
    QRegExp rx("(.+):(-?\\d+)([FTL]*)");

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

    emit hp_changed(who, delta, nature, nature_str == "L");
}

void Client::setStatus(Status status){
    if(this->status != status||status == NotActive){
        this->status = status;
        emit status_changed(status);
    }
}

Client::Status Client::getStatus() const{
    return status;
}

void Client::jilei(const QString &jilei_str){
    Self->jilei(jilei_str);
}

void Client::cardLock(const QString &card_str){
    Self->setCardLocked(card_str);
}

void Client::judgeResult(const QString &result_str){
    QStringList texts = result_str.split(":");
    QString who = texts.at(0);
    QString result = texts.at(1);

    emit judge_result(who, result);
}

QString Client::getSkillLine() const{
    return skill_line;
}

Replayer *Client::getReplayer() const{
    return replayer;
}

QString Client::getPlayerName(const QString &str){
    QRegExp rx("sgs\\d+");
    QString general_name;
    if(rx.exactMatch(str)){
        ClientPlayer *player = getPlayer(str);
        general_name = player->getGeneralName();
        general_name = Sanguosha->translate(general_name);
        if(ServerInfo.GameMode == "08same" || player->getGeneralName() == "anjiang")
            general_name = QString("%1[%2]").arg(general_name).arg(player->getSeat());
        return general_name;

    }else
        return Sanguosha->translate(str);
}

QString Client::getPattern() const{
    return card_pattern;
}

QString Client::getSkillNameToInvoke() const{
    return skill_to_invoke;
}

void Client::onPlayerInvokeSkill(bool invoke){    
    if (skill_name == "surrender")
        replyToServer(S_COMMAND_SURRENDER, invoke);
    else
        replyToServer(S_COMMAND_INVOKE_SKILL, invoke);
    setStatus(NotActive);
}

void Client::setPromptList(const QStringList &texts){
    QString prompt = Sanguosha->translate(texts.at(0));
    if(texts.length() >= 2)
        prompt.replace("%src", getPlayerName(texts.at(1)));

    if(texts.length() >= 3)
        prompt.replace("%dest", getPlayerName(texts.at(2)));

    if(texts.length() >= 4){
        QString arg = Sanguosha->translate(texts.at(3));
        prompt.replace("%arg", arg);
    }

    if(texts.length() >= 5){
        QString arg2 = Sanguosha->translate(texts.at(4));
        prompt.replace("%2arg", arg2);
    }

    prompt_doc->setHtml(prompt);
}

void Client::commandFormatWarning(const QString &str, const QRegExp &rx, const char *command){
    QString text = tr("The argument (%1) of command %2 does not conform the format %3")
                   .arg(str).arg(command).arg(rx.pattern());
    QMessageBox::warning(NULL, tr("Command format warning"), text);
}

void Client::_askForCardOrUseCard(const Json::Value &cardUsage){
    
    Q_ASSERT(isStringArray(cardUsage, 0, 1));
    card_pattern = toQString(cardUsage[0]);
    QStringList texts = toQString(cardUsage[1]).split(":");

    if(texts.isEmpty()){
        return;
    }else
        setPromptList(texts);

    if(card_pattern.endsWith("!"))
        m_isDiscardActionRefusable = false;
    else
        m_isDiscardActionRefusable = true;

    QRegExp rx("^@@?(\\w+)(-card)?$");
    if(rx.exactMatch(card_pattern)){
        QString skill_name = rx.capturedTexts().at(1);
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill){
            QString text = prompt_doc->toHtml();
            text.append(tr("<br/> <b>Notice</b>: %1<br/>").arg(skill->getDescription()));
            prompt_doc->setHtml(text);
        }
    }

    setStatus(Responsing);
}

void Client::askForCard(const Json::Value &req){
    if (!req.isArray() || req.size() == 0) return;
    m_isUseCard = toQString(req[0]).startsWith("@@");
    _askForCardOrUseCard(req);
}

void Client::askForUseCard(const Json::Value &req){
    m_isUseCard = true;
    _askForCardOrUseCard(req);
}

void Client::askForSkillInvoke(const Json::Value &arg){    
    if (!isStringArray(arg, 0, 1)) return;
    QString skill_name = toQString(arg[0]);
    QString data = toQString(arg[1]);

    skill_to_invoke = skill_name;
        
    QString text;
    if(data.isEmpty())
        text = tr("Do you want to invoke skill [%1] ?").arg(Sanguosha->translate(skill_name));
    else
        text = Sanguosha->translate(QString("%1:%2").arg(skill_name).arg(data));

    const Skill *skill = Sanguosha->getSkill(skill_name);
    if(skill){
        text.append(tr("<br/> <b>Notice</b>: %1<br/>").arg(skill->getDescription()));
    }

    prompt_doc->setHtml(text);
    setStatus(AskForSkillInvoke);
}

void Client::onPlayerMakeChoice(){
    QString option = sender()->objectName();
    replyToServer(S_COMMAND_MULTIPLE_CHOICE, toJsonString(option));    
    setStatus(NotActive);
}

void Client::askForSurrender(const Json::Value &initiator){
    
    if (!initiator.isString()) return;    

    QString text = tr("%1 initiated a vote for disadvataged side to claim "
                        "capitulation. Click \"OK\" to surrender or \"Cancel\" to resist.")
                            .arg(Sanguosha->translate(toQString(initiator)));
    text.append(tr("<br/> <b>Noitce</b>: if all people on your side decides to surrender. "
                   "You'll lose this game."));
    skill_name = "surrender";

    prompt_doc->setHtml(text);
    setStatus(AskForSkillInvoke);
}

void Client::askForChoice(const Json::Value &ask_str){
    if (!isStringArray(ask_str, 0, 1)) return;
        
    QString skill_name = toQString(ask_str[0]);
    QStringList options =toQString(ask_str[1]).split("+");

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(Sanguosha->translate(skill_name));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("Please choose:")));

    foreach(QString option, options){
        QCommandLinkButton *button = new QCommandLinkButton;
        QString text = QString("%1:%2").arg(skill_name).arg(option);
        QString translated = Sanguosha->translate(text);
        if(text == translated)
            translated = Sanguosha->translate(option);

        button->setObjectName(option);
        button->setText(translated);

        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
        connect(button, SIGNAL(clicked()), this, SLOT(onPlayerMakeChoice()));

        layout->addWidget(button);
    }

    dialog->setObjectName(options.first());
    connect(dialog, SIGNAL(rejected()), this, SLOT(onPlayerMakeChoice()));

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

void Client::askForNullification(const Json::Value &arg){
    if (!arg.isArray() || arg.size() != 3 || !arg[0].isString()
        || !(arg[1].isNull() ||arg[1].isString())
        || !arg[2].isString()) return;
    
    QString trick_name = toQString(arg[0]);
    Json::Value source_name = arg[1];
    ClientPlayer* target_player = getPlayer(toQString(arg[2]));

    if (!target_player || !target_player->getGeneral()) return;

    const Card *trick_card = Sanguosha->findChild<const Card *>(trick_name);
    ClientPlayer *source = NULL;
    if(source_name != Json::Value::null)
        source = getPlayer(source_name.asCString());

    if(Config.NeverNullifyMyTrick && source == Self){
        if(trick_card->inherits("SingleTargetTrick") || trick_card->objectName() == "iron_chain"){
            onPlayerResponseCard(NULL);
            return;
        }
    }

    QString trick_path = trick_card->getPixmapPath();
    QString to = target_player->getGeneral()->getPixmapPath("big");
    if(source == NULL){
        prompt_doc->setHtml(QString("<img src='%1' /> ==&gt; <img src='%2' />").arg(trick_path).arg(to));
    }else{
        QString from = source->getGeneral()->getPixmapPath("big");
        prompt_doc->setHtml(QString("<img src='%1' /> <img src='%2'/> ==&gt; <img src='%3' />").arg(trick_path).arg(from).arg(to));
    }

    card_pattern = "nullification";
    m_isDiscardActionRefusable = true;
    m_isUseCard = false;

    setStatus(Responsing);
}

void Client::playAudio(const QString &name){
    Sanguosha->playAudio(name);
}

void Client::askForCardChosen(const Json::Value &ask_str){
    if (!isStringArray(ask_str, 0, 2)) return;
    QString player_name = toQString(ask_str[0]);
    QString flags = toQString(ask_str[1]);
    QString reason = toQString(ask_str[2]);
    ClientPlayer *player = getPlayer(player_name);
    if (player == NULL) return;

    PlayerCardDialog *dialog = new PlayerCardDialog(player, flags);
    dialog->setWindowTitle(Sanguosha->translate(reason));

    connect(dialog, SIGNAL(card_id_chosen(int)), this, SLOT(onPlayerChooseCard(int)));
    connect(dialog, SIGNAL(rejected()), this, SLOT(onPlayerChooseCard()));

    ask_dialog = dialog;
    setStatus(ExecDialog);
}

void Client::playCardEffect(const QString &play_str){
    QRegExp rx1("(@?\\w+):([MF])");
    QRegExp rx2("(\\w+)@(\\w+):([MF])"); // old version

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

void Client::onPlayerChooseCard(int card_id){
    Q_ASSERT(ask_dialog->inherits("PlayerCardDialog"));
    Json::Value reply = Json::Value::null;
    if(card_id != -2){   
        delete ask_dialog;
        ask_dialog = NULL;
        reply = card_id;
    }
    replyToServer(S_COMMAND_CHOOSE_CARD, reply);
    setStatus(NotActive);
}

void Client::onPlayerChoosePlayer(const Player *player){
    if(player == NULL)
        player = findChild<const Player *>(players_to_choose.first());

    if (player == NULL) return;
    replyToServer(S_COMMAND_CHOOSE_PLAYER, toJsonString(player->objectName()));    
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

void Client::requestSurrender(){
    requestToServer(S_COMMAND_SURRENDER);
    setStatus(NotActive);
}

void Client::speakToServer(const QString &text){
    if(text.isEmpty())
        return;

    QByteArray data = text.toUtf8().toBase64();
    request(QString("speak %1").arg(QString(data)));
}

void Client::addHistory(const QString &add_str){
    if(add_str == "pushPile")
    {
        emit card_used();
        return;
    }

    QRegExp rx("(.+)(#\\d+)?");
    if(rx.exactMatch(add_str)){
        QStringList texts = rx.capturedTexts();
        QString card_name = texts.at(1);
        QString times_str = texts.at(2);

        int times = 1;
        if(!times_str.isEmpty()){
            times_str.remove(QChar('#'));
            times = times_str.toInt();
        }

        Self->addHistory(card_name, times);
    }
}

int Client::alivePlayerCount() const{
    return alive_count;
}

void Client::onPlayerResponseCard(const Card *card){
    if(card)
        replyToServer(S_COMMAND_RESPONSE_CARD, toJsonString(card->toString()));
        //request(QString("responseCard %1").arg(card->toString()));
    else
        replyToServer(S_COMMAND_RESPONSE_CARD, Json::Value::null);
        //request("responseCard .");

    card_pattern.clear();
    setStatus(NotActive);
}

bool Client::hasNoTargetResponsing() const{
    return status == Responsing && !m_isUseCard;
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
    swap_pile ++;
    updatePileNum();

    emit pile_cleared();
}

void Client::setPileNumber(const QString &pile_str){
    pile_num = pile_str.toInt();

    updatePileNum();
}

void Client::setStatistics(const QString &property_str){
    QRegExp rx("(\\w+):(\\w+)");
    if(!rx.exactMatch(property_str))
        return;

    QStringList texts = rx.capturedTexts();
    QString property_name = texts.at(1);
    QString value_str = texts.at(2);

    StatisticsStruct *statistics = Self->getStatistics();
    bool ok;
    value_str.toInt(&ok);
    if(ok)
        statistics->setStatistics(property_name, value_str.toInt());
    else
        statistics->setStatistics(property_name, value_str);

    Self->setStatistics(statistics);
}

void Client::updatePileNum(){
    QString pile_str = tr("Draw pile: <b>%1</b>, discard pile: <b>%2</b>, swap times: <b>%3</b>")
                       .arg(pile_num).arg(discarded_list.length()).arg(swap_pile);

    if(skill_title.isEmpty())
        lines_doc->setHtml(pile_str);
    else
        lines_doc->setHtml(QString("%1 <br/> <b>%2</b>: %3").arg(pile_str).arg(skill_title).arg(skill_line));
}

void Client::askForDiscard(const Json::Value &req){
    
    if (!req.isArray() || !req[0].isInt() || !req[1].isBool() || !req[2].isBool())
        return;

    discard_num = req[0].asInt();    
    m_isDiscardActionRefusable = req[1].asBool();
    m_canDiscardEquip = req[2].asBool();

    QString prompt;
    if(m_canDiscardEquip)
        prompt = tr("Please discard %1 card(s), include equip").arg(discard_num);
    else
        prompt = tr("Please discard %1 card(s), only hand cards is allowed").arg(discard_num);

    prompt_doc->setHtml(prompt);

    setStatus(Discarding);
}

void Client::askForExchange(const Json::Value &exchange_str){
    if (!exchange_str.isInt())
    {
        QMessageBox::warning(NULL, tr("Warning"), tr("Exchange string is not well formatted!"));
        return;
    }

    discard_num = exchange_str.asInt();
    m_isDiscardActionRefusable = false;
    m_canDiscardEquip = false;

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

    QSet<QString> winners = winner.split("+").toSet();
    foreach(const ClientPlayer *player, players){
        QString role = player->getRole();
        bool win = winners.contains(player->objectName()) || winners.contains(role);

        ClientPlayer *p = const_cast<ClientPlayer *>(player);
        p->setProperty("win", win);
    }

    emit game_over();
}

void Client::killPlayer(const QString &player_name){
    alive_count --;

    ClientPlayer *player = getPlayer(player_name);
    if(player == Self){
        foreach(const Skill *skill, Self->getVisibleSkills())
            emit skill_detached(skill->objectName());
    }

    player->loseAllSkills();

    if(!Self->hasFlag("marshalling")){
        QString general_name = player->getGeneralName();
        QString last_word = Sanguosha->translate(QString("~%1").arg(general_name));
        if(last_word.startsWith("~")){
            QStringList origin_generals = general_name.split("_");
            if(origin_generals.length()>1)
                last_word = Sanguosha->translate(("~") +  origin_generals.at(1));
        }

        if(last_word.startsWith("~") && general_name.endsWith("f")){
            QString origin_general = general_name;
            origin_general.chop(1);
            if(Sanguosha->getGeneral(origin_general))
                last_word = Sanguosha->translate(("~") + origin_general);
        }
        skill_title = tr("%1[dead]").arg(Sanguosha->translate(general_name));
        skill_line = last_word;

        updatePileNum();
    }

    emit player_killed(player_name);
}

void Client::revivePlayer(const QString &player_name){
    alive_count ++;

    emit player_revived(player_name);
}


void Client::warn(const QString &reason){
    QString msg;
    if(reason == "GAME_OVER")
        msg = tr("Game is over now");
    else if(reason == "REQUIRE_PASSWORD")
        msg = tr("The server require password to signup");
    else if(reason == "WRONG_PASSWORD")
        msg = tr("Your password is wrong");
    else if(reason == "INVALID_FORMAT")
        msg = tr("Invalid signup string");
    else if(reason == "LEVEL_LIMITATION")
        msg = tr("Your level is not enough");
    else
        msg = tr("Unknown warning: %1").arg(reason);

    disconnectFromHost();
    QMessageBox::warning(NULL, tr("Warning"), msg);
}

void Client::askForSuit(const Json::Value &){
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

        connect(button, SIGNAL(clicked()), this, SLOT(onPlayerChooseSuit()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
    }

    connect(dialog, SIGNAL(rejected()), this, SLOT(onPlayerChooseSuit()));

    dialog->setObjectName(".");
    dialog->setWindowTitle(tr("Please choose a suit"));
    dialog->setLayout(layout);

    setStatus(ExecDialog);
}

void Client::askForKingdom(const Json::Value&){
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

        connect(button, SIGNAL(clicked()), this, SLOT(onPlayerChooseKingdom()));
        connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
    }

    dialog->setObjectName(".");
    connect(dialog, SIGNAL(rejected()), this, SLOT(onPlayerChooseKingdom()));

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

void Client::doFilter(const QString &){
    emit do_filter();
}

void Client::onPlayerChooseSuit(){
    replyToServer(S_COMMAND_CHOOSE_SUIT, toJsonString(sender()->objectName()));
    setStatus(NotActive);
}

void Client::onPlayerChooseKingdom(){
    replyToServer(S_COMMAND_CHOOSE_KINGDOM, toJsonString(sender()->objectName()));
    setStatus(NotActive);
}

void Client::onPlayerDiscardCards(const Card *cards){
    Json::Value val;
    if (!cards) val = Json::Value::null;
    else
    {
        foreach(int card_id, cards->getSubcards())        
            val.append(card_id);
    }
    replyToServer(S_COMMAND_DISCARD_CARD, val);

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

void Client::disableAG(const QString &disable_str){
    emit ag_disabled(disable_str == "true");
}

void Client::askForSinglePeach(const Json::Value &arg){
    
    if (!arg.isArray() || arg.size() != 2 || !arg[0].isString() || !arg[1].isInt()) return;

    ClientPlayer *dying = getPlayer(toQString(arg[0]));
    int peaches = arg[1].asInt();

    if(dying == Self){
        prompt_doc->setHtml(tr("You are dying, please provide %1 peach(es)(or analeptic) to save yourself").arg(peaches));
        card_pattern = "peach+analeptic";
    }else{
        QString dying_general = Sanguosha->translate(dying->getGeneralName());
        prompt_doc->setHtml(tr("%1 is dying, please provide %2 peach(es) to save him").arg(dying_general).arg(peaches));
        card_pattern = "peach";
    }

    m_isDiscardActionRefusable = true;
    m_isUseCard = false;
    setStatus(Responsing);
}

void Client::askForCardShow(const Json::Value &requestor){
    if (!requestor.isString()) return;
    QString name = Sanguosha->translate(toQString(requestor));
    prompt_doc->setHtml(tr("%1 request you to show one hand card").arg(name));

    card_pattern = ".";
    m_isDiscardActionRefusable = false;
    m_isUseCard = false;
    setStatus(Responsing);
}

void Client::askForAG(const Json::Value &arg){
    if (!arg.isBool()) return;
    m_isDiscardActionRefusable = arg.asBool();
    setStatus(AskForAG);
}

void Client::onPlayerChooseAG(int card_id){
    replyToServer(S_COMMAND_AMAZING_GRACE, card_id);
    setStatus(NotActive);
}

QList<const ClientPlayer*> Client::getPlayers() const{
    return players;
}

void Client::clearTurnTag(){
    switch(Self->getPhase()){
    case Player::Start:{
            Sanguosha->playAudio("your-turn");
            QApplication::alert(QApplication::focusWidget());
            break;
    }

    case Player::Play:{
            Self->clearHistory();
            break;
        }

    case Player::NotActive:{
            Self->clearFlags();
            break;
        }

    default:
        break;
    }
}

void Client::showCard(const Json::Value &show_str){
    
    if (!show_str.isArray() || show_str.size() != 2
        || !show_str[0].isString() || !show_str[1].isInt())
        return;

    QString player_name = toQString(show_str[0]);
    int card_id = show_str[1].asInt();

    ClientPlayer *player = getPlayer(player_name);
    if(player != Self)
        player->addKnownHandCard(Sanguosha->getCard(card_id));

    emit card_shown(player_name, card_id);
}

void Client::attachSkill(const QString &skill_name){
    Self->acquireSkill(skill_name);
    emit skill_attached(skill_name, true);
}

void Client::detachSkill(const QString &detach_str){
    QStringList texts = detach_str.split(":");
    ClientPlayer *player = NULL;
    QString skill_name;
    if(texts.length() == 1){
        player = Self;
        skill_name = texts.first();
    }else if(texts.length() == 2){
        player = getPlayer(texts.first());
        skill_name = texts.last();
    }

    player->loseSkill(skill_name);

    if(player == Self)
        emit skill_detached(skill_name);
}

void Client::askForAssign(const Json::Value &){
    emit assign_asked();
}

void Client::onPlayerAssignRole(const QList<QString> &names, const QList<QString> &roles)
{
    Q_ASSERT(names.size() == roles.size());
    Json::Value reply(Json::arrayValue);
    reply[0] = toJsonStringArray(names);
    reply[1] = toJsonStringArray(roles);    
    replyToServer(S_COMMAND_CHOOSE_ROLE, reply);
}

void Client::askForGuanxing(const Json::Value &arg){
    Json::Value deck = arg[0];
    bool up_only = arg[1].asBool();
    QList<int> card_ids;
    tryParse(deck, card_ids);
    
    emit guanxing(card_ids, up_only);
    setStatus(AskForGuanxing);
}

void Client::askForGongxin(const Json::Value &arg){
    if (!arg.isArray() || arg.size() != 3
        || !arg[0].isString() || ! arg[1].isBool())
        return;
    ClientPlayer *who = getPlayer(toQString(arg[0]));
    bool enable_heart = arg[1].asBool();   
    QList<int> card_ids;
    if (!tryParse(arg[2], card_ids)) return;    

    who->setCards(card_ids);

    emit gongxin(card_ids, enable_heart);
    setStatus(AskForGongxin);
}

void Client::onPlayerReplyGongxin(int card_id){
    Json::Value reply = Json::Value::null;
    if(card_id != -1)
        reply = card_id;    
    replyToServer(S_COMMAND_SKILL_GONGXIN, reply);
    setStatus(NotActive);
}

void Client::askForPindian(const Json::Value &ask_str){
    if (!isStringArray(ask_str, 0, 1)) return;
    QString from = toQString(ask_str[0]);
    if(from == Self->objectName())
        prompt_doc->setHtml(tr("Please play a card for pindian"));
    else{
        QString requestor = getPlayerName(from);
        prompt_doc->setHtml(tr("%1 ask for you to play a card to pindian").arg(requestor));
    }
    m_isUseCard = false;
    card_pattern = ".";
    m_isDiscardActionRefusable = false;
    setStatus(Responsing);
}

void Client::askForYiji(const Json::Value &card_list){
    if (!card_list.isArray()) return;
    int count = card_list.size();
    prompt_doc->setHtml(tr("Please distribute %1 cards as you wish").arg(count));
    //@todo: use cards directly rather than the QString
    QStringList card_str;
    for (unsigned int i = 0; i < card_list.size(); i++)
        card_str << QString::number(card_list[i].asInt());       
    card_pattern = card_str.join("+");
    setStatus(AskForYiji);
}

void Client::askForPlayerChosen(const Json::Value &players){
    if (!players.isArray() || players.size() != 2) return;
    if (!players[1].isString() || !players[0].isArray()) return;
    if (players[0].size() == 0) return;
    skill_name = toQString(players[1]);
    players_to_choose.clear();
    for (unsigned int i = 0; i < players[0].size(); i++)
        players_to_choose.push_back(toQString(players[0][i]));    

    setStatus(AskForPlayerChoose);
}

void Client::askForGeneral(const Json::Value &arg){
    QStringList generals;
    if (!tryParse(arg, generals)) return;
    emit generals_got(generals);
}

void Client::onPlayerReplyYiji(const Card *card, const Player *to){
    Json::Value req;
    if (!card) req = Json::Value::null;
    else 
    {
        req = Json::Value(Json::arrayValue);
        req[0] = toJsonIntArray(card->getSubcards());
        req[1] = toJsonString(to->objectName());
    }        
    replyToServer(S_COMMAND_SKILL_YIJI, req);

    setStatus(NotActive);
}

void Client::onPlayerReplyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards){
    Json::Value decks(Json::arrayValue);
    decks[0] = toJsonIntArray(up_cards);
    decks[1] = toJsonIntArray(down_cards);

    replyToServer(S_COMMAND_SKILL_GUANXING, decks);
    //request(QString("replyGuanxing %1:%2").arg(up_items.join("+")).arg(down_items.join("+")));

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
    emit text_spoken(text);

    if(who == "."){
        QString line = tr("<font color='red'>System: %1</font>").arg(text);
        emit line_spoken(QString("<p style=\"margin:3px 2px;\">%1</p>").arg(line));
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

    emit line_spoken(QString("<p style=\"margin:3px 2px;\">%1</p>").arg(line));
}

void Client::moveFocus(const Json::Value &focus){
    QString test = toQString(focus);
    emit focus_moved(QString(focus.asCString()));
}

void Client::setEmotion(const QString &set_str){
    QStringList words = set_str.split(":");
    QString target_name = words.at(0);
    QString emotion = words.at(1);

    emit emotion_set(target_name, emotion);
}

void Client::skillInvoked(const Json::Value &arg){
    if (!isStringArray(arg,0,1)) return;
    emit skill_invoked(QString(arg[1].asCString()), QString(arg[0].asCString()));
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

void Client::animate(const QString &animate_str){
    QStringList args = animate_str.split(":");
    QString name = args.takeFirst();

    emit animated(name, args);
}

void Client::setScreenName(const QString &set_str){
    QStringList words = set_str.split(":");
    ClientPlayer *player = getPlayer(words.first());
    QString base64 = words.at(1);
    QString screen_name = QString::fromUtf8(QByteArray::fromBase64(base64.toAscii()));
    player->setScreenName(screen_name);
}

void Client::setFixedDistance(const QString &set_str){
    QRegExp rx("(\\w+)~(\\w+)=(-?\\d+)");
    if(!rx.exactMatch(set_str))
        return;

    QStringList texts = rx.capturedTexts();
    ClientPlayer *from = getPlayer(texts.at(1));
    ClientPlayer *to = getPlayer(texts.at(2));
    int distance = texts.at(3).toInt();

    if(from && to)
        from->setFixedDistance(to, distance);
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

void Client::transfigure(const QString &transfigure_tr){
    QStringList generals = transfigure_tr.split(":");

    if(generals.length() >= 2){
        const General *furui = Sanguosha->getGeneral(generals.first());
        const General *atarashi = Sanguosha->getGeneral(generals.last());

        if(furui)foreach(const Skill *skill, furui->getVisibleSkills()){
            emit skill_detached(skill->objectName());
        }

        if(atarashi)foreach(const Skill *skill, atarashi->getVisibleSkills()){
            emit skill_attached(skill->objectName(), false);
        }
    }
}

void Client::fillGenerals(const QString &generals){
    emit generals_filled(generals.split("+"));
}

void Client::askForGeneral3v3(const QString &){
    emit general_asked();
}

void Client::takeGeneral(const QString &take_str){
    QStringList texts = take_str.split(":");
    QString who = texts.at(0);
    QString name = texts.at(1);

    emit general_taken(who, name);
}

void Client::startArrange(const QString &){
    emit arrange_started();
}

void Client::askForOrder(const Json::Value &arg){
    if (!arg.isInt()) return;
    QDialog *dialog = new QDialog;
    Game3v3ChooseOrderCommand reason = (Game3v3ChooseOrderCommand)arg.asInt();
    if (reason == S_REASON_CHOOSE_ORDER_SELECT)
        dialog->setWindowTitle(tr("The order who first choose general"));
    else if (reason == S_REASON_CHOOSE_ORDER_TURN)
        dialog->setWindowTitle(tr("The order who first in turn"));

    QLabel *prompt = new QLabel(tr("Please select the order"));
    OptionButton *warm_button = new OptionButton("image/system/3v3/warm.png", tr("Warm"));
    warm_button->setObjectName("warm");
    OptionButton *cool_button = new OptionButton("image/system/3v3/cool.png", tr("Cool"));
    cool_button->setObjectName("cool");

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(warm_button);
    hlayout->addWidget(cool_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(prompt);
    layout->addLayout(hlayout);
    dialog->setLayout(layout);

    connect(warm_button, SIGNAL(clicked()), this, SLOT(onPlayerChooseOrder()));
    connect(cool_button, SIGNAL(clicked()), this, SLOT(onPlayerChooseOrder()));
    connect(warm_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cool_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(dialog, SIGNAL(rejected()), this, SLOT(onPlayerChooseOrder()));

    ask_dialog = dialog;

    setStatus(ExecDialog);
}

void Client::askForRole3v3(const Json::Value &arg){
    if (!arg.isArray() || arg.size() != 2
        || !arg[0].isString() || !arg[1].isArray()) return;
    QStringList roleList;
    if (!tryParse(arg[1], roleList)) return;
    
    QString scheme = toQString(arg[0]);    
    QSet<QString> role_set = roleList.toSet();

    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Select role in 3v3 mode"));

    QLabel *prompt = new QLabel(tr("Please select a role"));
    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(prompt);   
    
    QStringList roles;
    if(scheme == "AllRoles")
        roles << "lord" << "loyalist" << "renegade" << "rebel";
    else
        roles << "leader1" << "guard1" << "leader2" << "guard2";

    static QMap<QString, QString> jargon;
    if(jargon.isEmpty()){
        jargon["lord"] = tr("Warm leader");
        jargon["loyalist"] = tr("Warm guard");
        jargon["renegade"] = tr("Cool leader");
        jargon["rebel"] = tr("Cool guard");

        jargon["leader1"] = tr("Leader of Team 1");
        jargon["guard1"] = tr("Guard of Team 1");
        jargon["leader2"] = tr("Leader of Team 2");
        jargon["guard2"] = tr("Guard of Team 2");
    }

    foreach(QString role, roles){
        QCommandLinkButton *button = new QCommandLinkButton(jargon[role]);
        if(scheme == "AllRoles")
            button->setIcon(QIcon(QString("image/system/roles/%1.png").arg(role)));

        layout->addWidget(button);

        if(role_set.contains(role)){
            button->setObjectName(role);
            connect(button, SIGNAL(clicked()), this, SLOT(onPlayerChooseRole3v3()));
            connect(button, SIGNAL(clicked()), dialog, SLOT(accept()));
        }else
            button->setDisabled(true);
    }

    QCommandLinkButton *abstain_button = new QCommandLinkButton(tr("Abstain"));
    connect(abstain_button, SIGNAL(clicked()), dialog, SLOT(reject()));
    layout->addWidget(abstain_button);

    dialog->setObjectName("abstain");
    connect(dialog, SIGNAL(rejected()), this, SLOT(onPlayerChooseRole3v3()));

    dialog->setLayout(layout);

    ask_dialog = dialog;

    setStatus(ExecDialog);
}

void Client::onPlayerChooseRole3v3(){
    replyToServer(S_COMMAND_CHOOSE_ROLE_3V3, toJsonString(sender()->objectName()));
    setStatus(NotActive);
}

void Client::askForDirection(const Json::Value &){
    QDialog *dialog = new QDialog;
    dialog->setWindowTitle(tr("Please select the direction"));

    QLabel *prompt = new QLabel(dialog->windowTitle());

    OptionButton *cw_button = new OptionButton("image/system/3v3/cw.png", tr("CW"));
    cw_button->setObjectName("cw");

    OptionButton *ccw_button = new OptionButton("image/system/3v3/ccw.png", tr("CCW"));
    ccw_button->setObjectName("ccw");

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(cw_button);
    hlayout->addWidget(ccw_button);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(prompt);
    layout->addLayout(hlayout);
    dialog->setLayout(layout);

    dialog->setObjectName("ccw");
    connect(ccw_button, SIGNAL(clicked()), this, SLOT(onPlayerMakeChoice()));
    connect(ccw_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(cw_button, SIGNAL(clicked()), this, SLOT(onPlayerMakeChoice()));
    connect(cw_button, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(dialog, SIGNAL(rejected()), this, SLOT(onPlayerMakeChoice()));

    ask_dialog = dialog;

    setStatus(ExecDialog);
}

void Client::recoverGeneral(const QString &recover_str){
    QRegExp rx("(\\d):(\\w+)");
    if(!rx.exactMatch(recover_str))
        return;

    QStringList texts = rx.capturedTexts();
    int index = texts.at(1).toInt();
    QString name = texts.at(2);

    emit general_recovered(index, name);
}

void Client::revealGeneral(const QString &reveal_str){
    QRegExp rx("(\\w+):(\\w+)");
    if(!rx.exactMatch(reveal_str))
        return;

    QStringList texts = rx.capturedTexts();
    bool self = texts.at(1) == Self->objectName();
    QString general = texts.at(2);

    emit general_revealed(self, general);
}

void Client::onPlayerChooseOrder(){
    OptionButton *button = qobject_cast<OptionButton *>(sender());
    QString order;
    if(button){
        order = button->objectName();
    }else{
        if(qrand() % 2 == 0)
            order = "warm";
        else
            order = "cool";
    }
    int req;
    if (order == "warm") req = (int)S_CAMP_WARM;
    else req = (int)S_CAMP_COOL;
    replyToServer(S_COMMAND_CHOOSE_ORDER, req);
    setStatus(NotActive);
}

void Client::updateStateItem(const QString &state_str)
{
    emit role_state_changed(state_str);
}
