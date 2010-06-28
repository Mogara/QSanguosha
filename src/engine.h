#ifndef ENGINE_H
#define ENGINE_H

#include "general.h"

#include <QScriptEngine>

class Engine : public QScriptEngine
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent);

    // invokable methods, functions that marked this flags can be called by scripts enironment
    Q_INVOKABLE QObject *addGeneral(const QString &name, const QString &kingdom, int max_hp, bool male);
    Q_INVOKABLE QObject *addCard(const QString &card_class, const QString &suit_str, int number);
    Q_INVOKABLE QObject *addCardClass(const QString &class_name);

    Q_INVOKABLE void addTranslationTable(const QScriptValue &table);
    Q_INVOKABLE QString translate(const QString &to_translate);

    Q_INVOKABLE QScriptValue doScript(const QString &filename);
    Q_INVOKABLE void alert(const QString &message);
    Q_INVOKABLE void quit(const QString &reason = "");

    General *getGeneral(const QString &name);

private:
    QObject *generals;
    QObject *translation;
    QObject *card_classes;
};

extern Engine *Sanguosha;

#endif // ENGINE_H
