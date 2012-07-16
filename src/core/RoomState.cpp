#include "RoomState.h"
#include "engine.h"
#include "card.h"

RoomState::~RoomState()
{
    foreach (Card* card, m_cards.values())
    {
        delete card;
    }
    m_cards.clear();
}

const Card* RoomState::getCard(int cardId) const
{
    if (!m_cards.contains(cardId))
    {
        return NULL;
    }
    return m_cards[cardId];
}

bool RoomState::setCard(int cardId, Card *newCard)
{
   Card* oldCard;
   if (!m_cards.contains(cardId))
       oldCard = NULL;
   else
       oldCard = m_cards[cardId];
   Q_ASSERT(oldCard == NULL || oldCard->getId() == cardId);
   if (oldCard == newCard) return true;

   m_cards[cardId] = newCard;
   newCard->setId(cardId);
   newCard->setModified(true);
   // @todo: notify client of the change in the future;

   if (oldCard != NULL) {
       delete oldCard;
   }

   return true;
}

void RoomState::resetCard(int cardId)
{
    Card* newCard = Card::Clone(Sanguosha->getEngineCard(cardId));
    if (newCard == NULL) return;
    setCard(cardId, newCard);
    newCard->setModified(false);
}

// Reset all cards, generals' states of the room instance
void RoomState::reset()
{
    int n = Sanguosha->getCardCount();
    for (int i = 0; i < n; i++)
    {
        resetCard(i);
    }
}