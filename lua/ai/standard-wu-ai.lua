--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Rara

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 3.0
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Rara
*********************************************************************]]
local zhiheng_skill = {}
zhiheng_skill.name = "zhiheng"
table.insert(sgs.ai_skills, zhiheng_skill)
zhiheng_skill.getTurnUseCard = function(self)
	if ( self:willShowForAttack() or self:willShowForDefence() ) and not self.player:hasUsed("ZhihengCard") then
		return sgs.Card_Parse("@ZhihengCard=.&zhiheng")
	end
end

sgs.ai_skill_use_func.ZhihengCard = function(card, use, self)
	local unpreferedCards = {}
	local cards = sgs.QList2Table(self.player:getHandcards())

	if self:getCardsNum("Crossbow", 'he') > 0 and #self.enemies > 0 and self.player:getCardCount(true) >= 4 then
		local zcards = sgs.QList2Table(self.player:getCards("he"))
		self:sortByUseValue(zcards, true)
		for _, zcard in ipairs(zcards) do
			if not isCard("Peach", zcard, self.player) and (self.player:getOffensiveHorse() or card:isKindOf("OffensiveHorse")) and not self.player:isJilei(zcard) then
				table.insert(unpreferedCards, zcard:getEffectiveId())
				if #unpreferedCards >= self.player:getMaxHp() then break end
			end
		end
		if #unpreferedCards > 0 then
			use.card = sgs.Card_Parse("@ZhihengCard=" .. table.concat(unpreferedCards, "+") .. "&zhiheng")
			return
		end
	end

	if self.player:getHp() < 3 then
		local zcards = self.player:getCards("he")
		local use_slash, keep_jink, keep_analeptic, keep_weapon = false, false, false
		local zcards = sgs.QList2Table(self.player:getCards("he"))
		self:sortByUseValue(zcards, true)
		for _, zcard in ipairs(zcards) do
			if not isCard("Peach", zcard, self.player) and not isCard("ExNihilo", zcard, self.player) then
				local shouldUse = true
				if isCard("Slash", zcard, self.player) and not use_slash then
					local dummy_use = { isDummy = true , to = sgs.SPlayerList()}
					self:useBasicCard(zcard, dummy_use)
					if dummy_use.card then
						if dummy_use.to then
							for _, p in sgs.qlist(dummy_use.to) do
								if p:getHp() <= 1 then
									shouldUse = false
									if self.player:distanceTo(p) > 1 then keep_weapon = self.player:getWeapon() end
									break
								end
							end
							if dummy_use.to:length() > 1 then shouldUse = false end
						end
						if not self:isWeak() then shouldUse = false end
						if not shouldUse then use_slash = true end
					end
				end
				if zcard:getTypeId() == sgs.Card_TypeTrick then
					local dummy_use = { isDummy = true }
					self:useTrickCard(zcard, dummy_use)
					if dummy_use.card then shouldUse = false end
				end
				if zcard:getTypeId() == sgs.Card_TypeEquip and not self.player:hasEquip(zcard) then
					local dummy_use = { isDummy = true }
					self:useEquipCard(zcard, dummy_use)
					if dummy_use.card then shouldUse = false end
					if keep_weapon and zcard:getEffectiveId() == keep_weapon:getEffectiveId() then shouldUse = false end
				end
				if self.player:hasEquip(zcard) and zcard:isKindOf("Armor") and not self:needToThrowArmor() then shouldUse = false end
				if self.player:hasEquip(zcard) and zcard:isKindOf("DefensiveHorse") and not self:needToThrowArmor() then shouldUse = false end
				if isCard("Jink", zcard, self.player) and not keep_jink then
					keep_jink = true
					shouldUse = false
				end
				if self.player:getHp() == 1 and isCard("Analeptic", zcard, self.player) and not keep_analeptic then
					keep_analeptic = true
					shouldUse = false
				end
				if shouldUse then table.insert(unpreferedCards, zcard:getId()) end
			end
		end
	end

	if #unpreferedCards == 0 then
		local use_slash_num = 0
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			if card:isKindOf("Slash") then
				local will_use = false
				if use_slash_num <= sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_Residue, self.player, card) then
					local dummy_use = { isDummy = true }
					self:useBasicCard(card, dummy_use)
					if dummy_use.card then
						will_use = true
						use_slash_num = use_slash_num + 1
					end
				end
				if not will_use then table.insert(unpreferedCards, card:getId()) end
			end
		end

		local num = self:getCardsNum("Jink") - 1
		if self.player:getArmor() then num = num + 1 end
		if num > 0 then
			for _, card in ipairs(cards) do
				if card:isKindOf("Jink") and num > 0 then
					table.insert(unpreferedCards, card:getId())
					num = num - 1
				end
			end
		end
		for _, card in ipairs(cards) do
			if (card:isKindOf("Weapon") and self.player:getHandcardNum() < 3) or card:isKindOf("OffensiveHorse")
				or self:getSameEquip(card, self.player) or card:isKindOf("AmazingGrace") then
				table.insert(unpreferedCards, card:getId())
			elseif card:getTypeId() == sgs.Card_TypeTrick then
				local dummy_use = { isDummy = true }
				self:useTrickCard(card, dummy_use)
				if not dummy_use.card then table.insert(unpreferedCards, card:getId()) end
			end
		end

		local maxEquipNum = 9
		local insertEquipNum = 0
		if self.player:hasSkill("xiaoji") then maxEquipNum = 1 end

		if self.player:getWeapon() and self.player:getHandcardNum() < 3 and insertEquipNum < maxEquipNum then
			table.insert(unpreferedCards, self.player:getWeapon():getId())
			insertEquipNum = insertEquipNum + 1
		end

		if self:needToThrowArmor() and insertEquipNum < maxEquipNum then
			table.insert(unpreferedCards, self.player:getArmor():getId())
			insertEquipNum = insertEquipNum + 1
		end

		if self.player:getOffensiveHorse() and self.player:getWeapon() and insertEquipNum < maxEquipNum then
			table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
			insertEquipNum = insertEquipNum + 1
		end

		if self.player:getDefensiveHorse() and self.player:hasSkill("xiaoji") and insertEquipNum < maxEquipNum then
			table.insert(unpreferedCards, self.player:getDefensiveHorse():getId())
			insertEquipNum = insertEquipNum + 1
		end

	end

	local use_cards = {}
	for index = #unpreferedCards, 1, -1 do
		if not self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then
			if #use_cards < self.player:getMaxHp() then
				table.insert(use_cards, unpreferedCards[index])
			end
		end
	end

	if #use_cards > 0 then
		use.card = sgs.Card_Parse("@ZhihengCard=" .. table.concat(use_cards, "+") .. "&zhiheng")
	end
end

sgs.ai_use_value.ZhihengCard = 9
sgs.ai_use_priority.ZhihengCard = 2.61
sgs.dynamic_value.benefit.ZhihengCard = true

function sgs.ai_cardneed.zhiheng(to, card)
	return not card:isKindOf("Jink")
end

