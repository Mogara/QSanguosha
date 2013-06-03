#include "engine.h"
#include "card.h"
#include "client.h"
#include "ai.h"
#include "settings.h"
#include "scenario.h"
#include "lua.hpp"
#include "banpair.h"
#include "audio.h"
#include "protocol.h"
#include "jsonutils.h"
#include "structs.h"
#include "RoomState.h"

#include "guandu-scenario.h"
#include "couple-scenario.h"
#include "boss-mode-scenario.h"
#include "zombie-scenario.h"
#include "fancheng-scenario.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QApplication>
#include <scenario.h>
#include <miniscenarios.h>

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
    addScenario(new GuanduScenario());
    addScenario(new CoupleScenario());
    addScenario(new FanchengScenario());
    addScenario(new ZombieScenario());
    addScenario(new ImpasseScenario());
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

    QStringList package_names = GetConfigFromLuaState(lua, "package_names").toStringList();
    foreach (QString name, package_names)
        addPackage(name);

    _loadMiniScenarios();
    _loadModScenarios();
    m_customScene = new CustomScenario();

    DoLuaScript(lua, "lua/sanguosha.lua");

    // available game modes
    modes["02p"] = tr("2 players");
    //modes["02pbb"] = tr("2 players (using blance beam)");
    modes["02_1v1"] = tr("2 players (KOF style)");
    modes["03p"] = tr("3 players");
    modes["04p"] = tr("4 players");
    modes["04_1v3"] = tr("4 players (Hulao Pass)");
    modes["05p"] = tr("5 players");
    modes["06p"] = tr("6 players");
    modes["06pd"] = tr("6 players (2 renegades)");
    modes["06_3v3"] = tr("6 players (3v3)");
    modes["06_XMode"] = tr("6 players (XMode)");
    modes["07p"] = tr("7 players");
    modes["08p"] = tr("8 players");
    modes["08pd"] = tr("8 players (2 renegades)");
    modes["08pz"] = tr("8 players (0 renegade)");
    modes["09p"] = tr("9 players");
    modes["10pd"] = tr("10 players");
    modes["10p"] = tr("10 players (1 renegade)");
    modes["10pz"] = tr("10 players (0 renegade)");

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(deleteLater()));

    foreach (const Skill *skill, skills.values()) {
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
#ifdef AUDIO_SUPPORT
    Audio::quit();
#endif
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
    foreach (const Skill *skill, all_skills) {
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

void Engine::addPackage(Package *package) {
    if (findChild<const Package *>(package->objectName()))
        return;

    package->setParent(this);

    QList<Card *> all_cards = package->findChildren<Card *>();
    foreach (Card *card, all_cards) {
        card->setId(cards.length());
        cards << card;

        Q_ASSERT(card->metaObject() != NULL);
        QString class_name = card->metaObject()->className();
        metaobjects.insert(class_name, card->metaObject());
        className2objectName.insert(class_name, card->objectName());
    }

    addSkills(package->getSkills());

    QList<General *> all_generals = package->findChildren<General *>();
    foreach (General *general, all_generals) {
        addSkills(general->findChildren<const Skill *>());
        foreach (QString skill_name, general->getExtraSkillSet()) {
            if (skill_name.startsWith("#")) continue;
            foreach (const Skill *related, getRelatedSkills(skill_name))
                general->addSkill(related->objectName());
        }

        if ((general->isHidden() && !Config.value("EnableHidden", false).toBool())
            || (general->objectName() == "shenlvbu1" || general->objectName() == "shenlvbu2")) {
            hidden_generals.insert(general->objectName(), general);
            continue;
        }

        if (general->isLord())
            lord_list << general->objectName();
        else
            nonlord_list << general->objectName();

        generals.insert(general->objectName(), general);
    }

    QList<const QMetaObject *> metas = package->getMetaObjects();
    foreach (const QMetaObject *meta, metas)
        metaobjects.insert(meta->className(), meta);

    patterns.unite(package->getPatterns());
    related_skills.unite(package->getRelatedSkills());
}

void Engine::addBanPackage(const QString &package_name) {
    ban_package.insert(package_name);
}

QStringList Engine::getBanPackages() const{
    if (qApp->arguments().contains("-server"))
        return Config.BanPackages;
    else
        return ban_package.toList();
}

QString Engine::translate(const QString &to_translate) const{
    return translations.value(to_translate, to_translate);
}

int Engine::getRoleIndex() const{
    if (ServerInfo.GameMode == "06_3v3" || ServerInfo.GameMode == "06_XMode") {
        return 4;
    } else if (ServerInfo.EnableHegemony) {
        return 5;
    } else
        return 1;
}

const CardPattern *Engine::getPattern(const QString &name) const{
    const CardPattern *ptn = patterns.value(name, NULL);
    if (ptn) return ptn;

    ExpPattern *expptn = new ExpPattern(name);
    patterns.insert(name, expptn);
    return expptn;
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
    foreach (QString name, related_skills.values(skill_name))
        skills << getSkill(name);

    return skills;
}

const Skill *Engine::getMainSkill(const QString &skill_name) const{
    const Skill *skill = getSkill(skill_name);
    if (!skill || skill->isVisible() || related_skills.keys().contains(skill_name)) return skill;
    foreach (QString key, related_skills.keys()) {
        foreach (QString name, related_skills.values(key))
            if (name == skill_name) return getSkill(key);
    }
    return skill;
}

const General *Engine::getGeneral(const QString &name) const{
    if (generals.contains(name))
        return generals.value(name);
    else
        return hidden_generals.value(name, NULL);
}

int Engine::getGeneralCount(bool include_banned) const{
    if (include_banned)
        return generals.size();

    int total = generals.size();
    QHashIterator<QString, const General *> itor(generals);
    while (itor.hasNext()) {
        itor.next();
        const General *general = itor.value();
        if (getBanPackages().contains(general->getPackage()))
            total--;
        else if ((isNormalGameMode(ServerInfo.GameMode)
                  || ServerInfo.GameMode.contains("_mini_")
                  || ServerInfo.GameMode == "custom_scenario")
                 && Config.value("Banlist/Roles").toStringList().contains(general->objectName()))
            total--;
        else if (ServerInfo.GameMode == "04_1v3"
                 && Config.value("Banlist/HulaoPass").toStringList().contains(general->objectName()))
            total--;
        else if (ServerInfo.GameMode == "06_XMode"
                 && Config.value("Banlist/XMode").toStringList().contains(general->objectName()))
            total--;
        else if (ServerInfo.Enable2ndGeneral && BanPair::isBanned(general->objectName()))
            total--;
        else if (ServerInfo.EnableBasara
                 && Config.value("Banlist/Basara").toStringList().contains(general->objectName()))
            total--;
        else if (ServerInfo.EnableHegemony
                 && Config.value("Banlist/Hegemony").toStringList().contains(general->objectName()))
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
    } else {
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
    } else {
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
        Q_ASSERT(FALSE);
        return NULL;
    } else {
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
    const QMetaObject *meta = metaobjects.value(name, NULL);
    if (meta == NULL)
        meta = metaobjects.value(className2objectName.key(name, QString()), NULL);
    if (meta) {
        QObject *card_obj = meta->newInstance(Q_ARG(Card::Suit, suit), Q_ARG(int, number));
        card_obj->setObjectName(className2objectName.value(name, name));
        card = qobject_cast<Card *>(card_obj);
    }
    if (!card) return NULL;
    card->clearFlags();
    if (!flags.isEmpty()) {
        foreach (QString flag, flags)
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
    } else
        return NULL;
}

QString Engine::getVersionNumber() const{
    return "20130610";
}

QString Engine::getVersion() const{
    QString version_number = getVersionNumber();
    QString mod_name = getMODName();
    if(mod_name == "official")
        return version_number;
    else
        return QString("%1:%2").arg(version_number).arg(mod_name);
}

QString Engine::getVersionName() const{
    return "V2";
}

QString Engine::getMODName() const{
    return "official";
}

QStringList Engine::getExtensions() const{
    QStringList extensions;
    QList<const Package *> packages = findChildren<const Package *>();
    foreach (const Package *package, packages) {
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
        foreach (QString k, getKingdoms()) {
            QString color_str = GetConfigFromLuaState(lua, ("color_" + k).toAscii()).toString();
            QRegExp rx("#?([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})");
            if (rx.exactMatch(color_str)) {
                QStringList results = rx.capturedTexts();
                int red = results.at(1).toInt(NULL, 16);
                int green = results.at(2).toInt(NULL, 16);
                int blue = results.at(3).toInt(NULL, 16);

                color_map.insert(k, QColor(red, green, blue));
            }
        }

        Q_ASSERT(!color_map.isEmpty());
    }

    return color_map.value(kingdom);
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
    if (Config.Enable2ndGeneral)
        flags.append("S");
    if (Config.EnableScene)
        flags.append("N");
    if (Config.EnableSame)
        flags.append("T");
    if (Config.EnableBasara)
        flags.append("B");
    if (Config.EnableHegemony)
        flags.append("H");
    if (Config.EnableAI)
        flags.append("A");
    if (Config.DisableChat)
        flags.append("M");

    if (Config.MaxHpScheme == 1)
        flags.append("1");
    else if (Config.MaxHpScheme == 2)
        flags.append("2");
    else if (Config.MaxHpScheme == 3)
        flags.append("3");
    else if (Config.MaxHpScheme == 0) {
        char c = Config.Scheme0Subtraction + 5 + 'a'; // from -5 to 12
        flags.append(c);
    }

    QString server_name = Config.ServerName.toUtf8().toBase64();
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
    } else {
        // scenario mode
        const Scenario *scenario = getScenario(mode);
        Q_ASSERT(scenario);
        return scenario->getPlayerCount();
    }

    return -1;
}

QString Engine::getRoles(const QString &mode) const{
    int n = getPlayerCount(mode);

    if (mode == "02_1v1") {
        return "ZN";
    } else if (mode == "04_1v3") {
        return "ZFFF";
    }

    if (modes.contains(mode)) {
        static const char *table1[] = {
            "",
            "",

            "ZF", // 2
            "ZFN", // 3
            "ZNFF", // 4
            "ZCFFN", // 5
            "ZCFFFN", // 6
            "ZCCFFFN", // 7
            "ZCCFFFFN", // 8
            "ZCCCFFFFN", // 9
            "ZCCCFFFFFN" // 10
        };

        static const char *table2[] = {
            "",
            "",

            "ZF", // 2
            "ZFN", // 3
            "ZNFF", // 4
            "ZCFFN", // 5
            "ZCFFNN", // 6
            "ZCCFFFN", // 7
            "ZCCFFFNN", // 8
            "ZCCCFFFFN", // 9
            "ZCCCFFFFNN" // 10
        };

        const char **table = mode.endsWith("d") ? table2 : table1;
        QString rolechar = table[n];
        if (mode.endsWith("z"))
            rolechar.replace("N", "C");
        else if (Config.EnableHegemony) {
            rolechar.replace("F", "N");
            rolechar.replace("C", "N");
        }

        return rolechar;
    } else if (mode.startsWith("@")) {
        if (n == 8)
            return "ZCCCNFFF";
        else if (n == 6)
            return "ZCCNFF";
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
        switch(roles[i].toAscii()) {
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

QStringList Engine::getLords(bool contain_banned) const{
    QStringList lords;

    // add intrinsic lord
    foreach (QString lord, lord_list) {
        const General *general = generals.value(lord);
        if (getBanPackages().contains(general->getPackage()))
            continue;
        if (!contain_banned) {
            if (ServerInfo.GameMode.endsWith("p")
                || ServerInfo.GameMode.endsWith("pd")
                || ServerInfo.GameMode.endsWith("pz")
                || ServerInfo.GameMode.contains("_mini_")
                || ServerInfo.GameMode == "custom_scenario")
                if (Config.value("Banlist/Roles", "").toStringList().contains(lord))
                    continue;
            if (Config.Enable2ndGeneral && BanPair::isBanned(general->objectName()))
                continue;
        }
        lords << lord;
    }

    return lords;
}

QStringList Engine::getRandomLords() const{
    QStringList banlist_ban;
    if (Config.EnableBasara)
        banlist_ban = Config.value("Banlist/Basara").toStringList();

    if (Config.GameMode == "zombie_mode")
        banlist_ban.append(Config.value("Banlist/Zombie").toStringList());
    else if (isNormalGameMode(Config.GameMode))
        banlist_ban.append(Config.value("Banlist/Roles").toStringList());

    QStringList lords;

    foreach (QString alord, getLords()) {
        if (banlist_ban.contains(alord)) continue;
        lords << alord;
    }

    int lord_num = Config.value("LordMaxChoice", -1).toInt();
    if (lord_num != -1 && lord_num < lords.length()) {
        int to_remove = lords.length() - lord_num;
        for (int i = 0; i < to_remove; i++) {
            lords.removeAt(qrand() % lords.length());
        }
    }

    QStringList nonlord_list;
    foreach (QString nonlord, this->nonlord_list) {
        const General *general = generals.value(nonlord);
        if (getBanPackages().contains(general->getPackage()))
            continue;
        if (Config.Enable2ndGeneral && BanPair::isBanned(general->objectName()))
            continue;
        if (banlist_ban.contains(general->objectName()))
            continue;

        nonlord_list << nonlord;
    }

    qShuffle(nonlord_list);

    int i;
    int extra = Config.value("NonLordMaxChoice", 2).toInt();
    if (lord_num == 0 && extra == 0)
        extra = 1;
    for (i = 0; i < extra; i++) {
        lords << nonlord_list.at(i);
        if (i == nonlord_list.length() - 1) break;
    }

    return lords;
}

QStringList Engine::getLimitedGeneralNames() const{
    QStringList general_names;
    QHashIterator<QString, const General *> itor(generals);
    if (ServerInfo.GameMode == "04_1v3") {
        QList<const General *> hulao_generals = QList<const General *>();
        foreach (QString pack_name, GetConfigFromLuaState(lua, "hulao_packages").toStringList()) {
             const Package *pack = Sanguosha->findChild<const Package *>(pack_name);
             if (pack) hulao_generals << pack->findChildren<const General *>();
        }

        foreach (const General *general, hulao_generals) {
            if (general->isTotallyHidden())
                continue;
            general_names << general->objectName();
        }
    } else {
        while (itor.hasNext()) {
            itor.next();
            if (!getBanPackages().contains(itor.value()->getPackage()))
                general_names << itor.key();
        }
    }

    if (!getBanPackages().contains("sp") && !getBanPackages().contains("assassins")) {
        general_names.removeOne("liuxie");
        general_names.removeOne("lingju");
        general_names.removeOne("fuwan");
    }

    return general_names;
}

QStringList Engine::getRandomGenerals(int count, const QSet<QString> &ban_set) const{
    QStringList all_generals = getLimitedGeneralNames();
    QSet<QString> general_set = all_generals.toSet();

    Q_ASSERT(all_generals.count() >= count);

    if (Config.EnableBasara)
        general_set = general_set.subtract(Config.value("Banlist/Basara", "").toStringList().toSet());
    if (Config.EnableHegemony)
        general_set = general_set.subtract(Config.value("Banlist/Hegemony", "").toStringList().toSet());

    if (isNormalGameMode(ServerInfo.GameMode)
        || ServerInfo.GameMode.contains("_mini_")
        || ServerInfo.GameMode == "custom_scenario")
        general_set.subtract(Config.value("Banlist/Roles", "").toStringList().toSet());
    else if (ServerInfo.GameMode == "04_1v3")
        general_set.subtract(Config.value("Banlist/HulaoPass", "").toStringList().toSet());
    else if (ServerInfo.GameMode == "06_XMode")
        general_set.subtract(Config.value("Banlist/XMode", "").toStringList().toSet());

    all_generals = general_set.subtract(ban_set).toList();

    // shuffle them
    qShuffle(all_generals);

    QStringList general_list = all_generals.mid(0, count);
    Q_ASSERT(general_list.count() == count);

    return general_list;
}

QList<int> Engine::getRandomCards() const{
    bool exclude_disaters = false, using_2012_3v3 = false, using_2013_3v3 = false;

    if (Config.GameMode == "06_3v3") {
        using_2012_3v3 = (Config.value("3v3/OfficialRule", "2012").toString() == "2012");
        using_2013_3v3 = (Config.value("3v3/OfficialRule", "2012").toString() == "2013");
        exclude_disaters = !Config.value("3v3/UsingExtension", false).toBool() || Config.value("3v3/ExcludeDisasters", true).toBool();
    }

    if (Config.GameMode == "04_1v3")
        exclude_disaters = true;

    QList<int> list;
    foreach (Card *card, cards) {
        card->clearFlags();

        if (exclude_disaters && card->isKindOf("Disaster"))
            continue;

        if (card->getPackage() == "New3v3Card" && (using_2012_3v3 || using_2013_3v3))
            list << card->getId();
        else if (card->getPackage() == "New3v3_2013Card" && using_2013_3v3)
            list << card->getId();

        if (Config.GameMode == "02_1v1" && !Config.value("1v1/UsingCardExtension", false).toBool()) {
            if (card->getPackage() == "New1v1Card")
                list << card->getId();
            continue;
        }

        if (Config.GameMode == "06_3v3" && !Config.value("3v3/UsingExtension", false).toBool()
            && card->getPackage() != "standard_cards" && card->getPackage() != "standard_ex_cards")
            continue;
        if (!getBanPackages().contains(card->getPackage()))
            list << card->getId();
    }
    if (using_2012_3v3 || using_2013_3v3)
        list.removeOne(98);
    if (using_2013_3v3) {
        list.removeOne(53);
        list.removeOne(54);
    }

    qShuffle(list);

    return list;
}

QString Engine::getRandomGeneralName() const{
    return generals.keys().at(qrand() % generals.size());
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
    } else
        return NULL;
}

const ProhibitSkill *Engine::isProhibited(const Player *from, const Player *to, const Card *card, const QList<const Player *> &others) const{
    foreach (const ProhibitSkill *skill, prohibit_skills) {
        if (skill->isProhibited(from, to, card, others))
            return skill;
    }

    return NULL;
}

int Engine::correctDistance(const Player *from, const Player *to) const{
    int correct = 0;

    foreach (const DistanceSkill *skill, distance_skills) {
        correct += skill->getCorrect(from, to);
    }

    return correct;
}

int Engine::correctMaxCards(const Player *target) const{
    int extra = 0;

    foreach (const MaxCardsSkill *skill, maxcards_skills)
        extra += skill->getExtra(target);

    return extra;
}

int Engine::correctCardTarget(const TargetModSkill::ModType type, const Player *from, const Card *card) const{
    int x = 0;

    if (type == TargetModSkill::Residue) {
        foreach (const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card)) {
                int residue = skill->getResidueNum(from, card);
                if (residue >= 998) return residue;
                x += residue;
            }
        }
    } else if (type == TargetModSkill::DistanceLimit) {
        foreach (const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card)) {
                int distance_limit = skill->getDistanceLimit(from, card);
                if (distance_limit >= 998) return distance_limit;
                x += distance_limit;
            }
        }
    } else if (type == TargetModSkill::ExtraTarget) {
        foreach (const TargetModSkill *skill, targetmod_skills) {
            ExpPattern p(skill->getPattern());
            if (p.match(from, card)) {
                x += skill->getExtraTargetNum(from, card);
            }
        }
    }

    return x;
}

