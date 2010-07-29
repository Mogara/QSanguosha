#include "engine.h"
#include "card.h"
#include "standard.h"
#include "client.h"

#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QDir>

Engine *Sanguosha = NULL;

class BasicRule : public Skill{
public:
    BasicRule():Skill("basic_rule"){

    }

    void onPhaseChange(Client *client, Player::Phase phase) const{
        // const ClientPlayer *player = client->getPlayer();

        switch(phase){
        case Player::Start:{
                client->tag.insert("slash_count", 0);
                client->availability.clear();

                if(client->tag.value("auto_end_start", true).toBool()){
                    client->endPhase();
                    client->tag["auto_end_start"] = true;
                }

                break;
            }

        case Player::Judge:{
                if(client->tag.value("skip_judge", false).toBool()){
                    client->endPhase();
                    client->tag["skip_judge"] = false;
                }else{
                    // request for judge

                    client->endPhase();
                }
                break;
        }

        case Player::Draw:{
                if(client->tag.value("skip_draw", false).toBool()){
                    client->endPhase();
                    client->tag["skip_draw"] = false;
                }else{
                    client->askForCards(2);
                    client->endPhase();
                }
                break;
            }

        case Player::Play:{
                if(client->tag.value("skip_play", false).toBool()){
                    client->endPhase();
                    client->tag["skip_play"] = false;
                }else{
                    client->setActivity(true);
                }

                break;
            }

        case Player::Discard:{
                // FIXME: if no cards need to discard, skip this phase

                if(client->tag.value("skip_discard", false).toBool()){
                    client->endPhase();
                    client->tag["skip_discard"] = false;
                }else{
                    // ask for discard cards;
                }

                break;
            }

        case Player::Finish:{
                if(client->tag.value("auto_end_finish", true).toBool()){
                    client->endPhase();
                    client->tag["auto_end_finish"] = true;
                }
            }

        default:
            ;
        }
    }

    virtual void trigger(Client *client, TriggerReason reason, const QVariant &data) const{
        const ClientPlayer *player = client->getPlayer();

        switch(reason){
        case GameStart: break;
        case PhaseChange: onPhaseChange(client, player->getPhase()); break;
        case HpDamage:
        default:  ;
        }
    }
};

Engine::Engine(QObject *parent)
    :QObject(parent), basic_rule(new BasicRule),
    effect(Phonon::createPlayer(Phonon::MusicCategory))
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


QString Engine::translate(const QString &to_translate) const{
    return translations.value(to_translate, to_translate);
}

QEvent::Type Engine::getEventType() const{
    return event_type;
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
    Card *card = findChild<Card *>(name);
    if(card)
        return card->clone(suit, number);
    else
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

const Skill *Engine::getBasicRule() const{
    return basic_rule;
}

void Engine::playEffect(const Phonon::MediaSource &source){
    effect->setCurrentSource(source);
    effect->play();
}
