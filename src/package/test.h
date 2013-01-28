#ifndef TEST_H
#define TEST_H

#include "package.h"
#include "card.h"

class DanchuangPackage: public Package{
    Q_OBJECT

public:
    DanchuangPackage();
};

class TestPackage: public Package{
    Q_OBJECT

public:
    TestPackage();
};

#endif // TEST_H
