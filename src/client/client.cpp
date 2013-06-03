#include "client.h"
#include "settings.h"
#include "engine.h"
#include "standard.h"
#include "choosegeneraldialog.h"
#include "nativesocket.h"
#include "recorder.h"
#include "jsonutils.h"
#include "SkinBank.h"

#include <QApplication>
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
    : QObject(parent), m_isDiscardActionRefusable(true),
      status(NotActive), alive_count(1), swap_pile(0),
      _m_roomState(true)
{
    ClientInstance = this;
    m_isGameOver = false;

    callbacks["checkVersion"] = &Client::checkVersion;
    callbacks["setup"] = &Client::setup;
    callbacks["networkDelayTest"] = &Client::networkDelayTest;
    callbacks["addPlayer"] = &Client::addPlayer;
    callbacks["removePlayer"] = &Client::removePlayer;
    callbacks["startInXs"] = &Client::startInXs;
    callbacks["arrangeSeats"] = &Client::arrangeSeats;
    callbacks["warn"] = &Client::warn;
    callbacks["speak"] = &Client::speak;

    m_callbacks[S_COMMAND_GAME_START] = &Client::startGame;
    m_callbacks[S_COMMAND_GAME_OVER] = &Client::gameOver;

    m_callbacks[S_COMMAND_CHANGE_HP] = &Client::hpChange;
    m_callbacks[S_COMMAND_CHANGE_MAXHP] = &Client::maxhpChange;
    m_callbacks[S_COMMAND_KILL_PLAYER] = &Client::killPlayer;
    m_callbacks[S_COMMAND_REVIVE_PLAYER] = &Client::revivePlayer;
    m_callbacks[S_COMMAND_SHOW_CARD] = &Client::showCard;
    m_callbacks[S_COMMAND_UPDATE_CARD] = &Client::updateCard;
    m_callbacks[S_COMMAND_SET_MARK] = &Client::setMark;
    m_callbacks[S_COMMAND_LOG_SKILL] = &Client::log;
    m_callbacks[S_COMMAND_ATTACH_SKILL] = &Client::attachSkill;
    m_callbacks[S_COMMAND_MOVE_FOCUS] = &Client::moveFocus; 
    m_callbacks[S_COMMAND_SET_EMOTION] = &Client::setEmotion;
    m_callbacks[S_COMMAND_INVOKE_SKILL] = &Client::skillInvoked;
    m_callbacks[S_COMMAND_SHOW_ALL_CARDS] = &Client::showAllCards;
    m_callbacks[S_COMMAND_SKILL_GONGXIN] = &Client::askForGongxin;
    m_callbacks[S_COMMAND_LOG_EVENT] = &Client::handleGameEvent;
    m_callbacks[S_COMMAND_ADD_HISTORY] = &Client::addHistory;
    m_callbacks[S_COMMAND_ANIMATE] = &Client::animate;
    m_callbacks[S_COMMAND_FIXED_DISTANCE] = &Client::setFixedDistance;
    m_callbacks[S_COMMAND_CARD_LIMITATION] = &Client::cardLimitation;
    m_callbacks[S_COMMAND_NULLIFICATION_ASKED] = &Client::setNullification;
    m_callbacks[S_COMMAND_ENABLE_SURRENDER] = &Client::enableSurrender;
    m_callbacks[S_COMMAND_EXCHANGE_KNOWN_CARDS] = &Client::exchangeKnownCards;
    m_callbacks[S_COMMAND_SET_KNOWN_CARDS] = &Client::setKnownCards;

    m_callbacks[S_COMMAND_UPDATE_STATE_ITEM] = &Client::updateStateItem;
    m_callbacks[S_COMMAND_AVAILABLE_CARDS] = &Client::setAvailableCards;

    m_callbacks[S_COMMAND_GET_CARD] = &Client::getCards;
    m_callbacks[S_COMMAND_LOSE_CARD] = &Client::loseCards;
    m_callbacks[S_COMMAND_SET_PROPERTY] = &Client::updateProperty;
    m_callbacks[S_COMMAND_RESET_PILE] = &Client::resetPiles;
    m_callbacks[S_COMMAND_UPDATE_PILE] = &Client::setPileNumber;
    m_callbacks[S_COMMAND_CARD_FLAG] = &Client::setCardFlag;

    // interactive methods    
    m_interactions[S_COMMAND_CHOOSE_GENERAL] = &Client::askForGeneral;
    m_interactions[S_COMMAND_CHOOSE_PLAYER] = &Client::askForPlayerChosen;
    m_interactions[S_COMMAND_CHOOSE_ROLE] = &Client::askForAssign;
    m_interactions[S_COMMAND_CHOOSE_DIRECTION] = &Client::askForDirection;
    m_interactions[S_COMMAND_EXCHANGE_CARD] = &Client::askForExchange;
    m_interactions[S_COMMAND_ASK_PEACH] = &Client::askForSinglePeach;
    m_interactions[S_COMMAND_SKILL_GUANXING] = &Client::askForGuanxing;
    m_interactions[S_COMMAND_SKILL_GONGXIN] = &Client::askForGongxin;
    m_interactions[S_COMMAND_SKILL_YIJI] = &Client::askForYiji;
    m_interactions[S_COMMAND_PLAY_CARD] = &Client::activate;
    m_interactions[S_COMMAND_DISCARD_CARD] = &Client::askForDiscard;
    m_interactions[S_COMMAND_CHOOSE_SUIT] = &Client::askForSuit;
    m_interactions[S_COMMAND_CHOOSE_KINGDOM] = &Client::askForKingdom;
    m_interactions[S_COMMAND_RESPONSE_CARD] = &Client::askForCardOrUseCard;
    m_interactions[S_COMMAND_INVOKE_SKILL] = &Client::askForSkillInvoke;
    m_interactions[S_COMMAND_MULTIPLE_CHOICE] = &Client::askForChoice;
    m_interactions[S_COMMAND_NULLIFICATION] = &Client::askForNullification;
    m_interactions[S_COMMAND_SHOW_CARD] = &Client::askForCardShow;
    m_interactions[S_COMMAND_AMAZING_GRACE] = &Client::askForAG;
    m_interactions[S_COMMAND_PINDIAN] = &Client::askForPindian;
    m_interactions[S_COMMAND_CHOOSE_CARD] = &Client::askForCardChosen;
    m_interactions[S_COMMAND_CHOOSE_ORDER] = &Client::askForOrder;
    m_interactions[S_COMMAND_CHOOSE_ROLE_3V3] = &Client::askForRole3v3;
    m_interactions[S_COMMAND_SURRENDER] = &Client::askForSurrender;

    m_callbacks[S_COMMAND_FILL_AMAZING_GRACE] = &Client::fillAG;
    m_callbacks[S_COMMAND_TAKE_AMAZING_GRACE] = &Client::takeAG;
    m_callbacks[S_COMMAND_CLEAR_AMAZING_GRACE] = &Client::clearAG;

    // 3v3 mode & 1v1 mode
    m_interactions[S_COMMAND_ASK_GENERAL] = &Client::askForGeneral3v3;
    m_interactions[S_COMMAND_ARRANGE_GENERAL] = &Client::startArrange;

    m_callbacks[S_COMMAND_FILL_GENERAL] = &Client::fillGenerals;
    m_callbacks[S_COMMAND_TAKE_GENERAL] = &Client::takeGeneral;
    m_callbacks[S_COMMAND_RECOVER_GENERAL] = &Client::recoverGeneral;
    m_callbacks[S_COMMAND_REVEAL_GENERAL] = &Client::revealGeneral;

    m_noNullificationThisTime = false;
    m_noNullificationTrickName = ".";

    Self = new ClientPlayer(this);
    Self->setScreenName(Config.UserName);
    Self->setProperty("avatar", Config.UserAvatar);
    connect(Self, SIGNAL(phase_changed()), this, SLOT(alertFocus()));
    connect(Self, SIGNAL(role_changed(QString)), this, SLOT(notifyRoleChange(QString)));

    players << Self;

    if (!filename.isEmpty()) {
        socket = NULL;
        recorder = NULL;

        replayer = new Replayer(this, filename);
        connect(replayer, SIGNAL(command_parsed(QString)), this, SLOT(processServerPacket(QString)));
    } else {
        socket = new NativeClientSocket;
        socket->setParent(this);

        recorder = new Recorder(this);

        connect(socket, SIGNAL(message_got(const char *)), recorder, SLOT(record(const char *)));
        connect(socket, SIGNAL(message_got(const char *)), this, SLOT(processServerPacket(const char *)));
        connect(socket, SIGNAL(error_message(QString)), this, SIGNAL(error_message(QString)));
        socket->connectToHost();

        replayer = NULL;
    }

    lines_doc = new QTextDocument(this);

    prompt_doc = new QTextDocument(this);
    prompt_doc->setTextWidth(350);
#ifdef Q_OS_LINUX
    prompt_doc->setDefaultFont(QFont("DroidSansFallback"));
#else
    prompt_doc->setDefaultFont(QFont("SimHei"));
#endif
}

