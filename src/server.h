#ifndef SERVER_H
#define SERVER_H

class Room;

#include "socket.h"
#include "detector.h"
#include "libircclient.h"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QComboBox>
#include <QLayoutItem>

class ServerDialog: public QDialog{
    Q_OBJECT

public:
    ServerDialog(QWidget *parent);
    bool config();

private:
    QLayout *createLeft();
    QLayout *createRight();
    QLayout *createButtonLayout();

    QLineEdit *server_name_edit;
    QLineEdit *player_count_edit;
    QSpinBox *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QCheckBox *free_choose_checkbox;
    QCheckBox *forbid_same_ip_checkbox;
    QCheckBox *second_general_checkbox;
    QCheckBox *announce_ip_checkbox;
    QComboBox *scenario_combobox;
    QLineEdit *address_edit;
    QLineEdit *port_edit;

    QButtonGroup *ai_group;
    QButtonGroup *extension_group;
    QButtonGroup *mode_group;

private slots:
    void onOkButtonClicked();
    void onDetectButtonClicked();
    void onHttpDone(bool error);
    void onGameModeChanged();
};

class Scenario;

class Server : public QObject{
    Q_OBJECT

public:
    explicit Server(QObject *parent);
    ~Server();

    bool listen();
    void daemonize();
    void emitDetectableMessage();
    void emitServerMessage(const QString &msg);
    void giveInfo(const char *nick);
    void removeNick(const char *nick);
    void tellLack(const char *nick = NULL);

private:
    ServerSocket *server;
    Room *current;
    QSet<QString> addresses, nicks;
    irc_session_t *session;

private slots:
    void processNewConnection(ClientSocket *socket);
    void cleanup();

signals:
    void server_message(const QString &);
};

#endif // SERVER_H