local qixi_skill = {}
qixi_skill.name = "qixi"
table.insert(sgs.ai_skills, qixi_skill)
qixi_skill.getTurnUseCard = function(self, inclusive)

	local cards = {}
	if self.player:hasSkill("xiaoji") and not self.player:getEquips():isEmpty() then
		for _, c in sgs.qlist(self.player:getEquips()) do
			if c:isBlack() then table.insert(cards, c) end
		end
		if #cards > 0 then
			self:sortByUseValue(cards, true)
			local black_card = cards[1]
			local suit = black_card:getSuitString()
			local number = black_card:getNumberString()
			local card_id = black_card:getEffectiveId()
			local card_str = ("dismantlement:qixi[%s:%s]=%d%s"):format(suit, number, card_id, "&qixi")
			local dismantlement = sgs.Card_Parse(card_str)

			assert(dismantlement)

			return dismantlement
		end
	end

	cards = sgs.QList2Table(self.player:getCards("he"))
	self:sortByUseValue(cards, true)

	local has_weapon = false
	local black_card
	for _, card in ipairs(cards) do
		if card:isKindOf("Weapon") and card:isBlack() then has_weapon = true end
	end

	for _, card in ipairs(cards) do
		if card:isBlack() and ((self:getUseValue(card) < sgs.ai_use_value.Dismantlement) or inclusive or self:getOverflow() > 0) then
			local shouldUse = true

			if card:isKindOf("Armor") then
				if not self.player:getArmor() then shouldUse = false
				elseif self.player:hasEquip(card) and not self:needToThrowArmor() then shouldUse = false
				end
			elseif card:isKindOf("Weapon") then
				if not self.player:getWeapon() then shouldUse = false
				elseif self.player:hasEquip(card) and not has_weapon then shouldUse = false
				end
			elseif card:isKindOf("Slash") then
				local dummy_use = {isDummy = true}
				if self:getCardsNum("Slash") == 1 then
					self:useBasicCard(card, dummy_use)
					if dummy_use.card then shouldUse = false end
				end
			elseif card:isKindOf("TrickCard") and self:getUseValue(card) > sgs.ai_use_value.Dismantlement then
				local dummy_use = {isDummy = true}
				self:useTrickCard(card, dummy_use)
				if dummy_use.card then shouldUse = false end
			end

			if not self:willShowForAttack() then
				shouldUse = false
			end

			if shouldUse then
				black_card = card
				break
			end

		end
	end

	if black_card then
		local suit = black_card:getSuitString()
		local number = black_card:getNumberString()
		local card_id = black_card:getEffectiveId()
		local card_str = ("dismantlement:qixi[%s:%s]=%d%s"):format(suit, number, card_id, "&qixi")
		local dismantlement = sgs.Card_Parse(card_str)

		assert(dismantlement)

		return dismantlement
	end
end

sgs.qixi_suit_value = {
	spade = 3.9,
	club = 3.9
}

sgs.ai_suit_priority.qixi= "diamond|heart|club|spade"

function sgs.ai_cardneed.qixi(to, card)
	return card:isBlack()
end


local kurou_skill = {}
kurou_skill.name = "kurou"
table.insert(sgs.ai_skills, kurou_skill)
kurou_skill.getTurnUseCard = function(self, inclusive)

	self.player:setFlags("-Kurou_toDie")
	sgs.ai_use_priority.KurouCard = 6.8
	local kuroucard = sgs.Card_Parse("@KurouCard=.&kurou")

	if not self:willShowForAttack() then
		return nil
	end

	if self.player:getMark("Global_TurnCount") < 2 and not self.player:hasShownOneGeneral() then return nil end

	if ((self.player:getHp() > 3 and self.player:getLostHp() <= 1 and self:getOverflow(self.player, false) < 2) or self:getOverflow(self.player, false) < -1) then
		return kuroucard
	end

	if self.player:hasSkill("jieyin") and not self.player:hasUsed("JieyinCard") and not self.player:isWounded() then
		local jiyou = self:getWoundedFriend(true)
		if jiyou then
			return kuroucard
		end
	end

	if (self.player:getHp() > 2 and self.player:getLostHp() <= 1 and self.player:hasSkill("xiaoji") and self.player:getCards("e"):length() > 1) then
		return kuroucard
	end

	local slash = sgs.cloneCard("slash")
	if self:hasCrossbowEffect(self.player) then
		for _, enemy in ipairs(self.enemies) do
			if enemy:hasShownOneGeneral() then
				if self.player:canSlash(enemy, nil, true) and self:slashIsEffective(slash, enemy)
					and not (enemy:hasShownSkill("kongcheng") and enemy:isKongcheng())
					and not (enemy:hasShownSkills("fankui") and not self.player:hasSkill("paoxiao"))
					and sgs.isGoodTarget(enemy, self.enemies, self) and not self:slashProhibit(slash, enemy) and self.player:getHp() > 1 then
					return kuroucard
				end
			end
		end
	end
	if self.player:getHp() == 1 and self:getCardsNum("Analeptic") >= 1 then
		return kuroucard
	end

	--Suicide by Kurou
	local nextplayer = self.player:getNextAlive()
	if self.player:getHp() == 1 and self:getCardsNum("Armor") == 0 and self:getCardsNum("Jink") == 0 and self:getKingdomCount() > 1 then
		local to_death = false
		if self:isFriend(nextplayer) then
			for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if p:hasShownSkill("xiaoguo") and not self:isFriend(p) and not p:isKongcheng() and self.player:getEquips():isEmpty() then
					to_death = true
					break
				end
			end
			if not to_death and not self:willSkipPlayPhase(nextplayer) then
				if nextplayer:hasShownSkill("jieyin") and self.player:isMale() then return end
				if nextplayer:hasShownSkill("qingnang") then return end
			end
		end
		if not self:isFriend(nextplayer) and (not self:willSkipPlayPhase(nextplayer) or nextplayer:hasShownSkill("shensu")) then
			to_death = true
		end
		if to_death then
			local caopi = sgs.findPlayerByShownSkillName("xingshang")
			if caopi and self:isEnemy(caopi) and self.player:getHandcardNum() > 3 then
				to_death = false
			end
			if #self.friends == 1 and #self.enemies == 1 and self.player:aliveCount() == 2 then to_death = false end
		end
		if to_death then
			self.player:setFlags("Kurou_toDie")
			sgs.ai_use_priority.KurouCard = 0
			return kuroucard
		end
	end
end

sgs.ai_skill_use_func.KurouCard = function(card, use, self)
	use.card = card
end

sgs.ai_use_priority.KurouCard = 6.8


sgs.ai_skill_invoke.yingzi = function(self, data)

	if not self:willShowForAttack() and not self:willShowForDefence() then
		return false
	end

	if self.player:hasSkill("haoshi") then
		local num = self.player:getHandcardNum()
		local skills = self.player:getVisibleSkillList()
		local count = self:ImitateResult_DrawNCards(self.player, skills)
		if num + count > 5 then
			local others = self.room:getOtherPlayers(self.player)
			local least = 999
			local target = nil
			for _,p in sgs.qlist(others) do
				local handcardnum = p:getHandcardNum()
				if handcardnum < least then
					least = handcardnum
					target = p
				end
			end
			if target and not self:isFriend(target) and num + count == 6 then return false end
		end
	end
	return true
end

local fanjian_skill = {}
fanjian_skill.name = "fanjian"
table.insert(sgs.ai_skills, fanjian_skill)
fanjian_skill.getTurnUseCard = function(self)
	if not self:willShowForAttack() then return nil end
	if self.player:isKongcheng() then return nil end
	if self.player:hasUsed("FanjianCard") then return nil end
	return sgs.Card_Parse("@FanjianCard=.&fanjian")
end

sgs.ai_skill_use_func.FanjianCard = function(fjCard, use, self)

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)
	if #cards == 1 and cards[1]:getSuit() == sgs.Card_Diamond then return end
	if #cards <= 4 and (self:getCardsNum("Peach") > 0 or self:getCardsNum("Analeptic") > 0) then return end
	self:sort(self.enemies, "hp")

	local suits = {}
	local suits_num = 0
	for _, c in ipairs(cards) do
		if not suits[c:getSuitString()] then
			suits[c:getSuitString()] = true
			suits_num = suits_num + 1
		end
	end

	for _, enemy in ipairs(self.enemies) do
		local visible = 0
		for _, card in ipairs(cards) do
			local flag = string.format("%s_%s_%s", "visible", enemy:objectName(), self.player:objectName())
			if card:hasFlag("visible") or card:hasFlag(flag) then visible = visible + 1 end
		end
		if visible > 0 and (#cards <= 2 or suits_num <= 2) then continue end
		if self:canAttack(enemy) and not enemy:hasShownSkills("qingnang|jijiu|tianxiang") then
			use.card = fjCard
			if use.to then use.to:append(enemy) end
			return
		end
	end
end

sgs.ai_card_intention.FanjianCard = 70

function sgs.ai_skill_suit.fanjian(self)
	local map = {0, 0, 1, 2, 2, 3, 3, 3}
	local suit = map[math.random(1, 8)]
	local tg = self.room:getCurrent()
	local suits = {}
	local maxnum, maxsuit = 0
	for _, c in sgs.qlist(tg:getHandcards()) do
		local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), tg:objectName())
		if c:hasFlag(flag) or c:hasFlag("visible") then
			if not suits[c:getSuitString()] then suits[c:getSuitString()] = 1 else suits[c:getSuitString()] = suits[c:getSuitString()] + 1 end
			if suits[c:getSuitString()] > maxnum then
				maxnum = suits[c:getSuitString()]
				maxsuit = c:getSuit()
			end
		end
	end
	if self.player:hasSkill("hongyan") and (maxsuit == sgs.Card_Spade or suit == sgs.Card_Spade) then
		return sgs.Card_Heart
	end
	if maxsuit then
		if self.player:hasSkill("hongyan") and maxsuit == sgs.Card_Spade then return sgs.Card_Heart end
		return maxsuit
	else
		if self.player:hasSkill("hongyan") and suit == sgs.Card_Spade then return sgs.Card_Heart end
		return suit
	end
