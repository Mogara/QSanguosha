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
const int Settings::S_CHOOSE_GENERAL_TIMEOUT = 15;
const int Settings::S_GUANXING_TIMEOUT = 20;
const int Settings::S_SURRNDER_REQUEST_MIN_INTERVAL = 60;

Settings::Settings()

#ifdef Q_OS_WIN32
    :QSettings("config.ini", QSettings::IniFormat)
#else
    :QSettings("QSanguosha.com", "QSanguosha")
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
                << "sp" << "sp_cards" << "BGM" << "YJCM2012" << "Special3v3"
                << "joy" << "joy_equip" ;

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
    EnableBasara= value("EnableBasara",false).toBool();
    EnableHegemony = value("EnableHegemony",false).toBool();
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
    UserAvatar = value("UserAvatar", "zhangliao").toString();
    HistoryIPs = value("HistoryIPs").toStringList();
    DetectorPort = value("DetectorPort", 9526u).toUInt();
    MaxCards = value("MaxCards", 15).toInt();

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

    BackgroundBrush = value("BackgroundBrush", "backdrop/new-version.jpg").toString();

    QStringList roles_ban, kof_ban, basara_ban, hegemony_ban, pairs_ban;

    roles_ban << "zhugejin";

    kof_ban << "sunquan" << "huatuo" << "zhangliao" << "liubei" << "zhugejin";

    basara_ban << "dongzhuo" << "zuoci" << "shenzhugeliang" << "shenlvbu" << "zhanggongqi" << "zhugejin";

    hegemony_ban.append(basara_ban);
    hegemony_ban << "xiahoujuan" << "zhugejin";
    foreach(QString general, Sanguosha->getLimitedGeneralNames()){
        if(Sanguosha->getGeneral(general)->getKingdom() == "god" && !hegemony_ban.contains(general))
            hegemony_ban << general;
    }

    pairs_ban << "shencaocao" << "dongzhuo" << "zuoci" << "zhoutai" << "+luboyan" << "liaohua"
              << "caocao+caochong" << "xushu+zhugeliang" << "simayi+caizhaoji" << "wisjiangwei+zhanggongqi"
                << "zhenji+zhangjiao" << "zhenji+simayi" << "huanggai+yuanshao"
                << "huanggai+wuguotai" << "dengshizai+caoren" << "dengshizai+shenlvbu" << "dengshizai+bgm_diaochan"
                << "luxun+liubei" << "luxun+wolong" << "luxun+yuji" << "luxun+daqiao"
                << "huangyueying+wolong" << "huangyueying+yuanshao" << "huangyueying+ganning"
                << "shuangxiong+sunce" << "shuangxiong+huanggai" << "shuangxiong+huangyueying"
                << "dengai+guojia" << "dengai+simayi" << "dengai+zhangjiao"
                << "dengai+shenzhugeliang" << "dengai+shensimayi"
                << "jiangboyue+huangyueying" << "jiangboyue+wolong" << "jiangboyue+yuanshao"
                << "jiangboyue+shuangxiong" << "jiangboyue+ganning" << "jiangboyue+luxun" << "jiangboyue+zhanggongqi"
                << "weiyan+huanggai" << "caoren+shenlvbu" << "bgm_pangtong+huanggai"
                << "fazheng+xiahoudun" << "luxun+zhanggongqi" << "sunquan+lingtong"
                << "sunquan+sunshangxiang" << "wuguotai+guojia" << "wuguotai+xunyu"
                << "caizhaoji+caoren" << "caizhaoji+dengshizai" << "yuanshu+zhanghe" << "caizhaoji+caozhi" << "caizhaoji+shenlvbu"
                << "yuanshu+lvmeng" << "yuanshu+caochong" << "huatuo+guojia"
                << "huatuo+xunyu" << "huatuo+xiahoujuan" << "huatuo+zhanggongqi"
                << "lukang+liubei" << "lukang+wolong" << "lukang+yuji" << "jiangboyue+lukang"
                << "lukang+zhanggongqi" << "bgm_diaochan+caoren" << "bgm_diaochan+shenlvbu"
                << "bgm_diaochan+caizhaoji" << "caozhi+shenlvbu" << "caoren+caozhi"
                << "guanxingzhangbao+luxun" << "guanxingzhangbao+sunce" << "bgm_caoren+caoren"
                << "bgm_caoren+caozhi" << "bgm_caoren+shenlvbu" << "bgm_caoren+bgm_diaochan"
                << "bgm_caoren+dengshizai" << "bgm_caoren+caizhaoji" << "bgm_pangtong+huanggai"
                << "huanggai+guanxingzhangbao";

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

    QStringList forbid_packages;
    forbid_packages << "Special3v3";
    setValue("ForbidPackages", forbid_packages.join("+"));

//ui
    setValue("UI/ExpandDashboard", value("UI/ExpandDashboard", true).toBool());
}
