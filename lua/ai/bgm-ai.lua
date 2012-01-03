sgs.ai_skill_invoke.chongzhen = function(self, data)
	local target = self.player:getTag("ChongZhenTarget"):toPlayer()
	return not self:isFriend(target)
end

sgs.ai_skill_invoke.huantong = sgs.ai_skill_invoke.fanqun
