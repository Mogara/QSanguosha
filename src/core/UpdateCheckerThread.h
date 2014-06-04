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

#ifndef _UPDATE_CHECKER_THREAD
#define _UPDATE_CHECKER_THREAD

#include <QThread>
#include <QStringList>

struct UpdateInfoStruct {
    QString version_number;
    bool is_patch;
    QString address;
};

class UpdateCheckerThread : public QThread {
    Q_OBJECT

public:
    explicit UpdateCheckerThread();
    virtual void run();

signals:
    void storeKeyAndValue(const QString &key, const QString &value);

};

#endif