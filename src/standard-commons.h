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

class Mashu: public GameStartSkill{
public:
    Mashu():GameStartSkill("mashu")
    {
        frequency = Compulsory;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        player->getRoom()->setPlayerCorrect(player, "M");
    }
};

#endif // STANDARDCOMMONS_H
