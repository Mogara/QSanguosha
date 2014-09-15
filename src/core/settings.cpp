/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

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

Settings *SettingsInstance = NULL;

static const qreal ViewWidth = 1280 * 0.8;
static const qreal ViewHeight = 800 * 0.8;

//consts
const int Settings::S_SURRENDER_REQUEST_MIN_INTERVAL = 5000;
const int Settings::S_PROGRESS_BAR_UPDATE_INTERVAL = 200;
const int Settings::S_SERVER_TIMEOUT_GRACIOUS_PERIOD = 1000;
const int Settings::S_MOVE_CARD_ANIMATION_DURATION = 600;
const int Settings::S_JUDGE_ANIMATION_DURATION = 1200;
const int Settings::S_JUDGE_LONG_DELAY = 800;

Settings::Settings()
#ifdef Q_OS_WIN32
    : QSettings("config.ini", QSettings::IniFormat),
#else
    : QSettings("QSanguosha.org", "QSanguosha"),
#endif
    Rect(-ViewWidth / 2, -ViewHeight / 2, ViewWidth, ViewHeight)
{
    Q_ASSERT(SettingsInstance == NULL);
    SettingsInstance = this;
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));
}

void Settings::init() {
    if (!qApp->arguments().contains("-server")) {
        QString font_path = value("DefaultFontPath", "font/simli.ttf").toString();
        int font_id = QFontDatabase::addApplicationFont(font_path);
        if (font_id != -1) {
            QString font_family = QFontDatabase::applicationFontFamilies(font_id).first();
            BigFont.setFamily(font_family);
            SmallFont.setFamily(font_family);
            TinyFont.setFamily(font_family);
        } else {
            QMessageBox::warning(NULL, tr("Warning"), tr("Font file %1 could not be loaded!").arg(font_path));
        }

        BigFont.setPixelSize(56);
        SmallFont.setPixelSize(27);
        TinyFont.setPixelSize(18);

        SmallFont.setWeight(QFont::Bold);

        AppFont = value("AppFont", QApplication::font("QMainWindow")).value<QFont>();
        UIFont = value("UIFont", QApplication::font("QTextEdit")).value<QFont>();
        TextEditColor = QColor(value("TextEditColor", "white").toString());
        SkillDescriptionInToolTipColor = value("SkillDescriptionInToolTipColor", "#FFFF33").toString();
        SkillDescriptionInOverviewColor = value("SkillDescriptionInOverviewColor", "#FF0080").toString();
        ToolTipBackgroundColor = value("ToolTipBackgroundColor", "#000000").toString();
    }

    CountDownSeconds = value("CountDownSeconds", 3).toInt();
    GameMode = value("GameMode", "08p").toString();

    BanPackages = value("BanPackages", "Test").toStringList();
    RandomSeat = value("RandomSeat", true).toBool();
    EnableCheat = value("EnableCheat", false).toBool();
    FreeChoose = EnableCheat && value("FreeChoose", false).toBool();
    ForbidSIMC = value("ForbidSIMC", false).toBool();
    DisableChat = value("DisableChat", false).toBool();
    Address = value("Address", QString()).toString();
    ForbidAddingRobot = value("ForbidAddingRobot", false).toBool();
    OriginAIDelay = value("OriginAIDelay", 1000).toInt();
    AlterAIDelayAD = value("AlterAIDelayAD", false).toBool();
    AIDelayAD = value("AIDelayAD", 0).toInt();
    SurrenderAtDeath = value("SurrenderAtDeath", false).toBool();
    LuckCardLimitation = value("LuckCardLimitation", 0).toInt();
    ServerPort = value("ServerPort", 9527u).toUInt();
    DisableLua = value("DisableLua", false).toBool();
    RewardTheFirstShowingPlayer = value("RewardTheFirstShowingPlayer", false).toBool();

#ifdef Q_OS_WIN32
    UserName = value("UserName", qgetenv("USERNAME")).toString();
#else
    UserName = value("UserName", qgetenv("USER")).toString();
#endif

    if (UserName == "root" || UserName == "Administrator" || UserName.isEmpty())
        UserName = tr("Sanguosha-fans");
    ServerName = value("ServerName", tr("%1's server").arg(UserName)).toString();

    HostAddress = value("HostAddress", "127.0.0.1").toString();
    //Set Cao Cao as default avatar to pay tribute to Moligaloo, the founder of QSanguosha.
    UserAvatar = value("UserAvatar", "caocao").toString();
    HistoryIPs = value("HistoryIPs").toStringList();
    DetectorPort = value("DetectorPort", 9526u).toUInt();
    MaxCards = value("MaxCards", 15).toInt();

    EnableHotKey = value("EnableHotKey", true).toBool();
    NeverNullifyMyTrick = value("NeverNullifyMyTrick", false).toBool(); // disabled by default because of the new diaochan
    EnableMinimizeDialog = value("EnableMinimizeDialog", false).toBool();
    EnableAutoTarget = value("EnableAutoTarget", true).toBool();
    EnableIntellectualSelection = value("EnableIntellectualSelection", true).toBool();
    EnableSuperDrag = value("EnableSuperDrag", false).toBool(); // set it to true?
    EnableDoubleClick = value("EnableDoubleClick", false).toBool();
    NullificationCountDown = value("NullificationCountDown", 8).toInt();
    OperationTimeout = value("OperationTimeout", 15).toInt();
    OperationNoLimit = value("OperationNoLimit", false).toBool();
    EnableEffects = value("EnableEffects", true).toBool();
    EnableLastWord = value("EnableLastWord", true).toBool();
    EnableBgMusic = value("EnableBgMusic", true).toBool();
    BGMVolume = value("BGMVolume", 1.0f).toFloat();
    EffectVolume = value("EffectVolume", 1.0f).toFloat();

    BackgroundImage = value("BackgroundImage", "image/backdrop/new-version.jpg").toString();
    TableBgImage = value("TableBgImage", "image/backdrop/default.jpg").toString();

    EnableAutoSaveRecord = value("EnableAutoSaveRecord", false).toBool();
    NetworkOnly = value("NetworkOnly", false).toBool();
    RecordSavePaths = value("RecordSavePaths", "records/").toString();

    EnableAutoPreshowInConsoleMode = value("EnableAutoPreshowInConsoleMode", false).toBool();

    BubbleChatBoxKeepSeconds = value("BubbleChatBoxKeepSeconds", 2).toInt();

    IgnoreOthersSwitchesOfSkin = value("IgnoreOthersSwitchesOfSkin", false).toBool();

    lua_State *lua = Sanguosha->getLuaState();
    Config.ExtraHiddenGenerals = GetConfigFromLuaState(lua, "extra_hidden_generals").toStringList();
    Config.RemovedHiddenGenerals = GetConfigFromLuaState(lua, "removed_hidden_generals").toStringList();

    QStringList forbid_packages = value("ForbidPackages").toStringList();
    if (forbid_packages.isEmpty()) {
        forbid_packages << "test" << "jiange-defense";
        setValue("ForbidPackages", forbid_packages);
    }
}
