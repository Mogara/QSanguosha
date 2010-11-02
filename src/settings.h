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
    explicit Settings(const QString &organization, const QString &application);
    void init();
    void changeBackground(const QString &new_bg);

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;
    QFont TinyFont;

    QFont AppFont;
    QFont UIFont;

    // server side
    QString ServerName;
    int CountDownSeconds;
    int PlayerCount;
    QStringList BanPackages;
    bool FreeChoose;
    bool ForbidSIMC;
    bool Enable2ndGeneral;
    int AILevel;
    int Protocol;
    QString Scenario;
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
    bool EnableBgMusic;
    float Volume;

    QBrush BackgroundBrush;
};

extern Settings Config;

#endif // SETTINGS_H
