#include "engine.h"
#include "card.h"
#include "cardclass.h"

#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QScriptValueIterator>

Engine *Sanguosha = NULL;

Engine::Engine(QObject *parent) :
    QScriptEngine(parent)
{
    globalObject().setProperty("sgs", newQObject(this));

    generals = new QObject(this);
    generals->setObjectName("generals");

    translation = new QObject(this);
    translation->setObjectName("translation");

    card_classes = new QObject(this);
    card_classes->setObjectName("card_classes");

    QStringList script_files;
    script_files << "init.js" << "cards.js" << "generals.js";
    foreach(QString filename, script_files){
        doScript("scripts/" + filename);
    }
}

QObject *Engine::addGeneral(const QString &name, const QString &kingdom, int max_hp, bool male){
    General *general = new General(name, kingdom, max_hp, male);
    general->setParent(generals);
    return general;
}

QObject *Engine::addCard(const QString &name, const QString &suit_str, int number){
    Card::Suit suit;
    if(suit_str == "spade")
        suit = Card::Spade;
    else if(suit_str == "club")
        suit = Card::Club;
    else if(suit_str == "heart")
        suit = Card::Heart;
    else if(suit_str == "diamond")
        suit = Card::Diamond;
    else
        suit = Card::NoSuit;

    Card *card = new Card(name, suit, number);
    return card;
}

QObject *Engine::addCardClass(const QString &class_name){
    CardClass *card_class = new CardClass(class_name, card_classes);
    return card_class;
}

void Engine::addTranslationTable(const QScriptValue &table)
{
    if(!table.isObject())
        return;

    QScriptValueIterator itor(table);
    while(itor.hasNext()){
        itor.next();
        translation->setProperty(itor.name().toAscii(), itor.value().toString());
    }
}

QString Engine::translate(const QString &to_translate){
    return translation->property(to_translate.toAscii()).toString();
}

General *Engine::getGeneral(const QString &name){
    return generals->findChild<General*>(name);
}

QScriptValue Engine::doScript(const QString &filename){
    QString error_msg;
    QScriptValue result;
    QFile file(filename);
    if(file.open(QIODevice::ReadOnly)){
        result = evaluate(file.readAll(), filename);
        if(hasUncaughtException()){
            error_msg =  tr("%1\n\n Stack trace:\n %2")
                         .arg(uncaughtException().toString())
                         .arg(uncaughtExceptionBacktrace().join("\n"));
        }
    }else
        error_msg = tr("Script file %1 can not be opened for execution!").arg(filename);

    if(!error_msg.isEmpty()){
        QMessageBox::warning(NULL, tr("Script exception!"), error_msg);
        exit(1);
    }

    return result;
}

void Engine::alert(const QString &message){
    QMessageBox::information(NULL, tr("Script alert"), message);
}

void Engine::quit(const QString &reason){
    if(!reason.isEmpty())
        QMessageBox::warning(NULL, tr("Script quit"), reason);
    exit(0);
}

