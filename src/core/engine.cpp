/********************************************************************
    Copyright (c) 2013-2014 - QSanguosha-Rara

    This file is part of QSanguosha-Hegemony.

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 3.0
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    See the LICENSE file for more details.

    QSanguosha-Rara
    *********************************************************************/

#include "engine.h"
#include "card.h"
#include "client.h"
#include "ai.h"
#include "settings.h"
#include "scenario.h"
#include "audio.h"
#include "structs.h"
#include "lua-wrapper.h"
#include "RoomState.h"
#include "banpair.h"
#include "miniscenarios.h"
#include "jiange-defense-scenario.h"

#include <lua.hpp>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QApplication>

Engine *Sanguosha = NULL;

int Engine::getMiniSceneCounts() {
    return m_miniScenes.size();
}

void Engine::_loadMiniScenarios() {
    static bool loaded = false;
    if (loaded) return;
    int i = 1;
    while (true) {
        if (!QFile::exists(QString("etc/customScenes/%1.txt").arg(QString::number(i))))
            break;

        QString sceneKey = QString(MiniScene::S_KEY_MINISCENE).arg(QString::number(i));
        m_miniScenes[sceneKey] = new LoadedScenario(QString::number(i));
        i++;
    }
    loaded = true;
}

void Engine::_loadModScenarios() {
    //wait for a new scenario
    addScenario(new JiangeDefenseScenario());
}

void Engine::addPackage(const QString &name) {
    Package *pack = PackageAdder::packages()[name];
    if (pack)
        addPackage(pack);
    else
        qWarning("Package %s cannot be loaded!", qPrintable(name));
}

Engine::Engine()
{
    Sanguosha = this;

    lua = CreateLuaState();
    DoLuaScript(lua, "lua/config.lua");

    QStringList stringlist_sp_convert = GetConfigFromLuaState(lua, "convert_pairs").toStringList();
    foreach(QString cv_pair, stringlist_sp_convert) {
        QStringList pairs = cv_pair.split("->");
        QStringList cv_to = pairs.at(1).split("|");
        foreach(QString to, cv_to)
            sp_convert_pairs.insertMulti(pairs.at(0), to);
    }

    QStringList package_names = GetConfigFromLuaState(lua, "package_names").toStringList();
    foreach(QString name, package_names)
        addPackage(name);

    metaobjects.insert("TransferCard", &TransferCard::staticMetaObject);

    transfer = new TransferSkill;

    _loadMiniScenarios();
    _loadModScenarios();
    m_customScene = new CustomScenario();

    DoLuaScript(lua, "lua/sanguosha.lua");

    // available game modes
    modes["02p"] = tr("2 players");
    modes["03p"] = tr("3 players");
    modes["04p"] = tr("4 players");
    modes["05p"] = tr("5 players");
    modes["06p"] = tr("6 players");
    modes["07p"] = tr("7 players");
    modes["08p"] = tr("8 players");
    modes["09p"] = tr("9 players");
    modes["10p"] = tr("10 players");

    BanPair::loadBanPairs();

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));

    foreach (const Skill *skill, skills) {
        Skill *mutable_skill = const_cast<Skill *>(skill);
        mutable_skill->initMediaSource();
    }
}

lua_State *Engine::getLuaState() const{
    return lua;
}

void Engine::addTranslationEntry(const char *key, const char *value) {
    translations.insert(key, QString::fromUtf8(value));
}

Engine::~Engine() {
    lua_close(lua);
    delete m_customScene;
    delete transfer;
#ifdef AUDIO_SUPPORT
    Audio::quit();
#endif

    foreach (ExpPattern *pattern, enginePatterns) {
        delete pattern;
    }
}

QStringList Engine::getModScenarioNames() const{
    return m_scenarios.keys();
}

void Engine::addScenario(Scenario *scenario) {
    QString key = scenario->objectName();
    m_scenarios[key] = scenario;
    addPackage(scenario);
}

const Scenario *Engine::getScenario(const QString &name) const{
    if (m_scenarios.contains(name))
        return m_scenarios[name];
    else if (m_miniScenes.contains(name))
        return m_miniScenes[name];
    else if (name == "custom_scenario")
        return m_customScene;
    else return NULL;
}

