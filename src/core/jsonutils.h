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

#ifndef _JSON_UTILS_H
#define _JSON_UTILS_H


#include "json/json.h"

#include <string>
#include <list>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qrect.h>
#include <qcolor.h>

namespace QSanProtocol
{
    namespace Utils
    {
        inline QString toQString(const Json::Value &value) {
            Q_ASSERT(value.isString());
            return QString::fromUtf8(value.asCString());
        }
        inline Json::Value toJsonString(const QString &s) {
            return Json::Value(s.toUtf8().constData());
        }
        Json::Value toJsonArray(const QString &s1, const QString &s2);
        Json::Value toJsonArray(const QString &s1, const Json::Value &s2);
        Json::Value toJsonArray(const QString &s1, const QString &s2, const QString &s3);
        Json::Value toJsonArray(const QList<int> &);
        Json::Value toJsonArray(const QList<QString> &);
        Json::Value toJsonArray(const QStringList &);
        bool tryParse(const Json::Value &, int &);
        bool tryParse(const Json::Value &, double &);
        bool tryParse(const Json::Value &, bool &);
        bool tryParse(const Json::Value &, QList<int> &);
        bool tryParse(const Json::Value &, QString &);
        bool tryParse(const Json::Value &, QStringList &);
        bool tryParse(const Json::Value &, QRect &);
        bool tryParse(const Json::Value &arg, QSize &result);
        bool tryParse(const Json::Value &arg, QPoint &result);
        bool tryParse(const Json::Value &arg, QColor &result);
        bool tryParse(const Json::Value &arg, Qt::Alignment &align);
    }
}

#endif

