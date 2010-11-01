#ifndef STANDARDAI_H
#define STANDARDAI_H

#include "ai.h"

class CaocaoAI: public SmartAI{
    Q_OBJECT

public:
    Q_INVOKABLE CaocaoAI(ServerPlayer *player);

    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) ;
};

class SimayiAI: public SmartAI{
    Q_OBJECT

public:
    Q_INVOKABLE SimayiAI(ServerPlayer *player);

    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) ;
};

class XiahoudunAI: public SmartAI{
    Q_OBJECT

public:
    Q_INVOKABLE XiahoudunAI(ServerPlayer *player);

    virtual bool askForSkillInvoke(const QString &skill_name, const QVariant &data) ;
};

class LumengAI: public SmartAI{
    Q_OBJECT

public:
    Q_INVOKABLE LumengAI(ServerPlayer *player);
};

#endif // STANDARDAI_H
