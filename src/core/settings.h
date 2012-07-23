#ifndef SETTINGS_H
#define SETTINGS_H

#include "protocol.h"
#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QBrush>

class Settings : public QSettings{
    Q_OBJECT

public:
    explicit Settings();
    void init();

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;
    QFont TinyFont;

    QFont AppFont;
    QFont UIFont;
    QColor TextEditColor;

    // server side
    QString ServerName;
    int CountDownSeconds;
    QString GameMode;
    QStringList BanPackages;
    bool ContestMode;
    bool FreeChoose;
    bool ForbidSIMC;
    bool DisableChat;
    bool FreeAssignSelf;
    bool Enable2ndGeneral;
    bool EnableScene;	//changjing
    bool EnableSame;
    bool EnableBasara;
    bool EnableHegemony;
    int MaxHpScheme;
    bool AnnounceIP;
    QString Address;
    bool EnableAI;
    int AIDelay;
    ushort ServerPort;

    // client side
    QString HostAddress;
    QString UserName;
    QString UserAvatar;
    QString Password;
    QStringList HistoryIPs;
    ushort DetectorPort;
    int MaxCards;

    bool EnableHotKey;
    bool EnableMinimizeDialog;
    bool NeverNullifyMyTrick;
    bool EnableAutoTarget;
    int NullificationCountDown;
    int OperationTimeout;
    bool OperationNoLimit;
    bool EnableEffects;
    bool EnableLastWord;
    bool EnableBgMusic;
    float BGMVolume;
    float EffectVolume;

    QString BackgroundImage;

    // consts
    static const int S_CHOOSE_GENERAL_TIMEOUT;
    static const int S_GUANXING_TIMEOUT;
    static const int S_SURRNDER_REQUEST_MIN_INTERVAL;
    static const int S_PROGRESS_BAR_UPDATE_INTERVAL;
    static const int S_SERVER_TIMEOUT_GRACIOUS_PERIOD;
    static const int S_MOVE_CARD_ANIMATION_DURAION;
    static const int S_JUDGE_ANIMATION_DURATION;
    static const int S_REGULAR_ANIMATION_SLOW_DURAION;
    static const int S_JUDGE_RESULT_DELAY;
};

extern Settings Config;

#endif // SETTINGS_H
