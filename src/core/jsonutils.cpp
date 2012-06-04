#include "jsonutils.h"

Json::Value QSanProtocol::Utils::toJsonArray(const QString& s1, const QString& s2)
{
    Json::Value val(Json::arrayValue);
    val[0] = s1.toAscii().constData();
    val[1] = s2.toAscii().constData();
    return val;
}

Json::Value QSanProtocol::Utils::toJsonArray(const QString& s1, const QString& s2, const QString& s3)
{
    Json::Value val(Json::arrayValue);
    val[0] = s1.toAscii().constData();
    val[1] = s2.toAscii().constData();
    val[2] = s3.toAscii().constData();
    return val;
}

Json::Value QSanProtocol::Utils::toJsonArray(const QString& s1, const Json::Value& s2)
{
    Json::Value val(Json::arrayValue);
    val[0] = s1.toAscii().constData();
    val[1] = s2;
    return val;
}

Json::Value QSanProtocol::Utils::toJsonArray(const QList<int>& arg)
{
    Json::Value val(Json::arrayValue);
    foreach(int i, arg)
        val.append(i);
    return val;
}

bool QSanProtocol::Utils::tryParse(const Json::Value& arg, QList<int>& result)
{
    if (!arg.isArray()) return false;
    for (unsigned int i = 0; i< arg.size(); i++)
    {
        if (!arg[i].isInt()) return false;        
    }
    for (unsigned int i = 0; i< arg.size(); i++)
    {
        result.append(arg[i].asInt());        
    }    
    return true;
}

Json::Value QSanProtocol::Utils:: toJsonArray(const QList<QString>& arg)
{
    Json::Value val(Json::arrayValue);
    foreach(QString s, arg)
        val.append(toJsonString(s));
    return val;
}

Json::Value QSanProtocol::Utils:: toJsonArray(const QStringList& arg)
{
    Json::Value val(Json::arrayValue);
    foreach(QString s, arg)
        val.append(toJsonString(s));
    return val;
}

bool QSanProtocol::Utils::tryParse(const Json::Value& arg, QStringList& result)
{
    if (!arg.isArray()) return false;
    for (unsigned int i = 0; i< arg.size(); i++)
    {
        if (!arg[i].isString()) return false;        
    }
    for (unsigned int i = 0; i< arg.size(); i++)
    {
        result.append(arg[i].asCString());        
    }    
    return true;
}

bool QSanProtocol::Utils::tryParse(const Json::Value& arg, QRect& result)
{
    if (!arg.isArray() || arg.size() != 4) return false;
    result.setLeft(arg[0].asInt());
    result.setTop(arg[1].asInt());
    result.setWidth(arg[2].asInt());
    result.setHeight(arg[3].asInt());
	return true;
}