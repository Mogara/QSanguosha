
#include "zombie-mode-scenario.h"
#include "engine.h"
#include "standard-skillcards.h"
#include "zombie-package.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"


class Peaching: public OneCardViewAsSkill{
public:
    Peaching():OneCardViewAsSkill("peaching"){

    }

    virtual bool isEnabledAtPlay() const{
        return true;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("Peach");
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        PeachingCard *qingnang_card = new PeachingCard;
        qingnang_card->addSubcard(card_item->getCard()->getId());

        return qingnang_card;
    }
};

class ZombieRule: public ScenarioRule{
public:

    ZombieRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death << GameOverJudge << TurnStart;



    }

    bool zombify(ServerPlayer *player, Room *room,ServerPlayer *killer=NULL) const{
        player->setGeneral2Name(player->getGeneralName());
        player->sendProperty("general2");
        room->broadcastProperty(player, "general2");

        QList<const TriggerSkill *> skills = player->getTriggerSkills();
        room->transfigure(player,"zombie",true);

//        QVariant void_data;

//        foreach(const TriggerSkill *skill, skills){
//            if(skill->isLordSkill()){
//                    continue;
//            }
//            room->getThread()->addTriggerSkill(skill);

//            if(skill->getTriggerEvents().contains(GameStart))
//                skill->trigger(GameStart, player, void_data);
//        }

        if(killer)
        {
            int hp=killer->getMaxHP();
            if(hp%2)hp++;
            player->setMaxHP(hp/2);
            room->broadcastProperty(player, "maxhp");
        }else
        {
            player->setMaxHP(5);
            room->broadcastProperty(player, "maxhp");
        }
        player->setHp(player->getMaxHP());
        room->broadcastProperty(player, "hp");
        return false;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        switch(event)
        {
        case GameStart:{
            Room *room = player->getRoom();

            QVariant data = QVariant::fromValue(0);

            player->getRoom()->setTag("rounds",data);

            if(player->getRole() == "rebel" )
            {
                data = player->getRoom()->getTag("MotherZombie");

                QString name=data.toString();

                data = QVariant::fromValue(player->objectName());

                if(name.length()>0)player->getRoom()->setTag("FatherZombie",data);
                else player->getRoom()->setTag("MotherZombie",data);
                //zombify(player,room);

                player->setProperty("role","loyalist");
                player->sendProperty("role");

            }else
            {
                room->acquireSkill(player, new Peaching);
            }
            break;
        }
        case GameOverJudge:{

                Room *room = player->getRoom();
                QStringList roles = room->aliveRoles(player);
                for(int i=0;i<roles.length();i++){
                    if(roles.at(i)=="loyalist" || roles.at(i)=="lord"){
                        return true;
                    }
                }

            break;
        }
        case Death:{
            if(player->isLord())
            {
                foreach(ServerPlayer *p,player->getRoom()->getAlivePlayers())
                {
                    if(p->getRoleEnum()==Player::Loyalist)
                    {
                        player->setProperty("role","loyalist");
                        player->sendProperty("role");
                        player->getRoom()->broadcastProperty(player, "role");

                        p->setProperty("role","lord");
                        p->sendProperty("role");
                        p->getRoom()->broadcastProperty(player, "role");
                        break;
                    }
                }
            }
            DamageStar damage = data.value<DamageStar>();
                if(damage && damage->from){
                    ServerPlayer *killer = damage->from;

                    if(killer == player)
                        return false;

                    if(player->getRoleEnum()==Player::Rebel)
                    {
                        if(!(killer->getRoleEnum()==Player::Rebel))killer->setHp(killer->getMaxHP());
                        return false;
                    }

                    if(killer->getGeneralName()=="zombie")
                    {
                        player->getRoom()->revivePlayer(player);
                        zombify(player,player->getRoom(),killer);
                        player->setProperty("role","rebel");
                        player->sendProperty("role");
                        player->getRoom()->broadcastProperty(player, "role");
                        player->throwAllHandCards();
                        player->throwAllEquips();
                        player->drawCards(3);
                        return true;
                    }
                    killer->throwAllHandCards();
                    killer->throwAllEquips();
                    return true;
                }

            break;

        }
        case TurnStart:{

            QVariant data = player->getRoom()->getTag("rounds");
            int rc=data.toInt();

            if(player->isLord())
            {
                if(rc>9)player->getRoom()->gameOver("lord+loyalist");
                else {
                    rc++;
                    data=QVariant::fromValue(rc);
                    player->getRoom()->setTag("rounds",data);
                }
            }

            data=player->getRoom()->getTag("MotherZombie");
            QString name1=data.toString();
            data=player->getRoom()->getTag("FatherZombie");
            QString name2=data.toString();

            if(player->objectName().compare(&name1)==0 || player->objectName().compare(&name2)==0)
            {
                if(rc>1 &&!(player->getGeneralName()=="zombie"))
                {
                    player->setProperty("role","rebel");
                    player->sendProperty("role");
                    player->getRoom()->broadcastProperty(player, "role");
                    zombify(player,player->getRoom());
                }

            }
        }
        default:break;
        }
        return false;
    }
};

ZombieScenario::ZombieScenario()
    :Scenario("zombie_m")
{
    females << "zhenji" << "huangyueying" << "zhurong"
            << "daqiao" << "luxun" << "xiaoqiao"
            << "diaochan" << "caizhaoji" << "sunshangxiang";

    standard_roles << "lord" << "loyalist" << "loyalist"
            << "loyalist" << "loyalist" << "loyalist" << "rebel" << "rebel";

    rule = new ZombieRule(this);

    skills<< new Peaching;


}

bool ZombieScenario::exposeRoles() const{
    return false;
}

void ZombieScenario::assign(QStringList &generals, QStringList &roles) const{
//    generals = females;
//    qShuffle(generals);
//    generals.removeLast();

    roles = standard_roles;
    qShuffle(roles);
}

int ZombieScenario::getPlayerCount() const{
    return 8;
}

void ZombieScenario::getRoles(char *roles) const{
    strcpy(roles, "ZCCCCCCC");
}

void ZombieScenario::onTagSet(Room *room, const QString &key) const{
    // dummy
}

ADD_SCENARIO(Zombie)
