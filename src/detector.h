#ifndef DETECTOR_H
#define DETECTOR_H

#include <QObject>
#include <QString>
#include <QUdpSocket>
#include <QThread>

#include "libircclient.h"
#include "clientstruct.h"

class Detector : public QObject
{
    Q_OBJECT

public slots:
    virtual void detect() = 0;
    virtual void stop() = 0;

signals:
    void detected(const QString &server_name, const QString &address);
};

class UdpDetector: public Detector{
    Q_OBJECT

public:
    UdpDetector();
    virtual void detect();
    virtual void stop();

private slots:
    void onReadReady();

private:
    QUdpSocket *socket;
};

class IrcDetector: public Detector{
    Q_OBJECT

public:
    static IrcDetector *GetInstance();

    virtual void detect();
    virtual void stop();
    void emitConnected();

    void setAddrMap(const char *nick, const char *addr);
    void setInfoMap(const char *nick, const char *server_info);
    void clearMap();

private:
    IrcDetector();
    irc_session_t *session;
    QMap<QString, QString> nick2addr;
    QMap<QString, ServerInfoStruct> nick2info;

signals:
    void server_connected();
};

class IrcRunner: public QThread{
    Q_OBJECT

public:
    IrcRunner(QObject *parent, irc_session_t *session);

protected:
    virtual void run();

private:
    irc_session_t *session;
};

#endif // DETECTOR_H
