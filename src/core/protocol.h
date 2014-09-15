/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <QByteArray>
#include <QVariant>

namespace QSanProtocol {

    enum PacketDescription {
        S_DESC_UNKNOWN,
        S_TYPE_REQUEST = 0x1,
        S_TYPE_REPLY = 0x2,
        S_TYPE_NOTIFICATION = 0x4,
        S_TYPE_MASK = 0xf,
        S_SRC_ROOM = 0x10,
        S_SRC_LOBBY = 0x20,
        S_SRC_CLIENT = 0x40,
        S_SRC_MASK = 0xf0,
        S_DEST_ROOM = 0x100,
        S_DEST_LOBBY = 0x200,
        S_DEST_CLIENT = 0x400,
        S_DEST_MASK = 0xf00,

        S_DESC_DUMMY
    };

    enum ProcessInstanceType {
        S_SERVER_INSTANCE,
        S_CLIENT_INSTANCE
    };

    enum CheatCode {
        S_CHEAT_GET_ONE_CARD,
        S_CHEAT_KILL_PLAYER,
        S_CHEAT_REVIVE_PLAYER,
        S_CHEAT_MAKE_DAMAGE,
        S_CHEAT_RUN_SCRIPT
    };

    enum CheatCategory {
        S_CHEAT_FIRE_DAMAGE,
        S_CHEAT_THUNDER_DAMAGE,
        S_CHEAT_NORMAL_DAMAGE,
        S_CHEAT_HP_RECOVER,
        S_CHEAT_HP_LOSE,
        S_CHEAT_MAX_HP_LOSE,
        S_CHEAT_MAX_HP_RESET
    };

    enum CommandType {
        S_COMMAND_UNKNOWN,
        S_COMMAND_CHOOSE_CARD,
        S_COMMAND_PLAY_CARD,
        S_COMMAND_RESPONSE_CARD,
        S_COMMAND_SHOW_CARD,
        S_COMMAND_SHOW_ALL_CARDS,
        S_COMMAND_EXCHANGE_CARD,
        S_COMMAND_DISCARD_CARD,
        S_COMMAND_INVOKE_SKILL,
        S_COMMAND_MOVE_FOCUS,
        S_COMMAND_CHOOSE_GENERAL,
        S_COMMAND_CHOOSE_KINGDOM,
        S_COMMAND_CHOOSE_SUIT,
        S_COMMAND_CHOOSE_DIRECTION,
        S_COMMAND_CHOOSE_PLAYER,
        S_COMMAND_CHOOSE_ORDER,
        S_COMMAND_ASK_PEACH,
        S_COMMAND_SET_MARK,
        S_COMMAND_SET_FLAG,
        S_COMMAND_CARD_FLAG,
        S_COMMAND_NULLIFICATION,
        S_COMMAND_MULTIPLE_CHOICE,
        S_COMMAND_PINDIAN,
        S_COMMAND_AMAZING_GRACE,
        S_COMMAND_SKILL_YIJI,
        S_COMMAND_SKILL_GUANXING,
        S_COMMAND_SKILL_GONGXIN,
        S_COMMAND_SET_PROPERTY,
        S_COMMAND_CHANGE_HP,
        S_COMMAND_CHANGE_MAXHP,
        S_COMMAND_CHEAT,
        S_COMMAND_SURRENDER,
        S_COMMAND_ENABLE_SURRENDER,
        S_COMMAND_GAME_OVER,
        S_COMMAND_GAME_START,
        S_COMMAND_MOVE_CARD,
        S_COMMAND_GET_CARD,
        S_COMMAND_LOSE_CARD,
        S_COMMAND_LOG_EVENT,
        S_COMMAND_LOG_SKILL,
        S_COMMAND_UPDATE_CARD,
        S_COMMAND_CARD_LIMITATION,
        S_COMMAND_ADD_HISTORY,
        S_COMMAND_SET_EMOTION,
        S_COMMAND_FILL_AMAZING_GRACE,
        S_COMMAND_CLEAR_AMAZING_GRACE,
        S_COMMAND_TAKE_AMAZING_GRACE,
        S_COMMAND_FIXED_DISTANCE,
        S_COMMAND_KILL_PLAYER,
        S_COMMAND_REVIVE_PLAYER,
        S_COMMAND_ATTACH_SKILL,
        S_COMMAND_NULLIFICATION_ASKED,
        S_COMMAND_EXCHANGE_KNOWN_CARDS, // For Dimeng only
        S_COMMAND_SET_KNOWN_CARDS,
        S_COMMAND_UPDATE_PILE,
        S_COMMAND_RESET_PILE,
        S_COMMAND_UPDATE_HANDCARD_NUM,
        S_COMMAND_UPDATE_STATE_ITEM,
        S_COMMAND_SPEAK,
        S_COMMAND_ARRANGE_GENERAL,
        S_COMMAND_FILL_GENERAL,
        S_COMMAND_TAKE_GENERAL,
        S_COMMAND_RECOVER_GENERAL,
        S_COMMAND_REVEAL_GENERAL,
        S_COMMAND_AVAILABLE_CARDS,
        S_COMMAND_ANIMATE,
        S_COMMAND_LUCK_CARD,
        S_COMMAND_VIEW_GENERALS,
        S_COMMAND_SET_DASHBOARD_SHADOW,
        S_COMMAND_PRESHOW,
        S_COMMAND_TOGGLE_READY,
        S_COMMAND_ADD_ROBOT,
        S_COMMAND_FILL_ROBOTS,
        S_COMMAND_TRUST,
        S_COMMAND_PAUSE,
        S_COMMAND_NETWORK_DELAY_TEST,
        S_COMMAND_CHECK_VERSION,
        S_COMMAND_SETUP,
        S_COMMAND_ADD_PLAYER,
        S_COMMAND_REMOVE_PLAYER,
        S_COMMAND_START_IN_X_SECONDS,
        S_COMMAND_ARRANGE_SEATS,
        S_COMMAND_WARN,
        S_COMMAND_SIGNUP,
        S_COMMAND_DISABLE_SHOW,
        S_COMMAND_TRIGGER_ORDER,
        S_COMMAND_MIRROR_GUANXING_STEP,
        S_COMMAND_CHANGE_SKIN
    };

