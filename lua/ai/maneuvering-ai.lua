--[[********************************************************************
	Copyright (c) 2013-2014 - QSanguosha-Hegemony Team

  This file is part of QSanguosha-Hegemony.

  This game is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3.0 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  See the LICENSE file for more details.

  QSanguosha-Hegemony Team
*********************************************************************]]

function SmartAI:useCardThunderSlash(...)
	self:useCardSlash(...)
end

sgs.ai_card_intention.ThunderSlash = sgs.ai_card_intention.Slash

sgs.ai_use_value.ThunderSlash = 4.55
sgs.ai_keep_value.ThunderSlash = 3.66
sgs.ai_use_priority.ThunderSlash = 2.5

function SmartAI:useCardFireSlash(...)
	self:useCardSlash(...)
end

sgs.ai_card_intention.FireSlash = sgs.ai_card_intention.Slash

sgs.ai_use_value.FireSlash = 4.6
sgs.ai_keep_value.FireSlash = 3.63
sgs.ai_use_priority.FireSlash = 2.5

sgs.weapon_range.Fan = 4
sgs.ai_use_priority.Fan = 2.655
sgs.ai_use_priority.Vine = 0.95

sgs.ai_skill_invoke.Fan = function(self, data)
	local use = data:toCardUse()

	for _, target in sgs.qlist(use.to) do
		if self:isFriend(target) then
			if not self:damageIsEffective(target, sgs.DamageStruct_Fire) then return true end
			if target:isChained() and self:isGoodChainTarget(target, nil, nil, nil, use.card) then return true end
		else
			if not self:damageIsEffective(target, sgs.DamageStruct_Fire) then return false end
			if target:isChained() and not self:isGoodChainTarget(target, nil, nil, nil, use.card) then return false end
			if target:hasArmorEffect("Vine") then
				return true
			end
		end
	end
	return false
end
sgs.ai_view_as.Fan = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if sgs.Sanguosha:getCurrentCardUseReason() ~= sgs.CardUseStruct_CARD_USE_REASON_RESPONSE
		and card_place ~= sgs.Player_PlaceSpecial and card:objectName() == "slash" then
		return ("fire_slash:fan[%s:%s]=%d&Fan"):format(suit, number, card_id)
	end
end

local fan_skill = {}
fan_skill.name = "Fan"
table.insert(sgs.ai_skills, fan_skill)
fan_skill.getTurnUseCard = function(self)
	local cards = self.player:getCards("h")
	cards = sgs.QList2Table(cards)
	local slash_card

	for _,card in ipairs(cards)  do
		if card:isKindOf("Slash") and not (card:isKindOf("FireSlash") or card:isKindOf("ThunderSlash")) then
			slash_card = card
			break
		end
	end

	if not slash_card  then return nil end
	local suit = slash_card:getSuitString()
	local number = slash_card:getNumberString()
	local card_id = slash_card:getEffectiveId()
	local card_str = ("fire_slash:Fan[%s:%s]=%d&Fan"):format(suit, number, card_id)
	local fireslash = sgs.Card_Parse(card_str)
	assert(fireslash)

	return fireslash
end

function sgs.ai_weapon_value.Fan(self, enemy)
	if enemy and enemy:hasArmorEffect("Vine") then return 6 end
end

