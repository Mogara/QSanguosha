
#include "zombie-mode-scenario.h"
#include "engine.h"
#include "standard-skillcards.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"

class ZombieRule: public ScenarioRule{
public:
    ZombieRule(Scenario *scenario)
        :ScenarioRule(scenario)
    {
        events << GameStart << Death << GameOverJudge << TurnStart;
    }

    void zombify(ServerPlayer *player, ServerPlayer *killer = NULL) const{
        Room *room = player->getRoom();
        room->setPlayerProperty(player, "general2", "zombie");
        room->getThread()->addPlayerSkills(player, true);

        int maxhp = killer ? (killer->getMaxHP() + 1)/2 : 5;
        room->setPlayerProperty(player, "maxhp", maxhp);
        room->setPlayerProperty(player, "hp", player->getMaxHP());
        room->setPlayerProperty(player, "role", "rebel");
        player->loseSkill("peaching");

        if((player->getState() == "online" || player->getState() == "trust") && (maxhp<5))
            player->setState("robot");

        QString gender = player->getGeneral()->isMale() ? "male" : "female";
        room->broadcastInvoke("playAudio", QString("zombify-%1").arg(gender));
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const{
        Room *room = player->getRoom();

        switch(event){
        case GameStart:{
                room->acquireSkill(player, "peaching");
                break;
            }

        case GameOverJudge:{
                QStringList roles = room->aliveRoles(player);
                foreach(QString role, roles){
                    if(role == "loyalist" || role == "lord")
                        return true;
                }

                break;
            }

        case Death:{
            if(player->isLord()){
                foreach(ServerPlayer *p, room->getAlivePlayers()) {
                    if(p->getRoleEnum()==Player::Loyalist){
                        room->setPlayerProperty(player, "role", "loyalist");
                        room->setPlayerProperty(p, "role", "lord");

                        break;
                    }
                }
            }

            DamageStar damage = data.value<DamageStar>();
            if(damage && damage->from){
                ServerPlayer *killer = damage->from;

                if(killer == player)
                    return false;

                if(killer->getRole() == "loyalist" && player->getRole() == "rebel"){
                    RecoverStruct recover;
                    recover.who = killer;
                    recover.recover = killer->getLostHp();
                    room->recover(killer, recover);

                    return false;
                }

                if(killer->getRole() == "rebel" && player->getRole() != "rebel"){
                    zombify(player, killer);
                    player->getRoom()->revivePlayer(player);

                    player->throwAllCards();
                    player->drawCards(3);

                    return false;
                }
            }

            break;
        }

        case TurnStart:{                
                int round = room->getTag("Round").toInt();
                if(player->isLord()){
                    room->setTag("Round", ++round);

                    if(round > 9)
                        room->gameOver("lord+loyalist");
                    else if(round == 2){
                        QList<ServerPlayer *> players = room->getOtherPlayers(player);
                        qShuffle(players);

                        zombify(players.at(0));
                        room->getThread()->delay();
                        room->setTag("SurpriseZombie", players.at(1)->objectName());
                    }
                }else if(round==2 && player->objectName().compare(room->getTag("SurpriseZombie").toString())==0)
                {
                    zombify(player);
                    room->setTag("SurpriseZombie",NULL);
                    room->getThread()->delay();
                }
            }

        default:
            break;
        }

        return false;
    }
};

bool ZombieScenario::exposeRoles() const{
    return true;
}

void ZombieScenario::assign(QStringList &generals, QStringList &roles) const{
    Q_UNUSED(generals);

    roles << "lord";
    int i;
    for(i=0; i<7; i++)
        roles << "loyalist";

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

class Zaibian: public DrawCardsSkill{
public:
    Zaibian():DrawCardsSkill("zaibian"){
        frequency = Compulsory;
    }

    int getNumDiff(ServerPlayer *zombie) const{
        int human = 0, zombies = 0;
        foreach(ServerPlayer *player, zombie->getRoom()->getAlivePlayers()){
            switch(player->getRoleEnum()){
            case Player::Lord:
            case Player::Loyalist: human ++; break;
            case Player::Rebel: zombies ++; break;
            default:
                break;
            }
        }

        int x = human - zombies + 1;
        if(x < 0)
            return 0;
        else
            return x;
    }

    virtual int getDrawNum(ServerPlayer *zombie, int n) const{
        int x = getNumDiff(zombie);
        if(x > 0){
            Room *room = zombie->getRoom();
            LogMessage log;
            log.type = "#ZaibianGood";
            log.from = zombie;
            log.arg = QString::number(x);
            room->sendLog(log);
        }

        return n + x;
    }
};

class Xunmeng: public TriggerSkill{
public:
    Xunmeng():TriggerSkill("xunmeng"){
        events << Predamage;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *zombie, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();

        const Card *reason = damage.card;
        if(reason == NULL)
            return false;

        if(reason->inherits("Slash")){
            LogMessage log;
            log.type = "#Xunmeng";
            log.from = zombie;
            log.to << damage.to;
            log.arg = QString::number(damage.damage);
            log.arg2 = QString::number(damage.damage + 1);
            zombie->getRoom()->sendLog(log);

            if(zombie->getHp()>2)zombie->getRoom()->loseHp(zombie);
            damage.damage ++;
            data = QVariant::fromValue(damage);
        }

        return false;
    }
};

PeachingCard::PeachingCard()
    :QingnangCard()
{

}

bool PeachingCard::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    if(targets.length() > 0)return false;
    return to_select->isWounded() && (Self->distanceTo(to_select) <= 1);
}

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

GanranEquip::GanranEquip(Card::Suit suit, int number)
    :IronChain(suit, number)
{

}

bool GanranEquip::targetFilter(const QList<const ClientPlayer *> &targets, const ClientPlayer *to_select) const{
    return false;
}

class Ganran: public FilterSkill{
public:
    Ganran():FilterSkill("ganran"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->getTypeId() == Card::Equip;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        GanranEquip *ironchain = new GanranEquip(card->getSuit(), card->getNumber());
        ironchain->addSubcard(card_item->getCard()->getId());
        ironchain->setSkillName(objectName());

        return ironchain;
    }
};

ZombieScenario::ZombieScenario()
    :Scenario("zombie_m")
{
    rule = new ZombieRule(this);

    skills<< new Peaching;

    General *zombie = new General(this,"zombie","zombies",3, true, true);
    zombie->addSkill(new Xunmeng);
    zombie->addSkill(new Skill("yicong", Skill::Compulsory));
    zombie->addSkill(new Skill("paoxiao"));
    zombie->addSkill(new Ganran);
    zombie->addSkill(new Zaibian);
    zombie->addSkill(new Skill("wansha", Skill::Compulsory));

    addMetaObject<PeachingCard>();
}

ADD_SCENARIO(Zombie)
