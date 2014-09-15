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

#ifndef _ENGINE_H
#define _ENGINE_H

#include "RoomState.h"
#include "card.h"
#include "general.h"
#include "skill.h"
#include "package.h"
#include "exppattern.h"
#include "util.h"
#include "version.h"
#include "aux-skills.h"

#include <QHash>
#include <QStringList>
#include <QMetaObject>
#include <QThread>
#include <QList>
#include <QMutex>

class AI;
class Scenario;
class LuaBasicCard;
class LuaTrickCard;
class LuaWeapon;
class LuaArmor;
class LuaTreasure;

struct lua_State;

class Engine : public QObject {
    Q_OBJECT

public:
    Engine();
    ~Engine();

    void addTranslationEntry(const char *key, const char *value);
    QString translate(const QString &toTranslate) const;
    QString translate(const QString &toTranslate, const QString &defaultValue) const;
    lua_State *getLuaState() const;

    int getMiniSceneCounts();

    void addPackage(Package *package);
    void addBanPackage(const QString &package_name);
    QList<const Package *> getPackages() const;

    QStringList getBanPackages() const;
    Card *cloneCard(const Card *card) const;
    Card *cloneCard(const QString &name, Card::Suit suit = Card::SuitToBeDecided, int number = -1, const QStringList &flags = QStringList()) const;
    SkillCard *cloneSkillCard(const QString &name) const;
    //************************************
    // Method:    getVersionNumber
    // FullName:  Engine::getVersionNumber
    // Access:    public
    // Returns:   QSanVersionNumber
    // Qualifier: const
    // Description: Get current version number.
    //
    // Last Updated By Fsu0413
    // To update version number
    //
    // QSanguosha-Rara
    // June 2 2014
    //************************************
    QSanVersionNumber getVersionNumber() const;
    QString getVersion() const;
    QString getVersionName() const;
    QString getMODName() const;
    QStringList getExtensions() const;
    QStringList getKingdoms() const;
    QColor getKingdomColor(const QString &kingdom) const;
    QMap<QString, QColor> getSkillColorMap() const;
    QColor getSkillColor(const QString &skill_type) const;
    QStringList getChattingEasyTexts() const;
    QString getSetupString() const;

    QMap<QString, QString> getAvailableModes() const;
    QString getModeName(const QString &mode) const;
    int getPlayerCount(const QString &mode) const;
    QString getRoles(const QString &mode) const;
    QStringList getRoleList(const QString &mode) const;

    const CardPattern *getPattern(const QString &name) const;
    bool matchExpPattern(const QString &pattern, const Player *player, const Card *card) const;
    Card::HandlingMethod getCardHandlingMethod(const QString &method_name) const;
    QList<const Skill *> getRelatedSkills(const QString &skill_name) const;
    const Skill *getMainSkill(const QString &skill_name) const;

    QStringList getModScenarioNames() const;
    void addScenario(Scenario *scenario);
    const Scenario *getScenario(const QString &name) const;
    void addPackage(const QString &name);

    const General *getGeneral(const QString &name) const;
    int getGeneralCount(bool include_banned = false) const;
    const Skill *getSkill(const QString &skill_name) const;
    const Skill *getSkill(const EquipCard *card) const;
    QStringList getSkillNames() const;
    const TriggerSkill *getTriggerSkill(const QString &skill_name) const;
    const ViewAsSkill *getViewAsSkill(const QString &skill_name) const;
    QList<const DistanceSkill *> getDistanceSkills() const;
    QList<const MaxCardsSkill *> getMaxCardsSkills() const;
    QList<const TargetModSkill *> getTargetModSkills() const;
    QList<const TriggerSkill *> getGlobalTriggerSkills() const;
    QList<const AttackRangeSkill *> getAttackRangeSkills() const;
    void addSkills(const QList<const Skill *> &skills);

    int getCardCount() const;
    const Card *getEngineCard(int cardId) const;
    // @todo: consider making this const Card *
    Card *getCard(int cardId);
    WrappedCard *getWrappedCard(int cardId);