function sgs.ai_armor_value.Vine(player, self)
	if self:needKongcheng(player) and player:getHandcardNum() == 1 then
		return player:hasShownSkill("kongcheng") and 5 or 3.8
	end
	if self.player:hasSkills(sgs.lose_equip_skill) then return 3.8 end
	if not self:damageIsEffective(player, sgs.DamageStruct_Fire) then return 6 end

	local fslash = sgs.cloneCard("fire_slash")
	local tslash = sgs.cloneCard("thunder_slash")
	if player:isChained() and (not self:isGoodChainTarget(player, self.player, nil, nil, fslash) or not self:isGoodChainTarget(player, self.player, nil, nil, tslash)) then return -2 end

	for _, enemy in ipairs(self:getEnemies(player)) do
		if enemy:hasShownSkill("jgbiantian") then return -2 end
		if (enemy:canSlash(player) and enemy:hasWeapon("Fan")) or enemy:hasShownSkill("huoji") then return -2 end
		if getKnownCard(enemy, player, "FireSlash", true) >= 1 or getKnownCard(enemy, player, "FireAttack", true) >= 1 or
			getKnownCard(enemy, player, "Fan") >= 1 then return -2 end
	end

	if (#self.enemies < 3 and sgs.turncount > 2) or player:getHp() <= 2 then return 5 end
	return 1
end

function SmartAI:shouldUseAnaleptic(target, card_use)

	if target:hasArmorEffect("SilverLion") and not self.player:hasWeapon("QinggangSword") then return false end
	if sgs.isAnjiang(target) and self:objectiveLevel(target) == 3.5 then return end

	for _, p in sgs.qlist(self.room:getAlivePlayers()) do
		if p:hasShownSkill("qianhuan") and not p:getPile("sorcery"):isEmpty() and p:getKingdom() == target:getKingdom() and card_use.to:length() <= 1 then
			return false
		end
	end

	if target:hasShownSkill("xiangle") then
		local basicnum = 0
		for _, acard in sgs.qlist(self.player:getHandcards()) do
			if acard:getTypeId() == sgs.Card_TypeBasic and not acard:isKindOf("Peach") then basicnum = basicnum + 1 end
		end
		if basicnum < 3 then return false end
	end

	local hcard = target:getHandcardNum()
	if self.player:hasSkill("liegong") and self.player:getPhase() == sgs.Player_Play and (hcard >= self.player:getHp() or hcard <= self.player:getAttackRange()) then return true end
	if self.player:hasSkill("tieqi") then return true end

	if self.player:hasWeapon("axe") and self.player:getCards("he"):length() > 4 then return true end

	if self.player:hasSkill("wushuang") then
		if getKnownCard(target, player, "Jink", true, "he") >= 2 then return false end
		return getCardsNum("Jink", target, self.player) < 2
	end

	if getKnownCard(target, self.player, "Jink", true, "he") >= 1 and not (self:getOverflow() > 0 and self:getCardsNum("Analeptic") > 1) then return false end
	return self:getCardsNum("Analeptic") > 1 or getCardsNum("Jink", target) < 1 or sgs.card_lack[target:objectName()]["Jink"] == 1 or self:getOverflow() > 0
end

function SmartAI:useCardAnaleptic(card, use)
	if not self.player:hasEquip(card) and not self:hasLoseHandcardEffective() and not self:isWeak()
		and sgs.Analeptic_IsAvailable(self.player, card) then
		use.card = card
	end
end

function SmartAI:searchForAnaleptic(use, enemy, slash)
	if not self.toUse then return nil end
	if not use.to then return nil end

	local analeptic = self:getCard("Analeptic")
	if not analeptic then return nil end

	local analepticAvail = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_Residue, self.player, analeptic)
	local slashAvail = 0

	for _, card in ipairs(self.toUse) do
		if analepticAvail == 1 and card:getEffectiveId() ~= slash:getEffectiveId() and card:isKindOf("Slash") then return nil end
		if card:isKindOf("Slash") then slashAvail = slashAvail + 1 end
	end

	if analepticAvail > 1 and analepticAvail < slashAvail then return nil end
	if not sgs.Analeptic_IsAvailable(self.player) then return nil end

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:fillSkillCards(cards)
	local allcards = self.player:getCards("he")
	allcards = sgs.QList2Table(allcards)

	local card_str = self:getCardId("Analeptic")
	if card_str then return sgs.Card_Parse(card_str) end

	for _, anal in ipairs(cards) do
		if (anal:getClassName() == "Analeptic") and not (anal:getEffectiveId() == slash:getEffectiveId()) then
			return anal
		end
	end
