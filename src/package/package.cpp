#include "package.h"
#include "skill.h"

Package::~Package() {
    foreach(const Skill *skill, skills)
        delete skill;

    foreach(const QString key, patterns.keys())
        delete patterns[key];
}

Q_GLOBAL_STATIC(PackageHash, Packages)
PackageHash &PackageAdder::packages() {
    return *(::Packages());
}

