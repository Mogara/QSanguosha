#ifndef IRCDETECTOR_H
#define IRCDETECTOR_H

#include "libircclient.h"
#include "clientstruct.h"
#include "detector.h"

#include <QDialog>
#include <QProgressBar>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

class IrcDetector: public Detector{
    Q_OBJECT

public:
    IrcDetector();
    ~IrcDetector();

    virtual void detect();
    virtual void stop();
    void emitConnected();

    void setAddrMap(const char *nick, const char *addr);
    void setInfoMap(const char *nick, const char *server_info);

    QString getAddr(const QString &nick) const;
    bool getInfo(const QString &nick, ServerInfoStruct &info) const;

    void clearMap();

private:
    irc_session_t *session;
    QMap<QString, QString> nick2addr;
    QMap<QString, ServerInfoStruct> nick2info;

signals:
    void server_connected();
    void detected(const QString &nick);
    void parted(const QString &nick);
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

class IrcDetectorDialog: public QDialog{
    Q_OBJECT

public:
    IrcDetectorDialog();

private:
    IrcDetector *detector;
    QLabel *info_label;
    QListWidget *list;
    QProgressBar *progress_bar;
    QPushButton *detect_button;

private slots:
    void onServerConnected();
    void startDetection();
    void addNick(const QString &nick);
    void chooseAddress(QListWidgetItem *item);
};

#endif // WANDETECTOR_H
