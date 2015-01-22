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

sgs.ai_skill_invoke.tuntian = function(self, data)
	if not (self:willShowForAttack() or self:willShowForDefence()) then
		return false
	end
	return true
end
sgs.ai_skill_invoke._tuntian = function(self, data)
	if not (self:willShowForAttack() or self:willShowForDefence()) then
		return false
	end
	return true
end

local jixi_skill = {}
jixi_skill.name = "jixi"
table.insert(sgs.ai_skills, jixi_skill)
jixi_skill.getTurnUseCard = function(self)
	if self.player:getPile("field"):isEmpty()
		or (self.player:getHandcardNum() >= self.player:getHp() + 2
			and self.player:getPile("field"):length() <= self.room:getAlivePlayers():length() / 2 - 1) then
		return
	end
	local can_use = false
	for i = 0, self.player:getPile("field"):length() - 1, 1 do
		local snatch = sgs.Sanguosha:getCard(self.player:getPile("field"):at(i))
		local snatch_str = ("snatch:jixi[%s:%s]=%d&jixi"):format(snatch:getSuitString(), snatch:getNumberString(), self.player:getPile("field"):at(i))
		local jixisnatch = sgs.Card_Parse(snatch_str)
		assert(jixisnatch)

		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if (self.player:distanceTo(player, 1) <= 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_DistanceLimit, self.player, jixisnatch))
				and self:hasTrickEffective(jixisnatch, player) then

				local suit = snatch:getSuitString()
				local number = snatch:getNumberString()
				local card_id = snatch:getEffectiveId()
				local card_str = ("snatch:jixi[%s:%s]=%d%s"):format(suit, number, card_id, "&jixi")
				local snatch = sgs.Card_Parse(card_str)
				assert(snatch)
				return snatch
			end
		end
	end
end

sgs.ai_view_as.jixi = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceSpecial and player:getPileName(card_id) == "field" then
		return ("snatch:jixi[%s:%s]=%d%s"):format(suit, number, card_id, "&jixi")
	end
end

local getZiliangCard = function(self, damage)
	if not (damage.to:getPhase() == sgs.Player_NotActive and self:needKongcheng(damage.to, true)) then
		local ids = sgs.QList2Table(self.player:getPile("field"))
		local cards = {}
		for _, id in ipairs(ids) do table.insert(cards, sgs.Sanguosha:getCard(id)) end
		for _, card in ipairs(cards) do
			if card:isKindOf("Peach") or card:isKindOf("Analeptic") then return card:getEffectiveId() end
		end
		for _, card in ipairs(cards) do
			if card:isKindOf("Jink") then return card:getEffectiveId() end
		end
		self:sortByKeepValue(cards, true)
		return cards[1]:getEffectiveId()
	else
		return nil
	end
end

sgs.ai_skill_use["@@ziliang"] = function(self)
	local damage = self.player:getTag("ziliang_aidata"):toDamage()
	local id = getZiliangCard(self, damage)
	if id then
		return "@ZiliangCard=" .. tostring(id) .. "&ziliang"
	end
	return "."
end

local function huyuan_validate(self, equip_type, is_handcard)
	local targets = {}
	if is_handcard then targets = self.friends else targets = self.friends_noself end
	if equip_type == "SilverLion" then
		for _, enemy in ipairs(self.enemies) do
			if enemy:hasShownSkill("bazhen") and not enemy:getArmor() then table.insert(targets, enemy) end
		end
	end
	for _, friend in ipairs(targets) do
		local has_equip = false
		for _, equip in sgs.qlist(friend:getEquips()) do
			if equip:isKindOf(equip_type == "SilverLion" and "Armor" or equip_type) then
				has_equip = true
				break
			end
		end
		if not has_equip and not ((equip_type == "Armor" or equip_type == "SilverLion") and friend:hasShownSkill("bazhen")) then
			self:sort(self.enemies, "defense")
			for _, enemy in ipairs(self.enemies) do
				if friend:distanceTo(enemy) == 1 and self.player:canDiscard(enemy, "he") then
					enemy:setFlags("AI_HuyuanToChoose")
					return friend
				end
			end
		end
	end
	return nil
