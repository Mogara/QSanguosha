#include "settings.h"
#include "photo.h"
#include "card.h"

#include <QFontDatabase>
#include <QStringList>
#include <QFile>
#include <QMessageBox>
#include <QApplication>
#include <QNetworkInterface>
#include <QDateTime>

Settings Config;

static const qreal ViewWidth = 1280 * 0.8;
static const qreal ViewHeight = 800 * 0.8;

Settings::Settings()
    :QSettings("config.ini", QSettings::IniFormat),
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
    SmallFont.setPixelSize(27);
    TinyFont.setPixelSize(18);

    SmallFont.setWeight(QFont::Bold);

    AppFont = value("AppFont", QApplication::font("QMainWindow")).value<QFont>();
    UIFont = value("UIFont", QApplication::font("QTextEdit")).value<QFont>();
    TextEditColor = QColor(value("TextEditColor", "white").toString());

    CountDownSeconds = value("CountDownSeconds", 3).toInt();
    GameMode = value("GameMode", "02p").toString();
    BanPackages = value("BanPackages").toStringList();
    ContestMode = value("ContestMode", false).toBool();
    FreeChoose = value("FreeChoose", false).toBool();
    ForbidSIMC = value("ForbidSIMC", false).toBool();
    DisableChat = value("DisableChat", false).toBool();
    Enable2ndGeneral = value("Enable2ndGeneral", false).toBool();
    MaxHpScheme = value("MaxHpScheme", 0).toInt();
    AnnounceIP = value("AnnounceIP", false).toBool();
    Address = value("Address", QString()).toString();
    EnableAI = value("EnableAI", false).toBool();
    AIDelay = value("AIDelay", 1000).toInt();
    ServerPort = value("ServerPort", 9527u).toUInt();

    UserName = value("UserName", qgetenv("USERNAME")).toString();
    if(UserName == "Admin" || UserName == "Administrator")
        UserName = tr("Sanguosha-fans");
    ServerName = value("ServerName", tr("%1's server").arg(UserName)).toString();

    HostAddress = value("HostAddress", "127.0.0.1").toString();
    UserAvatar = value("UserAvatar", "zhangliao").toString();
    HistoryIPs = value("HistoryIPs").toStringList();
    DetectorPort = value("DetectorPort", 9526u).toUInt();
    MaxCards = value("MaxCards", 15).toInt();

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

    BackgroundBrush = value("BackgroundBrush", "backdrop/default.jpg").toString();
}
