#include "miniscenarios.h"
#include "scenario.h"
#include "engine.h"
#include "room.h"

class MiniSceneRule : public ScenarioRule
{
public:
    MiniSceneRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << PhaseChange;
    }

    virtual void assign(QStringList &generals, QStringList &roles) const{
        for(int i=0;i<players.length();i++)
        {
            QMap<QString,QString> sp =players.at(i);
            QString name = sp["general"];
            if(name == "select")name = "sujiang";
            generals << name;
            roles << sp["role"];
        }
    }

    QStringList existedGenerals() const
    {
        QStringList names;
        for(int i=0;i<players.length();i++)
        {
            QMap<QString,QString> sp =players.at(i);
            QString name = sp["general"];
            if(name == "select")name = "sujiang";
            names << name;
            name = sp["general2"];
            if(name == NULL )continue;
            if(name == "select")name = "sujiang";
            names << name;
        }
        return names;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const
    {
        Room* room = player->getRoom();

        if(event == PhaseChange)
        {
            if(player->getPhase()==Player::Start && this->players.first()["beforeNext"] != NULL
                    &&player->getState() != "robot")
            {
                if(room->getTag("playerHasPlayed").toBool())
                room->gameOver(this->players.first()["beforeNext"]);
                else room->setTag("playerHasPlayed",true);
            }

            if(player->getPhase() != Player::NotActive)return false;
            if(player->getState() == "robot" || this->players.first()["singleTurn"] == NULL)
                return false;
            room->gameOver(this->players.first()["SingleTurn"]);
        }
        if(player->getRoom()->getTag("WaitForPlayer").toBool())
            return true;

        QList<ServerPlayer*> players = room->getAllPlayers();
        while(players.first()->getState() == "robot")
            players.append(players.takeFirst());

        QStringList cards= setup.split(",");
        foreach(QString id,cards)
        {
            room->moveCardTo(Sanguosha->getCard(id.toInt()),NULL,Player::Special,true);
            room->moveCardTo(Sanguosha->getCard(id.toInt()),NULL,Player::DrawPile,true);
            room->broadcastInvoke("addHistory","pushPile");
        }

        int i=0;
        foreach(ServerPlayer * sp,players)
        {
            room->setPlayerProperty(sp,"role",this->players.at(i)["role"]);

            if(sp->getState()!= "robot")
            {
                QString general = this->players.at(i)["general"];
                {
                    QString original = sp->getGeneralName();
                    if(general == "select")
                    {
                        QStringList available,all,existed;
                        existed = existedGenerals();
                        all = Sanguosha->getRandomGenerals(Sanguosha->getGeneralCount());
                        for(int i=0;i<5;i++)
                        {
                            sp->setGeneral(NULL);
                            QString choice = sp->findReasonable(all);
                            if(existed.contains(choice))
                            {
                                all.removeOne(choice);
                                i--;
                                continue;
                            }
                            available << choice;
                            all.removeOne(choice);
                        }
                        general = room->askForGeneral(sp,available);
                    }
                    QString trans = QString("%1:%2").arg(original).arg(general);
                    sp->invoke("transfigure",trans);
                    room->setPlayerProperty(sp,"general",general);
                }
                general = this->players.at(i)["general2"];
                {
                    if(general == "select")
                    {
                        QStringList available,all,existed;
                        existed = existedGenerals();
                        all = Sanguosha->getRandomGenerals(Sanguosha->getGeneralCount());
                        for(int i=0;i<5;i++)
                        {
                            room->setPlayerProperty(sp,"general2",NULL);
                            QString choice = sp->findReasonable(all);
                            if(existed.contains(choice))
                            {
                                all.removeOne(choice);
                                i--;
                                continue;
                            }
                            available << choice;
                            all.removeOne(choice);
                        }
                        general = room->askForGeneral(sp,available);
                    }
                    if(general == sp->getGeneralName())general = this->players.at(i)["general3"];
                    QString trans = QString("%1:%2").arg("sujiang").arg(general);
                    sp->invoke("transfigure",trans);
                    room->setPlayerProperty(sp,"general2",general);
                }
            }
            else
            {
                room->setPlayerProperty(sp,"general",this->players.at(i)["general"]);
                if(this->players.at(i)["general2"]!=NULL)room->setPlayerProperty(sp,"general2",this->players.at(i)["general2"]);
            }

            room->setPlayerProperty(sp,"kingdom",sp->getGeneral()->getKingdom());

            QString str = this->players.at(i)["maxhp"];
            if(str==NULL)str=QString::number(sp->getGeneralMaxHP());
            room->setPlayerProperty(sp,"maxhp",str.toInt());

            str = this->players.at(i)["hpadj"];
            if(str!=NULL)
                room->setPlayerProperty(sp,"maxhp",sp->getMaxHP()+str.toInt());
            str=QString::number(sp->getMaxHP());

            QString str2 = this->players.at(i)["hp"];
            if(str2 != NULL)str = str2;
            room->setPlayerProperty(sp,"hp",str.toInt());

            str = this->players.at(i)["equip"];
            QStringList equips = str.split(",");
            foreach(QString equip,equips)
            {
                room->installEquip(sp,equip);
            }

            str = this->players.at(i)["hand"];
            if(str !=NULL)
            {
                QStringList hands = str.split(",");
                foreach(QString hand,hands)
                {
                    room->obtainCard(sp,hand.toInt());
                }

            }

            QVariant v;
            foreach(const TriggerSkill *skill, sp->getTriggerSkills()){
                if(!skill->inherits("SPConvertSkill"))
                    room->getThread()->addTriggerSkill(skill);
                else continue;

                if(skill->getTriggerEvents().contains(GameStart))
                    skill->trigger(GameStart, sp, v);
            }

            if(this->players.at(i)["starter"] != NULL)
                room->setCurrent(sp);

            i++;
        }
        i =0;
        foreach(ServerPlayer *sp,players)
        {
            QString str = this->players.at(i)["draw"];
            if(str == NULL)str = "4";
            room->drawCards(sp,str.toInt());

            if(this->players.at(i)["marks"] != NULL)
            {
                QStringList marks = this->players.at(i)["marks"].split(",");
                foreach(QString qs,marks)
                {
                    QStringList keys = qs.split("*");
                    str = keys.at(1);
                    sp->gainMark(keys.at(0),str.toInt());
                }
            }

            i++;
        }

        room->setTag("WaitForPlayer",QVariant(true));
        room->updateStateItem();
        return true;
    }

    void addNPC(QString feature)
    {
        QMap<QString, QString> player;
        QStringList features = feature.split("|");
        foreach(QString str, features)
        {
            QStringList keys = str.split(":");
            player.insert(keys.at(0),keys.at(1));
        }

        players << player;
    }

    void setPile(QString cardList)
    {
        setup = cardList;
    }

private:
    QList< QMap<QString, QString> > players;
    QString setup;
};


