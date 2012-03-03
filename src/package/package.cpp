#include "package.h"

Q_GLOBAL_STATIC(PackageHash, Packages)
PackageHash& PackageAdder::packages(){
    return *(::Packages());
}
