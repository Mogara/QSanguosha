#include "engine.h"
#include "card.h"
#include "client.h"

#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QDir>
#include <QLibrary>
#include <QApplication>

Engine *Sanguosha = NULL;

extern "C" {
    Package *NewStandard();
    Package *NewWind();
    Package *NewFire();
    Package *NewThicket();
    Package *NewManeuvering();
    Package *NewGod();
    Package *NewYitian();
}

Engine::Engine(QObject *parent)
    :QObject(parent)
{
    addPackage(NewStandard());
    addPackage(NewWind());
    addPackage(NewFire());
    addPackage(NewThicket());
    addPackage(NewManeuvering());
    addPackage(NewGod());
    addPackage(NewYitian());
}

void Engine::addPackage(Package *package){
    package->setParent(this);
    QString package_name = package->objectName();

    QList<Card *> all_cards = package->findChildren<Card *>();
    foreach(Card *card, all_cards){
        card->setId(cards.length());
        cards << card;

        QString card_name = card->objectName();
        metaobjects.insert(card_name, card->metaObject());

        if(!male_effects.contains(card_name)){
            MediaSource male_source(QString("%1/cards/effect/male/%2.mp3").arg(package_name).arg(card_name));
            male_effects.insert(card_name, male_source);
        }

        if(!female_effects.contains(card_name)){
            MediaSource female_source(QString("%1/cards/effect/female/%2.mp3").arg(package_name).arg(card_name));
            female_effects.insert(card_name, female_source);
        }
    }

    QList<General *> all_generals = package->findChildren<General *>();
    foreach(General *general, all_generals){
        if(general->isLord())
            lord_list << general->objectName();
        else
            nonlord_list << general->objectName();

        generals.insert(general->objectName(), general);

        QList<const Skill *> all_skills = general->findChildren<const Skill *>();
        foreach(const Skill *skill, all_skills)
            skills.insert(skill->objectName(), skill);
    }

    QList<const QMetaObject *> metas = package->getMetaObjects();
    foreach(const QMetaObject *meta, metas)
        metaobjects.insert(meta->className(), meta);

    translations.unite(package->getTranslation());

    QList<const Skill *> extra_skills = package->getSkills();
    foreach(const Skill *skill, extra_skills)
        skills.insert(skill->objectName(), skill);
}

void Engine::addBanPackage(const QString &package_name){
    ban_package.insert(package_name);
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
        return cards.at(index);
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

int Engine::getCardCount() const{
    return cards.length();
}

QStringList Engine::getRandomLords() const{
    QStringList lords;

    // add intrinsic lord
    foreach(QString lord, lord_list){
        const General *general = generals.value(lord);
        if(!ban_package.contains(general->getPackage()))
            lords << lord;        
    }

    QStringList nonlord_list;
    foreach(QString nonlord, this->nonlord_list){
        const General *general = generals.value(nonlord);
        if(!ban_package.contains(general->getPackage()))
            nonlord_list << nonlord;
    }

    int i, n = nonlord_list.length();
    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        nonlord_list.swap(r1, r2);
    }

    const static int extra = 2;
    for(i=0; i< extra; i++)
        lords << nonlord_list.at(i);

    return lords;
}

QStringList Engine::getLimitedGeneralNames() const{
    QStringList general_names;
    QHashIterator<QString, const General *> itor(generals);
    while(itor.hasNext()){
        itor.next();
        if(!ban_package.contains(itor.value()->getPackage())){
            general_names << itor.key();
        }
    }

    return general_names;
}

QStringList Engine::getRandomGenerals(int count, const QSet<QString> &ban_set) const{
    QStringList all_generals = getLimitedGeneralNames();

    int n = all_generals.count();
    Q_ASSERT(n >= count);

    // shuffle them
    int i;
    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        all_generals.swap(r1, r2);
    }

    if(!ban_set.isEmpty()){
        QSet<QString> general_set = all_generals.toSet();
        all_generals = general_set.subtract(ban_set).toList();
    }

    QStringList general_list = all_generals.mid(0, count);
    Q_ASSERT(general_list.count() == count);

    return general_list;
}

QList<int> Engine::getRandomCards() const{
    QList<int> list;
    int n = cards.count(), i;
    for(i=0; i<n; i++){
        const Card *card = cards.at(i);
        if(!ban_package.contains(card->parent()->objectName()))
            list << i;
    }

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        list.swap(r1, r2);
    }

    return list;
}

void Engine::playEffect(const MediaSource &source){
    foreach(MediaObject *effect, effects){
        if(effect->currentSource().fileName() == source.fileName())
            return;
    }

    MediaObject *effect = Phonon::createPlayer(Phonon::MusicCategory);
    effects << effect;

    effect->setCurrentSource(source);
    effect->play();

    connect(effect, SIGNAL(finished()), this, SLOT(removeFromEffects()));
}

void Engine::playSkillEffect(const QString &skill_name, int index){
    const Skill *skill = skills.value(skill_name, NULL);
    if(skill)
        skill->playEffect(index);
}

void Engine::playCardEffect(const QString &card_name, bool is_male){
    QHash<QString, MediaSource> &source = is_male ? male_effects : female_effects;
    if(source.contains(card_name))
        playEffect(source[card_name]);
    else
        QMessageBox::warning(NULL, tr("Warning"), tr("No suitable card effect was found! Card name is %1").arg(card_name));
}

const Skill *Engine::getSkill(const QString &skill_name) const{
    return skills.value(skill_name, NULL);
}

void Engine::removeFromEffects(){
    MediaObject *effect = qobject_cast<MediaObject *>(sender());
    if(effect){
        effects.removeOne(effect);
    }
}
