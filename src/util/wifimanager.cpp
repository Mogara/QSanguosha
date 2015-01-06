#include "wifimanager.h"

const char *WifiManager::SSID_PREFIX = "QSanguosha-";

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>

static QAndroidJniObject activity = QtAndroid::androidActivity();
static QAndroidJniObject manager = activity.callObjectMethod("getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;", QAndroidJniObject::fromString("wifi").object<jstring>());
#endif

WifiManager::WifiManager(const QString &deviceName)
    :mDeviceName(deviceName)
{
}

bool WifiManager::enableHotspot()
{
#ifdef Q_OS_ANDROID
    setWifiEnabled(false);

    QAndroidJniObject ssid = QAndroidJniObject::fromString(QString("%1%2").arg(SSID_PREFIX).arg(mDeviceName));
    QAndroidJniObject key = QAndroidJniObject::fromString("QSanguosha");

    QAndroidJniObject config("android/net/wifi/WifiConfiguration");
    config.setField<jstring>("SSID", ssid.object<jstring>());
    config.setField<jstring>("preSharedKey", key.object<jstring>());
    return manager.callMethod<jboolean>("setWifiApEnabled", "(Landroid/net/wifi/WifiConfiguration;Z)Z", config.object(), true);
#else
    return false;
#endif
}

bool WifiManager::disableHotspot()
{
#ifdef Q_OS_ANDROID
    return manager.callMethod<jboolean>("setWifiApEnabled", "(Landroid/net/wifi/WifiConfiguration;Z)Z", NULL, false);
#else
    return false;
#endif
}

bool WifiManager::isWifiEnabled() const
{
#ifdef Q_OS_ANDROID
    return manager.callMethod<jboolean>("isWifiEnabled");
#else
    return false;
#endif
}

bool WifiManager::setWifiEnabled(bool enabled)
{
#ifdef Q_OS_ANDROID
    return manager.callMethod<jboolean>("setWifiEnabled", "(Z)Z", enabled);
#else
    return false;
#endif
}

QStringList WifiManager::detect()
{
    if(!isWifiEnabled())
        setWifiEnabled(true);

    QStringList hotspots;
#ifdef Q_OS_ANDROID
    manager.callMethod<jboolean>("startScan");
    QAndroidJniObject results = manager.callObjectMethod("getScanResults", "()Ljava/util/List;");
    int length = results.callMethod<jint>("size");
    for (int i = 0; i < length; i++) {
        QAndroidJniObject result = results.callObjectMethod("get", "(I)Ljava/lang/Object;", i);
        QString ssid = result.getObjectField("SSID", "Ljava/lang/String;").toString();
        if (ssid.startsWith(SSID_PREFIX)) {
            ssid.remove(0, strlen(SSID_PREFIX));
            hotspots << ssid;
        }
    }
#endif
    return hotspots;
}

bool WifiManager::connect(const QString &ssid)
{
    (void) ssid;
    return false;
}
