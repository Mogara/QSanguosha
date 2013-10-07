function sgs.ai_skill_invoke.ziliang(self, data)
	self.ziliang_id = nil
	local damage = data:toDamage()
	if damage.to:hasSkill("manjuan") and damage.to:getPhase() == sgs.Player_NotActive then return false end
	if not self:isFriend(damage.to) then
		if damage.to:getPhase() == sgs.Player_NotActive and self:needKongcheng(damage.to, true) then
			local ids = sgs.QList2Table(self.player:getPile("field"))
			for _, id in ipairs(ids) do
				local card = sgs.Sanguosha:getCard(id)
				if card:isKindOf("Disaster") or card:isKindOf("GodSalvation") or card:isKindOf("AmazingGrace") or card:isKindOf("FireAttack") then
					self.ziliang_id = id
					return true
				end
			end
		else
			return false
		end
	else
		if not (damage.to:getPhase() == sgs.Player_NotActive and self:needKongcheng(damage.to, true)) then
			local ids = sgs.QList2Table(self.player:getPile("field"))
			local cards = {}
			for _, id in ipairs(ids) do table.insert(cards, sgs.Sanguosha:getCard(id)) end
			for _, card in ipairs(cards) do
				if card:isKindOf("Peach") then self.ziliang_id = card:getEffectiveId() return true end
			end
			for _, card in ipairs(cards) do
				if card:isKindOf("Jink") then self.ziliang_id = card:getEffectiveId() return true end
			end
			self:sortByKeepValue(cards, true)
			self.ziliang_id = cards[1]:getEffectiveId()
			return true
		else
			return false
		end
	end
end

sgs.ai_skill_askforag.ziliang = function(self, card_ids)
	return self.ziliang_id
end