    enum GameEventType {
        S_GAME_EVENT_PLAYER_DYING,
        S_GAME_EVENT_PLAYER_QUITDYING,
        S_GAME_EVENT_PLAY_EFFECT,
        S_GAME_EVENT_JUDGE_RESULT,
        S_GAME_EVENT_DETACH_SKILL,
        S_GAME_EVENT_ACQUIRE_SKILL,
        S_GAME_EVENT_ADD_SKILL,
        S_GAME_EVENT_LOSE_SKILL,
        S_GAME_EVENT_UPDATE_SKILL,
        S_GAME_EVENT_UPDATE_PRESHOW,
        S_GAME_EVENT_CHANGE_GENDER,
        S_GAME_EVENT_CHANGE_HERO,
        S_GAME_EVENT_PLAYER_REFORM,
        S_GAME_EVENT_SKILL_INVOKED,
        S_GAME_EVENT_PAUSE,
        S_GAME_EVENT_REVEAL_PINDIAN
    };

    enum AnimateType {
        S_ANIMATE_NULL,

        S_ANIMATE_INDICATE,
        S_ANIMATE_LIGHTBOX,
        S_ANIMATE_NULLIFICATION,
        S_ANIMATE_FIRE,
        S_ANIMATE_LIGHTNING
    };

    enum Game3v3ChooseOrderCommand {
        S_REASON_CHOOSE_ORDER_TURN,
        S_REASON_CHOOSE_ORDER_SELECT
    };

    enum Game3v3Camp {
        S_CAMP_WARM,
        S_CAMP_COOL
    };

    enum GuanxingStep {
        S_GUANXING_START,
        S_GUANXING_MOVE,
        S_GUANXING_FINISH
    };

    //static consts
    extern const char *S_PLAYER_SELF_REFERENCE_ID;

    extern const int S_ALL_ALIVE_PLAYERS;

    class Countdown {
    public:
        enum CountdownType {
            S_COUNTDOWN_NO_LIMIT,
            S_COUNTDOWN_USE_SPECIFIED,
            S_COUNTDOWN_USE_DEFAULT
        } type;

        time_t current;
        time_t max;
        inline Countdown(CountdownType type = S_COUNTDOWN_NO_LIMIT, time_t current = 0, time_t max = 0)
            : type(type), current(current), max(max) {}
        bool tryParse(const QVariant &var);
        QVariant toVariant() const;
        inline bool hasTimedOut() {
            if (type == S_COUNTDOWN_NO_LIMIT)
                return false;
            else
                return current >= max;
        }
    };

    class AbstractPacket {
    public:
        virtual bool parse(const QByteArray &) = 0;
        virtual QByteArray toJson() const = 0;
        virtual QString toString() const = 0;
        virtual PacketDescription getPacketDestination() const = 0;
        virtual PacketDescription getPacketSource() const = 0;
        virtual PacketDescription getPacketType() const = 0;
        virtual PacketDescription getPacketDescription() const = 0;
        virtual CommandType getCommandType() const = 0;
    };

    class Packet : public AbstractPacket {
    public:
        //format: [global_serial, local_serial, packet_type, command_name, command_body]
        unsigned int globalSerial;
        unsigned int localSerial;

        Packet(int packetDescription = S_DESC_UNKNOWN, CommandType command = S_COMMAND_UNKNOWN);
        unsigned int createGlobalSerial();
        inline void setMessageBody(const QVariant &value) { messageBody = value; }
        inline const QVariant &getMessageBody() const{ return messageBody; }
        virtual bool parse(const QByteArray &raw);
        virtual QByteArray toJson() const;
        virtual QString toString() const;
        virtual PacketDescription getPacketDestination() const{
            return static_cast<PacketDescription>(packetDescription & S_DEST_MASK);
        }
        virtual PacketDescription getPacketSource() const{
            return static_cast<PacketDescription>(packetDescription & S_SRC_MASK);
        }
        virtual PacketDescription getPacketType() const{
            return static_cast<PacketDescription>(packetDescription & S_TYPE_MASK);
        }
        virtual PacketDescription getPacketDescription() const{ return packetDescription; }
        virtual CommandType getCommandType() const{ return command; }

    protected:
        static unsigned int globalSerialSequence;
        CommandType command;
        PacketDescription packetDescription;
        QVariant messageBody;

        //helper functions
        static const int S_MAX_PACKET_SIZE;
    };
}

#endif

