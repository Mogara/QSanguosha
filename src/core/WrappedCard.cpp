#include "WrappedCard.h"

WrappedCard::WrappedCard(Card* card):
    Card(card->getSuit(), card->getNumber()), m_card(NULL), m_isModified(false)
{
    copyEverythingFrom(card);
}

WrappedCard::~WrappedCard()
{
    Q_ASSERT(m_card != NULL);
    delete m_card;
}

void WrappedCard::takeOver(Card* card)
{
    if (m_card != NULL){m_isModified = true;delete m_card;}
    setObjectName(card->objectName());
    setSuit(card->getSuit());
    setNumber(card->getNumber());
    if(getId() > -1)
        m_card->setId(getId());
    m_card = card;
}

void WrappedCard::copyEverythingFrom(Card* card)
{
    takeOver(card);
    setId(card->getEffectiveId());
    flags = card->getFlags();
    skill_name = card->getSkillName();
}
