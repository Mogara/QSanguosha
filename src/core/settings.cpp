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
    if(!qApp->arguments().contains("-server")){
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
    }

    CountDownSeconds = value("CountDownSeconds", 3).toInt();
    GameMode = value("GameMode", "02p").toString();


    if(!contains("BanPackages")){
        QStringList banlist;
        banlist << "nostalgia" << "yitian" << "wisdom" << "test"
                << "disaster" << "god" << "YJCM" << "yitian_cards"
                << "sp" << "sp_cards"
                << "joy" << "joy_equip";

        setValue("BanPackages", banlist);
    }

    BanPackages = value("BanPackages").toStringList();

    ContestMode = value("ContestMode", false).toBool();
    FreeChoose = value("FreeChoose", false).toBool();
    ForbidSIMC = value("ForbidSIMC", false).toBool();
    DisableChat = value("DisableChat", false).toBool();
    Enable2ndGeneral = value("Enable2ndGeneral", false).toBool();
    EnableScene = value("EnableScene", false).toBool();	//changjing
    MaxHpScheme = value("MaxHpScheme", 0).toInt();
    AnnounceIP = value("AnnounceIP", false).toBool();
    Address = value("Address", QString()).toString();
    EnableAI = value("EnableAI", false).toBool();
    AIDelay = value("AIDelay", 1000).toInt();
    ServerPort = value("ServerPort", 9527u).toUInt();

#ifdef Q_OS_WIN32
    UserName = value("UserName", qgetenv("USERNAME")).toString();
#else
    UserName = value("USERNAME", qgetenv("USER")).toString();
#endif

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

    BackgroundBrush = value("BackgroundBrush", "backdrop/mid-autumn.jpg").toString();

    if(!contains("1v1/Banlist")){
        QStringList banlist;
        banlist << "sunquan" << "huatuo" << "zhangliao" << "liubei";
        setValue("1v1/Banlist", banlist);
    }

    if(!contains("style/dock"))
    {
        setValue("style/button"," QPushButton {\r\n\tfont : 12px;\r\n     border: 1px solid gray;\r\n     background-image: url(image/system/button/back.png);\r\n\t color: yellow;\r\n\t padding: 6px 12px 6px 12px;\r\n }\r\n QPushButton:disabled {\r\n\tfont : 12px;\r\n     background-image: url(image/system/button/back_shade.png);\r\n\t color: white;\r\n }\r\n QCheckBox {\r\n\tfont : 12px;\r\n     border: 1px solid gray;\r\n\t color: yellow;\r\n\t padding: 6px 12px 6px 8px;\r\n }\r\n\r\n QCheckBox::indicator {\r\n     width: 0px;\r\n     height: 0px;\r\n }\r\n\r\n QCheckBox:unchecked {\r\n     background-image: url(image/system/button/frequent_checkbox/unchecked.png);\r\n }\r\n\r\n QCheckBox:unchecked:hover {\r\n     background-image: url(image/system/button/frequent_checkbox/unchecked_hover.png);\r\n }\r\n\r\n QCheckBox:unchecked:pressed {\r\n     background-image: url(image/system/button/frequent_checkbox/unchecked_press.png);\r\n }\r\n\r\n QCheckBox:checked {\r\n     background-image: url(image/system/button/frequent_checkbox/checked.png);\r\n }\r\n\r\n QCheckBox:checked:hover {\r\n     background-image: url(image/system/button/frequent_checkbox/checked_hover.png);\r\n }\r\n\r\n QCheckBox:checked:pressed {\r\n     background-image: url(image/system/button/frequent_checkbox/checked_press.png);\r\n }\r\n \r\n  QComboBox {\r\n\tbackground-image: url(image/system/button/back2.png);\r\n\tcolor : white;\r\n     border: 1px solid gray;\r\n     border-radius: 3px;\r\n     padding: 1px 18px 1px 3px;\r\n }\r\n\r\n QComboBox:on { /* shift the text when the popup opens */\r\n     padding-top: 3px;\r\n     padding-left: 4px;\r\n }\r\n\r\n QComboBox::drop-down {\r\n     subcontrol-origin: padding;\r\n     subcontrol-position: top right;\r\n     width: 15px;\r\n\r\n     border-left-width: 1px;\r\n     border-left-color: darkgray;\r\n     border-left-style: solid; /* just a single line */\r\n     border-top-right-radius: 3px; /* same radius as the QComboBox */\r\n     border-bottom-right-radius: 3px;\r\n }\r\n\r\n\r\n QComboBox::down-arrow:on { /* shift the arrow when popup is open */\r\n     top: 1px;\r\n     left: 1px;\r\n }\r\n");
        setValue("style/dock","\r\n     background-image: url(image/system/skill-dock.png);\r\n ");
    }
}
