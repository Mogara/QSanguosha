#include "cardpattern.h"
#include "card.h"

CardPattern::CardPattern(const QString &pattern_str)
    :pattern_str(pattern_str)
{
}

QString CardPattern::toString() const{
    return QString("%1-%2").arg(metaObject()->className()).arg(pattern_str);
}

// --------------------------

NamePattern::NamePattern(const QString &pattern_str)
    :CardPattern(pattern_str)
{
}

bool NamePattern::match(const Card *card) const{
    return card->objectName() == pattern_str;
}


// --------------------------

TypePattern::TypePattern(const QString &pattern_str)
    :CardPattern(pattern_str)
{

}

bool TypePattern::match(const Card *card) const{
    return card->getType() == pattern_str;
}
