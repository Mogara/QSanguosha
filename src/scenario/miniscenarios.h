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

class MiniScene_11 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_11();
};

class MiniScene_12 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_12();
};

class MiniScene_13 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_13();
};

class MiniScene_14 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_14();
};

class MiniScene_15 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_15();
};

class MiniScene_16 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_16();
};

class MiniScene_17 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_17();
};

class MiniScene_18 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_18();
};

class MiniScene_19 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_19();
    AI::Relation relationTo(const ServerPlayer *a, const ServerPlayer *b) const;
};

class MiniScene_20 : public MiniScene
{
    Q_OBJECT

public:
    MiniScene_20();
};

#endif // MINISCENARIOS_H
