#include "zombie-mode-scenario.h"
#include "engine.h"
#include "standard-skillcards.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"
#include "general.h"

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
        room->getThread()->addPlayerSkills(player, false);

        int maxhp = killer ? (killer->getMaxHp() + 1)/2 : 5;
        room->setPlayerProperty(player, "maxhp", maxhp);
        room->setPlayerProperty(player, "hp", player->getMaxHp());
        room->setPlayerProperty(player, "role", "renegade");
        player->loseSkill("peaching");

        LogMessage log;
        log.type = "#Zombify";
        log.from = player;
        room->sendLog(log);

        QString gender = player->getGeneral()->isMale() ? "male" : "female";
        room->broadcastInvoke("playSystemAudioEffect", QString("zombify-%1").arg(gender));
        room->updateStateItem();

        player->tag.remove("zombie");
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        switch(event){
        case GameStart:{
            foreach (ServerPlayer* player, room->getPlayers())
            {
                room->acquireSkill(player, "peaching");
                break;
            }
            }

        case GameOverJudge:{
                return true;
                break;
            }

        case Death:{
            bool hasHuman=false;
            if(player->isLord()){
                foreach(ServerPlayer *p, room->getAlivePlayers()) {
                    if(p->getRoleEnum()==Player::Loyalist){
                        room->setPlayerProperty(player, "role", "loyalist");
                        room->setPlayerProperty(p, "role", "lord");
                        room->setPlayerProperty(p, "maxhp",  p->getMaxHp()+1);

                        RecoverStruct recover;
                        recover.who = p;
                        recover.recover = 1;
                        room->recover(p, recover);

                        p->gainMark("@round",player->getMark("@round")>1 ? player->getMark("@round")-1 : 1);
                        hasHuman=true;
                        break;
                    }
                }

            }else hasHuman=true;

            DamageStar damage = data.value<DamageStar>();
            if(damage && damage->from){
                ServerPlayer *killer = damage->from;

                if(player->getGeneral2Name()=="zombie"){
                    RecoverStruct recover;
                    recover.who = killer;
                    recover.recover = killer->getLostHp();
                    room->recover(killer, recover);
                    if(player->getRole()=="renegade")killer->drawCards(3);

                }

                else if(killer->getGeneral2Name()=="zombie"){
                    zombify(player, killer);
                    room->setPlayerProperty(player, "role", "renegade");
                    player->getRoom()->revivePlayer(player);
                    room->setPlayerProperty(killer,"role","rebel");

                }
            }

            if(!hasHuman)room->gameOver("rebel");

            break;
        }

        case TurnStart:{
                int round = room->getTag("Round").toInt();
                if(player->isLord()){
                    room->setTag("Round", ++round);
                    player->gainMark("@round");

                    QList<ServerPlayer *> players = room->getOtherPlayers(player);
                    qShuffle(players);

                    bool hasZombie=false;
                    foreach(ServerPlayer *p,players)
                    {
                        if (p->getGeneral2Name()=="zombie")
                        {
                            hasZombie=true;
                            break;
                        }
                    }

                    if(round>2&&!hasZombie)room->gameOver("lord+loyalist");

                    if(player->getMark("@round") > 7)
                    {
                        LogMessage log;
                        log.type = "#survive_victory";
                        log.from = player;
                        room->sendLog(log);

                        room->gameOver("lord+loyalist");
                    }
                    else if(round == 2){
                        players.at(0)->tag["zombie"]=true;
                        players.at(1)->tag["zombie"]=true;
                    }

                }else if(player->tag.contains("zombie"))
                {

                    player->bury();
                    room->killPlayer(player);
                    zombify(player);
                    room->setPlayerProperty(player,"role","rebel");
                    room->revivePlayer(player);
                    player->drawCards(5);
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

QString ZombieScenario::getRoles() const{
    return "ZCCCCCCC";
}

void ZombieScenario::onTagSet(Room *, const QString &) const{
    // dummy
}

bool ZombieScenario::generalSelection() const{
    return true;
}

AI::Relation ZombieScenario::relationTo(const ServerPlayer *a, const ServerPlayer *b) const{
    bool aZombie=true;
    bool bZombie=true;
    if(a->isLord() || a->getRoleEnum()==Player::Loyalist)aZombie=false;
    if(b->isLord() || b->getRoleEnum()==Player::Loyalist)bZombie=false;
    if(aZombie==bZombie)return AI::Friend;
    return AI::Enemy;
}

class Zaibian: public TriggerSkill{
public:
    Zaibian():TriggerSkill("zaibian"){
        events << EventPhaseStart ;
        frequency = Compulsory;
    }

    int getNumDiff(ServerPlayer *zombie) const{
        int human = 0, zombies = 0;
        foreach(ServerPlayer *player, zombie->getRoom()->getAlivePlayers()){
            switch(player->getRoleEnum()){
            case Player::Lord:
            case Player::Loyalist: human ++; break;
            case Player::Rebel: zombies ++; break;
                case Player::Renegade: zombies ++; break;
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

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *zombie, QVariant &) const{
        if(event == EventPhaseStart && zombie->getPhase() == Player::Play){
        int x = getNumDiff(zombie);
        if(x > 0){
            Room *room = zombie->getRoom();
            LogMessage log;
            log.type = "#ZaibianGood";
            log.from = zombie;
            log.arg = QString::number(x);
            log.arg2 = objectName();
            room->sendLog(log);
            zombie->drawCards(x);
        }

        }
        return false;
    }
};

class Xunmeng: public TriggerSkill{
public:
    Xunmeng():TriggerSkill("xunmeng"){
        events << Predamage;

        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *zombie, QVariant &data) const{
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

            if(zombie->getHp()>1)zombie->getRoom()->loseHp(zombie);

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

bool PeachingCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() > 0)return false;
    return to_select->isWounded() && (Self->distanceTo(to_select) <= 1);
}

class Peaching: public OneCardViewAsSkill{
public:
    Peaching():OneCardViewAsSkill("peaching"){

    }

    virtual bool isEnabledAtPlay(const Player *) const{
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
    :Scenario("zombie_mode")
{
    rule = new ZombieRule(this);

    skills<< new Peaching;

    General *zombie = new General(this, "zombie", "die", 3, true, true);
    zombie->addSkill(new Xunmeng);
    zombie->addSkill(new Ganran);
    zombie->addSkill(new Zaibian);

    zombie->addSkill("paoxiao");
    zombie->addSkill("wansha");

    addMetaObject<PeachingCard>();
    addMetaObject<GanranEquip>();
}

ADD_SCENARIO(Zombie)
