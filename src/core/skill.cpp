#include "skill.h"
#include "settings.h"
#include "engine.h"
#include "player.h"
#include "room.h"
#include "client.h"
#include "standard.h"
#include "scenario.h"

#include <QFile>

Skill::Skill(const QString &name, Frequency frequency)
    :frequency(frequency), default_choice("no"), attached_lord_skill(false)
{
    static QChar lord_symbol('$');

    if(name.endsWith(lord_symbol)){
        QString copy = name;
        copy.remove(lord_symbol);
        setObjectName(copy);
        lord_skill = true;
    }else{
        setObjectName(name);
        lord_skill = false;
    }
}

bool Skill::isLordSkill() const{
    return lord_skill;
}

bool Skill::isAttachedLordSkill() const{
    return attached_lord_skill;
}

QString Skill::getDescription() const{
    QString des_src = Sanguosha->translate(":" + objectName());
    if(des_src == ":" + objectName())
        return QString();
    return des_src;
}

QString Skill::getNotice(int index) const{
    if(index == -1)
        return Sanguosha->translate("~" + objectName());

    return Sanguosha->translate(QString("~%1%2").arg(objectName()).arg(index));
}

bool Skill::isVisible() const{
    return !objectName().startsWith("#");
}

QString Skill::getDefaultChoice(ServerPlayer *) const{
    return default_choice;
}

int Skill::getEffectIndex(const ServerPlayer *, const Card *) const{
    return -1;
}

void Skill::initMediaSource(){
    sources.clear();
    for (int i = 1; ;i++) {
        QString effect_file = QString("audio/skill/%1%2.ogg").arg(objectName()).arg(QString::number(i));
        if(QFile::exists(effect_file))
            sources << effect_file;
        else
            break;
    }

    if(sources.isEmpty()){
        QString effect_file = QString("audio/skill/%1.ogg").arg(objectName());
        if(QFile::exists(effect_file))
            sources << effect_file;
    }
}

Skill::Location Skill::getLocation() const{
    return parent() ? Right : Left;
}

void Skill::playAudioEffect(int index) const{
    if(!sources.isEmpty()){
        if(index == -1)
            index = qrand() % sources.length();
        else
            index--;

        // check length
        QString filename;
        if(index >= 0 && index < sources.length())
            filename = sources.at(index);
        else if (index >= sources.length()) {
            while (index >= sources.length()) {
                index -= sources.length();
            }
            filename = sources.at(index);
        }
        else
            filename = sources.first();

        Sanguosha->playAudioEffect(filename);
        if(ClientInstance)
            ClientInstance->setLines(filename);
    }
}

void Skill::setFlag(ServerPlayer *player) const{
    player->getRoom()->setPlayerFlag(player, objectName());
}

void Skill::unsetFlag(ServerPlayer *player) const{
    player->getRoom()->setPlayerFlag(player, "-" + objectName());
}

Skill::Frequency Skill::getFrequency() const{
    return frequency;
}

QStringList Skill::getSources() const{
    return sources;
}

QDialog *Skill::getDialog() const{
    return NULL;
}

ViewAsSkill::ViewAsSkill(const QString &name)
    :Skill(name)
{

}

bool ViewAsSkill::isAvailable(const Player* invoker,
                              CardUseStruct::CardUseReason reason, 
                              const QString &pattern) const{
    if (!invoker->hasSkill(objectName()))
        return false;
    switch (reason) {
    case CardUseStruct::CARD_USE_REASON_PLAY: return isEnabledAtPlay(invoker);
    case CardUseStruct::CARD_USE_REASON_RESPONSE: return isEnabledAtResponse(invoker, pattern);
    default:
        return false;
    }
}

bool ViewAsSkill::isEnabledAtPlay(const Player *) const{
    return true;
}

bool ViewAsSkill::isEnabledAtResponse(const Player *, const QString &) const{
    return false;
}

bool ViewAsSkill::isEnabledAtNullification(const ServerPlayer *) const{
    return false;
}

