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

#ifndef _ROOM_STATE_H
#define _ROOM_STATE_H

#include "player.h"
#include "structs.h"
#include "wrappedcard.h"

// RoomState is a singleton that stores virtual generals, cards (versus factory loaded
// generals, cards in the Engine). Each room or roomscene should have one and only one
// associated RoomState.

class RoomState {
public:

    inline RoomState(bool isClient) { m_isClient = isClient; }
    ~RoomState();
    inline bool isClient() const{ return m_isClient; }
    Card *getCard(int cardId) const;
    inline void setCurrentPlayer(Player *player) { m_currentPlayer = player; }
    inline QString getCurrentCardUsePattern() const{ return m_currentCardUsePattern; }
    inline void setCurrentCardUsePattern(const QString &newPattern) { m_currentCardUsePattern = newPattern; }
    inline Player *getCurrentPlayer() const{ return m_currentPlayer; }
    inline CardUseStruct::CardUseReason getCurrentCardUseReason() const{ return m_currentCardUseReason; }
    inline void setCurrentCardUseReason(CardUseStruct::CardUseReason reason) { m_currentCardUseReason = reason; }

    inline QString getCurrentCardResponsePrompt() const
    {
        return m_currentCardResponsePrompt;
    }

    inline void setCurrentCardResponsePrompt(const QString &prompt)
    {
        m_currentCardResponsePrompt = prompt;
    }

    // Update a card in the room.
    // @param cardId
    //        Id of card to be updated.
    // @param newCard
    //        Card to be updated in the room.
    // @return
    void resetCard(int cardId);
    // Reset all cards, generals' states of the room instance
    void reset();

protected:
    QHash<int, WrappedCard *> m_cards;
    bool m_isClient;
    Player *m_currentPlayer;
    QString m_currentCardUsePattern;
    CardUseStruct::CardUseReason m_currentCardUseReason;
    QString m_currentCardResponsePrompt;
};

#endif

