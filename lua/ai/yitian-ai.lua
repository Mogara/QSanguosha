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
	if not self.lexue_used then
		self:sort(self.friends_noself, "handcard")
		if #self.friends_noself>0 then
			local friend = self.friends_noself[1]
			if use.to and not friend:isKongcheng() then 
				use.to:append(friend) 
				use.card = sgs.Card_Parse("@LexueCard=.")
				self.lexue_used = true
				return 
			end
		end
	
		self:sort(self.enemies,"handcard")
		for _,enemy in ipairs(self.enemies) do
			if use.to and not enemy:isKongcheng() then
				use.to:append(enemy) 
				use.card = sgs.Card_Parse("@LexueCard=.")
				self.lexue_used = true
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