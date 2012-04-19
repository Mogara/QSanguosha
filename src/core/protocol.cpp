#include "protocol.h"
#include <json/json.h>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace QSanProtocol;

unsigned int QSanProtocol::QSanGeneralPacket::_m_globalSerial = 0;
const unsigned int QSanProtocol::QSanGeneralPacket::S_MAX_PACKET_SIZE = 1000;

bool QSanProtocol::Utils::isStringArray(const Json::Value &jsonObject, unsigned int startIndex, unsigned int endIndex)
{
    if (!jsonObject.isArray() || jsonObject.size() <= endIndex)
    {
        return false;
    }
    for (unsigned int i = startIndex; i <= endIndex; i++)
    {
        if (!jsonObject[i].isString())
        {
            return false;
        }
    }
    return true;
}

bool QSanProtocol::Utils::isIntArray(const Json::Value &jsonObject, unsigned int startIndex, unsigned int endIndex)
{
    if (!jsonObject.isArray() || jsonObject.size() <= endIndex)
    {
        return false;
    }
    for (unsigned int i = startIndex; i <= endIndex; i++)
    {
        if (!jsonObject[i].isInt())
        {
            return false;
        }
    }
    return true;
}

bool QSanProtocol::QSanGeneralPacket::tryParse(const string &s, int &val)
{
    istringstream iss(s);
    iss >> val;
    return true;
}

bool QSanProtocol::QSanGeneralPacket::parse(const string &s)
{
    if (s.length() > S_MAX_PACKET_SIZE)
    {
        return false;
    }

    Json::Value result;
    bool success = m_jsonReader.parse(s, result);
    if (!success || !Utils::isIntArray(result, 0, 3) || result.size() > 5)
    {
        return false;
    }    

    m_globalSerial = result[0].asInt();
    m_localSerial = result[1].asInt();
    m_packetType = (PacketType)result[2].asInt();
    m_command = (CommandType)result[3].asInt();

    if (result.size() == 5)
        parseBody(result[4]);
    return true;
}

string QSanProtocol::QSanGeneralPacket::toString() const
{
    Json::Value result(Json::arrayValue);
    result[0] = m_globalSerial;
    result[1] = m_localSerial;
    result[2] = m_packetType;
    result[3] = m_command;
    const Json::Value &body = constructBody();
    if (body != Json::nullValue)
        result[4] = body;
    
    string msg = result.toStyledString();
    msg.erase(remove_if(msg.begin(), msg.end(), (int(*)(int))isspace), msg.end());
    //truncate too long messages
    if (msg.length() > S_MAX_PACKET_SIZE)
    {
        return msg.substr(0, S_MAX_PACKET_SIZE);
    }
    return msg;
}