end

sgs.dynamic_value.benefit.Analeptic = true

sgs.ai_use_value.Analeptic = 5.98
sgs.ai_keep_value.Analeptic = 4.1
sgs.ai_use_priority.Analeptic = 3.0

local function handcard_subtract_hp(a, b)
	local diff1 = a:getHandcardNum() - a:getHp()
	local diff2 = b:getHandcardNum() - b:getHp()

	return diff1 < diff2
end

function SmartAI:useCardSupplyShortage(card, use)
	local enemies = self:exclude(self.enemies, card)

	local zhanghe = self.room:findPlayerBySkillName("qiaobian")
	local zhanghe_seat = zhanghe and zhanghe:faceUp() and not zhanghe:isKongcheng() and not self:isFriend(zhanghe) and zhanghe:getSeat() or 0

	if #enemies == 0 then return end

	local getvalue = function(enemy)
		if card:isBlack() and enemy:hasShownSkill("weimu") then return -100 end
		if enemy:containsTrick("supply_shortage") or enemy:containsTrick("YanxiaoCard") then return -100 end
		if enemy:hasShownSkill("qiaobian") and not enemy:containsTrick("supply_shortage") and not enemy:containsTrick("indulgence") then return -100 end
		if zhanghe_seat > 0 and (self:playerGetRound(zhanghe) <= self:playerGetRound(enemy) and self:enemiesContainsTrick() <= 1 or not enemy:faceUp()) then
			return - 100 end

		local value = 0 - enemy:getHandcardNum()

		if enemy:hasShownSkills("haoshi|tuxi|lijian|fanjian|dimeng|jijiu|jieyin|beige")
		  or (enemy:hasShownSkill("zaiqi") and enemy:getLostHp() > 1)
			then value = value + 10
		end
		if enemy:hasShownSkills(sgs.cardneed_skill .. "|tianxiang")
			then value = value + 5
		end
		if enemy:hasShownSkills("yingzi|duoshi") then value = value + 1 end
		if self:isWeak(enemy) then value = value + 5 end
		if enemy:isLord() then value = value + 3 end

		if self:objectiveLevel(enemy) < 3 then value = value - 10 end
		if not enemy:faceUp() then value = value - 10 end
		if enemy:hasShownSkills("keji|shensu") then value = value - enemy:getHandcardNum() end
		if enemy:hasShownSkills("guanxing|tiandu|guidao") then value = value - 5 end
		if not sgs.isGoodTarget(enemy, self.enemies, self) then value = value - 1 end
		if self:needKongcheng(enemy) then value = value - 1 end
		return value
	end

	local cmp = function(a,b)
		return getvalue(a) > getvalue(b)
	end

	table.sort(enemies, cmp)

	local target = enemies[1]
	if getvalue(target) > -100 then
		use.card = card
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_value.SupplyShortage = 7
sgs.ai_keep_value.SupplyShortage = 3.48
sgs.ai_use_priority.SupplyShortage = 0.5
sgs.ai_card_intention.SupplyShortage = 120

sgs.dynamic_value.control_usecard.SupplyShortage = true

function SmartAI:getChainedFriends(player)
	player = player or self.player
	local chainedFriends = {}
	for _, friend in ipairs(self:getFriends(player)) do
		if friend:isChained() then
			table.insert(chainedFriends, friend)
		end
	end
	return chainedFriends
end

function SmartAI:getChainedEnemies(player)
	player = player or self.player
	local chainedEnemies = {}
	for _, enemy in ipairs(self:getEnemies(player)) do
		if enemy:isChained() then
			table.insert(chainedEnemies,enemy)
		end
	end
	return chainedEnemies
end

