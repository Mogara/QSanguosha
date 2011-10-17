-- this scripts contains the AI classes for generals of fire package

-- bazhen
sgs.ai_skill_invoke.bazhen = true

-- niepan
sgs.ai_skill_invoke.niepan = function(self, data)
	local dying = data:toDying()
	local peaches = 1 - dying.who:getHp()

	local cards = self.player:getHandcards()
	local n = 0
	for _, card in sgs.qlist(cards) do
		if card:inherits "Peach" or card:inherits "Analeptic" then
			n = n + 1
		end
	end

	return n < peaches
end

local quhu_skill={}
quhu_skill.name="quhu"
table.insert(sgs.ai_skills,quhu_skill)
quhu_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("QuhuCard") and not self.player:isKongcheng() then
		local max_card = self:getMaxCard()
		return sgs.Card_Parse("@QuhuCard=" .. max_card:getEffectiveId())
	end
end

sgs.ai_skill_use_func["QuhuCard"] = function(card, use, self)
	local max_card = self:getMaxCard()
	local max_point = max_card:getNumber()
	self:sort(self.enemies, "handcard")

	for _, enemy in ipairs(self.enemies) do
		if enemy:getHp() > self.player:getHp() then
			local enemy_max_card = self:getMaxCard(enemy)
			if enemy_max_card and max_point > enemy_max_card:getNumber() then
				for _, enemy2 in ipairs(self.enemies) do
					if (enemy:objectName() ~= enemy2:objectName()) and enemy:inMyAttackRange(enemy2) then
						local card_id = max_card:getEffectiveId()
						local card_str = "@QuhuCard=" .. card_id
						if use.to then
							use.to:append(enemy)					
						end
						use.card = sgs.Card_Parse(card_str)
						return
					end
				end
			end
		end
	end
	if not self.player:isWounded() or (self.player:getHp() == 1 and self:getCardsNum("Analeptic") > 0) then
		local use_quhu
		for _, friend in ipairs(self.friends) do
			if math.min(5, friend:getMaxHP()) - friend:getHandcardNum() >= 2 then
				self:sort(self.enemies, "handcard")
				if self.enemies[#self.enemies]:getHandcardNum() > 0 then use_quhu = true break end
			end
		end
		if use_quhu then
			for _, enemy in ipairs(self.enemies) do
				if not enemy:isKongcheng() and self.player:getHp() < enemy:getHp() then 
					local cards = self.player:getHandcards()
					cards = sgs.QList2Table(cards)
					self:sortByUseValue(cards, true)
					local card_id = cards[1]:getEffectiveId()
					local card_str = "@QuhuCard=" .. card_id
					if use.to then
						use.to:append(enemy)
					end
					use.card = sgs.Card_Parse(card_str)
					return
				end
			end
		end
	end
end

sgs.ai_skill_playerchosen.quhu = function(self, targets)
	for _, player in sgs.qlist(targets) do
		if self:isEnemy(player) then
			return player
		end
	end
end

sgs.ai_skill_use["@@jieming"] = function(self, prompt)
	self:sort(self.friends)

	local max_x = 0
	local target
	for _, friend in ipairs(self.friends) do
		local x = math.min(friend:getMaxHP(), 5) - friend:getHandcardNum()		

		if x > max_x then
			max_x = x
			target = friend
		end
	end

	if target then
		return "@JiemingCard=.->" .. target:objectName()
	else
		return "."
	end
end

-- mengjin
sgs.ai_skill_invoke.mengjin = function(self, data)
	local effect = data:toSlashEffect()
	return not self:isFriend(effect.to)
end

local qiangxi_skill={}
qiangxi_skill.name="qiangxi"
table.insert(sgs.ai_skills,qiangxi_skill)
qiangxi_skill.getTurnUseCard=function(self)
	if not self.player:hasUsed("QiangxiCard") then
		return sgs.Card_Parse("@QiangxiCard=.")
	end
end

sgs.ai_skill_use_func["QiangxiCard"] = function(card, use, self)
	local weapon = self.player:getWeapon()
	if weapon then
		local hand_weapon, cards
		cards = self.player:getHandcards()
		for _, card in sgs.qlist(cards) do
			if card:inherits("Weapon") then
				hand_weapon = card
				break
			end
		end
		self:sort(self.enemies)
		for _, enemy in ipairs(self.enemies) do
			if hand_weapon and self.player:inMyAttackRange(enemy) then
				use.card = sgs.Card_Parse("@QiangxiCard=" .. hand_weapon:getId())
				if use.to then
					use.to:append(enemy)
				end
				break
			end
			if self.player:distanceTo(enemy) <= 1 then
				use.card = sgs.Card_Parse("@QiangxiCard=" .. weapon:getId())
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	else
		self:sort(self.enemies, "hp")
		for _, enemy in ipairs(self.enemies) do
			if self.player:inMyAttackRange(enemy) and self.player:getHp() > enemy:getHp() and self.player:getHp() > 2 then
				use.card = sgs.Card_Parse("@QiangxiCard=.")
				if use.to then
					use.to:append(enemy)
				end
				return
			end
		end
	end
end
	
--shuangxiong

sgs.ai_skill_invoke["shuangxiong"]=function(self,data)
    if self.player:isSkipped(sgs.Player_Play) or self.player:getHp() < 2 then
		return false
	end
    
    local cards=self.player:getCards("h")
    cards=sgs.QList2Table(cards)
    
    local handnum=0
    
    for _,card in ipairs(cards) do  
        if self:getUseValue(card)<8 then
			handnum=handnum+1
		end
    end
    
    handnum=handnum/2
    self:sort(self.enemies, "hp")
    for _, enemy in ipairs(self.enemies) do
        if (self:getCardsNum("Slash", enemy)+enemy:getHp()<=handnum) and (self:getCardsNum("Slash")>=self:getCardsNum("Slash", enemy)) then return true end
    end
	
    return self.player:getHandcardNum()>=self.player:getHp()
end
