sgs.ai_skill_playerchosen.koftuxi = function(self, targets)
	local cardstr = sgs.ai_skill_use["@@tuxi"](self, "@tuxi")
	if cardstr:match("->") then
		local targetstr = cardstr:split("->")[2]:split("+")
		if #targetstr > 0 then
			local target = findPlayerByObjectName(self.room, targetstr[1])
			return target
		end
	end
	return nil
end

sgs.ai_playerchosen_intention.koftuxi = function(self, from, to)
	local lord = self.room:getLord()
	if sgs.evaluatePlayerRole(from) == "neutral" and sgs.evaluatePlayerRole(to) == "neutral"
		and lord and not lord:isKongcheng()
		and not self:doNotDiscard(lord, "h", true) and from:aliveCount() >= 4 then
		sgs.updateIntention(from, lord, -35)
		return
	end
	if from:getState() == "online" then
		if (to:hasSkills("kongcheng|zhiji|lianying") and to:getHandcardNum() == 1) or to:hasSkills("tuntian+zaoxian") then
		else
			sgs.updateIntention(from, to, 80)
		end
	else
		local intention = from:hasFlag("tuxi_isfriend_" .. to:objectName()) and -5 or 80
		sgs.updateIntention(from, to, intention)
	end
end

sgs.ai_chaofeng.kof_zhangliao = 4

sgs.ai_view_as.kofqingguo = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_PlaceEquip then
		return ("jink:kofqingguo[%s:%s]=%d"):format(suit, number, card_id)
	end
end

function sgs.ai_cardneed.kofqingguo(to, card)
	if card:isKindOf("Weapon") then return not to:getWeapon()
	elseif card:isKindOf("Armor") then return not to:getArmor()
	elseif card:isKindOf("OffensiveHorse") then return not to:getOffensiveHorse()
	elseif card:isKindOf("DefensiveHorse") then return not to:getDefensiveHorse()
	end
	return false
end


sgs.ai_skill_invoke.kofliegong = sgs.ai_skill_invoke.liegong

function sgs.ai_cardneed.kofliegong(to, card)
	return isCard("Slash", card, to) and getKnownCard(to, "Slash", true) == 0
end

sgs.ai_skill_invoke.yinli = function(self)
	return not self:needKongcheng(self.player, true)
end

sgs.ai_skill_askforag.yinli = function(self, card_ids)
	if self:needKongcheng(self.player, true) then return card_ids[1] else return -1 end
end

sgs.ai_skill_choice.kofxiaoji = function(self, choices)
	if choices:match("recover") then return "recover" else return "draw" end
end

sgs.kofxiaoji_keep_value = sgs.xiaoji_keep_value

local mouzhu_skill = {}
mouzhu_skill.name = "mouzhu"
table.insert(sgs.ai_skills, mouzhu_skill)
mouzhu_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("MouzhuCard") then return end
	return sgs.Card_Parse("@MouzhuCard=.")
end

