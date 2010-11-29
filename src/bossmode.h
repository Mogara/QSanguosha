#ifndef BOSSMODE_H
#define BOSSMODE_H

#include "gamerule.h"

class BossMode : public GameRule{
    Q_OBJECT

public:    
    BossMode(QObject *parent);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
};

#endif // BOSSMODE_H
