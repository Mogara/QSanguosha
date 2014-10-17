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

function SmartAI:shouldUseRende()
	if (self:hasCrossbowEffect() or self:getCardsNum("Crossbow") > 0 or self.player:hasSkill("paoxiao") ) and self:getCardsNum("Slash") > 0 	then
		self:sort(self.enemies, "defense")
		for _, enemy in ipairs(self.enemies) do
			local inAttackRange = self.player:distanceTo(enemy) == 1 or self.player:distanceTo(enemy) == 2
									and self:getCardsNum("OffensiveHorse") > 0 and not self.player:getOffensiveHorse()
			local inPaoxiaoAttackRange =  self.player:distanceTo(enemy) <= self.player:getAttackRange()	and	self.player:hasSkill("paoxiao")
			if (inAttackRange or inPaoxiaoAttackRange) and sgs.isGoodTarget(enemy, self.enemies, self) then
				local slashs = self:getCards("Slash")
				local slash_count = 0
				for _, slash in ipairs(slashs) do
					if not self:slashProhibit(slash, enemy) and self:slashIsEffective(slash, enemy) then
						slash_count = slash_count + 1
					end
				end
				if slash_count >= enemy:getHp() then return false end
			end
		end
	end

	for _, enemy in ipairs(self.enemies) do
		if enemy:canSlash(self.player) and not self:slashProhibit(nil, self.player, enemy)
			and self:hasCrossbowEffect(enemy) and getCardsNum("Slash", enemy) > 1 and self:getOverflow() <= 0 then
			return false
		end
	end
	for _, player in ipairs(self.friends_noself) do
		if (player:hasShownSkill("haoshi") and not player:containsTrick("supply_shortage")) or player:hasShownSkill("jijiu") then
			return true
		end
	end

	local keepNum = 1
	if self.player:getMark("rende") == 0 then
		if self.player:getHandcardNum() == 3 then
			keepNum = 0
		end
		if self.player:getHandcardNum() > 3 then
			keepNum = 3
		end
	end
	if self.player:hasSkill("kongcheng") then
		keepNum = 0
	end

	if self:getOverflow() > 0  then
		return true
	end
	if self.player:getHandcardNum() > keepNum  then
		return true
	end
	if self.player:getMark("rende") ~= 0 and self.player:getMark("rende") < 3
		and (3 - self.player:getMark("rende")) >=  (self.player:getHandcardNum() - keepNum) then
		return true
	end

	if self.player:hasSkill("kongcheng") then
		return true
	end

	return false
end

local rende_skill = {}
rende_skill.name = "rende"
table.insert(sgs.ai_skills, rende_skill)
rende_skill.getTurnUseCard = function(self)
	if self.player:isKongcheng() then return end

	if self:shouldUseRende() then
		return sgs.Card_Parse("@RendeCard=.&rende")
	end
end

