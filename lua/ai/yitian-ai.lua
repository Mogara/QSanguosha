-- fanji
sgs.ai_skill_invoke.fanji = true

-- lianli
sgs.ai_skill_use["@lianli"] = function(self, prompt)
	self:sort(self.friends)
	
	for _, friend in ipairs(self.friends) do
		if friend:getGeneral():isMale() then
			return "@LianliCard=.->" .. friend:objectName()
		end
	end
	
	return "."	
end

-- tongxin
sgs.ai_skill_invoke.tongxin = true

-- wuling, choose a effect randomly
sgs.ai_skill_choice.wuling = function(self, choices)
	local choices_table = choices:split("+")
	return choices_table[math.random(1, #choices_table)]
end

-- caizhaoji_hujia
sgs.ai_skill_invoke.caizhaoji_hujia = true

-- zhenggong, always invoke
sgs.ai_skill_invoke.zhenggong  = true

sgs.ai_skill_invoke.toudu = function(self, data)
	for _, enemy in ipairs(self.enemies) do
		if self.player:canSlash(enemy, false) then
			return true
		end
	end
end

sgs.ai_skill_playerchosen.toudu = function(self, targets)
	local enemies = {}
	for _, target in sgs.qlist(targets) do
		if self:isEnemy(target) and self.player:canSlash(target, false) then
			table.insert(enemies, enemy)
		end
	end
	
	self:sort(enemies)
	return enemies[1]
end

-- yitian-sword

-- hit enemy when yitian sword was lost
sgs.ai_skill_invoke["yitian-lost"] = function(self, data)
	if next(self.enemies) then
		return true
	else
		return false
	end
end

sgs.ai_skill_playerchosen["yitian_lost"] = function(self, targets)
	self:sort(self.enemies, "hp")
	return self.enemies[1]
end

--jiangboyue
local jiangboyue_ai = SmartAI:newSubclass "jiangboyue"

function jiangboyue_ai:activate(use)
	self:log(type(use))
	if not self.player:hasUsed("LexueCard") then
		self:sort(self.friends_noself, "handcard")
		if #self.friends_noself>0 then
			local friend = self.friends_noself[1]
			if use.to and not friend:isKongcheng() then 
				use.to:append(friend) 
				use.card = sgs.Card_Parse("@LexueCard=.")
				return 
			end
		end
	
		self:sort(self.enemies,"handcard")
		for _,enemy in ipairs(self.enemies) do
			if use.to and not enemy:isKongcheng() then
				use.to:append(enemy) 
				use.card = sgs.Card_Parse("@LexueCard=.")
				return 
			end
		end	
	end
	
	super.activate(self, use)
end

-- zhenwei
sgs.ai_skill_invoke.zhenwei = true

sgs.ai_skill_invoke.yitian = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

-- weiwudi (guixin2)
sgs.ai_skill_invoke.guixin2 = true

local function findPlayerForModifyKingdom(self, players)
	local lord = self.room:getLord()
	local isGood = self:isFriend(lord)

	for _, player in sgs.qlist(players) do
		if player:getRole() == "loyalist" then
			local sameKingdom = player:getKingdom() == lord:getKingdom()
			if isGood ~= sameKingdom then
				return player
			end
		elseif lord:hasLordSkill("xueyi") and not player:isLord() then
			local isQun = player:getKingdom() == "qun"
			if isGood ~= isQun then
				return player
			end
		end
	end
end

local function chooseKingdomForPlayer(self, to_modify)
	local lord = self.room:getLord()
	local isGood = self:isFriend(lord)
	if to_modify:getRole() == "loyalist" or to_modify:getRole() == "renegade" then
		if isGood then
			return lord:getKingdom()
		else
			-- find a kingdom that is different from the lord
			local kingdoms = {"wei", "shu", "wu", "qun"}
			for _, kingdom in ipairs(kingdoms) do
				if lord:getKingdom() ~= kingdom then
					return kingdom
				end
			end
		end
	elseif lord:hasLordSkill("xueyi") and not to_modify:isLord() then
		return isGood and "qun" or "wei"
	end

	return "wei"
end

sgs.ai_skill_choice.guixin2 = function(self, choices)
	if choices == "wei+shu+wu+qun" then
		local to_modify = self.room:getTag("Guixin2Modify"):toPlayer()
		return chooseKingdomForPlayer(self, to_modify)
	end

	if choices ~= "modify+obtain" then
		return choices:split("+")[1]
	end

	-- two choices: modify and obtain
	if self.player:getRole() == "renegade" or self.player:getRole() == "lord" then
		return "obtain"
	end
	
	local lord = self.room:getLord()
	local skills = lord:getVisibleSkillList()
	local hasLordSkill = false
	for _, skill in sgs.qlist(skills) do
		if skill:isLordSkill() then
			hasLordSkill = true
			break
		end
	end

	if not hasLordSkill then
		return "obtain"
	end

	local players = self.room:getOtherPlayers(self.player)
	players:removeOne(lord)
	if findPlayerForModifyKingdom(self, players) then
		return "modify"
	else
		return "obtain"
	end
end

sgs.ai_skill_playerchosen.guixin2 = function(self, players)
	local player = findPlayerForModifyKingdom(self, players)
	return player or players:first()
end

-- Lu Kang's Weiyan
sgs.ai_skill_invoke.lukang_weiyan = function(self, data)
	local handcard = self.player:getHandcardNum()
	local max_card = self.player:getMaxCards()

	if self.player:getPhase() == sgs.Player_Draw then
		-- weiyan1: Draw -> Play
		return handcard >= max_card
	elseif self.player:getPhase() == sgs.Player_Play then
		-- weiyan2: Play -> Draw
		return handcard < max_card
	end
end