end

sgs.dynamic_value.damage_card.FanjianCard = true


local duoshi_skill = {}
duoshi_skill.name = "duoshi"
table.insert(sgs.ai_skills, duoshi_skill)
duoshi_skill.getTurnUseCard = function(self, inclusive)
	local DuoTime = 2
	if self.player:hasSkills("fenming|zhiheng|fenxun|keji") then
		DuoTime = 1
	end
	if self.player:hasSkills("hongyan|yingzi") then
		DuoTime = 3
	end
	if self.player:hasSkills("xiaoji|haoshi") then
		DuoTime = 4
	end
	for _, player in ipairs(self.friends) do
		if player:hasShownSkills("xiaoji|haoshi") then
			DuoTime = 4
			break
		end
	end

	if (self.player:usedTimes("DuoshiAE") >= DuoTime and self:getOverflow() <= 0) or self.player:usedTimes("DuoshiAE") >= 4 	then return end


	if sgs.turncount <= 1 and #self.friends_noself == 0 and not self:isWeak() and self:getOverflow() <= 0 then return end
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)


	if (self:hasCrossbowEffect() or self:getCardsNum("Crossbow") > 0) and self:getCardsNum("Slash") > 0 then
		self:sort(self.enemies, "defense")
		for _, enemy in ipairs(self.enemies) do
			local inAttackRange = self.player:distanceTo(enemy) == 1 or self.player:distanceTo(enemy) == 2
									and self:getCardsNum("OffensiveHorse") > 0 and not self.player:getOffensiveHorse()
			if inAttackRange  and sgs.isGoodTarget(enemy, self.enemies, self) then
				local slashes = self:getCards("Slash")
				local slash_count = 0
				for _, slash in ipairs(slashes) do
					if not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) then
						slash_count = slash_count + 1
					end
				end
				if slash_count >= enemy:getHp() then return end
			end
		end
	end

	local red_card
	if self.player:getHandcardNum() <= 2 then return end
	self:sortByUseValue(cards, true)

	for _, card in ipairs(cards) do
		if card:isRed() then
			local shouldUse = true
			if card:isKindOf("Slash") then
				local dummy_use = { isDummy = true }
				if self:getCardsNum("Slash") == 1 then
					self:useBasicCard(card, dummy_use)
					if dummy_use.card then shouldUse = false end
				end
			end

			if self:getUseValue(card) > sgs.ai_use_value.AwaitExhausted and card:isKindOf("TrickCard") then
				local dummy_use = { isDummy = true }
				self:useTrickCard(card, dummy_use)
				if dummy_use.card then shouldUse = false end
			end

			local sunshangxiang = false
			if self.player:hasSkills("xiaoji") and self.player:getCards("e"):length() > 0 then
				sunshangxiang = true
			end
			for _, player in ipairs(self.friends) do
				if player:hasShownSkill("xiaoji") and player:getCards("e"):length() > 0 then
					sunshangxiang = true
					break
				end
			end

			if not self:willShowForDefence() and not sunshangxiang then
				shouldUse = false
			end

			if shouldUse and not card:isKindOf("Peach") then
				red_card = card
				break
			end

		end
	end

	if red_card then
		local card_id = red_card:getEffectiveId()
		local card_str = string.format("await_exhausted:duoshi[%s:%d]=%d&duoshi",red_card:getSuitString(), red_card:getNumber(), red_card:getEffectiveId())
		local await = sgs.Card_Parse(card_str)
		assert(await)
		return await
	end
end


local guose_skill = {}
guose_skill.name = "guose"
table.insert(sgs.ai_skills, guose_skill)
guose_skill.getTurnUseCard = function(self, inclusive)

	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards, true)

	local has_weapon, has_armor = false, false

	for _,acard in ipairs(cards)  do
		if acard:isKindOf("Weapon") and not (acard:getSuit() == sgs.Card_Diamond) then has_weapon=true end
	end

	for _,acard in ipairs(cards)  do
		if acard:isKindOf("Armor") and not (acard:getSuit() == sgs.Card_Diamond) then has_armor=true end
	end

	for _,acard in ipairs(cards)  do
		if (acard:getSuit() == sgs.Card_Diamond) and ((self:getUseValue(acard)<sgs.ai_use_value.Indulgence) or inclusive) then
			local shouldUse=true

			if acard:isKindOf("Armor") then
				if not self.player:getArmor() then shouldUse = false
				elseif self.player:hasEquip(acard) and not has_armor and self:evaluateArmor() > 0 then shouldUse = false
				end
			end

			if acard:isKindOf("Weapon") then
				if not self.player:getWeapon() then shouldUse = false
				elseif self.player:hasEquip(acard) and not has_weapon then shouldUse = false
				end
			end

			if not self:willShowForAttack() then
				shouldUse = false
			end

			if shouldUse then
				card = acard
				break
			end
		end
	end

	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("indulgence:guose[diamond:%s]=%d&guose"):format(number, card_id)
	local indulgence = sgs.Card_Parse(card_str)
	assert(indulgence)
	return indulgence
end

function sgs.ai_cardneed.guose(to, card)
	return card:getSuit() == sgs.Card_Diamond
end

sgs.ai_suit_priority.guose= "club|spade|heart|diamond"


sgs.ai_skill_use["@@liuli"] = function(self, prompt, method)
	local others = self.room:getOtherPlayers(self.player)
	others = sgs.QList2Table(others)
	local source
	for _, player in ipairs(others) do
		if player:hasFlag("LiuliSlashSource") then
			source = player
			break
		end
	end
	local slash = self.player:getTag("liuli-card"):toCard()
	local nature = sgs.Slash_Natures[slash:getClassName()]

	local doLiuli = function(who)
		if not self:isFriend(who) and who:hasShownSkill("leiji")
			and (self:hasSuit("spade", true, who) or who:getHandcardNum() >= 3)
			and (getKnownCard(who, self.player, "Jink", true) >= 1 or self:hasEightDiagramEffect(who)) then
			return "."
		end

		local cards = self.player:getCards("h")
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			if not self.player:isCardLimited(card, method) and self.player:canSlash(who) then
				if self:isFriend(who) and not (isCard("Peach", card, self.player) or isCard("Analeptic", card, self.player)) then
					return "@LiuliCard="..card:getEffectiveId().."&liuli->"..who:objectName()
				else
					return "@LiuliCard="..card:getEffectiveId().."&liuli->"..who:objectName()
				end
			end
		end

		local cards = self.player:getCards("e")
		cards = sgs.QList2Table(cards)
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			local range_fix = 0
			if card:isKindOf("Weapon") then range_fix = range_fix + sgs.weapon_range[card:getClassName()] - self.player:getAttackRange(false) end
			if card:isKindOf("OffensiveHorse") then range_fix = range_fix + 1 end
			if not self.player:isCardLimited(card, method) and self.player:canSlash(who, nil, true, range_fix) then
				return "@LiuliCard=" .. card:getEffectiveId() .. "&liuli->" .. who:objectName()
			end
		end
		return "."
	end

	local isJinkEffected
	for _, jink in ipairs(self:getCards("Jink")) do
		if self.room:isJinkEffected(user, jink) then isJinkEffected = true break end
	end

	local liuli = {}

	if not self:damageIsEffective(self.player, nature, source) then liuli[2] = "."
	elseif self:needToLoseHp(self.player, source, true) then liuli[2] = "."
	elseif self:getDamagedEffects(self.player, source, true) then liuli[2] = "." end

	self:sort(others, "defense")
	for _, player in ipairs(others) do
		if not (source and source:objectName() == player:objectName()) then
			if self:isEnemy(player) then
				if not (source and source:objectName() == player:objectName()) then
					if self:slashIsEffective(slash, player, false, source) then
						if not self:getDamagedEffects(player, source, true) then
							if self:hasHeavySlashDamage(source, slash, player) then
								if not source or self:isFriend(source, player) then
									local ret = doLiuli(player)
									if ret ~= "." then return ret end
								elseif not liuli[1] then
									local ret = doLiuli(player)
									if ret ~= "." then liuli[1] = ret end
								end
							elseif not liuli[5] then
								local ret = doLiuli(player)
								if ret ~= "." then liuli[5] = ret end
							end
						elseif not liuli[8] then
							local ret = doLiuli(player)
							if ret ~= "." then liuli[8] = ret end
						end
					elseif not liuli[6] then
						local ret = doLiuli(player)
						if ret ~= "." then liuli[6] = ret end
					end
				end
			elseif self:isFriend(player) then
				if not (source and source:objectName() == player:objectName()) then
					if self:slashIsEffective(slash, player, source) then
						if self:findLeijiTarget(player, 50, source) then
							local ret = doLiuli(player)
							if ret ~= "." then liuli[3] = ret end
						elseif not self:hasHeavySlashDamage(source, slash, player) then
							if self:getDamagedEffects(player, source, true) or self:needToLoseHp(player, source, true, true) then
								local ret = doLiuli(player)
								if ret ~= "." then liuli[4] = ret end
							end
						elseif self:isWeak() and (not isJinkEffected or self:canHit(self.player, source)) then
							if getCardsNum("Jink", player, self.player) >= 1 then
								local ret = doLiuli(player)
								if ret ~= "." then liuli[10] = ret end
							elseif not self:isWeak(player) then
								local ret = doLiuli(player)
								if ret ~= "." then liuli[11] = ret end
							end
						end
					else
						local ret = doLiuli(player)
						if ret ~= "." then liuli[7] = ret end
					end
				end
			else
				local ret = doLiuli(player)
				if ret ~= "." then liuli[9] = ret end
			end
		end
	end

	local ret = "."
	local i = 99
	for k, str in pairs(liuli) do
		if k < i then
			i = k
			ret = str
		end
	end

	return ret