    //************************************
    // Method:    getRandomGenerals
    // FullName:  Engine::getRandomGenerals
    // Access:    public
    // Returns:   QT_NAMESPACE::QStringList
    // Qualifier: const
    // Parameter: int count
    // Parameter: const QSet<QString> & ban_set
    // Description: Get [count] available generals and no one is in [ban_set].
    //
    // Last Updated By Yanguam Siliagim
    // To use a proper way to convert generals and cards
    //
    // QSanguosha-Rara
    // March 17 2014
    //************************************
    QStringList getRandomGenerals(int count, const QSet<QString> &ban_set = QSet<QString>()) const;
    //************************************
    // Method:    getRandomCards
    // FullName:  Engine::getRandomCards
    // Access:    public
    // Returns:   QList<int>
    // Qualifier: const
    // Description: Get IDs of all the available cards.
    //
    // Last Updated By Yanguam Siliagim
    // To use a proper way to convert generals and cards
    //
    // QSanguosha-Rara
    // March 17 2014
    //************************************
    QList<int> getRandomCards() const;
    QString getRandomGeneralName() const;
    //************************************
    // Method:    getLimitedGeneralNames
    // FullName:  Engine::getLimitedGeneralNames
    // Access:    public
    // Returns:   QT_NAMESPACE::QStringList
    // Qualifier: const
    // Description: It was designed to get the list of all available generals.
    //
    // Last Updated By Yanguam Siliagim
    // To use a proper way to convert generals and cards
    //
    // QSanguosha-Rara
    // March 17 2014
    //************************************
    QStringList getLimitedGeneralNames() const;
    QStringList getGeneralNames() const;
    GeneralList getGeneralList() const;

    void playSystemAudioEffect(const QString &name) const;
    void playAudioEffect(const QString &filename) const;
    void playSkillAudioEffect(const QString &skill_name, int index) const;

    const ProhibitSkill *isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others = QList<const Player *>()) const;
    int correctDistance(const Player *from, const Player *to) const;
    int correctMaxCards(const ServerPlayer *target, bool fixed = false, MaxCardsType::MaxCardsCount type = MaxCardsType::Max) const;
    int correctCardTarget(const TargetModSkill::ModType type, const Player *from, const Card *card) const;
    int correctAttackRange(const Player *target, bool include_weapon = true, bool fixed = false) const;

    void registerRoom(QObject *room);
    void unregisterRoom();
    QObject *currentRoomObject();
    Room *currentRoom();
    RoomState *currentRoomState();

    QString getCurrentCardUsePattern();
    CardUseStruct::CardUseReason getCurrentCardUseReason();

    bool isGeneralHidden(const QString &general_name) const;

    TransferSkill *getTransfer() const;

private:
    void _loadMiniScenarios();
    void _loadModScenarios();

    QMutex m_mutex;
    QHash<QString, QString> translations;
    GeneralList generalList;
    QHash<QString, const General *> generalHash;
    QHash<QString, const QMetaObject *> metaobjects;
    QHash<QString, QString> className2objectName;
    QHash<QString, const Skill *> skills;
    QHash<QThread *, QObject *> m_rooms;
    QMap<QString, QString> modes;
    QMultiMap<QString, QString> related_skills;
    mutable QMap<QString, const CardPattern *> patterns;
    mutable QList<ExpPattern *> enginePatterns;

    // special skills
    QList<const ProhibitSkill *> prohibit_skills;
    QList<const DistanceSkill *> distance_skills;
    QList<const MaxCardsSkill *> maxcards_skills;
    QList<const TargetModSkill *> targetmod_skills;
    QList<const AttackRangeSkill *> attackrange_skills;
    QList<const TriggerSkill *> global_trigger_skills;

    QList<const Package *> packages;
    QList<Card *> cards;
    QStringList lord_list;
    QSet<QString> ban_package;
    QHash<QString, Scenario *> m_scenarios;
    QHash<QString, Scenario *> m_miniScenes;
    Scenario *m_customScene;

    lua_State *lua;

    QHash<QString, QString> luaBasicCard_className2objectName;
    QHash<QString, const LuaBasicCard *> luaBasicCards;
    QHash<QString, QString> luaTrickCard_className2objectName;
    QHash<QString, const LuaTrickCard *> luaTrickCards;
    QHash<QString, QString> luaWeapon_className2objectName;
    QHash<QString, const LuaWeapon*> luaWeapons;
    QHash<QString, QString> luaArmor_className2objectName;
    QHash<QString, const LuaArmor *> luaArmors;
    QHash<QString, QString> luaTreasure_className2objectName;
    QHash<QString, const LuaTreasure *> luaTreasures;

    QMultiMap<QString, QString> sp_convert_pairs;

    TransferSkill *transfer;
};

static inline QVariant GetConfigFromLuaState(lua_State *L, const char *key) {
    return GetValueFromLuaState(L, "config", key);
}

extern Engine *Sanguosha;

#endif

