#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QFont>
#include <MediaObject>
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
    Phonon::MediaSource ButtonHoverSource, ButtonDownSource;
    QScriptEngine *engine;
    QString UserName; 
    bool FitInView;
};

extern Settings Config;

#endif // SETTINGS_H
