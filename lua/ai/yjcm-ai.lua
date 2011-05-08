-- pojun
sgs.ai_skill_invoke.pojun = function(self, data)
	local damage = data:toDamage()
	local good = damage.to:getHp() > 2
	
	if self:isFriend(damage.to) then
		return good
	elseif self:isEnemy(damage.to) then
		return not good
	end
end

-- buyi
sgs.ai_skill_invoke.buyi = function(self, data)
	local dying = data:toDying()
	return self:isFriend(dying.who)
end