--技能：奢靡
local shemi_skill = {}
shemi_skill.name = "shemi" --只保证能否发动
table.insert(sgs.ai_skills, shemi_skill)
shemi_skill.getTurnUseCard = function(self)
	if self.player:hasUsed("ShemiAG") then return end
	local usenum = 2
	local usecard
	local hp = self.player:getHp()
	if hp <= 2 then usenum = 1 end
	local first_card, second_card 
	if self.player:getCards("he"):length() >= usenum then
		local flag = "he"
		if self:needToThrowArmor() and self.player:getArmor() then flag = "eh" 
			elseif self:getOverflow() > 0 then flag = "h" 
		end
		local cds = sgs.QList2Table(self.player:getCards(flag))
		local eid = self:getValuableCard(self.player) 
		self:sortByCardNeed(cds)
		for _, fcard in ipairs(cds) do
			if  not (fcard:isKindOf("Peach") or fcard:isKindOf("ExNihilo") or fcard:isKindOf("AmazingGrace")) and (fcard:getEffectiveId() ~= eid or not eid) then
				first_card = fcard
				if hp <= 2 then break end
				for _, scard in ipairs(cds) do
					if hp <= 2 then break end
					if first_card ~= scard and not (scard:isKindOf("Peach") or scard:isKindOf("ExNihilo") or scard:isKindOf("AmazingGrace")) and (scard:getEffectiveId() ~= eid or not eid) then
						second_card = scard
						local am = sgs.Sanguosha:cloneCard("amazing_grace", sgs.Card_NoSuit, 0)
						am:addSubcard(first_card)
						am:addSubcard(second_card)
						local dummy_use = {isDummy = true}
						self:useTrickCard(am, dummy_use)
						if not dummy_use.card then first_card=nil;second_card=nil break end	
					end
				end
				if first_card and second_card then break end
			end
		end
	end
	if usenum == 2 then
		if first_card and second_card then
		local first_id, second_id =  first_card:getId(), second_card:getId()
		local amazing = sgs.Card_Parse(("amazing_grace:shemi[to_be_decided:0]=%d+%d"):format(first_id, second_id))
		assert(amazing)
		return amazing
		end
	else	
		 usecard = first_card 
		 if usecard then
			local usecardid = usecard:getId()
			local amazing = sgs.Card_Parse(("amazing_grace:shemi[to_be_decided:0]=%d"):format(usecardid))
			assert(amazing)
			return amazing
		end
	end	
end

--技能：宽惠
local function need_kuanhui(self, who)
	local card = sgs.Card_Parse(self.room:getTag("kuanhui_user"):toString())
	if card == nil then return false end
	local from = self.room:getCurrent()
	if self:isEnemy(who) then
		if card:isKindOf("GodSalvation") and who:isWounded() and self:hasTrickEffective(card, who, from) then
			if who:hasSkill("manjuan") and who:getPhase() == sgs.Player_NotActive then return true end
			if self:isWeak(who) then return true end
			if self:hasSkills(sgs.masochism_skill, who) then return true end
		end
		if card:isKindOf("AmazingGrace") and self:hasTrickEffective(card, who, from) then return true end
		return false
	elseif self:isFriend(who) then
		if who:hasSkill("noswuyan") and from:objectName() ~= who:objectName() then return true end
		if card:isKindOf("GodSalvation") and who:isWounded() and self:hasTrickEffective(card, who, from) then
			if self:needToLoseHp(who, nil, nil, true, true) then return true end
			return false
		end
		if card:isKindOf("IronChain") and (self:needKongcheng(who, true) or (who:isChained() and self:hasTrickEffective(card, who, from))) then
			return false
		end
		if card:isKindOf("AmazingGrace") and self:hasTrickEffective(card, who, from) and self:needKongcheng(who, true) then return true end
		if card:isKindOf("AmazingGrace") and self:hasTrickEffective(card, who, from) and not self:needKongcheng(who, true) then return false end
		if sgs.dynamic_value.benefit[card:getClassName()] == true then return false end
		return true
	end
end

function sgs.ai_skill_invoke.kuanhui(self, data)
	local effect = data:toCardUse()
	local tos = effect.to
	local invoke = false
	for _, to in sgs.qlist(tos) do
		if to and need_kuanhui(self, to) then invoke = true break end
	end
	return invoke
end

sgs.ai_skill_playerchosen.kuanhui = function(self, targets)
	targets = sgs.QList2Table(targets)
	self:sort(targets, "defenseSlash")
	for _, player in ipairs(targets) do
	if player and need_kuanhui(self, player) then return player end
	end
end
sgs.ai_playerchosen_intention.kuanhui = function(self, from, to)
	local intention = -77
	local cardx = sgs.Card_Parse(self.room:getTag("kuanhui_user"):toString())
	if not cardx then return end
	if cardx:isKindOf("GodSalvation") and to:isWounded() and not self:needToLoseHp(to, nil, nil, true, true) then intention = 50 end
	if self:needKongcheng(to, true) then intention = 0 end
	if cardx:isKindOf("AmazingGrace") and self:hasTrickEffective(cardx, to) then intention = 0 end
	sgs.updateIntention(from, to, intention)
end


--技能：宏量
sgs.ai_skill_cardask["@HongliangGive"] = function(self, data)
	local damage = data:toDamage()
	local target = damage.from
	local eid = self:getValuableCard(self.player) 
	if not target then return "." end
	if self:needKongcheng(self.player, true) and self.player:getHandcardNum() <= 2 then
		if self.player:getHandcardNum() == 1 then
			local card = self.player:getHandcards():first()
			return (isCard("Jink", card, self.player)) and "." or ("$" .. card:getEffectiveId())
		end
		if self.player:getHandcardNum() == 2 then
			local first = self.player:getHandcards():first()
			local last = self.player:getHandcards():last()			
			local jink = isCard("Jink", first, self.player) and first or (isCard("Jink", last, self.player) and last)		
			if jink then
				return first:getEffectiveId() == jink:getEffectiveId() and ("$"..last:getEffectiveId()) or ("$"..first:getEffectiveId())
			end
		end		
	end
	if self:needToThrowArmor() and self.player:getArmor() then 
		return "$"..self.player:getArmor():getEffectiveId() 
	end
	if self:isFriend(target) then 
		local cards = sgs.QList2Table(self.player:getCards("he"))
		self:sortByCardNeed(cards)
		if self:isWeak(target) then return "$"..cards[#cards]:getEffectiveId() end
		if self:isWeak() then
			for _, card in ipairs(cards) do
				if  card:isKindOf("Peach") or (card:getEffectiveId() == eid) then return "." end
			end
		else	
			return "$"..cards[1]:getEffectiveId() 
		end
	else
		local flag = "h"
		if self:hasSkills(sgs.lose_equip_skill) then flag = "eh" end
		local card2s = sgs.QList2Table(self.player:getCards(flag))
		self:sortByUseValue(card2s, true)
			for _, card in ipairs(card2s) do
				if not self:isValuableCard(card) and (card:getEffectiveId() ~= eid) then return "$"..card:getEffectiveId() end
			end
		end	
	return "." 
end




















