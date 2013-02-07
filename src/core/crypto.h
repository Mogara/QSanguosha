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
    bool decryptMusicFile(const QString &filename, const QString &GlobalKey);
    FMOD_SOUND *initEncryptedFile(FMOD_SYSTEM *System, const QString &filename, const char *GlobalKey = "http://qsanguosha.org");
    const uchar *getEncryptedFile(const QString &filename, const char *GlobalKey = "http://qsanguosha.org");
private:
    qint64 size;
};

#endif // CRYPTO_H
