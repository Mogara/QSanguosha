-- liefu
sgs.ai_skill_invoke["liefu"] = function(self, data)
	local effect = data:toSlashEffect()
	self.liefutarget = effect.to
	return self:isEnemy(effect.to)
end
sgs.ai_skill_choice["liefu"] = function(self, choices)
	if self.player:getHandcardNum() < 3 then
		return "pan"
	elseif self.liefutarget:getHp() < 3 then
		return "feng"
	end
	return "pan"
end

-- gangli
sgs.ai_skill_invoke["gangli"] = function(self, data)
--	local effect = data:toDamage()
	return #self.enemies > 1
end
sgs.ai_skill_playerchosen["gangli"] = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets,"defense")
	for _, target in ipairs(targets) do
		if self:isEnemy(target) then
			return target
		end
	end
end

-- yaliang
sgs.ai_skill_invoke["yaliang"] = function(self, data)
	local effect = data:toCardEffect()
	if effect.card:inherits("GlobalEffect") then return false end
	return self:getCardsNum("Jink") > 0
end

-- xia0guo
sgs.ai_skill_invoke["xia0guo"] = function(self, data)
	local effect = data:toSlashEffect()
	return self:isEnemy(effect.to)
end

-- jingrui
local jingrui_skill={}
jingrui_skill.name="jingrui"
table.insert(sgs.ai_skills,jingrui_skill)
jingrui_skill.getTurnUseCard=function(self)
	if self.player:isKongcheng() or self.player:getHandcardNum() < self.player:getHp() then return end
	local cards = self.player:getCards("h")	
	cards=sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	local jink_card = cards[1]
	local suit = jink_card:getSuitString()
	local number = jink_card:getNumberString()
	local card_id = jink_card:getEffectiveId()
	local card_str = ("slash:jingrui[%s:%s]=%d"):format(suit, number, card_id)
	local slash = sgs.Card_Parse(card_str)
	assert(slash)
	return slash
end

sgs.ai_view_as["jingrui"] = function(card, player, card_place)
	if player:isKongcheng() or player:getHandcardNum() < player:getHp() then return end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	return ("slash:jingrui[%s:%s]=%d"):format(suit, number, card_id) or ("jink:jingrui[%s:%s]=%d"):format(suit, number, card_id)
end

-- yinsi
yinsi_skill={}
yinsi_skill.name="yinsi"
table.insert(sgs.ai_skills,yinsi_skill)
yinsi_skill.getTurnUseCard=function(self)
	local cards = self.player:getCards("he")
	cards=sgs.QList2Table(cards)
	local card
	self:sortByUseValue(cards,true)
	for _,acard in ipairs(cards)  do
		if acard:inherits("EquipCard") then
			card = acard
			break
		end
	end
	if not card then return nil end
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	local card_str = ("analeptic:yinsi[%s:%s]=%d"):format(suit, number, card_id)
	local analeptic = sgs.Card_Parse(card_str)
	assert(analeptic)
	return analeptic
end
sgs.ai_view_as["yinsi"] = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card:inherits("EquipCard") then
		return ("analeptic:yinsi[%s:%s]=%d"):format(suit, number, card_id)
	end
end

-- yanhe
sgs.ai_skill_invoke["yanhe"] = function(self, data)
	for _, player in ipairs(self.friends_noself) do
		if self:hasSkills(sgs.lose_equip_skill, player) and not player:getEquips():isEmpty() then
			self.yanhetarget = player
			return true
		end
	end
	return false
end
sgs.ai_skill_playerchosen["yanhe"] = function(self, targets)
	return self.yanhetarget
end

-- quanjian
quanjian_skill={}
quanjian_skill.name="quanjian"
table.insert(sgs.ai_skills,quanjian_skill)
quanjian_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("QuanjianCard") then return end
	local hcards = self.player:getCards("h")
	hcards = sgs.QList2Table(hcards)
	self:sortByUseValue(hcards, true)
	for _, hcard in ipairs(hcards) do
		if hcard:inherits("Jink") then
			card = hcard
			break
		end
	end
	local cani = false
	if card then
		for _, friend in ipairs(self.friends_noself) do
			if friend:isKongcheng() then
				cani = true
				break
			end
		end
		if cani then
			card = sgs.Card_Parse("@QuanjianCard=" .. card:getEffectiveId())
			return card
		end
	end
	return nil
end
sgs.ai_skill_use_func["QuanjianCard"]=function(card,use,self)
	self:sort(self.friends_noself, "defense")
	for _, friend in ipairs(self.friends_noself) do
		if friend:isKongcheng() and not (friend:hasSkill("kongcheng") and friend:isKongcheng()) then
			use.card=card
			if use.to then
				use.to:append(friend)
			end
			return
		end
	end
end

-- sijie
sgs.ai_skill_use["@@sijie"] = function(self, prompt)
	local target
	self:sort(self.enemies)
	for _, player in ipairs(self.enemies) do
		if player:isWounded() and not player:isNude() then
			return "@SijieCard=.->" .. player:objectName()
		end
	end
	return "."
end
