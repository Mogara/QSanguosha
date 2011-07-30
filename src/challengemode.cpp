#include "challengemode.h"

ChallengeMode::ChallengeMode(const QString &name, const QString &general_str)
{
    setObjectName(name);

    generals = general_str.split("+");
}

QStringList ChallengeMode::getGenerals() const{
    return generals;
}

ChallengeModeSet::ChallengeModeSet(QObject *parent)
    :Package("challenge_modes")
{
    setParent(parent);

    addMode("@4lords", "caocao+liubei+sunquan+zhangjiao");
    addMode("@disaster4fr", "guojia+simayi+wolong+luxun");
    addMode("@p4luoyi","xuchu+liubei+ganning+zhangliao");
    addMode("@test4pile", "sunshangxiang+huangyueying+luxun+zhenji");
    addMode("@mythology", "shencaocao+shenzhugeliang+shenzhouyu+shenlvmeng");
    addMode("@2strike", "diaochan+zhouyu+dianwei+xiahouyuan");
    addMode("@tortoise", "liubei+huatuo+sunshangxiang+guojia");
    addMode("@manshow", "liubei+lvmeng+zhangliao+daqiao");
    addMode("@greatrange", "machao+pangde+xiahouyuan+taishici");
    addMode("@p4cure", "liubei+menghuo+huatuo+weiyan");

    type = SpecialPack;
}

const ChallengeMode *ChallengeModeSet::getMode(const QString &name) const{
    return modes.value(name, NULL);
}

QList<const ChallengeMode *> ChallengeModeSet::allModes() const{
    return modes.values();
}

void ChallengeModeSet::addMode(const QString &name, const QString &general_str){
    ChallengeMode *mode = new ChallengeMode(name, general_str);
    modes[name] = mode;

    mode->setParent(this);
}

ChallengeModeRule::ChallengeModeRule(QObject *parent)
    :GameRule(parent)
{
    setObjectName("challenge_mode");
}

bool ChallengeModeRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
    if(event == Death){
        static const QString attack = "rebel+renegade";
        static const QString defense = "lord+loyalist";

        Room *room = player->getRoom();
        QStringList alive_roles = room->aliveRoles(player);
        QString victim_role = player->getRole();
        if(attack.contains(victim_role)){
            if(!alive_roles.contains("rebel") && !alive_roles.contains("renegade")){
                room->gameOver(defense);
                return true;
            }
        }else{
            if(!alive_roles.contains("lord") && !alive_roles.contains("loyalist")){
                room->gameOver(attack);
                return true;
            }
        }

        return false;
    }

    return GameRule::trigger(event, player, data);
}

