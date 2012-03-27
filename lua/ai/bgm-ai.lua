sgs.ai_skill_invoke.chongzhen = function(self, data)
	local target = self.player:getTag("ChongZhenTarget"):toPlayer()
	if self:isFriend(target) then
		return target:hasSkill("kongcheng") and target:getHandcardNum() == 1
	else
		return not (target:hasSkill("kongcheng") and target:getHandcardNum() == 1 and target:getEquips():isEmpty())
	end
end

sgs.ai_skill_invoke.huantong = function(self, data)
	if self.room:getLord():getKingdom() == "qun" then return true
	elseif self.room:getLord():getKingdom() == "shu" then return false
	elseif math.random(0, 1) == 0 then return true else return false end
end

--AI for BGM Diaochan
--code by clarkcyt and William915

local lihun_skill={}
lihun_skill.name="lihun"
table.insert(sgs.ai_skills,lihun_skill)
lihun_skill.getTurnUseCard=function(self)
	if self.player:hasUsed("LihunCard") or self.player:isNude() then return end
	local card_id
	if (self:isEquip("SilverLion") and self.player:isWounded()) or self:evaluateArmor() < -5 then
		return sgs.Card_Parse("@LihunCard=" .. self.player:getArmor():getId())
	elseif self.player:getHandcardNum() > self.player:getHp() then
		local cards = self.player:getHandcards()
		cards=sgs.QList2Table(cards)

		for _, acard in ipairs(cards) do
			if (acard:getTypeId() ~= sgs.Card_Trick or acard:inherits("AmazingGrace"))
				and not acard:inherits("Peach") and not acard:inherits("Shit") then
				card_id = acard:getEffectiveId()
				break
			end
		end
	elseif not self.player:getEquips():isEmpty() then
		local player=self.player
		if player:getWeapon() then card_id=player:getWeapon():getId()
		elseif player:getOffensiveHorse() then card_id=player:getOffensiveHorse():getId()
		elseif player:getDefensiveHorse() then card_id=player:getDefensiveHorse():getId()
		elseif player:getArmor() and player:getHandcardNum()<=1 then card_id=player:getArmor():getId()
		end
	end
	if not card_id then
		cards=sgs.QList2Table(self.player:getHandcards())
		for _, acard in ipairs(cards) do
			if (acard:getTypeId() ~= sgs.Card_Trick or acard:inherits("AmazingGrace"))
				and not acard:inherits("Peach") and not acard:inherits("Shit") then
				card_id = acard:getEffectiveId()
				break
			end
		end
	end
	if not card_id then
		return nil
	else
		return sgs.Card_Parse("@LihunCard=" .. card_id)
	end
end

sgs.ai_skill_use_func.LihunCard = function(card,use,self)
	local cards=self.player:getHandcards()
	cards=sgs.QList2Table(cards)

	if not self.player:hasUsed("LihunCard") then
		self:sort(self.enemies, "hp")
		local target
		for _, enemy in ipairs(self.enemies) do
			if enemy:getGeneral():isMale() and not enemy:hasSkill("kongcheng") then
			    if (enemy:hasSkill("lianying") and self:damageMinusHp(self, enemy, 1) > 0) or
				   (enemy:getHp() < 3 and self:damageMinusHp(self, enemy, 0) > 0  and enemy:getHandcardNum() > 0) or
				   (enemy:getHandcardNum() >= enemy:getHp() and enemy:getHp() > 2 and self:damageMinusHp(self, enemy, 0) >= -1) or
				   (enemy:getHandcardNum() - enemy:getHp() > 4) then
					target = enemy
					break
				end
			end
		end

		if target then
			use.card = card
			if use.to then
				use.to:append(target)
			end
		end
	end
end

sgs.ai_skill_cardchosen.lihun = function(self)
	if self:isEquip("SilverLion") and self.player:isWounded() or self:evaluateArmor() < -5 then
		return self.player:getArmor()
	end
	local shit = self:getCard("Shit")
	if shit then return shit end
	local cards = sgs.QList2Table(self.player:getHandcards())
	self:sortByKeepValue(cards)
	return cards[1]
end

sgs.ai_use_value.LihunCard = 8.5
sgs.ai_use_priority.LihunCard = 6

--AI for BGM Caoren

sgs.ai_skill_invoke.jiagu = function(self, data)
    if math.random(0, 1) == 0 then return true else return false end
end

function sgs.ai_skill_invoke.kuiwei(self, data)
    local weapon = 0
    if not self.player:faceUp() then return true end
	for _, friend in ipairs(self.friends) do
		if self:hasSkills("fangzhu|jilve", friend) then return true end
	end
	for _, aplayer in sgs.qlist(self.room:getAlivePlayers()) do
	    if aplayer:getWeapon() then weapon = weapon + 1 end
	end
	if weapon >1 then return true end
	return self:isWeak()
end

sgs.ai_view_as.yanzheng = function(card, player, card_place)
	local suit = card:getSuitString()
	local number = card:getNumberString()
	local card_id = card:getEffectiveId()
	if card_place == sgs.Player_Equip then
	    return ("nullification:yanzheng[%s:%s]=%d"):format(suit, number, card_id)
	end
end

-- AI for bgm_pangong

sgs.ai_skill_invoke.manjuan = true
sgs.ai_skill_invoke.zuixiang = true

sgs.ai_skill_askforag.manjuan = function(self, card_ids)
	local cards = {}
	for _, card_id in ipairs(card_ids) do
		table.insert(cards, sgs.Sanguosha:getCard(card_id))
	end
	for _, card in ipairs(cards) do
		if card:inherits("ExNihilo") then return card:getEffectiveId() end
	end
	for _, card in ipairs(cards) do
		if card:inherits("Snatch") then
			self:sort(self.enemies,"defense")
			if sgs.getDefense(self.enemies[1]) >= 8 then self:sort(self.enemies, "threat") end
			local enemies = self:exclude(self.enemies, card)
			for _,enemy in ipairs(enemies) do
				if self:hasTrickEffective(card, enemy) then
					return card:getEffectiveId()
				end
			end
		end
	end
	for _, card in ipairs(cards) do
		if card:inherits("Peach") and self.player:isWounded() and self:getCardsNum("Peach") < self.player:getLostHp() then return card:getEffectiveId() end
	end
	for _, card in ipairs(cards) do
		if card:inherits("AOE") and self:getAoeValue(card) > 0 then return card:getEffectiveId() end
	end
	self:sortByCardNeed(cards)
	return cards[#cards]:getEffectiveId()
end