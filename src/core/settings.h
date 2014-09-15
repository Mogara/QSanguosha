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

#ifndef _SETTINGS_H
#define _SETTINGS_H

#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QBrush>

class Settings : public QSettings {
    Q_OBJECT

public:
    explicit Settings();
    //************************************
    // Method:    init
    // FullName:  Settings::init
    // Access:    public
    // Returns:   void
    // Qualifier:
    // Description: Initialize Config and create a user setting file to save user's settings.
    //
    // Last Updated By Yanguam Siliagim
    // To use a proper way to convert generals and cards
    //
    // QSanguosha-Rara
    // March 17 2014
    //************************************
    void init();

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;
    QFont TinyFont;

    QFont AppFont;
    QFont UIFont;
    QColor TextEditColor;
    QColor SkillDescriptionInToolTipColor;
    QColor SkillDescriptionInOverviewColor;
    QColor ToolTipBackgroundColor;

    // server side
    QString ServerName;
    int CountDownSeconds;
    int NullificationCountDown;
    bool EnableMinimizeDialog;
    QString GameMode;
    QStringList BanPackages;
    bool RandomSeat;
    bool EnableCheat;
    bool FreeChoose;
    bool ForbidSIMC;
    bool DisableChat;
    QString Address;
    bool ForbidAddingRobot;
    int AIDelay;
    int OriginAIDelay;
    bool AlterAIDelayAD;
    int AIDelayAD;
    bool SurrenderAtDeath;
    int LuckCardLimitation;
    ushort ServerPort;
    bool DisableLua;

    QStringList ExtraHiddenGenerals;
    QStringList RemovedHiddenGenerals;

    bool RewardTheFirstShowingPlayer;

    // client side
    QString HostAddress;
    QString UserName;
    QString UserAvatar;
    QStringList HistoryIPs;
    ushort DetectorPort;
    int MaxCards;

    bool EnableHotKey;
    bool NeverNullifyMyTrick;
    bool EnableAutoTarget;
    bool EnableIntellectualSelection;
    bool EnableSuperDrag;
    bool EnableDoubleClick;
    bool EnableAutoSaveRecord;
    bool NetworkOnly;
    bool EnableAutoPreshowInConsoleMode;
    int OperationTimeout;
    bool OperationNoLimit;
    bool EnableEffects;
    bool EnableLastWord;
    bool EnableBgMusic;
    float BGMVolume;
    float EffectVolume;

    QString BackgroundImage;
    QString TableBgImage;
    QString RecordSavePaths;

    int BubbleChatBoxKeepSeconds;
    bool IgnoreOthersSwitchesOfSkin;

    // consts
    static const int S_SURRENDER_REQUEST_MIN_INTERVAL;
    static const int S_PROGRESS_BAR_UPDATE_INTERVAL;
    static const int S_SERVER_TIMEOUT_GRACIOUS_PERIOD;
    static const int S_MOVE_CARD_ANIMATION_DURATION;
    static const int S_JUDGE_ANIMATION_DURATION;
    static const int S_JUDGE_LONG_DELAY;
};

extern Settings *SettingsInstance;
#define Config (*SettingsInstance)

#endif

