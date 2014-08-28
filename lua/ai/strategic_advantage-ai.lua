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
		if card:targetFilter(players, friend, self.player) and not players:contains(friend) and self:needToThrowArmor(friend) then
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

sgs.ai_use_value.Drowning = 5
sgs.ai_use_priority.Drowning = 7

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

local transfer_skill = {}
transfer_skill.name = "transfer"
table.insert(sgs.ai_skills, transfer_skill)
transfer_skill.getTurnUseCard = function(self, inclusive)
	if self.player:isKongcheng() then return end
	if not self.player:hasSkill("kongcheng") then
		if self:isWeak() and self:getOverflow() <= 0 then return end
		if not self.player:hasShownOneGeneral() then return end
	end
	return sgs.Card_Parse("@TransferCard=.")
end

sgs.ai_skill_use_func.TransferCard = function(card, use, self)

	local friends, friends_other, isWeakFriend = {}, {}
	for _, friend in ipairs(self.friends_noself) do
		if not self:needKongcheng(friend, true) and friend:hasShownOneGeneral() then
			if self.player:isFriendWith(friend) then
				if self:isWeak(friend) then isWeakFriend = true end
				table.insert(friends, friend)
			else
				table.insert(friends_other, friend)
			end
		end
	end
	if isWeakFriend then friends_other = {} end
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

	if #friends_other > 0 then
		local card, target = self:getCardNeedPlayer(cards, friends_other)
		if card and target then
			use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
			if use.to then use.to:append(target) end
			return
		end
	end

	if #friends == 0 then return end

	cards = {}
	oneJink = self.player:hasSkill("kongcheng")
	for _, c in sgs.qlist(self.player:getHandcards()) do
		if c:isTransferable() then
			if not oneJink and isCard("Jink", c, self.player) then
				oneJink = true
				continue
			end
			table.insert(cards, c)
		end
	end
	if #cards == 0 then return end

	local card, target = self:getCardNeedPlayer(cards, friends)
	if card and target then
		use.card = sgs.Card_Parse("@TransferCard=" .. card:getEffectiveId())
		if use.to then use.to:append(target) end
		return
	elseif self:getOverflow() > 0 then
		self:sort(friends, "handcard")
		use.card = sgs.Card_Parse("@TransferCard=" .. cards[1]:getEffectiveId())
		if use.to then use.to:append(friends[1]) end
		return
	end
end

sgs.ai_use_priority.TransferCard = 0
sgs.ai_card_intention.TransferCard = -10

function sgs.ai_armor_value.IronArmor(player, self)
	return 2.5
end

sgs.ai_use_priority.IronArmor = 0.82

--[[
function SmartAI:useCardBurningCamps(card, use)
	if not card:isAvailable(self.player) then return end
	local player = self.room:nextPlayer(self.player)
	if self:isFriendWith(player) then
		return
	else
		local enemies = player:getFormation()
		local shouldUse
		for _, enemy in sgs.qlist(enemies) do
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


sgs.ai_use_value.BurningCamps = 7
sgs.ai_use_priority.BurningCamps = 4.7
sgs.ai_card_intention.BurningCamps = 10
]]