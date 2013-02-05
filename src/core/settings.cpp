#include "settings.h"
#include "photo.h"
#include "card.h"
#include "engine.h"

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

//consts
const int Settings::S_MINI_MAX_COUNT = 33;

Settings::Settings()

#ifdef Q_OS_WIN32
    :QSettings("config.ini", QSettings::IniFormat)
#else
    :QSettings("QSanguosha.org", "QSanguosha")
#endif

     ,Rect(-ViewWidth/2, -ViewHeight/2, ViewWidth, ViewHeight)
{
}

void Settings::init(){
    if(!qApp->arguments().contains("-server")){
        QString font_path = value("DefaultFontPath", "font/font.ttf").toString();
        int font_id = QFontDatabase::addApplicationFont(font_path);
        if(font_id!=-1){
            QString font_family = QFontDatabase::applicationFontFamilies(font_id).first();
            BigFont.setFamily(font_family);
            SmallFont.setFamily(font_family);
            TinyFont.setFamily(font_family);
        }
        //else
        //    QMessageBox::warning(NULL, tr("Warning"), tr("Font file %1 could not be loaded!").arg(font_path));

        BigFont.setPixelSize(56);
        SmallFont.setPixelSize(27);
        TinyFont.setPixelSize(18);

        SmallFont.setWeight(QFont::Bold);

        AppFont = value("AppFont", QApplication::font("QMainWindow")).value<QFont>();
        UIFont = value("UIFont", QApplication::font("QTextEdit")).value<QFont>();
        TextEditColor = QColor(value("TextEditColor", "white").toString());
    }

    CountDownSeconds = value("CountDownSeconds", 3).toInt();
    GameMode = value("GameMode", "08p").toString();


    if(!contains("BanPackages")){
        QStringList banlist;
        banlist << "nostalgia" << "nostal_general" << "yitian" << "wisdom" << "sand_zhimeng"
                << "disaster" << "god" << "YJCM" << "yitian_cards" << "test"
                << "sp" << "sp_cards" << "BGM" << "YJCM2012" << "Special3v3"
                << "New3v3Card" << "joy" << "joy_equip" << "hegemony_card" 
                << "hegemony" << "ling" << "assassins" << "olympics" << "paster"
                << "changban_slope";

        setValue("BanPackages", banlist);
    }
    BanPackages = value("BanPackages").toStringList();

    ContestMode = value("ContestMode", false).toBool();
    FreeChoose = value("FreeChoose", false).toBool();
    ForbidSIMC = value("ForbidSIMC", false).toBool();
    DisableChat = value("DisableChat", false).toBool();
    FreeAssignSelf = value("FreeAssignSelf", false).toBool();
    Enable2ndGeneral = value("Enable2ndGeneral", false).toBool();
    EnableScene = value("EnableScene", false).toBool();	//changjing
    EnableSame = value("EnableSame", false).toBool();
    EnableBasara = value("EnableBasara", false).toBool();
    EnableHegemony = value("EnableHegemony", false).toBool();
    MaxHpScheme = value("MaxHpScheme", 0).toInt();
    AnnounceIP = value("AnnounceIP", false).toBool();
    Address = value("Address", QString()).toString();
    EnableAI = value("EnableAI", true).toBool();
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
    UserAvatar = value("UserAvatar", "caiwenji").toString();
    HistoryIPs = value("HistoryIPs").toStringList();
    DetectorPort = value("DetectorPort", 9526u).toUInt();
    MaxCards = value("MaxCards", 15).toInt();

    BackgroundBrush = value("BackgroundBrush", "backdrop/default.jpg").toString();
    CircularView = value("CircularView", QApplication::desktop()->width() < 1030 ? false: true).toBool();
    FitInView = value("FitInView", false).toBool();
    EnableHotKey = value("EnableHotKey", true).toBool();
    NeverNullifyMyTrick = value("NeverNullifyMyTrick", true).toBool();
    EnableMinimizeDialog = value("EnableMinimizeDialog", false).toBool();
    EnableAutoTarget = value("EnableAutoTarget", false).toBool();
    NullificationCountDown = value("NullificationCountDown", 8).toInt();
    OperationTimeout = value("OperationTimeout", 15).toInt();
    OperationNoLimit = value("OperationNoLimit", false).toBool();
    EnableEffects = value("EnableEffects", true).toBool();
    EnableLastWord = value("EnableLastWord", true).toBool();
    EnableBgMusic = value("EnableBgMusic", true).toBool();
    BGMVolume = value("BGMVolume", 1.0f).toFloat();
    EffectVolume = value("EffectVolume", 1.0f).toFloat();
    DisableLua = value("DisableLua", false).toBool();

//banlist
    QStringList roles_ban, kof_ban, basara_ban, hegemony_ban, pairs_ban, couple_ban;

    lua_State *lua = Sanguosha->getLuaState();
    roles_ban = GetConfigFromLuaState(lua, "roles_ban", "ban_list").toStringList();
    kof_ban = GetConfigFromLuaState(lua, "kof_ban", "ban_list").toStringList();
    basara_ban = GetConfigFromLuaState(lua, "basara_ban", "ban_list").toStringList();
    hegemony_ban = GetConfigFromLuaState(lua, "hegemony_ban", "ban_list").toStringList();
    couple_ban = GetConfigFromLuaState(lua, "couple_ban", "ban_list").toStringList();
    foreach(QString general, Sanguosha->getLimitedGeneralNames())
        if(Sanguosha->getGeneral(general)->getKingdom() == "god" && !hegemony_ban.contains(general))
            hegemony_ban << general;

    pairs_ban = GetConfigFromLuaState(lua, "pairs_ban", "ban_list").toStringList();

    QStringList banlist = value("Banlist/Roles").toStringList();
    foreach(QString ban_general, roles_ban){
        if(!banlist.contains(ban_general))
            banlist << ban_general;
    }
    setValue("Banlist/Roles", banlist);

    banlist = value("Banlist/1v1").toStringList();
    foreach(QString ban_general, kof_ban){
        if(!banlist.contains(ban_general))
            banlist << ban_general;
    }
    setValue("Banlist/1v1", banlist);

    banlist = value("Banlist/Basara").toStringList();
    foreach(QString ban_general, basara_ban){
        if(!banlist.contains(ban_general))
            banlist << ban_general;
    }
    setValue("Banlist/Basara", banlist);

    banlist = value("Banlist/Hegemony").toStringList();
    foreach(QString ban_general, hegemony_ban){
        if(!banlist.contains(ban_general))
            banlist << ban_general;
    }
    setValue("Banlist/Hegemony", banlist);

    banlist = value("Banlist/Pairs").toStringList();
    foreach(QString ban_general, pairs_ban){
        if(!banlist.contains(ban_general))
            banlist << ban_general;
    }
    setValue("Banlist/Pairs", banlist);

    banlist = value("Banlist/Couple").toStringList();
    foreach(QString ban_general, couple_ban){
        if(!banlist.contains(ban_general))
            banlist << ban_general;
    }
    setValue("Banlist/Couple", banlist);

    QStringList forbid_packages = GetConfigFromLuaState(lua, "forbid_packages", "ban_list").toStringList();
    setValue("ForbidPackages", forbid_packages.join("+"));
}
