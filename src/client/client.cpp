/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3.0 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Hegemony Team
    *********************************************************************/

#include "client.h"
#include "settings.h"
#include "engine.h"
#include "standard.h"
#include "choosegeneraldialog.h"
#include "nativesocket.h"
#include "recorder.h"
#include "jsonutils.h"
#include "SkinBank.h"
#include "roomscene.h"

#include <json/json.h>
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

    callbacks[S_COMMAND_CHECK_VERSION] = &Client::checkVersion;
    callbacks[S_COMMAND_SETUP] = &Client::setup;
    callbacks[S_COMMAND_NETWORK_DELAY_TEST] = &Client::networkDelayTest;
    callbacks[S_COMMAND_ADD_PLAYER] = &Client::addPlayer;
    callbacks[S_COMMAND_REMOVE_PLAYER] = &Client::removePlayer;
    callbacks[S_COMMAND_START_IN_X_SECONDS] = &Client::startInXs;
    callbacks[S_COMMAND_ARRANGE_SEATS] = &Client::arrangeSeats;
    callbacks[S_COMMAND_WARN] = &Client::warn;
    callbacks[S_COMMAND_SPEAK] = &Client::speak;

    callbacks[S_COMMAND_GAME_START] = &Client::startGame;
    callbacks[S_COMMAND_GAME_OVER] = &Client::gameOver;

    callbacks[S_COMMAND_CHANGE_HP] = &Client::hpChange;
    callbacks[S_COMMAND_CHANGE_MAXHP] = &Client::maxhpChange;
    callbacks[S_COMMAND_KILL_PLAYER] = &Client::killPlayer;
    callbacks[S_COMMAND_REVIVE_PLAYER] = &Client::revivePlayer;
    callbacks[S_COMMAND_SHOW_CARD] = &Client::showCard;
    callbacks[S_COMMAND_UPDATE_CARD] = &Client::updateCard;
    callbacks[S_COMMAND_SET_MARK] = &Client::setMark;
    callbacks[S_COMMAND_LOG_SKILL] = &Client::log;
    callbacks[S_COMMAND_ATTACH_SKILL] = &Client::attachSkill;
    callbacks[S_COMMAND_MOVE_FOCUS] = &Client::moveFocus;
    callbacks[S_COMMAND_SET_EMOTION] = &Client::setEmotion;
    callbacks[S_COMMAND_INVOKE_SKILL] = &Client::skillInvoked;
    callbacks[S_COMMAND_SHOW_ALL_CARDS] = &Client::showAllCards;
    callbacks[S_COMMAND_SKILL_GONGXIN] = &Client::askForGongxin;
    callbacks[S_COMMAND_LOG_EVENT] = &Client::handleGameEvent;
    callbacks[S_COMMAND_ADD_HISTORY] = &Client::addHistory;
    callbacks[S_COMMAND_ANIMATE] = &Client::animate;
    callbacks[S_COMMAND_FIXED_DISTANCE] = &Client::setFixedDistance;
    callbacks[S_COMMAND_CARD_LIMITATION] = &Client::cardLimitation;
    callbacks[S_COMMAND_DISABLE_SHOW] = &Client::disableShow;
    callbacks[S_COMMAND_NULLIFICATION_ASKED] = &Client::setNullification;
    callbacks[S_COMMAND_ENABLE_SURRENDER] = &Client::enableSurrender;
    callbacks[S_COMMAND_EXCHANGE_KNOWN_CARDS] = &Client::exchangeKnownCards;
    callbacks[S_COMMAND_SET_KNOWN_CARDS] = &Client::setKnownCards;
    callbacks[S_COMMAND_VIEW_GENERALS] = &Client::viewGenerals;
    callbacks[S_COMMAND_SET_DASHBOARD_SHADOW] = &Client::setDashboardShadow;

    callbacks[S_COMMAND_UPDATE_STATE_ITEM] = &Client::updateStateItem;
    callbacks[S_COMMAND_AVAILABLE_CARDS] = &Client::setAvailableCards;

    callbacks[S_COMMAND_GET_CARD] = &Client::getCards;
    callbacks[S_COMMAND_LOSE_CARD] = &Client::loseCards;
    callbacks[S_COMMAND_SET_PROPERTY] = &Client::updateProperty;
    m_callbacks[S_COMMAND_RESET_PILE] = &Client::resetPiles;
    m_callbacks[S_COMMAND_UPDATE_PILE] = &Client::setPileNumber;
    m_callbacks[S_COMMAND_CARD_FLAG] = &Client::setCardFlag;
    m_callbacks[S_COMMAND_UPDATE_HANDCARD_NUM] = &Client::setHandcardNum;

    // interactive methods
    m_interactions[S_COMMAND_CHOOSE_GENERAL] = &Client::askForGeneral;
    m_interactions[S_COMMAND_CHOOSE_PLAYER] = &Client::askForPlayerChosen;
    m_interactions[S_COMMAND_CHOOSE_DIRECTION] = &Client::askForDirection;
    m_interactions[S_COMMAND_EXCHANGE_CARD] = &Client::askForExchange;
    m_interactions[S_COMMAND_ASK_PEACH] = &Client::askForSinglePeach;
    m_interactions[S_COMMAND_SKILL_GUANXING] = &Client::askForGuanxing;
    interactions[S_COMMAND_SKILL_GONGXIN] = &Client::askForGongxin;
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
    m_interactions[S_COMMAND_SURRENDER] = &Client::askForSurrender;
    m_interactions[S_COMMAND_LUCK_CARD] = &Client::askForLuckCard;
    m_interactions[S_COMMAND_TRIGGER_ORDER] = &Client::askForTriggerOrder;

    m_callbacks[S_COMMAND_FILL_AMAZING_GRACE] = &Client::fillAG;
    m_callbacks[S_COMMAND_TAKE_AMAZING_GRACE] = &Client::takeAG;
    m_callbacks[S_COMMAND_CLEAR_AMAZING_GRACE] = &Client::clearAG;

    // 3v3 mode & 1v1 mode
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
    }
    else {
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

void Client::updateCard(const QVariant &val) {
    if (JsonUtils::isNumber(val.type())) {
        // reset card
        int cardId = val.toInt();
        Card *card = _m_roomState.getCard(cardId);
        if (!card->isModified()) return;
        _m_roomState.resetCard(cardId);
    }
    else {
        // update card
        JsonArray args = val.value<JsonArray>();
        Q_ASSERT(args.size() >= 5);
        int cardId = args[0].toInt();
        Card::Suit suit = (Card::Suit) args[1].toInt();
        int number = args[2].toInt();
        QString cardName = args[3].toString();
        QString skillName = args[4].toString();
        QString objectName = args[5].toString();
        QStringList flags;
        JsonUtils::tryParse(args[6].value<JsonArray>(), flags);

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
        JsonArray arg;
        arg << Config.value("EnableReconnection", false).toBool();
        arg << Config.UserName;
        arg << Config.UserAvatar;
        notifyServer(S_COMMAND_SIGNUP, arg);
    }
}

void Client::networkDelayTest(const QVariant &) {
    notifyServer(S_COMMAND_NETWORK_DELAY_TEST);
}

void Client::replyToServer(CommandType command, const QVariant &arg) {
    if (socket) {
        Packet packet(S_SRC_CLIENT | S_TYPE_REPLY | S_DEST_ROOM, command);
        packet.localSerial = _m_lastServerSerial;
        packet.setMessageBody(arg);
        socket->send(packet.toUtf8());
    }
}

void Client::handleGameEvent(const QVariant &arg) {
    emit event_received(arg);
}

void Client::requestServer(CommandType command, const QVariant &arg) {
    if (socket) {
        Packet packet(S_SRC_CLIENT | S_TYPE_REQUEST | S_DEST_ROOM, command);
        packet.setMessageBody(arg);
        socket->send(packet.toUtf8());
    }
}

void Client::notifyServer(CommandType command, const QVariant &arg) {
    if (socket) {
        Packet packet(S_SRC_CLIENT | S_TYPE_NOTIFICATION | S_DEST_ROOM, command);
        packet.setMessageBody(arg);
        socket->send(packet.toUtf8());
    }
}

void Client::request(const QByteArray &raw) {
    if (socket)
        socket->send(raw);
}

void Client::checkVersion(const QVariant &server_version) {
    QString version = server_version.toString();
    QString version_number, mod_name;
    if (version.contains(QChar(':'))) {
        QStringList texts = version.split(QChar(':'));
        version_number = texts.value(0);
        mod_name = texts.value(1);
    }
    else {
        version_number = version;
        mod_name = "official";
    }

    emit version_checked(version_number, mod_name);
}

void Client::setup(const QVariant &setup_json) {
    if (socket && !socket->isConnected())
        return;

    QString setup_str = setup_json.toString();
    if (ServerInfo.parse(setup_str)) {
        emit server_connected();
        notifyServer(S_COMMAND_TOGGLE_READY);
    }
    else {
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
    processServerPacket(cmd.toUtf8().data());
}

void Client::processServerPacket(const char *cmd) {
    if (m_isGameOver) return;
    Packet packet;
    if (packet.parse(cmd)) {
        if (packet.getPacketType() == S_TYPE_NOTIFICATION) {
            Callback callback = callbacks[packet.getCommandType()];
            if (callback) {
                (this->*callback)(packet.getMessageBody());
            } else {//@to-do: remove the branch after all callbacks are migrated
                CallBack deprecated_callback = m_callbacks[packet.getCommandType()];
                if (deprecated_callback) {
                    (this->*deprecated_callback)(VariantToJsonValue(packet.getMessageBody()));
                }
            }
        }
        else if (packet.getPacketType() == S_TYPE_REQUEST) {
            if (replayer && packet.getPacketDescription() == 0x411 && packet.getCommandType() == S_COMMAND_CHOOSE_GENERAL) {
                CallBack callback = m_interactions[S_COMMAND_CHOOSE_GENERAL];
                if (callback)
                    (this->*callback)(VariantToJsonValue(packet.getMessageBody()));
            }
            else if (!replayer)
                processServerRequest(packet);
        }
    }
    else
        processObsoleteServerPacket(cmd);
}

bool Client::processServerRequest(const Packet &packet) {
    setStatus(NotActive);

    _m_lastServerSerial = packet.globalSerial;
    CommandType command = packet.getCommandType();

    if (!replayer) {
        Countdown countdown;
        countdown.current = 0;
        countdown.type = Countdown::S_COUNTDOWN_USE_DEFAULT;
        countdown.max = ServerInfo.getCommandTimeout(command, S_CLIENT_INSTANCE);
        setCountdown(countdown);
    }

    Callback callback = interactions[command];
    if (callback) {
        (this->*callback)(packet.getMessageBody());
        return true;
    }

    CallBack deprecated_callback = m_interactions[command];
    if (deprecated_callback) {
        (this->*deprecated_callback)(VariantToJsonValue(packet.getMessageBody()));
        return true;
    }

    return false;
}

void Client::processObsoleteServerPacket(const QString &cmd) {
    // invoke methods
    QMessageBox::information(NULL, tr("Warning"), tr("No such invokable method named \"%1\"").arg(cmd));
}

void Client::addPlayer(const QVariant &player_info) {
    if (!player_info.canConvert<JsonArray>())
        return;

    JsonArray info = player_info.value<JsonArray>();
    if(info.size() < 3)
        return;

    QString name = info[0].toString();
    QString screen_name = info[1].toString();
    QString avatar = info[2].toString();

    ClientPlayer *player = new ClientPlayer(this);
    player->setObjectName(name);
    player->setScreenName(screen_name);
    player->setProperty("avatar", avatar);

    players << player;
    alive_count++;
    emit player_added(player);
}

void Client::updateProperty(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    if (!JsonUtils::isStringArray(args, 0, 2)) return;
    QString object_name = args[0].toString();
    ClientPlayer *player = getPlayer(object_name);
    if (!player) return;
    player->setProperty(args[1].toString().toLatin1().constData(), args[2].toString());

    //for shuangxiong { RoomScene::detachSkill(const QString &) }
    if (args[1] == "phase" && player->getPhase() == Player::Finish
        && player->hasFlag("shuangxiong_postpone") && player == Self && !Self->ownSkill("shuangxiong"))
        emit skill_detached("shuangxiong");

}

void Client::removePlayer(const QVariant &player_name) {
    QString name = player_name.toString();
    ClientPlayer *player = findChild<ClientPlayer *>(name);
    if (player) {
        player->setParent(NULL);
        alive_count--;
        emit player_removed(name);
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
        if (move.to_place == Player::DrawPile || move.to_place == Player::DrawPileBottom)
            pile_num++;
        // @todo: synchronize discard pile when "marshal"
        else if (move.to_place == Player::DiscardPile)
            discarded_list.prepend(card);
    }
    return true;
}

void Client::getCards(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    Q_ASSERT(args.size() >= 1);
    int moveId = args[0].toInt();
    QList<CardsMoveStruct> moves;
    for (int i = 1; i < args.size(); i++) {
        CardsMoveStruct move;
        if (!move.tryParse(args[i])) return;
        move.from = getPlayer(move.from_player_name);
        move.to = getPlayer(move.to_player_name);
        Player::Place dstPlace = move.to_place;

        if (dstPlace == Player::PlaceSpecial)
            ((ClientPlayer *)move.to)->changePile(move.to_pile_name, true, move.card_ids);
        else {
            foreach(int card_id, move.card_ids)
                _getSingleCard(card_id, move); // DDHEJ->DDHEJ, DDH/EJ->EJ
        }
        moves.append(move);
    }
    updatePileNum();
    emit move_cards_got(moveId, moves);
}

void Client::loseCards(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    Q_ASSERT(args.size() >= 1);
    int moveId = args[0].toInt();
    QList<CardsMoveStruct> moves;
    for (int i = 1; i < args.size(); i++) {
        CardsMoveStruct move;
        if (!move.tryParse(args[i])) return;
        move.from = getPlayer(move.from_player_name);
        move.to = getPlayer(move.to_player_name);
        Player::Place srcPlace = move.from_place;

        if (srcPlace == Player::PlaceSpecial)
            ((ClientPlayer *)move.from)->changePile(move.from_pile_name, false, move.card_ids);
        else {
            foreach(int card_id, move.card_ids)
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
        replyToServer(S_COMMAND_CHOOSE_GENERAL, item_name);
        Sanguosha->playSystemAudioEffect("choose-item");
    }
}

void Client::requestCheatRunScript(const QString &script) {
    JsonArray cheatReq;
    cheatReq << (int)S_CHEAT_RUN_SCRIPT;
    cheatReq << script;
    requestServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatRevive(const QString &name) {
    JsonArray cheatReq;
    cheatReq << (int)S_CHEAT_REVIVE_PLAYER;
    cheatReq << name;
    requestServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatDamage(const QString &source, const QString &target, DamageStruct::Nature nature, int points) {
    JsonArray cheatReq, cheatArg;
    cheatArg << source;
    cheatArg << target;
    cheatArg << (int)nature;
    cheatArg << points;

    cheatReq << (int)S_CHEAT_MAKE_DAMAGE;
    cheatReq << QVariant(cheatArg);
    requestServer(S_COMMAND_CHEAT, cheatReq);
}

void Client::requestCheatKill(const QString &killer, const QString &victim) {
    JsonArray cheatArg;
    cheatArg << (int)S_CHEAT_KILL_PLAYER;
    cheatArg << QVariant(JsonArray() << killer << victim);
    requestServer(S_COMMAND_CHEAT, cheatArg);
}

void Client::requestCheatGetOneCard(int card_id) {
    JsonArray cheatArg;
    cheatArg << (int)S_CHEAT_GET_ONE_CARD;
    cheatArg << card_id;
    requestServer(S_COMMAND_CHEAT, cheatArg);
}

void Client::addRobot() {
    notifyServer(S_COMMAND_ADD_ROBOT);
}

void Client::fillRobots() {
    notifyServer(S_COMMAND_FILL_ROBOTS);
}

void Client::arrange(const QStringList &order) {
    Q_ASSERT(order.length() == 3);

    request(QString("arrange %1").arg(order.join("+")));
}

void Client::onPlayerResponseCard(const Card *card, const QList<const Player *> &targets) {
    if ((status & ClientStatusBasicMask) == Responding)
        _m_roomState.setCurrentCardUsePattern(QString());
    if (card == NULL) {
        replyToServer(S_COMMAND_RESPONSE_CARD);
    } else {
        JsonArray targetNames;
        if (!card->targetFixed()) {
            foreach(const Player *target, targets)
                targetNames << target->objectName();
        }

        replyToServer(S_COMMAND_RESPONSE_CARD, JsonArray() << card->toString() << QVariant::fromValue(targetNames));

        if (card->isVirtualCard() && !card->parent())
            delete card;
    }

    if (Self->hasFlag("Client_PreventPeach")) {
        Self->setFlags("-Client_PreventPeach");
        Self->removeCardLimitation("use", "Peach$0");
    }
    setStatus(NotActive);
}

void Client::startInXs(const QVariant &left_seconds) {
    int seconds = left_seconds.toInt();
    if (seconds > 0)
        lines_doc->setHtml(tr("<p align = \"center\">Game will start in <b>%1</b> seconds...</p>").arg(seconds));
    else
        lines_doc->setHtml(QString());

    emit start_in_xs();
    if (seconds == 0 && Sanguosha->getScenario(ServerInfo.GameMode) == NULL) {
        emit avatars_hiden();
    }
}

void Client::arrangeSeats(const QVariant &seats_arr) {
    QStringList player_names;
    if (seats_arr.canConvert<JsonArray>()) {
        JsonArray seats = seats_arr.value<JsonArray>();
        foreach (const QVariant &seat, seats){
            player_names << seat.toString();
        }
    }
    players.clear();

    for (int i = 0; i < player_names.length(); i++) {
        ClientPlayer *player = findChild<ClientPlayer *>(player_names.at(i));

        Q_ASSERT(player != NULL);

        player->setSeat(i + 1);
        if (i > 0) {
            ClientPlayer *prev_player = findChild<ClientPlayer *>(player_names.at(i - 1));
            prev_player->setNext(player->objectName());

            if (i == player_names.length() - 1) {
                ClientPlayer *first_player = findChild<ClientPlayer *>(player_names.first());
                player->setNext(first_player->objectName());
            }
        }

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

void Client::startGame(const QVariant &) {
    Sanguosha->registerRoom(this);
    _m_roomState.reset();

    QList<ClientPlayer *> players = findChildren<ClientPlayer *>();
    alive_count = players.count();

    emit game_started();
}

void Client::hpChange(const QVariant &change_str) {
    JsonArray change = change_str.value<JsonArray>();
    if (change.size() != 3) return;
    if (change[0].type() != QMetaType::QString || !JsonUtils::isNumber(change[1]) || !JsonUtils::isNumber(change[2])) return;

    QString who = change[0].toString();
    int delta = change[1].toInt();

    int nature_index = change[2].toInt();
    DamageStruct::Nature nature = DamageStruct::Normal;
    if (nature_index > 0) nature = (DamageStruct::Nature)nature_index;

    emit hp_changed(who, delta, nature, nature_index == -1);
}

void Client::maxhpChange(const QVariant &change_str) {
    JsonArray change = change_str.value<JsonArray>();
    if (change.size() != 2) return;
    if (change[0].type() != QMetaType::QString || !JsonUtils::isNumber(change[1])) return;

    QString who = change[0].toString();
    int delta = change[1].toInt();
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

void Client::cardLimitation(const QVariant &limit) {
    JsonArray args = limit.value<JsonArray>();
    if (args.size() != 4) return;

    bool set = args[0].toBool();
    bool single_turn = args[3].toBool();
    if (args[1].isNull() && args[2].isNull()) {
        Self->clearCardLimitation(single_turn);
    } else {
        if (args[1].type() != QMetaType::QString || args[2].type() != QMetaType::QString) return;
        QString limit_list = args[1].toString();
        QString pattern = args[2].toString();
        if (set)
            Self->setCardLimitation(limit_list, pattern, single_turn);
        else
            Self->removeCardLimitation(limit_list, pattern);
    }
}

void Client::disableShow(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 4) return;

    ClientPlayer *p = getPlayer(args[0].toString());
    if (p == NULL) return;

    bool set = args[1].toBool();
    QString reason = args[3].toString();
    if (set){
        QString flags = args[2].toString();
        p->setDisableShow(flags, reason);
    }
    else
        p->removeDisableShow(reason);
}

void Client::setNullification(const QVariant &str) {
    if (str.type() != QMetaType::QString) return;
    QString astr = str.toString();
    if (astr != ".") {
        if (m_noNullificationTrickName == ".") {
            m_noNullificationThisTime = false;
            m_noNullificationTrickName = astr;
            emit nullification_asked(true);
        }
    }
    else {
        m_noNullificationThisTime = false;
        m_noNullificationTrickName = ".";
        emit nullification_asked(false);
    }
}

void Client::enableSurrender(const QVariant &enabled) {
    if (enabled.type() != QMetaType::Bool) return;
    bool en = enabled.toBool();
    emit surrender_enabled(en);
}

void Client::exchangeKnownCards(const QVariant &players) {
    JsonArray args = players.value<JsonArray>();
    if (args.size() != 2 || args[0].type() != QMetaType::QString || args[1].type() != QMetaType::QString) return;
    ClientPlayer *a = getPlayer(args[0].toString()), *b = getPlayer(args[1].toString());
    QList<int> a_known, b_known;
    foreach(const Card *card, a->getHandcards())
        a_known << card->getId();
    foreach(const Card *card, b->getHandcards())
        b_known << card->getId();
    a->setCards(b_known);
    b->setCards(a_known);
}

void Client::setKnownCards(const QVariant &set_str) {
    JsonArray set = set_str.value<JsonArray>();
    if (set.size() != 2) return;
    QString name = set[0].toString();
    ClientPlayer *player = getPlayer(name);
    if (player == NULL) return;
    QList<int> ids;
    JsonUtils::tryParse(set[1].value<JsonArray>(), ids);
    player->setCards(ids);

}

void Client::viewGenerals(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 2 || args[0].type() != QMetaType::QString) return;
    QString reason = args[0].toString();
    QStringList names;
    if (!JsonUtils::tryParse(args[1].value<JsonArray>(), names)) return;
    emit generals_viewed(reason, names);
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
        if (general_name.contains(Sanguosha->translate("sujiang"))) {
            QStringList names = general_name.split("/");
            if (names.length() == 2) {
                if (names[0].contains(Sanguosha->translate("sujiang")))
                    names.removeAt(0);
                else
                    names.removeAt(1);
                general_name = names.first();
            }
        }
        if (player->getGeneralName() == "anjiang" && player->getGeneral2() != NULL && player->getGeneral2Name() == "anjiang")
            general_name = Sanguosha->translate(QString("SEAT(%1)").arg(QString::number(player->property("UI_Seat").toInt())));
        return general_name;
    }
    else
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

    if (skill_name.endsWith("!"))
        skill_name.chop(1);

    QString text;
    if (skill_name.startsWith("userdefine:")) {
        QString name = skill_name.mid(4);
        prompt_doc->setHtml(Sanguosha->translate("@" + name));
    }
    else if (data.isEmpty()) {
        text = tr("Do you want to invoke skill [%1] ?").arg(Sanguosha->translate(skill_name));
        prompt_doc->setHtml(text);
    }
    else if (data.startsWith("playerdata:")) {
        QString name = getPlayerName(data.split(":").last());
        text = tr("Do you want to invoke skill [%1] to %2 ?").arg(Sanguosha->translate(skill_name)).arg(name);
        prompt_doc->setHtml(text);
    }
    else if (skill_name.startsWith("cv_")) {
        setPromptList(QStringList() << "@sp_convert" << QString() << QString() << data);
    }
    else {
        QStringList texts = data.split(":");
        text = QString("%1:%2").arg(skill_name).arg(texts.first());
        texts.replace(0, text);
        setPromptList(texts);
    }

    setStatus(AskForSkillInvoke);
}

void Client::onPlayerMakeChoice(const QString &choice) {
    replyToServer(S_COMMAND_MULTIPLE_CHOICE, choice);
    setStatus(NotActive);
}

void Client::askForSurrender(const Json::Value &initiator) {
    if (!initiator.isString()) return;

    QString text = tr("%1 initiated a vote for disadvataged side to claim "
        "capitulation. Click \"OK\" to surrender or \"Cancel\" to resist.")
        .arg(Sanguosha->translate(toQString(initiator)));
    text.append(tr("<br/> <b>Notice</b>: if more than half people decides to surrender. "
        "This game will over."));
    skill_name = "surrender";

    prompt_doc->setHtml(text);
    setStatus(AskForSkillInvoke);
}

void Client::askForLuckCard(const Json::Value &) {
    skill_to_invoke = "luck_card";
    prompt_doc->setHtml(tr("Do you want to use the luck card?"));
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
    }
    else {
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
    QVariant reply;
    if (card_id != -2)
        reply = card_id;
    replyToServer(S_COMMAND_CHOOSE_CARD, reply);
    setStatus(NotActive);
}

void Client::onPlayerChoosePlayer(const Player *player) {
    if (replayer) return;
    if (player == NULL && !m_isDiscardActionRefusable)
        player = findChild<const Player *>(players_to_choose.first());

    replyToServer(S_COMMAND_CHOOSE_PLAYER, (player == NULL) ? QVariant() : player->objectName());
    setStatus(NotActive);
}

void Client::onPlayerChooseTriggerOrder(const QString &choice)
{
    replyToServer(S_COMMAND_TRIGGER_ORDER, choice);
    setStatus(NotActive);
}

void Client::trust() {
    notifyServer(S_COMMAND_TRUST);

    if (Self->getState() == "trust")
        Sanguosha->playSystemAudioEffect("untrust");
    else
        Sanguosha->playSystemAudioEffect("trust");

    setStatus(NotActive);
}

void Client::preshow(const QString &skill_name, const bool isPreshowed) {
    JsonArray arg;
    arg << skill_name;
    arg << isPreshowed;
    requestServer(S_COMMAND_PRESHOW, arg);
    Self->setSkillPreshowed(skill_name, isPreshowed);

    if (Self->inHeadSkills(skill_name))
        emit head_preshowed();
    else
        emit deputy_preshowed();
}

void Client::requestSurrender() {
    requestServer(S_COMMAND_SURRENDER);
    setStatus(NotActive);
}

void Client::speakToServer(const QString &text) {
    if (text.isEmpty())
        return;

    notifyServer(S_COMMAND_SPEAK, text);
}

void Client::addHistory(const QVariant &history) {
    JsonArray args = history.value<JsonArray>();
    if (args.size() != 2 || args[0].type() != QMetaType::QString || !JsonUtils::isNumber(args[1])) return;

    QString add_str = args[0].toString();
    int times = args[1].toInt();
    if (add_str == "pushPile") {
        emit card_used();
        return;
    }
    else if (add_str == ".") {
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

void Client::setHandcardNum(const Json::Value &num_array){
    if (!num_array.isArray()) return;
    for (unsigned i = 0u; i < num_array.size(); i++){
        Json::Value _current = num_array[i];
        if (!_current.isArray())
            continue;
        if (_current.size() != 2 || !_current[0].isString() || !_current[1].isInt())
            continue;

        QString name = toQString(_current[0]);
        int num = _current[1].asInt();

        ClientPlayer *p = getPlayer(name);
        if (p != NULL)
            p->setHandcardNum(num);
    }

    emit update_handcard_num();
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
    if (!req.isArray() || !req[0].isInt() || !req[1].isInt() || !req[2].isBool()
            || !req[3].isBool() || !req[4].isString() || !req[5].isString())
        return;

    discard_num = req[0].asInt();
    min_num = req[1].asInt();
    m_isDiscardActionRefusable = req[2].asBool();
    m_canDiscardEquip = req[3].asBool();
    QString prompt = toQString(req[4]);
    discard_reason = toQString(req[5]);

    if (prompt.isEmpty()) {
        if (m_canDiscardEquip)
            prompt = tr("Please discard %1 card(s), include equip").arg(discard_num);
        else
            prompt = tr("Please discard %1 card(s), only hand cards is allowed").arg(discard_num);
        if (min_num < discard_num) {
            prompt.append("<br/>");
            prompt.append(tr("%1 %2 card(s) are required at least").arg(min_num).arg(m_canDiscardEquip ? "" : tr("hand")));
        }
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
    }
    else {
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

void Client::gameOver(const QVariant &arg) {
    disconnectFromHost();
    m_isGameOver = true;
    setStatus(Client::NotActive);

    JsonArray args = arg.value<JsonArray>();
    if (args.size() < 2)
        return;

    QString winner = args[0].toString();
    QStringList roles;
    foreach (const QVariant &role, args[1].value<JsonArray>()) {
        roles << role.toString();
    }

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
    foreach(const ClientPlayer *player, players) {
        QString role = player->getRole();
        bool win = winners.contains(player->objectName()) || winners.contains(role);

        ClientPlayer *p = const_cast<ClientPlayer *>(player);
        p->setProperty("win", win);
    }

    Sanguosha->unregisterRoom();
    emit game_over();
}

void Client::killPlayer(const QVariant &player_name) {
    if (player_name.type() != QMetaType::QString) return;
    QString name = player_name.toString();

    alive_count--;
    ClientPlayer *player = getPlayer(name);
    if (player == Self) {
        foreach(const Skill *skill, Self->getVisibleSkills())
            emit skill_detached(skill->objectName());
    }
    player->detachAllSkills();

    if (!Self->hasFlag("marshalling")) {
        QString general_name = player->getGeneralName();
        QString last_word = Sanguosha->translate(QString("~%1").arg(general_name));
        if (last_word.startsWith("~")) {
            QStringList origin_generals = general_name.split("_");
            if (origin_generals.length() > 1)
                last_word = Sanguosha->translate(("~") + origin_generals.at(1));
        }

        if (last_word.startsWith("~") && general_name.endsWith("f")) {
            QString origin_general = general_name;
            origin_general.chop(1);
            if (Sanguosha->getGeneral(origin_general))
                last_word = Sanguosha->translate(("~") + origin_general);
        }
        updatePileNum();
    }

    emit player_killed(name);
}

void Client::setDashboardShadow(const QVariant &player) {
    if (player.type() != QMetaType::QString) return;
    emit dashboard_death(player.toString());
}

void Client::revivePlayer(const QVariant &player) {
    if (player.type() != QMetaType::QString) return;

    QString player_name = player.toString();
    alive_count++;
    emit player_revived(player_name);
}


void Client::warn(const QVariant &reason_var) {
    QString reason = reason_var.toString();
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
    if (!tryParse(arg[0], generals)) return;
    bool single_result;
    if (!tryParse(arg[1], single_result)) return;
    emit generals_got(generals, single_result);
    setStatus(AskForGeneralChosen);
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
    setStatus(AskForChoice);
}

void Client::askForCardChosen(const Json::Value &ask_str) {
    if (!ask_str.isArray() || ask_str.size() != 6 || !isStringArray(ask_str, 0, 2)
        || !ask_str[3].isBool() || !ask_str[4].isInt()) return;
    QString player_name = toQString(ask_str[0]);
    QString flags = toQString(ask_str[1]);
    QString reason = toQString(ask_str[2]);
    bool handcard_visible = ask_str[3].asBool();
    Card::HandlingMethod method = (Card::HandlingMethod)ask_str[4].asInt();
    ClientPlayer *player = getPlayer(player_name);
    if (player == NULL) return;
    QList<int> disabled_ids;
    tryParse(ask_str[5], disabled_ids);
    emit cards_got(player, flags, reason, handcard_visible, method, disabled_ids);
    setStatus(AskForCardChosen);
}

void Client::askForOrder(const Json::Value &arg) {
    if (!arg.isInt()) return;
    Game3v3ChooseOrderCommand reason = (Game3v3ChooseOrderCommand)arg.asInt();
    emit orders_got(reason);
    setStatus(ExecDialog);
}

void Client::askForDirection(const Json::Value &) {
    emit directions_got();
    setStatus(ExecDialog);
}

void Client::askForTriggerOrder(const Json::Value &ask_str)
{
    if (!ask_str.isArray() || ask_str.size() != 3
            || !ask_str[0].isString() || !ask_str[1].isArray()
            || !ask_str[2].isBool()) return;

    QString reason = toQString(ask_str[0]);

    QStringList choices;
    tryParse(ask_str[1], choices);

    bool optional = ask_str[2].asBool();

    emit triggers_got(reason, choices, optional);
    setStatus(AskForTriggerOrder);
}

void Client::setMark(const QVariant &mark_var) {
    JsonArray mark_str = mark_var.value<JsonArray>();
    if (mark_str.size() != 3) return;
    if (mark_str[0].type() != QMetaType::QString || mark_str[1].type() != QMetaType::QString || !JsonUtils::isNumber(mark_str[2])) return;

    QString who = mark_str[0].toString();
    QString mark = mark_str[1].toString();
    int value = mark_str[2].toInt();

    ClientPlayer *player = getPlayer(who);
    if (player)
        player->setMark(mark, value);
}

void Client::onPlayerChooseSuit() {
    replyToServer(S_COMMAND_CHOOSE_SUIT, sender()->objectName());
    setStatus(NotActive);
}

void Client::onPlayerChooseKingdom() {
    replyToServer(S_COMMAND_CHOOSE_KINGDOM, sender()->objectName());
    setStatus(NotActive);
}

void Client::onPlayerDiscardCards(const Card *cards) {
    if (cards) {
        JsonArray arr;
        foreach(int card_id, cards->getSubcards())
            arr << card_id;
        if (cards->isVirtualCard() && !cards->parent())
            delete cards;
        replyToServer(S_COMMAND_DISCARD_CARD, arr);
    } else {
        replyToServer(S_COMMAND_DISCARD_CARD);
    }

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
    }
    else {
        QString dying_general = getPlayerName(dying->objectName());
        prompt_doc->setHtml(tr("%1 is dying, please provide %2 peach(es) to save him").arg(dying_general).arg(peaches));
        pattern << "peach";
        _m_roomState.setCurrentCardUsePattern("peach");
    }
    if (Self->hasFlag("Global_PreventPeach")) {
        bool has_skill = false;
        foreach(const Skill *skill, Self->getVisibleSkillList(true)) {
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
        }
        else {
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

void Client::showCard(const QVariant &show_str) {
    JsonArray show = show_str.value<JsonArray>();
    if (show.size() != 2 || show[0].type() != QMetaType::QString || !JsonUtils::isNumber(show[1]))
        return;

    QString player_name = show[0].toString();
    int card_id = show[1].toInt();

    ClientPlayer *player = getPlayer(player_name);
    if (player != Self)
        player->addKnownHandCard(Sanguosha->getCard(card_id));

    emit card_shown(player_name, card_id);
}

void Client::attachSkill(const QVariant &skill) {
    if (skill.type() != QMetaType::QString) return;

    QString skill_name = skill.toString();
    Self->acquireSkill(skill_name);
    emit skill_attached(skill_name, true);
}

void Client::askForGuanxing(const Json::Value &arg) {
    Json::Value deck = arg[0];
    bool single_side = arg[1].asBool();
    QList<int> card_ids;
    tryParse(deck, card_ids);

    emit guanxing(card_ids, single_side);
    setStatus(AskForGuanxing);
}

void Client::showAllCards(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    if (args.size() != 3 || args[0].type() != QMetaType::QString || args[1].type() != QMetaType::Bool)
        return;

    ClientPlayer *who = getPlayer(args[0].toString());
    QList<int> card_ids;
    if (!JsonUtils::tryParse(args[2].value<JsonArray>(), card_ids)) return;

    if (who)
        who->setCards(card_ids);

    emit gongxin(card_ids, false, QList<int>());
}

void Client::askForGongxin(const QVariant &args) {
    JsonArray arg = args.value<JsonArray>();
    if (arg.size() != 4 || arg[0].type() != QMetaType::QString || arg[1].type() != QMetaType::Bool)
        return;

    ClientPlayer *who = getPlayer(arg[0].toString());
    bool enable_heart = arg[1].toBool();
    QList<int> card_ids;
    if (!JsonUtils::tryParse(arg[2].value<JsonArray>(), card_ids)) return;
    QList<int> enabled_ids;
    if (!JsonUtils::tryParse(arg[3].value<JsonArray>(), enabled_ids)) return;

    who->setCards(card_ids);

    emit gongxin(card_ids, enable_heart, enabled_ids);
    setStatus(AskForGongxin);
}

void Client::onPlayerReplyGongxin(int card_id) {
    QVariant reply;
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
    if (!ask_str.isArray() || (ask_str.size() != 4 && ask_str.size() != 5)) return;
    //if (!ask_str[0].isArray() || !ask_str[1].isBool() || !ask_str[2].isInt()) return;
    Json::Value card_list = ask_str[0];
    int count = ask_str[2].asInt();
    m_isDiscardActionRefusable = ask_str[1].asBool();

    if (ask_str.size() == 5) {
        QString prompt = toQString(ask_str[4]);
        QStringList texts = prompt.split(":");
        if (texts.length() < 4) {
            while (texts.length() < 3)
                texts.append(QString());
            texts.append(QString::number(count));
        }
        setPromptList(texts);
    }
    else {
        prompt_doc->setHtml(tr("Please distribute %1 cards %2 as you wish")
            .arg(count)
            .arg(m_isDiscardActionRefusable ? QString() : tr("to another player")));
    }

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
    }
    else {
        text = tr("Please choose a player");
        if (!description.isEmpty() && description != skill_name)
            text.append(tr("<br/> <b>Source</b>: %1<br/>").arg(description));
    }
    prompt_doc->setHtml(text);

    setStatus(AskForPlayerChoose);
}

void Client::onPlayerReplyYiji(const Card *card, const Player *to) {
    if (!card)
        replyToServer(S_COMMAND_SKILL_YIJI);
    else {
        JsonArray req;
        req << JsonUtils::toJsonArray(card->getSubcards());
        req << to->objectName();
        replyToServer(S_COMMAND_SKILL_YIJI, req);
    }

    setStatus(NotActive);
}

void Client::onPlayerReplyGuanxing(const QList<int> &up_cards, const QList<int> &down_cards) {
    JsonArray decks;
    decks << JsonUtils::toJsonArray(up_cards);
    decks << JsonUtils::toJsonArray(down_cards);

    replyToServer(S_COMMAND_SKILL_GUANXING, decks);

    setStatus(NotActive);
}

void Client::log(const QVariant &log_str) {
    QStringList log;

    if (!JsonUtils::tryParse(log_str.value<JsonArray>(), log) || log.size() != 6)
        emit log_received(QStringList() << QString());
    else {
        if (log.first() == "#BasaraReveal")
            Sanguosha->playSystemAudioEffect("choose-item");
        else if (log.first() == "#UseLuckCard") {
            ClientPlayer *from = getPlayer(log.at(1));
            if (from && from != Self)
                from->setHandcardNum(0);
        }
        emit log_received(log);
    }
}

void Client::speak(const QVariant &speak) {
    if (!speak.canConvert<JsonArray>()) {
        qDebug() << speak;
        return;
    }

    JsonArray args = speak.value<JsonArray>();
    QString who = args[0].toString();
    QString text = args[1].toString();

    static const QString prefix("<img width=14 height=14 src='image/system/chatface/");
    static const QString suffix(".png'></img>");
    text = text.replace("<#", prefix).replace("#>", suffix);

    if (who == ".") {
        QString line = tr("<font color='red'>System: %1</font>").arg(text);
        emit lineSpoken(QString("<p style=\"margin:3px 2px;\">%1</p>").arg(line));
        return;
    }

    emit playerSpoke(who, QString("<p style=\"margin:3px 2px;\">%1</p>").arg(text));

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

    emit lineSpoken(QString("<p style=\"margin:3px 2px;\">%1</p>").arg(line));
}

void Client::moveFocus(const QVariant &focus) {
    Countdown countdown;

    JsonArray args = focus.value<JsonArray>();
    Q_ASSERT(!args.isEmpty());

    QStringList players;
    JsonArray json_players = args[0].value<JsonArray>();
    if (!json_players.isEmpty()) {
        JsonUtils::tryParse(json_players, players);
    } else {
        foreach(const ClientPlayer *player, this->players) {
            if (player->isAlive()) {
                players << player->objectName();
            }
        }
    }

    if (args.size() == 1) {//default countdown
        countdown.type = Countdown::S_COUNTDOWN_USE_SPECIFIED;
        countdown.current = 0;
        countdown.max = ServerInfo.getCommandTimeout(S_COMMAND_UNKNOWN, S_CLIENT_INSTANCE);

    } else {// focus[1] is the moveFocus reason, which is now removed.
        Json::ArrayIndex countdown_index = args.size() >= 3 ? 2 : 1;
        if (!countdown.tryParse(args[countdown_index])) {
            return;
        }
    }

    emit focus_moved(players, countdown);
}

void Client::moveFocus(const QString &focus, CommandType command) {
    QStringList focuses;
    focuses.append(focus);

    Countdown countdown;
    countdown.max = ServerInfo.getCommandTimeout(command, S_CLIENT_INSTANCE);
    countdown.type = Countdown::S_COUNTDOWN_USE_SPECIFIED;

    emit focus_moved(focuses, countdown);
}

void Client::setEmotion(const QVariant &set_str) {
    JsonArray set = set_str.value<JsonArray>();
    if (set.size() != 2) return;
    if (!JsonUtils::isStringArray(set, 0, 1)) return;

    QString target_name = set[0].toString();
    QString emotion = set[1].toString();

    emit emotion_set(target_name, emotion);
}

void Client::skillInvoked(const QVariant &arg) {
    JsonArray args = arg.value<JsonArray>();
    if (!JsonUtils::isStringArray(args, 0, 1)) return;
    emit skill_invoked(args[1].toString(), args[0].toString());
}

void Client::animate(const QVariant &animate_str) {
    JsonArray animate = animate_str.value<JsonArray>();
    if (animate.size() != 3 || !JsonUtils::isNumber(animate[0])
        || animate[1].type() != QMetaType::QString || animate[2].type() != QMetaType::QString)
        return;

    QStringList args;
    args << animate[1].toString() << animate[2].toString();
    int name = animate[0].toInt();
    emit animated(name, args);
}

void Client::setFixedDistance(const QVariant &set_str) {
    JsonArray set = set_str.value<JsonArray>();
    if (set.size() != 3
            || set[0].type() != QMetaType::QString
            || set[1].type() != QMetaType::QString
            || !JsonUtils::isNumber(set[2])) return;

    ClientPlayer *from = getPlayer(set[0].toString());
    ClientPlayer *to = getPlayer(set[1].toString());
    int distance = set[2].toInt();

    if (from && to)
        from->setFixedDistance(to, distance);
}

void Client::fillGenerals(const Json::Value &generals) {
    if (!generals.isArray()) return;
    QStringList filled;
    tryParse(generals, filled);
    emit generals_filled(filled);
}

void Client::takeGeneral(const Json::Value &take_str) {
    if (!isStringArray(take_str, 0, 2)) return;
    QString who = toQString(take_str[0]);
    QString name = toQString(take_str[1]);
    QString rule = toQString(take_str[2]);

    emit general_taken(who, name, rule);
}

void Client::startArrange(const Json::Value &to_arrange) {
    if (to_arrange.isNull()) {
        emit arrange_started(QString());
    }
    else {
        if (!to_arrange.isArray()) return;
        QStringList arrangelist;
        tryParse(to_arrange, arrangelist);
        emit arrange_started(arrangelist.join("+"));
    }
    setStatus(AskForArrangement);
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
    }
    else {
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

void Client::updateStateItem(const QVariant &state) {
    if (state.type() != QMetaType::QString) return;
    emit role_state_changed(state.toString());
}

void Client::setAvailableCards(const QVariant &pile) {
    if (!pile.canConvert<JsonArray>()) return;
    QList<int> drawPile;
    JsonUtils::tryParse(pile.value<JsonArray>(), drawPile);
    available_cards = drawPile;
}
