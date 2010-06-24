#include "engine.h"

#include <QFile>
#include <QStringList>

Engine::Engine(QObject *parent) :
    QScriptEngine(parent)
{
    QScriptValue mainwindow = newQObject(parent);
    globalObject().setProperty("mainwindow", mainwindow);

    QStringList script_files;
    script_files << "init.js" << "cards.js" << "cagenerals.js";
    foreach(QString filename, script_files){
        QFile file("scripts/" + filename);
        if(file.open(QIODevice::ReadOnly)){
            evaluate(file.readAll());
        }
    }
}
