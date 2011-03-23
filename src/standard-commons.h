#ifndef STANDARDCOMMONS_H
#define STANDARDCOMMONS_H

#include "skill.h"

class Qianxun: public ProhibitSkill{
public:
    Qianxun():ProhibitSkill("qianxun"){

    }

    virtual bool isProhibited(const Player *, const Player *, const Card *card) const{
        return card->inherits("Snatch") || card->inherits("Indulgence");
    }
};

class Mashu: public Skill{
public:
    Mashu():Skill("mashu", Skill::Compulsory)
    {
    }
};

#endif // STANDARDCOMMONS_H
