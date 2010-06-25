#ifndef ENGINE_H
#define ENGINE_H

#include "general.h"

#include <QScriptEngine>

class Engine : public QScriptEngine
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent);
    void init(); // evaluate the scripts

    // invokable methods, functions that marked this flags can be called by scripts enironment
    Q_INVOKABLE General *addGeneral(const QString &name, const QString &kingdom, int max_hp, bool male);

private:
    QObject *generals;
    QObject *chinese;
};

#endif // ENGINE_H