const ViewAsSkill* ViewAsSkill::parseViewAsSkill(const Skill *skill)
{
    if (skill == NULL) return NULL;
    if (skill->inherits("ViewAsSkill"))
    {
        const ViewAsSkill *view_as_skill = qobject_cast<const ViewAsSkill *>(skill);
        return view_as_skill;
    }
    if (skill->inherits("TriggerSkill")) {
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        Q_ASSERT(trigger_skill != NULL);
        const ViewAsSkill *view_as_skill = trigger_skill->getViewAsSkill();
        if (view_as_skill != NULL) return view_as_skill;
    }
    return NULL;
}

ZeroCardViewAsSkill::ZeroCardViewAsSkill(const QString &name)
    :ViewAsSkill(name)
{

}

const Card *ZeroCardViewAsSkill::viewAs(const QList<const Card *> &cards) const{
    if(cards.isEmpty())
        return viewAs();
    else
        return NULL;
}

bool ZeroCardViewAsSkill::viewFilter(const QList<const Card *> &, const Card *) const{
    return false;
}

OneCardViewAsSkill::OneCardViewAsSkill(const QString &name)
    :ViewAsSkill(name)
{

}

bool OneCardViewAsSkill::viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
    return selected.isEmpty() && viewFilter(to_select) && !to_select->hasFlag("using");
}

const Card *OneCardViewAsSkill::viewAs(const QList<const Card *> &cards) const{
    if(cards.length() != 1)
        return NULL;
    else
        return viewAs(cards.first());
}

FilterSkill::FilterSkill(const QString &name)
    :OneCardViewAsSkill(name)
{
    frequency = Compulsory;
}

TriggerSkill::TriggerSkill(const QString &name)
    :Skill(name), view_as_skill(NULL)
{

}

const ViewAsSkill *TriggerSkill::getViewAsSkill() const{
    return view_as_skill;
}

QList<TriggerEvent> TriggerSkill::getTriggerEvents() const{
    return events;
}

int TriggerSkill::getPriority() const{
    switch(frequency){
    case Wake: return 3;
    default:
        return 2;
    }
}

bool TriggerSkill::triggerable(const ServerPlayer *target) const{
    return target != NULL && target->isAlive() && target->hasSkill(objectName());
}

ScenarioRule::ScenarioRule(Scenario *scenario)
    :TriggerSkill(scenario->objectName())
{
    setParent(scenario);
}

int ScenarioRule::getPriority() const{
    return 0;
}

bool ScenarioRule::triggerable(const ServerPlayer *) const{
    return true;
}

MasochismSkill::MasochismSkill(const QString &name)
    :TriggerSkill(name)
{
    events << Damaged;
}

bool MasochismSkill::trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
    DamageStruct damage = data.value<DamageStruct>();

    if(player->isAlive())
        onDamaged(player, damage);

    return false;
}

PhaseChangeSkill::PhaseChangeSkill(const QString &name)
    :TriggerSkill(name)
{
    events << EventPhaseStart;
}

bool PhaseChangeSkill::trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
    return onPhaseChange(player);
}

DrawCardsSkill::DrawCardsSkill(const QString &name)
    :TriggerSkill(name)
{
    events << DrawNCards;
}

bool DrawCardsSkill::trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
    int n = data.toInt();
    data = getDrawNum(player, n);
    return false;
}

GameStartSkill::GameStartSkill(const QString &name)
    :TriggerSkill(name)
{
    events << GameStart;
}

bool GameStartSkill::trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &) const{
    onGameStart(player);
    return false;
}

SPConvertSkill::SPConvertSkill(const QString &from, const QString &to)
    : GameStartSkill(QString("cv_%1").arg(from)), from(from), to(to)
{
    to_list = to.split("+");
}

bool SPConvertSkill::triggerable(const ServerPlayer *target) const{
    if (target == NULL) return false;
    if (!Config.value("EnableSPConvert", true).toBool()) return false;
    bool canInvoke = ServerInfo.GameMode.endsWith("p") || ServerInfo.GameMode.endsWith("pd")
                     || ServerInfo.GameMode.endsWith("pz");
    if (!canInvoke) return false;
    bool available = false;
    foreach (QString to_gen, to_list) {
        const General *gen = Sanguosha->getGeneral(to_gen);
        if (gen && !Config.value("Ban/Roles", "").toStringList().contains(to_gen)
            && !Sanguosha->getBanPackages().contains(gen->getPackage())) {
            available = true;
            break;
        }
    }
    return GameStartSkill::triggerable(target) && target->getGeneralName() == from && available && canInvoke;
}

