#ifndef CRYPTO_H
#define CRYPTO_H

#include <QString>

class Crypto{
public:
    typedef enum{
        Encryption,
        Decryption
    }Method;

    static int getKeySize();
    static void setKey(const char *key);

    static void backupKey();
    static void restoreKey();

    static const char *encrypt(const char *in, size_t &length);
    static const char *decrypt(const char *in, size_t &length);

    static void encryptFile(const QString &from, const QString &to);
    static void decryptFile(const QString &from, const QString &to);
    static const char *decryptFile(const QString &filename, qint32 &length);

    static int encryptFiles(const QString &dirname, const QString &fromSuffix, const QString &toSuffix);
    static int decryptFiles(const QString &dirname, const QString &fromSuffix, const QString &toSuffix);

    static QString getVersion();
};

#endif // CRYPTO_H
