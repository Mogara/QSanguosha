#include "sp-package.h"
#include "general.h"
#include "skill.h"

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
            data = data.toInt() + getKingdoms(yuanshu);
            return false;
        }else if(event == PhaseChange && yuanshu->getPhase() == Player::Discard){
            int to_discard = yuanshu->getHandcardNum() - yuanshu->getMaxCards();
            int x = getKingdoms(yuanshu);
            if(to_discard < x){
                if(x >= yuanshu->getHandcardNum())
                    yuanshu->throwAllHandCards();
                else{
                    Room *room = yuanshu->getRoom();
                    room->askForDiscard(yuanshu, "yongsi", x);
                }

                return true;
            }else
                return false;
        }

        return false;
    }
};

SPPackage::SPPackage()
    :Package("sp")
{
    General *gongsunzan = new General(this, "gongsunzan", "qun");
    gongsunzan->addSkill(new Skill("yicong", Skill::Compulsory));

    General *yuanshu = new General(this, "yuanshu", "qun");
    yuanshu->addSkill(new Yongsi);
}

ADD_PACKAGE(SP);
