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

Engine::Engine(QObject *parent)
    :QObject(parent)
{
    QStringList package_names;
    package_names << "Standard" << "Wind" << "Maneuvering" << "Thicket";

    QLibrary library(qApp->applicationFilePath());

    if(!library.load()){
        QMessageBox::critical(NULL, tr("Fatal error"), tr("Package loading error!"));
        exit(1);
    }

    typedef Package *(*package_new_func)();

    foreach(QString package_name, package_names){
        QString func_name = QString("New%1").arg(package_name);
        package_new_func new_func = (package_new_func)library.resolve(func_name.toAscii());
        if(new_func){
            Package *package = new_func();
            addPackage(package);
        }else
            QMessageBox::critical(NULL, tr("Fatal error"), tr("Package %1 loading error!").arg(package_name));
    }
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

QStringList Engine::getRandomLords(int lord_count) const{
    QStringList lords = lord_list;

    if(lord_count < lord_list.length()){
        QMessageBox::warning(NULL, tr("Warning"),
                             tr("The lord count must greater or equal to the intrinsic lord number(%1)").arg(lord_list.length()));
        return lord_list;
    }

    QStringList nonlord_list = this->nonlord_list;
    int i, n = nonlord_list.length();
    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        nonlord_list.swap(r1, r2);
    }

    int extra = lord_count - lord_list.length();
    for(i=0; i< extra; i++)
        lords << nonlord_list.at(i);

    return lords;
}

QStringList Engine::getRandomGenerals(int count, const QSet<QString> &ban_set) const{
    QStringList all_generals = generals.keys();

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

#ifndef QT_NO_DEBUG
    QStringList my_list;
    my_list << "zhugeliang" << "menghuo" << "xiahoudun";

    for(i=0; i<my_list.length(); i++){
        QString my_general = my_list.at(i);
        int index = all_generals.indexOf(my_general);
        all_generals.swap(index, i);
    }

#endif

    QStringList general_list = all_generals.mid(0, count);
    Q_ASSERT(general_list.count() == count);

    return general_list;
}

QList<int> Engine::getRandomCards() const{
    QList<int> list;
    int n = cards.count(), i;
    for(i=0; i<n; i++)
        list << i;

    for(i=0; i<n; i++){
        int r1 = qrand() % n;
        int r2 = qrand() % n;
        list.swap(r1, r2);
    }

#ifndef QT_NO_DEBUG

    QList<int> my_list;
    my_list << 60;

    for(i=0; i<my_list.length(); i++){
        int card_id = my_list.at(i);
        int index = list.indexOf(card_id);
        list.swap(index, i);
    }

#endif

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
