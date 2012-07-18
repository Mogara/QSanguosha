#include "WrappedCard.h"

WrappedCard::WrappedCard(Card* card):
             Card(card->getSuit(), card->getNumber())
{
    m_card = NULL;
    copyEverythingFrom(card);
}

WrappedCard::~WrappedCard()
{
    Q_ASSERT(m_card != NULL);
    delete m_card;
}

void WrappedCard::takeOver(Card* card)
{
    if (m_card != NULL) delete m_card;
    setObjectName(card->objectName());
    m_card = card;
}

void WrappedCard::copyEverythingFrom(Card* card)
{
    takeOver(card);
    setId(card->getId());
    setSuit(card->getSuit());
    setNumber(card->getNumber());
    flags = card->getFlags();
}
