#ifndef SETTINGS_H
#define SETTINGS_H

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

    bool FitInView;
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
    bool DisableLua;

    QString BackgroundBrush;

    // consts
    static const int S_MINI_MAX_COUNT;
};

extern Settings Config;

#endif // SETTINGS_H