MiniScene_01::MiniScene_01()
    :MiniScene("_mini_01")
{
    lord = "zhaoyun";
    rebels << "sujiang";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:rebel|starter:true");
    arule->addNPC("general:zhaoyun|general2:zhangfei|maxhp:5|hp:5|role:lord|draw:5|equip:qinggang_sword,zixing");

    rule =arule;
}

MiniScene_02::MiniScene_02()
    :MiniScene("_mini_02")
{
    lord = "sunce";
    rebels << "sujiang";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:rebel");
    arule->addNPC("general:sunce|maxhp:5|hp:1|role:lord|equip:guding_blade,hualiu|starter:true");

    rule =arule;
}

MiniScene_03::MiniScene_03()
    :MiniScene("_mini_03")
{
    lord = "caocao";
    rebels << "sujiang";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:rebel");
    arule->addNPC("general:caocao|maxhp:5|hp:5|role:lord|equip:yitian_sword|starter:true");

    rule =arule;
}


MiniScene_04::MiniScene_04()
    :MiniScene("_mini_04")
{
    lord = "guojia";
    rebels << "sujiang" << "yuanshao";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:rebel");
    arule->addNPC("general:yuanshao|role:rebel|starter:true");
    arule->addNPC("general:guojia|general2:huatuo|maxhp:3|role:lord|equip:eight_diagram");

    rule =arule;
}