Client::~Client() {
    ClientInstance = NULL;
}

void Client::updateCard(const Json::Value &val) {
    if (val.isInt()) {
        // reset card
        int cardId = val.asInt();
        Card *card = _m_roomState.getCard(cardId);
        if (!card->isModified()) return;
        _m_roomState.resetCard(cardId);
    } else {
        // update card
        Q_ASSERT(val.size() >= 5);
        int cardId = val[0].asInt();
        Card::Suit suit = (Card::Suit)val[1].asInt();
        int number = val[2].asInt();
        QString cardName = val[3].asCString();
        QString skillName = val[4].asCString();
        QString objectName = val[5].asCString();
        QStringList flags;
        tryParse(val[6], flags);

        Card *card = Sanguosha->cloneCard(cardName, suit, number, flags);
        card->setId(cardId);
        card->setSkillName(skillName);
        card->setObjectName(objectName);
        WrappedCard *wrapped = Sanguosha->getWrappedCard(cardId);
        Q_ASSERT(wrapped != NULL);
        wrapped->copyEverythingFrom(card);
    }
}

void Client::signup() {
    if (replayer)
        replayer->start();
    else {
        QString base64 = Config.UserName.toUtf8().toBase64();
        QString command = Config.value("EnableReconnection", false).toBool() ? "signupr" : "signup";
        QString signup_str = QString("%1 %2:%3").arg(command).arg(base64).arg(Config.UserAvatar);
        request(signup_str);
    }
}

void Client::networkDelayTest(const QString &) {
    request("networkDelayTest .");
}

void Client::replyToServer(CommandType command, const Json::Value &arg) {
    if (socket) {
        QSanGeneralPacket packet(S_SRC_CLIENT | S_TYPE_REPLY | S_DEST_ROOM, command);
        packet.m_localSerial = _m_lastServerSerial;
        packet.setMessageBody(arg);
        socket->send(toQString(packet.toString()));
    }
}

void Client::handleGameEvent(const Json::Value &arg) {
    emit event_received(arg);
}

void Client::requestToServer(CommandType command, const Json::Value &arg) {
    if (socket) {
        QSanGeneralPacket packet(S_SRC_CLIENT | S_TYPE_REQUEST | S_DEST_ROOM, command);
        packet.setMessageBody(arg);
        socket->send(toQString(packet.toString()));
    }
}

void Client::request(const QString &message) {
    if (socket)
        socket->send(message);
}

void Client::checkVersion(const QString &server_version) {
    QString version_number, mod_name;
    if (server_version.contains(QChar(':'))) {
        QStringList texts = server_version.split(QChar(':'));
        version_number = texts.value(0);
        mod_name = texts.value(1);
    } else {
        version_number = server_version;
        mod_name = "official";
    }

    emit version_checked(version_number, mod_name);
}

void Client::setup(const QString &setup_str) {
    if (socket && !socket->isConnected())
        return;

    if (ServerInfo.parse(setup_str)) {
        emit server_connected();
        request("toggleReady .");
    } else {
        QMessageBox::warning(NULL, tr("Warning"), tr("Setup string can not be parsed: %1").arg(setup_str));
    }
}

void Client::disconnectFromHost() {
    if (socket) {
        socket->disconnectFromHost();
        socket = NULL;
    }
}

typedef char buffer_t[65535];

void Client::processServerPacket(const QString &cmd) {
    processServerPacket(cmd.toAscii().data());
}

void Client::processServerPacket(const char *cmd) {
    if (m_isGameOver) return;
    QSanGeneralPacket packet;
    if (packet.parse(cmd)) {
        if (packet.getPacketType() == S_TYPE_NOTIFICATION) {
            CallBack callback = m_callbacks[packet.getCommandType()];
            if (callback) {            
                (this->*callback)(packet.getMessageBody());
            }
        } else if (packet.getPacketType() == S_TYPE_REQUEST) {
            if (!replayer)
                processServerRequest(packet);
        }
    } else
        processObsoleteServerPacket(cmd);
}

bool Client::processServerRequest(const QSanGeneralPacket &packet) {
    setStatus(NotActive);
    _m_lastServerSerial = packet.m_globalSerial;
    CommandType command = packet.getCommandType();
    Json::Value msg = packet.getMessageBody();    
    Countdown countdown;
    countdown.m_current = 0;
    if (!msg.isArray() || msg.size() <= 1 
        || !countdown.tryParse(msg[msg.size() - 1])) {
        countdown.m_type = Countdown::S_COUNTDOWN_USE_DEFAULT;    
        countdown.m_max = ServerInfo.getCommandTimeout(command, S_CLIENT_INSTANCE);
    }
    setCountdown(countdown);
    CallBack callback = m_interactions[command];
    if (!callback) return false;
    (this->*callback)(msg);    
    return true;
}

void Client::processObsoleteServerPacket(const QString &cmd) {
    // invoke methods
    QStringList args = cmd.trimmed().split(" ");
    QString method = args[0];

    Callback callback = callbacks.value(method, NULL);
    if (callback) {
        QString arg_str = args[1];
        (this->*callback)(arg_str);
    } else
        QMessageBox::information(NULL, tr("Warning"), tr("No such invokable method named \"%1\"").arg(method));

}

void Client::addPlayer(const QString &player_info) {
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
    alive_count++;
    emit player_added(player);
}

void Client::updateProperty(const Json::Value &arg) {
    if (!isStringArray(arg, 0, 2)) return;
    QString object_name = toQString(arg[0]);
    ClientPlayer *player = getPlayer(object_name);
    if (!player) return; 
    player->setProperty(arg[1].asCString(), toQString(arg[2]));
}

void Client::removePlayer(const QString &player_name) {
    ClientPlayer *player = findChild<ClientPlayer *>(player_name);
    if (player) {
        player->setParent(NULL);
        alive_count--;
        emit player_removed(player_name);
    }
}

bool Client::_loseSingleCard(int card_id, CardsMoveStruct move) {
    const Card *card = Sanguosha->getCard(card_id);
    if (move.from)
        move.from->removeCard(card, move.from_place);
    else {
        // @todo: synchronize discard pile when "marshal"
        if (move.from_place == Player::DiscardPile)
            discarded_list.removeOne(card);
        else if (move.from_place == Player::DrawPile && !Self->hasFlag("marshalling"))
            pile_num--;        
    }
    return true;
}

