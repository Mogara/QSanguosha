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
    :m_deviceName(deviceName)
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

    //@todo: Replace integer constants with constant identifiers
    config.setField<jint>("status", 2);
    QAndroidJniObject allowedGroupCiphers = config.getObjectField("allowedGroupCiphers", "Ljava/util/BitSet;");
    allowedGroupCiphers.callMethod<void>("set", "(I)V", 2);
    allowedGroupCiphers.callMethod<void>("set", "(I)V", 3);
    QAndroidJniObject allowedKeyManagement = config.getObjectField("allowedKeyManagement", "Ljava/util/BitSet;");
    allowedKeyManagement.callMethod<void>("set", "(I)V", 1);
    QAndroidJniObject allowedPairwiseCiphers = config.getObjectField("allowedPairwiseCiphers", "Ljava/util/BitSet;");
    allowedPairwiseCiphers.callMethod<void>("set", "(I)V", 1);
    allowedPairwiseCiphers.callMethod<void>("set", "(I)V", 2);
    QAndroidJniObject allowedProtocols = config.getObjectField("allowedProtocols", "Ljava/util/BitSet;");
    allowedProtocols.callMethod<void>("set", "(I)V", 1);

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
    Q_UNUSED(enabled)
    return false;
#endif
}

QStringList WifiManager::detectServer()
{
    if(!isWifiEnabled())
        setWifiEnabled(true);

    QStringList hotspots;
#ifdef Q_OS_ANDROID
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

QString WifiManager::currentServer() const
{
#ifdef Q_OS_ANDROID
    QAndroidJniObject currentConnection = manager.callObjectMethod("getConnectionInfo", "()Landroid/net/wifi/WifiInfo;");
    QString currentSsid = currentConnection.callObjectMethod("getSSID", "()Ljava/lang/String;").toString();
    if (currentSsid.startsWith(SSID_PREFIX))
        return currentSsid.mid(strlen(SSID_PREFIX));
#endif

    return QString();
}

bool WifiManager::connectToServer(const QString &server)
{
    QString currentSsid = currentServer();
    if (currentSsid == server)
        return true;

#ifdef Q_OS_ANDROID
    QAndroidJniObject ssid = QAndroidJniObject::fromString(QString("\"%1%2\"").arg(SSID_PREFIX).arg(server));
    QAndroidJniObject key = QAndroidJniObject::fromString("\"QSanguosha\"");

    QAndroidJniObject configs = manager.callObjectMethod("getConfiguredNetworks", "()Ljava/util/List;");
    int length = configs.callMethod<jint>("size");
    int networkId = -1;
    for (int i = 0; i < length; i++) {
        QAndroidJniObject config = configs.callObjectMethod("get", "(I)Ljava/lang/Object;", i);
        if (config.getObjectField("SSID", "Ljava/lang/String;").toString() == "TETSU") {
            networkId = config.getField<jint>("networkId");
            break;
        }
    }

    if (networkId == -1) {
        QAndroidJniObject config("android/net/wifi/WifiConfiguration");
        config.setField<jstring>("SSID", ssid.object<jstring>());
        config.setField<jstring>("preSharedKey", key.object<jstring>());

        //@todo: Replace integer constants with constant identifiers
        config.setField<jint>("status", 2);
        QAndroidJniObject allowedGroupCiphers = config.getObjectField("allowedGroupCiphers", "Ljava/util/BitSet;");
        allowedGroupCiphers.callMethod<void>("set", "(I)V", 2);
        allowedGroupCiphers.callMethod<void>("set", "(I)V", 3);
        QAndroidJniObject allowedKeyManagement = config.getObjectField("allowedKeyManagement", "Ljava/util/BitSet;");
        allowedKeyManagement.callMethod<void>("set", "(I)V", 1);
        QAndroidJniObject allowedPairwiseCiphers = config.getObjectField("allowedPairwiseCiphers", "Ljava/util/BitSet;");
        allowedPairwiseCiphers.callMethod<void>("set", "(I)V", 1);
        allowedPairwiseCiphers.callMethod<void>("set", "(I)V", 2);
        QAndroidJniObject allowedProtocols = config.getObjectField("allowedProtocols", "Ljava/util/BitSet;");
        allowedProtocols.callMethod<void>("set", "(I)V", 1);

        networkId = manager.callMethod<jint>("addNetwork", "(Landroid/net/wifi/WifiConfiguration;)I", config.object());
    }

    return manager.callMethod<jboolean>("enableNetwork", "(IZ)Z", networkId, true);
#else
    return false;
#endif
}