MiniScene_05::MiniScene_05()
    :MiniScene("_mini_05")
{
    lord = "caocao";
    loyalists << "dianwei" << "xuchu";
    rebels << "machao" << "sujiang";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:rebel|equip:axe|starter:true");
    arule->addNPC("general:machao|role:rebel");
    arule->addNPC("general:dianwei|role:loyalist");
    arule->addNPC("general:caocao|role:lord|hp:4|draw:2");
    arule->addNPC("general:xuchu|role:loyalist");

    rule =arule;
}

MiniScene_06::MiniScene_06()
    :MiniScene("_mini_06")
{
    lord = "shenzhaoyun";
    rebels << "sujiang"<<"zhanghe";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:rebel|hp:2|draw:2|starter:true");
    arule->addNPC("general:zhanghe|hp:2|draw:2|role:rebel");
    arule->addNPC("general:shenzhaoyun|general2:luxun|maxhp:3|hp:1|role:lord|equip:silver_lion,qinggang_sword,dayuan,hualiu");

    rule =arule;
}

MiniScene_07::MiniScene_07()
    :MiniScene("_mini_07")
{
    lord = "wolong";
    loyalists << "sujiang";
    rebels << "yujin"<<"xiahouyuan"<<"panglingming";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|general2:select|role:loyalist|starter:true");
    arule->addNPC("general:wolong|general2:pangtong|maxhp:4|role:lord|equip:fan");
    arule->addNPC("general:yujin|role:rebel");
    arule->addNPC("general:xiahouyuan|role:rebel|equip:vine");
    arule->addNPC("general:panglingming|role:rebel|equip:vine,axe");

    rule =arule;
}

MiniScene_08::MiniScene_08()
    :MiniScene("_mini_08")
{
    lord = "lubu";
    loyalists << "chengong";
    rebels << "zhangfei"<<"liubei"<<"guanyu";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|general2:liubei|general3:gongsunzan|role:rebel");
    arule->addNPC("general:zhangfei|role:rebel|equip:spear");
    arule->addNPC("general:chengong|general2:diaochan|maxhp:3|role:loyalist|equip:hualiu|starter:true");
    arule->addNPC("general:lubu|general2:gaoshun|maxhp:5|draw:5|role:lord|equip:halberd,chitu");
    arule->addNPC("general:guanyu|role:rebel|equip:blade");

    rule =arule;
}

MiniScene_09::MiniScene_09()
    :MiniScene("_mini_09")
{
    lord = "sujiang";
    loyalists << "fazheng";
    rebels << "simayi" << "xiahoudun";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:lord|starter:true");
    arule->addNPC("general:xiahoudun|role:rebel");
    arule->addNPC("general:fazheng|general2:caiwenji|role:loyalist");
    arule->addNPC("general:simayi|general2:xunyu|maxhp:3|role:rebel");

    rule =arule;
}

MiniScene_10::MiniScene_10()
    :MiniScene("_mini_10")
{
    lord = "sujiang";
    loyalists << "zhenji";
    rebels << "daqiao" << "sunshangxiang" << "wuguotai";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|general2:select|role:lord|equip:double_sword|starter:true");
    arule->addNPC("general:daqiao|role:rebel");
    arule->addNPC("general:zhenji|role:loyalist");
    arule->addNPC("general:sp_sunshangxiang|maxhp:4|role:rebel");
    arule->addNPC("general:wuguotai|role:rebel");

    rule =arule;
}

