#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <QStringList>

class WifiManager
{
public:
    WifiManager(const QString &serverName);

    bool enableHotspot();
    void disableHotspot();

    QStringList detect();
    bool connect(const QString &ssid);

private:
    QString mServerName;
};

#endif // WIFIMANAGER_H