end


function sgs.ai_slash_prohibit.liuli(self, from, to, card)
	if self:isFriend(to, from) then return false end
	if to:isNude() then return false end
	for _, friend in ipairs(self:getFriendsNoself(from)) do
		if to:canSlash(friend, card) and self:slashIsEffective(card, friend, from) then return true end
	end
end

function sgs.ai_cardneed.liuli(to, card)
	return to:getCards("he"):length() <= 2
end

sgs.guose_suit_value = { diamond = 3.9 }


function SmartAI:getWoundedFriend(maleOnly)
	self:sort(self.friends, "hp")
	local list1 = {}	-- need help
	local list2 = {}	-- do not need help
	local addToList = function(p,index)
		if ( (not maleOnly) or (maleOnly and p:isMale()) ) and p:isWounded() then
			table.insert(index ==1 and list1 or list2, p)
		end
	end

	local getCmpHp = function(p)
		local hp = p:getHp()
		if p:isLord() and self:isWeak(p) then hp = hp - 10 end
		if p:objectName() == self.player:objectName() and self:isWeak(p) and p:hasShownSkill("qingnang") then hp = hp - 5 end
		if p:hasShownSkill("buqu") and p:getPile("buqu"):length() > 0 then hp = hp + math.max(0, 5 - p:getPile("buqu"):length()) end
		if p:hasShownSkills("rende|kuanggu|zaiqi") and p:getHp() >= 2 then hp = hp + 5 end
		return hp
	end


	local cmp = function (a ,b)
		if getCmpHp(a) == getCmpHp(b) then
			return sgs.getDefenseSlash(a, self) < sgs.getDefenseSlash(b, self)
		else
			return getCmpHp(a) < getCmpHp(b)
		end
	end

	for _, friend in ipairs(self.friends) do
		if friend:isLord() then
			if self:needToLoseHp(friend, nil, nil, true, true) then
				addToList(friend, 2)
			else
				addToList(friend, 1)
			end
		else
			if self:needToLoseHp(friend, nil, nil, nil, true) or (friend:hasShownSkills("rende|kuanggu|zaiqi") and friend:getHp() >= 2) then
				addToList(friend, 2)
			else
				addToList(friend, 1)
			end
		end
	end
	table.sort(list1, cmp)
	table.sort(list2, cmp)
	return list1, list2
end

local jieyin_skill = {}
jieyin_skill.name = "jieyin"
table.insert(sgs.ai_skills, jieyin_skill)
jieyin_skill.getTurnUseCard = function(self)
	if self.player:getHandcardNum() < 2 then return nil end
	if self.player:hasUsed("JieyinCard") then return nil end

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)

	local first, second
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if card:isKindOf("TrickCard") then
			local dummy_use = {isDummy = true}
			self:useTrickCard(card, dummy_use)
			if not dummy_use.card then
				if not first then first = card:getEffectiveId()
				elseif first and not second then second = card:getEffectiveId()
				end
			end
			if first and second then break end
		end
	end

	for _, card in ipairs(cards) do
		if card:getTypeId() ~= sgs.Card_TypeEquip and (not self:isValuableCard(card) or self.player:isWounded()) then
			if not first then first = card:getEffectiveId()
			elseif first and first ~= card:getEffectiveId() and not second then second = card:getEffectiveId()
			end
		end
		if first and second then break end
	end

	if not second or not first then return end
	local card_str = ("@JieyinCard=%d+%d%s"):format(first, second, "&jieyin")
	assert(card_str)
	return sgs.Card_Parse(card_str)
end

sgs.ai_skill_use_func.JieyinCard = function(card, use, self)
	local arr1, arr2 = self:getWoundedFriend(true)
	table.removeOne(arr1, self.player)
	table.removeOne(arr2, self.player)
	local target = nil

	repeat
		if #arr1 > 0 and (self:isWeak(arr1[1]) or self:isWeak() or self:getOverflow() >= 1) then
			target = arr1[1]
			break
		end
		if #arr2 > 0 and self:isWeak() then
			target = arr2[1]
			break
		end
	until true

	if not target and self:isWeak() and self:getOverflow() >= 2 and (self.role == "lord" or self.role == "renegade") then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if other:isWounded() and other:isMale() and not other:hasShownSkills(sgs.masochism_skill) then
				target = other
				self.player:setFlags("jieyin_isenemy_" .. other:objectName())
				break
			end
		end
	end

	if target then
		use.card = card
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_priority.JieyinCard = 2.8

sgs.ai_card_intention.JieyinCard = function(self, card, from, tos)
	if not from:hasFlag("jieyin_isenemy_"..tos[1]:objectName()) then
		sgs.updateIntention(from, tos[1], -80)
	end
end

sgs.dynamic_value.benefit.JieyinCard = true

sgs.xiaoji_keep_value = {
	Weapon = 4.9,
	Armor = 5,
	OffensiveHorse = 4.8,
	DefensiveHorse = 5
}

sgs.ai_cardneed.xiaoji = sgs.ai_cardneed.equip

