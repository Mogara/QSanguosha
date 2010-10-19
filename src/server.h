#ifndef SERVER_H
#define SERVER_H

class Room;

#include "socket.h"

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QButtonGroup>

class ServerDialog: public QDialog{
    Q_OBJECT

public:
    ServerDialog(QWidget *parent);
    bool config();

private:
    QSpinBox *player_count_spinbox, *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QCheckBox *free_choose_checkbox;
    QButtonGroup *extension_group;
};

class Server : public QObject{
    Q_OBJECT

public:
    explicit Server(QObject *parent);
    bool listen();

private:
    ServerSocket *server;
    QList<Room*> rooms;

private slots:
    void processNewConnection(ClientSocket *socket);

signals:
    void server_message(const QString &);
};

#endif // SERVER_H
