#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QBrush>

class Settings : public QSettings
{
public:
    explicit Settings(const QString &organization, const QString &application);
    void init();

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;
    QFont TinyFont;

    // server side
    QString ListenAddress;
    int CountDownSeconds;
    int LordCount;
    int PlayerCount;

    // client side
    QString HostAddress;
    ushort Port;
    QString UserName;
    QString UserAvatar;

    bool FitInView;
    bool EnableHotKey;
    bool NeverNullifyMyTrick;
    bool EnableAutoTarget;

    QBrush BackgroundBrush;
};

extern Settings Config;

#endif // SETTINGS_H
