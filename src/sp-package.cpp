#include "sp-package.h"
#include "general.h"
#include "skill.h"
#include "standard-skillcards.h"

class Yongsi: public TriggerSkill{
public:
    Yongsi():TriggerSkill("yongsi"){
        events << DrawNCards << PhaseChange;
        frequency = Compulsory;
    }

    int getKingdoms(ServerPlayer *yuanshu) const{
        QSet<QString> kingdom_set;
        Room *room = yuanshu->getRoom();
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            kingdom_set << p->getKingdom();
        }

        return kingdom_set.size();
    }

    virtual bool trigger(TriggerEvent event, ServerPlayer *yuanshu, QVariant &data) const{
        if(event == DrawNCards){
            int x = getKingdoms(yuanshu);
            data = data.toInt() + x;

            Room *room = yuanshu->getRoom();
            LogMessage log;
            log.type = "#YongsiGood";
            log.from = yuanshu;
            log.arg = QString::number(x);
            room->sendLog(log);

            room->playSkillEffect("yongsi", x);

        }else if(event == PhaseChange && yuanshu->getPhase() == Player::Discard){
            int x = getKingdoms(yuanshu);
            int total = yuanshu->getEquips().length() + yuanshu->getHandcardNum();
            Room *room = yuanshu->getRoom();

            if(total <= x){
                yuanshu->throwAllHandCards();
                yuanshu->throwAllEquips();

                LogMessage log;
                log.type = "#YongsiWorst";
                log.from = yuanshu;
                log.arg = QString::number(total);
                room->sendLog(log);

            }else{
                room->askForDiscard(yuanshu, "yongsi", x, false, true);

                LogMessage log;
                log.type = "#YongsiBad";
                log.from = yuanshu;
                log.arg = QString::number(x);
                room->sendLog(log);
            }
        }

        return false;
    }
};

class Weidi: public GameStartSkill{
public:
    Weidi():GameStartSkill("weidi"){
        frequency = Compulsory;
    }

    virtual void onGameStart(ServerPlayer *player) const{
        Room *room = player->getRoom();
        ServerPlayer *lord = room->getLord();
        if(lord != player){
            const General *general = lord->getGeneral();
            QList<const Skill *> skills = general->findChildren<const Skill *>();
            foreach(const Skill *skill, skills){
                if(skill->isLordSkill()){
                    room->acquireSkill(player, skill->objectName());
                    return;
                }
            }
        }
    }
};

class Yicong: public DistanceSkill{
public:
    Yicong():DistanceSkill("yicong"){

    }

    virtual int getCorrect(const Player *from, const Player *to) const{
        int correct = 0;
        if(from->hasSkill(objectName()) && from->getHp() > 2)
            correct --;
        if(to->hasSkill(objectName()) && to->getHp() <= 2)
            correct ++;

        return correct;
    }
};

class Xuwei: public ZeroCardViewAsSkill{
public:
    Xuwei():ZeroCardViewAsSkill("xuwei"){
        huanzhuang_card = new HuanzhuangCard;
        huanzhuang_card->setParent(this);
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        if(player->hasUsed("HuanzhuangCard"))
            return false;

        return player->getGeneralName() == "sp_diaochan";
    }

    virtual const Card *viewAs() const{
        return huanzhuang_card;
    }

private:
    HuanzhuangCard *huanzhuang_card;
};

SPPackage::SPPackage()
    :Package("sp")
{
    General *gongsunzan = new General(this, "gongsunzan", "qun");
    gongsunzan->addSkill(new Yicong);

    General *yuanshu = new General(this, "yuanshu", "qun");
    yuanshu->addSkill(new Yongsi);
    yuanshu->addSkill(new Weidi);

    General *sp_diaochan = new General(this, "sp_diaochan", "qun", 3, false, true);
    sp_diaochan->addSkill("lijian");
    sp_diaochan->addSkill("biyue");
    sp_diaochan->addSkill(new Xuwei);

    General *sp_sunshangxiang = new General(this, "sp_sunshangxiang", "shu", 3, false, true);
    sp_sunshangxiang->addSkill("jieyin");
    sp_sunshangxiang->addSkill("xiaoji");
}

ADD_PACKAGE(SP);
