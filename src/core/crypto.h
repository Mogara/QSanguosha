#ifndef CRYPTO_H
#define CRYPTO_H
#include "fmod.h"
#include <QtCore/QCoreApplication>
#include <crypto++/des.h>
#include <stdio.h>
#include <QFileInfo>

class Crypto{
public:
    bool encryptMusicFile(const QString &filename, const char *GlobalKey = "http://qsanguosha.org");
    FMOD_SOUND *initEncryptedFile(FMOD_SYSTEM *System, const QString &filename, const char *GlobalKey = "http://qsanguosha.org");
};

#endif // CRYPTO_H
