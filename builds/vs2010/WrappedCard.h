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


    // Inherited member functions
    inline virtual bool isOnce() const {return m_card->isOnce();}
    inline virtual bool isMute() const {return m_card->isMute();}
    inline virtual bool willThrow() const {return m_card->willThrow();}
    inline virtual bool canJilei() const {return m_card->canJilei();}
    inline virtual bool hasPreAction() const {return m_card->hasPreAction();}
    inline virtual QString getPackage() const {return m_card->getPackage();}
    inline virtual bool isVirtualCard() const { return false; }
    inline virtual bool isEquipped() const { return m_card->isEquipped(); }
    inline virtual QString getCommonEffectName() const { return m_card->getCommonEffectName(); }
    inline virtual bool match(const QString &pattern) const { return m_card->match(pattern); }

    inline virtual void addSubcard(int card_id) { Q_ASSERT(false); }
    inline virtual void addSubcard(const Card *card) { Q_ASSERT(false); }
    inline virtual void addSubcards(const QList<const Card *> &cards) { Q_ASSERT(false); }
    // inline virtual QList<int> getSubcards() const;
    // inline virtual void clearSubcards();
    // inline virtual QString subcardString() const;
    // inline virtual int subcardsLength() const;

    inline virtual QString getType() const { return m_card->getType(); }
    inline virtual QString getSubtype() const { return m_card->getSubtype(); }
    inline virtual CardType getTypeId() const { return m_card->getTypeId(); }
    inline virtual QString toString() const { return m_card->toString(); }
    inline virtual bool isNDTrick() const
    { return m_card->isNDTrick(); }

    // card target selection
    inline virtual bool targetFixed() const { return m_card->targetFixed(); }
    inline virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
    { return m_card->targetsFeasible(targets, Self); }

    // @todo: the following two functions should be merged into one.
    inline virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
    {
        return m_card->targetFilter(targets, to_select, Self);
    }
    
    inline virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self,
                              int &maxVotes) const
    {
        return m_card->targetFilter(targets, to_select, Self, maxVotes);
    }

    inline virtual bool isAvailable(const Player *player) const { return m_card->isAvailable(player); }
    
    inline virtual const Card *validate(const CardUseStruct *cardUse) const
    { return m_card->validate(cardUse); }

    inline virtual const Card *validateInResposing(ServerPlayer *user, bool &continuable) const
    { return m_card->validateInResposing(user, continuable); }

    inline virtual void doPreAction(Room *room, const CardUseStruct &cardUse) const 
    { m_card->doPreAction(room, cardUse); }
    
    inline virtual void onUse(Room *room, const CardUseStruct &cardUse) const { onUse(room, cardUse); }

    inline virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
    {
        m_card->use(room, source, targets);
    }

    inline virtual void onEffect(const CardEffectStruct &effect) const { m_card->onEffect(effect); }
    
    inline virtual bool isCancelable(const CardEffectStruct &effect) const { return m_card->isCancelable(effect); }

    inline virtual bool isKindOf(const char* cardType) const { return m_card->inherits(cardType); }

protected:
    Card* m_card;
    bool m_isModified;
};



#endif
