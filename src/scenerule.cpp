#include "engine.h"
#include "standard-skillcards.h"
#include "clientplayer.h"
#include "client.h"
#include "carditem.h"
#include "scenerule.h"

#include <QTime>

class SceneDistanceEffect : public DistanceSkill {
public:
	SceneDistanceEffect(const QString &name) : DistanceSkill(name) { }

	virtual int getCorrect(const Player *from, const Player *to) const {
		const ServerPlayer *svFrom = qobject_cast<const ServerPlayer *>(from);
		const ServerPlayer *svTo = qobject_cast<const ServerPlayer *>(to);
		if(!svFrom || !svTo)
			return 0;

		switch(svFrom->getRoom()->getTag("SceneID").toInt()) {
		case 11:
			return -1;

		case 12:
			return 1;

		case 13:
			if(svTo->isKongcheng()) {
				int right = qAbs(from->getSeat() - to->getSeat());
				int left = from->aliveCount() - right;
				return 1 - qMin(left, right);
			}
			break;
		}
		return 0;
	}
};

class Scene26Effect : public TriggerSkill {
public:
	Scene26Effect(const QString &name) : TriggerSkill(name) { events << PhaseChange; }

	virtual bool triggerable(const ServerPlayer *target) const {
		Q_UNUSED(target);

		return true;
	}

	virtual int getPriority() const { return 3; }

	virtual bool trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const {
		Q_UNUSED(data);

		Room *room = player->getRoom();

		if(room->getTag("SceneID").toInt() != 26)
			return false;

		if(player == room->getCurrent() && event == PhaseChange) {
			if(player->getPhase() == Player::Start) {
				room->getThread()->delay();
				if(room->getTag("SceneTurnLeft").toInt() != 4) player->skip(Player::Judge);
				if(room->getTag("SceneTurnLeft").toInt() != 3) player->skip(Player::Draw);
				if(room->getTag("SceneTurnLeft").toInt() != 2) player->skip(Player::Play);
				if(room->getTag("SceneTurnLeft").toInt() != 1) player->skip(Player::Discard);
				if(room->getTag("SceneTurnLeft").toInt() != 0) player->skip(Player::Finish);
				if(room->getTag("SceneTurnLeft").toInt() != 5) return true;
			}
		}

		return false;
	}
};

class Scene27Card : public SkillCard {
	Q_OBJECT

public:
	virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const {
		if(!targets.isEmpty())
			return false;
		if(to_select->isAllNude())
			return false;
		return true;
	}

	virtual void onEffect(const CardEffectStruct &effect) const {
		if(effect.to->isAllNude())
			return;

		Room *room = effect.to->getRoom();
		int card_id = room->askForCardChosen(effect.from, effect.to, "hej", "scene_27_effect");

		if(room->getCardPlace(card_id) == Player::Hand)
			room->moveCardTo(Sanguosha->getCard(card_id), effect.from, Player::Hand, false);
		else
			room->obtainCard(effect.from, card_id);
	}
};

class Scene27Skill : public OneCardViewAsSkill {
	Scene27Skill():OneCardViewAsSkill("liangshangjunzi") { }

	virtual bool viewFilter(const CardItem *to_select) const{
		return to_select->getFilteredCard()->inherits("Dismantlement") || to_select->getFilteredCard()->inherits("Snatch");
	}

	virtual const Card *viewAs(CardItem *card_item) const{
		const Card *first = card_item->getCard();
		Scene27Card *skill_card = new Scene27Card();
		skill_card->addSubcard(first->getId());
		skill_card->setSkillName("scene_27_effect");
		return skill_card;
	}
};

SceneRule::SceneRule(QObject *parent) : GameRule(parent) {
	qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

	events << Predamaged << Damaged;

	if(!Sanguosha->getSkill("#scene_dst_effect")) {
		QList<const Skill *> skillList;
		skillList << new SceneDistanceEffect("#scene_dst_effect");
		Sanguosha->addSkills(skillList);
	}
}