sgs.ai_skill_use_func.RendeCard = function(card, use, self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByUseValue(cards, true)

	for i = 1, #cards do
		local card, friend = self:getCardNeedPlayer(cards, nil, "rende")
		if card and friend then
			cards = self:resetCards(cards, card)
		else
			break
		end
		if self.player:getHandcardNum() < 3 and self.player:hasSkill("kongcheng") then
			for _, p in ipairs(self.friends_noself) do
				friend = p
			end
		end

		if friend:objectName() == self.player:objectName() or not self.player:getHandcards():contains(card) then continue end

		if card:isAvailable(self.player) and (card:isKindOf("Slash") or card:isKindOf("Duel") or card:isKindOf("Snatch") or card:isKindOf("Dismantlement")) then
			local dummy_use = { isDummy = true, to = sgs.SPlayerList() }
			local cardtype = card:getTypeId()
			self["use" .. sgs.ai_type_name[cardtype + 1] .. "Card"](self, card, dummy_use)
			if dummy_use.card and dummy_use.to:length() > 0 then
				if card:isKindOf("Slash") or card:isKindOf("Duel") then
					local t1 = dummy_use.to:first()
					if dummy_use.to:length() > 1 then continue
					elseif t1:getHp() == 1 or sgs.card_lack[t1:objectName()]["Jink"] == 1
							or t1:isCardLimited(sgs.cloneCard("jink"), sgs.Card_MethodResponse) then continue
					end
				elseif (card:isKindOf("Snatch") or card:isKindOf("Dismantlement")) and self:getEnemyNumBySeat(self.player, friend) > 0 then
					local hasDelayedTrick
					for _, p in sgs.qlist(dummy_use.to) do
						if self:isFriend(p) and (self:willSkipDrawPhase(p) or self:willSkipPlayPhase(p)) then hasDelayedTrick = true break end
					end
					if hasDelayedTrick then continue end
				end
			end
		elseif card:isAvailable(self.player) and self:getEnemyNumBySeat(self.player, friend) > 0 and (card:isKindOf("Indulgence") or card:isKindOf("SupplyShortage")) then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then continue end
		end

		use.card = sgs.Card_Parse("@RendeCard=" .. card:getId() .. "&rende")
		if use.to then use.to:append(friend) return end
	end

end

sgs.ai_use_value.RendeCard = 8.5
sgs.ai_use_priority.RendeCard = 8.2

sgs.ai_card_intention.RendeCard = function(self, card, from, tos)
	local to = tos[1]
	local intention = -70
	if to:hasShownSkill("kongcheng") and to:isKongcheng() then
		intention = 30
	end
	sgs.updateIntention(from, to, intention)
end

sgs.dynamic_value.benefit.RendeCard = true

sgs.ai_view_as.wusheng = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place ~= sgs.Player_PlaceSpecial and (player:getLord() and player:getLord():hasShownSkill("shouyue") or card:isRed()) and not card:isKindOf("Peach") and not card:hasFlag("using") then
		return ("slash:wusheng[%s:%s]=%d&wusheng"):format(suit, number, card_id)
	end
end

local wusheng_skill = {}
wusheng_skill.name = "wusheng"
table.insert(sgs.ai_skills, wusheng_skill)
wusheng_skill.getTurnUseCard = function(self, inclusive)

	self:sort(self.enemies, "defense")
	local useAll = false
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() == 1 and not enemy:hasArmorEffect("EightDiagram") and self.player:distanceTo(enemy) <= self.player:getAttackRange()  then
			useAll = true
			break
		end
	end

	local cards = {}
	for _, card in sgs.qlist(self.player:getCards("he")) do
		if (self.player:getLord() and self.player:getLord():hasShownSkill("shouyue") or card:isRed()) and not card:isKindOf("Slash")
			and ((not isCard("Peach", card, self.player) and not isCard("ExNihilo", card, self.player)) or useAll) then
			local suit = card:getSuitString()
			local number = card:getNumberString()
			local card_id = card:getEffectiveId()
			local card_str = ("slash:wusheng[%s:%s]=%d&wusheng"):format(suit, number, card_id)
			local slash = sgs.Card_Parse(card_str)
			assert(slash)
			if self:slashIsAvailable(self.player, slash) then
				table.insert(cards, slash)
			end
		end
	end

	if #cards == 0 then return end

	self:sortByUsePriority(cards)
	return cards[1]
end

function sgs.ai_cardneed.wusheng(to, card)
	return to:getHandcardNum() < 3 and card:isRed()
end

sgs.ai_suit_priority.wusheng= "club|spade|diamond|heart"

function sgs.ai_cardneed.paoxiao(to, card, self)
	local cards = to:getHandcards()
	local has_weapon = to:getWeapon() and not to:getWeapon():isKindOf("Crossbow")
	local slash_num = 0
	for _, c in sgs.qlist(cards) do
		local flag=string.format("%s_%s_%s","visible",self.room:getCurrent():objectName(),to:objectName())
		if c:hasFlag("visible") or c:hasFlag(flag) then
			if c:isKindOf("Weapon") and not c:isKindOf("Crossbow") then
				has_weapon=true
			end
			if c:isKindOf("Slash") then slash_num = slash_num +1 end
		end
	end

	if not has_weapon then
		return card:isKindOf("Weapon") and not card:isKindOf("Crossbow")
	else
		return to:hasWeapon("Spear") or card:isKindOf("Slash") or (slash_num > 1 and card:isKindOf("Analeptic"))
	end
end

sgs.paoxiao_keep_value = {
	Peach = 6,
	Analeptic = 5.8,
	Jink = 5.7,
	FireSlash = 5.6,
	Slash = 5.4,
	ThunderSlash = 5.5,
	ExNihilo = 4.7
}


local longdan_skill = {}
longdan_skill.name = "longdan"
table.insert(sgs.ai_skills, longdan_skill)
longdan_skill.getTurnUseCard = function(self)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local jink_card

	self:sortByUseValue(cards,true)

	for _,card in ipairs(cards)  do
		if card:isKindOf("Jink") then
			jink_card = card
			break
		end
	end

	if not jink_card then return nil end
	local suit = jink_card:getSuitString()
	local number = jink_card:getNumberString()
	local card_id = jink_card:getEffectiveId()
	local card_str = ("slash:longdan[%s:%s]=%d&longdan"):format(suit, number, card_id)
	local slash = sgs.Card_Parse(card_str)
	assert(slash)

	return slash

end

sgs.ai_view_as.longdan = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceHand then
		if card:isKindOf("Jink") then
			return ("slash:longdan[%s:%s]=%d&longdan"):format(suit, number, card_id)
		elseif card:isKindOf("Slash") then
			return ("jink:longdan[%s:%s]=%d&longdan"):format(suit, number, card_id)
		end
	end
end

sgs.longdan_keep_value = {
	Jink = 5.2,
	FireSlash = 5.21,
	Slash = 5.2,
	ThunderSlash = 5.22,
	ExNihilo = 4.3
}

sgs.ai_skill_invoke.tieqi = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) then return false end

	local zj = sgs.findPlayerByShownSkillName("guidao")
	if zj and self:isEnemy(zj) and self:canRetrial(zj) then return false end
	return true
