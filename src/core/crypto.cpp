#include "crypto.h"
#include "tomcrypt.h"

#include <QFile>
#include <QDir>

static const int KEYSIZE = 8;
static const int BLOCKSIZE = 8;
static const int ROUNDS = 16;

static unsigned char key[KEYSIZE];

int Crypto::getKeySize(){
    return KEYSIZE;
}

void Crypto::setKey(const char *k){
    memcpy(key, k, KEYSIZE);
}

QString Crypto::getVersion(){
    return SCRYPT;
}

typedef int (* process_func)(const unsigned char *, unsigned char *, symmetric_key *skey);

static const char *process(const char *in, size_t &length, process_func func){
    symmetric_key skey;
    int result = des_setup(key, KEYSIZE, ROUNDS, &skey);
    if(result != CRYPT_OK)
        return NULL;

    length += length % BLOCKSIZE;
    char *out = new char[length];

    const int n = length / BLOCKSIZE;
    for(int i=0; i<n; i++){
        const int offset = i * BLOCKSIZE;
        const unsigned char *ct = reinterpret_cast<const unsigned char *>(in + offset);
        unsigned char *pt = reinterpret_cast<unsigned char *>(out + offset);
        int result = func(ct, pt, &skey);
        if(result != CRYPT_OK){
            delete[] out;
            return NULL;
        }
    }

    des_done(&skey);
    return out;
}

const char *Crypto::encrypt(const char *in, size_t &length){
    return process(in, length, des_ecb_encrypt);
}

const char *Crypto::decrypt(const char *in, size_t &length){
    return process(in, length, des_ecb_decrypt);
}

void Crypto::encryptFile(const QString &from, const QString &to){
    QFile in(from);
    if(!in.open(QIODevice::ReadOnly))
        return;

    QFile out(to);
    if(!out.open(QIODevice::WriteOnly))
        return;

    QByteArray data = in.readAll();
    size_t length = data.length();
    qint32 originalLength = length;
    const char *result = Crypto::encrypt(data.constData(), length);

    out.write((char *)&originalLength, sizeof(qint32));
    out.write(result, length);

    delete[] result;
}

const char *Crypto::decryptFile(const QString &filename, qint32 &length){
    QFile in(filename);
    if(!in.open(QIODevice::ReadOnly))
        return NULL;

    in.read((char *)&length, sizeof(qint32));

    QByteArray data = in.readAll();
    size_t dataLength = data.length();
    return Crypto::decrypt(data.constData(), dataLength);
}

void Crypto::decryptFile(const QString &from, const QString &to){
    QFile out(to);
    if(out.open(QIODevice::WriteOnly)){
        qint32 length = 0;
        const char *result = Crypto::decryptFile(from, length);
        out.write(result, length);
    }
}

typedef void (* process_method)(const QString &, const QString &);

static int process_files(const QString &dirname, const QString &fromSuffix, const QString &toSuffix, process_method method){
    QDir dir(dirname);
    int count = 0;
    foreach(QFileInfo info, dir.entryInfoList(QStringList() << "*." + fromSuffix)){
        QString from = info.absoluteFilePath();
        QString to = dir.filePath(QString("%1.%2").arg(info.baseName()).arg(toSuffix));

        method(from, to);

        count ++;
    }

    return count;
}

int Crypto::encryptFiles(const QString &dirname, const QString &fromSuffix, const QString &toSuffix){
    return process_files(dirname, fromSuffix, toSuffix, &Crypto::encryptFile);
}

int Crypto::decryptFiles(const QString &dirname, const QString &fromSuffix, const QString &toSuffix){
    return process_files(dirname, fromSuffix, toSuffix, &Crypto::decryptFile);
}
