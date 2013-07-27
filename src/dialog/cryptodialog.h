#ifndef CRYPTODIALOG_H
#define CRYPTODIALOG_H

#include <QDialog>
#include <QThread>

#include "crypto.h"

class CryptoThread: public QThread{
    Q_OBJECT

public:
    CryptoThread(const QStringList &filenames, Crypto::Method method)
        :filenames(filenames), method(method){}

    QStringList getFilenames() const{
        return filenames;
    }

    Crypto::Method getMethod() const{
        return method;
    }

    virtual void run();

private:
    QStringList filenames;
    Crypto::Method method;

signals:
    void fileDone(const QString &filename);
};

class CryptoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CryptoDialog(QWidget *parent = 0);

private:
    QString getKeyFromUser();
    
private slots:
    void encrypt();

#ifdef QT_DEBUG
    void decrypt();
#endif

    void onFileDone(const QString &filename);
    void onCryptDone();
};

#endif // CRYPTODIALOG_H
