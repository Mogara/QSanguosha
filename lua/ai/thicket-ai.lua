-- zaiqi
sgs.ai_skill_invoke["zaiqi"] = function(self, data)
	return self.player:getLostHp() >= 2
end

-- Sunjian's AI
sgs.ai_skill_choice.yinghun = function(self, choices)
	if self:isFriend(self.yinghun) then
		return "dxt1"
	else
		return "d1tx"
	end
end

sgs.ai_skill_use["@@yinghun"] = function(self, prompt)
	local x = self.player:getLostHp()
	if x == 1 and #self.friends == 1 then
		return "."
	end

	if #self.friends > 1 then
		self:sort(self.friends, "chaofeng")
		self.yinghun = self.friends_noself[1]
	else
		self:sort(self.enemies, "chaofeng")
		self.yinghun = self.enemies[1]
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
	return self:isFriend(self.room:getLord())
end

-- baonue
sgs.ai_skill_invoke.baonue = function(self, data)
	return self.player:getRole() == "loyalist"
end

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
	if self.player:getHandcardNum() <= 1 then
		return true
	end

	local beggar = getBeggar(self)
	return self:isFriend(beggar)
end

sgs.ai_skill_use["@@haoshi!"] = function(self, prompt)
	local beggar = getBeggar(self)

	local cards = self.player:getHandcards()
	local n = math.floor(self.player:getHandcardNum()/2)
	local card_ids = {}
	for i=1, n do
		table.insert(card_ids, cards:at(i-1):getEffectiveId())
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
