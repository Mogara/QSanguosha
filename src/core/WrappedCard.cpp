#include "WrappedCard.h"

WrappedCard::WrappedCard(Card* card):
             Card(card->getSuit(), card->getNumber())
{
    m_card = NULL;
    m_isModified = false;
    copyEverythingFrom(card);
}

WrappedCard::~WrappedCard()
{
    Q_ASSERT(m_card != NULL);
    delete m_card;
}

void WrappedCard::takeOver(Card* card)
{
    if (m_card != NULL){
        m_isModified = true;
        delete m_card;
    }
    setObjectName(card->objectName());
    m_card = card;
}

void WrappedCard::copyEverythingFrom(Card* card)
{
    takeOver(card);
    setId(card->getEffectiveId());
    setSuit(card->getSuit());
    setNumber(card->getNumber());
    flags = card->getFlags();
    skill_name = card->getSkillName();
}