MiniScene_11::MiniScene_11()
    :MiniScene("_mini_11")
{
    lord = "sujiang";
    loyalists << "sunshangxiang";
    rebels << "pangde" << "ganning" << "zhangliao";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|general2:select|role:lord|starter:true");
    arule->addNPC("general:pangde|equip:blade|role:rebel");
    arule->addNPC("general:ganning|general2:sunjian|maxhp:4|role:rebel|equip:guding_blade");
    arule->addNPC("general:zhangliao|general2:lukang|equip:ice_sword|role:rebel");
    arule->addNPC("general:sunshangxiang|general2:lingtong|maxhp:4|role:loyalist");

    rule =arule;
}

MiniScene_12::MiniScene_12()
    :MiniScene("_mini_12")
{
    lord = "caopi";
    loyalists << "caoren";
    rebels << "sujiang" << "xusheng";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:rebel|starter:true");
    arule->addNPC("general:xusheng|role:rebel");
    arule->addNPC("general:caopi|maxhp:4|role:lord");
    arule->addNPC("general:caoren|general2:caozhi|role:loyalist");

    rule =arule;
}

MiniScene_13::MiniScene_13()
    :MiniScene("_mini_13")
{
    lord = "liushan";
    loyalists << "sujiang" << "xushu";
    rebels << "jiaxu" << "lumeng" << "shensimayi";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:loyalist|equip:dilu");
    arule->addNPC("general:lumeng|role:rebel");
    arule->addNPC("general:jiaxu|general2:jiawenhe|maxhp:4|role:rebel|marks:@chaos*2");
    arule->addNPC("general:shensimayi|role:rebel");
    arule->addNPC("general:xushu|role:loyalist");
    arule->addNPC("general:liushan|role:lord|maxhp:3|starter:true");

    rule =arule;
}

MiniScene_14::MiniScene_14()
    :MiniScene("_mini_14")
{
    lord = "huanggai";
    rebels << "sujiang";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|general2:yangxiu|general3:zhouyu|role:rebel");
    arule->addNPC("general:huanggai|general2:huangyueying|maxhp:4|role:lord|equip:crossbow|starter:true");

    rule =arule;
}

MiniScene_15::MiniScene_15()
    :MiniScene("_mini_15")
{
    lord = "liushan";
    renegades << "sujiang";
    rebels << "zhurong";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|general2:select|role:renegade");
    arule->addNPC("general:liushan|general2:weiyan|maxhp:3|role:lord|starter:true");
    arule->addNPC("general:menghuo|general2:zhurong|maxhp:5|role:rebel");

    rule =arule;
}

MiniScene_16::MiniScene_16()
    :MiniScene("_mini_16")
{
    lord = "sujiang";
    loyalists << "shenguanyu";
    rebels << "taishici" << "shensimayi";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|general2:select|hpadj:1|role:lord|equip:zhuahuangfeidian");
    arule->addNPC("general:shensimayi|general2:yuanshu|maxhp:4|role:rebel");
    arule->addNPC("general:taishici|general2:erzhang|maxhp:4|role:rebel");
    arule->addNPC("general:shenguanyu|general2:shenlumeng|maxhp:5|role:renegade|starter:true");

    rule =arule;
}

MiniScene_17::MiniScene_17()
    :MiniScene("_mini_17")
{
    lord = "sujiang";
    loyalists << "jiangwei";
    rebels << "zhangjiao" << "simayi";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|general2:wuxingzhuge|general3:wolong|hpadj:1|role:lord|starter:true");
    arule->addNPC("general:simayi|general2:guojia|role:rebel|hand:103");
    arule->addNPC("general:zhangjiao|role:rebel|equip:eight_diagram");
    arule->addNPC("general:jiangwei|role:loyalist|equip:silver_lion");

    rule =arule;
}

MiniScene_18::MiniScene_18()
    :MiniScene("_mini_18")
{
    lord = "bgm_diaochan";
    loyalists << "huangzhong";
    rebels << "liubei" << "sunquan";
    renegades << "sujiang";

    MiniSceneRule *arule = new MiniSceneRule(this);
    arule->addNPC("general:select|role:renegade|starter:true");
    arule->addNPC("general:sunquan|general2:lusu|maxhp:3|role:rebel");
    arule->addNPC("general:liubei|general2:xunyu|maxhp:3|role:rebel");
    arule->addNPC("general:huangzhong|general2:zhoutai|maxhp:4|hp:2|role:loyalist");
    arule->addNPC("general:bgm_diaochan|general2:xiaoqiao|maxhp:3|role:lord");

    rule =arule;
}

