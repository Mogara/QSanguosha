#ifndef CLIENT_H
#define CLIENT_H

#include <QTcpSocket>

class Client : public QTcpSocket
{
Q_OBJECT
public:
    explicit Client(QObject *parent = 0);
    void request(const QString &message);
    void setField(const QString &key, const QString &value);
    void signup();

private:
    int seat_no;

private slots:
    void raiseError(QAbstractSocket::SocketError socket_error);

signals:
    void errorMessage(const QString &msg);
};

#endif // CLIENT_H