end


function sgs.ai_cardneed.jizhi(to, card)
	return card:getTypeId() == sgs.Card_TypeTrick
end

sgs.jizhi_keep_value = {
	Peach 		= 6,
	Analeptic 	= 5.9,
	Jink 		= 5.8,
	ExNihilo	= 5.7,
	Snatch 		= 5.7,
	Dismantlement = 5.6,
	IronChain 	= 5.5,
	SavageAssault=5.4,
	Duel 		= 5.3,
	ArcheryAttack = 5.2,
	AmazingGrace = 5.1,
	Collateral 	= 5,
	FireAttack	=4.9
}



sgs.ai_skill_invoke.liegong = function(self, data)
	local target = data:toPlayer()
	return not self:isFriend(target)
end

function SmartAI:canLiegong(to, from)
	from = from or self.room:getCurrent()
	to = to or self.player
	if not from then return false end
	if from:hasShownSkill("liegong") and from:getPhase() == sgs.Player_Play and (to:getHandcardNum() >= from:getHp() or to:getHandcardNum() <= from:getAttackRange()) then return true end
	return false
end

function sgs.ai_cardneed.kuanggu(to, card, self)
	return card:isKindOf("OffensiveHorse") and not (to:getOffensiveHorse() or getKnownCard(to, self.player, "OffensiveHorse", false) > 0)
end

local lianhuan_skill = {}
lianhuan_skill.name = "lianhuan"
table.insert(sgs.ai_skills, lianhuan_skill)
lianhuan_skill.getTurnUseCard = function(self)

	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)

	local card
	self:sortByUseValue(cards, true)

	local slash = self:getCard("FireSlash") or self:getCard("ThunderSlash") or self:getCard("Slash")
	if slash then
		local dummy_use = { isDummy = true }
		self:useBasicCard(slash, dummy_use)
		if not dummy_use.card then slash = nil end
	end

	for _, acard in ipairs(cards) do
		if acard:getSuit() == sgs.Card_Club then
			local shouldUse = true
			if self:getUseValue(acard) > sgs.ai_use_value.IronChain and acard:getTypeId() == sgs.Card_TypeTrick then
				local dummy_use = { isDummy = true }
				self:useTrickCard(acard, dummy_use)
				if dummy_use.card then shouldUse = false end
			end
			if acard:getTypeId() == sgs.Card_TypeEquip then
				local dummy_use = { isDummy = true }
				self:useEquipCard(acard, dummy_use)
				if dummy_use.card then shouldUse = false end
			end
			if shouldUse and (not slash or slash:getEffectiveId() ~= acard:getEffectiveId()) then
				card = acard
				break
			end
		end
	end

	if not self:willShowForAttack() then
		return nil
	end
	if not card then return nil end
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("iron_chain:lianhuan[club:%s]=%d%s"):format(number, card_id, "&lianhuan")
	local skillcard = sgs.Card_Parse(card_str)
	assert(skillcard)
	return skillcard