end

sgs.ai_skill_use["@@huyuan"] = function(self, prompt)
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByKeepValue(cards)
	if self.player:hasArmorEffect("SilverLion") then
		local player = huyuan_validate(self, "SilverLion", false)
		if player then return "@HuyuanCard=" .. self.player:getArmor():getEffectiveId() .. "->" .. player:objectName() end
	end
	if self.player:getOffensiveHorse() then
		local player = huyuan_validate(self, "OffensiveHorse", false)
		if player then return "@HuyuanCard=" .. self.player:getOffensiveHorse():getEffectiveId() .. "->" .. player:objectName() end
	end
	if self.player:getWeapon() then
		local player = huyuan_validate(self, "Weapon", false)
		if player then return "@HuyuanCard=" .. self.player:getWeapon():getEffectiveId() .. "->" .. player:objectName() end
	end
	if self.player:getArmor() and self.player:getLostHp() <= 1 and self.player:getHandcardNum() >= 3 then
		local player = huyuan_validate(self, "Armor", false)
		if player then return "@HuyuanCard=" .. self.player:getArmor():getEffectiveId() .. "->" .. player:objectName() end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("DefensiveHorse") then
			local player = huyuan_validate(self, "DefensiveHorse", true)
			if player then return "@HuyuanCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("OffensiveHorse") then
			local player = huyuan_validate(self, "OffensiveHorse", true)
			if player then return "@HuyuanCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("Weapon") then
			local player = huyuan_validate(self, "Weapon", true)
			if player then return "@HuyuanCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
	end
	for _, card in ipairs(cards) do
		if card:isKindOf("SilverLion") then
			local player = huyuan_validate(self, "SilverLion", true)
			if player then return "@HuyuanCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
		if card:isKindOf("Armor") and huyuan_validate(self, "Armor", true) then
			local player = huyuan_validate(self, "Armor", true)
			if player then return "@HuyuanCard=" .. card:getEffectiveId() .. "->" .. player:objectName() end
		end
	end
end

sgs.ai_skill_playerchosen.huyuan = function(self, targets)
	targets = sgs.QList2Table(targets)
	for _, p in ipairs(targets) do
		if p:hasFlag("AI_HuyuanToChoose") then
			p:setFlags("-AI_HuyuanToChoose")
			return p
		end
	end
	return targets[1]
end

sgs.ai_card_intention.HuyuanCard = function(self, card, from, to)
	if to[1]:hasShownSkill("bazhen") then
		if sgs.Sanguosha:getCard(card:getEffectiveId()):isKindOf("SilverLion") then
			sgs.updateIntention(from, to[1], 10)
			return
		end
	end
	sgs.updateIntention(from, to[1], -50)
end

sgs.ai_cardneed.huyuan = sgs.ai_cardneed.equip

sgs.huyuan_keep_value = {
	Peach = 6,
	Jink = 5.1,
	EquipCard = 4.8
}


function SmartAI:isTiaoxinTarget(enemy)
	if not enemy then self.room:writeToConsole(debug.traceback()) return end
	if getCardsNum("Slash", enemy, self.player) < 1 and self.player:getHp() > 1 and not self:canHit(self.player, enemy)
		and not (enemy:hasWeapon("DoubleSword") and self.player:getGender() ~= enemy:getGender())
		then return true end
	if sgs.card_lack[enemy:objectName()]["Slash"] == 1
		or self:needLeiji(self.player, enemy)
		or self:getDamagedEffects(self.player, enemy, true)
		or self:needToLoseHp(self.player, enemy, true, true)
		then return true end
	if self.player:hasSkill("xiangle") and (enemy:getHandcardNum() < 2 or getKnownCard(enemy, self.player, "BasicCard") < 2
												and enemy:getHandcardNum() - getKnownNum(enemy, self.player) < 2) then return true end
	return false
end

