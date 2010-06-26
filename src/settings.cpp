#include "settings.h"
#include "photo.h"
#include "card.h"

#include <QFontDatabase>
#include <QStringList>
#include <QFile>

Settings Config("Donghua University", "Sanguosha");

static const qreal ViewWidth = 1280 * 0.8;
static const qreal ViewHeight = 800 * 0.8;

Settings::Settings(const QString &organization, const QString &application) :
    QSettings(organization, application),
    Rect(-ViewWidth/2, -ViewHeight/2, ViewWidth, ViewHeight)
{
}


Settings::~Settings()
{
}

void Settings::init(){
    QString font_path = value("DefaultFontPath", "font/girl.ttf").toString();
    int font_id = QFontDatabase::addApplicationFont(font_path);
    if(font_id!=-1){
        QString font_family = QFontDatabase::applicationFontFamilies(font_id).front();
        BigFont.setFamily(font_family);
        SmallFont.setFamily(font_family);
    }

    BigFont.setPixelSize(64);
    SmallFont.setPixelSize(32);

    UserName = value("UserName", getenv("USERNAME")).toString();
    HostAddress = value("HostAddress", "127.0.0.1").toString();
    Port = value("Port", 9527u).toUInt();

    FitInView = value("FitInView", false).toBool();
    UseOpenGL = value("UseOpenGL", false).toBool();

    QString bgbrush = value("BackgroundBrush", ":/images/rock.jpg").toString();
    BackgroundBrush = QBrush(QPixmap(bgbrush));
}
