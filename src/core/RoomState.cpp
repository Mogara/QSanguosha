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

#include "RoomState.h"
#include "engine.h"
#include "WrappedCard.h"

RoomState::~RoomState() {
    foreach (WrappedCard *card, m_cards) {
        delete card;
    }
    m_cards.clear();
}

Card *RoomState::getCard(int cardId) const{
    if (!m_cards.contains(cardId))
        return NULL;
    return m_cards[cardId];
}

void RoomState::resetCard(int cardId) {
    Card *newCard = Card::Clone(Sanguosha->getEngineCard(cardId));
    if (newCard == NULL) return;
    newCard->setFlags(m_cards[cardId]->getFlags());
    m_cards[cardId]->copyEverythingFrom(newCard);
    newCard->clearFlags();
    m_cards[cardId]->setModified(false);
}

// Reset all cards, generals' states of the room instance
void RoomState::reset() {
    foreach (WrappedCard *card, m_cards) {
        delete card;
    }
    m_cards.clear();

    int n = Sanguosha->getCardCount();
    for (int i = 0; i < n; i++) {
        Card *newCard = Card::Clone(Sanguosha->getEngineCard(i));
        m_cards[i] = new WrappedCard(newCard);
    }
}
