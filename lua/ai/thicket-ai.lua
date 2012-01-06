-- zaiqi
sgs.ai_skill_invoke["zaiqi"] = function(self, data)
	return self.player:getLostHp() >= 2
end

-- Sunjian's AI
sgs.ai_skill_choice.yinghun = function(self, choices)
	return self.yinghunchoice
end

sgs.ai_skill_use["@@yinghun"] = function(self, prompt)
	local x = self.player:getLostHp()
	if x == 1 and #self.friends == 1 then
		return "."
	end

	if #self.friends > 1 then
		for _, friend in ipairs(self.friends_noself) do
			if self:hasSkills(sgs.lose_equip_skill, friend) and friend:getEquips():length()>x/2 then
				self.yinghun = friend
				self.yinghunchoice = "d1tx"
				break
			end
		end
		self:sort(self.friends, "chaofeng")
		self.yinghun = self.friends_noself[1]
		self.yinghunchoice = "dxt1"
	else
		self:sort(self.enemies, "handcard")
		for index = #self.enemies, 1, -1 do
			local enemy = self.enemies[index]
			if not self:hasSkills(sgs.lose_equip_skill, enemy) or enemy:getEquips():length()<x/2 then
				self.yinghun = enemy
				self.yinghunchoice = "d1tx"
				break
			end
		end
	end

	if self.yinghun then
		return "@YinghunCard=.->" .. self.yinghun:objectName()
	else
		return "."
	end
end

-- xingshang
sgs.ai_skill_invoke.xingshang = function(self, data)
	local damage = data:toDamageStar()
	if not damage then return true end
	local cards = damage.to:getHandcards()
	local shit_num = 0
	for _, card in sgs.qlist(cards) do
		if card:inherits("Shit") then
			shit_num = shit_num + 1
			if card:getSuit() == sgs.Card_Spade then
				shit_num = shit_num + 1
			end
		end
	end
	if shit_num > 1 then return false end
	return true
end

-- fangzhu, fangzhu
sgs.ai_skill_use["@@fangzhu"] = function(self, prompt)
	self:sort(self.friends_noself)
	local target
	for _, friend in ipairs(self.friends_noself) do
		if not friend:faceUp() then
			target = friend
			break
		end

		if friend:hasSkill("jushou") and friend:getPhase() == sgs.Player_Play then
			target = friend
			break
		end
	end

	if not target then
		local x = self.player:getLostHp()
		if x >= 3 then
			target = self.friends_noself[1]
		else
			self:sort(self.enemies)
			for _, enemy in ipairs(self.enemies) do
				if enemy:faceUp() then
					target = enemy
					break
				end
			end
		end
	end

	if target then
		return "@FangzhuCard=.->" .. target:objectName()
	else
		return "."
	end
end

sgs.ai_skill_invoke.songwei = function(self, data)
	local who = data:toPlayer()
	return self:isFriend(who)
end

-- baonue
sgs.ai_skill_invoke.baonue = sgs.ai_skill_invoke.songwei

local function getLowerBoundOfHandcard(self)
	local least = math.huge
	local players = self.room:getOtherPlayers(self.player)
	for _, player in sgs.qlist(players) do
		least = math.min(player:getHandcardNum(), least)
	end

	return least
end

local function getBeggar(self)
	local least = getLowerBoundOfHandcard(self)

	self:sort(self.friends_noself)
	for _, friend in ipairs(self.friends_noself) do
		if friend:getHandcardNum() == least then
			return friend
		end
	end

	for _, player in sgs.qlist(self.room:getOtherPlayers(self.player)) do
		if player:getHandcardNum() == least then
			return player
		end
	end
end

-- haoshi
sgs.ai_skill_invoke.haoshi = function(self, data)
	if self.player:getHandcardNum() <= 1 and not self.player:hasSkill("yongsi") then
		return true
	end

	local beggar = getBeggar(self)
	return self:isFriend(beggar)
end

sgs.ai_skill_use["@@haoshi!"] = function(self, prompt)
	local beggar = getBeggar(self)

	local cards = self.player:getHandcards()
	cards = sgs.QList2Table(cards)
	self:sortByUseValue(cards,true)
	local card_ids = {}
	for i=1, math.floor(#cards/2) do
		table.insert(card_ids, cards[i]:getEffectiveId())
	end

	return "@HaoshiCard=" .. table.concat(card_ids, "+") .. "->" .. beggar:objectName()
end

sgs.ai_skill_invoke.lieren = function(self, data)
	local damage = data:toDamage()
	if self:isEnemy(damage.to) then
		if self.player:getHandcardNum()>=self.player:getHp() then return true
		else return false
		end
	end

	return false
end
