#ifndef SERVER_H
#define SERVER_H

class Room;

#include <QTcpServer>
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
    QLineEdit *port_edit;
    QSpinBox *player_count_spinbox, *timeout_spinbox;
    QCheckBox *nolimit_checkbox;
    QButtonGroup *extension_group;
};

class Server : public QObject{
    Q_OBJECT

public:
    explicit Server(QObject *parent);
    bool listen();

private:
    QTcpServer *server;
    QList<Room*> rooms;

private slots:
    void processNewConnection();

signals:
    void server_message(const QString &);
};

#endif // SERVER_H
