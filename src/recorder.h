#ifndef RECORDER_H
#define RECORDER_H

#include <QObject>
#include <QTime>
#include <QThread>


class Recorder : public QObject
{
    Q_OBJECT
public:
    explicit Recorder(QObject *parent);
    bool save(const QString &filename) const;
    void recordLine(const QString &line);

public slots:
    void record(char *line);

private:
    QTime watch;
    QByteArray data;
};

class Replayer: public QThread
{
    Q_OBJECT

public:
    explicit Replayer(QObject *parent, const QString &filename);

protected:
    virtual void run();

private:
    QString filename;

signals:
    void command_parsed(const QString &cmd);
};

#endif // RECORDER_H
