#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <QStringList>

class WifiManager
{
public:
    WifiManager(const QString &deviceName);

    bool enableHotspot();
    bool disableHotspot();

    bool isWifiEnabled() const;
    bool setWifiEnabled(bool enabled);

    QStringList detect();
    bool connect(const QString &ssid);

private:
    QString mDeviceName;
    static const char *SSID_PREFIX;
};

#endif // WIFIMANAGER_H
