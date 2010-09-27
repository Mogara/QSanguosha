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

    // server side
    int CountDownSeconds;
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