local tiaoxin_skill = {}
tiaoxin_skill.name = "tiaoxin"
table.insert(sgs.ai_skills, tiaoxin_skill)
tiaoxin_skill.getTurnUseCard = function(self)
	if not self:willShowForAttack() then
		return
	end
	if self.player:hasUsed("TiaoxinCard") then return end
	return sgs.Card_Parse("@TiaoxinCard=.&tiaoxin")
end

sgs.ai_skill_use_func.TiaoxinCard = function(TXCard, use, self)
	local distance = use.defHorse and 1 or 0
	local targets = {}
	for _, enemy in ipairs(self.enemies) do
		if enemy:distanceTo(self.player, distance) <= enemy:getAttackRange() and not self:doNotDiscard(enemy) and self:isTiaoxinTarget(enemy) then
			table.insert(targets, enemy)
		end
	end

	if #targets == 0 then return end

	sgs.ai_use_priority.TiaoxinCard = 8
	if not self.player:getArmor() and not self.player:isKongcheng() then
		for _, card in sgs.qlist(self.player:getCards("h")) do
			if card:isKindOf("Armor") and self:evaluateArmor(card) > 3 then
				sgs.ai_use_priority.TiaoxinCard = 5.9
				break
			end
		end
	end

	if use.to then
		self:sort(targets, "defenseSlash")
		use.to:append(targets[1])
	end
	use.card = TXCard
end

sgs.ai_skill_cardask["@tiaoxin-slash"] = function(self, data, pattern, target)
	if target then
		local cards = self:getCards("Slash")
		self:sortByUseValue(cards)
		for _, slash in ipairs(cards) do
			if self:isFriend(target) and self:slashIsEffective(slash, target) then
				if self:needLeiji(target, self.player) then return slash:toString() end
				if self:getDamagedEffects(target, self.player) then return slash:toString() end
				if self:needToLoseHp(target, self.player, nil, true) then return slash:toString() end
			end
			if not self:isFriend(target) and self:slashIsEffective(slash, target)
				and not self:getDamagedEffects(target, self.player, true) and not self:needLeiji(target, self.player) then
					return slash:toString()
			end
		end
		for _, slash in ipairs(cards) do
			if not self:isFriend(target) then
				if not self:needLeiji(target, self.player) and not self:getDamagedEffects(target, self.player, true) then return slash:toString() end
				if not self:slashIsEffective(slash, target) then return slash:toString() end
			end
		end
	end
	return "."
end

sgs.ai_card_intention.TiaoxinCard = 80
sgs.ai_use_priority.TiaoxinCard = 4

sgs.ai_skill_invoke.shoucheng = function(self, data)
	local move = data:toMoveOneTime()
	if move and move.from then
		local from = findPlayerByObjectName(move.from:objectName())
		if from and self:isFriend(from) and not self:needKongcheng(move.from, true) then
			return true
		end
	end
	return false
end

local shangyi_skill = {}
shangyi_skill.name = "shangyi"
table.insert(sgs.ai_skills, shangyi_skill)
shangyi_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ShangyiCard") then return end
	if self.player:isKongcheng() then return end
	if not self:willShowForAttack() then return end
	local card_str = ("@ShangyiCard=.&shangyi")
	local shangyi_card = sgs.Card_Parse(card_str)
	assert(shangyi_card)
	return shangyi_card
end

sgs.ai_skill_use_func.ShangyiCard = function(card, use, self)
	self:sort(self.enemies, "handcard")

	for index = #self.enemies, 1, -1 do
		if not self.enemies[index]:isKongcheng() and self:objectiveLevel(self.enemies[index]) > 0 then
			use.card = card
			if use.to then
				use.to:append(self.enemies[index])
			end
			return
		end
	end
end

sgs.ai_skill_choice.shangyi = function(self, choices)
	return "handcards"
end

sgs.ai_use_value.ShangyiCard = 4
sgs.ai_use_priority.ShangyiCard = 9
sgs.ai_card_intention.ShangyiCard = 50

sgs.ai_skill_invoke.yicheng = function(self, data)
	if not self:willShowForDefence() then
		return false
	end
	return true
end