void Engine::addSkills(const QList<const Skill *> &all_skills) {
    foreach(const Skill *skill, all_skills) {
        if (skills.contains(skill->objectName()))
            QMessageBox::warning(NULL, "", tr("Duplicated skill : %1").arg(skill->objectName()));

        skills.insert(skill->objectName(), skill);

        if (skill->inherits("ProhibitSkill"))
            prohibit_skills << qobject_cast<const ProhibitSkill *>(skill);
        else if (skill->inherits("DistanceSkill"))
            distance_skills << qobject_cast<const DistanceSkill *>(skill);
        else if (skill->inherits("MaxCardsSkill"))
            maxcards_skills << qobject_cast<const MaxCardsSkill *>(skill);
        else if (skill->inherits("TargetModSkill"))
            targetmod_skills << qobject_cast<const TargetModSkill *>(skill);
        else if (skill->inherits("AttackRangeSkill"))
            attackrange_skills << qobject_cast<const AttackRangeSkill *>(skill);
        else if (skill->inherits("TriggerSkill")) {
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            if (trigger_skill && trigger_skill->isGlobal())
                global_trigger_skills << trigger_skill;
        }
    }
}

QList<const DistanceSkill *> Engine::getDistanceSkills() const{
    return distance_skills;
}

QList<const MaxCardsSkill *> Engine::getMaxCardsSkills() const{
    return maxcards_skills;
}

QList<const TargetModSkill *> Engine::getTargetModSkills() const{
    return targetmod_skills;
}

QList<const AttackRangeSkill *> Engine::getAttackRangeSkills() const{
    return attackrange_skills;
}

QList<const TriggerSkill *> Engine::getGlobalTriggerSkills() const{
    return global_trigger_skills;
}

void Engine::addPackage(Package *package) {
    foreach (const Package *p, packages) {
        if (p->objectName() == package->objectName())
            return;
    }

    packages << package;
    package->setParent(this);
    sp_convert_pairs.unite(package->getConvertPairs());
    patterns.unite(package->getPatterns());
    related_skills.unite(package->getRelatedSkills());

    QList<Card *> all_cards = package->findChildren<Card *>();
    foreach(Card *card, all_cards) {
        card->setId(cards.length());
        cards << card;

        if (card->isKindOf("LuaBasicCard")) {
            const LuaBasicCard *lcard = qobject_cast<const LuaBasicCard *>(card);
            Q_ASSERT(lcard != NULL);
            luaBasicCard_className2objectName.insert(lcard->getClassName(), lcard->objectName());
            if (!luaBasicCards.contains(lcard->getClassName()))
                luaBasicCards.insert(lcard->getClassName(), lcard->clone());
        } else if (card->isKindOf("LuaTrickCard")) {
            const LuaTrickCard *lcard = qobject_cast<const LuaTrickCard *>(card);
            Q_ASSERT(lcard != NULL);
            luaTrickCard_className2objectName.insert(lcard->getClassName(), lcard->objectName());
            if (!luaTrickCards.contains(lcard->getClassName()))
                luaTrickCards.insert(lcard->getClassName(), lcard->clone());
        } else if (card->isKindOf("LuaWeapon")) {
            const LuaWeapon *lcard = qobject_cast<const LuaWeapon *>(card);
            Q_ASSERT(lcard != NULL);
            luaWeapon_className2objectName.insert(lcard->getClassName(), lcard->objectName());
            if (!luaWeapons.contains(lcard->getClassName()))
                luaWeapons.insert(lcard->getClassName(), lcard->clone());
        } else if (card->isKindOf("LuaArmor")) {
            const LuaArmor *lcard = qobject_cast<const LuaArmor *>(card);
            Q_ASSERT(lcard != NULL);
            luaArmor_className2objectName.insert(lcard->getClassName(), lcard->objectName());
            if (!luaArmors.contains(lcard->getClassName()))
                luaArmors.insert(lcard->getClassName(), lcard->clone());
        } else if (card->isKindOf("LuaTreasure")) {
            const LuaTreasure *lcard = qobject_cast<const LuaTreasure *>(card);
            Q_ASSERT(lcard != NULL);
            luaTreasure_className2objectName.insert(lcard->getClassName(), lcard->objectName());
            if (!luaTreasures.contains(lcard->getClassName()))
                luaTreasures.insert(lcard->getClassName(), lcard->clone());
        } else {
            QString class_name = card->metaObject()->className();
            metaobjects.insert(class_name, card->metaObject());
            className2objectName.insert(class_name, card->objectName());
        }
    }

    addSkills(package->getSkills());

    QList<General *> all_generals = package->findChildren<General *>();
    foreach(General *general, all_generals) {
        addSkills(general->findChildren<const Skill *>());
        foreach(QString skill_name, general->getExtraSkillSet()) {
            if (skill_name.startsWith("#")) continue;
            foreach(const Skill *related, getRelatedSkills(skill_name))
                general->addSkill(related->objectName());
        }
        generalList << general;
        generalHash.insert(general->objectName(), general);
        if (isGeneralHidden(general->objectName())) continue;
        if (general->isLord()) lord_list << general->objectName();
    }

    QList<const QMetaObject *> metas = package->getMetaObjects();
    foreach(const QMetaObject *meta, metas)
        metaobjects.insert(meta->className(), meta);
}