sgs.ai_choicemade_filter.skillInvoke.ziliang = function(self, player, promptlist)
	local damage = self.room:getTag("CurrentDamageStruct"):toDamage()
	if damage.to and promptlist[#promptlist] == "yes" then
		local intention = -40
		if damage.to:getPhase() == sgs.Player_NotActive and self:needKongcheng(damage.to, true) then intention = 10 end
		sgs.updateIntention(player, damage.to, intention)
	end
end

local function huyuan_validate(self, equip_type, is_handcard)
	local targets
	if is_handcard then targets = self.friends else targets = self.friends_noself end
	if equip_type == "SilverLion" then
		for _, enemy in ipairs(self.enemies) do
			if enemy:hasSkills("yizhong|bazhen") then table.insert(targets, enemy) end
		end
	end
	for _, friend in ipairs(targets) do
		local has_equip = false
		for _, equip in sgs.qlist(friend:getEquips()) do
			if equip:isKindOf(equip_type) then
				has_equip = true
				break
			end
		end
		if not has_equip and not ((equip_type == "Armor" or equip_type == "SilverLion") and friend:hasSkills("yizhong|bazhen")) then
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
	if self.player:hasArmorEffect("silver_lion") then
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
	if to[1]:hasSkills("bazhen|yizhong") then
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

sgs.ai_skill_use["@@heyi"] = function(self, prompt)
	local players = sgs.QList2Table(self.room:getOtherPlayers(self.player))
	local first, last = self.player, self.player
	for i = 1, #players do
		if self:isFriend(players[i]) then last = players[i] else break end
	end
	for i = #players, 1, -1 do
		if self:isFriend(players[i]) then first = players[i] else break end
	end
	if first:objectName() == self.player:objectName() and last:objectName() == self.player:objectName() then return "." end
	return ("@HeyiCard=.->%s+%s"):format(first:objectName(), last:objectName())
end

sgs.ai_card_intention.HeyiCard = function(self, card, from, tos)
	local players = sgs.QList2Table(self.room:getOtherPlayers(from))
	local first, last = tos[1], tos[2]
	if first:objectName() == from:objectName() then
		for i = 1, #players do
			if players[i]:objectName() ~= last:objectName() then sgs.updateIntention(from, players[i], -60) else break end
		end
		sgs.updateIntention(from, last, -60)
	elseif last:objectName() == from:objectName() then
		for i = #players, 1, -1 do
			if players[i]:objectName() ~= first:objectName() then sgs.updateIntention(from, players[i], -60) else break end
		end
		sgs.updateIntention(from, first, -60)
	else
		if table.indexOf(players, first) < table.index(player, last) then
			first = tos[2]
			last = tos[1]
		end
		for i = 1, #players do
			if players[i]:objectName() ~= last:objectName() then sgs.updateIntention(from, players[i], -60) else break end
		end
		for i = #players, 1, -1 do
			if players[i]:objectName() ~= first:objectName() then sgs.updateIntention(from, players[i], -60) else break end
		end
		sgs.updateIntention(from, last, -60)
		sgs.updateIntention(from, first, -60)
	end
end

sgs.ai_skill_invoke.tianfu = function(self, data)
	local jiangwei = data:toPlayer()
	return jiangwei and self:isFriend(jiangwei)
end

sgs.ai_skill_invoke.shoucheng = function(self, data)
	local move = data:toMoveOneTime()
	return move.from and self:isFriend(move.from)
			and not (move.from:getPhase() == sgs.Player_NotActive and (move.from:hasSkill("manjuan") or self:needKongcheng(move.from, true)))
end

sgs.ai_skill_choice.shoucheng = function(self, choices)
	return (self.player:getPhase() == sgs.Player_NotActive and self:needKongcheng(self.player, true)) and "reject" or "accept"
end

local shangyi_skill = {}
shangyi_skill.name = "shangyi"
table.insert(sgs.ai_skills, shangyi_skill)
shangyi_skill.getTurnUseCard = function(self)
	local card_str = ("@ShangyiCard=.")
	local shangyi_card = sgs.Card_Parse(card_str)
	assert(shangyi_card)
	return shangyi_card
end

sgs.ai_skill_use_func.ShangyiCard = function(card, use, self)
	if self.player:hasUsed("ShangyiCard") then return end
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

sgs.ai_skill_invoke.niaoxiang = function(self, data)
	local p = data:toPlayer()
	if not self:isEnemy(p) then return false end
	if p:hasSkills("leiji|nosleiji") and getCardsNum("Jink", p) >= 1 then return false end
	return true
end

sgs.ai_skill_invoke.yicheng = function(self, data)
	local player = data:toPlayer()
	if player:hasSkill("manjuan") and player:getPhase() == sgs.Player_NotActive then
		if player:canDiscard(player, "he") then return self:isEnemy(player) else return false end
	else
		return self:isFriend(player)
	end
end

sgs.ai_skill_discard.yicheng = function(self, discard_num, min_num, optional, include_equip)
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
	local use = data:toCardUse()
	if not use.card then
		local yuji = self.room:findPlayerBySkillName("qianhuan")
		return yuji and self:isFriend(yuji) and yuji:getPile("sorcery"):length() < 4
	else
		local to = use.to:first()
		if to:objectName() == self.player:objectName() then
			return not (use.from and (use.from:objectName() == to:objectName()
										or (use.card:isKindOf("Slash") and self:isPriorFriendOfSlash(self.player, use.card, use.from))))
		else
			return self:isFriend(to) and not (use.from and use.from:objectName() == to:objectName())
		end
	end
end

sgs.ai_skill_choice.qianhuan = function(self, choices, data)
	local use = data:toCardUse()
	if use.card:isKindOf("Peach") or use.card:isKindOf("Analeptic") or use.card:isKindOf("ExNihilo") then return "reject" end
	if use.from and use.from:objectName() == self.player:objectName() then return "reject" end
	if use.from and use.card:isKindOf("Slash") and self:isPriorFriendOfSlash(self.player, use.card, use.from) then return "reject" end
	return "accept"
end

local function will_discard_zhendu(self)
	local current = self.room:getCurrent()
	local need_damage = self:getDamagedEffects(current, self.player) or self:needToLoseHp(current, self.player)
	if self:isFriend(current) then
		if current:getMark("drank") > 0 and not need_damage then return -1 end
		if (getKnownCard(current, "Slash") > 0 or (getCardsNum("Slash", current) >= 1 and current:getHandcardNum() >= 2))
			and (not self:damageIsEffective(current, nil, self.player) or current:getHp() > 2 or (getCardsNum("Peach", current) > 1 and not self:isWeak(current))) then
			local slash = sgs.Sanguosha:cloneCard("slash")
			local trend = 3
			if current:hasWeapon("axe") then trend = trend - 1
			elseif current:hasSkills("liegong|kofliegong|tieji|wushuang|niaoxiang") then trend = trend - 0.4 end
			for _, enemy in ipairs(self.enemies) do
				if current:canSlash(enemy) and not self:slashProhibit(slash, enemy, current)
					and self:slashIsEffective(slash, enemy, current) and sgs.isGoodTarget(enemy, self.enemies, self, true) then
					return trend
				end
			end
		end
		if need_damage then return 3 end
	elseif self:isEnemy(current) then
		if need_damage or current:getHandcardNum() >= 2 then return -1 end
		if getKnownCard(current, "Slash") == 0 and getCardsNum("Slash", current) < 0.5 then return 3.5 end
	end
	return -1
end

sgs.ai_skill_cardask["@zhendu-discard"] = function(self, data)
	local discard_trend = will_discard_zhendu(self)
	if discard_trend <= 0 then return "." end
	if self.player:getHandcardNum() + math.random(1, 100) / 100 >= discard_trend then
		local cards = sgs.QList2Table(self.player:getHandcards())
		self:sortByKeepValue(cards)
		for _, card in ipairs(cards) do
			if not self:isValuableCard(card, self.player) then return "$" .. card:getEffectiveId() end
		end
	end
	return "."
end