sgs.ai_skill_discard.yicheng = function(self, discard_num, min_num, optional, include_equip)
	if self.player:hasSkill("hongyan") then
		return self:askForDiscard("dummyreason", 1, 1, false, true)
	end

	local unpreferedCards = {}
	local cards = sgs.QList2Table(self.player:getHandcards())

	if self:getCardsNum("Slash") > 1 then
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			if card:isKindOf("Slash") then table.insert(unpreferedCards, card:getId()) end
		end
		table.remove(unpreferedCards, 1)
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
			or self:getSameEquip(card, self.player) or card:isKindOf("AmazingGrace") or card:isKindOf("Lightning") then
			table.insert(unpreferedCards, card:getId())
		end
	end

	if self.player:getWeapon() and self.player:getHandcardNum() < 3 then
		table.insert(unpreferedCards, self.player:getWeapon():getId())
	end

	if self:needToThrowArmor() then
		table.insert(unpreferedCards, self.player:getArmor():getId())
	end

	if self.player:getOffensiveHorse() and self.player:getWeapon() then
		table.insert(unpreferedCards, self.player:getOffensiveHorse():getId())
	end

	for index = #unpreferedCards, 1, -1 do
		if not self.player:isJilei(sgs.Sanguosha:getCard(unpreferedCards[index])) then return { unpreferedCards[index] } end
	end

	return self:askForDiscard("dummyreason", 1, 1, false, true)
end

sgs.ai_skill_invoke.qianhuan = function(self, data)
	if not (self:willShowForAttack() or self:willShowForDefence() or self:willShowForMasochism() ) then
		return false
	end
	return true
end

local invoke_qianhuan = function(self, use)
	if (use.from and self:isFriend(use.from)) then return false end
	if use.to:isEmpty() then return false end
	if use.card:isKindOf("Peach") then return false end
	if use.card:isKindOf("Lightning") then return end
	local to = use.to:first()
	if use.card:isKindOf("Slash") and not self:slashIsEffective(use.card, to, use.from) then return end
	if use.card:isKindOf("TrickCard") and not self:hasTrickEffective(use.card, to, use.from) then return end
	if self.player:getPile("sorcery"):length() == 1 then
		if use.card:isKindOf("Slash") or use.card:isKindOf("Duel") or use.card:isKindOf("FireAttack") or use.card:isKindOf("BurningCamps")
			or use.card:isKindOf("ArcheryAttack") or use.card:isKindOf("Drowning") or use.card:isKindOf("SavageAssault") then
			return true
		end
		if use.card:isKindOf("KnownBoth") or use.card:isKindOf("Dismantlement") or use.card:isKindOf("Indulgence") or use.card:isKindOf("SupplyShortage") then
			--@todo
			return false
		end
		self.room:writeToConsole("invoke_qianhuan ? " .. use.card:getClassName())
		return false
	end
	if to and to:objectName() == self.player:objectName() then
		return not (use.from and (use.from:objectName() == to:objectName()
									or (use.card:isKindOf("Slash") and self:isPriorFriendOfSlash(self.player, use.card, use.from))))
	else
		return not (use.from and use.from:objectName() == to:objectName())
	end
end
sgs.ai_skill_use["@@qianhuan"] = function(self)
	local use = self.player:getTag("qianhuan_data"):toCardUse()
	local invoke = invoke_qianhuan(self, use)
	if invoke then
		return "@QianhuanCard=" .. self.player:getPile("sorcery"):first()
	end
	return "."
end