void Engine::addBanPackage(const QString &package_name) {
    ban_package.insert(package_name);
}

QList<const Package *> Engine::getPackages() const{
    return packages;
}

QStringList Engine::getBanPackages() const{
    if (qApp->arguments().contains("-server"))
        return Config.BanPackages;
    else
        return ban_package.toList();
}

QString Engine::translate(const QString &toTranslate) const{
    QStringList list = toTranslate.split("\\");
    QString res;
    foreach(QString str, list)
        res.append(translations.value(str, str));
    return res;
}

QString Engine::translate(const QString &toTranslate, const QString &defaultValue) const
{
    return translations.value(toTranslate, defaultValue);
}

const CardPattern *Engine::getPattern(const QString &name) const{
    const CardPattern *ptn = patterns.value(name, NULL);
    if (ptn) return ptn;

    ExpPattern *expptn = new ExpPattern(name);
    enginePatterns << expptn;
    patterns.insert(name, expptn);

    return expptn;
}

bool Engine::matchExpPattern(const QString &pattern, const Player *player, const Card *card) const{
    ExpPattern p(pattern);
    return p.match(player, card);
}

Card::HandlingMethod Engine::getCardHandlingMethod(const QString &method_name) const{
    if (method_name == "use")
        return Card::MethodUse;
    else if (method_name == "response")
        return Card::MethodResponse;
    else if (method_name == "discard")
        return Card::MethodDiscard;
    else if (method_name == "recast")
        return Card::MethodRecast;
    else if (method_name == "pindian")
        return Card::MethodPindian;
    else {
        Q_ASSERT(false);
        return Card::MethodNone;
    }
}

QList<const Skill *> Engine::getRelatedSkills(const QString &skill_name) const{
    QList<const Skill *> skills;
    foreach(QString name, related_skills.values(skill_name))
        skills << getSkill(name);

    return skills;
}

const Skill *Engine::getMainSkill(const QString &skill_name) const{
    const Skill *skill = getSkill(skill_name);
    if (!skill || skill->isVisible() || related_skills.contains(skill_name)) return skill;
    foreach(QString key, related_skills.keys()) {
        foreach(QString name, related_skills.values(key))
            if (name == skill_name) return getSkill(key);
    }
    return skill;
}

const General *Engine::getGeneral(const QString &name) const{
    if (generalHash.contains(name))
        return generalHash.value(name);
    else
        return NULL;
}

int Engine::getGeneralCount(bool include_banned) const{
    if (include_banned)
        return generalList.size();

    int total = generalList.size();
    foreach (const General *general, generalList) {
        if (getBanPackages().contains(general->getPackage()))
            total--;
    }

    return total;
}

void Engine::registerRoom(QObject *room) {
    m_mutex.lock();
    m_rooms[QThread::currentThread()] = room;
    m_mutex.unlock();
}

void Engine::unregisterRoom() {
    m_mutex.lock();
    m_rooms.remove(QThread::currentThread());
    m_mutex.unlock();
}

QObject *Engine::currentRoomObject() {
    QObject *room = NULL;
    m_mutex.lock();
    room = m_rooms[QThread::currentThread()];
    Q_ASSERT(room);
    m_mutex.unlock();
    return room;
}