bool Client::_getSingleCard(int card_id, CardsMoveStruct move) {
    const Card *card = Sanguosha->getCard(card_id);
    if (move.to)
        move.to->addCard(card, move.to_place);
    else {
        if (move.to_place == Player::DrawPile)
            pile_num++;
        // @todo: synchronize discard pile when "marshal"
        else if (move.to_place == Player::DiscardPile)
            discarded_list.prepend(card);        
    }
    return true;
}

void Client::getCards(const Json::Value &arg) {
    Q_ASSERT(arg.isArray() && arg.size() >= 1);
    int moveId = arg[0].asInt();
    QList<CardsMoveStruct> moves;
    for (unsigned int i = 1; i < arg.size(); i++) {
        CardsMoveStruct move;
        if (!move.tryParse(arg[i])) return;
        move.from = getPlayer(move.from_player_name);
        move.to = getPlayer(move.to_player_name);
        Player::Place dstPlace = move.to_place;
    
        if (dstPlace == Player::PlaceSpecial)
            ((ClientPlayer *)move.to)->changePile(move.to_pile_name, true, move.card_ids);
        else {
            foreach (int card_id, move.card_ids)
                _getSingleCard(card_id, move); // DDHEJ->DDHEJ, DDH/EJ->EJ
        }
        moves.append(move);
    }
    updatePileNum();
    emit move_cards_got(moveId, moves);
}

void Client::loseCards(const Json::Value &arg) {
    Q_ASSERT(arg.isArray() && arg.size() >= 1);
    int moveId = arg[0].asInt();
    QList<CardsMoveStruct> moves;
    for (unsigned int i = 1; i < arg.size(); i++) {
        CardsMoveStruct move;
        if (!move.tryParse(arg[i])) return;
        move.from = getPlayer(move.from_player_name);
        move.to = getPlayer(move.to_player_name);
        Player::Place srcPlace = move.from_place;   
        
        if (srcPlace == Player::PlaceSpecial)        
            ((ClientPlayer *)move.from)->changePile(move.from_pile_name, false, move.card_ids);
        else {
            foreach (int card_id, move.card_ids)
                _loseSingleCard(card_id, move); // DDHEJ->DDHEJ, DDH/EJ->EJ
        }
        moves.append(move);
    }
    updatePileNum();
    emit move_cards_lost(moveId, moves);    
}

void Client::onPlayerChooseGeneral(const QString &item_name) {
    setStatus(NotActive);
    if (!item_name.isEmpty()) {
        replyToServer(S_COMMAND_CHOOSE_GENERAL, toJsonString(item_name));        
        Sanguosha->playSystemAudioEffect("choose-item");
    }

}