local function will_discard_zhendu(self)
	local current = self.room:getCurrent()
	local need_damage = self:getDamagedEffects(current, self.player) or self:needToLoseHp(current, self.player)
	if self:isFriend(current) then
		if current:getMark("drank") > 0 and not need_damage then return -1 end
		if (getKnownCard(current, self.player, "Slash") > 0 or (getCardsNum("Slash", current, self.player) >= 1 and current:getHandcardNum() >= 2))
			and (not self:damageIsEffective(current, nil, self.player) or current:getHp() > 2 or (getCardsNum("Peach", current, self.player) > 1 and not self:isWeak(current))) then
			local slash = sgs.cloneCard("slash")
			local trend = 3
			if current:hasWeapon("Axe") then trend = trend - 1
			elseif current:hasShownSkills("liegong|tieqi|wushuang|niaoxiang") then trend = trend - 0.4 end
			for _, enemy in ipairs(self.enemies) do
				if ((enemy:getHp() < 3 and enemy:getHandcardNum() < 3) or (enemy:getHandcardNum() < 2)) and current:canSlash(enemy) and not self:slashProhibit(slash, enemy, current)
					and self:slashIsEffective(slash, enemy, current) and sgs.isGoodTarget(enemy, self.enemies, self, true) then
					return trend
				end
			end
		end
		if need_damage then return 3 end
	elseif self:isEnemy(current) then
		if current:getHp() == 1 then return 1 end
		if need_damage or current:getHandcardNum() >= 2 then return -1 end
		if getKnownCard(current, self.player, "Slash") == 0 and getCardsNum("Slash", current, self.player) < 0.5 then return 3.5 end
	end
	return -1
end

sgs.ai_skill_discard.zhendu = function(self)
	local discard_trend = will_discard_zhendu(self)
	if discard_trend <= 0 then return "." end
	if self.player:getHandcardNum() + math.random(1, 100) / 100 >= discard_trend then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			if not self:isValuableCard(card, self.player) then return {card:getEffectiveId()} end
		end
	end
	return {}
end

sgs.ai_skill_invoke.jizhao = sgs.ai_skill_invoke.niepan

sgs.ai_skill_invoke.zhangwu = true

sgs.weapon_range.DragonPhoenix = 2
sgs.ai_use_priority.DragonPhoenix = 2.400
function sgs.ai_weapon_value.DragonPhoenix(self, enemy, player)
	local lordliubei = nil
	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:hasShownSkill("zhangwu") then
			lordliubei = p
			break
		end
	end
	if lordliubei and player:getWeapon() and not player:hasShownSkill("xiaoji") then
		return -10
	end
	if enemy and enemy:getHp() <= 1 and (sgs.card_lack[enemy:objectName()]["Jink"] == 1 or getCardsNum("Jink", enemy, self.player) == 0) then
		return 4.1
	end
end

function sgs.ai_slash_weaponfilter.DragonPhoenix(self, to, player)
	if player:distanceTo(to) > math.max(sgs.weapon_range.DragonPhoenix, player:getAttackRange()) then return end
	return getCardsNum("Peach", to, self.player) + getCardsNum("Jink", to, self.player) < 1
		and (sgs.card_lack[to:objectName()]["Jink"] == 1 or getCardsNum("Jink", to, self.player) == 0)
end

sgs.ai_skill_invoke.DragonPhoenix = function(self, data)
	if data:toString() == "revive" then return true end
	local death = data:toDeath()
	if death.who then return true
	else
		local to = data:toPlayer()
		return self:doNotDiscard(to) == self:isFriend(to)
	end
end

