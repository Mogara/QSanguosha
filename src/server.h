#ifndef SERVER_H
#define SERVER_H

class Room;

#include "socket.h"

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

    QLineEdit *server_name_lineedit;
    QSpinBox *player_count_spinbox, *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QCheckBox *free_choose_checkbox;
    QCheckBox *forbid_same_ip_checkbox;
    QCheckBox *second_general_checkbox;
    QComboBox *scenario_combobox;

    QButtonGroup *ai_group;
    QButtonGroup *connection_group;
    QButtonGroup *extension_group;
};

class Server : public QObject{
    Q_OBJECT

public:
    explicit Server(QObject *parent);
    bool listen();
    void daemonize();

private:
    ServerSocket *server;
    QList<Room*> rooms;
    QSet<QString> addresses;

private slots:
    void processNewConnection(ClientSocket *socket);
    void removeAddress();

signals:
    void server_message(const QString &);
};

#endif // SERVER_H
