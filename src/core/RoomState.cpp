#include "RoomState.h"
#include "engine.h"
#include "WrappedCard.h"

Card* RoomState::getCard(int cardId) const
{
    if (!m_cards.contains(cardId))
    {
        return NULL;
    }
    return m_cards[cardId];
}

void RoomState::resetCard(int cardId)
{
    Card* newCard = Card::Clone(Sanguosha->getEngineCard(cardId));
    if (newCard == NULL) return;
    m_cards[cardId]->copyEverythingFrom(newCard);
    // newCard->setModified(false);
}

// Reset all cards, generals' states of the room instance
void RoomState::reset()
{
    foreach (WrappedCard* card, m_cards.values())
    {
        delete card;
    }
    m_cards.clear();

    int n = Sanguosha->getCardCount();
    for (int i = 0; i < n; i++)
    {
        m_cards[i] = new WrappedCard(Card::Clone(Sanguosha->getEngineCard(i)));
    }
}