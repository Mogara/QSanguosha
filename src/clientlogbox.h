#ifndef CLIENTLOGBOX_H
#define CLIENTLOGBOX_H

#include <QTextEdit>

class ClientLogBox : public QTextEdit{
    Q_OBJECT

public:
    explicit ClientLogBox(QWidget *parent = 0);

public slots:
    void appendLog(const QString &log_str);
};

#endif // CLIENTLOGBOX_H
