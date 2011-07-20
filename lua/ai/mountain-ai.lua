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