MiniScene_19::MiniScene_19()
    :MiniScene("_mini_19")
{
    lord = "zhangjiao";
    loyalists << "luxun";
    renegades << "sujiang";

    MiniSceneRule *arule = new MiniSceneRule(this);
    QStringList player;
    player.append("general:select");
    player.append("role:renegade");
    player.append("maxhp:3");
    player.append("equip:silver_lion");
    player.append("draw:0");
    player.append("hand:135,136");
    player.append("starter:true");
    player.append("singleTurn:Lord+Loyalist");

    arule->addNPC(player.join("|"));

    player.clear();
    player.append("general:zhangjiao");
    player.append("role:lord");
    player.append("maxhp:4");
    player.append("hp:2");
    player.append("equip:eight_diagram");
    player.append("draw:0");
    player.append("hand:83,44");

    arule->addNPC(player.join("|"));

    player.clear();
    player.append("general:luxun");
    player.append("role:loyalist");
    player.append("maxhp:3");
    player.append("hp:2");
    player.append("equip:vine");
    player.append("draw:0");
    player.append("hand:52");

    arule->addNPC(player.join("|"));

    arule->setPile("62,31,101,131,90,2,31,18,103");

    rule =arule;
}

AI::Relation MiniScene_19::relationTo(const ServerPlayer *a, const ServerPlayer *b) const
{
    if(a->getRole() == "renegade" || b->getRole() == "renegade")return AI::Enemy;
    return AI::GetRelation(a, b);
}

MiniScene_20::MiniScene_20()
    :MiniScene("_mini_20")
{
    lord = "caocao";
    loyalists << "sujiang";
    rebels << "zhangjiao";

    MiniSceneRule *arule = new MiniSceneRule(this);
    QStringList player;
    player.append("general:select");
    player.append("general2:select");
    player.append("role:loyalist");
    player.append("hp:1");
    player.append("draw:0");
    player.append("hand:86,103");
    player.append("starter:true");
    player.append("beforeNext:Rebel");

    arule->addNPC(player.join("|"));

    player.clear();
    player.append("general:zhangjiao");
    player.append("general2:chengong");
    player.append("role:rebel");
    player.append("maxhp:3");
    player.append("equip:eight_diagram,blade");
    player.append("draw:0");
    player.append("hand:143,134,97,37");

    arule->addNPC(player.join("|"));

    player.clear();
    player.append("general:caocao");
    player.append("role:lord");
    player.append("maxhp:5");
    player.append("hp:2");
    player.append("equip:vine,guding_blade");
    player.append("draw:0");

    arule->addNPC(player.join("|"));

    arule->setPile("96,51,110,32,126,120");

    rule =arule;
}

void MiniScene::onTagSet(Room *room, const QString &key) const
{

}

#define ADD_MINISCENARIO(num) extern "C" { Q_DECL_EXPORT Scenario *NewMiniScene_##num() { return new MiniScene_##num; } }

ADD_MINISCENARIO(01);
ADD_MINISCENARIO(02);
ADD_MINISCENARIO(03);
ADD_MINISCENARIO(04);
ADD_MINISCENARIO(05);
ADD_MINISCENARIO(06);
ADD_MINISCENARIO(07);
ADD_MINISCENARIO(08);
ADD_MINISCENARIO(09);
ADD_MINISCENARIO(10);
ADD_MINISCENARIO(11);
ADD_MINISCENARIO(12);
ADD_MINISCENARIO(13);
ADD_MINISCENARIO(14);
ADD_MINISCENARIO(15);
ADD_MINISCENARIO(16);
ADD_MINISCENARIO(17);
ADD_MINISCENARIO(18);
ADD_MINISCENARIO(19);
ADD_MINISCENARIO(20);
