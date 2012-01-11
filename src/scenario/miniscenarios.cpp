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
        events << GameStart;
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

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const
    {
        if(player->getRoom()->getTag("WaitForPlayer").toBool())
            return true;

        Room* room = player->getRoom();

        QList<ServerPlayer*> players = room->getAllPlayers();
        while(players.first()->getState() == "robot")
            players.append(players.takeFirst());

        int i=0;
        foreach(ServerPlayer * sp,players)
        {
            room->setPlayerProperty(sp,"role",this->players.at(i)["role"]);

            if(sp->getState()!= "robot")
            {
                QString general = this->players.at(i)["general"];
                {
                    if(general == "select")
                    {
                        QStringList available,all,existed;
                        assign(existed,all);
                        all = Sanguosha->getRandomGenerals(Sanguosha->getGeneralCount());
                        for(int i=0;i<5;i++)
                        {
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
                    QString trans = QString("%1:%2").arg(sp->getGeneralName()).arg(general);
                    sp->invoke("transfigure",trans);
                    room->setPlayerProperty(sp,"general",general);
                }
                general = this->players.at(i)["general2"];
                {
                    if(general == "select")
                    {
                        QStringList available,all,existed;
                        assign(existed,all);
                        all = Sanguosha->getRandomGenerals(Sanguosha->getGeneralCount());
                        for(int i=0;i<5;i++)
                        {
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

            QString str2 = this->players.at(i)["hp"];
            if(str2 != NULL)str = str2;
            room->setPlayerProperty(sp,"hp",str.toInt());

            str = this->players.at(i)["draw"];
            if(str == NULL)str = "4";
            room->drawCards(sp,str.toInt());

            str = this->players.at(i)["equip"];
            QStringList equips = str.split(",");
            foreach(QString equip,equips)
            {
                room->installEquip(sp,equip);
            }
            room->getThread()->addPlayerSkills(sp);

            if(this->players.at(i)["starter"] != NULL)
                room->setCurrent(sp);

            i++;
        }

        room->setTag("WaitForPlayer",QVariant(true));
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

private:
    QList< QMap<QString, QString> > players;
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
    arule->addNPC("general:caocao|role:lord|draw:2");
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
    arule->addNPC("general:chengong|general2:diaochan|maxhp:3|role:loyalist");
    arule->addNPC("general:lubu|general2:gaoshun|maxhp:5|role:lord|starter:true|equip:halberd,chitu");
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
    arule->addNPC("general:simayi|general:xunyu|maxhp:3|role:rebel");

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
