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
		if self:isEnemy(target) then
			table.insert(enemies, enemy)
		end
	end
	
	self:sort(enemies)
	return enemies[1]
end

-- yitian-sword
sgs.ai_skill_invoke.yitian_sword = function(self, data)
	if next(self.enemies) then
		return true
	else
		return false
	end
end

sgs.ai_skill_playerchosen.yitian_sword = sgs.ai_skill_playerchosen.toudu