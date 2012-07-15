#ifndef _H_ROOM_STATE
#define _H_ROOM_STATE
#include <QHash>

class Card;

// RoomState is a singleton that stores virtual generals, cards (versus factory loaded
// generals, cards in the Engine). Each room or roomscene should have one and only one
// associated RoomState.

class RoomState
{
public:
    inline RoomState(bool isClient) { m_isClient = isClient; }
    ~RoomState();
    inline bool isClient() const { return m_isClient; }
    Card* getCard(int cardId) const;

    // Update a card in the room.
    // @param cardId
    //        Id of card to be updated.
    // @param newCard
    //        Card to be updated in the room.
    // @return
    bool setCard(int cardId, Card *newCard);
    void resetCard(int cardId);
    // Reset all cards, generals' states of the room instance
    void reset();
protected:
    QHash<int, Card*> m_cards;
    bool m_isClient;
};

#endif