Room *Engine::currentRoom() {
    QObject *roomObject = currentRoomObject();
    Room *room = qobject_cast<Room *>(roomObject);
    Q_ASSERT(room != NULL);
    return room;
}

RoomState *Engine::currentRoomState() {
    QObject *roomObject = currentRoomObject();
    Room *room = qobject_cast<Room *>(roomObject);
    if (room != NULL) {
        return room->getRoomState();
    }
    else {
        Client *client = qobject_cast<Client *>(roomObject);
        Q_ASSERT(client != NULL);
        return client->getRoomState();
    }
}

QString Engine::getCurrentCardUsePattern() {
    return currentRoomState()->getCurrentCardUsePattern();
}

CardUseStruct::CardUseReason Engine::getCurrentCardUseReason() {
    return currentRoomState()->getCurrentCardUseReason();
}

bool Engine::isGeneralHidden(const QString &general_name) const{
    const General *general = getGeneral(general_name);
    if (!general) return false;
    if (!general->isHidden())
        return Config.ExtraHiddenGenerals.contains(general_name);
    else
        return !Config.RemovedHiddenGenerals.contains(general_name);
}

TransferSkill *Engine::getTransfer() const
{
    return transfer;
}

WrappedCard *Engine::getWrappedCard(int cardId) {
    Card *card = getCard(cardId);
    WrappedCard *wrappedCard = qobject_cast<WrappedCard *>(card);
    Q_ASSERT(wrappedCard != NULL && wrappedCard->getId() == cardId);
    return wrappedCard;
}

Card *Engine::getCard(int cardId) {
    Card *card = NULL;
    if (cardId < 0 || cardId >= cards.length())
        return NULL;
    QObject *room = currentRoomObject();
    Q_ASSERT(room);
    Room *serverRoom = qobject_cast<Room *>(room);
    if (serverRoom != NULL) {
        card = serverRoom->getCard(cardId);
    }
    else {
        Client *clientRoom = qobject_cast<Client *>(room);
        Q_ASSERT(clientRoom != NULL);
        card = clientRoom->getCard(cardId);
    }
    Q_ASSERT(card);
    return card;
}

const Card *Engine::getEngineCard(int cardId) const{
    if (cardId == Card::S_UNKNOWN_CARD_ID)
        return NULL;
    else if (cardId < 0 || cardId >= cards.length()) {
        Q_ASSERT(false);
        return NULL;
    }
    else {
        Q_ASSERT(cards[cardId] != NULL);
        return cards[cardId];
    }
}

Card *Engine::cloneCard(const Card *card) const{
    Q_ASSERT(card->metaObject() != NULL);
    QString name = card->metaObject()->className();
    Card *result = cloneCard(name, card->getSuit(), card->getNumber(), card->getFlags());
    if (result == NULL)
        return NULL;
    result->setId(card->getEffectiveId());
    result->setSkillName(card->getSkillName(false));
    result->setObjectName(card->objectName());
    return result;
}

