#ifndef SPPACKAGE_H
#define SPPACKAGE_H

#include "package.h"
#include "card.h"

class SPPackage: public Package{
    Q_OBJECT

public:
    SPPackage();
};

class SPCardPackage: public Package{
    Q_OBJECT

public:
    SPCardPackage();
};

#endif // SPPACKAGE_H
