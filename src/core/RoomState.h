#ifndef _H_ROOM_STATE
#define _H_ROOM_STATE
#include <QHash>
#include "player.h"
#include "structs.h"
#include "WrappedCard.h"

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
    inline void setCurrentPlayer(Player* player) { m_currentPlayer = player; }
    inline QString getCurrentCardUsePattern() const { return m_currentCardUsePattern; }
    inline void setCurrentCardUsePattern(const QString& newPattern) 
    { m_currentCardUsePattern = newPattern; }
    inline Player* getCurrentPlayer() const { return m_currentPlayer; }
    inline CardUseStruct::CardUseReason getCurrentCardUseReason() const { return m_currentCardUseReason; }
    inline void setCurrentCardUseReason(CardUseStruct::CardUseReason reason) { m_currentCardUseReason = reason; }

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
    QHash<int, WrappedCard*> m_cards;
    bool m_isClient;
    Player *m_currentPlayer;
    QString m_currentCardUsePattern;
    CardUseStruct::CardUseReason m_currentCardUseReason;
};

#endif
