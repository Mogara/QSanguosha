#ifndef CLO_SOU
#include "crypto.h"
#else
#include "crypt0.h"
#endif

void DES_Process(const char *keyString, byte *block, size_t length, CryptoPP::CipherDir direction){
    using namespace CryptoPP;

    byte key[DES_EDE2::KEYLENGTH];
    memcpy(key, keyString, DES_EDE2::KEYLENGTH);
    BlockTransformation *t = NULL;

    if(direction == ENCRYPTION)
        t = new DES_EDE2_Encryption(key, DES_EDE2::KEYLENGTH);
    else
        t = new DES_EDE2_Decryption(key, DES_EDE2::KEYLENGTH);

    int steps = length / t->BlockSize();
    for(int i=0; i<steps; i++){
        int offset = i * t->BlockSize();
        t->ProcessBlock(block + offset);
    }

    delete t;
}

bool Crypto::encryptMusicFile(const QString &filename, const char *GlobalKey){
    QFileInfo info(filename);
    QString output = QString("%1/%2.dat").arg(info.absolutePath()).arg(info.baseName());

    QFile file(filename);

    if(file.open(QIODevice::ReadOnly) == false)
        return false;

    qint64 realSize = file.size();
    int padding = realSize % 8;
    qint64 size = realSize + padding;

    byte *buffer = new byte[size];

    int readed = file.read((char *)buffer, size);
    if(readed == -1){
        delete buffer;
        return false;
    }

    DES_Process(GlobalKey, buffer, size, CryptoPP::ENCRYPTION);

    QFile newFile(output);
    if(newFile.open(QIODevice::WriteOnly)){
        newFile.write((char *)buffer, size);

        delete buffer;

        return true;

    }else{
        delete buffer;
        return false;
    }
}

bool Crypto::decryptMusicFile(const QString &filename, const QString &GlobalKey){
    QFile file(filename);
    QFileInfo info(filename);
    QString output = QString("%1/%2.ogg").arg(info.absolutePath()).arg(info.baseName());

    if(file.open(QIODevice::ReadOnly) == false)
        return NULL;

    const char *key = GlobalKey.toLocal8Bit().data();
    qint64 size = file.size();
    byte *buffer = new byte[size];

    file.read((char *)buffer, size);
    DES_Process(key, buffer, size, CryptoPP::DECRYPTION);

    QFile newFile(output);
    if(newFile.open(QIODevice::WriteOnly)){
        newFile.write((char *)buffer, size);
        delete buffer;
        return true;
    }else{
        delete buffer;
        return false;
    }
}

const uchar *Crypto::getEncryptedFile(const QString &filename, const char *GlobalKey){
    QFile file(filename);

    if(file.open(QIODevice::ReadOnly) == false)
        return NULL;

    size = file.size();
    byte *buffer = new byte[size];

    file.read((char *)buffer, size);
    DES_Process(GlobalKey, buffer, size, CryptoPP::DECRYPTION);

    return (const uchar *)buffer;
}

FMOD_SOUND *Crypto::initEncryptedFile(FMOD_SYSTEM *System, const QString &filename, const char *GlobalKey){
    const uchar *buffer = getEncryptedFile(filename, GlobalKey);

    FMOD_SOUND *sound;

    FMOD_CREATESOUNDEXINFO info;
    memset(&info, 0, sizeof(info));
    info.cbsize = sizeof(info);
    info.length = size;

    FMOD_System_CreateSound(System, (const char *)buffer, FMOD_OPENMEMORY, &info, &sound);
    delete buffer;

    return sound;
}