sgs.ai_skill_use_func.MouzhuCard = function(card, use, self)

	local canleiji
	if self.player:hasSkill("leiji") and self:findLeijiTarget(self.player, 51) and self:hasSuit("spade", true) then
		canleiji = true
		self:sort(self.friends_noself, "handcard")
		sgs.reverse(self.friends_noself)
		for _, friend in ipairs(self.friend_noself) do
			if not friend:isKongcheng() and friend:getHandcardNum() < self.player:getHandcardNum() + 2
				and (self.player:getCardsNum("Jink") > 0 or not IgnoreArmor(friend, to) and not self:isWeak() and self:hasEightDiagramEffect()) then
				use.card = card
				if use.to then use.to:append(friend) end
				return
			end
		end
	end

	for _, enemy in ipairs(self.enemies) do	
		if enemy:getHandcardNum() > 0 and  (self:getDamagedEffects(self.player, enemy) or self:needToLoseHp(self.player, enemy)) then
			use.card = card
			if use.to then use.to:append(enemy) end
			return
		end
	end
	
	local first, second, third, fourth
	local slash = self:getCard("Slash")
	local slash_nosuit = sgs.Sanguosha:cloneCard("slash")
	for _, enemy in ipairs(self.enemies) do
		if enemy:getHandcardNum() >= self.player:getHandcardNum() + 2 then
			first = enemy
			break
		elseif enemy:getHandcardNum() > 0 then
			if not self:slashIsEffective(slash_nosuit, self.player, nil, enemy) and self:getCardsNum("Slash") > getCardsNum("Slash", enemy) and not second then
				second = enemy
			elseif not enemy:hasSkills("wushuang|mengjin|tieji")
				and not ((enemy:hasSkill("roulin") or enemy:hasWeapon("DoubleSword")) and enemy:getGender() ~= self.player:getGender()) then
				
				if enemy:getHandcardNum() == 1 and slash and not third and self.player:inMyAttackRange(enemy)
					and (self:hasHeavySlashDamage(self.player, slash, enemy) or self.player:hasWeapon("GudingBlade") and not self:needKongcheng(enemy))
					and (not self:isWeak() or self:getCardsNum("Peach") + self:getCardsNum("Analeptic") > 0) then
					third = enemy
				elseif self:getCardsNum("Jink") > 0 and self:getCardsNum("Slash") > getCardsNum("Slash", enemy) and not fourth then
					fourth = enemy
				end
			end
		end
	end
	
	local target
	if canleiji then
		target = fourth and third or first or second
	else
		target = first or second or third or fourth
	end
	if target then
		use.card = card
		if use.to then use.to:append(target) end
		return
	end
	
end

sgs.ai_skill_cardask["@mouzhu-give"] = function(self, data)
	local target = self.room:getCurrent()
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	if self:isFriend(target) then
		if target:hasSkill("leiji") then
			local jink, spade
			for _, c in ipairs(cards) do
				if isCard("Jink", c, target) then jink = c:getEffectiveId() end
				if c:getSuit() == sgs.Card_Spade then spade = c:getEffectiveId() end
			end
			if self:hasSuit("spade", true, target) and jink then return jink
			elseif not self:hasEightDiagramEffect(target) and jink then return jink
			elseif spade or jink then return spade or jink
			end
		end
		
	else
		if target:hasSkill("leiji") then
			for _, c in ipairs(cards) do
				if not c:isKindOf("Peach") and not c:isKindOf("Jink") and c:getSuit() ~= sgs.Card_Spade then
					return c:getEffectiveId()
				end
			end
		end
		for _, c in ipairs(cards) do
			if not c:isKindOf("Peach") then return c:getEffectiveId() end
		end
	end
	
	return cards[1]:getEffectiveId()
end

sgs.ai_skill_choice.mouzhu = function(self, choices)
	local target = self.room:getCurrent()
	local slash = sgs.Sanguosha:cloneCard("slash")
	if target:hasSkill("leiji") then
		if self:isFriend(target) then
			if choices:match("slash") then return "slash" end
		else
			if choices:match("duel") then return "duel" end
		end
	end
	
	if self:isFriend(target) then
		if (target:hasSkill("leiji") or not self:slashIsEffective(slash, target)) and choices:match("slash") then return "slash" end
		if self:getDamagedEffects(self.player, target) and getCardsNum("Slash", target) >= 1 and choices:match("duel") then return "duel" end
	else
		if target:hasSkill("leiji") and choices:match("duel") then return "duel" end
		if self:getCardsNum("Slash") > getCardsNum("Slash", target) and choices:match("duel") then return "duel" end
	end
	
	if choices:match("slash") then return "slash" else return "duel" end
end

sgs.ai_use_priority.MouzhuCard = 5.5

sgs.ai_card_intention.MouzhuCard = function(self, card, from, tos)
	if not self.player:hasSkill("leiji") then sgs.updateIntention(from, tos[1], 30) end
end

sgs.ai_skill_invoke.yanhuo = function(self, data)
	local opponent = self.player:getOtherPlayers(self.player, true):first()
	return opponent:isAlive() and not self:doNotDiscard(opponent)
end

sgs.ai_skill_playerchosen.yanhuo = function(self, targets)
	local target = self:findPlayerToDiscard(nil, nil, nil, targets)
	if target and target:objectName() ~= self.player:objectName() then return target end
end
