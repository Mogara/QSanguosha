#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QScriptEngine>

class Settings : public QSettings
{
Q_OBJECT
public:
    explicit Settings(const QString &organization, const QString &application);
    ~Settings();
    void init();

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;
    QScriptEngine *engine;
    QString UserName; 
    uint Port;
    bool FitInView;
    bool UseOpenGL;
};

extern Settings Config;

#endif // SETTINGS_H
