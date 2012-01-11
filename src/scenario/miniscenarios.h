#ifndef MINISCENARIOS_H
#define MINISCENARIOS_H

#include "scenario.h"

class MiniScene : public Scenario
{
    Q_OBJECT

public:
    MiniScene(const QString &name)
        :Scenario(name){};
    virtual void onTagSet(Room *room, const QString &key) const;
};

class MiniScene_01 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_01();
};

class MiniScene_02 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_02();
};

class MiniScene_03 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_03();
};

class MiniScene_04 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_04();
};

class MiniScene_05 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_05();
};

class MiniScene_06 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_06();
};

class MiniScene_07 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_07();
};

class MiniScene_08 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_08();
};

class MiniScene_09 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_09();
};

class MiniScene_10 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_10();
};

#endif // MINISCENARIOS_H
