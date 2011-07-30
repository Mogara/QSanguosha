--qiaobian
sgs.ai_skill_use["@qiaobian"] = function(self, prompt)
    self:updatePlayers()
	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards, true)
	local card = cards[1]
	
	if prompt == "@qiaobian-judge" then
		if (self.player:containsTrick("supplyshortage") and self.player:getHp() > self.player:getHandcardNum()) or
			(self.player:containsTrick("indulgence") and self.player:getHandcardNum() > self.player:getHp()-1) or
			(self.player:containsTrick("lightning") and not self:hasWizard(self.friends) and self:hasWizard(self.enemies)) or
			(self.player:containsTrick("lightning") and #self.friends > #self.enemies) then
			return "@QiaobianCard=" .. card:getEffectiveId() .."->."
		end
	end
	
	if prompt == "@qiaobian-draw" then
		self:sort(self.enemies, "handcard")
		local first, second
		if #self.enemies > 1 then
			first, second = self.enemies[1], self.enemies[2]
			if first:getHandcardNum() > 0 and second:getHandcardNum() > 0 then
				return "@QiaobianCard=" .. card:getEffectiveId() .."->".. first:objectName() .."+".. second:objectName()
			end
		elseif #self.enemies == 1 and #self.friends > 1 then
			first = self.enemies[1]
			if first:getHandcardNum() > 0 then
				return "@QiaobianCard=" .. card:getEffectiveId() .."->".. first:objectName()
			end
		end
	end
	
	if prompt == "@qiaobian-play" then
		if self.player:getHandcardNum()-2 > self.player:getHp() then return "." end
		
		self:sort(self.enemies, "hp")
		local has_armor = true
		local judge
		for _, friend in ipairs(self.friends_noself) do
			if (friend:containsTrick("lightning") and self:hasWizard(self.friends) and self:hasWizard(self.enemied)) or
				(friend:containsTrick("indulgence") and friend:getHandcardNum()-1 > friend:getHp()) or 
				friend:containsTrick("supplyshortage") then
				return "@QiaobianCard=" .. card:getEffectiveId() .."->".. friend:objectName()
			end
		end	
		
		for _, friend in ipairs(self.friends_noself) do
			if self:hasSkills(sgs.lose_equip_skill, friend) then
				return "@QiaobianCard=" .. card:getEffectiveId() .."->".. friend:objectName()
			end
			if not friend:getArmor() then has_armor = false end
		end
		
		local top_value = 0
		for _, hcard in ipairs(cards) do
			if not hcard:inherits("Jink") then
				if self:getUseValue(hcard) > top_value then	top_value = self:getUseValue(hcard) end
			end
		end
		if top_value >= 3.7 then return "." end
		
		local targets = {}
		for _, enemy in ipairs(self.enemies) do
			if enemy:getArmor() and not has_armor then
				table.insert(targets, enemy)
			end
		end
		
		self:sort(targets, "defense")
		for _, target in ipairs(targets) do
			return "@QiaobianCard=" .. card:getEffectiveId() .."->".. target:objectName()
		end
	end
	
	if prompt == "@qiaobian-discard" then
		if self.player:getHandcardNum()-1 > self.player:getHp() then 
			return "@QiaobianCard=" .. card:getEffectiveId() .."->."
		end
	end
	
	return "."
end

sgs.ai_skill_playerchosen.qiaobian = function(self, targets)
	local has_delay = false;
	for _, friend in ipairs(self.friends_noself) do
		if friend:getCards("j") and friend:getCards("j"):length() > 0 then
			has_delay = true
			break
		end
	end
	
	for _, target in sgs.qlist(targets) do
		if not has_delay and self:isFriend(target) then
			return friend
		end
		if has_delay and self:isEnemy(target) then 
			return enemy
		end
	end
end

-- beige
sgs.ai_skill_invoke.beige = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

-- guzheng
sgs.ai_skill_invoke.guzheng = function(self, data)
	local player = self.room:getCurrent()	
	return self:isFriend(player) or data:toInt() >= 3		
end

sgs.ai_skill_invoke.tuntian = true

sgs.ai_skill_invoke.fangquan = function(self, data)
	if #self.friends == 1 then
		return false
	end

	local limit = self.player:getMaxCards()
	return self.player:getHandcardNum() <= limit
end

sgs.ai_skill_playerchosen.fangquan = function(self, targets)
	for _, target in sgs.qlist(targets) do
		if self:isFriend(target) then
			return target
		end
	end
end