#ifndef _QSAN_PROTOCOL_H
#define _QSAN_PROTOCOL_H

#include <string>
#include <list>
#include <json/json.h>

namespace QSanProtocol
{
    namespace Utils
    {
        bool isStringArray(const Json::Value &jsonObject, unsigned int startIndex, unsigned int endIndex);
        bool isIntArray(const Json::Value &jsonObject, unsigned int startIndex, unsigned int endIndex);
    }

    enum PacketType
    {
        S_UNKNOWN_PACKET,
        S_SERVER_REQUEST,
        S_SERVER_REPLY,
        S_SERVER_NOTIFICATION,
        S_CLIENT_REQUEST,
        S_CLIENT_REPLY,
        S_CLIENT_NOTIFICATION
    };

    enum ProcessInstanceType
    {
        S_SERVER_INSTANCE,
        S_CLIENT_INSTANCE
    };

    enum CheatCode
    {
        S_CHEAT_GET_ONE_CARD,
        S_CHEAT_CHANGE_GENERAL,
        S_CHEAT_KILL_PLAYER,
        S_CHEAT_REVIVE_PLAYER,
        S_CHEAT_MAKE_DAMAGE,
        S_CHEAT_RUN_SCRIPT
    };

    enum CheatCategory
    {
        S_CHEAT_FIRE_DAMAGE,
        S_CHEAT_THUNDER_DAMAGE,
        S_CHEAT_NORMAL_DAMAGE,
        S_CHEAT_HP_RECOVER,
        S_CHEAT_HP_LOSE,
        S_CHEAT_MAX_HP_LOSE,
        S_CHEAT_MAX_HP_RESET
    };

    enum CommandType
    {
        S_COMMAND_UNKNOWN,
        S_COMMAND_CHOOSE_CARD,
        S_COMMAND_PLAY_CARD,
        S_COMMAND_USE_CARD,        
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
        S_COMMAND_CHOOSE_ROLE,
        S_COMMAND_CHOOSE_ROLE_3V3,
        S_COMMAND_CHOOSE_DIRECTION,
        S_COMMAND_CHOOSE_PLAYER,
        S_COMMAND_CHOOSE_ORDER,
        S_COMMAND_ASK_PEACH,
        S_COMMAND_CLAIM_GENERAL,
        S_COMMAND_CLAIM_SKILL_ADD,
        S_COMMAND_CLAIM_SKILL_REMOVE,        
        S_COMMAND_NULLIFICATION,
        S_COMMAND_MULTIPLE_CHOICE,        
        S_COMMAND_PINDIAN,
        S_COMMAND_AMAZING_GRACE,
        S_COMMAND_SKILL_YIJI,
        S_COMMAND_SKILL_GUANXING,
        S_COMMAND_SKILL_GONGXIN,
        S_COMMAND_SET_PROPERTY,
        S_COMMAND_SET_HP,
        S_COMMAND_SET_MAXHP,
        S_COMMAND_CHEAT,
        S_COMMAND_SURRENDER,
        S_COMMAND_GAME_OVER, 
        S_COMMAND_MOVE_CARD,
        S_COMMAND_GET_CARD,
        S_COMMAND_LOSE_CARD,
        S_COMMAND_LOG_EVENT,
        S_COMMAND_LOG_SKILL
    };

    enum GameEventType
    {
        S_GAME_EVENT_PLAYER_DYING,
        S_GAME_EVENT_SKILL_INVOKED
    };

    enum Game3v3ChooseOrderCommand
    {
        S_REASON_CHOOSE_ORDER_TURN,
        S_REASON_CHOOSE_ORDER_SELECT
    };

    enum Game3v3Camp
    {
        S_CAMP_WARM = 0,
        S_CAMP_COOL = 1
    };

    //static consts
    extern const char* S_PLAYER_SELF_REFERENCE_ID;

    class Countdown
    {
    public:
        enum CountdownType
        {            
            S_COUNTDOWN_NO_LIMIT,
            S_COUNTDOWN_USE_SPECIFIED,
            S_COUNTDOWN_USE_DEFAULT           
        } m_type;
        static const std::string S_COUNTDOWN_MAGIC;
        time_t m_current;
        time_t m_max;
        inline Countdown(CountdownType type = S_COUNTDOWN_NO_LIMIT, time_t current = 0, time_t max = 0):
            m_type(type), m_current(current), m_max(max) {}
        bool tryParse(Json::Value val);        
        inline Json::Value toJsonValue()
        {
            if (m_type == S_COUNTDOWN_NO_LIMIT
                || m_type == S_COUNTDOWN_USE_DEFAULT)
            {
                Json::Value val(Json::arrayValue);
                val[0] = S_COUNTDOWN_MAGIC;
                val[1] = (int)m_type;                
                return val;                
            }
            else
            {
                Json::Value val(Json::arrayValue);
                val[0] = S_COUNTDOWN_MAGIC;
                val[1] = (int)m_current;
                val[2] = (int)m_max;
                return val;
            }
        }
        inline bool hasTimedOut()
        {
            if (m_type == S_COUNTDOWN_NO_LIMIT)
                return false;
            else
                return m_current >= m_max;
        }
    };

    class QSanPacket
    {
    public:
        virtual bool parse(const std::string&) = 0;
        virtual std::string toString() const = 0;
        virtual PacketType getPacketType() const = 0;
        virtual CommandType getCommandType() const = 0;        
    };
        
    class QSanGeneralPacket: public QSanPacket
    {
    public:
        //format: [global_serial,local_serial,packet_type,command_name,command_body]
        unsigned int m_globalSerial;
        unsigned int m_localSerial;        
        inline QSanGeneralPacket(PacketType packetType = S_UNKNOWN_PACKET, CommandType command = S_COMMAND_UNKNOWN)
        {
            _m_globalSerial++;
            m_globalSerial = _m_globalSerial;
            m_localSerial = 0;
            m_packetType = packetType;
            m_command = command;
            m_msgBody = Json::nullValue;
        }
        inline void setMessageBody(const Json::Value &value){m_msgBody = value;}
        inline Json::Value& getMessageBody(){return m_msgBody;}
        inline const Json::Value& getMessageBody() const {return m_msgBody;}
        virtual bool parse(const std::string&);
        virtual std::string toString() const;
        inline virtual PacketType getPacketType() const { return m_packetType; }
        inline virtual CommandType getCommandType() const { return m_command; }
    protected:
        static unsigned int _m_globalSerial;
        CommandType m_command;
        PacketType m_packetType;
        Json::Value m_msgBody;
        inline virtual bool parseBody(const Json::Value &value) { m_msgBody = value; return true; }
        virtual const Json::Value& constructBody() const { return m_msgBody; }

        //helper functions                
        static bool tryParse(const std::string &result, int &val);
        static const unsigned int S_MAX_PACKET_SIZE;

        Json::Reader m_jsonReader;
    };    
}

#endif