sgs.ai_skill_playerchosen.yinghun = function(self, targets)

	if not self:willShowForAttack() and not self:willShowForDefence() then
		return nil
	end

	local x = self.player:getLostHp()
	local n = x - 1
	self:updatePlayers()

	self.yinghun = nil
	local player = self:AssistTarget()

	if x == 1 then
		self:sort(self.friends_noself, "handcard")
		self.friends_noself = sgs.reverse(self.friends_noself)
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasShownSkills(sgs.lose_equip_skill) and friend:getCards("e"):length() > 0 then
				self.yinghun = friend
				break
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if friend:hasShownSkill("tuntian") then
					self.yinghun = friend
					break
				end
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if self:needToThrowArmor(friend) then
					self.yinghun = friend
					break
				end
			end
		end

		if not self.yinghun and player and player:getCardCount(true) > 0 and not self:needKongcheng(player, true) then
			self.yinghun = player
		end

		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if friend:getCards("he"):length() > 0 then
					self.yinghun = friend
					break
				end
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				self.yinghun = friend
				break
			end
		end
	elseif #self.friends > 1 then
		self:sort(self.friends_noself)
		for _, friend in ipairs(self.friends_noself) do
			if friend:hasShownSkills(sgs.lose_equip_skill) and friend:getCards("e"):length() > 0 then
				self.yinghun = friend
				break
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if friend:hasShownSkill("tuntian") then
					self.yinghun = friend
					break
				end
			end
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				if self:needToThrowArmor(friend) then
					self.yinghun = friend
					break
				end
			end
		end
		if not self.yinghun and #self.enemies > 0 then
			local wf
			if self.player:isLord() then
				if self:isWeak() and (self.player:getHp() < 2 and self:getCardsNum("Peach") < 1) then
					wf = true
				end
			end
			if not wf then
				for _, friend in ipairs(self.friends_noself) do
					if self:isWeak(friend) then
						wf = true
						break
					end
				end
			end
			if not wf then
				self:sort(self.enemies)
				for _, enemy in ipairs(self.enemies) do
					if enemy:getCards("he"):length() == n
						and not self:doNotDiscard(enemy, "nil", true, n) then
						self.yinghunchoice = "d1tx"
						return enemy
					end
				end
				for _, enemy in ipairs(self.enemies) do
					if enemy:getCards("he"):length() >= n
						and not self:doNotDiscard(enemy, "nil", true, n)
						and enemy:hasShownSkills(sgs.cardneed_skill) then
						self.yinghunchoice = "d1tx"
						return enemy
					end
				end
			end
		end

		if not self.yinghun and player and not self:needKongcheng(player, true) then
			self.yinghun = player
		end

		if not self.yinghun then
			self.yinghun = self:findPlayerToDraw(false, n)
		end
		if not self.yinghun then
			for _, friend in ipairs(self.friends_noself) do
				self.yinghun = friend
				break
			end
		end
		if self.yinghun then self.yinghunchoice = "dxt1" end
	end
	if not self.yinghun and x > 1 and #self.enemies > 0 then
		self:sort(self.enemies, "handcard")
		for _, enemy in ipairs(self.enemies) do
			if enemy:getCards("he"):length() >= n
				and not self:doNotDiscard(enemy, "nil", true, n) then
				self.yinghunchoice = "d1tx"
				return enemy
			end
		end
		self.enemies = sgs.reverse(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isNude()
				and not (enemy:hasShownSkills(sgs.lose_equip_skill) and enemy:getCards("e"):length() > 0)
				and not self:needToThrowArmor(enemy)
				and not enemy:hasShownSkill("tuntian") then
				self.yinghunchoice = "d1tx"
				return enemy
			end
		end
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isNude()
				and not (enemy:hasShownSkills(sgs.lose_equip_skill) and enemy:getCards("e"):length() > 0)
				and not self:needToThrowArmor(enemy)
				and not (enemy:hasShownSkill("tuntian") and x < 3 and enemy:getCards("he"):length() < 2) then
				self.yinghunchoice = "d1tx"
				return enemy
			end
		end
	end

	return self.yinghun
end

sgs.ai_skill_choice.yinghun = function(self, choices)
	return self.yinghunchoice
end

sgs.ai_playerchosen_intention.yinghun = function(self, from, to)
	if from:getLostHp() > 1 then return end
	local intention = -80
	sgs.updateIntention(from, to, intention)
end

sgs.ai_choicemade_filter.skillChoice.yinghun = function(self, player, promptlist)
	local to
	for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
		if p:hasFlag("YinghunTarget") then
			to = p
			break
		end
	end
	local choice = promptlist[#promptlist]
	local intention = (choice == "dxt1") and -80 or 80
	sgs.updateIntention(player, to, intention)
end

sgs.ai_skill_use["@@tianxiang"] = function(self, data, method)
	if not method then method = sgs.Card_MethodDiscard end
	local friend_lost_hp = 10
	local friend_hp = 0
	local card_id
	local target
	local cant_use_skill
	local dmg

	if data == "@tianxiang-card" then
		dmg = self.player:getTag("TianxiangDamage"):toDamage()
	else
		dmg = data
	end

	if not dmg then self.room:writeToConsole(debug.traceback()) return "." end

	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	for _, card in ipairs(cards) do
		if not self.player:isCardLimited(card, method) and (card:getSuit() == sgs.Card_Heart or (self.player:hasSkill("hongyan") and card:getSuit() == sgs.Card_Spade)) then
			card_id = card:getId()
			break
		end
	end
	if not card_id then return "." end

	self:sort(self.enemies, "hp")

	for _, enemy in ipairs(self.enemies) do
		if (enemy:getHp() <= dmg.damage and enemy:isAlive()) then
			if (enemy:getHandcardNum() <= 2 or enemy:hasShownSkills("guose|leiji|ganglie|qingguo|kongcheng") or enemy:containsTrick("indulgence"))
				and self:canAttack(enemy, dmg.from or self.room:getCurrent(), dmg.nature) then
				return "@TianxiangCard=" .. card_id .. "&tianxiang->" .. enemy:objectName()
			end
		end
	end

	local newDamageStruct = dmg
	for _, friend in ipairs(self.friends_noself) do
		newDamageStruct.to = friend
		if not self:damageIsEffective_(newDamageStruct) then
			return "@TianxiangCard=" .. card_id .. "&tianxiang->" .. friend:objectName()
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if (friend:getLostHp() + dmg.damage > 1 and friend:isAlive()) then
			if friend:isChained() and dmg.nature ~= sgs.DamageStruct_Normal and not self:isGoodChainTarget(friend, dmg.from, dmg.nature, dmg.damage, dmg.card) then
			elseif friend:getHp() >= 2 and dmg.damage < 2
					and (friend:hasShownSkills("yiji|buqu|shuangxiong|zaiqi|yinghun|jianxiong|fangzhu")
						or self:getDamagedEffects(friend, dmg.from or self.room:getCurrent())
						or self:needToLoseHp(friend)
						or (friend:getHandcardNum() < 3 and friend:hasShownSkill("rende"))) then
				return "@TianxiangCard=" .. card_id .. "&tianxiang->" .. friend:objectName()
			elseif hasBuquEffect(friend) then return "@TianxiangCard=" .. card_id .. "&tianxiang->" .. friend:objectName() end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if (enemy:getLostHp() <= 1 or dmg.damage > 1) and enemy:isAlive() then
			if (enemy:getHandcardNum() <= 2)
				or enemy:containsTrick("indulgence") or enemy:hasShownSkills("guose|leiji|ganglie|qingguo|kongcheng")
				and self:canAttack(enemy, (dmg.from or self.room:getCurrent()), dmg.nature) then
				return "@TianxiangCard=" .. card_id .. "&tianxiang->" .. enemy:objectName() end
		end
	end

	for i = #self.enemies, 1, -1 do
		local enemy = self.enemies[i]
		if not enemy:isWounded() and not enemy:hasShownSkills(sgs.masochism_skill) and enemy:isAlive()
			and self:canAttack(enemy, dmg.from or self.room:getCurrent(), dmg.nature) and self:isWeak() then
			return "@TianxiangCard=" .. card_id .. "&tianxiang->" .. enemy:objectName()
		end
	end

	if dmg.damage > 1 or dmg.damage >= self.player:getHp() and self:getCardsNum({"Peach", "Analeptic"}) == 0 then
		local targets = self.enemies
		if #targets == 0 then
			for _, p in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if not self:isFriend(p) then table.insert(targets, p) end
			end
		end
		if #targets == 0 and dmg.from then table.insert(targets, dmg.from) end
		if #targets == 0 then table.insert(targets, room:nextPlayer(self.player)) end
		if #targets > 0 then
			self:sort(targets, "hp")
			targets = sgs.reverse(targets)
			return "@TianxiangCard=" .. card_id .. "&tianxiang->" .. targets[1]:objectName()
		end
	end

	return "."
end

sgs.ai_card_intention.TianxiangCard = function(self, card, from, tos)
	local to = tos[1]
	if self:getDamagedEffects(to) or self:needToLoseHp(to) then return end
	local intention = 10
	if hasBuquEffect(to) then intention = 0
	elseif (to:getHp() >= 2 and to:hasShownSkills("yiji|shuangxiong|zaiqi|yinghun|jianxiong|fangzhu"))
		or to:getHandcardNum() < 3 and to:hasShownSkill("rende") then
		intention = -10
	end
	sgs.updateIntention(from, to, intention)
end

function sgs.ai_slash_prohibit.tianxiang(self, from, to)
	if self:isFriend(to, from) then return false end
	return self:cantbeHurt(to, from)
end

sgs.tianxiang_suit_value = {
	heart = 4.9
}

function sgs.ai_cardneed.tianxiang(to, card, self)
	return (card:getSuit() == sgs.Card_Heart or (to:hasShownSkill("hongyan") and card:getSuit() == sgs.Card_Spade))
		and (getKnownCard(to, self.player, "heart", false) + getKnownCard(to, self.player, "spade", false)) < 2
end

sgs.ai_suit_priority.hongyan= "club|diamond|spade|heart"


local tianyi_skill = {}
tianyi_skill.name = "tianyi"
table.insert(sgs.ai_skills, tianyi_skill)
tianyi_skill.getTurnUseCard = function(self)
	if self:willShowForAttack() and not self.player:hasUsed("TianyiCard") and not self.player:isKongcheng() then return sgs.Card_Parse("@TianyiCard=.&tianyi") end
end

sgs.ai_skill_use_func.TianyiCard = function(TYCard, use, self)
	self:sort(self.enemies, "handcard")
	local cards = sgs.CardList()
	local peach = 0
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if isCard("Peach", c, self.player) and peach < 2 then
			peach = peach + 1
		else
			cards:append(c)
		end
	end
	local max_card = self:getMaxCard(self.player, cards)
	if not max_card then return end
	local max_point = max_card:getNumber()
	if self.player:hasSkill("yingyang") then max_point = math.min(max_point + 3, 13) end
	local slashcount = self:getCardsNum("Slash")
	if isCard("Slash", max_card, self.player) then slashcount = slashcount - 1 end
	if self.player:hasSkill("kongcheng") and self.player:getHandcardNum() == 1 then
		for _, enemy in ipairs(self.enemies) do
			if not enemy:isKongcheng() and not self:doNotDiscard(enemy, "h") then
				sgs.ai_use_priority.TianyiCard = 1.2
				self.tianyi_card = max_card:getId()
				use.card = TYCard
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
	for _, enemy in ipairs(self.enemies) do
		if enemy:hasFlag("AI_HuangtianPindian") and enemy:getHandcardNum() == 1 then
			sgs.ai_use_priority.TianyiCard = 7.2
			self.tianyi_card = max_card:getId()
			use.card = TYCard
			if use.to then
				use.to:append(enemy)
				enemy:setFlags("-AI_HuangtianPindian")
			end
			return
		end
	end

	local zhugeliang = sgs.findPlayerByShownSkillName("kongcheng")

	local slash = self:getCard("Slash")
	local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
	self.player:setFlags("TianyiSuccess")
	self.player:setFlags("slashNoDistanceLimit")
	if slash then self:useBasicCard(slash, dummy_use) end
	self.player:setFlags("-slashNoDistanceLimit")
	self.player:setFlags("-TianyiSuccess")

	sgs.ai_use_priority.TianyiCard = (slashcount >= 1 and dummy_use.card) and 7.2 or 1.2
	if slashcount >= 1 and slash and dummy_use.card then
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasShownSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
				local enemy_max_card = self:getMaxCard(enemy)
				local enemy_max_point = enemy_max_card and enemy_max_card:getNumber() or 100
				if enemy_max_card and enemy:hasShownSkill("yingyang") then enemy_max_point = math.min(enemy_max_point + 3, 13) end
				if max_point > enemy_max_point then
					self.tianyi_card = max_card:getId()
					use.card = TYCard
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
		for _, enemy in ipairs(self.enemies) do
			if not (enemy:hasShownSkill("kongcheng") and enemy:getHandcardNum() == 1) and not enemy:isKongcheng() then
				if max_point >= 10 then
					self.tianyi_card = max_card:getId()
					use.card = TYCard
					if use.to then use.to:append(enemy) end
					return
				end
			end
		end
		if #self.enemies < 1 then return end
		if dummy_use.to:length() > 1 then
			self:sort(self.friends_noself, "handcard")
			for index = #self.friends_noself, 1, -1 do
				local friend = self.friends_noself[index]
				if not friend:isKongcheng() then
					local friend_min_card = self:getMinCard(friend)
					local friend_min_point = friend_min_card and friend_min_card:getNumber() or 100
					if friend:hasShownSkill("yingyang") then friend_min_point = math.max(1, friend_min_point - 3) end
					if max_point > friend_min_point then
						self.tianyi_card = max_card:getId()
						use.card = TYCard
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
		end

		if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1 and zhugeliang:objectName() ~= self.player:objectName() then
			if max_point >= 7 then
				self.tianyi_card = max_card:getId()
				use.card = TYCard
				if use.to then use.to:append(zhugeliang) end
				return
			end
		end

		if dummy_use.to:length() > 1 then
			for index = #self.friends_noself, 1, -1 do
				local friend = self.friends_noself[index]
				if not friend:isKongcheng() then
					if max_point >= 7 then
						self.tianyi_card = max_card:getId()
						use.card = TYCard
						if use.to then use.to:append(friend) end
						return
					end
				end
			end
		end
	end

	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	if zhugeliang and self:isFriend(zhugeliang) and zhugeliang:getHandcardNum() == 1
		and zhugeliang:objectName() ~= self.player:objectName() and self:getEnemyNumBySeat(self.player, zhugeliang) >= 1 then
		if isCard("Jink", cards[1], self.player) and self:getCardsNum("Jink") == 1 then return end
		self.tianyi_card = cards[1]:getId()
		use.card = TYCard
		if use.to then use.to:append(zhugeliang) end
		return
	end

	if self:getOverflow() > 0 then
		for _, enemy in ipairs(self.enemies) do
			if not self:doNotDiscard(enemy, "h", true) and not enemy:isKongcheng() then
				self.tianyi_card = cards[1]:getId()
				use.card = TYCard
				if use.to then use.to:append(enemy) end
				return
			end
		end
	end
	return nil
end

function sgs.ai_skill_pindian.tianyi(minusecard, self, requestor)
	if requestor:getHandcardNum() == 1 then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		return cards[1]
	end
	local maxcard = self:getMaxCard()
	return self:isFriend(requestor) and self:getMinCard() or (maxcard:getNumber() < 6 and minusecard or maxcard)
end

sgs.ai_cardneed.tianyi = function(to, card, self)
	local cards = to:getHandcards()
	local has_big = false
	for _, c in sgs.qlist(cards) do
		local flag = string.format("%s_%s_%s", "visible", self.room:getCurrent():objectName(), to:objectName())
		if c:hasFlag("visible") or c:hasFlag(flag) then
			if c:getNumber() > 10 then
				has_big = true
				break
			end
		end
	end
	if not has_big then
		return card:getNumber() > 10
	else
		return card:isKindOf("Slash") or card:isKindOf("Analeptic")
	end
end

sgs.ai_card_intention.TianyiCard = 0
sgs.dynamic_value.control_card.TianyiCard = true

sgs.ai_use_value.TianyiCard = 8.5

sgs.ai_skill_askforag.buqu = function(self, card_ids)
	for i, card_id in ipairs(card_ids) do
		for j, card_id2 in ipairs(card_ids) do
			if i ~= j and sgs.Sanguosha:getCard(card_id):getNumber() == sgs.Sanguosha:getCard(card_id2):getNumber() then
				return card_id
			end
		end
	end

	return card_ids[1]
end

function sgs.ai_skill_invoke.buqu(self, data)
	return true
end

sgs.ai_skill_invoke.haoshi = function(self, data)
	self.haoshi_target = nil
	local extra = 0
	local draw_skills = { ["yingzi"] = 1, ["luoyi"] = -1 }
	for skill_name, n in ipairs(draw_skills) do
		if self.player:hasSkill(skill_name) then
			local skill = sgs.Sanguosha:getSkill(skill_name)
			if skill and skill:getFrequency() == sgs.Skill_Compulsory then
				extra = extra + n
			elseif self:askForSkillInvoke(skill_name, data) then
				extra = extra + n
			end
		end
	end
	if self.player:hasTreasure("JadeSeal") then
		extra = extra + 1
	end
	if self.player:getHandcardNum() + extra <= 1 then return true end
	if not self:willShowForDefence() and not self:willShowForAttack() then return false end

	local otherPlayers = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	self:sort(otherPlayers, "handcard")
	local leastNum = otherPlayers[1]:getHandcardNum()

	self:sort(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() == leastNum and friend:isAlive() and self:isFriendWith(friend) then
			self.haoshi_target = friend	
		end
	end
	if not self.haoshi_target then 
		for _, friend in ipairs(self.friends_noself) do
			if friend:getHandcardNum() == leastNum and friend:isAlive() then
				self.haoshi_target = friend	
			end
		end
	end	
	if self.haoshi_target then return true end
	return false
end

sgs.ai_skill_use["@@haoshi!"] = function(self, prompt)
	local target = self.haoshi_target
	if not self.haoshi_target or self.haoshi_target:isDead() then
		local otherPlayers = sgs.QList2Table(self.room:getOtherPlayers(self.player))
		self:sort(otherPlayers, "handcard")
		target = otherPlayers[1]
	end

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card_ids = {}
	for i = 1, math.floor(#cards / 2) do
		table.insert(card_ids, cards[i]:getEffectiveId())
	end

	return "@HaoshiCard=" .. table.concat(card_ids, "+") .. "&haoshi->" .. target:objectName()
end

sgs.ai_card_intention.HaoshiCard = -80

function sgs.ai_cardneed.haoshi(to, card, self)
	return not self:willSkipDrawPhase(to)
end

dimeng_skill = {}
dimeng_skill.name = "dimeng"
table.insert(sgs.ai_skills, dimeng_skill)
dimeng_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("DimengCard") then return end
	card = sgs.Card_Parse("@DimengCard=.&dimeng")
	return card
end

local dimeng_discard = function(self, discard_num, cards)
	local to_discard = {}

	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_PlaceEquip then
			if card:isKindOf("SilverLion") and self.player:isWounded() then return -2
			elseif card:isKindOf("OffensiveHorse") then return 1
			elseif card:isKindOf("Weapon") then return 2
			elseif card:isKindOf("DefensiveHorse") then return 3
			elseif card:isKindOf("Armor") then return 4
			end
		elseif self:getUseValue(card) >= 6 then return 3
		elseif self.player:hasSkills(sgs.lose_equip_skill) then return 5
		else return 0
		end
		return 0
	end

	local compare_func = function(a, b)
		if aux_func(a) ~= aux_func(b) then
			return aux_func(a) < aux_func(b)
		end
		return self:getKeepValue(a) < self:getKeepValue(b)
	end

	table.sort(cards, compare_func)
	for _, card in ipairs(cards) do
		if not self.player:isJilei(card) then table.insert(to_discard, card:getId()) end
		if #to_discard >= discard_num then break end
	end
	if #to_discard ~= discard_num then return {} end
	return to_discard
end

--要求：mycards是经过sortByKeepValue排序的--
function DimengIsWorth(self, friend, enemy, mycards, myequips)
	local e_hand1, e_hand2 = enemy:getHandcardNum(), enemy:getHandcardNum() - self:getLeastHandcardNum(enemy)
	local f_hand1, f_hand2 = friend:getHandcardNum(), friend:getHandcardNum() - self:getLeastHandcardNum(friend)
	local e_peach, f_peach = getCardsNum("Peach", enemy, self.player), getCardsNum("Peach", friend, self.player)
	if e_hand1 < f_hand1 then
		return false
	elseif e_hand2 <= f_hand2 and e_peach <= f_peach then
		return false
	elseif e_peach < f_peach and e_peach < 1 then
		return false
	elseif e_hand1 == f_hand1 and e_hand1 > 0 then
		return friend:hasShownSkill("tuntian")
	end
	local cardNum = #mycards
	local delt = e_hand1 - f_hand1 --assert: delt>0
	if delt > cardNum then
		return false
	end
	if #myequips > 0 and self.player:hasSkill("xiaoji") then return true end
	--now e_hand1>f_hand1 and delt<=cardNum
	local soKeep = 0
	local soUse = 0
	local marker = math.ceil(delt / 2)
	for i = 1, delt, 1 do
		local card = mycards[i]
		local keepValue = self:getKeepValue(card)
		if keepValue > 4 then
			soKeep = soKeep + 1
		end
		local useValue = self:getUseValue(card)
		if useValue >= 6 then
			soUse = soUse + 1
		end
	end
	if soKeep > marker then
		return false
	end
	if soUse > marker then
		return false
	end
	return true
end


sgs.ai_skill_use_func.DimengCard = function(card,use,self)
	local mycards = {}
	local myequips = {}
	local keepaslash
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if not self.player:isJilei(c) then
			local shouldUse
			if not keepaslash and isCard("Slash", c, self.player) then
				local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
				self:useBasicCard(c, dummy_use)
				if dummy_use.card and not dummy_use.to:isEmpty() and (dummy_use.to:length() > 1 or dummy_use.to:first():getHp() <= 1) then
					shouldUse = true
				end
			end
			if not shouldUse then table.insert(mycards, c) end
		end
	end
	for _, c in sgs.qlist(self.player:getEquips()) do
		if not self.player:isJilei(c) then
			table.insert(mycards, c)
			table.insert(myequips, c)
		end
	end
	if #mycards == 0 then return end
	self:sortByKeepValue(mycards)

	self:sort(self.enemies,"handcard")
	local friends = {}
	for _, player in ipairs(self.friends_noself) do
		table.insert(friends, player)
	end
	if #friends == 0 then return end

	self:sort(friends, "defense")
	local function cmp_HandcardNum(a, b)
		local x = a:getHandcardNum() - self:getLeastHandcardNum(a)
		local y = b:getHandcardNum() - self:getLeastHandcardNum(b)
		return x < y
	end
	table.sort(friends, cmp_HandcardNum)

	self:sort(self.enemies, "defense")

	for _, enemy in ipairs(self.enemies) do
		local e_hand = enemy:getHandcardNum()
		for _, friend in ipairs(friends) do
			local f_hand = friend:getHandcardNum()
			if DimengIsWorth(self, friend, enemy, mycards, myequips) and (e_hand > 0 or f_hand > 0) then
				if e_hand == f_hand then
					use.card = card
				else
					local discard_num = math.abs(e_hand - f_hand)
					local discards = dimeng_discard(self, discard_num, mycards)
					if #discards > 0 then use.card = sgs.Card_Parse("@DimengCard=" .. table.concat(discards, "+") .."&dimeng") end
				end
				if use.to then
					use.to:append(enemy)
					use.to:append(friend)
					end
				return
			end
		end
	end
end

sgs.ai_card_intention.DimengCard = function(self,card, from, to)
	local compare_func = function(a, b)
		return a:getHandcardNum() < b:getHandcardNum()
	end
	table.sort(to, compare_func)
	if to[1]:getHandcardNum() < to[2]:getHandcardNum() then
		sgs.updateIntention(from, to[1], -80)
	end
end

sgs.ai_use_value.DimengCard = 3.5
sgs.ai_use_priority.DimengCard = 2.8

sgs.dynamic_value.control_card.DimengCard = true

local zhijian_skill = {}
zhijian_skill.name = "zhijian"
table.insert(sgs.ai_skills, zhijian_skill)
zhijian_skill.getTurnUseCard = function(self)
	local equips = {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:getTypeId() == sgs.Card_TypeEquip then
			table.insert(equips, card)
		end
	end
	if #equips == 0 then return end

	return sgs.Card_Parse("@ZhijianCard=.&zhijian")
end

sgs.ai_skill_use_func.ZhijianCard = function(card, use, self)
	local equips = {}
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isKindOf("Armor") or card:isKindOf("Weapon") then
			if not self:getSameEquip(card) then
			elseif card:isKindOf("GudingBlade") and self:getCardsNum("Slash") > 0 then
				local HeavyDamage
				local slash = self:getCard("Slash")
				for _, enemy in ipairs(self.enemies) do
					if self.player:canSlash(enemy, slash, true) and not self:slashProhibit(slash, enemy)
						and self:slashIsEffective(slash, enemy) and enemy:isKongcheng() then
							HeavyDamage = true
							break
					end
				end
				if not HeavyDamage then table.insert(equips, card) end
			else
				table.insert(equips, card)
			end
		elseif card:getTypeId() == sgs.Card_TypeEquip then
			table.insert(equips, card)
		end
	end

	if #equips == 0 then return end

	local select_equip, target
	for _, friend in ipairs(self.friends_noself) do
		for _, equip in ipairs(equips) do
			if not self:getSameEquip(equip, friend) and friend:hasShownSkills(sgs.need_equip_skill .. "|" .. sgs.lose_equip_skill) then
				target = friend
				select_equip = equip
				break
			end
		end
		if target then break end
		for _, equip in ipairs(equips) do
			if not self:getSameEquip(equip, friend) then
				target = friend
				select_equip = equip
				break
			end
		end
		if target then break end
	end

	if not target then return end
	if use.to then use.to:append(target) end
	local zhijian = sgs.Card_Parse("@ZhijianCard=" .. select_equip:getId() .. "&zhijian")
	assert(zhijian)
	use.card = zhijian
end

sgs.ai_card_intention.ZhijianCard = -80
sgs.ai_use_priority.ZhijianCard = sgs.ai_use_priority.RendeCard + 0.1  -- 刘备二张双将的话，优先直谏
sgs.ai_cardneed.zhijian = sgs.ai_cardneed.equip

sgs.ai_skill_invoke.guzheng = function(self, data)
	if not self.player:hasShownOneGeneral() then
		if not  (self:willShowForAttack() or self:willShowForDefence())  and data:toInt() < 3  then
			return false
		end
	end
	local player = self.room:getCurrent()
	local invoke = (self:isFriend(player) and not self:needKongcheng(player, true))
					or (data:toInt() >= 3 or (data:toInt() == 2 and not player:hasShownSkills(sgs.cardneed_skill)))
					or (self:isEnemy(player) and self:needKongcheng(player, true))
	return invoke
end

sgs.ai_skill_askforag.guzheng = function(self, card_ids)
	local who = self.room:getCurrent()

	local cards, except_Equip, except_Key = {}, {}, {}
	for _, card_id in ipairs(card_ids) do
		local card = sgs.Sanguosha:getCard(card_id)
		if self.player:hasSkill("zhijian") and not card:isKindOf("EquipCard") then
			table.insert(except_Equip, card)
		end
		if not card:isKindOf("Peach") and not card:isKindOf("Jink") and not card:isKindOf("Analeptic") and
			not card:isKindOf("Nullification") and not (card:isKindOf("EquipCard") and self.player:hasSkill("zhijian")) then
			table.insert(except_Key, card)
		end
		table.insert(cards, card)
	end

	if self:isFriend(who) then
		local peach_num, peach, jink, analeptic, slash = 0
		for _, card in ipairs(cards) do
			if card:isKindOf("Peach") then peach = card:getEffectiveId() peach_num = peach_num + 1 end
			if card:isKindOf("Jink") then jink = card:getEffectiveId() end
			if card:isKindOf("Analeptic") then analeptic = card:getEffectiveId() end
			if card:isKindOf("Slash") then slash = card:getEffectiveId() end
		end
		if peach then
			if peach_num > 1
				or (self:getCardsNum("Peach") >= self.player:getMaxCards())
				or who:getHp() < self.player:getHp() then
					return peach
			end
		end
		if self:isWeak(who) and (jink or analeptic) then
			return jink or analeptic
		end

		for _, card in ipairs(cards) do
			if not card:isKindOf("EquipCard") then
				for _, askill in sgs.qlist(who:getVisibleSkillList()) do
					local callback = sgs.ai_cardneed[askill:objectName()]
					if type(callback)=="function" and callback(who, card, self) then
						return card:getEffectiveId()
					end
				end
			end
		end

		if jink or analeptic or slash then
			return jink or analeptic or slash
		end

		for _, card in ipairs(cards) do
			if not card:isKindOf("EquipCard") and not card:isKindOf("Peach") then
				return card:getEffectiveId()
			end
		end
	else
		for _, card in ipairs(cards) do
			if card:isKindOf("EquipCard") and self.player:hasSkill("zhijian") then
				local Cant_Zhijian = true
				for _, friend in ipairs(self.friends) do
					if not self:getSameEquip(card, friend) then
						Cant_Zhijian = false
					end
				end
				if Cant_Zhijian then
					return card:getEffectiveId()
				end
			end
		end

		local new_cards = (#except_Key > 0 and except_Key) or (#except_Equip > 0 and except_Equip) or cards

		self:sortByKeepValue(new_cards)
		local valueless, slash
		for _, card in ipairs(new_cards) do
			if card:isKindOf("Lightning") and not who:hasShownSkills(sgs.wizard_harm_skill) then
				return card:getEffectiveId()
			end

			if card:isKindOf("Slash") then slash = card:getEffectiveId() end

			if not valueless and not card:isKindOf("Peach") then
				for _, askill in sgs.qlist(who:getVisibleSkillList()) do
					local callback = sgs.ai_cardneed[askill:objectName()]
					if (type(callback) == "function" and not callback(who, card, self)) or not callback then
						valueless = card:getEffectiveId()
						break
					end
				end
			end
		end

		if slash or valueless then
			return slash or valueless
		end

		return new_cards[1]:getEffectiveId()
	end

	return card_ids[1]
end


local fenxun_skill = {}
fenxun_skill.name = "fenxun"
table.insert(sgs.ai_skills, fenxun_skill)
fenxun_skill.getTurnUseCard = function(self)
	if not self:willShowForAttack() then return end
	if self.player:hasUsed("FenxunCard") then return end
	if self.player:isNude() then return end
	return sgs.Card_Parse("@FenxunCard=.&fenxun")
end

sgs.ai_skill_use_func.FenxunCard = function(card, use, self)
	local shouldUse, slashCard = true
	local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
	for _, slash in ipairs(self:getCards("Slash")) do
		dummy_use.to = sgs.SPlayerList()
		dummy_use.card = nil
		self:useCardSlash(slash, dummy_use)
		if dummy_use.to:length() > 1 then
			local x = 0
			for _, to in sgs.qlist(dummy_use.to) do
				if self.player:distanceTo(to) > 1 then x = x + 1 end
			end
			if x <= 1 then shouldUse = false end
			slashCard = slash
			break
		end
	end

	if self:needToThrowArmor() then
		use.card = sgs.Card_Parse("@FenxunCard=" .. self.player:getArmor():getId() .. "&fenxun")
		if use.to then
			if not dummy_use.to:isEmpty() then
				use.to:append(dummy_use.to:first())
			else use.to:append(self.room:getOtherPlayers(self.player):first())
			end
			return
		end
	else
		if #self.enemies == 0 then return end
		if not self:slashIsAvailable() then return end
		if not shouldUse then return end
		if not slashCard then return end
		if dummy_use.to:isEmpty() then return end
		local cards = {}
		for _, c in sgs.qlist(self.player:getCards("he")) do
			if c:getEffectiveId() ~= slashCard:getEffectiveId() then table.insert(cards, c) end
		end
		self:sortByKeepValue(cards)

		local card_id
		for _, c in ipairs(cards) do
			if c:isKindOf("Lightning") and not isCard("Peach", c, self.player) and not self:willUseLightning(c) then
				card_id = c:getEffectiveId()
				break
			end
		end

		if not card_id then
			for _, c in ipairs(cards) do
				if not isCard("Peach", c, self.player)
					and (c:isKindOf("AmazingGrace") or c:isKindOf("GodSalvation") and not self:willUseGodSalvation(c)) then
					card_id = c:getEffectiveId()
					break
				end
			end
		end

		if not card_id then
			local isWeak
			for _, to in sgs.qlist(dummy_use.to) do
				if self:isWeak(to) and to:getHp() <= 1 then isWeak = true break end
			end

			for _, c in ipairs(cards) do
				if (not isCard("Peach", c, self.player) or self:getCardsNum("Peach") > 1)
					and (not isCard("Jink", c, self.player) or self:getCardsNum("Jink") > 1 or isWeak)
					and not (self.player:getWeapon() and self.player:getWeapon():getEffectiveId() == c:getEffectiveId())
					and not (self.player:getOffensiveHorse() and self.player:getOffensiveHorse():getEffectiveId() == c:getEffectiveId()) then
					card_id = c:getEffectiveId()
				end
			end
		end
		if card_id then
			use.card = sgs.Card_Parse("@FenxunCard=" .. card_id .. "&fenxun")
			if use.to then
				for _, to in sgs.qlist(dummy_use.to) do
					if self.player:distanceTo(to) > 1 then use.to:append(to) break end
				end
				if use.to:isEmpty() then use.to:append(dummy_use.to:first()) end
			end
		end
	end
end

sgs.ai_use_value.FenxunCard = 5.5
sgs.ai_use_priority.FenxunCard = 8
sgs.ai_card_intention.FenxunCard = 50

sgs.ai_skill_invoke.keji = function(self, data)
	if sgs.isAnjiang(self.player) and self:getOverflow() <= 0 then
		return
	elseif not self:willShowForDefence() and not self.player:hasSkill("tianxiang") then
		return false
	end
	return true
end
