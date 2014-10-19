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

#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QWidget>

class QLabel;
class QTextEdit;

struct UpdateInfoStruct;

class UpdateChecker : public QWidget
{
    Q_OBJECT

public:
    explicit UpdateChecker();

    void fill(UpdateInfoStruct info);
    void clear();

private:
    QLabel *state_label;
    QLabel *address_label;
    QTextEdit *page;
};

#endif // UPDATECHECKER_H
