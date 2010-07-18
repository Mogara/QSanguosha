#ifndef CARDCLASS_H
#define CARDCLASS_H

#include <QObject>
#include <QScriptValue>

class Card;

class CardClass : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QScriptValue available_func READ availableFunc WRITE setAvailableFunc)
public:
    friend class Card;

    explicit CardClass(const QString &name, const QString &type, const QString &subtype, int id, const QString &pixmap_dir);
    QString getPixmapPath() const;

    QScriptValue availableFunc() const;
    void setAvailableFunc(const QScriptValue &func);
    bool isAvailable(const QScriptValue &env);

private:
    QString type, subtype;
    int id;
    QString pixmap_dir;
    QScriptValue available_func;
};

#endif // CARDCLASS_H