function SmartAI:isGoodChainPartner(player)
	player = player or self.player
	if hasBuquEffect(player) or (self.player:hasSkill("niepan") and self.player:getMark("@nirvana") > 0) or self:needToLoseHp(player) or self:getDamagedEffects(player) then
		return true
	end
	return false
end

function SmartAI:isGoodChainTarget(who, source, nature, damagecount, card)
	if not who then self.room:writeToConsole(debug.traceback()) return end
	if not who:isChained() then return not self:isFriend(who) end
	local damageStruct = {}
	damageStruct.to = who
	damageStruct.from = source or self.player
	damageStruct.nature = nature or sgs.DamageStruct_Fire
	damageStruct.damage = damagecount or 1
	damageStruct.card = card
	return self:isGoodChainTarget_(damageStruct)
end

function SmartAI:isGoodChainTarget_(damageStruct)
	local to = damageStruct.to
	if not to then self.room:writeToConsole(debug.traceback()) return end
	if not to:isChained() then return not self:isFriend(to) end
	local from = damageStruct.from or self.player
	local nature = damageStruct.nature or sgs.DamageStruct_Fire
	local damage = damageStruct.damage or 1
	local card = damageStruct.card

	if card and card:isKindOf("Slash") then
		nature = card:isKindOf("FireSlash") and sgs.DamageStruct_Fire
					or card:isKindOf("ThunderSlash") and sgs.DamageStruct_Thunder
					or sgs.DamageStruct_Normal
		damage = self:hasHeavySlashDamage(from, card, to, true)
	elseif nature == sgs.DamageStruct_Fire then
		if to:getMark("@gale") > 0 then damage = damage + 1 end
	end

	if not self:damageIsEffective_(damageStruct) then return end
	if card and card:isKindOf("FireAttack") and not self:hasTrickEffective(card, to, self.player) then return end

	local jiaren_zidan = self.room:findPlayerBySkillName("jgchiying")
	if jiaren_zidan and jiaren_zidan:isFriendWith(to) then
		damage = 1
	end

	if nature == sgs.DamageStruct_Fire then
		if to:hasArmorEffect("Vine") then damage = damage + 1 end
	end

	if to:hasArmorEffect("SilverLion") then damage = 1 end

	local kills, the_enemy = 0
	local good, bad, F_count, E_count = 0, 0, 0, 0
	local peach_num = self.player:objectName() == from:objectName() and self:getCardsNum("Peach") or getCardsNum("Peach", from, self.player)

	local function getChainedPlayerValue(target, dmg)
		local newvalue = 0
		if self:isGoodChainPartner(target) then newvalue = newvalue + 1 end
		if self:isWeak(target) then newvalue = newvalue - 1 end
		if dmg and nature == sgs.DamageStruct_Fire then
			if target:hasArmorEffect("Vine") then dmg = dmg + 1 end
			if target:getMark("@gale") > 0 then damage = damage + 1 end
		end
		if self:cantbeHurt(target, from, damage) then newvalue = newvalue - 100 end
		if damage + (dmg or 0) >= target:getHp() then
			newvalue = newvalue - 2
			if self:isEnemy(target) then kills = kills + 1 end
			if target:objectName() == self.player:objectName() and #self.friends_noself == 0 and peach_num < damage + (dmg or 0) then newvalue = newvalue - 100 end
		else
			if self:isEnemy(target) and from:getHandcardNum() < 2 and target:hasShownSkills("ganglie") and from:getHp() == 1
				and self:damageIsEffective(from, nil, target) and peach_num < 1 then newvalue = newvalue - 100 end
		end

		if target:hasArmorEffect("SilverLion") then return newvalue - 1 end
		return newvalue - damage - (dmg or 0)
	end


	local value = getChainedPlayerValue(to)
	if self:isFriend(to) then
		good = value
		F_count = F_count + 1
	else
		bad = value
		E_count = E_count + 1
	end

	if nature == sgs.DamageStruct_Normal then return good >= bad end

	for _, player in sgs.qlist(self.room:getAllPlayers()) do
		local newDamageStruct = damageStruct
		newDamageStruct.to = player
		if nature == sgs.DamageStruct_Fire and player:hasArmorEffect("Vine") then newDamageStruct.damage = newDamageStruct.damage + 1 end
		if player:objectName() ~= to:objectName() and player:isChained() and self:damageIsEffective_(newDamageStruct)
			and not (card and card:isKindOf("FireAttack") and not self:hasTrickEffective(card, to, self.player)) then
			local getvalue = getChainedPlayerValue(player, 0)
			if kills == #self.enemies and sgs.getDefenseSlash(player, self) < 2 then
				if card then self.room:setCardFlag(card, "AIGlobal_KillOff") end
				return true
			end
			if self:isFriend(player) then
				good = good + getvalue
				F_count = F_count + 1
			else
				bad = bad + getvalue
				E_count = E_count + 1
				the_enemy = player
			end
		end
	end

	if card and F_count == 1 and E_count == 1 and the_enemy and the_enemy:isKongcheng() and the_enemy:getHp() == 1 then
		for _, c in ipairs(self:getCards("Slash")) do
			if not c:isKindOf("NatureSlash") and not self:slashProhibit(c, the_enemy, source) then return end
		end
	end

	if F_count > 0 and E_count <= 0 then return end

	return good >= bad
