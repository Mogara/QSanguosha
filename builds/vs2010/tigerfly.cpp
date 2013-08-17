#include "skill.h"
#include "clientplayer.h"
#include "standard.h"
#include "tigerfly.h"
#include "engine.h"

class Shemi: public ViewAsSkill {
public:
	Shemi():ViewAsSkill("shemi") {
	}

	virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const{
		int hp = Self->getHp();
		if (hp <= 2) {
			return selected.length() < 1;
		}else {
			return selected.length() < 2;
		};
    };

	virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("ShemiAG");
    };

    virtual const Card *viewAs(const QList<const Card *> &cards) const{
		int hp = Self->getHp();
		if ((hp <= 2 && cards.length() != 1) || (hp > 2 && cards.length() != 2))
			return NULL;

		Card *amazing_grace = new AmazingGrace(Card::SuitToBeDecided, -1);
		amazing_grace->addSubcards(cards);
		amazing_grace->setSkillName(objectName());
		Self->addHistory("ShemiAG");
		return amazing_grace;
    };

};

TigerFlyPackage::TigerFlyPackage(): Package("tigerfly") {
	General *caorui = new General(this, "caorui$", "wei", 3);
	caorui->addSkill(new Shemi);
};

ADD_PACKAGE(TigerFly)
