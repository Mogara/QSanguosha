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

    QStringList detectServer();
    bool connectToServer(const QString &server);
    QString currentServer() const;

private:
    QString m_deviceName;
    static const char *SSID_PREFIX;
};

#endif // WIFIMANAGER_H
