#include "settings.h"
#include "photo.h"
#include "card.h"

#include <QFontDatabase>
#include <QStringList>
#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QCryptographicHash>
#include <QNetworkInterface>
#include <QDateTime>

Settings Config("Donghua University", "Sanguosha");

static const qreal ViewWidth = 1280 * 0.8;
static const qreal ViewHeight = 800 * 0.8;

Settings::Settings(const QString &organization, const QString &application) :
    QSettings(organization, application),
    Rect(-ViewWidth/2, -ViewHeight/2, ViewWidth, ViewHeight)
{
}

void Settings::init(){
    QString font_path = value("DefaultFontPath", "font/girl.ttf").toString();
    int font_id = QFontDatabase::addApplicationFont(font_path);
    if(font_id!=-1){
        QString font_family = QFontDatabase::applicationFontFamilies(font_id).first();
        BigFont.setFamily(font_family);
        SmallFont.setFamily(font_family);
        TinyFont.setFamily(font_family);
    }else
        QMessageBox::warning(NULL, tr("Warning"), tr("Font file %1 could not be loaded!").arg(font_path));

    BigFont.setPixelSize(56);
    SmallFont.setPixelSize(32);
    TinyFont.setPixelSize(18);

    AppFont = value("AppFont", QApplication::font("QMainWindow")).value<QFont>();
    UIFont = value("UIFont", QApplication::font("QTextEdit")).value<QFont>();

    CountDownSeconds = value("CountDownSeconds", 3).toInt();
    GameMode = value("GameMode", "02p").toString();
    BanPackages = value("BanPackages").toStringList();
    FreeChoose = value("FreeChoose", false).toBool();
    ForbidSIMC = value("ForbidSIMC", false).toBool();
    Enable2ndGeneral = value("Enable2ndGeneral", false).toBool();
    MaxHpScheme = value("MaxHpScheme", 0).toInt();
    AnnounceIP = value("AnnounceIP", false).toBool();
    Address = value("Address", QString()).toString();
    AILevel = value("AILevel", 2).toInt();
    ServerPort = value("ServerPort", 9527u).toUInt();

    QString default_host;

#ifndef QT_NO_DEBUG
    default_host = "localhost";
#else
    default_host = "irc.freenode.net";
#endif

    IrcHost = value("IrcHost", default_host).toString();
    IrcPort = value("IrcPort", 6667).toUInt();
    IrcNick = value("IrcNick").toString();
    if(IrcNick.isEmpty()){
        QString mac = QNetworkInterface::allInterfaces().first().hardwareAddress();
        QDateTime now = QDateTime::currentDateTime();
        QString combined = mac + now.toString();
        QCryptographicHash::Algorithm algorithm;
        if(now.toTime_t() % 2 == 0)
            algorithm = QCryptographicHash::Md5;
        else
            algorithm = QCryptographicHash::Sha1;

        QString hash = QCryptographicHash::hash(combined.toAscii(), algorithm).toHex().toUpper().right(10);
        IrcNick = QString("SGS_%1").arg(hash);
    }
    IrcChannel = value("IrcChannel", "#sanguosha").toString();

    UserName = value("UserName", getenv("USERNAME")).toString();
    if(UserName == "Admin" || UserName == "Administrator")
        UserName = tr("Sanguosha-fans");
    ServerName = value("ServerName", tr("%1's server").arg(UserName)).toString();

    HostAddress = value("HostAddress", "127.0.0.1").toString();
    UserAvatar = value("UserAvatar", "zhangliao").toString();
    HistoryIPs = value("HistoryIPs").toStringList();
    DetectorPort = value("DetectorPort", 9526u).toUInt();

    FitInView = value("FitInView", false).toBool();
    EnableHotKey = value("EnableHotKey", true).toBool();
    NeverNullifyMyTrick = value("NeverNullifyMyTrick", true).toBool();
    EnableAutoTarget = value("EnableAutoTarget", false).toBool();
    NullificationCountDown = value("NullificationCountDown", 8).toInt();
    OperationTimeout = value("OperationTimeout", 15).toInt();
    OperationNoLimit = value("OperationNoLimit", false).toBool();
    EnableEffects = value("EnableEffects", true).toBool();
    EnableLastWord = value("EnableLastWord", true).toBool();
    EnableBgMusic = value("EnableBgMusic", true).toBool();
    Volume = value("Volume", 1.0f).toFloat();

    QString bg_path = value("BackgroundBrush", ":/background.png").toString();
    changeBackground(bg_path);
}

void Settings::changeBackground(const QString &new_bg){
    QPixmap bgbrush(new_bg);
    BackgroundBrush = QBrush(bgbrush);
    QTransform transform;
    transform.translate(Rect.x(), Rect.y());
    BackgroundBrush.setTransform(transform);

    setValue("BackgroundBrush", new_bg);
}