void Client::requestCheatRunScript(const QString &script) {
    Json::Value cheatReq(Json::arrayValue);
    cheatReq[0] = (int)S_CHEAT_RUN_SCRIPT;    
    cheatReq[1] = toJsonString(script);
    requestToServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatRevive(const QString &name) {
    Json::Value cheatReq(Json::arrayValue);
    cheatReq[0] = (int)S_CHEAT_REVIVE_PLAYER;    
    cheatReq[1] = toJsonString(name);
    requestToServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatDamage(const QString &source, const QString &target, DamageStruct::Nature nature, int points) {
    Json::Value cheatReq(Json::arrayValue), cheatArg(Json::arrayValue);
    cheatArg[0] = toJsonString(source);
    cheatArg[1] = toJsonString(target);
    cheatArg[2] = (int)nature;
    cheatArg[3] = points;    

    cheatReq[0] = (int)S_CHEAT_MAKE_DAMAGE;    
    cheatReq[1] = cheatArg;
    requestToServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatKill(const QString &killer, const QString &victim) {
    Json::Value cheatArg;
    cheatArg[0] = (int)S_CHEAT_KILL_PLAYER;
    cheatArg[1] = toJsonArray(killer, victim);
    requestToServer(S_COMMAND_CHEAT, cheatArg);
}

void Client::requestCheatGetOneCard(int card_id) {
    Json::Value cheatArg;
    cheatArg[0] = (int)S_CHEAT_GET_ONE_CARD;
    cheatArg[1] = card_id;
    requestToServer(S_COMMAND_CHEAT, cheatArg);
}

void Client::requestCheatChangeGeneral(const QString &name, bool isSecondaryHero) {
    Json::Value cheatArg;
    cheatArg[0] = (int)S_CHEAT_CHANGE_GENERAL;
    cheatArg[1] = toJsonString(name);
    cheatArg[2] = isSecondaryHero;
    requestToServer(S_COMMAND_CHEAT, cheatArg);
}

void Client::addRobot() {
    request("addRobot .");
}

void Client::fillRobots() {
    request("fillRobots .");
}

void Client::arrange(const QStringList &order) {
    Q_ASSERT(order.length() == 3);

    request(QString("arrange %1").arg(order.join("+")));
}

void Client::onPlayerResponseCard(const Card *card, const QList<const Player *> &targets) {
    if ((status & ClientStatusBasicMask) == Responding)
        _m_roomState.setCurrentCardUsePattern(QString());
    if (card == NULL) {
        replyToServer(S_COMMAND_RESPONSE_CARD, Json::Value::null);
    } else {
        Json::Value targetNames(Json::arrayValue);
        if (!card->targetFixed()) {
            foreach (const Player *target, targets)
                targetNames.append(toJsonString(target->objectName()));
        }

        replyToServer(S_COMMAND_RESPONSE_CARD, toJsonArray(card->toString(), targetNames));

        if (card->isVirtualCard() && !card->parent())
            delete card;
    }

    if (Self->hasFlag("Client_PreventPeach")) {
        Self->setFlags("-Client_PreventPeach");
        Self->removeCardLimitation("use", "Peach$0");
    }
    setStatus(NotActive);
}

void Client::startInXs(const QString &left_seconds) {
    int seconds = left_seconds.toInt();
    if (seconds > 0)
        lines_doc->setHtml(tr("<p align = \"center\">Game will start in <b>%1</b> seconds...</p>").arg(left_seconds));
    else
        lines_doc->setHtml(QString());

    emit start_in_xs();
    if (seconds == 0 && Sanguosha->getScenario(ServerInfo.GameMode) == NULL) {
        emit avatars_hiden();
    }
}

void Client::arrangeSeats(const QString &seats_str) {
    QStringList player_names = seats_str.split("+");
    players.clear();
    
    for (int i = 0; i < player_names.length(); i++) {
        ClientPlayer *player = findChild<ClientPlayer *>(player_names.at(i));

        Q_ASSERT(player != NULL);

        player->setSeat(i + 1);
        players << player;
    }

    QList<const ClientPlayer *> seats;
    int self_index = players.indexOf(Self);

    Q_ASSERT(self_index != -1);

    for (int i = self_index + 1; i < players.length(); i++)
        seats.append(players.at(i));
    for (int i = 0; i < self_index; i++)
        seats.append(players.at(i));

    Q_ASSERT(seats.length() == players.length() - 1);

    emit seats_arranged(seats);
}

void Client::notifyRoleChange(const QString &new_role) {
    if (!new_role.isEmpty()) {
        QString prompt_str = tr("Your role is %1").arg(Sanguosha->translate(new_role));
        if (new_role != "lord")
            prompt_str += tr("\n wait for the lord player choosing general, please");
        lines_doc->setHtml(QString("<p align = \"center\">%1</p>").arg(prompt_str));
    }
}

void Client::activate(const Json::Value &playerId) {
    setStatus(toQString(playerId) == Self->objectName() ? Playing : NotActive);
}

void Client::startGame(const Json::Value &) {
    Sanguosha->registerRoom(this);
    _m_roomState.reset();

    QList<ClientPlayer *> players = findChildren<ClientPlayer *>();
    alive_count = players.count();

    emit game_started();
}

void Client::hpChange(const Json::Value &change_str) {
    if (!change_str.isArray() || change_str.size() != 3) return;
    if (!change_str[0].isString() || !change_str[1].isInt() || !change_str[2].isInt()) return;

    QString who = toQString(change_str[0]);
    int delta = change_str[1].asInt();

    int nature_index = change_str[2].asInt();
    DamageStruct::Nature nature = DamageStruct::Normal;
    if (nature_index > 0) nature = (DamageStruct::Nature)nature_index;

    emit hp_changed(who, delta, nature, nature_index == -1);
}

void Client::maxhpChange(const Json::Value &change_str) {
    if (!change_str.isArray() || change_str.size() != 2) return;
    if (!change_str[0].isString() || !change_str[1].isInt()) return;

    QString who = toQString(change_str[0]);
    int delta = change_str[1].asInt();
    emit maxhp_changed(who, delta);
}

void Client::setStatus(Status status) {
    Status old_status = this->status;
    this->status = status;
    if (status == Client::Playing)
        _m_roomState.setCurrentCardUseReason(CardUseStruct::CARD_USE_REASON_PLAY);
    else if (status == Responding)
        _m_roomState.setCurrentCardUseReason(CardUseStruct::CARD_USE_REASON_RESPONSE);
    else if (status == RespondingUse)
        _m_roomState.setCurrentCardUseReason(CardUseStruct::CARD_USE_REASON_RESPONSE_USE);
    else
        _m_roomState.setCurrentCardUseReason(CardUseStruct::CARD_USE_REASON_UNKNOWN);
    emit status_changed(old_status, status);
}

Client::Status Client::getStatus() const{
    return status;
}

void Client::cardLimitation(const Json::Value &limit) {
    if (!limit.isArray() || limit.size() != 4) return;

    bool set = limit[0].asBool();
    bool single_turn = limit[3].asBool();
    if (limit[1].isNull() && limit[2].isNull()) {
        Self->clearCardLimitation(single_turn);
    } else {
        if (!limit[1].isString() || !limit[2].isString()) return;
        QString limit_list = toQString(limit[1]);
        QString pattern = toQString(limit[2]);
        if (set)
            Self->setCardLimitation(limit_list, pattern, single_turn);
        else
            Self->removeCardLimitation(limit_list, pattern);
    }
}

void Client::setNullification(const Json::Value &str) {
    if (!str.isString()) return;
    QString astr = toQString(str);
    if (astr != ".") {
        if (m_noNullificationTrickName == ".") {
            m_noNullificationThisTime = false;
            m_noNullificationTrickName = astr;
            emit nullification_asked(true);
        }
    } else {
        m_noNullificationThisTime = false;
        m_noNullificationTrickName = ".";
        emit nullification_asked(false);
    }
}

void Client::enableSurrender(const Json::Value &enabled) {
    if (!enabled.isBool()) return;
    bool en = enabled.asBool();
    emit surrender_enabled(en);
}

void Client::exchangeKnownCards(const Json::Value &players) {
    if (!players.isArray() || players.size() != 2 || !players[0].isString() || !players[1].isString()) return;
    ClientPlayer *a = getPlayer(toQString(players[0])), *b = getPlayer(toQString(players[1]));
    QList<int> a_known, b_known;
    foreach (const Card *card, a->getHandcards())
        a_known << card->getId();
    foreach (const Card *card, b->getHandcards())
        b_known << card->getId();
    a->setCards(b_known);
    b->setCards(a_known);
}

void Client::setKnownCards(const Json::Value &set_str) {
    if (!set_str.isArray() || set_str.size() != 2) return;
    QString name = toQString(set_str[0]);
    ClientPlayer *player = getPlayer(name);
    if (player == NULL) return;
    QList<int> ids;
    tryParse(set_str[1], ids);
    player->setCards(ids);

}

Replayer *Client::getReplayer() const{
    return replayer;
}

QString Client::getPlayerName(const QString &str) {
    QRegExp rx("sgs\\d+");
    QString general_name;
    if (rx.exactMatch(str)) {
        ClientPlayer *player = getPlayer(str);
        general_name = player->getGeneralName();
        general_name = Sanguosha->translate(general_name);
        if (player->getGeneral2())
            general_name.append("/" + Sanguosha->translate(player->getGeneral2Name()));
        if (ServerInfo.EnableSame || player->getGeneralName() == "anjiang")
            general_name = QString("%1[%2]").arg(general_name).arg(player->getSeat());
        return general_name;
    } else
        return Sanguosha->translate(str);
}

QString Client::getSkillNameToInvoke() const{
    return skill_to_invoke;
}

void Client::onPlayerInvokeSkill(bool invoke) {
    if (skill_name == "surrender")
        replyToServer(S_COMMAND_SURRENDER, invoke);
    else
        replyToServer(S_COMMAND_INVOKE_SKILL, invoke);
    setStatus(NotActive);
}

QString Client::setPromptList(const QStringList &texts) {
    QString prompt = Sanguosha->translate(texts.at(0));
    if (texts.length() >= 2)
        prompt.replace("%src", getPlayerName(texts.at(1)));

    if (texts.length() >= 3)
        prompt.replace("%dest", getPlayerName(texts.at(2)));

    if (texts.length() >= 5) {
        QString arg2 = Sanguosha->translate(texts.at(4));
        prompt.replace("%arg2", arg2);
    }

    if (texts.length() >= 4) {
        QString arg = Sanguosha->translate(texts.at(3));
        prompt.replace("%arg", arg);
    }

    prompt_doc->setHtml(prompt);
    return prompt;
}

void Client::commandFormatWarning(const QString &str, const QRegExp &rx, const char *command) {
    QString text = tr("The argument (%1) of command %2 does not conform the format %3")
                      .arg(str).arg(command).arg(rx.pattern());
    QMessageBox::warning(NULL, tr("Command format warning"), text);
}

QString Client::_processCardPattern(const QString &pattern) {
    const QChar c = pattern.at(pattern.length() - 1);
    if (c == '!' || c.isNumber())
        return pattern.left(pattern.length() - 1);

    return pattern;
}

void Client::askForCardOrUseCard(const Json::Value &cardUsage) {
    if (!cardUsage.isArray() || !cardUsage[0].isString() || !cardUsage[1].isString())
        return;
    QString card_pattern = toQString(cardUsage[0]);
    _m_roomState.setCurrentCardUsePattern(card_pattern);
    QStringList texts = toQString(cardUsage[1]).split(":");
    int index = -1;
    if (cardUsage[3].isInt() && cardUsage[3].asInt() > 0)
        index = cardUsage[3].asInt();

    if (texts.isEmpty())
        return;
    else
        setPromptList(texts);

    if (card_pattern.endsWith("!"))
        m_isDiscardActionRefusable = false;
    else
        m_isDiscardActionRefusable = true;

    QString temp_pattern = _processCardPattern(card_pattern);
    QRegExp rx("^@@?(\\w+)(-card)?$");
    if (rx.exactMatch(temp_pattern)) {
        QString skill_name = rx.capturedTexts().at(1);
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if (skill) {
            QString text = prompt_doc->toHtml();
            text.append(tr("<br/> <b>Notice</b>: %1<br/>").arg(skill->getNotice(index)));
            prompt_doc->setHtml(text);
        }
    }

    Status status = Responding;
    if (cardUsage[2].isInt()) {
        Card::HandlingMethod method = (Card::HandlingMethod)(cardUsage[2].asInt());
        switch (method) {
        case Card::MethodDiscard: status = RespondingForDiscard; break;
        case Card::MethodUse: status = RespondingUse; break;
        case Card::MethodResponse: status = Responding; break;
        default: status = RespondingNonTrigger; break;
        }
    }
    setStatus(status);
}

void Client::askForSkillInvoke(const Json::Value &arg) {
    if (!isStringArray(arg, 0, 1)) return;
    QString skill_name = toQString(arg[0]);
    QString data = toQString(arg[1]);

    skill_to_invoke = skill_name;
        
    QString text;
    if (data.isEmpty()) {
        text = tr("Do you want to invoke skill [%1] ?").arg(Sanguosha->translate(skill_name));
        prompt_doc->setHtml(text);
    } else {
        QStringList texts = data.split(":");
        text = QString("%1:%2").arg(skill_name).arg(texts.first());
        texts.replace(0, text);
        setPromptList(texts);
    }

    setStatus(AskForSkillInvoke);
}

void Client::onPlayerMakeChoice() {
    QString option = sender()->objectName();
    replyToServer(S_COMMAND_MULTIPLE_CHOICE, toJsonString(option));    
    setStatus(NotActive);
}

void Client::askForSurrender(const Json::Value &initiator) {
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

void Client::askForNullification(const Json::Value &arg) {
    if (!arg.isArray() || arg.size() != 3 || !arg[0].isString()
        || !(arg[1].isNull() || arg[1].isString())
        || !arg[2].isString())
        return;
    
    QString trick_name = toQString(arg[0]);
    Json::Value source_name = arg[1];
    ClientPlayer *target_player = getPlayer(toQString(arg[2]));

    if (!target_player || !target_player->getGeneral()) return;

    ClientPlayer *source = NULL;
    if (source_name != Json::Value::null)
        source = getPlayer(source_name.asCString());

    const Card *trick_card = Sanguosha->findChild<const Card *>(trick_name);
    if (Config.NeverNullifyMyTrick && source == Self) {
        if (trick_card->isKindOf("SingleTargetTrick") || trick_card->isKindOf("IronChain")) {
            onPlayerResponseCard(NULL);
            return;
        }
    }
    if (m_noNullificationThisTime && m_noNullificationTrickName == trick_name) {
        if (trick_card->isKindOf("AOE") || trick_card->isKindOf("GlobalEffect")) {
            onPlayerResponseCard(NULL);
            return;
        }
    }

    if (source == NULL) {
        prompt_doc->setHtml(tr("Do you want to use nullification to trick card %1 from %2?")
                               .arg(Sanguosha->translate(trick_card->objectName()))
                               .arg(getPlayerName(target_player->objectName())));
    } else {
        prompt_doc->setHtml(tr("%1 used trick card %2 to %3 <br>Do you want to use nullification?")
                               .arg(getPlayerName(source->objectName()))
                               .arg(Sanguosha->translate(trick_name))
                               .arg(getPlayerName(target_player->objectName())));
    }

    _m_roomState.setCurrentCardUsePattern("nullification");
    m_isDiscardActionRefusable = true;

    setStatus(RespondingUse);
}

void Client::onPlayerChooseCard(int card_id) {
    Json::Value reply = Json::Value::null;
    if (card_id != -2)
        reply = card_id;
    replyToServer(S_COMMAND_CHOOSE_CARD, reply);
    setStatus(NotActive);
}

void Client::onPlayerChoosePlayer(const Player *player) {
    if (player == NULL && !m_isDiscardActionRefusable)
        player = findChild<const Player *>(players_to_choose.first());

    replyToServer(S_COMMAND_CHOOSE_PLAYER, (player == NULL) ? Json::Value::null : toJsonString(player->objectName()));
    setStatus(NotActive);
}

void Client::trust() {
    request("trust .");

    if (Self->getState() == "trust")
        Sanguosha->playSystemAudioEffect("untrust");
    else
        Sanguosha->playSystemAudioEffect("trust");

    setStatus(NotActive);
}

void Client::requestSurrender() {
    requestToServer(S_COMMAND_SURRENDER);
    setStatus(NotActive);
}

void Client::speakToServer(const QString &text) {
    if (text.isEmpty())
        return;

    QByteArray data = text.toUtf8().toBase64();
    request(QString("speak %1").arg(QString(data)));
}

void Client::addHistory(const Json::Value &history) {
    if (!history.isArray() || history.size() != 2) return;
    if (!history[0].isString() || !history[1].isInt()) return;

    QString add_str = toQString(history[0]);
    int times = history[1].asInt();
    if (add_str == "pushPile") {
        emit card_used();
        return;
    } else if (add_str == ".") {
        Self->clearHistory();
        return;
    }

    Self->addHistory(add_str, times);
}

int Client::alivePlayerCount() const{
    return alive_count;
}

ClientPlayer *Client::getPlayer(const QString &name) {
    if (name == Self->objectName() ||
        name == QSanProtocol::S_PLAYER_SELF_REFERENCE_ID)
        return Self;
    else
        return findChild<ClientPlayer *>(name);
}

bool Client::save(const QString &filename) const{
    if (recorder)
        return recorder->save(filename);
    else
        return false;
}

QList<QString> Client::getRecords() const{
    if (recorder)
        return recorder->getRecords();
    else
        return QList<QString>();
}

QString Client::getReplayPath() const{
    if (replayer)
        return replayer->getPath();
    else
        return QString();
}

void Client::setLines(const QString &filename) {
    QRegExp rx(".+/(\\w+\\d?).ogg");
    if (rx.exactMatch(filename)) {
        QString skill_name = rx.capturedTexts().at(1);

        QChar last_char = skill_name[skill_name.length() - 1];
        if (last_char.isDigit())
            skill_name.chop(1);
    }
}

QTextDocument *Client::getLinesDoc() const{
    return lines_doc;
}

QTextDocument *Client::getPromptDoc() const{
    return prompt_doc;
}

void Client::resetPiles(const Json::Value &) {
    discarded_list.clear();
    swap_pile++;
    updatePileNum();
    emit pile_reset();
}

void Client::setPileNumber(const Json::Value &pile_str) {
    if (!pile_str.isInt()) return;
    pile_num = pile_str.asInt();
    updatePileNum();
}

void Client::setCardFlag(const Json::Value &pattern_str) {
    if (!pattern_str.isArray() || pattern_str.size() != 2) return;
    if (!pattern_str[0].isInt() || !pattern_str[1].isString()) return;

    int id = pattern_str[0].asInt();
    QString flag = toQString(pattern_str[1]);
    Sanguosha->getCard(id)->setFlags(flag);
}

void Client::updatePileNum() {
    QString pile_str = tr("Draw pile: <b>%1</b>, discard pile: <b>%2</b>, swap times: <b>%3</b>")
                       .arg(pile_num).arg(discarded_list.length()).arg(swap_pile);
    lines_doc->setHtml(QString("<font color='%1'><p align = \"center\">" + pile_str + "</p></font>").arg(Config.TextEditColor.name()));
}

void Client::askForDiscard(const Json::Value &req) {
    if (!req.isArray() || !req[0].isInt() || !req[1].isInt() || !req[2].isBool() || !req[3].isBool())
        return;

    discard_num = req[0].asInt();    
    min_num = req[1].asInt();
    m_isDiscardActionRefusable = req[2].asBool();
    m_canDiscardEquip = req[3].asBool();
    QString prompt = req[4].asCString();

    if (prompt.isEmpty()) {
        if (m_canDiscardEquip)
            prompt = tr("Please discard %1 card(s), include equip").arg(discard_num);
        else
            prompt = tr("Please discard %1 card(s), only hand cards is allowed").arg(discard_num);
        prompt_doc->setHtml(prompt);
    } else {
        QStringList texts = prompt.split(":");
        if (texts.length() < 4) {
            while (texts.length() < 3)
                texts.append(QString());
            texts.append(QString::number(discard_num));
        }
        setPromptList(texts);
    }

    setStatus(Discarding);
}

void Client::askForExchange(const Json::Value &exchange_str) {
    if (!exchange_str.isArray() || !exchange_str[0].isInt() || !exchange_str[1].isBool()
        || !exchange_str[2].isString() || !exchange_str[3].isBool()) {
        QMessageBox::warning(NULL, tr("Warning"), tr("Exchange string is not well formatted!"));
        return;
    }

    discard_num = exchange_str[0].asInt();
    m_canDiscardEquip = exchange_str[1].asBool();
    QString prompt = exchange_str[2].asCString();
    min_num = discard_num;
    m_isDiscardActionRefusable = exchange_str[3].asBool();

    if (prompt.isEmpty()) {
        prompt = tr("Please give %1 cards to exchange").arg(discard_num);
        prompt_doc->setHtml(prompt);
    } else {
        QStringList texts = prompt.split(":");
        if (texts.length() < 4) {
            while (texts.length() < 3)
                texts.append(QString());
            texts.append(QString::number(discard_num));
        }
        setPromptList(texts);
    }
    setStatus(Exchanging);
}

void Client::gameOver(const Json::Value &arg) {
    disconnectFromHost();
    m_isGameOver = true;
    setStatus(Client::NotActive);
    QString winner = toQString(arg[0]);
    QStringList roles;
    tryParse(arg[1], roles);

    Q_ASSERT(roles.length() == players.length());

    for (int i = 0; i < roles.length(); i++) {
        QString name = players.at(i)->objectName();
        getPlayer(name)->setRole(roles.at(i));
    }

    if (winner == ".") {
        emit standoff();
        return;
    }

    QSet<QString> winners = winner.split("+").toSet();
    foreach (const ClientPlayer *player, players) {
        QString role = player->getRole();
        bool win = winners.contains(player->objectName()) || winners.contains(role);

        ClientPlayer *p = const_cast<ClientPlayer *>(player);
        p->setProperty("win", win);
    }

    Sanguosha->unregisterRoom();
    emit game_over();
}

void Client::killPlayer(const Json::Value &player_arg) {
    if (!player_arg.isString()) return;
    QString player_name = toQString(player_arg);

    alive_count--;
    ClientPlayer *player = getPlayer(player_name);
    if (player == Self) {
        foreach (const Skill *skill, Self->getVisibleSkills())
            emit skill_detached(skill->objectName());
    }
    player->detachAllSkills();

    if (!Self->hasFlag("marshalling")) {
        QString general_name = player->getGeneralName();
        QString last_word = Sanguosha->translate(QString("~%1").arg(general_name));
        if (last_word.startsWith("~")) {
            QStringList origin_generals = general_name.split("_");
            if (origin_generals.length() > 1)
                last_word = Sanguosha->translate(("~") +  origin_generals.at(1));
        }

        if (last_word.startsWith("~") && general_name.endsWith("f")) {
            QString origin_general = general_name;
            origin_general.chop(1);
            if (Sanguosha->getGeneral(origin_general))
                last_word = Sanguosha->translate(("~") + origin_general);
        }
        updatePileNum();
    }

    emit player_killed(player_name);
}

void Client::revivePlayer(const Json::Value &player_arg) {
    if (!player_arg.isString()) return;
    QString player_name = toQString(player_arg);

    alive_count++;
    emit player_revived(player_name);
}


void Client::warn(const QString &reason) {
    QString msg;
    if (reason == "GAME_OVER")
        msg = tr("Game is over now");
    else if (reason == "INVALID_FORMAT")
        msg = tr("Invalid signup string");
    else if (reason == "LEVEL_LIMITATION")
        msg = tr("Your level is not enough");
    else
        msg = tr("Unknown warning: %1").arg(reason);

    disconnectFromHost();
    QMessageBox::warning(NULL, tr("Warning"), msg);
}

void Client::askForGeneral(const Json::Value &arg) {
    QStringList generals;
    if (!tryParse(arg, generals)) return;
    emit generals_got(generals);
    setStatus(ExecDialog);
}


void Client::askForSuit(const Json::Value &) {
    QStringList suits;
    suits << "spade" << "club" << "heart" << "diamond";
    emit suits_got(suits);
    setStatus(ExecDialog);
}

void Client::askForKingdom(const Json::Value &) {
    QStringList kingdoms = Sanguosha->getKingdoms();
    kingdoms.removeOne("god"); // god kingdom does not really exist
    emit kingdoms_got(kingdoms);
    setStatus(ExecDialog);
}

void Client::askForChoice(const Json::Value &ask_str) {
    if (!isStringArray(ask_str, 0, 1)) return;        
    QString skill_name = toQString(ask_str[0]);
    QStringList options = toQString(ask_str[1]).split("+");
    emit options_got(skill_name, options);
    setStatus(ExecDialog);
}

void Client::askForCardChosen(const Json::Value &ask_str) {
    if (!ask_str.isArray() || ask_str.size() != 5 || !isStringArray(ask_str, 0, 2)
        || !ask_str[3].isBool() || !ask_str[4].isInt()) return;
    QString player_name = toQString(ask_str[0]);
    QString flags = toQString(ask_str[1]);
    QString reason = toQString(ask_str[2]);
    bool handcard_visible = ask_str[3].asBool();
    Card::HandlingMethod method = (Card::HandlingMethod)ask_str[4].asInt();
    ClientPlayer *player = getPlayer(player_name);
    if (player == NULL) return;
    emit cards_got(player, flags, reason, handcard_visible, method);
    setStatus(ExecDialog);
}


void Client::askForOrder(const Json::Value &arg) {
    if (!arg.isInt()) return;    
    Game3v3ChooseOrderCommand reason = (Game3v3ChooseOrderCommand)arg.asInt();
    emit orders_got(reason);
    setStatus(ExecDialog);
}

void Client::askForRole3v3(const Json::Value &arg) {
    if (!arg.isArray() || arg.size() != 2
        || !arg[0].isString() || !arg[1].isArray()) return;
    QStringList roles;
    if (!tryParse(arg[1], roles)) return;    
    QString scheme = toQString(arg[0]);
    emit roles_got(scheme, roles);
    setStatus(ExecDialog);
}

void Client::askForDirection(const Json::Value &) {
    emit directions_got();
    setStatus(ExecDialog);
}


void Client::setMark(const Json::Value &mark_str) {
    if (!mark_str.isArray() || mark_str.size() != 3) return;
    if (!mark_str[0].isString() || !mark_str[1].isString() || !mark_str[2].isInt()) return;

    QString who = toQString(mark_str[0]);
    QString mark = toQString(mark_str[1]);
    int value = mark_str[2].asInt();

    ClientPlayer *player = getPlayer(who);
    player->setMark(mark, value);
}

void Client::onPlayerChooseSuit() {
    replyToServer(S_COMMAND_CHOOSE_SUIT, toJsonString(sender()->objectName()));
    setStatus(NotActive);
}

void Client::onPlayerChooseKingdom() {
    replyToServer(S_COMMAND_CHOOSE_KINGDOM, toJsonString(sender()->objectName()));
    setStatus(NotActive);
}

void Client::onPlayerDiscardCards(const Card *cards) {
    Json::Value val;
    if (!cards)
        val = Json::Value::null;
    else {
        foreach (int card_id, cards->getSubcards())
            val.append(card_id);
        if (cards->isVirtualCard() && !cards->parent())
            delete cards;
    }
    replyToServer(S_COMMAND_DISCARD_CARD, val);

    setStatus(NotActive);
}

void Client::fillAG(const Json::Value &cards_str) {
    if (!cards_str.isArray() || cards_str.size() != 2) return;
    QList<int> card_ids, disabled_ids;
    tryParse(cards_str[0], card_ids);
    tryParse(cards_str[1], disabled_ids);
    emit ag_filled(card_ids, disabled_ids);
}

void Client::takeAG(const Json::Value &take_str) {
    if (!take_str.isArray() || take_str.size() != 3) return;
    if (!take_str[1].isInt() || !take_str[2].isBool()) return;

    int card_id = take_str[1].asInt();
    bool move_cards = take_str[2].asBool();
    const Card *card = Sanguosha->getCard(card_id);

    if (take_str[0].isNull()) {
        if (move_cards) {
            discarded_list.prepend(card);
            updatePileNum();
        }
        emit ag_taken(NULL, card_id, move_cards);
    } else {
        ClientPlayer *taker = getPlayer(toQString(take_str[0]));
		if (move_cards)
            taker->addCard(card, Player::PlaceHand);
        emit ag_taken(taker, card_id, move_cards);
    }
}

void Client::clearAG(const Json::Value &) {
    emit ag_cleared();
}

void Client::askForSinglePeach(const Json::Value &arg) {
    if (!arg.isArray() || arg.size() != 2 || !arg[0].isString() || !arg[1].isInt()) return;

    ClientPlayer *dying = getPlayer(toQString(arg[0]));
    int peaches = arg[1].asInt();

    // @todo: anti-cheating of askForSinglePeach is not done yet!!!
    QStringList pattern;
    if (dying == Self) {
        prompt_doc->setHtml(tr("You are dying, please provide %1 peach(es)(or analeptic) to save yourself").arg(peaches));
        pattern << "peach" << "analeptic";
        _m_roomState.setCurrentCardUsePattern("peach+analeptic");
    } else {
        QString dying_general = getPlayerName(dying->objectName());
        prompt_doc->setHtml(tr("%1 is dying, please provide %2 peach(es) to save him").arg(dying_general).arg(peaches));
        pattern << "peach";
        _m_roomState.setCurrentCardUsePattern("peach");
    }
    if (Self->hasFlag("Global_PreventPeach")) {
        bool has_skill = false;
        foreach (const Skill *skill, Self->getVisibleSkillList(true)) {
            const ViewAsSkill *view_as_skill = ViewAsSkill::parseViewAsSkill(skill);
            if (view_as_skill && view_as_skill->isAvailable(Self, CardUseStruct::CARD_USE_REASON_RESPONSE_USE, pattern.join("+"))) {
                has_skill = true;
                break;
            }
        }
        if (!has_skill) {
            pattern.removeOne("peach");
            if (pattern.isEmpty()) {
                onPlayerResponseCard(NULL);
                return;
            }
        } else {
            Self->setFlags("Client_PreventPeach");
            Self->setCardLimitation("use", "Peach");
        }
    }
    _m_roomState.setCurrentCardUsePattern(pattern.join("+"));
    m_isDiscardActionRefusable = true;
    setStatus(RespondingUse);
}

void Client::askForCardShow(const Json::Value &requestor) {
    if (!requestor.isString()) return;
    QString name = Sanguosha->translate(toQString(requestor));
    prompt_doc->setHtml(tr("%1 request you to show one hand card").arg(name));

    _m_roomState.setCurrentCardUsePattern(".");
    setStatus(AskForShowOrPindian);
}

void Client::askForAG(const Json::Value &arg) {
    if (!arg.isBool()) return;
    m_isDiscardActionRefusable = arg.asBool();
    setStatus(AskForAG);
}

void Client::onPlayerChooseAG(int card_id) {
    replyToServer(S_COMMAND_AMAZING_GRACE, card_id);
    setStatus(NotActive);
}

QList<const ClientPlayer *> Client::getPlayers() const{
    return players;
}

void Client::alertFocus() {
    if (Self->getPhase() == Player::Play)
        QApplication::alert(QApplication::focusWidget());
}

void Client::showCard(const Json::Value &show_str) {
    if (!show_str.isArray() || show_str.size() != 2
        || !show_str[0].isString() || !show_str[1].isInt())
        return;

    QString player_name = toQString(show_str[0]);
    int card_id = show_str[1].asInt();

    ClientPlayer *player = getPlayer(player_name);
    if (player != Self)
        player->addKnownHandCard(Sanguosha->getCard(card_id));

    emit card_shown(player_name, card_id);
}

void Client::attachSkill(const Json::Value &skill) {
    if (!skill.isString()) return;

    QString skill_name = toQString(skill);
    Self->acquireSkill(skill_name);
    emit skill_attached(skill_name, true);
}

void Client::askForAssign(const Json::Value &) {
    emit assign_asked();
}

void Client::onPlayerAssignRole(const QList<QString> &names, const QList<QString> &roles) {
    Q_ASSERT(names.size() == roles.size());
    Json::Value reply(Json::arrayValue);
    reply[0] = toJsonArray(names);
    reply[1] = toJsonArray(roles);    
    replyToServer(S_COMMAND_CHOOSE_ROLE, reply);
}

void Client::askForGuanxing(const Json::Value &arg) {
    Json::Value deck = arg[0];
    bool up_only = arg[1].asBool();
    QList<int> card_ids;
    tryParse(deck, card_ids);
    
    emit guanxing(card_ids, up_only);
    setStatus(AskForGuanxing);
}

void Client::showAllCards(const Json::Value &arg) {
    if (!arg.isArray() || arg.size() != 3
        || !arg[0].isString() || ! arg[1].isBool())
        return;
    ClientPlayer *who = getPlayer(toQString(arg[0]));
    QList<int> card_ids;
    if (!tryParse(arg[2], card_ids)) return;

    if (who) who->setCards(card_ids);

    emit gongxin(card_ids, false);
}

void Client::askForGongxin(const Json::Value &arg) {
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

void Client::onPlayerReplyGongxin(int card_id) {
    Json::Value reply = Json::Value::null;
    if (card_id != -1)
        reply = card_id;    
    replyToServer(S_COMMAND_SKILL_GONGXIN, reply);
    setStatus(NotActive);
}

void Client::askForPindian(const Json::Value &ask_str) {
    if (!isStringArray(ask_str, 0, 1)) return;
    QString from = toQString(ask_str[0]);
    if (from == Self->objectName())
        prompt_doc->setHtml(tr("Please play a card for pindian"));
    else {
        QString requestor = getPlayerName(from);
        prompt_doc->setHtml(tr("%1 ask for you to play a card to pindian").arg(requestor));
    }
    _m_roomState.setCurrentCardUsePattern(".");
    setStatus(AskForShowOrPindian);
}

void Client::askForYiji(const Json::Value &ask_str) {
    if (!ask_str.isArray() || ask_str.size() != 4) return;
    //if (!ask_str[0].isArray() || !ask_str[1].isBool() || !ask_str[2].isInt()) return;
    Json::Value card_list = ask_str[0];
    int count = ask_str[2].asInt();
    m_isDiscardActionRefusable = ask_str[1].asBool();
    prompt_doc->setHtml(tr("Please distribute %1 cards %2 as you wish")
                           .arg(count)
                           .arg(m_isDiscardActionRefusable ? QString() : tr("to another player")));
    //@todo: use cards directly rather than the QString
    QStringList card_str;
    for (unsigned int i = 0; i < card_list.size(); i++)
        card_str << QString::number(card_list[i].asInt());
    Json::Value players = ask_str[3];
    QStringList names;
    tryParse(players, names);
    _m_roomState.setCurrentCardUsePattern(QString("%1=%2=%3").arg(count).arg(card_str.join("+")).arg(names.join("+")));
    setStatus(AskForYiji);
}

void Client::askForPlayerChosen(const Json::Value &players) {
    if (!players.isArray() || players.size() != 4) return;
    if (!players[1].isString() || !players[0].isArray() || !players[3].isBool()) return;
    if (players[0].size() == 0) return;
    skill_name = toQString(players[1]);
    players_to_choose.clear();
    for (unsigned int i = 0; i < players[0].size(); i++)
        players_to_choose.push_back(toQString(players[0][i]));
    m_isDiscardActionRefusable = players[3].asBool();

    QString text;
    QString description = Sanguosha->translate(ClientInstance->skill_name);
    QString prompt = toQString(players[2]);
    if (!prompt.isEmpty()) {
        QStringList texts = prompt.split(":");
        text = setPromptList(texts);
        if (prompt.startsWith("@") && !description.isEmpty() && description != skill_name)
            text.append(tr("<br/> <b>Source</b>: %1<br/>").arg(description));
    } else {
        text = tr("Please choose a player");
        if (!description.isEmpty() && description != skill_name)
            text.append(tr("<br/> <b>Source</b>: %1<br/>").arg(description));
    }
    prompt_doc->setHtml(text);

    setStatus(AskForPlayerChoose);
}

void Client::onPlayerReplyYiji(const Card *card, const Player *to) {
    Json::Value req;
    if (!card)
        req = Json::Value::null;
    else {
        req = Json::Value(Json::arrayValue);
        req[0] = toJsonArray(card->getSubcards());
        req[1] = toJsonString(to->objectName());
    }        
    replyToServer(S_COMMAND_SKILL_YIJI, req);

    setStatus(NotActive);
}

void Client::onPlayerReplyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards) {
    Json::Value decks(Json::arrayValue);
    decks[0] = toJsonArray(up_cards);
    decks[1] = toJsonArray(down_cards);

    replyToServer(S_COMMAND_SKILL_GUANXING, decks);

    setStatus(NotActive);
}

void Client::log(const Json::Value &log_str) {
    if (!log_str.isArray() || log_str.size() != 6)
        emit log_received(QStringList() << QString());
    else {
        QStringList log;
        tryParse(log_str, log);
        if (log.first() == "#BasaraReveal")
            Sanguosha->playSystemAudioEffect("choose-item");
        emit log_received(log);
    }
}

void Client::speak(const QString &speak_data) {
    QStringList words = speak_data.split(":");
    QString who = words.at(0);
    QString base64 = words.at(1);

    QByteArray data = QByteArray::fromBase64(base64.toAscii());
    QString text = QString::fromUtf8(data);
    emit text_spoken(text);

    if (who == ".") {
        QString line = tr("<font color='red'>System: %1</font>").arg(text);
        emit line_spoken(QString("<p style=\"margin:3px 2px;\">%1</p>").arg(line));
        return;
    }

    const ClientPlayer *from = getPlayer(who);

    QString title;
    if (from) {
        title = from->getGeneralName();
        title = Sanguosha->translate(title);
        title.append(QString("(%1)").arg(from->screenName()));
    }

    title = QString("<b>%1</b>").arg(title);

    QString line = tr("<font color='%1'>[%2] said: %3 </font>")
                   .arg(Config.TextEditColor.name()).arg(title).arg(text);

    emit line_spoken(QString("<p style=\"margin:3px 2px;\">%1</p>").arg(line));
}

void Client::moveFocus(const Json::Value &focus) {
    QStringList players;
    Countdown countdown;

    Q_ASSERT(focus.isArray() && focus.size() == 3);
    tryParse(focus[0], players);
    // focus[1] is the moveFocus reason, which is unused for now.
    countdown.tryParse(focus[2]);
    emit focus_moved(players, countdown);
}

void Client::setEmotion(const Json::Value &set_str) {
    if (!set_str.isArray() || set_str.size() != 2) return;
    if (!set_str[0].isString() || !set_str[1].isString()) return;

    QString target_name = toQString(set_str[0]);
    QString emotion = toQString(set_str[1]);

    emit emotion_set(target_name, emotion);
}

void Client::skillInvoked(const Json::Value &arg) {
    if (!isStringArray(arg, 0, 1)) return;
    emit skill_invoked(QString(arg[1].asCString()), QString(arg[0].asCString()));
}

void Client::animate(const Json::Value &animate_str) {
    if (!animate_str.isArray() || !animate_str[0].isInt()
        || !animate_str[1].isString() ||  !animate_str[2].isString())
        return;
    QStringList args;
    args << toQString(animate_str[1]) << toQString(animate_str[2]);
    int name = animate_str[0].asInt();
    emit animated(name, args);
}

void Client::setFixedDistance(const Json::Value &set_str) {
    if (!set_str.isArray() || set_str.size() != 3) return;
    if (!set_str[0].isString() || !set_str[1].isString() || !set_str[2].isInt()) return;

    ClientPlayer *from = getPlayer(toQString(set_str[0]));
    ClientPlayer *to = getPlayer(toQString(set_str[1]));
    int distance = set_str[2].asInt();

    if (from && to)
        from->setFixedDistance(to, distance);
}

void Client::fillGenerals(const Json::Value &generals) {
    if (!generals.isArray()) return;
    QStringList filled;
    tryParse(generals, filled);
    emit generals_filled(filled);
}

void Client::askForGeneral3v3(const Json::Value &) {
    emit general_asked();
    setStatus(AskForGeneralTaken);
}

void Client::takeGeneral(const Json::Value &take_str) {
    if (!take_str.isArray() || take_str.size() != 2 || !take_str[0].isString() || !take_str[1].isString()) return;
    QString who = toQString(take_str[0]);
    QString name = toQString(take_str[1]);

    emit general_taken(who, name);
}

void Client::startArrange(const Json::Value &to_arrange) {
    if (to_arrange.isNull()) {
        emit arrange_started(QString());
    } else {
        if (!to_arrange.isArray()) return;
        QStringList arrangelist;
        tryParse(to_arrange, arrangelist);
        emit arrange_started(arrangelist.join("+"));
    }
    setStatus(AskForArrangement);
}

void Client::onPlayerChooseRole3v3() {
    replyToServer(S_COMMAND_CHOOSE_ROLE_3V3, toJsonString(sender()->objectName()));
    setStatus(NotActive);
}

void Client::recoverGeneral(const Json::Value &recover_str) {
    if (!recover_str.isArray() || recover_str.size() != 2 || !recover_str[0].isInt() || !recover_str[1].isString()) return;
    int index = recover_str[0].asInt();
    QString name = toQString(recover_str[1]);

    emit general_recovered(index, name);
}

void Client::revealGeneral(const Json::Value &reveal_str) {
    if (!reveal_str.isArray() || reveal_str.size() != 2 || !reveal_str[0].isString() || !reveal_str[1].isString()) return;
    bool self = (toQString(reveal_str[0]) == Self->objectName());
    QString general = toQString(reveal_str[1]);

    emit general_revealed(self, general);
}

void Client::onPlayerChooseOrder() {
    OptionButton *button = qobject_cast<OptionButton *>(sender());
    QString order;
    if (button) {
        order = button->objectName();
    } else {
        if (qrand() % 2 == 0)
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

void Client::updateStateItem(const Json::Value &state_str) {
    if (!state_str.isString()) return;
    emit role_state_changed(toQString(state_str));
}

void Client::setAvailableCards(const Json::Value &pile) {
    if (!pile.isArray()) return;
    QList<int> drawPile;
    tryParse(pile, drawPile);
    available_cards = drawPile;
}
