#include "cardpattern.h"
#include "card.h"

CardPattern::CardPattern(const QString &pattern_str)
    :pattern_str(pattern_str)
{
}

DirectCardPattern::DirectCardPattern(const QString &pattern_str)
    :CardPattern(pattern_str)
{
}

bool DirectCardPattern::match(const Card *card) const{
    return card->match(pattern_str);
}
