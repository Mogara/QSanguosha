#ifndef CHALLENGEMODE_H
#define CHALLENGEMODE_H

#include "package.h"

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



#endif // CHALLENGEMODE_H
