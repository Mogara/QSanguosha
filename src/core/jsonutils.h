#ifndef _QSAN_JSON_UTILS_H
#define _QSAN_JSON_UTILS_H

#include <string>
#include <list>
#include <json/json.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qrect.h>
#include <qcolor.h>

namespace QSanProtocol
{
    namespace Utils
    {        
        inline QString toQString(const Json::Value& value)
        {
            Q_ASSERT(value.isString());
            return QString::fromLocal8Bit(value.asCString());
        }
        inline Json::Value toJsonString(const QString& s)
        {            
            return Json::Value(s.toAscii().constData());
        }
        Json::Value toJsonArray(const QString& s1, const QString& s2);
        Json::Value toJsonArray(const QString& s1, const Json::Value& s2);
        Json::Value toJsonArray(const QString& s1, const QString& s2, const QString& s3);
        Json::Value toJsonArray(const QList<int>&);
        Json::Value toJsonArray(const QList<QString>&);
        Json::Value toJsonArray(const QStringList&);
        bool tryParse(const Json::Value&, QList<int> &);
        bool tryParse(const Json::Value&, QStringList &);
        bool tryParse(const Json::Value&, QRect &);
        bool tryParse(const Json::Value& arg, QSize& result);
        bool tryParse(const Json::Value& arg, QPoint& result);
        bool tryParse(const Json::Value& arg, QColor& result);
    }
}

#endif