end

sgs.ai_cardneed.lianhuan = function(to, card)
	return card:getSuit() == sgs.Card_Club and to:getHandcardNum() <= 2
end

sgs.ai_skill_invoke.niepan = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()
	return self:getCardsNum("Peach") + self:getCardsNum("Analeptic") < peaches
end

sgs.ai_suit_priority.lianhuan= "club|diamond|heart|spade"


local huoji_skill = {}
huoji_skill.name = "huoji"
table.insert(sgs.ai_skills, huoji_skill)
huoji_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)

	local card

	self:sortByUseValue(cards,true)

	for _,acard in ipairs(cards) do
		if acard:isRed() and not isCard("Peach", acard, self.player) and (self:getDynamicUsePriority(acard) < sgs.ai_use_value.FireAttack or self:getOverflow() > 0) then
			if acard:isKindOf("Slash") and self:getCardsNum("Slash") == 1 then
				local keep
				local dummy_use = { isDummy = true , to = sgs.SPlayerList() }
				self:useBasicCard(acard, dummy_use)
				if dummy_use.card and dummy_use.to and dummy_use.to:length() > 0 then
					for _, p in sgs.qlist(dummy_use.to) do
						if p:getHp() <= 1 then keep = true break end
					end
					if dummy_use.to:length() > 1 then keep = true end
				end
				if keep then sgs.ai_use_priority.Slash = sgs.ai_use_priority.FireAttack + 0.1
				else
					sgs.ai_use_priority.Slash = 2.6
					card = acard
					break
				end
			else
				card = acard
				break
			end
		end
	end

	if not self:willShowForAttack() then
		return nil
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("fire_attack:huoji[%s:%s]=%d%s"):format(suit, number, card_id, "&huoji")
	local skillcard = sgs.Card_Parse(card_str)

	assert(skillcard)
	return skillcard
end

sgs.ai_cardneed.huoji = function(to, card, self)
	return to:getHandcardNum() >= 2 and card:isRed()
end

sgs.ai_suit_priority.huoji= "club|spade|diamond|heart"


sgs.ai_view_as.kanpo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceHand then
		if card:isBlack() then
			return ("nullification:kanpo[%s:%s]=%d%s"):format(suit, number, card_id, "&kanpo")
		end
	end
end

sgs.ai_cardneed.kanpo = function(to, card, self)
	return card:isBlack()
end

sgs.ai_suit_priority.kanpo= "diamond|heart|club|spade"

sgs.kanpo_suit_value = {
	spade = 3.9,
	club = 3.9
}

sgs.ai_skill_invoke.bazhen = function(self, data)
	if (not self:willShowForDefence() and self:getCardsNum("Jink") > 0) then
		return false
	end
	return sgs.ai_skill_invoke.EightDiagram
end

function sgs.ai_armor_value.bazhen(card)
	if not card then return 4 end
end

sgs.ai_skill_invoke.xiangle = function(self, data)
	local use = data:toCardUse()
	return not self:needToLoseHp(self.player, use.from, true)
end

sgs.ai_skill_cardask["@xiangle-discard"] = function(self, data)
	local target = data:toPlayer()
	if self:isFriend(target) and not self:findLeijiTarget(target, 50, self.player) then return "." end
	local has_peach, has_analeptic, has_slash, has_jink
	for _, card in sgs.qlist(self.player:getHandcards()) do
		if card:isKindOf("Peach") then has_peach = card
		elseif card:isKindOf("Analeptic") then has_analeptic = card
		elseif card:isKindOf("Slash") then has_slash = card
		elseif card:isKindOf("Jink") then has_jink = card
		end
	end

	if has_slash then return "$" .. has_slash:getEffectiveId()
	elseif has_jink then return "$" .. has_jink:getEffectiveId()
	elseif has_analeptic or has_peach then
		if getCardsNum("Jink", target, self.player) == 0 and self.player:getMark("drank") > 0 and self:getAllPeachNum(target) == 0 then
			if has_analeptic then return "$" .. has_analeptic:getEffectiveId()
			else return "$" .. has_peach:getEffectiveId()
			end
		end
	else return "."
	end
