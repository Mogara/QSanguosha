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

#include "version.h"
#include <QRegExp>
#include <QStringList>

QSanVersionNumber::QSanVersionNumber(const QString &str){
    if (!tryParse(str))
        Q_ASSERT(false);
}

QSanVersionNumber::QSanVersionNumber(int major, int minor, int sub, VersionType type, int step)
    : m_major(major), m_minor(minor), m_sub(sub), m_type(type), m_step(step)
{

}

bool QSanVersionNumber::tryParse(const QString &str){
    QRegExp regexp("(\\d+)\\.(\\d+)\\.(\\d+)(\\-([a-z]+)(\\d+))?");
    if (regexp.exactMatch(str)){
        QStringList l = regexp.capturedTexts();
        m_major = l[1].toInt();
        m_minor = l[2].toInt();
        m_sub = l[3].toInt();
        if (l[5] == "alpha")
            m_type = alpha;
        else if (l[5] == "beta")
            m_type = beta;
        else if (l[5] == "offical")
            m_type = offical;
        else
            m_type = other;
        if (l.length() > 5)
            m_step = l[6].toInt();
        else
            m_step = 0;

        return true;
    }

    return false;
}

QString QSanVersionNumber::toString() const{
    QString str = "%1.%2.%3-%4";
    QString type_str;
    switch (m_type){
    case alpha:
        type_str = "alpha";
        break;
    case beta:
        type_str = "beta";
        break;
    case offical:
        type_str = "offical";
        break;
    case other:
        type_str = "other";
        break;
    default:
        type_str = "unknown";
        break;
    }
    str = str.arg(m_major).arg(m_minor).arg(m_sub).arg(type_str);
    if (m_step != 0)
        str.append(QString::number(m_step));

    return str;
}

QSanVersionNumber::operator QString() const{
    return toString();
}

bool QSanVersionNumber::operator ==(const QSanVersionNumber &arg2) const{
    return (m_major == arg2.m_major
            && m_minor == arg2.m_minor
            && m_sub == arg2.m_sub
            && m_type == arg2.m_type
            && m_step == arg2.m_step);
}

bool QSanVersionNumber::operator <(const QSanVersionNumber &arg2) const{
    if (m_major < arg2.m_major) {
        return true;
    } else if (m_major == arg2.m_major) {
        if (m_minor < arg2.m_minor) {
            return true;
        } else if (m_minor == arg2.m_minor) {
            if (m_sub < arg2.m_sub) {
                return true;
            } else if (m_sub == arg2.m_sub){
                if (m_type < arg2.m_type)
                    return true;
                else
                    return m_step < arg2.m_step;
            }
        }
    }
    return false;
}

bool QSanVersionNumber::operator >(const QSanVersionNumber &arg2) const{
    const QSanVersionNumber &arg1 = *this;
    return !(arg1 == arg2 || arg1 < arg2);
}

bool QSanVersionNumber::operator !=(const QSanVersionNumber &arg2) const{
    const QSanVersionNumber &arg1 = *this;
    return !(arg1 == arg2);
}

bool QSanVersionNumber::operator <=(const QSanVersionNumber &arg2) const{
    const QSanVersionNumber &arg1 = *this;
    return (arg1 == arg2 || arg1 < arg2);
}

bool QSanVersionNumber::operator >=(const QSanVersionNumber &arg2) const{
    const QSanVersionNumber &arg1 = *this;
    return !(arg1 < arg2);
}
