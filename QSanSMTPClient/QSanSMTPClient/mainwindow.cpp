#include "mainwindow.h"
#include "SmtpMime"

#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QLabel *label = new QLabel(tr("Please wait for a moment..."), this);
    label->show();
}

MainWindow::~MainWindow()
{

}

QString getSystemVersion() {
#ifdef Q_OS_WIN32
    QSysInfo::WinVersion version = QSysInfo::WinVersion();
    //DOS
    switch (version) {
    case (QSysInfo::WV_32s) :
        return "Windows 3.1 with Win 32s";
    case (QSysInfo::WV_95) :
        return "Windows 95";
    case (QSysInfo::WV_98) :
        return "Windows 98";
    case (QSysInfo::WV_Me) :
        return "Windows Me";
    default :
        break;
    }

    //NT
    switch (version) {
    case (QSysInfo::WV_NT) :
        return "Windows NT (operating system version 4.0)";
    case (QSysInfo::WV_2000) :
        return "Windows 2000 (operating system version 5.0)";
    case (QSysInfo::WV_XP) :
        return "Windows XP (operating system version 5.1)";
    case (QSysInfo::WV_2003) :
        return "Windows Server 2003, Windows Server 2003 R2, Windows Home Server, Windows XP Professional x64 Edition (operating system version 5.2)";
    case (QSysInfo::WV_VISTA) :
        return "Windows Vista, Windows Server 2008 (operating system version 6.0)";
    case (QSysInfo::WV_WINDOWS7) :
        return "Windows 7, Windows Server 2008 R2 (operating system version 6.1)";
    case (QSysInfo::WV_WINDOWS8) :
        return "Windows 8 (operating system version 6.2)";
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    case (QSysInfo::WV_WINDOWS8_1) :
        return "Windows 8.1 (operating system version 6.3)";
#endif
    default :
        break;
    }

    //CE
    switch (version) {
    case (QSysInfo::WV_CE) :
        return "Windows CE";
    case (QSysInfo::WV_CENET) :
        return "Windows CE .NET";
    case (QSysInfo::WV_CE_5) :
        return "Windows CE 5.x";
    case (QSysInfo::WV_CE_6) :
        return "Windows CE 6.x";
    default :
        break;
    }
#endif
#ifdef Q_OS_OSX
    QSysInfo::MacVersion version = QSysInfo::MacVersion();

    switch (version) {
    case (QSysInfo::MV_9) :
        return "Mac OS 9";
    case (QSysInfo::MV_10_0) :
        return "Mac OS X 10.0 Cheetah";
    case (QSysInfo::MV_10_1) :
        return "Mac OS X 10.1 Puma";
    case (QSysInfo::MV_10_2) :
        return "Mac OS X 10.2 Jaguar";
    case (QSysInfo::MV_10_3) :
        return "Mac OS X 10.3 Panther";
    case (QSysInfo::MV_10_4) :
        return "Mac OS X 10.4 Tiger";
    case (QSysInfo::MV_10_5) :
        return "Mac OS X 10.5 Leopard";
    case (QSysInfo::MV_10_6) :
        return "Mac OS X 10.6 SnowLeopard";
    case (QSysInfo::MV_10_7) :
        return "OS X 10.7 Lion";
    case (QSysInfo::MV_10_8) :
        return "OS X 10.8 MountainLion";
    case (QSysInfo::MV_10_9) :
        return "OS X 10.9 Mavericks";
    default :
        break;
    }
#endif
    return "An unknown platform";
}

int MainWindow::askForUploading() {
    QString file_name = qApp->arguments().at(1);
    Q_ASSERT(!file_name.isEmpty());
    if (QMessageBox::Yes == QMessageBox::question(this, tr("The Program Crashed"), tr("I regret to tell you that the program crashed just now. Fortunately, We have generated an error report successfully. \n The problem may be solved if you click \"Yes\" to upload the report to us. It may take you a few minutes."), QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        SmtpClient smtp("smtp.qq.com", 465, SmtpClient::SslConnection);

        smtp.setName("QSGSH");
        smtp.setUser("QSGSH@qq.com");
        smtp.setPassword("abcdefghijklmnopqrstuvwxyz"); //to be changed in the future, now there is some personal information in that QQ id

        if (!smtp.connectToHost()) {
            QMessageBox::warning(this, tr("Error!"), tr("Failed to connect to the server"), QMessageBox::Ok, QMessageBox::Ok);
            return 9528;
        }

        //get sender's name
        QString UserName;
#ifdef Q_OS_WIN32
        QSettings config("config.ini", QSettings::IniFormat);
        UserName = config.value("UserName", qgetenv("USERNAME")).toString();
#else
        QSettings config("QSanguosha.org", "QSanguosha");
        UserName = value("USERNAME", qgetenv("USER")).toString();
#endif
        if (UserName == "Admin" || UserName == "Administrator")
            UserName = "Sanguosha-fans";

        MimeMessage message;
        message.setSender(new EmailAddress("QSGSH@qq.com", UserName));
        message.addRecipient(new EmailAddress("QSGSH@qq.com", "QSanguosha-Hegemony Team"));
        message.setSubject("Crash Report 0.7.2");

        MimeText text;
        text.setText("Hi!\n This is a mail with an error report. Sender's system:" + getSystemVersion());
        message.addPart(&text);

        message.addPart(new MimeAttachment(new QFile("dmp/" + file_name)));

        if (!smtp.login() || !smtp.sendMail(message)) {
            return 9529;
        }
        smtp.quit();
        QMessageBox::information(this, tr("Congratulation"), tr("The report has been uploaded successfully. We will parse the report and try to fix the crash as soon as possible. Thank you very much. \nQSanguosha-Hegemony Team"), QMessageBox::Ok, QMessageBox::Ok);
    } else
        QMessageBox::information(this, tr("Hint"), tr("You can upload the report to a cloud disk and inform us later on. The file name is %1. You could find it in the folder \"dmp\". Thanks.  \nQSanguosha-Hegemony Team").arg(file_name), QMessageBox::Ok, QMessageBox::Ok);
    return 0;
}
