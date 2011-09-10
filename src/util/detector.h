#ifndef DETECTOR_H
#define DETECTOR_H

#include <QObject>
#include <QString>
#include <QUdpSocket>
#include <QThread>

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

#endif // DETECTOR_H
