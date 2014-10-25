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

#ifndef _CLIENT_PLAYER_H
#define _CLIENT_PLAYER_H

#include "player.h"
#include "clientstruct.h"

class Client;
class QTextDocument;

class ClientPlayer : public Player {
    Q_OBJECT
    Q_PROPERTY(int handcard READ getHandcardNum WRITE setHandcardNum)

public:
    explicit ClientPlayer(Client *client);
    virtual QList<const Card *> getHandcards() const;
    void setCards(const QList<int> &card_ids);
    QTextDocument *getMarkDoc() const;
    void changePile(const QString &name, bool add, QList<int> card_ids);
    QString getDeathPixmapPath() const;
    void setHandcardNum(int n);
    virtual QString getGameMode() const;

    virtual void setFlags(const QString &flag);
    virtual int aliveCount(bool includeRemoved = true) const;
    virtual int getHandcardNum() const;
    virtual void removeCard(const Card *card, Place place);
    virtual void addCard(const Card *card, Place place);
    virtual void addKnownHandCard(const Card *card);
    virtual bool isLastHandCard(const Card *card, bool contain = false) const;
    virtual void setMark(const QString &mark, int value);

    virtual QStringList getBigKingdoms(const QString &reason, MaxCardsType::MaxCardsCount type = MaxCardsType::Min) const;

    virtual void setHeadSkinId(int id);
    virtual void setDeputySkinId(int id);

private:
    int handcard_num;
    QList<const Card *> known_cards;
    QTextDocument *mark_doc;

signals:
    void pile_changed(const QString &name);
    void drank_changed();
    void action_taken();
    //void skill_state_changed(const QString &skill_name);
    void duanchang_invoked();
    void headSkinIdChanged(const QString &generalName);
    void deputySkinIdChanged(const QString &generalName);
};

extern ClientPlayer *Self;

#endif

