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

#ifndef _CARDBUTTON_H
#define _CARDBUTTON_H

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <QCommandLinkButton>
typedef QCommandLinkButton CommandLinkButton;
#else
#include <QPushButton>
typedef QPushButton CommandLinkButton;
#endif

class Card;

class CardButton : public CommandLinkButton
{
    Q_OBJECT
public:
    explicit CardButton(const Card *card);

    virtual QSize sizeHint() const;
    inline void setScale(double scale) { this->scale = scale; }

private:
    const Card *card;
    double scale;

private slots:
    void onClicked();

signals:
    void idSelected(int id);
};

#endif // _CARDBUTTON_H