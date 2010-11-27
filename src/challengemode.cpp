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
    addMode("@mythology", "shencaocao+shenzhugeliang+shenzhouyu+shenlumeng");
    addMode("@2strike", "diaochan+zhouyu+dianwei+xiahouyuan");
    addMode("@tortoise", "liubei+huatuo+sunshangxiang+guojia");
    addMode("@manshow", "liubei+lumeng+zhangliao+daqiao");
    addMode("@greatrange", "machao+pangde+xiahouyuan+taishici");
    addMode("@p4cure", "liubei+menghuo+huatuo+weiyan");

    t["@4lords"] = tr("@4lords");
    t["@disaster4fr"] = tr("@disaster4fr");
    t["@p4luoyi"] = tr("@p4luoyi");
    t["@test4pile"] = tr("@test4pile");
    t["@mythology"] = tr("@mythology");
    t["@2strike"] = tr("@2strike");
    t["@tortoise"] = tr("@tortoise");
    t["@manshow"] = tr("@manshow");
    t["@greatrange"] = tr("@greatrange");
    t["@p4cure"] = tr("@p4cure");
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