void SPConvertSkill::onGameStart(ServerPlayer *player) const{
    QVariant data = "convert";
    if (player->askForSkillInvoke(objectName(), data)) {
        Room *room = player->getRoom();
        QStringList choicelist;
        foreach (QString to_gen, to_list) {
            const General *gen = Sanguosha->getGeneral(to_gen);
            if (gen && !Config.value("Ban/Roles", "").toStringList().contains(to_gen)
                    && !Sanguosha->getBanPackages().contains(gen->getPackage()))
                choicelist << to_gen;
        }
        QString to_cv = room->askForChoice(player, objectName(), choicelist.join("+"));

        LogMessage log;
        log.type = "#Transfigure";
        log.from = player;
        log.arg = to_cv;
        room->sendLog(log);
        room->setPlayerProperty(player, "general", to_cv);

        const General *general = Sanguosha->getGeneral(to_cv);
        const QString kingdom = general->getKingdom();
        if (kingdom != player->getKingdom())
            room->setPlayerProperty(player, "kingdom", kingdom);
    }
}

ProhibitSkill::ProhibitSkill(const QString &name)
    :Skill(name, Skill::Compulsory)
{
}

DistanceSkill::DistanceSkill(const QString &name)
    :Skill(name, Skill::Compulsory)
{
}

MaxCardsSkill::MaxCardsSkill(const QString &name)
    :Skill(name, Skill::Compulsory)
{
}

TargetModSkill::TargetModSkill(const QString &name)
    : Skill(name, Skill::Compulsory), pattern("Slash")
{
}

QString TargetModSkill::getPattern() const{
    return pattern;
}

int TargetModSkill::getResidueNum(const Player *, const Card *) const{
    return 0;
}

int TargetModSkill::getDistanceLimit(const Player *, const Card *) const{
    return 0;
}

int TargetModSkill::getExtraTargetNum(const Player *, const Card *) const{
    return 0;
}

SlashNoDistanceLimitSkill::SlashNoDistanceLimitSkill(const QString &skill_name)
    : TargetModSkill(QString("#%1-slash-ndl").arg(skill_name)), name(skill_name)
{
}

int SlashNoDistanceLimitSkill::getDistanceLimit(const Player *from, const Card *card) const{
    if (from->hasSkill(name) && card->getSkillName() == name)
        return 1000;
    else
        return 0;
}

FakeMoveSkill::FakeMoveSkill(const QString &name, FakeCondition condition)
    : TriggerSkill(QString("#%1-fake-move").arg(name)), name(name), condition(condition)
{
    events << CardsMoving << CardsMoveOneTime;
}

int FakeMoveSkill::getPriority() const{
    return 10;
}

bool FakeMoveSkill::triggerable(const ServerPlayer *target) const{
    return target != NULL;
}

bool FakeMoveSkill::trigger(TriggerEvent, Room *room, ServerPlayer *player, QVariant &) const{
    QString flag = QString("%1_InTempMoving").arg(name);
    if (condition == Global) {
        foreach (ServerPlayer *p, room->getAllPlayers())
            if (p->hasFlag(flag)) return true;
    } else if (condition == SourceOnly) {
        if (player->hasFlag(flag)) return true;
    }
    return false;
}

WeaponSkill::WeaponSkill(const QString &name)
    :TriggerSkill(name)
{
}

bool WeaponSkill::triggerable(const ServerPlayer *target) const{
    if (target == NULL) return false;
    return target->hasWeapon(objectName());
}

ArmorSkill::ArmorSkill(const QString &name)
    :TriggerSkill(name)
{

}

bool ArmorSkill::triggerable(const ServerPlayer *target) const{
    if (target == NULL || target->getArmor() == NULL) return false;
    return target->hasArmorEffect(objectName());
}

MarkAssignSkill::MarkAssignSkill(const QString &mark, int n)
    :GameStartSkill(QString("#%1-%2").arg(mark).arg(n)), mark_name(mark), n(n)
{
}

void MarkAssignSkill::onGameStart(ServerPlayer *player) const{
    player->getRoom()->setPlayerMark(player, mark_name, player->getMark(mark_name) + n);
}