end

function SmartAI:useCardIronChain(card, use)
	if self.player:isLocked(card) then return end
	use.card = card
	if #self.enemies == 1 and #self:getChainedFriends() <= 1 then return end
	local friendtargets, friendtargets2 = {}, {}
	local otherfriends = {}
	local enemytargets = {}
	self:sort(self.friends, "defense")
	for _, friend in ipairs(self.friends) do
		if friend:isChained() and not self:isGoodChainPartner(friend) and self:hasTrickEffective(card, friend) then
			if friend:containsTrick("lightning") then
				table.insert(friendtargets, friend)
			else
				table.insert(friendtargets2, friend)
			end
		else
			table.insert(otherfriends, friend)
		end
	end
	table.insertTable(friendtargets, friendtargets2)
	self:sort(self.enemies, "defense")
	for _, enemy in ipairs(self.enemies) do
		if not enemy:isChained() --[[and not sgs.Sanguosha:isProhibited(self.player, enemy, card)]]
			and self:hasTrickEffective(card, enemy) and not (self:objectiveLevel(enemy) <= 3)
			and not self:getDamagedEffects(enemy) and not self:needToLoseHp(enemy) and sgs.isGoodTarget(enemy, self.enemies, self) then
			table.insert(enemytargets, enemy)
		end
	end

	local chainSelf = (self:needToLoseHp(self.player) or self:getDamagedEffects(self.player)) and not self.player:isChained()
						and (self:getCardId("FireSlash") or self:getCardId("ThunderSlash") or
							(self:getCardId("Slash") and (self.player:hasWeapon("Fan")))
						or (self:getCardId("FireAttack") and self.player:getHandcardNum() > 2))

	local targets_num = 2 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card)

	if #friendtargets > 1 then
		if use.to then
			for _, friend in ipairs(friendtargets) do
				use.to:append(friend)
				if use.to:length() == targets_num then return end
			end
		end
	elseif #friendtargets == 1 then
		if #enemytargets > 0 then
			if use.to then
				use.to:append(friendtargets[1])
				for _, enemy in ipairs(enemytargets) do
					use.to:append(enemy)
					if use.to:length() == targets_num then return end
				end
			end
		elseif chainSelf then
			if use.to then use.to:append(friendtargets[1]) end
			if use.to then use.to:append(self.player) end
		end
	elseif #enemytargets > 1 then
		if use.to then
			for _, enemy in ipairs(enemytargets) do
				use.to:append(enemy)
				if use.to:length() == targets_num then return end
			end
		end
	elseif #enemytargets == 1 then
		if chainSelf then
			if use.to then use.to:append(enemytargets[1]) end
			if use.to then use.to:append(self.player) end
		end
	end
	if use.to then assert(use.to:length() < targets_num + 1) end
