#ifndef _WRAPPED_CARD_H
#define _WRAPPED_CARD_H
#include <card.h>

// This is a wrapper class around a card. Each card id should have one and only one WrappedCard
// copy in each room after game initialization is done. Each room's WrappedCards are isolated,
// but inside the room they are shared and synced between server/client.
//
// WrappedCard's internal card is only intended to provide CardEffect (the card face). The suit,
// number should not be modified to refelct the updated suit/number of WrappedCard. The modified
// suit/number/flags/... are maintained in WrappedCard's own member variables.
//
// All WrappedCard's member function that takes a Card as parameter will take over the Card passed
// in, meaning that the caller is resposible for allocating the memory, but WrappedCard is responsible
// for destroying it. No caller should ever delete any card that has been passed in to any member function
// of WrappedCard that takes Card* as parameter (unless the parameter is (const Card *)).
//
// WrappedCard should never have any subcard!!! It's a concrete, single piece card in the room no matter when.

class WrappedCard : public Card
{
    Q_OBJECT

public:

    WrappedCard(Suit suit, int number, bool target_fixed = false);
    WrappedCard(Card* card);
    ~WrappedCard();

    // Set the internal card to be the new card, update everything related
    // to CardEffect including objectName.
    void takeOver(Card* card);
    void copyEverythingFrom(Card* card);
    inline virtual bool isVirtualCard() const { return false; }
    virtual bool isEquipped() const;
    virtual QString getCommonEffectName() const;
    virtual bool match(const QString &pattern) const;

    inline virtual void addSubcard(int card_id) { Q_ASSERT(false); }
    inline virtual void addSubcard(const Card *card) { Q_ASSERT(false); }
    inline virtual void addSubcards(const QList<const Card *> &cards) { Q_ASSERT(false); }
    // inline virtual QList<int> getSubcards() const;
    // inline virtual void clearSubcards();
    // inline virtual QString subcardString() const;
    // inline virtual int subcardsLength() const;

    inline virtual QString getType() const { return m_card->getType(); }
    inline virtual QString getSubtype() const { return m_card->getSubtype(); }
    virtual CardType getTypeId() const = 0;
    virtual QString toString() const;
    virtual bool isNDTrick() const;

    // card target selection
    virtual bool targetFixed() const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    // @todo: the following two functions should be merged into one.
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self,
                              int &maxVotes) const;
    virtual bool isAvailable(const Player *player) const;
    virtual const Card *validate(const CardUseStruct *card_use) const;
    virtual const Card *validateInResposing(ServerPlayer *user, bool *continuable) const;
    virtual void doPreAction(Room *room, const CardUseStruct &card_use) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    inline virtual bool isKindOf(const char* cardType) const { return inherits(cardType); }

protected:
    Card* m_card;
};



#endif