Card *Engine::cloneCard(const QString &name, Card::Suit suit, int number, const QStringList &flags) const{
    Card *card = NULL;
    if (luaBasicCard_className2objectName.contains(name)) {
        const LuaBasicCard *lcard = luaBasicCards.value(name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaBasicCard_className2objectName.values().contains(name)) {
        QString class_name = luaBasicCard_className2objectName.key(name, name);
        const LuaBasicCard *lcard = luaBasicCards.value(class_name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaTrickCard_className2objectName.contains(name)) {
        const LuaTrickCard *lcard = luaTrickCards.value(name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaTrickCard_className2objectName.values().contains(name)) {
        QString class_name = luaTrickCard_className2objectName.key(name, name);
        const LuaTrickCard *lcard = luaTrickCards.value(class_name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaWeapon_className2objectName.contains(name)) {
        const LuaWeapon *lcard = luaWeapons.value(name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaWeapon_className2objectName.values().contains(name)) {
        QString class_name = luaWeapon_className2objectName.key(name, name);
        const LuaWeapon *lcard = luaWeapons.value(class_name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaArmor_className2objectName.contains(name)) {
        const LuaArmor *lcard = luaArmors.value(name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaArmor_className2objectName.values().contains(name)) {
        QString class_name = luaArmor_className2objectName.key(name, name);
        const LuaArmor *lcard = luaArmors.value(class_name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaTreasure_className2objectName.contains(name)) {
        const LuaTreasure *lcard = luaTreasures.value(name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else if (luaTreasure_className2objectName.values().contains(name)) {
        QString class_name = luaTreasure_className2objectName.key(name, name);
        const LuaTreasure *lcard = luaTreasures.value(class_name, NULL);
        if (!lcard) return NULL;
        card = lcard->clone(suit, number);
    } else {
        const QMetaObject *meta = metaobjects.value(name, NULL);
        if (meta == NULL)
            meta = metaobjects.value(className2objectName.key(name, QString()), NULL);
        if (meta) {
            QObject *card_obj = meta->newInstance(Q_ARG(Card::Suit, suit), Q_ARG(int, number));
            card_obj->setObjectName(className2objectName.value(name, name));
            card = qobject_cast<Card *>(card_obj);
        }
    }
    if (!card) return NULL;
    card->clearFlags();
    if (!flags.isEmpty()) {
        foreach(QString flag, flags)
            card->setFlags(flag);
    }
    return card;
}

SkillCard *Engine::cloneSkillCard(const QString &name) const{
    const QMetaObject *meta = metaobjects.value(name, NULL);
    if (meta) {
        QObject *card_obj = meta->newInstance();
        SkillCard *card = qobject_cast<SkillCard *>(card_obj);
        return card;
    }
    else
        return NULL;
}

QSanVersionNumber Engine::getVersionNumber() const{
    return QSanVersionNumber(0, 7, 3, QSanVersionNumber::alpha);
}

QString Engine::getVersion() const{
    QString version_number = getVersionNumber();
    QString mod_name = getMODName();
    if (mod_name == "official")
        return version_number;
    else
        return QString("%1:%2").arg(version_number).arg(mod_name);
}

QString Engine::getVersionName() const{
    return "Heg";
}

QString Engine::getMODName() const{
    return "official";
}

QStringList Engine::getExtensions() const{
    QStringList extensions;
    foreach(const Package *package, packages) {
        if (package->inherits("Scenario"))
            continue;

        extensions << package->objectName();
    }
    return extensions;
}

QStringList Engine::getKingdoms() const{
    static QStringList kingdoms;
    if (kingdoms.isEmpty())
        kingdoms = GetConfigFromLuaState(lua, "kingdoms").toStringList();

    return kingdoms;
}

QColor Engine::getKingdomColor(const QString &kingdom) const{
    static QMap<QString, QColor> color_map;
    if (color_map.isEmpty()) {
        QVariantMap map = GetValueFromLuaState(lua, "config", "kingdom_colors").toMap();
        QMapIterator<QString, QVariant> itor(map);
        while (itor.hasNext()) {
            itor.next();
            QColor color(itor.value().toString());
            if (!color.isValid()) {
                qWarning("Invalid color for kingdom %s", qPrintable(itor.key()));
                color = QColor(128, 128, 128);
            }
            color_map[itor.key()] = color;
        }

        Q_ASSERT(!color_map.isEmpty());
    }

    return color_map.value(kingdom);
}

QMap<QString, QColor> Engine::getSkillColorMap() const {
    static QMap<QString, QColor> color_map;
    if (color_map.isEmpty()) {
        QVariantMap map = GetValueFromLuaState(lua, "config", "skill_colors").toMap();
        QMapIterator<QString, QVariant> itor(map);
        while (itor.hasNext()) {
            itor.next();
            QColor color(itor.value().toString());
            if (!color.isValid()) {
                qWarning("Invalid color for skill %s", qPrintable(itor.key()));
                color = QColor(128, 128, 128);
            }
            color_map[itor.key()] = color;
        }

        Q_ASSERT(!color_map.isEmpty());
    }

    return color_map;
}

QColor Engine::getSkillColor(const QString &skill_type) const {
    return Engine::getSkillColorMap().value(skill_type);
}

QStringList Engine::getChattingEasyTexts() const{
    static QStringList easy_texts;
    if (easy_texts.isEmpty())
        easy_texts = GetConfigFromLuaState(lua, "easy_text").toStringList();

    return easy_texts;
}

QString Engine::getSetupString() const{
    int timeout = Config.OperationNoLimit ? 0 : Config.OperationTimeout;
    QString flags;
    if (Config.RandomSeat)
        flags.append("R");
    if (Config.EnableCheat)
        flags.append("C");
    if (Config.EnableCheat && Config.FreeChoose)
        flags.append("F");
    if (Config.ForbidAddingRobot)
        flags.append("A");
    if (Config.DisableChat)
        flags.append("M");
    if (Config.RewardTheFirstShowingPlayer)
        flags.append("S");

    QString server_name = Config.ServerName;
    QStringList setup_items;
    setup_items << server_name
        << Config.GameMode
        << QString::number(timeout)
        << QString::number(Config.NullificationCountDown)
        << Sanguosha->getBanPackages().join("+")
        << flags;

    return setup_items.join(":");
}

QMap<QString, QString> Engine::getAvailableModes() const{
    return modes;
}

QString Engine::getModeName(const QString &mode) const{
    if (modes.contains(mode))
        return modes.value(mode);
    else
        return tr("%1 [Scenario mode]").arg(translate(mode));
}

int Engine::getPlayerCount(const QString &mode) const{
    if (modes.contains(mode)) {
        QRegExp rx("(\\d+)");
        int index = rx.indexIn(mode);
        if (index != -1)
            return rx.capturedTexts().first().toInt();
    }
    else {
        // scenario mode
        const Scenario *scenario = getScenario(mode);
        Q_ASSERT(scenario);
        return scenario->getPlayerCount();
    }

    return -1;
}

QString Engine::getRoles(const QString &mode) const{
    int n = getPlayerCount(mode);

    if (modes.contains(mode)) {
        static const char *table[] = {
            "",
            "",

            "ZN", // 2
            "ZNN", // 3
            "ZNNN", // 4
            "ZNNNN", // 5
            "ZNNNNN", // 6
            "ZNNNNNN", // 7
            "ZNNNNNNN", // 8
            "ZNNNNNNNN", // 9
            "ZNNNNNNNNN" // 10
        };

        QString rolechar = table[n];

        return rolechar;
    } else {
        const Scenario *scenario = getScenario(mode);
        if (scenario)
            return scenario->getRoles();
    }
    return QString();
}

QStringList Engine::getRoleList(const QString &mode) const{
    QString roles = getRoles(mode);

    QStringList role_list;
    for (int i = 0; roles[i] != '\0'; i++) {
        QString role;
        switch (roles[i].toLatin1()) {
        case 'Z': role = "lord"; break;
        case 'C': role = "loyalist"; break;
        case 'N': role = "renegade"; break;
        case 'F': role = "rebel"; break;
        }
        role_list << role;
    }

    return role_list;
}

int Engine::getCardCount() const{
    return cards.length();
}

QStringList Engine::getGeneralNames() const{
    QStringList generalNames;
    foreach (const General *general, generalList) {
        generalNames << general->objectName();
    }
    return generalNames;
}

GeneralList Engine::getGeneralList() const{
     return generalList;
}

QStringList Engine::getLimitedGeneralNames() const{
    //for later use
    QStringList general_names = getGeneralNames();
    QStringList general_names_copy = general_names;

    foreach(QString name, general_names_copy) {
        if (isGeneralHidden(name) || getBanPackages().contains(getGeneral(name)->getPackage()))
            general_names.removeOne(name);
    }

    QStringList banned_generals = Config.value("Banlist/Generals", "").toStringList();
    foreach(QString banned, banned_generals){
        general_names.removeOne(banned);
    }

    return general_names;
}

QStringList Engine::getRandomGenerals(int count, const QSet<QString> &ban_set) const{
    QStringList all_generals = getLimitedGeneralNames();
    QSet<QString> general_set = all_generals.toSet();

    count = qMin(count, all_generals.count());
    Q_ASSERT(all_generals.count() >= count);

    all_generals = general_set.subtract(ban_set).toList();

    // shuffle them
    qShuffle(all_generals);

    QStringList general_list = all_generals.mid(0, count);
    Q_ASSERT(general_list.count() == count);

    return general_list;
}

QList<int> Engine::getRandomCards() const{
    QList<int> list;
    foreach(Card *card, cards) {
        card->clearFlags();

        if (!getBanPackages().contains(card->getPackage()))
            list << card->getId();
    }

    QStringList card_conversions = Config.value("CardConversions").toStringList();
    foreach(QString str, card_conversions) {
        if (str == "DragonPhoenix")
            list.removeOne(55);
        else
            list.removeOne(108);
    }

    qShuffle(list);

    return list;
}

QString Engine::getRandomGeneralName() const{
    const General *general = generalList.at(qrand() % generalList.size());
    while (general->getKingdom() == "programmer")
        general = generalList.at(qrand() % generalList.size());
    return general->objectName();
}

void Engine::playSystemAudioEffect(const QString &name) const{
    playAudioEffect(QString("audio/system/%1.ogg").arg(name));
}

void Engine::playAudioEffect(const QString &filename) const{
#ifdef AUDIO_SUPPORT
    if (!Config.EnableEffects)
        return;
    if (filename.isNull())
        return;

    Audio::play(filename);
#endif
}

void Engine::playSkillAudioEffect(const QString &skill_name, int index) const{
    const Skill *skill = skills.value(skill_name, NULL);
    if (skill)
        skill->playAudioEffect(index);
}

const Skill *Engine::getSkill(const QString &skill_name) const{
    return skills.value(skill_name, NULL);
}

const Skill *Engine::getSkill(const EquipCard *equip) const{
    const Skill *skill;
    if (equip == NULL)
        skill = NULL;
    else
        skill = Sanguosha->getSkill(equip->objectName());

    return skill;
}

QStringList Engine::getSkillNames() const{
    return skills.keys();
}

const TriggerSkill *Engine::getTriggerSkill(const QString &skill_name) const{
    const Skill *skill = getSkill(skill_name);
    if (skill)
        return qobject_cast<const TriggerSkill *>(skill);
    else
        return NULL;
}

const ViewAsSkill *Engine::getViewAsSkill(const QString &skill_name) const{
    const Skill *skill = getSkill(skill_name);
    if (skill == NULL)
        return NULL;

    if (skill->inherits("ViewAsSkill"))
        return qobject_cast<const ViewAsSkill *>(skill);
    else if (skill->inherits("TriggerSkill")) {
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        return trigger_skill->getViewAsSkill();
    }
    else
        return NULL;
}

const ProhibitSkill *Engine::isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others) const{
    foreach(const ProhibitSkill *skill, prohibit_skills) {
        if (skill->isProhibited(from, to, card, others))
            return skill;
    }

    return NULL;
}

int Engine::correctDistance(const Player *from, const Player *to) const{
    int correct = 0;

    foreach(const DistanceSkill *skill, distance_skills) {
        correct += skill->getCorrect(from, to);
    }

    return correct;
}

int Engine::correctMaxCards(const ServerPlayer *target, bool fixed, MaxCardsType::MaxCardsCount type) const{
    int extra = 0;

    foreach (const MaxCardsSkill *skill, maxcards_skills) {
        if (fixed) {
            int f = skill->getFixed(target, type);
            if (f > extra)
                extra = f;
        } else {
            extra += skill->getExtra(target, type);
        }
    }

    return extra;
}

int Engine::correctCardTarget(const TargetModSkill::ModType type, const Player *from, const Card *card) const{
    int x = 0;

    if (type == TargetModSkill::Residue) {
        foreach(const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card)) {
                int residue = skill->getResidueNum(from, card);
                if (residue >= 998) return residue;
                x += residue;
            }
        }
    }
    else if (type == TargetModSkill::DistanceLimit) {
        foreach(const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card)) {
                int distance_limit = skill->getDistanceLimit(from, card);
                if (distance_limit >= 998) return distance_limit;
                x += distance_limit;
            }
        }
    }
    else if (type == TargetModSkill::ExtraTarget) {
        foreach(const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card)) {
                x += skill->getExtraTargetNum(from, card);
            }
        }
    }

    return x;
}


int Engine::correctAttackRange(const Player *target, bool include_weapon, bool fixed) const{
    int extra = 0;

    foreach(const AttackRangeSkill *skill, attackrange_skills) {
        if (fixed) {
            int f = skill->getFixed(target, include_weapon);
            if (f > extra)
                extra = f;
        }
        else {
            extra += skill->getExtra(target, include_weapon);
        }
    }

    return extra;
}
