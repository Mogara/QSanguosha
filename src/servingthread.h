#ifndef SERVINGTHREAD_H
#define SERVINGTHREAD_H

#include <QThread>
#include <QTcpSocket>

class ServingThread : public QThread
{
    Q_OBJECT
public:
    explicit ServingThread(QObject *parent, QTcpSocket *socket);

protected:
    virtual void run();

private:
    QTcpSocket *socket;

signals:
    void thread_message(const QString &);
};

#endif // SERVINGTHREAD_H