bool SceneRule::trigger(TriggerEvent event, ServerPlayer *player, QVariant &data) const {
	Room *room = player->getRoom();

	switch(event) {
	case GameStart:
		if(player->isLord()) {
			room->setTag("SceneID", 0);
			room->setTag("SceneTurnLeft", 0);
			room->setTag("NextSceneID", 0);
			room->getThread()->addTriggerSkill(new Scene26Effect("#scene_26_effect"));
		}
		break;

	case TurnStart:
		if(player->isLord()) {
			if(room->getTag("SceneTurnLeft").toInt() > 0) {
				room->setTag("SceneTurnLeft", room->getTag("SceneTurnLeft").toInt() - 1);
			} else {
				LogMessage logMsg;
				if(room->getTag("SceneID").toInt()) {
					logMsg.type = "#SceneFinished";
					logMsg.arg = QString("Scene%1").arg(room->getTag("SceneID").toInt());
					room->sendLog(logMsg);
				}

				switch(room->getTag("SceneID").toInt()) {
				case 10:
				{
					QList<DummyCard *> cardToMove;
					foreach(ServerPlayer *p, room->getAlivePlayers()) {
						if(p->isKongcheng())
							cardToMove << NULL;
						else
							cardToMove << p->wholeHandCards();
					}

					QList<ServerPlayer *> prevp = room->getAlivePlayers();
					prevp.prepend(prevp.takeLast());
					for(int i = 0; i < prevp.count(); i++) {
						if(cardToMove[i]) {
							room->moveCardTo(cardToMove[i], prevp[i], Player::Hand, false);
							room->getThread()->delay();
						}
					}

					foreach(DummyCard *p, cardToMove)
						if(p)
							delete p;
					break;
				}

				}

				int nextSceneID = qrand() % 32 + 1;
				qsrand(QTime(0, 0).secsTo(QTime::currentTime()));
				room->setTag("SceneID", room->getTag("NextSceneID").toInt());
				room->setTag("NextSceneID", nextSceneID);

				logMsg.type = "#SceneChanged";
				logMsg.arg = QString("Scene%1").arg(room->getTag("SceneID").toInt());
				logMsg.arg2 = QString("Scene%1Effect").arg(room->getTag("SceneID").toInt());
				room->sendLog(logMsg);

				logMsg.type = "#BroadcastNextScene";
				logMsg.arg = QString("Scene%1").arg(room->getTag("NextSceneID").toInt());
				logMsg.arg2 = QString("Scene%1Effect").arg(room->getTag("NextSceneID").toInt());
				room->sendLog(logMsg);

				switch(room->getTag("SceneID").toInt()) {
				case 1:
					foreach(ServerPlayer *p, room->getAlivePlayers()) {
						int numToDraw = p->getHandcardNum();
						p->throwAllHandCards();
						p->drawCards(numToDraw);
					}
					break;

				case 4:
					foreach(ServerPlayer *p, room->getAlivePlayers()) {
						if(!p->isChained())
							room->setPlayerProperty(p, "chained", true);
					}
					break;

				case 8:
					foreach(ServerPlayer *p, room->getAlivePlayers()) {
						room->showAllCards(room->askForPlayerChosen(p, room->getOtherPlayers(p), "Scene8"), p);
					}
					break;

				case 10:
				{
					QList<DummyCard *> cardToMove;
					foreach(ServerPlayer *p, room->getAlivePlayers()) {
						if(p->isKongcheng())
							cardToMove << NULL;
						else
							cardToMove << p->wholeHandCards();
					}

					QList<ServerPlayer *> prevp = room->getAlivePlayers();
					prevp.append(prevp.takeFirst());
					for(int i = 0; i < prevp.count(); i++) {
						if(cardToMove[i]) {
							room->moveCardTo(cardToMove[i], prevp[i], Player::Hand, false);
							room->getThread()->delay();
						}
					}

					foreach(DummyCard *p, cardToMove)
						if(p)
							delete p;
					break;
				}

				case 18:
					foreach(ServerPlayer *p, room->getAlivePlayers()) {
						if(!p->isKongcheng())
							room->askForDiscard(p, "", 1);
					}
					break;

				case 23:
				/*
					foreach(ServerPlayer *p, room->getAlivePlayers()) {
						const Card *card1;
						if(p->isKongcheng())
							continue;
						card1 = room->askForCardShow(p, p, "Scene23");
						if(card1) {
							ServerPlayer *askp = p->getNextAlive();
							bool cardShowed = false;
							while(askp != p) {
								if(askp->isKongcheng()) {
									askp = askp->getNextAlive();
									continue;
								}
								const Card *card2 = room->askForCard(askp, card1->objectName(), "Scene23", false);
								if(card2) {
									room->showCard(askp, card2->getId());
									cardShowed = true;
									break;
								}
								askp = askp->getNextAlive();
							}
							if(!cardShowed)
								p->drawCards(1);
						}
					}
				*/
					break;

				case 24:
				{
					QList<const Card *> judgeCards;
					foreach(ServerPlayer *p, room->getOtherPlayers(player)) {
						JudgeStruct judge;
						judge.who = p;
						judge.pattern = QRegExp("(.*):(.*):(.*)");
						judge.good = true;

						room->judge(judge);
						judgeCards.append(judge.card);
					}

					QList<ServerPlayer *> players = room->getOtherPlayers(player);
					for(int i = 0; i < players.length() - 1; i++) {
						int maxCardPlayer = i;

						for(int j = i + 1; j < players.length(); j++)
							if(judgeCards.at(j)->getNumber() > judgeCards.at(maxCardPlayer)->getNumber())
								maxCardPlayer = j;

						if(maxCardPlayer != i) {
							room->swapSeat(players.at(maxCardPlayer), players.at(i));
							judgeCards.swap(maxCardPlayer, i);
						}
					}
					break;
				}

				case 26:
					room->setTag("SceneTurnLeft", 5);
					break;

				case 29:
					foreach(ServerPlayer *p, room->getAlivePlayers()) {
						ServerPlayer *nextAlivePlayer = p->getNextAlive();
						if(!p->isKongcheng() && !nextAlivePlayer->isKongcheng()) {
							if(p->pindian(nextAlivePlayer, "Scene29")) {
								if(room->askForChoice(p, "scene_29_eff", "dscorlose+recover") == "recover") {
									RecoverStruct recover;
									recover.who = p;
									room->recover(nextAlivePlayer, recover);
								} else {
									if(nextAlivePlayer->isKongcheng())
										room->loseHp(nextAlivePlayer);
									else
										room->askForDiscard(nextAlivePlayer, "Scene29", 1);
								}
							} else {
								if(room->askForChoice(nextAlivePlayer, "scene_29_eff", "dscorlose+recover") == "recover")
								{
									RecoverStruct recover;
									recover.who = nextAlivePlayer;
									room->recover(p, recover);
								} else {
									if(p->isKongcheng())
										room->loseHp(p);
									else
										room->askForDiscard(p, "Scene29", 1);
								}
							}
						}
					}
					break;

				case 32:
					room->setTag("SceneTurnLeft", 1);
					break;
				}
			}
		}

		switch(room->getTag("SceneID").toInt()) {
		}

		break;

	case PhaseChange:
		switch(room->getTag("SceneID").toInt()) {
		case 6:
			if(player->getPhase() == Player::Start) {
				player->skip(Player::Play);
				player->skip(Player::Discard);
			}
			break;
		}
		break;

	case CardUsed:
	{
		CardUseStruct use = data.value<CardUseStruct>();
		switch(room->getTag("SceneID").toInt()) {
		case 16:
			if(use.card->inherits("Peach") && player->getPhase() == Player::Play) {
				ServerPlayer *effectTo = room->askForPlayerChosen(player, room->getOtherPlayers(player), "Scene16");
				RecoverStruct recover;
				recover.who = effectTo;
				recover.card = use.card;
				return true;
			}
			break;
		}
		break;
	}

	case CardEffect:
	{
		CardEffectStruct effect = data.value<CardEffectStruct>();
		switch(room->getTag("SceneID").toInt()) {
		case 7:
			if(effect.card->inherits("TrickCard") && !effect.card->inherits("DelayedTrick"))
				return true;
			break;
		}
	}

	case Predamaged:
	{
		DamageStruct damage = data.value<DamageStruct>();
		switch(room->getTag("SceneID").toInt()) {
		case 5:
			return true;

		case 14:
			if(damage.nature == DamageStruct::Fire) {
				const Card *card;
				foreach(ServerPlayer *p, room->getAlivePlayers()) {
					while(card = room->askForCard(p, "fire_slash", "scene_14_prompt_fs"))
						damage.damage++;
				}
				foreach(ServerPlayer *p, room->getAlivePlayers()) {
					while(card = room->askForCard(p, "fire_attack", "scene_14_prompt_fa"))
						damage.damage++;
				}
			}
			data = QVariant::fromValue(damage);
			break;

		case 17:
			room->loseHp(player->getNextAlive(), damage.damage);
			return true;

		case 20:
			if((damage.nature == DamageStruct::Thunder) && damage.chain == false) {
				LogMessage log;
				log.type = "#Scene20Buff";
				log.from = player;
				log.to << damage.to;
				log.arg = QString::number(damage.damage);
				log.arg2 = QString::number(damage.damage + 1);
				player->getRoom()->sendLog(log);

				damage.damage++;
				data = QVariant::fromValue(damage);
			}
			break;

		case 21:
			if((damage.nature == DamageStruct::Fire) && damage.chain == false) {
				LogMessage log;
				log.type = "#Scene21Buff";
				log.from = player;
				log.to << damage.to;
				log.arg = QString::number(damage.damage);
				log.arg2 = QString::number(damage.damage + 1);
				player->getRoom()->sendLog(log);

				damage.damage++;
				data = QVariant::fromValue(damage);
			}
			break;

		case 22:
			if((damage.nature == DamageStruct::Thunder || damage.nature == DamageStruct::Fire) && damage.chain == false) {
				LogMessage log;
				log.type = "#Scene22Buff";
				log.from = player;
				log.to << damage.to;
				log.arg = QString::number(damage.damage);
				log.arg2 = QString::number(damage.damage + 1);
				player->getRoom()->sendLog(log);

				damage.damage++;
				data = QVariant::fromValue(damage);
			}
			break;
		}
		break;
	}

	case Damaged:
	{
		DamageStruct damage = data.value<DamageStruct>();
		switch(room->getTag("SceneID").toInt()) {
		case 15:
			if(damage.card && damage.card->inherits("Slash") && damage.nature == DamageStruct::Normal)
				if(!player->isKongcheng())
					room->askForDiscard(player, "", 1);
			break;
		}
		break;
	}

	default:
		break;
	}

	return GameRule::trigger(event, player, data);
}
