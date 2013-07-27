#include "cryptodialog.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QListView>
#include <QLabel>
#include <QCheckBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QStringListModel>
#include <QFile>
#include <QFileDialog>

static QString DecryptionSuffix;

CryptoDialog::CryptoDialog(QWidget *parent) :
    QDialog(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;

    QPushButton *encryptButton = new QPushButton(tr("Encrytion ..."));

#ifdef QT_DEBUG
    QPushButton *decryptButton = new QPushButton(tr("Decryption ..."));
#endif

    QCheckBox *checkbox = new QCheckBox(tr("Remove old files"));

    vbox->addWidget(encryptButton);

#ifdef QT_DEBUG
    vbox->addWidget(decryptButton);
#endif

    vbox->addWidget(checkbox);

    vbox->addWidget(new QListView);
    vbox->addStretch();

    QLabel *messageLabel = new QLabel;
    vbox->addWidget(messageLabel);

    QProgressBar *progressBar = new QProgressBar;
    vbox->addWidget(progressBar);
    progressBar->hide();

    connect(encryptButton, SIGNAL(clicked()), this, SLOT(encrypt()));

#ifdef QT_DEBUG
    connect(decryptButton, SIGNAL(clicked()), this, SLOT(decrypt()));
#endif

    setLayout(vbox);
}

QString CryptoDialog::getKeyFromUser(){
    size_t keySize = Crypto::getKeySize();
    QString key = QInputDialog::getText(this,
                                        tr("Key request"),
                                        tr("Please supply the key, length >= %1 (extra characters will be ignored)").arg(keySize));

    if(key.isEmpty())
        return QString();

    if(key.length() < (int)keySize){
        QMessageBox::warning(this, tr("Warning"), tr("Your input key is too short"));
        return QString();
    }else
        return key;
}

void CryptoDialog::encrypt()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Please select files to encrypt"));
    if(filenames.isEmpty())
        return;

    QListView *listView = findChild<QListView *>();
    if(listView){
        delete listView->model();
        listView->setModel(new QStringListModel(filenames));
    }

    QProgressBar *progressBar = findChild<QProgressBar *>();
    progressBar->setMaximum(filenames.length());
    progressBar->show();

    CryptoThread *thread = new CryptoThread(filenames, Crypto::Encryption);

    connect(thread, SIGNAL(fileDone(QString)), this, SLOT(onFileDone(QString)));
    connect(thread, SIGNAL(finished()), this, SLOT(onCryptDone()));

    thread->start();
}

#ifdef QT_DEBUG

void CryptoDialog::decrypt()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, tr("Please select files to decrypt"), QString(), tr("Encrypted files (*.dat)"));
    if(filenames.isEmpty())
        return;

#ifndef QT_DEBUG
    QString key = getKeyFromUser();
    if(key.isNull())
        return;
    QString suffix = QInputDialog::getText(this, tr("Original suffix"),
                                           tr("Please input the decrypted files' suffix"),
                                           QLineEdit::Normal, "ogg");
    if(suffix.isEmpty())
        suffix = "ogg";

    DecryptionSuffix = suffix;
#endif

    DecryptionSuffix = "ogg";

#ifndef QT_DEBUG
    Crypto::backupKey();
    Crypto::setKey(key.toLatin1());
#endif

    QListView *listView = findChild<QListView *>();
    if(listView){
        delete listView->model();
        listView->setModel(new QStringListModel(filenames));
    }

    QProgressBar *progressBar = findChild<QProgressBar *>();
    progressBar->setMaximum(filenames.length());
    progressBar->show();

    CryptoThread *thread = new CryptoThread(filenames, Crypto::Decryption);

    connect(thread, SIGNAL(fileDone(QString)), this, SLOT(onFileDone(QString)));
    connect(thread, SIGNAL(finished()), this, SLOT(onCryptDone()));

    thread->start();
}

#endif

void CryptoDialog::onFileDone(const QString &filename)
{
    QProgressBar *progressBar = findChild<QProgressBar *>();
    progressBar->setValue(progressBar->value() + 1);

    QLabel *label = findChild<QLabel *>();
    label->setText(tr("File %1 was processed").arg(filename));
}

void CryptoDialog::onCryptDone()
{
    CryptoThread *thread = qobject_cast<CryptoThread *>(sender());
    Crypto::Method method = thread->getMethod();
    QLabel *label = findChild<QLabel *>();

    QStringList filenames = thread->getFilenames();

    if(method == Crypto::Decryption){

#ifndef QT_DEBUG
        Crypto::restoreKey();
#endif

        label->setText(tr("Decrypt %1 files done!").arg(filenames.length()));
    }else if(method == Crypto::Encryption){
        label->setText(tr("Encrypt %1 files done!").arg(filenames.length()));
    }

    QProgressBar *progressBar = findChild<QProgressBar *>();
    progressBar->hide();

    QCheckBox *checkBox = findChild<QCheckBox *>();
    if(checkBox->isChecked()){
        foreach(QString filename, filenames){
            QFile::remove(filename);
        }
    }
}

void CryptoThread::run()
{
    if(method == Crypto::Encryption){
        foreach(QString filename, filenames){
            QFileInfo info(filename);
            QString to = info.dir().filePath(info.baseName() + ".dat");

            Crypto::encryptFile(filename, to);

            emit fileDone(filename);
        }
    }else if(method == Crypto::Decryption){
        foreach(QString filename, filenames){
            QFileInfo info(filename);
            QString to = info.dir().filePath(QString("%1.%2").arg(info.baseName()).arg(DecryptionSuffix));

            Crypto::decryptFile(filename, to);

            emit fileDone(filename);
        }
    }
}
