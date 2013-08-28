#include "formation.h"
#include "general.h"
#include "standard.h"
#include "standard-equips.h"
#include "skill.h"
#include "engine.h"
#include "client.h"
#include "serverplayer.h"
#include "room.h"
#include "ai.h"
#include "settings.h"

class Qianhuan: public TriggerSkill {
public:
	Qianhuan(): TriggerSkill("qianhuan") {
		events << Damaged << TargetConfirming;
	}
	
	virtual bool triggerable(const ServerPlayer *target) const{
		return target != NULL;
	}
	
	virtual bool trigger(TriggerEvent triggerEvent, Room *room, ServerPlayer *player, QVariant &data) const{
		if (triggerEvent == Damaged) {
			ServerPlayer *yuji = room->findPlayerBySkillName(objectName());
			if (yuji && room->askForSkillInvoke(player, objectName(), "choice:" + yuji->objectName())) {
				room->broadcastSkillInvoke(objectName());
				if (yuji != player) {
					room->notifySkillInvoked(yuji, objectName());
					LogMessage log;
					log.type = "#InvokeOthersSkill";
					log.from = player;
					log.to << yuji;
					log.arg = objectName();
					room->sendLog(log);
				}
				
				int id = room->drawCard();
				Card::Suit suit = Sanguosha->getCard(id)->getSuit();
				bool duplicate = false;
				foreach (int card_id, yuji->getPile("sorcery")) {
					if (Sanguosha->getCard(card_id)->getSuit() == suit) {
						duplicate = true;
						break;
					}
				}
				yuji->addToPile("sorcery", id);
				if (duplicate) {
					CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
					room->throwCard(Sanguosha->getCard(id), reason, NULL);
				}
			}
		} else if (triggerEvent == TargetConfirming) {
			CardUseStruct use = data.value<CardUseStruct>();
			if (use.to.length() != 1) return false;
			ServerPlayer *yuji = room->findPlayerBySkillName(objectName());
			if (!yuji || yuji->getPile("sorcery").isEmpty()) return false;
			if (room->askForSkillInvoke(yuji, objectName(), data)) {
				room->broadcastSkillInvoke(objectName());
				room->notifySkillInvoked(yuji, objectName());
				if (yuji == player || room->askForChoice(player, objectName(), "accept+reject") == "accept") {
					QList<int> ids = yuji->getPile("sorcery");
					int id = -1;
					if (ids.length() > 1) {
						room->fillAG(ids, yuji);
						id = room->askForAG(yuji, ids, false, objectName());
						room->clearAG(yuji);
					} else {
						id = ids.first();
					}
					CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), objectName(), QString());
					room->throwCard(Sanguosha->getCard(id), reason, NULL);
					
					use.to.clear();
					data = QVariant::fromValue(use);
				} else {
					LogMessage log;
					log.type = "#ZhibaReject";
					log.from = player;
					log.to << yuji;
					log.arg = objectName();
					room->sendLog(log);
				}
			}
		}
		return false;
	}
};

FormationPackage::FormationPackage()
	: Package("formation")
{
	General *heg_yuji = new General(this, "heg_yuji", "qun", 3);
	heg_yuji->addSkill(new Qianhuan);
}

ADD_PACKAGE(Formation)