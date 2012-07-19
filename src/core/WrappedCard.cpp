#include "WrappedCard.h"

WrappedCard::WrappedCard(Card* card):
    Card(card->getSuit(), card->getNumber()), m_card(NULL), m_isModified(false)
{
    m_id = card->getId();
    copyEverythingFrom(card);
}

WrappedCard::~WrappedCard()
{
    Q_ASSERT(m_card != NULL);
    delete m_card;
}

void WrappedCard::takeOver(Card* card)
{
    Q_ASSERT(getId() >= 0);
    Q_ASSERT(card != this);
    Q_ASSERT(m_card != card);
    if (m_card != NULL) {
        m_isModified = true;
        delete m_card;
    }
    m_card = card;
    setObjectName(card->objectName());
    setSuit(card->getSuit());
    setNumber(card->getNumber());
    m_card->setId(getId());
    card->setId(getId());
    card->setSuit(getSuit());
    card->setNumber(getNumber());
    card->setFlags(flags);
    m_skillName = card->getSkillName();
}

void WrappedCard::copyEverythingFrom(Card* card)
{
    Q_ASSERT(card->getId() >= 0);
    Q_ASSERT(card != this);
    Q_ASSERT(m_card != card);
    if (m_card != NULL) {
        m_isModified = true;
        delete m_card;
    }
    setObjectName(card->objectName());
    m_card = card;
    Card::setId(card->getEffectiveId());
    Card::setSuit(card->getSuit());
    Card::setNumber(card->getNumber());
    flags = card->getFlags();
    m_skillName = card->getSkillName();
}
