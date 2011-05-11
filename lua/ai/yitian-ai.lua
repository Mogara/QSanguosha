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

-- shaoying
sgs.ai_skill_invoke.shaoying = function(self, data)
	local to = data:toDamage().to:getNextAlive()	
	return self:isEnemy(to)	
end

-- zhenggong
sgs.ai_skill_invoke.zhenggong = true



