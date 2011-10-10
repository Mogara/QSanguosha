-- Wisdom's AI by Ubun.
-- shien
sgs.ai_skill_invoke.shien = function(self, data)
    return self:isFriend(data:toPlayer())
end

-- tanlan
sgs.ai_skill_invoke.tanlan = function(self, data)
    local damage = data:toDamage()
	return self:isEnemy(damage.from)
end

-- yicai
sgs.ai_skill_invoke.yicai = function(self, data)
	return true
end

-- bawang
sgs.ai_skill_invoke.bawang = function(self, data)
	local effect = data:toSlashEffect()
	return self:isEnemy(effect.to)
end

sgs.ai_skill_use["@@bawang"] = function(self, prompt)
	local first_index, second_index
	for i=1, #self.enemies-1 do																			
		if (self.enemies[i]:hasSkill("kongcheng") and self.enemies[i]:getHandcardNum() == 0)  then 
				local bullshit
				
		elseif not first_index then 
				first_index = i 
			else 
				second_index = i 
		end
		if second_index then break end
	end
	
	if first_index and not second_index then
		local others = self.room:getOtherPlayers(self.player)
		for _, other in sgs.qlist(others) do
			if (not self:isFriend(other) or (self:hasSkills(sgs.need_kongcheng, other) and other:getHandcardNum() == 0)) and 
				self.enemies[first_index]:objectName() ~= other:objectName() then 
				return ("@BawangCard=.->%s+%s"):format(self.enemies[first_index]:objectName(), other:objectName())
			end
		end
	end
	
	if not second_index then return "." end
	
	self:log(self.enemies[first_index]:getGeneralName() .. "+" .. self.enemies[second_index]:getGeneralName())
	local first = self.enemies[first_index]:objectName()
	local second = self.enemies[second_index]:objectName()
	return ("@BawangCard=.->%s+%s"):format(first, second)
end

-- fuzuo
sgs.ai_skill_choice.fuzuo = function(self , choices)
    return "cancel"
end
