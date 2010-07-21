#include "engine.h"
#include "card.h"
#include "standard.h"

#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QDir>

Engine *Sanguosha = NULL;

Engine::Engine(QObject *parent)
    :QObject(parent)
{
    addPackage(new StandardPackage);

    event_type = static_cast<QEvent::Type>(QEvent::registerEventType());
}

void Engine::addPackage(Package *package){
    package->setParent(this);

    QList<Card *> all_cards = package->findChildren<Card *>();
    foreach(Card *card, all_cards){
        card->setID(cards.length());
        cards << card;
    }

    QList<General *> all_generals = package->findChildren<General *>();
    foreach(General *general, all_generals){
        if(general->isLord())
            lord_names << general->objectName();
        generals.insert(general->objectName(), general);
    }

    translations.unite(package->getTranslation());
}


QString Engine::translate(const QString &to_translate){
    return translations.value(to_translate, to_translate);
}

QEvent::Type Engine::getEventType() const{
    return event_type;
}

const General *Engine::getGeneral(const QString &name){
    return generals.value(name, NULL);
}

int Engine::getGeneralCount() const{
    return generals.size();
}

Card *Engine::getCard(int index){
    if(index < 0 || index >= cards.length())
        return NULL;
    else
        return cards[index];
}

int Engine::getCardCount() const{
    return cards.length();
}

void Engine::getRandomLords(QStringList &lord_list, int lord_count){
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

void Engine::getRandomGenerals(QStringList &general_list, int count){
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

void Engine::getRandomCards(QList<int> &list){
    int n = cards.count(), i;
    for(i=0; i<n; i++)
        list << i;

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        list.swap(r1, r2);
    }
}
