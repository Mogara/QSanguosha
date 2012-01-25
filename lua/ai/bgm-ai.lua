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
