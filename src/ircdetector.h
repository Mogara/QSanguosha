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

struct ServerFullInfo: public ServerInfoStruct{
    ServerFullInfo():lack(0){

    }

    QString address;
    int lack;
};

class IrcDetector: public Detector{
    Q_OBJECT

public:
    IrcDetector();
    ~IrcDetector();

    virtual void detect();
    virtual void stop();

    void emitConnected();
    void emitParted(const char *nick);
    void askPerson(const char *nick, int count);

    void setAddrMap(const char *nick, const char *addr);
    void setInfoMap(const char *nick, const char *server_info);
    const ServerFullInfo &getInfo(const QString &nick);

    void clearMap();

private:
    irc_session_t *session;
    QMap<QString, ServerFullInfo> nick2info;

signals:
    void server_connected();
    void detected(const QString &nick);
    void parted(const QString &nick);
    void person_asked(const QString &nick, int);
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
    ServerInfoWidget *info_widget;

private slots:
    void onServerConnected();
    void startDetection();
    void addNick(const QString &nick);
    void removeNick(const QString &nick);   
    void copyAddress(QListWidgetItem *item);
    void updateServerInfo();
    void updateLack(const QString &nick, int lack);
};

#endif // WANDETECTOR_H
