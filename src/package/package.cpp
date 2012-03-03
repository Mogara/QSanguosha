#include "package.h"

PackageAdder::PackageHash& PackageAdder::packages(){
    return *(Packages());
}
