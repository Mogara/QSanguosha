#ifndef ENGINE_H
#define ENGINE_H

#include <QScriptEngine>

class Engine : public QScriptEngine
{
Q_OBJECT
public:
    explicit Engine(QObject *parent = 0);

signals:

public slots:

};

#endif // ENGINE_H
