#ifndef CARDCLASS_H
#define CARDCLASS_H

#include <QObject>

class CardClass : public QObject
{
    Q_OBJECT
public:
    explicit CardClass(const QString &name, QObject *parent);
signals:

public slots:

};

#endif // CARDCLASS_H
