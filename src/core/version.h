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

#ifndef _VERSION_H
#define _VERSION_H

#include <QString>

class QSanVersionNumber{
public:
    enum VersionType{
        alpha,
        beta,
        offical,
        other = -1
    };

    explicit QSanVersionNumber(const QString &ver_str);
    QSanVersionNumber(int major, int minor, int sub, VersionType type = offical, int step = 0);

    // Actually only these 2 operator overloads take effect here...
    bool operator <(const QSanVersionNumber &arg2) const;
    bool operator ==(const QSanVersionNumber &arg2) const;

    // these 4 operator overloads are just for convenience...
    bool operator >(const QSanVersionNumber &arg2) const;
    bool operator !=(const QSanVersionNumber &arg2) const;
    bool operator <=(const QSanVersionNumber &arg2) const;
    bool operator >=(const QSanVersionNumber &arg2) const;

    operator QString() const;
    QString toString() const;
    bool tryParse(const QString &ver_str);

private:
    int m_major;
    int m_minor;
    int m_sub;
    VersionType m_type;
    int m_step;
};

struct UpdateInfoStruct {
    QString version_number;
    bool is_patch;
    QString address;
};

#endif // _VERSION_H
