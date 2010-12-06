#ifndef CHALLENGEMODE_H
#define CHALLENGEMODE_H

#include "package.h"
#include "gamerule.h"

class ChallengeMode : public QObject{
    Q_OBJECT

public:
    ChallengeMode(const QString &name, const QString &general_str);
    QStringList getGenerals() const;

private:
    QStringList generals;
};

class ChallengeModeSet: public Package{
    Q_OBJECT

public:
    ChallengeModeSet(QObject *parent);

    const ChallengeMode *getMode(const QString &name) const;
    QList<const ChallengeMode *> allModes() const;

private:
    void addMode(const QString &name, const QString &general_str);
    QHash<QString, const ChallengeMode *> modes;
};

class ChallengeModeRule: public GameRule{
    Q_OBJECT

public:
    ChallengeModeRule(QObject *parent);

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const;
};

#endif // CHALLENGEMODE_H
