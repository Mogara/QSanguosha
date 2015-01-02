#include "wifimanager.h"

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>

static QAndroidJniObject activity = QtAndroid::androidActivity();
static QAndroidJniObject manager = activity.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", QAndroidJniObject::fromString("wifi").object<jstring>());
#endif

WifiManager::WifiManager(const QString &serverName)
    :mServerName(serverName)
{
}

bool WifiManager::enableHotspot()
{
#ifdef Q_OS_ANDROID
    manager.callMethod<jboolean>("setWifiEnabled", "(Z)Z", false);

    QAndroidJniObject ssid = QAndroidJniObject::fromString(QString("QSanguoshaServer-%1").arg(mServerName));
    QAndroidJniObject key = QAndroidJniObject::fromString("QSanguosha");

    QAndroidJniObject config("android/net/wifi/WifiConfiguration");
    config.setField<jstring>("SSID", ssid.object<jstring>());
    config.setField<jstring>("preSharedKey", key.object<jstring>());
    return manager.callMethod<jboolean>("setWifiApEnabled", "(Landroid/net/wifi/WifiConfiguration;Z)Z", config.object(), true);
#else
    return false;
#endif
}

void WifiManager::disableHotspot()
{
}

QStringList WifiManager::detect()
{
    return QStringList();
}

bool WifiManager::connect(const QString &ssid)
{
    (void) ssid;
    return false;
}
