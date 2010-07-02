#ifndef ROOM_H
#define ROOM_H

#include <QScriptValue>
#include <QTcpSocket>

class Room : public QObject
{
    Q_OBJECT
public:
    explicit Room(QObject *parent, int player_count);
    void addSocket(QTcpSocket *socket);
    bool isFull() const;

    Q_INVOKABLE void pushEvent(const QScriptValue &event);

protected:
    virtual bool event(QEvent *);

private:
    QList<QTcpSocket*> sockets;
    int player_count;

private slots:
    void reportDisconnection();
    void getRequest();

signals:
    void room_message(const QString &);
};

#endif // ROOM_H