end

function sgs.ai_slash_prohibit.xiangle(self, from, to)
	if self:isFriend(to, from) then return false end
	local slash_num, analeptic_num, jink_num
	if from:objectName() == self.player:objectName() then
		slash_num = self:getCardsNum("Slash")
		analeptic_num = self:getCardsNum("Analeptic")
		jink_num = self:getCardsNum("Jink")
	else
		slash_num = getCardsNum("Slash", from, self.player)
		analeptic_num = getCardsNum("Analpetic", from, self.player)
		jink_num = getCardsNum("Jink", from, self.player)
	end
	if self.player:getHandcardNum() == 2 then
		local needkongcheng = self:needKongcheng()
		if needkongcheng then return slash_num + analeptic_num + jink_num < 2 end
	end
	return slash_num + analeptic_num + math.max(jink_num - 1, 0) < 2
end

sgs.ai_skill_invoke.fangquan = function(self, data)
	self.fangquan_card_str = nil
	self.fangquan_target = nil
	if #self.friends == 1 then
		return false
	end


	-- First we'll judge whether it's worth skipping the Play Phase
	local cards = sgs.QList2Table(self.player:getHandcards())
	local shouldUse, range_fix = 0, 0
	local hasCrossbow, slashTo = false, false
	for _, card in ipairs(cards) do
		if card:isKindOf("TrickCard") and self:getUseValue(card) > 3.69 then
			local dummy_use = { isDummy = true }
			self:useTrickCard(card, dummy_use)
			if dummy_use.card then shouldUse = shouldUse + (card:isKindOf("ExNihilo") and 2 or 1) end
		end
		if card:isKindOf("Weapon") then
			local new_range = sgs.weapon_range[card:getClassName()]
			local current_range = self.player:getAttackRange()
			range_fix = math.min(current_range - new_range, 0)
		end
		if card:isKindOf("OffensiveHorse") and not self.player:getOffensiveHorse() then range_fix = range_fix - 1 end
		if card:isKindOf("DefensiveHorse") or card:isKindOf("Armor") and not self:getSameEquip(card) and (self:isWeak() or self:getCardsNum("Jink") == 0) then shouldUse = shouldUse + 1 end
		if card:isKindOf("Crossbow") or self:hasCrossbowEffect() then hasCrossbow = true end
	end


	local slashs = self:getCards("Slash")
	for _, enemy in ipairs(self.enemies) do
		for _, slash in ipairs(slashs) do
			if hasCrossbow and self:getCardsNum("Slash") > 1 and self:slashIsEffective(slash, enemy)
				and self.player:canSlash(enemy, slash, true, range_fix) then
				shouldUse = shouldUse + 2
				hasCrossbow = false
				break
			elseif not slashTo and self:slashIsAvailable() and self:slashIsEffective(slash, enemy)
				and self.player:canSlash(enemy, slash, true, range_fix) and getCardsNum("Jink", enemy, self.player) < 1 then
				shouldUse = shouldUse + 1
				slashTo = true
			end
		end
	end
	if shouldUse >= 2 then return end


	-- Then we need to find the card to be discarded
	local limit = self.player:getMaxCards()
	if self.player:isKongcheng() then return false end
	if self:getCardsNum("Peach") >= limit - 2 and self.player:isWounded() then return false end


	local to_discard = nil


	local index = 0
	local all_peaches = 0
	for _, card in ipairs(cards) do
		if isCard("Peach", card, self.player) then
			all_peaches = all_peaches + 1
		end
	end
	if all_peaches >= 2 and self:getOverflow() <= 0 then return false end
	self:sortByKeepValue(cards)
	cards = sgs.reverse(cards)


	for i = #cards, 1, -1 do
		local card = cards[i]
		if not isCard("Peach", card, self.player) and not self.player:isJilei(card) then
			to_discard = card:getEffectiveId()
			break
		end
	end
	if to_discard == nil then return false end


	-- At last we try to find the target


	local AssistTarget = self:AssistTarget()
	if AssistTarget and not self:willSkipPlayPhase(AssistTarget) then
		self.fangquan_target = AssistTarget
		self.fangquan_card_str = "@FangquanCard=" .. to_discard .. "&fangquan->" .. AssistTarget:objectName()
		return true
	end


	self:sort(self.friends_noself, "handcard")
	self.friends_noself = sgs.reverse(self.friends_noself)
	for _, target in ipairs(self.friends_noself) do
		if target:hasShownSkills("zhiheng|" .. sgs.priority_skill .. "|shensu") and (not self:willSkipPlayPhase(target) or target:hasShownSkill("shensu")) then
			self.fangquan_target = target
			self.fangquan_card_str = "@FangquanCard=" .. to_discard .. "&fangquan->" .. target:objectName()
			return true
		end
	end


	return false
