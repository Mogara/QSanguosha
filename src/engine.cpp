#include "engine.h"
#include "card.h"
#include "standard.h"
#include "client.h"

#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QDir>

Engine *Sanguosha = NULL;

Engine::Engine(QObject *parent)
    :QObject(parent), effect(Phonon::createPlayer(Phonon::MusicCategory))
{
    addPackage(new StandardPackage);
}

void Engine::addPackage(Package *package){
    package->setParent(this);

    QList<Card *> all_cards = package->findChildren<Card *>();
    foreach(Card *card, all_cards){
        card->setID(cards.length());
        cards << card;
        metaobjects.insert(card->objectName(), card->metaObject());
    }

    QList<General *> all_generals = package->findChildren<General *>();
    foreach(General *general, all_generals){
        if(general->isLord())
            lord_names << general->objectName();
        generals.insert(general->objectName(), general);
    }

    QList<const QMetaObject *> metas = package->getMetaObjects();
    foreach(const QMetaObject *meta, metas)
        metaobjects.insert(meta->className(), meta);

    translations.unite(package->getTranslation());
}

QString Engine::translate(const QString &to_translate) const{
    return translations.value(to_translate, to_translate);
}

const General *Engine::getGeneral(const QString &name) const{
    return generals.value(name, NULL);
}

int Engine::getGeneralCount() const{
    return generals.size();
}

const Card *Engine::getCard(int index) const{
    if(index < 0 || index >= cards.length())
        return NULL;
    else
        return cards[index];
}

Card *Engine::cloneCard(const QString &name, Card::Suit suit, int number) const{
    const QMetaObject *meta = metaobjects.value(name, NULL);
    if(meta){
        QObject *card_obj = meta->newInstance(Q_ARG(Card::Suit, suit), Q_ARG(int, number));
        return qobject_cast<Card *>(card_obj);
    }else
        return NULL;
}

SkillCard *Engine::cloneSkillCard(const QString &name){
    const QMetaObject *meta = metaobjects.value(name, NULL);
    if(meta){
        QObject *card_obj = meta->newInstance();
        SkillCard *card = qobject_cast<SkillCard *>(card_obj);
        return card;
    }else
        return NULL;
}

CardPattern *Engine::cloneCardPattern(const QString &name, const QString &pattern_str){
    const QMetaObject *meta = metaobjects.value(name, NULL);
    if(meta){
        QObject *pattern_obj = meta->newInstance(Q_ARG(QString, pattern_str));
        return qobject_cast<CardPattern *>(pattern_obj);
    }else
        return NULL;
}

int Engine::getCardCount() const{
    return cards.length();
}

void Engine::getRandomLords(QStringList &lord_list, int lord_count) const{
    int min = qMin(lord_count, lord_names.count()), i;
    for(i=0; i<min; i++)
        lord_list << lord_names[i];

    QList<const General*> all_generals = generals.values();
    for(i=0; i<lord_count-min; i++){
        const General *general = NULL;

        while(general == NULL){
            int r = qrand() % all_generals.count();
            const General *chosen = all_generals.at(r);
            if(!chosen->isLord())
                general = chosen;
        }

        lord_list << general->objectName();
    }    
}

void Engine::getRandomGenerals(QStringList &general_list, int count) const{
    QList<const General *> all_generals = generals.values();
    int n = all_generals.count();
    Q_ASSERT(n >= count);

    // shuffle them
    int i;
    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        all_generals.swap(r1, r2);
    }

    for(i=0; i<count; i++)
        general_list << all_generals.at(i)->objectName();

    Q_ASSERT(general_list.count() == count);
}

void Engine::getRandomCards(QList<int> &list) const{
    int n = cards.count(), i;
    for(i=0; i<n; i++)
        list << i;

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        list.swap(r1, r2);
    }
}

void Engine::playEffect(const Phonon::MediaSource &source){
    effect->setCurrentSource(source);
    effect->play();
}

