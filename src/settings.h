#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QScriptEngine>
#include <QBrush>

class Settings : public QSettings
{
Q_OBJECT
public:
    explicit Settings(const QString &organization, const QString &application);
    void init();

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;

    QString UserName;
    QString HostAddress;
    ushort Port;
    QString UserAvatar;

    bool FitInView;
    bool UseOpenGL;

    QBrush BackgroundBrush;    
};

extern Settings Config;

#endif // SETTINGS_H