end

sgs.ai_skill_use["@@fangquan"] = function(self, prompt)
	local fangquan_card = sgs.Card_Parse(self.fangquan_card_str)
	local in_handcard = true
	for _, id in sgs.qlist(fangquan_card:getSubcards()) do
		if not self.player:handCards():contains(id) then
			in_handcard = false
			break
		end
	end
	if in_handcard then return self.fangquan_card_str end

	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	cards = sgs.reverse(cards)

	if self.fangquan_target then
		for i = #cards, 1, -1 do
			local card = cards[i]
			if not isCard("Peach", card, self.player) and not self.player:isJilei(card) then
				return "@FangquanCard=" .. card:getEffectiveId() .. "&fangquan->" .. self.fangquan_target:objectName()
			end
		end
	end
end


sgs.ai_card_intention.FangquanCard = -120

sgs.ai_skill_invoke.zaiqi = function(self, data)
	local lostHp = 2
	if self.player:hasSkill("rende") and #self.friends_noself > 0 and not self:willSkipPlayPhase() then lostHp = 3 end
	return self.player:getLostHp() >= lostHp
end

sgs.ai_cardneed.lieren = function(to, card, self)
	return isCard("Slash", card, to) and getKnownCard(to, self.player, "Slash", true) == 0
end

sgs.ai_skill_invoke.lieren = function(self, data)
	local damage = data:toDamage()
	if not damage.to then return end
	if not self:isEnemy(damage.to) then return false end

	if self.player:getHandcardNum() == 1 then
		if (self:needKongcheng() or not self:hasLoseHandcardEffective()) and not self:isWeak() then return true end
		local card = self.player:getHandcards():first()
		if card:isKindOf("Jink") or card:isKindOf("Peach") then return end
	end

	if (self.player:getHandcardNum() >= self.player:getHp() or self:getMaxCard():getNumber() > 10
		or (self:needKongcheng() and self.player:getHandcardNum() == 1) or not self:hasLoseHandcardEffective())
		and not self:doNotDiscard(damage.to, "h", true) and not (self.player:getHandcardNum() == 1 and self:doNotDiscard(damage.to, "e", true)) then
			return true
	end
	if self:doNotDiscard(damage.to, "he", true, 2) then return false end
	return false
end

function sgs.ai_skill_pindian.lieren(minusecard, self, requestor)
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if requestor:objectName() == self.player:objectName() then
		return cards[1]:getId()
	end
	return self:getMaxCard(self.player):getId()
end

sgs.ai_skill_invoke.shushen = true

sgs.ai_skill_playerchosen.shushen = function(self, targets)
	if #self.friends_noself == 0 then return nil end
	return self:findPlayerToDraw(false, 1)
end

sgs.ai_card_intention.ShushenCard = -80

sgs.ai_skill_invoke.shenzhi = function(self, data)
	if self:getCardsNum("Peach") > 0 then return false end
	if self.player:getHandcardNum() >= 3 then return false end
	if self.player:getHandcardNum() >= self.player:getHp() and self.player:isWounded() then return true end
	return false
end

function sgs.ai_cardneed.shenzhi(to, card)
	return to:getHandcardNum() < to:getHp()
end

sgs.ai_skill_invoke.huoshou = true

sgs.ai_skill_invoke.juxiang = true

sgs.ai_skill_invoke.kongcheng = true

sgs.ai_skill_invoke.guanxing = function(self, data)
	if not self:willShowForDefence() and not self:willShowForAttack() and self.player:getJudgingArea():isEmpty() then
		return false
	end
	return true
end

