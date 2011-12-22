#ifndef CLIENTLOGBOX_H
#define CLIENTLOGBOX_H

class ClientPlayer;

#include <QTextEdit>

class ClientLogBox : public QTextEdit{
    Q_OBJECT

public:
    explicit ClientLogBox(QWidget *parent = 0);
    void appendLog(
            const QString &type,
            const QString &from_general,
            const QStringList &to,
            const QString card_str = QString(),
            const QString arg = QString(),
            const QString arg2 = QString()
            );

private:
    QString bold(const QString &str, QColor color) const;

public slots:
    void appendLog(const QString &log_str);
    void append(const QString &log_str);
    void appendSeparator();
};

#endif // CLIENTLOGBOX_H
