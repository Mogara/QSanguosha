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
    bool FreeChoose;
    bool ForbidSIMC;
    bool Enable2ndGeneral;
    int MaxHpScheme;
    bool AnnounceIP;
    QString Address;
    int AILevel;
    ushort ServerPort;

    // client side
    QString HostAddress;
    QString UserName;
    QString UserAvatar;
    QStringList HistoryIPs;
    ushort DetectorPort;

    bool FitInView;
    bool EnableHotKey;
    bool NeverNullifyMyTrick;
    bool EnableAutoTarget;
    int NullificationCountDown;
    int OperationTimeout;
    bool OperationNoLimit;
    bool EnableEffects;
    bool EnableLastWord;
    bool EnableBgMusic;
    float Volume;

    QString BackgroundBrush;
};

extern Settings Config;

#endif // SETTINGS_H
