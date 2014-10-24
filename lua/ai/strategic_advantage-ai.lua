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

--transfer
local transfer_skill = {}
transfer_skill.name = "transfer"
table.insert(sgs.ai_skills, transfer_skill)
transfer_skill.getTurnUseCard = function(self, inclusive)
	if self.player:isKongcheng() then return end
	if self.player:hasSkill("rende") then return end
	if not self.player:hasSkill("kongcheng") then
		if self:isWeak() and self:getOverflow() <= 0 then return end
		if not self.player:hasShownOneGeneral() then return end
	end
	return sgs.Card_Parse("@TransferCard=.")
end

sgs.ai_skill_use_func.TransferCard = function(transferCard, use, self)

	local friends, friends_other = {}, {}
	local targets = sgs.PlayerList()
	for _, friend in ipairs(self.friends_noself) do
		if transferCard:targetFilter(targets, friend, self.player) and not self:needKongcheng(friend, true) and friend:hasShownOneGeneral() then
			if friend:hasShownOneGeneral() then
				table.insert(friends, friend)
			else
				table.insert(friends_other, friend)
			end
		end
	end
	if #friends == 0 and #friends_other == 0 then return end

	local cards = {}
	local oneJink = self.player:hasSkill("kongcheng")
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isTransferable() and (not isCard("Peach", c, self.player) or #friends == 0) then
			if not oneJink and isCard("Jink", c, self.player) then
				oneJink = true
				continue
			end
			table.insert(cards, c)
		end
	end
	if #cards == 0 then return end

	self:sortByUseValue(cards)
	if #friends > 0 then
		local card, target = self:getCardNeedPlayer(cards, friends)
		if card and target then
			use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
			if use.to then use.to:append(target) end
			return
		end
	end

	if #friends_other == 0 then return end

	local card, target = self:getCardNeedPlayer(cards, friends_other)
	if card and target then
		use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
		if use.to then use.to:append(target) end
		return
	end
end

sgs.ai_use_priority.TransferCard = 0.01
sgs.ai_card_intention.TransferCard = -10

--Drowning
function SmartAI:useCardDrowning(card, use)
	if not card:isAvailable(self.player) then return end

	self:sort(self.enemies, "equip_defense")

	local players = sgs.PlayerList()
	for _, enemy in ipairs(self.enemies) do
		if card:targetFilter(players, enemy, self.player) and not players:contains(enemy) and enemy:hasEquip()
			and self:hasTrickEffective(card, enemy) and self:damageIsEffective(enemy) and self:canAttack(enemy)
			and not self:getDamagedEffects(enemy, self.player) and not self:needToLoseHp(enemy, self.player) then
			players:append(enemy)
			if use.to then use.to:append(enemy) end
		end
	end

	for _, friend in ipairs(self.friends_noself) do
		if card:targetFilter(players, friend, self.player) and not players:contains(friend) and self:needToThrowArmor(friend) and friend:getEquips():length() == 1 then
			players:append(friend)
			if use.to then use.to:append(friend) end
		end
	end

	if not players:isEmpty() then
		use.card = card
		return
	end
end

sgs.ai_card_intention.Drowning = function(self, card, from, tos)
	for _, to in ipairs(tos) do
		if not self:hasTrickEffective(card, to, from) or not self:damageIsEffective(to, sgs.DamageStruct_Normal, from)
			or self:needToThrowArmor(to) then
		else
			sgs.updateIntention(from, to, 80)
		end
	end
end

sgs.ai_skill_choice.drowning = function(self, choices, data)
	local effect = data:toCardEffect()
	if not self:damageIsEffective(self.player, sgs.DamageStruct_Normal, effect.from)
		or self:needToLoseHp(self.player, effect.from)
		or self:getDamagedEffects(self.player, effect.from) then return "damage" end

	if self.player:getHp() == 1 then return "throw" end

	local value = 0
	for _, equip in sgs.qlist(self.player:getEquips()) do
		if equip:isKindOf("Weapon") then value = value + self:evaluateWeapon(equip)
		elseif equip:isKindOf("Armor") then
			value = value + self:evaluateArmor(equip)
			if self:needToThrowArmor() then value = value - 5 end
		elseif equip:isKindOf("OffensiveHorse") then value = value + 2.5
		elseif equip:isKindOf("DefensiveHorse") then value = value + 5
		end
	end
	if value < 8 then return "throw" else return "damage" end
end

sgs.ai_nullification.Drowning = function(self, card, from, to, positive)
	if positive then
		if self:isFriend(to) then
			if self:needToThrowArmor(to) then return end
			if to:getEquips():length() >= 2 then return true end
		end
	else
		if self:isFriend(from) and (self:getOverflow() > 0 or self:getCardsNum("Nullification") > 1) then return true end
	end
	return
end

sgs.ai_use_value.Drowning = 5.1
sgs.ai_use_priority.Drowning = 7
sgs.ai_keep_value.Drowning = 3.4

--IronArmor
function sgs.ai_armor_value.IronArmor(player, self)
	if self:isWeak(player) then
		for _, p in sgs.qlist(self.room:getOtherPlayers(player)) do
			if p:hasShownSkill("huoji") and self:isEnemy(player, p) then
				return 5
			end
		end
	end
	return 2.5
end

sgs.ai_use_priority.IronArmor = 0.82

--BurningCamps
function SmartAI:useCardBurningCamps(card, use)
	if not card:isAvailable(self.player) then return end

	local player = self.room:nextPlayer(self.player)
	if self:isFriendWith(player) then
		return
	else
		local enemies = player:getFormation()
		local shouldUse
		for _, enemy in sgs.qlist(enemies) do
			if not self:hasTrickEffective(card, enemy, self.player) then continue end
			enemy = findPlayerByObjectName(enemy:objectName())
			local damage = {}
			damage.from = self.player
			damage.to = enemy
			damage.nature = sgs.DamageStruct_Fire
			damage.damage = 1
			if self:damageIsEffective_(damage) then
				if not enemy:isChained() or self:isGoodChainTarget_(damage) then
					shouldUse = true
				else
					shouldUse = false
					return
				end
			end
		end
		if shouldUse then
			use.card = card
		end
	end
end

sgs.ai_nullification.BurningCamps = function(self, card, from, to, positive)
	if positive then
		if self:isFriendWith(to) and self:isEnemy(from) then return true end
	else
		if self:isFriend(from) and self:isEnemy(to) then return true end
	end
	return
end

sgs.ai_use_value.BurningCamps = 7.1
sgs.ai_use_priority.BurningCamps = 4.7
sgs.ai_keep_value.BurningCamps = 3.38
sgs.ai_card_intention.BurningCamps = 10

--Breastplate
sgs.ai_skill_invoke.Breastplate = true

function sgs.ai_armor_value.Breastplate(player, self)
	if player:getHp() >= 3 then
		return 2
	else
		return 5.5
	end
end

sgs.ai_use_priority.Breastplate = 0.9

--LureTiger
function SmartAI:useCardLureTiger(card, use)
	sgs.ai_use_priority.LureTiger = 4.9
	if not card:isAvailable(self.player) then return end

	local players = sgs.PlayerList()

	local BurningCamps = self:getCard("BurningCamps")
	if BurningCamps and BurningCamps:isAvailable(self.player) then
		local nextp = self.room:nextPlayer(self.player)
		local first
		while true do
			if card:targetFilter(players, nextp, self.player) and self:hasTrickEffective(card, nextp, self.player) then
				if not first then
					if self:isEnemy(nextp) then
						first = nextp
					else
						players:append(nextp)
					end
				else
					if not first:isFriendWith(nextp) then
						players:append(nextp)
					end
				end
				nextp = self.room:nextPlayer(nextp)
			else
				break
			end
		end
		if first then
			use.card = card
			if use.to then use.to = sgs.PlayerList2SPlayerList(players) end
			return
		end
	end

	players = sgs.PlayerList()

	local ArcheryAttack = self:getCard("ArcheryAttack")
	if ArcheryAttack and ArcheryAttack:isAvailable(self.player) and self:getAoeValue(ArcheryAttack) > 0 then
		self:sort(self.friends_noself, "hp")
		for _, friend in ipairs(self.friends_noself) do
			if self:isFriendWith(friend) and card:targetFilter(players, friend, self.player) and self:hasTrickEffective(card, friend, self.player) then
				players:append(friend)
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if card:targetFilter(players, friend, self.player) and not players:contains(friend) and self:hasTrickEffective(card, friend, self.player) then
				players:append(friend)
			end
		end
		if players:length() > 0 then
			sgs.ai_use_priority.LureTiger = sgs.ai_use_priority.ArcheryAttack + 0.2
			use.card = card
			if use.to then use.to = sgs.PlayerList2SPlayerList(players) end
			return
		end
	end

	players = sgs.PlayerList()

	local SavageAssault = self:getCard("SavageAssault")
	if SavageAssault and SavageAssault:isAvailable(self.player) and self:getAoeValue(SavageAssault) > 0 then
		self:sort(self.friends_noself, "hp")
		for _, friend in ipairs(self.friends_noself) do
			if self:isFriendWith(friend) and card:targetFilter(players, friend, self.player) and self:aoeIsEffective(card, friend, self.player) then
				players:append(friend)
			end
		end
		for _, friend in ipairs(self.friends_noself) do
			if card:targetFilter(players, friend, self.player) and not players:contains(friend) and self:aoeIsEffective(card, friend, self.player) then
				players:append(friend)
			end
		end
		if players:length() > 0 then
			sgs.ai_use_priority.LureTiger = sgs.ai_use_priority.SavageAssault + 0.2
			use.card = card
			if use.to then use.to = sgs.PlayerList2SPlayerList(players) end
			return
		end
	end

	players = sgs.PlayerList()

	local Slash = self:getCard("Slash")
	if Slash and self:slashIsAvailable(self.player, Slash) then
		local total_num = 2 + sgs.Sanguosha:correctCardTarget(sgs.TargetModSkill_ExtraTarget, self.player, card)
		local dummyuse = { isDummy = true, to = sgs.SPlayerList(), distance = -total_num }
		self:useCardSlash(Slash, dummyuse)
		if dummyuse.card then

			local function getPlayersFromTo(one)
				local targets1 = sgs.PlayerList()
				local targets2 = sgs.PlayerList()
				local nextp = self.room:nextPlayer(self.player)
				while true do
					if card:targetFilter(targets1, nextp, self.player) and self:hasTrickEffective(card, nextp, self.player) then
						if one:objectName() ~= nextp:objectName() then
							targets1:append(nextp)
						else
							break
						end
						nextp = self.room:nextPlayer(nextp)
					else
						targets1 = sgs.PlayerList()
						break
					end
				end
				nextp = self.room:nextPlayer(one)
				while true do
					if card:targetFilter(targets2, nextp, self.player) and self:hasTrickEffective(card, nextp, self.player) then
						if self.player:objectName() ~= nextp:objectName() then
							targets2:append(nextp)
						else
							break
						end
						nextp = self.room:nextPlayer(nextp)
					else
						targets2 = sgs.PlayerList()
						break
					end
				end
				if targets1:length() > 0 and targets2:length() >= targets1:length() and targets1:length() <= total_num then
					return targets1
				elseif targets2:length() > 0 and targets1:length() >= targets2:length() and targets2:length() <= total_num then
					return targets2
				end
				return
			end

			for _, to in sgs.qlist(dummyuse.to) do
				if self.player:distanceTo(to) > self.player:getAttackRange() and self.player:distanceTo(to, -total_num) <= self.player:getAttackRange() then
					local sps = getPlayersFromTo(to)
					if sps then
						sgs.ai_use_priority.LureTiger = 3
						use.card = card
						if use.to then use.to = sgs.PlayerList2SPlayerList(sps) end
						return
					end
				end
			end
		end

	end

	players = sgs.PlayerList()

	local GodSalvation = self:getCard("GodSalvation")
	if GodSalvation and GodSalvation:isAvailable(self.player) then
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if card:targetFilter(players, enemy, self.player) and self:hasTrickEffective(card, enemy, self.player) then
				players:append(enemy)
			end
		end
		if players:length() > 0 then
			sgs.ai_use_priority.LureTiger = sgs.ai_use_priority.GodSalvation + 0.1
			use.card = card
			if use.to then use.to = sgs.PlayerList2SPlayerList(players) end
			return
		end
	end

	players = sgs.PlayerList()

	if self.player:objectName() == self.room:getCurrent():objectName() then
		for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if card:targetFilter(players, player, self.player) and self:hasTrickEffective(card, player, self.player) then
				sgs.ai_use_priority.LureTiger = 0.3
				use.card = card
				if use.to then use.to:append(player) end
				return
			end
		end
	end
end

sgs.ai_nullification.LureTiger = function(self, card, from, to, positive)
	if positive then
		if self:isFriendWith(to) and self:isEnemy(from) then return true end
	else
		if self:isFriend(from) and self:isEnemy(to) then return true end
	end
	return
end

sgs.ai_use_value.LureTiger = 5
sgs.ai_use_priority.LureTiger = 4.9
sgs.ai_keep_value.LureTiger = 3.22

--FightTogether
function SmartAI:useCardFightTogether(card, use)
	self.FightTogether_choice = nil
	if not card:isAvailable(self.player) then return end

	--@todo: consider hongfa
	local big_k, small_k = self:getBigAndSmallKingdoms()
	local big_p, small_p = {}, {}
	local isBig, isSmall
	for _,p in sgs.qlist(self.room:getAllPlayers()) do
		if self:hasTrickEffective(card, p, self.player) then
			local kingdom = p:objectName()
			if #big_k == 1 and big_k[1]:startsWith("sgs") then
				if table.contains(big_k, kingdom) then
					if self:isFriend(p) then isBig = true end
					table.insert(big_p, p)
				else
					if self:isFriend(p) then isSmall = true end
					table.insert(small_p, p)
				end
			else
				if not p:hasShownOneGeneral() then
					kingdom = "anjiang"
				elseif p:getRole() == "careerist" then
					kingdom = "careerist"
				else
					kingdom = p:getKingdom()
				end

				if table.contains(big_k, kingdom) then
					if self:isFriend(p) then isBig = true end
					table.insert(big_p, p)
				elseif table.contains(small_k, kingdom) then
					if self:isFriend(p) then isSmall = true end
					table.insert(small_p, p)
				end
			end
		end
	end

	local choices = {}
	if #big_p > 0 then table.insert(choices, "big") end
	if #small_p > 0 then table.insert(choices, "small") end
	if #choices == 0 then return end

	local v_big, v_small = 0, 0
	for _, p in ipairs(big_p) do
		v_big = v_big + (p:isChained() and -1 or 1)
	end
	if isBig then v_big = -v_big end
	for _, p in ipairs(small_p) do
		v_small = v_small + (p:isChained() and -1 or 1)
	end
	if isSmall then v_small = -v_small end

	local x = self:getOverflow() > 0 and -1 or 0
	if #choices == 1 then
		if table.contains(choices, "big") then
			if v_big > x then self.FightTogether_choice = "big" end
		else
			if v_small > x then self.FightTogether_choice = "small" end
		end
	else
		if isBig then
			if v_big > x and v_big == #big_p then self.FightTogether_choice = "big"
			elseif v_small > x then self.FightTogether_choice = "small"
			elseif v_big > x then self.FightTogether_choice = "big"
			end
		elseif isSmall then
			if v_small > x and v_small == #small_p then self.FightTogether_choice = "small"
			elseif v_big >= x then self.FightTogether_choice = "big"
			elseif v_small > x then self.FightTogether_choice = "small"
			end
		else
			if v_big > x and v_big > v_small then self.FightTogether_choice = "big"
			elseif v_small > x and v_small > v_big then self.FightTogether_choice = "small"
			elseif  v_big == v_small and v_big >= x then
				if #big_p > #small_p then return "big"
				elseif #big_p < #small_p then return "small"
				else
					return math.random(1, 2) == 1 and "big" or "small"
				end
			end
		end
	end

	if self.FightTogether_choice then
		use.card = card
	end

end

sgs.ai_skill_choice["fight_together"] = function(self, choices)
	choices = choices:split("+")
	if self.FightTogether_choice and table.contains(choices, self.FightTogether_choice) then
		return self.FightTogether_choice
	end
	return choices[1]
end

sgs.ai_nullification.FightTogether = function(self, card, from, to, positive)
	if positive then
		if to:isChained() then
			if self:isEnemy(to) and to:hasShownSkills(sgs.cardneed_skill .. "|" .. sgs.priority_skill .. "|" .. sgs.wizard_harm_skill) then return true end
		else
			if self:isFriendWith(to) and to:hasArmorEffect("Vine") then return true end
		end
	else
		if self:isFriendWith(to) and to:isChained() then return true end
	end
	return
end

sgs.ai_use_value.FightTogether = 5.2
sgs.ai_use_priority.FightTogether = 8.9
sgs.ai_keep_value.FightTogether = 3.24

--AllianceFeast
function SmartAI:useCardAllianceFeast(card, use)
	if not card:isAvailable(self.player) then return end
	local targets = {}
	local isEnemy
	for _, friend in ipairs(self.friends_noself) do
		if not self.player:isFriendWith(friend) and friend:hasShownOneGeneral() and self:hasTrickEffective(card, friend, self.player) then
			table.insert(targets, friend)
		end
	end
	if #targets == 0 then
		for _, target in sgs.qlist(self.room:getOtherPlayers(self.player)) do
			if not self.player:isFriendWith(target) and target:hasShownOneGeneral() and not self:isEnemy(target) and self:hasTrickEffective(card, target, self.player) then
				table.insert(targets, target)
			end
		end
	end
	if #targets == 0 then
		isEnemy = true
		for _, enemy in ipairs(self.enemies) do
			if not self.player:isFriendWith(enemy) and enemy:hasShownOneGeneral() and self:hasTrickEffective(card, enemy, self.player) then
				table.insert(targets, enemy)
			end
		end
	end

	if #targets > 0 then
		local function cmp_k(a, b)
			local v1 = a:getPlayerNumWithSameKingdom("AI")
			local v2 = b:getPlayerNumWithSameKingdom("AI")
			return v1 > v2
		end
		table.sort(targets, cmp_k)
		if isEnemy then targets = sgs.reverse(targets) end
		use.card = card
		if use.to then use.to:append(targets[1]) end
		return
	end
end

sgs.ai_skill_choice["alliance_feast"] = function(self, choices)
	if self.player:isWounded() then
		return "recover"
	else
		return "draw"
	end
end
sgs.ai_use_value.AllianceFeast = 7.5
sgs.ai_use_priority.AllianceFeast = 8.8
sgs.ai_keep_value.AllianceFeast = 3.26

sgs.ai_nullification.AllianceFeast = function(self, card, from, to, positive)
	return
end


--ThreatenEmperor
function SmartAI:useCardThreatenEmperor(card, use)
	if not card:isAvailable(self.player) then return end
	if self:getHandcardNum() < 2 then return end
	use.card = card
end
sgs.ai_use_value.ThreatenEmperor = 8
sgs.ai_use_priority.ThreatenEmperor = 0
sgs.ai_keep_value.ThreatenEmperor = 3.2

sgs.ai_nullification.ThreatenEmperor = function(self, card, from, to, positive)
	if positive then
		if self:isEnemy(from) then return true end
	else
		if self:isFriend(from) then return true end
	end
	return
end

sgs.ai_skill_cardask["@threaten_emperor"] = function(self)
	if self.player:isKongcheng() then return "." end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	return cards[1]:getEffectiveId()
end

--ImperialOrder
function SmartAI:useCardImperialOrder(card, use)
	if not card:isAvailable(self.player) then return end
	use.card = card
end
sgs.ai_use_value.ImperialOrder = 2.9
sgs.ai_use_priority.ImperialOrder = 8.9
sgs.ai_keep_value.ImperialOrder = 0

sgs.ai_nullification.ImperialOrder = function(self, card, from, to, positive)
	return
end

sgs.ai_skill_cardask["@imperial_order-equip"] = function(self)
	if self:needToThrowArmor() then
		return self.player:getArmor():getEffectiveId()
	end
	return "."
end

sgs.ai_skill_choice.imperial_order = function(self)
	if self:needToLoseHp() then return "losehp" end
	return "show"
end


--JadeSeal
sgs.ai_skill_use["@@JadeSeal!"] = function(self, prompt, method)
	local card = sgs.cloneCard("known_both")
	local dummyuse = { isDummy = true, to = sgs.SPlayerList() }
	self:useCardKnownBoth(card, dummyuse)
	local tos = {}
	if dummyuse.card and not dummyuse.to:isEmpty() then
		for _, to in sgs.qlist(dummyuse.to) do
			table.insert(tos, to:objectName())
		end
		return "known_both:JadeSeal[no_suit:0]=.&->" .. table.concat(tos, "+")
	end
	self:sort(self.enemies, "handcard")
	self.enemies = sgs.reverse(self.enemies)
	local targets = sgs.PlayerList()
	for _, enemy in ipairs(self.enemies) do
		if self:getKnownNum(enemy, self.player) ~= enemy:getHandcardNum() and card:targetFilter(targets, enemy, self.player) and not targets:contains(enemy) then
			targets:append(enemy)
			table.insert(tos, enemy:objectName())
			self.knownboth_choice[enemy:objectName()] = "handcards"
		end
	end
	self:sort(self.friends_noself, "handcard")
	self.friends_noself = sgs.reverse(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if self:getKnownNum(friend, self.player) ~= friend:getHandcardNum() and card:targetFilter(targets, friend, self.player) and not targets:contains(friend) then
			targets:append(friend)
			table.insert(tos, friend:objectName())
			self.knownboth_choice[friend:objectName()] = "handcards"
		end
	end

	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	self:sort(players, "handcard")
	players = sgs.reverse(players)
	for _, player in ipairs(players) do
		if card:targetFilter(targets, player, self.player) and not targets:contains(player) then
			targets:append(player)
			table.insert(tos, player:objectName())
			self.knownboth_choice[player:objectName()] = "handcards"
		end
	end
	assert(#tos > 0)
	return "known_both:JadeSeal[no_suit:0]=.&->" .. table.concat(tos, "+")
end

sgs.ai_use_priority.JadeSeal = 5.6
sgs.ai_keep_value.JadeSeal = 2.02

--Halberd
sgs.ai_view_as.Halberd = function(card, player, card_place)
	if card_place == sgs.Player_PlaceHand and card:isKindOf("Slash") and not player:hasFlag("Global_HalberdFailed") and not player:hasFlag("slashDisableExtraTarget")
		and sgs.Sanguosha:getCurrentCardUseReason() == sgs.CardUseStruct_CARD_USE_REASON_RESPONSE_USE and player:getMark("Equips_Nullified_to_Yourself") == 0 then
		return "@HalberdCard=."
	end
end

local Halberd_skill = {}
Halberd_skill.name = "Halberd"
table.insert(sgs.ai_skills, Halberd_skill)
Halberd_skill.getTurnUseCard = function(self, inclusive)
	if self.player:hasFlag("Global_HalberdFailed") or not self:slashIsAvailable() or self.player:getMark("Equips_Nullified_to_Yourself") > 0 then return end
	if self:getCard("Slash") then
		local HalberdCard = sgs.Card_Parse("@HalberdCard=.")
		assert(HalberdCard)
		return HalberdCard
	end
end

sgs.ai_skill_use_func.HalberdCard = function(card, use, self)
	local slash = self:getCard("Slash")
	self:useCardSlash(slash, use)
	if use.card and use.card:isKindOf("Analeptic") then
		return
	end
	use.card = card
	if use.to then use.to = sgs.SPlayerList() end
end


sgs.ai_use_priority.HalberdCard = sgs.ai_use_priority.Slash + 0.1

sgs.ai_skill_playerchosen.Halberd = sgs.ai_skill_playerchosen.slash_extra_targets

sgs.ai_skill_cardask["@halberd"] = function(self)
	local cards = sgs.QList2Table(self.player:getCards("Slash"))
	self:sortByUseValue(cards)
	for _, slash in ipairs(cards) do
		if slash:isKindOf("HalberdCard") then continue end
		local use = { to = sgs.SPlayerList() }
		local target
		if self.player:hasFlag("slashTargetFix") then
			for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
				if player:hasFlag("SlashAssignee") then
					if self.player:canSlash(player, slash) then
						use.to:append(player)
						target = player
						break
					else
						return "."
					end
				end
			end
		end
		self:useCardSlash(slash, use)
		local targets = {}
		for _, p in sgs.qlist(use.to) do
			table.insert(targets, p:objectName())
		end
		if #targets > 0 and (not target or table.contains(targets, target:objectName())) then return slash:toString() .. "->" .. table.concat(targets, "+") end
	end
	return "."
end

function sgs.ai_weapon_value.Halberd(self, enemy, player)
	return 2.1
end