sgs.ai_skill_choice.DragonPhoenix = function(self, choices, data)
	local kingdom = data:toString()
	choices_t = string.split(choices, "+")
	if (kingdom == "wei") then
		if (string.find(choices, "guojia")) then
			return "guojia"
		elseif (string.find(choices, "xunyu")) then
			return "xunyu"
		elseif (string.find(choices, "lidian")) then
			return "lidian"
		elseif (string.find(choices, "zhanghe")) then
			return "zhanghe"
		elseif (string.find(choices, "caopi")) then
			return "caopi"
		elseif (string.find(choices, "zhangliao")) then
			return "zhangliao"
		end

		table.removeOne(choices_t, "caohong")
		table.removeOne(choices_t, "zangba")
		table.removeOne(choices_t, "xuchu")
		table.removeOne(choices_t, "dianwei")
		table.removeOne(choices_t, "caoren")

	elseif (kingdom == "shu") then
		if (string.find(choices, "mifuren")) then
			return "mifuren"
		elseif (string.find(choices, "pangtong")) then
			return "pangtong"
		elseif (string.find(choices, "lord_liubei")) then
			return "lord_liubei"
		elseif (string.find(choices, "liushan")) then
			return "liushan"
		elseif (string.find(choices, "jiangwanfeiyi")) then
			return "jiangwanfeiyi"
		end

		table.removeOne(choices_t, "liubei")
		table.removeOne(choices_t, "guanyu")
		table.removeOne(choices_t, "zhangfei")
		table.removeOne(choices_t, "weiyan")
		table.removeOne(choices_t, "zhurong")
		table.removeOne(choices_t, "madai")

	elseif (kingdom == "wu") then
		if (string.find(choices, "zhoutai")) then
			return "zhoutai"
		elseif (string.find(choices, "lusu")) then
			return "lusu"
		elseif (string.find(choices, "taishici")) then
			return "taishici"
		elseif (string.find(choices, "sunjian")) then
			return "sunjian"
		end

		table.removeOne(choices_t, "sunce")
		table.removeOne(choices_t, "chenwudongxi")
		table.removeOne(choices_t, "luxun")
		table.removeOne(choices_t, "huanggai")

	elseif (kingdom == "qun") then
		if (string.find(choices, "yuji")) then
			return "yuji"
		elseif (string.find(choices, "caiwenji")) then
			return "caiwenji"
		elseif (string.find(choices, "mateng")) then
			return "mateng"
		elseif (string.find(choices, "kongrong")) then
			return "kongrong"
		elseif (string.find(choices, "lord_zhangjiao")) then
			return "lord_zhangjiao"
		end

		table.removeOne(choices_t, "dongzhuo")
		table.removeOne(choices_t, "tianfeng")
		table.removeOne(choices_t, "zhangjiao")

	end
	if #choices_t == 0 then choices_t = string.split(choices, "+") end
	return choices_t[math.random(1, #choices_t)]
end

sgs.ai_skill_discard.DragonPhoenix = function(self, discard_num, min_num, optional, include_equip)
	local to_discard = sgs.QList2Table(self.player:getCards("he"))

	if #to_discard == 1 then
		return {to_discard[1]:getEffectiveId()}
	end

	local aux_func = function(card)
		local place = self.room:getCardPlace(card:getEffectiveId())
		if place == sgs.Player_PlaceEquip then
			if card:isKindOf("SilverLion") and self.player:isWounded() then return -2 end

			if card:isKindOf("Weapon") then
				if self.player:getHandcardNum() < discard_num + 2 and not self:needKongcheng() then return 0
				else return 2 end
			elseif card:isKindOf("OffensiveHorse") then
				if self.player:getHandcardNum() < discard_num + 2 and not self:needKongcheng() then return 0
				else return 1 end
			elseif card:isKindOf("DefensiveHorse") then return 3
			elseif card:isKindOf("Armor") then
				if self.player:hasSkill("bazhen") then return 0
				else return 4 end
			else return 0 --@to-do: add the corrsponding value of Treasure
			end
		else
			if self.player:getMark("@qianxi_red") > 0 and card:isRed() and not card:isKindOf("Peach") then return 0 end
			if self.player:getMark("@qianxi_black") > 0 and card:isBlack() then return 0 end
			if self:isWeak() then return 5 else return 0 end
		end
	end

	local compare_func = function(card1, card2)
		local card1_aux = aux_func(card1)
		local card2_aux = aux_func(card2)
		if card1_aux ~= card2_aux then return card1_aux < card2_aux end
		return self:getKeepValue(card1) < self:getKeepValue(card2)
	end

	table.sort(to_discard, compare_func)

	for _, card in ipairs(to_discard) do
		if not self.player:isJilei(card) then return {card:getEffectiveId()} end
	end
end

sgs.ai_skill_invoke.shengxi = function(self, data)
	if not self:willShowForDefence() then
		return false
	end
	if self:getOverflow() >= 0 then
		local erzhang = sgs.findPlayerByShownSkillName("guzheng")
		if erzhang and self:isEnemy(erzhang) then return false end
	end
	return true
end