end

sgs.ai_card_intention.IronChain = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if not to:isChained() then
			sgs.updateIntention(from, to,60)
		else
			sgs.updateIntention(from, to, -60)
		end
	end
end

sgs.ai_use_value.IronChain = 5.4
sgs.ai_keep_value.IronChain = 3.32
sgs.ai_use_priority.IronChain = 9.1

sgs.ai_skill_cardask["@fire-attack"] = function(self, data, pattern, target)
	local cards = sgs.QList2Table(self.player:getHandcards())
	local convert = { [".S"] = "spade", [".D"] = "diamond", [".H"] = "heart", [".C"] = "club"}
	local card

	self:sortByUseValue(cards, true)

	for _, acard in ipairs(cards) do
		if acard:getSuitString() == convert[pattern] then
			if not isCard("Peach", acard, self.player) then
				card = acard
				break
			else
				local needKeepPeach = true
				if (self:isWeak(target) and not self:isWeak()) or target:getHp() == 1
						or self:isGoodChainTarget(target) or target:hasArmorEffect("Vine") then
					needKeepPeach = false
				end
				if not needKeepPeach then
					card = acard
					break
				end
			end
		end
	end
	return card and card:getId() or "."
end

function SmartAI:useCardFireAttack(fire_attack, use)

	local lack = {
		spade = true,
		club = true,
		heart = true,
		diamond = true,
	}

	local cards = self.player:getHandcards()
	local canDis = {}
	for _, card in sgs.qlist(cards) do
		if card:getEffectiveId() ~= fire_attack:getEffectiveId() then
			table.insert(canDis, card)
			lack[card:getSuitString()] = false
		end
	end

	if self.player:hasSkill("hongyan") then
		lack.spade = true
	end

	local suitnum = 0
	for suit,islack in pairs(lack) do
		if not islack then suitnum = suitnum + 1  end
	end


	self:sort(self.enemies, "defense")

	local can_attack = function(enemy)
		if self.player:hasFlag("FireAttackFailed_" .. enemy:objectName()) then
			return false
		end
		local damage = 1
		if not enemy:hasArmorEffect("SilverLion") then
			if enemy:hasArmorEffect("Vine") then damage = damage + 1 end
		end
		if enemy:hasShownSkill("mingshi") and not self.player:hasShownAllGenerals() then
			damage = damage - 1
		end
		return self:objectiveLevel(enemy) > 3 and damage > 0 and not enemy:isKongcheng()
				and self:damageIsEffective(enemy, sgs.DamageStruct_Fire, self.player) and not self:cantbeHurt(enemy, self.player, damage)
				and self:hasTrickEffective(fire_attack, enemy)
				and sgs.isGoodTarget(enemy, self.enemies, self)
				and (not (enemy:hasShownSkill("jianxiong") and not self:isWeak(enemy)) and not self:getDamagedEffects(enemy, self.player)
						and not (enemy:isChained() and not self:isGoodChainTarget(enemy)))
	end

	local enemies, targets = {}, {}
	for _, enemy in ipairs(self.enemies) do
		if can_attack(enemy) then
			table.insert(enemies, enemy)
		end
	end

	local can_FireAttack_self
	for _, card in ipairs(canDis) do
		if (not isCard("Peach", card, self.player) or self:getCardsNum("Peach") >= 3) and not self.player:hasArmorEffect("IronArmor")
			and (not isCard("Analeptic", card, self.player) or self:getCardsNum("Analeptic") >= 2) then
			can_FireAttack_self = true
		end
	end

	if can_FireAttack_self and self.player:isChained() and self:isGoodChainTarget(self.player)
		and self.player:getHandcardNum() > 1
		and self:damageIsEffective(self.player, sgs.DamageStruct_Fire, self.player) and not self:cantbeHurt(self.player)
		and self:hasTrickEffective(fire_attack, self.player) then

		if self.player:hasSkill("niepan") and self.player:getMark("@nirvana") > 0 then
			table.insert(targets, self.player)
		elseif hasBuquEffect(self.player)then
			table.insert(targets, self.player)
		else
			local leastHP = 1
			if self.player:hasArmorEffect("Vine") then leastHP = leastHP + 1 end
			if self.player:getHp() > leastHP then
				table.insert(targets, self.player)
			elseif self:getCardsNum("Peach") + self:getCardsNum("Analeptic") > self.player:getHp() - leastHP then
				table.insert(targets, self.player)
			end
		end
	end

	for _, enemy in ipairs(enemies) do
		if enemy:getHandcardNum() == 1 then
			local handcards = sgs.QList2Table(enemy:getHandcards())
			local flag = string.format("%s_%s_%s", "visible", self.player:objectName(), enemy:objectName())
			if handcards[1]:hasFlag("visible") or handcards[1]:hasFlag(flag) then
				local suitstring = handcards[1]:getSuitString()
				if not lack[suitstring] and not table.contains(targets, enemy) then
					table.insert(targets, enemy)
				end
			end
		end
	end

	if ((suitnum == 2 and lack.diamond == false) or suitnum <= 1)
		and self:getOverflow() <= (self.player:hasSkill("jizhi") and -2 or 0)
		and #targets == 0 then return end

	for _, enemy in ipairs(enemies) do
		local damage = 1
		if not enemy:hasArmorEffect("SilverLion") then
			if enemy:hasArmorEffect("Vine") then damage = damage + 1 end
		end
		if enemy:hasShownSkill("mingshi") and not self.player:hasShownAllGenerals() then
			damage = damage - 1
		end
		if self:damageIsEffective(enemy, sgs.DamageStruct_Fire, self.player) and damage > 1 then
			if not table.contains(targets, enemy) then table.insert(targets, enemy) end
		end
	end
	for _, enemy in ipairs(enemies) do
		if not table.contains(targets, enemy) then table.insert(targets, enemy) end
	end

	if #targets > 0 then
		local godsalvation = self:getCard("GodSalvation")
		if godsalvation and godsalvation:getId() ~= fire_attack:getId() and self:willUseGodSalvation(godsalvation) then
			local use_gs = true
			for _, p in ipairs(targets) do
				if not p:isWounded() or not self:hasTrickEffective(godsalvation, p, self.player) then break end
				use_gs = false
			end
			if use_gs then
				use.card = godsalvation
				return
			end
		end

		local targets_num = 1 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, fire_attack)
		use.card = fire_attack
		for i = 1, #targets, 1 do
			if use.to then
				use.to:append(targets[i])
				if use.to:length() == targets_num then return end
			end
		end
	end
end

sgs.ai_cardshow.fire_attack = function(self, requestor)
	local cards = sgs.QList2Table(self.player:getHandcards())
	if requestor:objectName() == self.player:objectName() then
		self:sortByUseValue(cards, true)
		return cards[1]
	end

	local priority = { heart = 4, spade = 3, club = 2, diamond = 1 }
	local index = -1
	local result
	for _, card in ipairs(cards) do
		if priority[card:getSuitString()] > index then
			result = card
			index = priority[card:getSuitString()]
		end
	end

	return result
end

sgs.ai_use_value.FireAttack = 4.8
sgs.ai_keep_value.FireAttack = 3.28
sgs.ai_use_priority.FireAttack = sgs.ai_use_priority.Dismantlement + 0.1

sgs.dynamic_value.damage_card.FireAttack = true

sgs.ai_card_intention.FireAttack = 80

sgs.dynamic_value.damage_card.FireAttack = true
