-- beige
sgs.ai_skill_invoke.beige = function(self, data)
	local damage = data:toDamage()
	return self:isFriend(damage.to)
end

-- guzheng
sgs.ai_skill_invoke.guzheng = function(self, data)
	local player = data:toPlayer()
	return self:isFriend(player)
end