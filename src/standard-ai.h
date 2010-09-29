#ifndef STANDARDAI_H
#define STANDARDAI_H

#include "ai.h"

class CaocaoAI: public TrustAI{
    Q_OBJECT

public:
    Q_INVOKABLE CaocaoAI(ServerPlayer *player);

    virtual bool askForSkillInvoke(const QString &skill_name) const;
};

#endif // STANDARDAI_H
