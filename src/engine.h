#ifndef ENGINE_H
#define ENGINE_H

#include "general.h"

#include <QScriptEngine>

class Engine : public QScriptEngine
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = 0);
    Q_INVOKABLE General *addGeneral(const QString &name, const QString &kingdom, int max_hp, bool male);
private:
    QObject *generals;
};

#endif // ENGINE_H
