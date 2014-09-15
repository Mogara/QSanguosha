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

#ifndef _CLIENT_LOG_BOX_H
#define _CLIENT_LOG_BOX_H

class ClientPlayer;

#include <QTextEdit>

class ClientLogBox : public QTextEdit {
    Q_OBJECT

public:
    explicit ClientLogBox(QWidget *parent = 0);
    void appendLog(const QString &type, const QString &from_general, const QStringList &to,
        const QString card_str = QString(), const QString arg = QString(), const QString arg2 = QString());

private:
    QString bold(const QString &str, QColor color) const;

public slots:
    void appendLog(const QStringList &log_str);
    void append(const QString &text);
};

#endif

