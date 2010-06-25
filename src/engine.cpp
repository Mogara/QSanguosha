#include "engine.h"

#include <QFile>
#include <QStringList>
#include <QMessageBox>

Engine::Engine(QObject *parent) :
    QScriptEngine(parent)
{
    globalObject().setProperty("sgs", newQObject(this));

    generals = new QObject(this);
    generals->setObjectName("generals");

    chinese = new QObject(this);
    chinese->setObjectName("chinese");
}

void Engine::init()
{
    QStringList script_files;
    script_files << "init.js" << "cards.js" << "cagenerals.js";
    foreach(QString filename, script_files){
        QFile file("scripts/" + filename);
        if(file.open(QIODevice::ReadOnly)){
            evaluate(file.readAll(), filename);
            if(this->hasUncaughtException()){
                QMessageBox::warning(NULL, tr("Script exception!"), uncaughtExceptionBacktrace().join("\n"));
                return;
            }
        }
    }
}

General *Engine::addGeneral(const QString &name, const QString &kingdom, int max_hp, bool male){
    General *general = new General(name, kingdom, max_hp, male);
    general->setParent(generals);
    return general;
}
