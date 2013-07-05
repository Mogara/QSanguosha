#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QBrush>
#include <QDesktopWidget>

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
    bool Pause;
    bool ContestMode;
    bool RandomSeat;
    bool ForbidSIMC;
    bool DisableChat;
    bool Enable2ndGeneral;
    bool NoLordSkill;
    bool EnableReincarnation;
    bool EnableScene;	//changjing
    bool EnableSame;
    bool EnableEndless;
    bool EnableBasara;
    bool EnableHegemony;
    int MaxHpScheme;
    bool AnnounceIP;
    QString Address;
    bool FreeChooseGenerals;
    bool FreeChooseCards;
    bool FreeAssignSelf;
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

    bool CircularView;
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
    bool DisableLightbox;
    bool DisableLua;

    QString BackgroundBrush;

    // consts
    static const int S_MINI_MAX_COUNT;
    static const int S_STYLE_INDEX;
};

extern Settings Config;

#endif // SETTINGS_H
